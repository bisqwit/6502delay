; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, and S
; Will preserve X, Y, and P
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
; - 4 bytes or less when n <= 17
; - 5 bytes or less when n <= 21
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 31
; - 8 bytes or less when n <= 295
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 320
; -11 bytes or less when n <= 1324
; -12 bytes or less when n <= 2050
; -13 bytes or less when n <= 3044
; -14 bytes or less when n <= 3578
; -15 bytes or less when n <= 4605
; -16 bytes or less when n <= 7944
; -17 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=3, avg=4.1, max=5
; - When 21 <= n <= 30: min=5, avg=6.3, max=7
; - When 31 <= n <= 50: min=6, avg=7.0, max=8
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=10
; - When 301 <= n <= 500: min=8, avg=9.6, max=11
; - When 501 <= n <= 750: min=7, avg=9.6, max=11
; - When 751 <= n <= 1000: min=7, avg=9.6, max=11
; - When 1001 <= n <= 2000: min=7, avg=10.4, max=12
; - When 2001 <= n <= 3000: min=7, avg=11.5, max=13
; - When 3001 <= n <= 5000: min=7, avg=12.0, max=16
; - When 5001 <= n <= 7500: min=7, avg=12.3, max=16
; - When 7501 <= n <= 10000: min=7, avg=12.5, max=17
; - When 10001 <= n <= 20000: min=7, avg=12.5, max=17
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
.elseif(n = 4 || n = 5 || n = 8 || n = 16 || n = 18 || n = 30 || n = 32)
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
.elseif((n > 8 && n < 12) || n = 13 || n = 19 || (n > 20 && n < 24))
	; 3..6 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15 || n = 24)
	; 4..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17 || n = 20 || n = 26 || n = 28 || n = 29)
	; 4..7 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 25)
	; 7 bytes
	php
	lda #0
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 27)
	; 7 bytes
	php
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	plp
.elseif(n = 31)
	; 6 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 33)
	; 7 bytes
	php
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 34)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 35)
	; 8 bytes
	php
	clc
	clc
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif((n > 35 && n < 46) || (n > 46 && n < 157))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 46)
	; 6 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	plp
.endif
.else
; 157 to 312
.if((n > 156 && n < 292))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 292 || n = 293 || n = 299 || n = 309)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 294 || n = 308)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 295)
	; 8 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	plp
.elseif((n > 295 && n < 299))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 300 || n = 310)
	; 9 bytes
	php
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	plp
.elseif(n = 301 || n = 303)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 302 || n = 307)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 304)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 305)
	; 8 bytes
	php
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	Jne *-2
	plp
.elseif(n = 306)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 311)
	; 9 bytes
	php
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	nop
	Jne *-3
	plp
.elseif(n = 312)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 369 || n = 376)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 314 || n = 319 || n = 321 || n = 324 || n = 326 || n = 329 || n = 331 || n = 344 || n = 349 || n = 351 || n = 354 || n = 359 || n = 364 || n = 366 || n = 379 || n = 384 || n = 389)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 335 || n = 340 || n = 350 || n = 355 || n = 365 || n = 370 || n = 375 || n = 380)
	; 9 bytes
	php
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	plp
.elseif(n = 316)
	; 10 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
	plp
.elseif(n = 317 || n = 322 || n = 327 || n = 352 || n = 357 || n = 372)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 318)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 320 || n = 325 || n = 330 || n = 345 || n = 360 || n = 385)
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 323 || n = 328 || n = 333 || n = 338 || n = 343 || n = 356 || n = 358 || n = 363 || n = 373 || n = 378)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && ((n > 331 && n < 348) || n = 362 || n = 367 || (n > 376 && n < 388)))
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 334 || n = 341 || n = 348 || n = 383)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 336 || n = 371)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 339 || n = 374 || n = 381)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 346)
	; 9 bytes
	php
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
	plp
.elseif(n = 353)
	; 9 bytes
	php
	clc
	lda #$EB ;hides 'sbc #$3A'
	.byte $3A ;nop ;first loop only
	nop
	Jne *-3
	plp
.elseif(n = 361)
	; 10 bytes
	php
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	delay_n 4
	Jne *-4
	plp
.elseif(n = 368)
	; 10 bytes
	php
	clc
	lda #216 ;hides 'cld'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 386)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 388)
	; 10 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 390)
	; 8 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	plp
.endif
.else
; 391 to 468
.if(n = 391 || n = 394 || n = 399 || n = 401 || n = 419 || n = 424 || n = 429 || n = 431 || n = 434 || n = 449 || n = 454 || n = 459 || n = 461 || n = 464)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 392 || n = 407 || n = 417 || n = 422 || n = 432 || n = 462 || n = 467)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && ((n > 392 && n < 414) || (n > 427 && n < 449) || n = 463 || n = 468))
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 0) && ((n > 394 && n < 426) || (n > 439 && n < 456) || n = 465))
	; 9 bytes
	php
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	plp
.elseif(n = 396)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 5 = 2) && (n = 397 || n = 402 || n = 412 || n = 427 || (n > 436 && n < 458)))
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 404 || n = 418)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 406)
	; 10 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 409)
	; 9 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 411 || n = 439 || n = 446 || n = 453)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 414)
	; 9 bytes
	php
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 416)
	; 9 bytes
	php
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 421)
	; 9 bytes
	php
	sec
	lda #58 ;hides 'nop'
	sbc #1
	Jpl *-3
	plp
.elseif(n = 423)
	; 9 bytes
	php
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 426)
	; 10 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_14cyc
	Jne *-5
	plp
.elseif(n = 430 || n = 435 || n = 460)
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 436)
	; 11 bytes
	php
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 441)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 444 || n = 451 || n = 458)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 456)
	; 10 bytes
	php
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	delay_n 9
	Jne *-5
	plp
.elseif(n = 466)
	; 10 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
	tay
	plp
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 471 || n = 484 || n = 489 || n = 494 || n = 499 || n = 504 || n = 506 || n = 519 || n = 524 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 541 || n = 542 || n = 546)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 470 || n = 485 || n = 495 || n = 500 || n = 505 || n = 520 || n = 525)
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 472 || n = 482 || n = 497 || n = 507 || n = 527)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 473 || n = 501 || n = 503 || n = 508 || n = 513 || n = 518 || n = 533 || n = 535 || n = 540)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 474 || n = 481 || n = 509 || n = 523)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 475 || n = 480 || n = 490 || n = 510 || n = 515 || n = 530 || n = 545)
	; 9 bytes
	php
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	plp
.elseif(n = 476)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 477 || n = 487 || n = 492 || n = 502 || n = 512 || n = 517 || n = 522)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 478)
	; 10 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
	tay
	plp
.elseif(n = 479)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 483)
	; 10 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	iny
	Jne *-3
	tay
	plp
.elseif(n = 486 || n = 493 || n = 521 || n = 528)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 488 || n = 516)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 491)
	; 10 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 496)
	; 10 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jmi *-3
	tay
	plp
.elseif(n = 498)
	; 9 bytes
	php
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jpl *-3
	plp
.elseif(n = 511)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 514)
	; 9 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
	plp
.elseif(n = 526)
	; 10 bytes
	php
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	delay_n 7
	Jne *-4
	plp
.elseif(n = 532)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 537)
	; 7 bytes
	php
	lda #2
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 538 || n = 543)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 548 || n = 573 || n = 578 || n = 583 || n = 588 || n = 603 || n = 608 || n = 623)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 549)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif((n .mod 5 = 0) && ((n > 549 && n < 561) || n = 575 || (n > 589 && n < 601) || (n > 609 && n < 626)))
	; 9 bytes
	php
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	plp
.elseif(n = 551)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 553)
	; 10 bytes
	php
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 554 || n = 559 || n = 564 || n = 566 || n = 569 || n = 574 || n = 581 || n = 589 || n = 594 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 556 || n = 584 || n = 591 || n = 619)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 558 || n = 593 || n = 614 || n = 621)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 561)
	; 10 bytes
	php
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jpl *-4
	plp
.elseif(n = 563)
	; 10 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
	plp
.elseif(n = 565 || n = 570 || n = 580 || n = 585 || n = 605)
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 568)
	; 10 bytes
	php
	tya
	ldy #122 ;hides 'nop'
	dey
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 571)
	; 10 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
	tay
	plp
.elseif(n = 576)
	; 11 bytes
	php
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 577 || n = 582 || n = 587 || n = 592 || n = 602 || n = 622)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 579 || n = 586)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 596)
	; 11 bytes
	php
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 598)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 606)
	; 11 bytes
	php
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 613)
	; 10 bytes
	php
	tya
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
	plp
.elseif(n = 616)
	; 10 bytes
	php
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	jsr @nearby_rts_14cyc
	Jne *-5
	plp
.elseif(n = 618)
	; 10 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	nop $04
	Jmi *-3
	tax
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
.if(n = 626 || n = 661 || n = 668)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 627 || n = 632 || n = 637 || n = 647 || n = 652 || n = 657)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 628 || n = 656 || n = 691)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 629 || n = 634 || n = 639 || n = 644 || n = 659 || n = 664 || n = 669 || n = 671 || n = 674 || n = 679 || n = 681 || n = 686 || n = 694 || n = 699)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 630 || n = 635 || n = 640 || n = 645 || n = 655)
	; 9 bytes
	php
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	plp
.elseif(n = 631)
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jmi *-3
	plp
.elseif(n = 633)
	; 9 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
	plp
.elseif(n = 636 || n = 641 || n = 643 || n = 648 || n = 653 || n = 673 || n = 678 || n = 683 || n = 688 || n = 693 || n = 698)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 638)
	; 9 bytes
	php
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif((n .mod 5 = 2) && (n = 642 || (n > 661 && n < 703)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 646)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 649 || n = 663 || n = 684)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif((n .mod 5 = 0) && (n = 650 || (n > 659 && n < 691) || n = 700))
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 651)
	; 11 bytes
	php
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 654 || n = 689 || n = 696 || n = 703)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 658)
	; 10 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
	plp
.elseif(n = 666)
	; 11 bytes
	php
	sec
	lda #(n-18)/8
	bit $24
	sbc #1
	Jpl *-4
	plp
.elseif(n = 676)
	; 11 bytes
	php
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 695)
	; 8 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	plp
.elseif(n = 701)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 704 to 781
.if(n = 704 || n = 709 || n = 711 || n = 714 || n = 729 || n = 734 || n = 739 || n = 744 || n = 749 || n = 751 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 704 && n < 781)))
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 706)
	; 11 bytes
	php
	sec
	lda #(n-18)/8
	bit $24
	sbc #1
	Jpl *-4
	plp
.elseif((n .mod 5 = 2) && ((n > 706 && n < 778)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 708 || n = 713 || n = 723 || n = 728 || n = 741 || n = 743 || n = 748 || n = 753 || n = 758 || n = 763 || n = 778)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 716)
	; 11 bytes
	php
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 718)
	; 10 bytes
	php
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	delay_n 7
	Jne *-4
	plp
.elseif(n = 719 || n = 726 || n = 754 || n = 761 || n = 768)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 721)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 724)
	; 9 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	tay
	plp
.elseif(n = 731)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 733)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 736)
	; 9 bytes
	php
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jpl *-3
	plp
.elseif(n = 738)
	; 9 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
	plp
.elseif(n = 746)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 756)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 759)
	; 9 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
	plp
.elseif(n = 766 || n = 773)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.endif
.endif
.else
.if((n >= 782) && (n < 860))
; 782 to 859
.if((n .mod 5 = 2) && (n = 782 || n = 787 || (n > 801 && n < 858)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 783 || n = 791 || n = 796 || n = 803 || n = 813 || n = 823 || n = 828 || n = 833 || n = 848 || n = 853 || n = 858)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 784 || n = 790 || n = 792 || n = 795 || n = 797 || n = 798 || n = 801 || n = 804 || n = 809 || n = 814 || n = 819 || n = 821 || n = 834 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 785 || (n > 804 && n < 856)))
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 786)
	; 11 bytes
	php
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 788)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 789 || n = 831 || n = 859)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 793)
	; 7 bytes
	php
	lda #3
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 794 || n = 799 || n = 806)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 800)
	; 9 bytes
	php
	pha
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 808 || n = 843)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 811)
	; 10 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	tay
	plp
.elseif(n = 816)
	; 11 bytes
	php
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 818)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 824 || n = 838)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 826)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 829 || n = 836)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 856)
	; 11 bytes
	php
	sec
	lda #(n-19)/9
	delay_n 4
	sbc #1
	Jpl *-4
	plp
.endif
.else
; 860 to 937
.if((n .mod 5 = 0) && ((n > 859 && n < 936)))
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 861 || n = 896 || n = 931)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 2) && ((n > 861 && n < 938)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && (n = 863 || n = 868 || (n > 882 && n < 904) || n = 918 || n = 923 || n = 933))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 864)
	; 9 bytes
	php
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 866 || n = 873 || n = 901 || n = 908)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 869)
	; 9 bytes
	php
	sec
	lda #250 ;hides 'nop'
	sbc #1
	Jmi *-3
	plp
.elseif(n = 871)
	; 9 bytes
	php
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 874 || n = 879 || n = 881 || n = 884 || n = 889 || n = 891 || n = 904 || n = 909 || n = 911 || n = 914 || n = 919 || n = 921 || n = 924)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 876)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 878 || n = 899 || n = 913)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 886)
	; 11 bytes
	php
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
	plp
.elseif(n = 894 || n = 929 || n = 936)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 906 || n = 934)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 916)
	; 11 bytes
	php
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 926)
	; 10 bytes
	php
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jpl *-4
	plp
.elseif(n = 928)
	; 10 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 951 || n = 953 || n = 958 || n = 963 || n = 968 || n = 988 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1023 || n = 1028 || n = 1033 || n = 1038 || n = 1043 || n = 1047 || n = 1052 || n = 1059 || n = 1068 || n = 1073 || n = 1078 || n = 1093)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 939 || n = 944 || n = 949 || n = 954 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 989 || n = 994 || n = 1009 || n = 1014 || n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1029 || n = 1046 || n = 1048 || n = 1051 || n = 1053 || n = 1054 || n = 1058 || n = 1064 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1021) || (n > 1029 && n < 1046) || (n > 1059 && n < 1091)))
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 941 || n = 976 || n = 983 || n = 1004 || n = 1011 || n = 1018 || n = 1039 || n = 1074 || n = 1088)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1043) || (n > 1056 && n < 1093)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 7 = 5) && (n = 943 || (n > 963 && n < 979) || (n > 998 && n < 1014) || n = 1034 || n = 1041 || (n > 1068 && n < 1084)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 946)
	; 9 bytes
	php
	clc
	lda #122 ;hides 'nop'
	adc #1
	Jne *-3
	plp
.elseif(n = 948)
	; 9 bytes
	php
	tya
	ldy #122 ;hides 'nop'
	iny
	Jne *-2
	tay
	plp
.elseif(n = 956 || n = 996 || n = 1036)
	; 11 bytes
	php
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 966 || n = 1001 || n = 1071)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 969)
	; 9 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
	tay
	plp
.elseif(n = 973)
	; 10 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jmi *-3
	tay
	plp
.elseif(n = 986 || n = 1026)
	; 11 bytes
	php
	sec
	lda #(n-18)/8
	bit $24
	sbc #1
	Jpl *-4
	plp
.elseif(n = 991)
	; 10 bytes
	php
	tya
	ldy #218 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1025)
	; 8 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	plp
.elseif(n = 1031 || n = 1050 || n = 1055)
	; 9..10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1044)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 1049)
	; 7 bytes
	php
	lda #4
	jsr delay_256a_16_clocks_b
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
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1063)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1066)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 1081)
	; 10 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	tay
	plp
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1096 || n = 1099 || n = 1101 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1149 || n = 1154 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1184 || n = 1189 || n = 1191 || n = 1194 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1219 || n = 1224 || n = 1229 || n = 1231 || n = 1234 || n = 1236 || n = 1239 || n = 1241)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1251)))
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif((n .mod 5 = 2) && ((n > 1096 && n < 1248)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && (n = 1098 || n = 1103 || n = 1113 || (n > 1127 && n < 1149) || (n > 1162 && n < 1184) || (n > 1197 && n < 1219) || n = 1233 || n = 1238 || n = 1248))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 1103 && n < 1119) || (n > 1138 && n < 1154) || (n > 1173 && n < 1189) || (n > 1208 && n < 1224) || n = 1244))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1106)
	; 10 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
	tay
	plp
.elseif(n = 1108)
	; 10 bytes
	php
	tya
	ldy #(n-10)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 7 = 3) && ((n > 1108 && n < 1124) || (n > 1143 && n < 1159) || (n > 1178 && n < 1194) || (n > 1213 && n < 1229) || n = 1249))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1126)
	; 10 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1141 || n = 1211)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1156 || n = 1196)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1171)
	; 11 bytes
	php
	sec
	lda #(n-19)/9
	delay_n 4
	sbc #1
	Jpl *-4
	plp
.elseif(n = 1176)
	; 11 bytes
	php
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 1226)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1243)
	; 10 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
	tay
	plp
.elseif(n = 1246)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.if(n = 1251 || n = 1258 || n = 1279 || n = 1286 || n = 1293 || n = 1321 || n = 1328)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif((n .mod 5 = 2) && ((n > 1251 && n < 1288)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 1253 || n = 1273 || n = 1278 || n = 1283 || n = 1288 || n = 1295 || n = 1299 || n = 1301 || n = 1303 || n = 1308 || n = 1315 || n = 1327)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1254 || n = 1259 || n = 1264 || n = 1269 || n = 1271 || n = 1274 || n = 1289 || n = 1294 || n = 1302 || n = 1304 || n = 1307 || n = 1309 || n = 1310 || n = 1313 || n = 1314 || n = 1316 || n = 1323)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1286)))
	; 9 bytes
	php
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	plp
.elseif(n = 1256 || n = 1263 || n = 1284 || n = 1291 || n = 1326)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1261)
	; 10 bytes
	php
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1266)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1268)
	; 10 bytes
	php
	clc
	lda #116 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 1276 || n = 1324)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1281)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1290)
	; 9 bytes
	php
	sec
	lda #0
	sbc #1
	Jne *-2
	plp
.elseif(n = 1292)
	; 9 bytes
	php
	tya
	ldy #0
	dey
	Jne *-1
	tay
	plp
.elseif(n = 1296)
	; 9 bytes
	php
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
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
.elseif(n = 1298)
	; 9 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1300)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 1305)
	; 7 bytes
	php
	lda #5
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1306 || n = 1311 || n = 1318)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1312)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1317)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1319)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1320)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 1322)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 1325)
	; 12 bytes
	delay_n 20
	delay_n (n-20)
.endif
.else
; 1329 to 1406
.if(n = 1329)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1330 || n = 1338 || n = 1346 || n = 1378 || n = 1386 || n = 1402)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1331 || n = 1336 || n = 1343 || n = 1345 || n = 1357 || n = 1359 || n = 1373 || n = 1397 || n = 1399 || n = 1406)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1332 || n = 1348 || n = 1364 || n = 1372 || n = 1388 || n = 1404)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1390) || n = 1403))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1334 || n = 1355 || n = 1367 || n = 1376 || n = 1381 || n = 1383 || n = 1395)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1406)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1337 || n = 1358 || n = 1365 || n = 1379 || n = 1393 || n = 1400)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1339)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 4
	sbc #1
	Jmi *-5
	plp
.elseif(n = 1341)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	tay
	plp
.elseif((n .mod 9 = 3) && ((n > 1343 && n < 1381)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1350)
	; 11 bytes
	php
	tya
	ldy #122 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1351 || n = 1360 || n = 1369)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1352)
	; 11 bytes
	php
	tya
	ldy #(n-10)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1366)
	; 11 bytes
	php
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
	plp
.elseif(n = 1374)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1385)
	; 11 bytes
	php
	tya
	ldy #250 ;hides 'nop'
	nop
	dey
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1387)
	; 10 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	tax
	plp
.elseif(n = 1390)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1392)
	; 11 bytes
	php
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 1394)
	; 9 bytes
	php
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
	plp
.elseif(n = 1396)
	; 9 bytes
	php
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	tay
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
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1416 || n = 1425 || n = 1443 || n = 1470 || n = 1479)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1408 || n = 1413 || n = 1415 || n = 1422 || n = 1427 || n = 1429 || n = 1434 || n = 1448 || n = 1455 || n = 1457 || n = 1469 || n = 1471 || n = 1478 || n = 1483)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1409 || n = 1411 || n = 1430 || n = 1439 || n = 1446 || n = 1453 || n = 1465 || n = 1467 || n = 1472 || n = 1481)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1410)
	; 9 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
	plp
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1483)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442) || n = 1477))
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 7 = 3) && (n = 1417 || n = 1424 || (n > 1437 && n < 1481)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1418 || n = 1442 || n = 1450 || n = 1458 || n = 1474)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 8 = 4) && ((n > 1419 && n < 1445) || n = 1460 || n = 1476 || n = 1484))
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1421 || n = 1435 || n = 1449 || n = 1456 || n = 1463)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1431)
	; 9 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
	plp
.elseif(n = 1437)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1451)
	; 11 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1462)
	; 11 bytes
	php
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
	plp
.elseif(n = 1464)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if(n = 1485 || n = 1499 || n = 1511 || n = 1518)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1486 || n = 1495)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1516)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1488 || n = 1497 || n = 1506)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1518)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1490 || n = 1498 || n = 1514)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1491 || n = 1505 || n = 1509 || n = 1519 || n = 1521)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1492 || n = 1500 || n = 1516)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1493)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1502)
	; 10 bytes
	php
	clc
	lda #90 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 1504)
	; 10 bytes
	php
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1507)
	; 10 bytes
	php
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1512)
	; 11 bytes
	php
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	plp
.elseif(n = 1513)
	; 10 bytes
	php
	clc
	lda #68 ;hides 'nop zp'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 1520)
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1522)
	; 9 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1523)
	; 10 bytes
	php
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	plp
.endif
.else
; 1524 to 1562
.if((n .mod 7 = 5) && ((n > 1523 && n < 1553)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1525 || n = 1537 || n = 1539 || n = 1553 || n = 1555 || n = 1559)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1526 || n = 1544 || n = 1547 || n = 1558 || n = 1560)
	; 8..12 bytes
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
.elseif(n = 1528 || n = 1562)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1529 || n = 1543 || n = 1550 || n = 1557)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1530 || n = 1546 || n = 1554)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1532 || n = 1540 || n = 1548)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1533 || n = 1542 || n = 1551)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1534)
	; 9 bytes
	php
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1535)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1536)
	; 9 bytes
	php
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
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
.elseif(n = 1549)
	; 10 bytes
	php
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1556)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 1561)
	; 7 bytes
	php
	lda #6
	jsr delay_256a_16_clocks_b
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
.if(n = 1563 || n = 1565 || n = 1566 || n = 1569 || n = 1570 || n = 1572 || n = 1577 || n = 1581 || n = 1589 || n = 1597)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1564 || n = 1571 || n = 1583 || n = 1590 || n = 1598)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1567 || n = 1574)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1568)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1573)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1575)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1576)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif((n .mod 7 = 3) && ((n > 1577 && n < 1600)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1579)
	; 10 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 7 = 5) && ((n > 1579 && n < 1602)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1582)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1584)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1586)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1588)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1591 || n = 1593)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1595)
	; 10 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1596)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1600)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 1602 to 1640
.if(n = 1602 || n = 1610 || n = 1626)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1603 || n = 1630)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1604 || n = 1612 || n = 1628)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1605 || n = 1614)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1606 || n = 1613 || n = 1627)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1607 || n = 1619 || n = 1633)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n .mod 7 = 5) && ((n > 1607 && n < 1637)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1609 || n = 1611 || n = 1616 || n = 1623 || n = 1635 || n = 1637)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 0) && ((n > 1616 && n < 1639)))
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1618)
	; 9 bytes
	php
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
	plp
.elseif(n = 1620)
	; 9 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
	tay
	plp
.elseif(n = 1621)
	; 10 bytes
	php
	txa
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	tax
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
.elseif(n = 1632)
	; 9 bytes
	php
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
	plp
.elseif(n = 1634)
	; 9 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
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
.elseif(n = 1640)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 1641 to 1718
.if((n .mod 7 = 3) && (n = 1641 || (n > 1654 && n < 1719)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1642 || n = 1658 || n = 1674 || n = 1682 || n = 1698 || n = 1714)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1714)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1644 || n = 1652 || n = 1660 || n = 1684 || n = 1700 || n = 1708 || n = 1716)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1645 || n = 1673 || n = 1687 || n = 1694 || n = 1701 || n = 1715)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646)
	; 9 bytes
	php
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1647 || n = 1661 || n = 1663 || n = 1689 || n = 1691 || n = 1696 || n = 1703 || n = 1705 || n = 1710 || n = 1712 || n = 1717)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1648)
	; 9 bytes
	php
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1649 || n = 1651 || n = 1665 || n = 1667 || n = 1672 || n = 1679 || n = 1681 || n = 1688 || n = 1707 || n = 1709)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
	; 10 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
	plp
.elseif(n = 1656 || n = 1680)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 9 = 3) && ((n > 1658 && n < 1696)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1666)
	; 10 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1670)
	; 11 bytes
	php
	tya
	ldy #90 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1675 || n = 1702)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1693)
	; 11 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	tax
	plp
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1733 || n = 1745 || n = 1759)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1791)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1721 || n = 1723 || n = 1735 || n = 1737 || n = 1742 || n = 1747 || n = 1749 || n = 1761 || n = 1766 || n = 1768 || n = 1777 || n = 1782 || n = 1784 || n = 1789 || n = 1791 || n = 1793)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1722 || n = 1731 || n = 1740 || n = 1785 || n = 1794)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 8 = 4) && (n = 1724 || (n > 1755 && n < 1781) || n = 1796))
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 7 = 3) && ((n > 1724 && n < 1740) || n = 1753 || (n > 1766 && n < 1796)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1726 || n = 1750)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1728 || n = 1752)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1729 || n = 1765)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1730 || n = 1738 || n = 1754 || n = 1770 || n = 1778 || n = 1786)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1736 || n = 1743 || n = 1757 || n = 1773 || n = 1775)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1744)
	; 9 bytes
	php
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1746)
	; 9 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
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
.elseif(n = 1758)
	; 9 bytes
	php
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1760)
	; 9 bytes
	php
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1763)
	; 10 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1771)
	; 10 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1779)
	; 10 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1787)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1792)
	; 10 bytes
	php
	tya
	ldy #58 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
	plp
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1798)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1799 || n = 1806 || n = 1814 || n = 1816 || n = 1819 || n = 1821 || n = 1822 || n = 1825 || n = 1826 || n = 1828 || n = 1833)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1801)
	; 10 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1802)
	; 10 bytes
	php
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1803)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1804)
	; 10 bytes
	php
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1805 || n = 1807 || n = 1813 || n = 1815 || n = 1820 || n = 1827)
	; 8..11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1810)
	; 10 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
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
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 1817)
	; 7 bytes
	php
	lda #7
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1818 || n = 1823 || n = 1830)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1824)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1829)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1831)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1832)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 1834)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 1835)
	; 10 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	dey
	Jne *-3
	tay
	plp
.endif
.else
; 1836 to 1875
.if(n = 1836 || n = 1852 || n = 1860 || n = 1868)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1837)
	; 10 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
	plp
.elseif(n = 1838 || n = 1840 || n = 1847 || n = 1849)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 3) && ((n > 1838 && n < 1876)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1841)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1842 || n = 1858 || n = 1874)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1843)
	; 10 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1844)
	; 10 bytes
	php
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 1845)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1846)
	; 10 bytes
	php
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1850)
	; 11 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	bit $24
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1851)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 1853 || n = 1854 || n = 1859 || (n > 1860 && n < 1864) || n = 1865 || n = 1867 || n = 1869 || n = 1871)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1855)
	; 10 bytes
	php
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1856)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1864 || n = 1873)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1870)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1872)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
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
.if((n .mod 8 = 4) && (n = 1876 || (n > 1891 && n < 1909) || (n > 1923 && n < 1949)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif((n > 1876 && n < 1881) || n = 1883 || n = 1885 || n = 1887 || n = 1895 || n = 1897 || (n > 1902 && n < 1906) || n = 1907 || n = 1910 || n = 1915 || n = 1917 || n = 1923 || n = 1925 || n = 1926 || n = 1931 || (n > 1932 && n < 1936) || n = 1937 || n = 1941 || n = 1943 || (n > 1948 && n < 1952) || n = 1953)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1881)
	; 11 bytes
	php
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	plp
.elseif((n .mod 8 = 2) && ((n > 1881 && n < 1931) || n = 1946))
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 9 = 3) && ((n > 1883 && n < 1948)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1886)
	; 11 bytes
	php
	tya
	ldy #122 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1888)
	; 11 bytes
	php
	clc
	lda #68 ;hides 'nop zp'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	plp
.elseif(n = 1889)
	; 11 bytes
	php
	clc
	lda #68 ;hides 'nop zp'
	delay_n 4
	adc #1
	Jne *-5
	plp
.elseif(n = 1891 || n = 1945)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1894 || n = 1942)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1896 || n = 1944)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1899)
	; 11 bytes
	php
	clc
	lda #84 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	plp
.elseif(n = 1901)
	; 11 bytes
	php
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1909)
	; 10 bytes
	php
	tya
	ldy #(n-1)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1912 || n = 1919 || n = 1921 || n = 1939)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1913)
	; 11 bytes
	php
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1916)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 1918)
	; 10 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1927)
	; 10 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1928)
	; 11 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	tay
	plp
.elseif(n = 1936)
	; 10 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1952)
	; 10 bytes
	php
	sec
	lda #216 ;hides 'cld'
	nop
	sbc #1
	Jne *-4
	plp
.endif
.else
; 1954 to 2031
.if(n = 1954)
	; 10 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1955 || n = 1957 || n = 1973 || n = 1975 || n = 2031)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 3) && ((n > 1955 && n < 2029)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif((n > 1957 && n < 1961) || n = 1967 || n = 1969 || n = 1976 || n = 1977 || n = 1982 || n = 1984 || n = 1985 || n = 1987 || n = 1989 || n = 1993 || n = 1995 || n = 1997 || n = 1998 || n = 2000 || n = 2003 || (n > 2004 && n < 2008) || n = 2009 || n = 2011 || n = 2013 || n = 2015)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 1961)
	; 11 bytes
	php
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.elseif((n .mod 8 = 2) && (n = 1962 || (n > 1977 && n < 2003) || n = 2018 || n = 2026))
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 9 = 1) && (n = 1963 || (n > 1989 && n < 2018)))
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 8 = 4) && (n = 1964 || (n > 1979 && n < 2021)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1966 || n = 2014)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1968 || n = 2016)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1970)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 1971)
	; 11 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1972)
	; 10 bytes
	php
	tya
	ldy #(n-10)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1979)
	; 11 bytes
	php
	txa
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
	tax
	plp
.elseif(n = 1981)
	; 11 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1991)
	; 11 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2021 || n = 2029)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2022)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2023)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	plp
.elseif(n = 2024)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 2025)
	; 12 bytes
	php
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2027)
	; 11 bytes
	php
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2030)
	; 12 bytes
	php
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032)
	; 11 bytes
	php
	sec
	lda #184 ;hides 'clv'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2033)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 2034)
	; 11 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2035 || n = 2053 || n = 2062)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2036 || n = 2044 || n = 2052)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 9 = 3) && ((n > 2036 && n < 2065)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2038)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2039 || n = 2048 || n = 2057 || n = 2066 || n = 2070)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2040)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2041)
	; 12 bytes
	php
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
	plp
.elseif(n = 2042 || n = 2050)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 2043)
	; 11 bytes
	php
	sec
	lda #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
	plp
.elseif(n = 2045)
	; 11 bytes
	php
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2047 || n = 2069)
	; 12 bytes
	php
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2049 || n = 2061 || n = 2063)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2051 || n = 2059)
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
.elseif(n = 2054)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2056 || n = 2067)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	plp
.elseif(n = 2058)
	; 11 bytes
	php
	sec
	lda #0
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 2060)
	; 11 bytes
	php
	tya
	ldy #0
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2065)
	; 12 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	php
	cmp $C5
	plp
	Jne *-5
	tay
	plp
.elseif(n = 2068)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2076 || n = 2083 || n = 2099 || n = 2101)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2072 || n = 2075 || n = 2077 || n = 2078 || n = 2081 || n = 2082 || n = 2084)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2073)
	; 7 bytes
	php
	lda #8
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2074 || n = 2079 || n = 2086 || n = 2095)
	; 9..12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2080)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2085)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2087)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2088)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 2089)
	; 10 bytes
	php
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	tax
	plp
.elseif(n = 2090)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 2091 || n = 2100 || n = 2109)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2092)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 2093)
	; 12 bytes
	delay_n 20
	delay_n (n-20)
.elseif(n = 2094)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2096)
	; 10 bytes
	php
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2097)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2098)
	; 10 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	tay
	plp
.elseif((n > 2101 && n < 2107) || n = 2108)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2107)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.endif
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2122 || n = 2146 || n = 2158 || n = 2182)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2111 || n = 2113 || n = 2115 || n = 2120 || n = 2121 || n = 2123 || n = 2126 || (n > 2128 && n < 2132) || n = 2133 || (n > 2137 && n < 2143) || n = 2144 || n = 2147 || (n > 2148 && n < 2152) || n = 2153 || (n > 2154 && n < 2158) || n = 2159 || n = 2162 || (n > 2163 && n < 2168) || n = 2169 || n = 2171 || (n > 2172 && n < 2179) || n = 2180 || n = 2183 || n = 2185 || n = 2187)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2112 || n = 2124 || n = 2148 || n = 2160 || n = 2184)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2114)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2116)
	; 10 bytes
	php
	tya
	ldy #(n-10)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2117 || n = 2119 || n = 2128 || n = 2135 || n = 2137)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 3) && ((n > 2117 && n < 2182)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2125)
	; 10 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 2132)
	; 10 bytes
	php
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2134)
	; 10 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
	plp
.elseif(n = 2143)
	; 10 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif((n .mod 9 = 1) && ((n > 2151 && n < 2180)))
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2168)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2186)
	; 11 bytes
	php
	clc
	lda #58 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
	plp
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
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2189 || (n > 2190 && n < 2194) || n = 2195 || n = 2198 || n = 2202 || n = 2203 || (n > 2211 && n < 2215) || n = 2216 || n = 2219 || n = 2222 || n = 2223 || n = 2225)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n .mod 9 = 3) && ((n > 2189 && n < 2227)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2194 || n = 2218)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2196 || n = 2220)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2197)
	; 10 bytes
	php
	tya
	ldy #(n-1)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2200)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2201)
	; 11 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2204)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2205)
	; 11 bytes
	php
	clc
	lda #12 ;hides 'nop abs'
	nop $0
	adc #1
	Jne *-5
	plp
.elseif(n = 2206)
	; 10 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2207)
	; 11 bytes
	php
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2209)
	; 11 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	delay_n 4
	adc #1
	Jne *-5
	plp
.elseif(n = 2210)
	; 11 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2211)
	; 11 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2215)
	; 10 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2221)
	; 11 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	plp
.endif
.else
; 2227 to 2265
.if((n > 2226 && n < 2230) || n = 2231 || n = 2234 || (n > 2235 && n < 2240) || n = 2241 || (n > 2245 && n < 2251) || n = 2255 || n = 2257 || n = 2259 || n = 2264)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2230 || n = 2254)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2232 || n = 2256)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2233)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 9 = 3) && ((n > 2234 && n < 2263)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2240)
	; 10 bytes
	php
	sec
	lda #248 ;hides 'sed'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2242)
	; 10 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2243)
	; 11 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	nop
	Jne *-4
	tax
	plp
.elseif(n = 2245 || n = 2263)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2251)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2252)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2258)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2260)
	; 10 bytes
	php
	tya
	ldy #(n-10)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2261)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2265)
	; 11 bytes
	php
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	plp
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
.if((n >= 2266) && (n < 2285))
; 2266 to 2284
.if(n = 2266)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2267 || n = 2272 || n = 2273)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2268)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2269)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2270)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2271 || n = 2280)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2274 || n = 2284)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2275)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2276)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
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
.elseif(n = 2278)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2279)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2281)
	; 11 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2282)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2283)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2287)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2288)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2289 || n = 2298)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2290)
	; 11 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2291)
	; 11 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2292 || n = 2304)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2293 || n = 2299 || n = 2303)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2294)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2295)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2296)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2297)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2300)
	; 12 bytes
	php
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2301)
	; 11 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2302)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.endif
.endif
.else
; 2305 to 2343
.if(n = 2305)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2306)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2307)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2308)
	; 11 bytes
	php
	clc
	lda #26 ;hides 'nop'
	nop $04
	adc #1
	Jne *-5
	plp
.elseif(n = 2309)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	plp
.elseif(n = 2310)
	; 11 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2311 || n = 2322)
	; 12 bytes
	php
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2312)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2313 || n = 2319 || n = 2325 || n = 2327 || n = 2332 || n = 2339)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2314)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2315)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2316)
	; 11 bytes
	php
	tya
	ldy #0
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2317 || n = 2318 || n = 2326 || n = 2328 || n = 2331 || n = 2333 || n = 2334 || n = 2337 || n = 2338 || n = 2340)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2320)
	; 12 bytes
	php
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
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
.elseif(n = 2324)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 2329)
	; 7 bytes
	php
	lda #9
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2330 || n = 2335 || n = 2342)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2336)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2341)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2343)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.else
; 2344 to 2500
.if(n = 2344)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 2345)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2346)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 2347)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2348)
	; 11 bytes
	php
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2349)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2350)
	; 11 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	bit $24
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2351)
	; 12 bytes
	php
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	plp
.elseif((n .mod 12 = 0) && ((n > 2351 && n < 2497)))
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2353)
	; 11 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif((n > 2353 && n < 2358))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 2357 && n < 2362) || (n > 2364 && n < 2368) || (n > 2368 && n < 2374) || n = 2375 || (n > 2376 && n < 2384) || n = 2385 || n = 2387 || (n > 2388 && n < 2398) || n = 2399 || (n > 2400 && n < 2406) || n = 2407 || n = 2409 || n = 2411 || (n > 2412 && n < 2422) || n = 2423 || (n > 2424 && n < 2434) || n = 2435 || (n > 2436 && n < 2446) || n = 2447 || (n > 2449 && n < 2458) || n = 2459 || (n > 2460 && n < 2470) || n = 2471 || (n > 2472 && n < 2482) || n = 2483 || (n > 2484 && n < 2488) || n = 2489 || (n > 2490 && n < 2494) || n = 2495 || (n > 2496 && n < 2501))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n .mod 12 = 10) && ((n > 2361 && n < 2435) || (n > 2457 && n < 2495)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2363)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 2368)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2384)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2406)
	; 11 bytes
	php
	sec
	lda #218 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2408)
	; 11 bytes
	php
	tya
	ldy #(n-10)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2446)
	; 10 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	plp
.elseif(n = 2449)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2488)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2490)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	bit $24
	dey
	Jne *-4
	tay
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
.if((n > 2500 && n < 2506) || n = 2507 || n = 2509 || (n > 2510 && n < 2518) || n = 2519 || (n > 2520 && n < 2530))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2506 || n = 2518 || n = 2530)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2508 || n = 2520 || n = 2532)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2510)
	; 11 bytes
	php
	tya
	ldy #250 ;hides 'nop'
	nop $04
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2531)
	; 12 bytes
	php
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2533 || n = 2535)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2534 || n = 2536)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2537)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 2538)
	; 11 bytes
	php
	clc
	lda #26 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
	plp
.elseif(n = 2539)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.endif
.else
; 2540 to 2578
.if(n = 2540)
	; 11 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2541 || n = 2547 || n = 2557 || n = 2559 || n = 2563 || n = 2565 || n = 2569 || n = 2571)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2542 || n = 2554 || n = 2566 || n = 2578)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2543 || n = 2577)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 2544 || n = 2556 || n = 2568)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2545)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2546 || n = 2574)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2548)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2549)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	sbc #1
	Jmi *-6
	plp
.elseif(n = 2550)
	; 12 bytes
	php
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2551)
	; 11 bytes
	php
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	tax
	plp
.elseif(n = 2552)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2553 || n = 2564 || n = 2575)
	; 12 bytes
	php
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2555 || n = 2576)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2558)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2560)
	; 11 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
	plp
.elseif(n = 2561)
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
	; 11 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	tay
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
.elseif(n = 2570)
	; 12 bytes
	php
	sec
	lda #0
	nop
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2572)
	; 12 bytes
	php
	tya
	ldy #0
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2573)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	plp
.endif
.endif
.else
; 2579 to 2656
.if(n = 2579)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2580)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 2581 || n = 2583 || n = 2588 || n = 2595)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2582 || n = 2584 || n = 2587 || n = 2589 || n = 2590 || n = 2593 || n = 2594 || n = 2596 || n = 2601 || n = 2607)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2585)
	; 7 bytes
	php
	lda #10
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2586 || n = 2591 || n = 2598)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2592)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2597)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2599)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2600)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 2602)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 2603)
	; 11 bytes
	php
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	plp
.elseif((n .mod 12 = 0) && ((n > 2603 && n < 2653)))
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2605)
	; 11 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2606)
	; 11 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2608)
	; 12 bytes
	php
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2609)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 2609 && n < 2613))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 2613)
	; 11 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	Jne *-4
	tay
	plp
.elseif(n = 2614 || n = 2626 || n = 2638 || n = 2650)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2615 || n = 2618 || (n > 2619 && n < 2624) || n = 2625 || n = 2627 || (n > 2628 && n < 2638) || n = 2639 || (n > 2640 && n < 2650) || n = 2651 || (n > 2652 && n < 2657))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2617)
	; 11 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2619)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 2624)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if((n > 2656 && n < 2662) || n = 2663 || (n > 2664 && n < 2674) || n = 2675 || (n > 2676 && n < 2683) || n = 2684 || n = 2685 || n = 2687 || n = 2689 || n = 2690 || (n > 2694 && n < 2698) || n = 2699 || (n > 2700 && n < 2705) || (n > 2705 && n < 2710) || n = 2711 || (n > 2712 && n < 2722) || n = 2723 || (n > 2724 && n < 2734))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n .mod 12 = 10) && ((n > 2661 && n < 2735)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 12 = 0) && ((n > 2663 && n < 2725)))
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2683)
	; 11 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2691)
	; 11 bytes
	php
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
	plp
.elseif(n = 2692)
	; 11 bytes
	php
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2693)
	; 11 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2694)
	; 11 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2705)
	; 11 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.endif
.else
; 2735 to 2812
.if(n = 2735 || n = 2737 || (n > 2738 && n < 2746) || n = 2747 || (n > 2748 && n < 2758) || n = 2759 || (n > 2760 && n < 2770) || n = 2771 || (n > 2772 && n < 2780) || n = 2781 || n = 2783 || n = 2785)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n .mod 12 = 0) && ((n > 2735 && n < 2809)))
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2738)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif((n .mod 12 = 10) && ((n > 2745 && n < 2807)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2780)
	; 11 bytes
	php
	clc
	lda #58 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	plp
.elseif(n = 2786)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2787 || n = 2788 || n = 2797 || n = 2799 || n = 2809 || n = 2811)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2790)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2791)
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
.elseif(n = 2792)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2793)
	; 12 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2795)
	; 12 bytes
	php
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2798 || n = 2812)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2800)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2802)
	; 13 bytes
	php
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2803)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 2804)
	; 12 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2805)
	; 12 bytes
	php
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2807)
	; 13 bytes
	php
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2810)
	; 11 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2821 || n = 2823 || n = 2833 || n = 2835 || n = 2839 || n = 2844 || n = 2851)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2814 || n = 2819 || n = 2822 || n = 2834 || n = 2838 || n = 2840 || n = 2843 || n = 2845 || n = 2846 || n = 2849 || n = 2850)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2815)
	; 12 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2816)
	; 12 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2817)
	; 12 bytes
	php
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2818 || n = 2830)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2820 || n = 2832)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2824)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
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
.elseif(n = 2826)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2827)
	; 12 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	nop
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2828)
	; 12 bytes
	php
	tya
	ldy #0
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2829)
	; 12 bytes
	php
	clc
	lda #68 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
	plp
.elseif(n = 2831)
	; 12 bytes
	php
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2836)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 2841)
	; 7 bytes
	php
	lda #11
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2842 || n = 2847)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2848)
	; 9 bytes
	php
	pha
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2857)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2853)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2854)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2855)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2856)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 2858)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 2859)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2860)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 2861)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2862)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2863)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2864)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2865)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2866 || n = 2878 || n = 2890)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2867 || n = 2869)
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 2868 || n = 2880)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif((n > 2869 && n < 2875) || n = 2876 || n = 2877 || n = 2879 || (n > 2880 && n < 2890))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 2875)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.else
; 2891 to 2968
.if(n = 2891 || (n > 2892 && n < 2902) || n = 2903 || (n > 2904 && n < 2914) || n = 2915 || (n > 2916 && n < 2926) || n = 2927 || (n > 2928 && n < 2938) || n = 2939 || (n > 2940 && n < 2950) || n = 2951 || (n > 2952 && n < 2962) || n = 2963 || (n > 2964 && n < 2969))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n .mod 12 = 0) && ((n > 2891 && n < 2965)))
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 12 = 10) && ((n > 2901 && n < 2963)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n > 2968 && n < 2974) || n = 2975 || (n > 2976 && n < 2986) || n = 2987 || (n > 2988 && n < 2998) || n = 2999 || (n > 3000 && n < 3010) || n = 3011 || (n > 3012 && n < 3022) || n = 3023 || (n > 3024 && n < 3032) || n = 3033 || n = 3035 || (n > 3036 && n < 3042))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n .mod 12 = 10) && ((n > 2973 && n < 3047)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 12 = 0) && ((n > 2975 && n < 3037)))
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3032)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3042 || n = 3043)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3044)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3045)
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
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	bit $24
	iny
	Jne *-6
	tay
	plp
.elseif(n = 3048 || n = 3060 || n = 3072)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3049 || n = 3051 || n = 3056 || n = 3061 || n = 3063 || n = 3073 || n = 3081 || n = 3085)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3050 || n = 3064 || n = 3078)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3052)
	; 12 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3053)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3054 || n = 3080)
	; 13 bytes
	php
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3055)
	; 12 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	php
	cmp $C5
	plp
	Jne *-5
	tax
	plp
.elseif(n = 3057)
	; 13 bytes
	php
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3058 || n = 3070)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 3059)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	bit $24
	iny
	Jne *-6
	tay
	plp
.elseif(n = 3062)
	; 11 bytes
	php
	tya
	ldy #(n-10)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3065)
	; 12 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3066)
	; 13 bytes
	php
	sec
	lda #(n-10)/16
	delay_n 11
	sbc #1
	Jne *-6
	plp
.elseif(n = 3067 || n = 3079)
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
.elseif(n = 3068)
	; 13 bytes
	php
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3069)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3071)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3074)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3075)
	; 12 bytes
	php
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	plp
.elseif(n = 3076)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3077)
	; 12 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3082)
	; 11 bytes
	php
	sec
	lda #0
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 3083)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	bit $24
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3084)
	; 11 bytes
	php
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3094 || n = 3096 || n = 3099 || n = 3101 || n = 3102 || n = 3105 || n = 3106 || n = 3108 || n = 3113)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3087)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3088)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3089)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3090)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3091)
	; 12 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3092)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3093)
	; 13 bytes
	php
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3095 || n = 3100 || n = 3107 || n = 3119)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3097)
	; 7 bytes
	php
	lda #12
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3098 || n = 3103 || n = 3110)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3104)
	; 9 bytes
	php
	pha
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3109)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3111 || n = 3118)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3112)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 3114)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 3115)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3116)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3117)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3120)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3121)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 3121 && n < 3126))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if((n > 3125 && n < 3131) || (n > 3131 && n < 3136) || (n > 3136 && n < 3228) || n = 3229 || (n > 3230 && n < 3256) || n = 3257 || (n > 3258 && n < 3282))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3131)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3136)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3228)
	; 11 bytes
	php
	clc
	lda #26 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	plp
.elseif(n = 3230)
	; 11 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
	plp
.elseif(n = 3256)
	; 11 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
	plp
.elseif(n = 3258)
	; 11 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
	plp
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3283 || n = 3285 || (n > 3286 && n < 3298))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3284)
	; 11 bytes
	php
	sec
	lda #(n-8)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3286)
	; 11 bytes
	php
	tya
	ldy #(n-10)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3298)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3299 || n = 3318)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3300 || n = 3314)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3301)
	; 13 bytes
	php
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3302 || n = 3316)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3303 || n = 3305 || n = 3317 || n = 3319)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3304 || n = 3315)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3306)
	; 13 bytes
	php
	sec
	lda #(n-10)/16
	delay_n 11
	sbc #1
	Jne *-6
	plp
.elseif(n = 3307)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 3308)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3309)
	; 12 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	delay_n 9
	adc #1
	Jne *-6
	plp
.elseif(n = 3310)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3311)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3312)
	; 13 bytes
	php
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3313)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 4
	iny
	Jne *-6
	tay
	plp
.elseif(n = 3320)
	; 13 bytes
	php
	sec
	lda #184 ;hides 'clv'
	delay_n 11
	sbc #1
	Jne *-7
	plp
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3331 || n = 3333 || n = 3345 || n = 3347 || n = 3351 || n = 3356)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3322)
	; 13 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	delay_n 11
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3323)
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
.elseif(n = 3324)
	; 13 bytes
	php
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3325)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3326)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3327)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3328 || n = 3342)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3329 || n = 3332 || n = 3334 || n = 3341 || n = 3346 || n = 3350 || n = 3352 || n = 3355 || n = 3357 || n = 3358)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3330 || n = 3344)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3335)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3336 || n = 3354 || n = 3359)
	; 9..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3337)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3338)
	; 13 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 4
	iny
	Jne *-6
	tay
	plp
.elseif(n = 3339)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3340)
	; 13 bytes
	php
	tya
	ldy #0
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3343)
	; 13 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3348)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3349)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 3353)
	; 7 bytes
	php
	lda #13
	jsr delay_256a_16_clocks_b
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
.elseif(n = 3361 || n = 3362 || n = 3364 || n = 3369)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363 || n = 3375)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3365)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3366)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3367)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3368)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 3370)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 3371)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3372)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3373)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3374)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3376)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3377)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 3377 && n < 3382))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 3381 && n < 3387) || (n > 3387 && n < 3392) || (n > 3392 && n < 3438))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3387)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3392)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if((n > 3437 && n < 3461) || (n > 3461 && n < 3480) || n = 3481 || (n > 3482 && n < 3508) || n = 3509 || (n > 3510 && n < 3516))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3461)
	; 11 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
	plp
.elseif(n = 3480)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3482)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3508)
	; 11 bytes
	php
	sec
	lda #(n-8)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3510)
	; 11 bytes
	php
	tya
	ldy #(n-10)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	plp
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if((n > 3515 && n < 3554))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3554)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.endif
.else
.if((n >= 3555) && (n < 3574))
; 3555 to 3573
.if(n = 3555)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3556 || n = 3557 || n = 3569 || n = 3571)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3558 || n = 3560)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3559)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 3561)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3562)
	; 13 bytes
	php
	sec
	lda #(n-10)/16
	delay_n 11
	sbc #1
	Jne *-6
	plp
.elseif(n = 3563)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3564)
	; 13 bytes
	php
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3565)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3566)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3567)
	; 13 bytes
	php
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3568)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3570)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3572)
	; 13 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
	tay
	plp
.elseif(n = 3573)
	; 14 bytes
	php
	pha
	sec
	lda #254
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
; 3574 to 3593
.if(n = 3574)
	; 13 bytes
	php
	tya
	ldy #58 ;hides 'nop'
	delay_n 11
	iny
	Jne *-6
	tay
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
.elseif(n = 3576)
	; 14 bytes
	php
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3577)
	; 14 bytes
	php
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3578)
	; 13 bytes
	php
	sec
	lda #(n-10)/16
	delay_n 11
	sbc #1
	Jne *-6
	plp
.elseif(n = 3579)
	; 15 bytes
	php
	tya
	ldy #122
	delay_n 24
	dey
	Jpl *-7
	tay
	plp
.elseif(n = 3580)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3581)
	; 14 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	cmp $C5
	iny
	Jne *-7
	tay
	plp
.elseif(n = 3582)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3583 || n = 3585 || n = 3587)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3584)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3586 || n = 3593)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 3588)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3590)
	; 14 bytes
	php
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
	tax
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
.elseif(n = 3592)
	; 13 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	delay_n 9
	dey
	dey
	Jne *-6
	tay
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594)
	; 12 bytes
	php
	sec
	lda #0
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3595)
	; 13 bytes
	php
	sec
	lda #239
	rol $26
	ror $26
	sbc #1
	Jne *-6
	plp
.elseif(n = 3596)
	; 12 bytes
	php
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3597)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3598 || n = 3605 || n = 3606 || n = 3608 || n = 3611 || n = 3613 || n = 3614 || n = 3617 || n = 3618 || n = 3620 || n = 3625)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3599)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3600 || n = 3602 || n = 3607 || n = 3612 || n = 3619 || n = 3631)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3601)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3603)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3604)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3609)
	; 7 bytes
	php
	lda #14
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3610 || n = 3615 || n = 3622)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3616)
	; 9 bytes
	php
	pha
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3621)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3623)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3624)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 3626)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 3627)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3628)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3629)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3630)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3632)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 3633 to 3671
.if(n = 3633)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 3633 && n < 3638))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 3637 && n < 3643) || (n > 3643 && n < 3648) || (n > 3648 && n < 3672))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3643)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3648)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 3672 to 3750
.if((n > 3671 && n < 3751))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
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
.if((n > 3750 && n < 3810))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3810)
	; 12 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3811)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3812)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3813)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3814)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3815 || n = 3821)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3816)
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
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3822)
	; 13 bytes
	php
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3823)
	; 13 bytes
	php
	sec
	lda #212 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-7
	plp
.elseif(n = 3824)
	; 13 bytes
	php
	sec
	lda #212 ;hides 'nop zp,x'
	delay_n 11
	sbc #1
	Jne *-7
	plp
.elseif(n = 3825)
	; 13 bytes
	php
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3826)
	; 13 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
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
	; 13 bytes
	php
	tya
	ldy #90 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	plp
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3848)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3830)
	; 14 bytes
	php
	sec
	lda #(n-10)/20
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 3831 || n = 3850)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3832 || n = 3853)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 3833 || n = 3839 || n = 3841 || n = 3845 || n = 3856 || n = 3858 || n = 3862 || n = 3864 || n = 3867)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3834)
	; 13 bytes
	php
	sec
	lda #(n-10)/16
	delay_n 11
	sbc #1
	Jne *-6
	plp
.elseif(n = 3835 || n = 3852)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3836)
	; 13 bytes
	php
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3837 || n = 3854)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3838 || n = 3840 || n = 3847 || n = 3851 || n = 3857 || n = 3863)
	; 8..14 bytes
	pha
	delay_n (n-3)
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
	; 13 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3844)
	; 13 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 11
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3846)
	; 14 bytes
	php
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3849)
	; 15 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3855)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3860)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3861)
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
.elseif(n = 3865)
	; 7 bytes
	php
	lda #15
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3866)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3875 || n = 3887)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3869 || n = 3870 || n = 3873 || n = 3874 || n = 3876 || n = 3881)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3871 || n = 3878)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3872)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3877)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3879)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3880)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 3882)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 3883)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3884)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3885)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3886)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3888)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3889)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 3889 && n < 3894))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 3893 && n < 3899) || (n > 3899 && n < 3904) || n = 3905 || n = 3906)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 3899)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 3904)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
; 3907 to 4062
.if((n > 3906 && n < 4063))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
; 4063 to 4101
.if((n > 4062 && n < 4066))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4066 || n = 4067)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4068 || n = 4079 || n = 4081 || n = 4093 || n = 4098 || n = 4100)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4069 || n = 4071 || n = 4072)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4070)
	; 13 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	nop $04
	jsr @nearby_rts
	Jne *-6
	tax
	plp
.elseif(n = 4073 || n = 4090)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4074)
	; 13 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 7
	iny
	Jne *-6
	tay
	plp
.elseif(n = 4075 || n = 4092)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4076 || n = 4095)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4077 || n = 4080 || n = 4088 || n = 4094 || n = 4099 || n = 4101)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4078 || n = 4097)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4082)
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
	tya
	pha
	ldy #42 ;hides 'rol a'
	jsr @nearby_rts
	iny
	Jne *-5
	pla
	tay
	plp
.elseif(n = 4084)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
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
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4087)
	; 14 bytes
	php
	clc
	lda #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	adc #1
	Jne *-8
	plp
.elseif(n = 4089)
	; 14 bytes
	php
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
	tay
	plp
.elseif(n = 4091)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
	plp
.elseif(n = 4096)
	; 14 bytes
	php
	sec
	lda #227
	nop
	delay_n 11
	sbc #1
	Jne *-7
	plp
.endif
.else
; 4102 to 4140
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
.elseif(n = 4103 || n = 4122 || n = 4127 || n = 4134)
	; 9..14 bytes
	delay_n 6
	delay_n (n-6)
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
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 4106)
	; 13 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 7
	iny
	Jne *-6
	tay
	plp
.elseif(n = 4107)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4108)
	; 13 bytes
	php
	tya
	ldy #0
	delay_n 11
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4109)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4110 || n = 4115 || n = 4117 || n = 4119 || n = 4124 || n = 4131)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4111 || n = 4113 || n = 4118 || n = 4120 || n = 4123 || n = 4125 || n = 4126 || n = 4129 || n = 4130 || n = 4132 || n = 4137)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4112)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	plp
.elseif(n = 4114)
	; 12 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4116)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4121)
	; 7 bytes
	php
	lda #16
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4128)
	; 9 bytes
	php
	pha
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4133)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4135)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4136)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 4138)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 4139)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4140)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4142)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4143)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4144)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4145)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 4145 && n < 4150))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 4149 && n < 4155) || (n > 4155 && n < 4160) || (n > 4160 && n < 4219))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4155)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4160)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 4219 to 4375
.if((n > 4218 && n < 4322))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4322)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4323 || n = 4342 || n = 4361)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4324 || n = 4326 || n = 4331 || n = 4333 || n = 4339 || n = 4341 || n = 4348 || n = 4350 || n = 4355 || n = 4360 || n = 4375)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 4325 || n = 4344 || n = 4363)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4327 || n = 4329 || n = 4332 || n = 4334 || n = 4340 || n = 4346 || n = 4349 || n = 4351 || n = 4365 || n = 4366 || n = 4373 || n = 4374)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4328 || n = 4345)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4330 || n = 4347)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4335 || n = 4352)
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
.elseif(n = 4336 || n = 4357)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
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
.elseif(n = 4338 || n = 4359)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4343)
	; 14 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	delay_n 11
	dey
	Jne *-7
	tay
	plp
.elseif(n = 4353)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4356)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4358)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #2
	jsr delay_256a_11_clocks_b_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 4362)
	; 12 bytes
	php
	sec
	lda #0
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4364)
	; 12 bytes
	php
	tya
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4367)
	; 13 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4368)
	; 14 bytes
	php
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
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
.elseif(n = 4370)
	; 14 bytes
	php
	sec
	lda #(n-10)/20
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
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
.elseif(n = 4372)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4379 || n = 4381 || n = 4382 || n = 4385 || n = 4386 || n = 4388 || n = 4393)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377)
	; 7 bytes
	php
	lda #17
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4378 || n = 4383 || n = 4390)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4380 || n = 4387 || n = 4399)
	; 8..12 bytes
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
.elseif(n = 4389)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4391)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4392)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 4394)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 4395)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4396)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4397)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4398)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4400)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4401)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 4401 && n < 4406))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 4405 && n < 4411) || (n > 4411 && n < 4416) || (n > 4416 && n < 4532))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4411)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4416)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if((n > 4531 && n < 4578))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4578 || n = 4579)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4580 || n = 4592 || n = 4594)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 4580 && n < 4585))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4586)
	; 12 bytes
	php
	sec
	lda #218 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 4587)
	; 13 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4588)
	; 12 bytes
	php
	tya
	ldy #218 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4589 || n = 4608)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4590)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4591)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4593 || n = 4595 || n = 4603 || n = 4604 || n = 4606)
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 4596)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4597)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
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
	; 15 bytes
	php
	tya
	ldy #139
	delay_n 28
	dey
	Jne *-7
	tay
	plp
.elseif(n = 4600)
	; 14 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
	plp
.elseif(n = 4601)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
	plp
.elseif(n = 4602)
	; 14 bytes
	php
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4605)
	; 15 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
.elseif(n = 4609)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4629)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4611)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4612 || n = 4614 || n = 4621 || n = 4622 || n = 4630 || n = 4632 || n = 4635 || n = 4637 || n = 4638 || n = 4641 || n = 4642 || n = 4644)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4613 || n = 4631 || n = 4636 || n = 4643)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4615)
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
.elseif(n = 4616 || n = 4634 || n = 4639 || n = 4646)
	; 9..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4617)
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
	; 14 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	iny
	Jne *-7
	tay
	plp
.elseif(n = 4619)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
	plp
.elseif(n = 4620)
	; 14 bytes
	php
	tya
	ldy #0
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4623)
	; 15 bytes
	php
	tya
	ldy #159
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 4624 || n = 4647)
	; 10..15 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4628)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4633)
	; 7 bytes
	php
	lda #18
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4640)
	; 9 bytes
	php
	pha
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4645)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4648)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.endif
.else
; 4649 to 4687
.if(n = 4649)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4650)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 4651)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4652)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4653)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4654)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4655)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4656)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4657)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 4657 && n < 4662))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 4661 && n < 4667) || (n > 4667 && n < 4672) || (n > 4672 && n < 4688))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4667)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4672)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
; 4688 to 4843
.if((n > 4687 && n < 4834))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4834 || n = 4835)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4836)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4837)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4838)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4839 || n = 4841 || n = 4843)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4840)
	; 12 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	plp
.elseif(n = 4842)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
.if((n >= 4844) && (n < 4863))
; 4844 to 4862
.if(n = 4844 || n = 4853 || n = 4859)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
	; 14 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 4847)
	; 14 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4848)
	; 14 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	adc #1
	Jne *-8
	plp
.elseif(n = 4849)
	; 13 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-7
	plp
.elseif(n = 4850)
	; 14 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	plp
.elseif(n = 4851)
	; 13 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	plp
.elseif(n = 4852)
	; 14 bytes
	php
	tya
	ldy #(n-12)/20
	cmp $C5
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4854 || n = 4858 || n = 4860)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4855)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4856)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	rol $26
	ror $26
	iny
	Jne *-8
	tay
	plp
.elseif(n = 4857)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4861)
	; 13 bytes
	php
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	tax
	plp
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
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
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
	; 14 bytes
	php
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4866 || n = 4877 || n = 4879)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4867)
	; 15 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
.elseif(n = 4869 || n = 4871)
	; 15 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4870)
	; 14 bytes
	php
	sec
	lda #(n-10)/20
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 4872)
	; 14 bytes
	php
	tya
	ldy #(n-12)/20
	cmp $C5
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4873)
	; 13 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	plp
.elseif(n = 4874)
	; 12 bytes
	php
	sec
	lda #0
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4875 || n = 4878)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4876)
	; 12 bytes
	php
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4880)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-6
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
.elseif(n = 4882)
	; 12 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	plp
.endif
.endif
.else
; 4883 to 4921
.if(n = 4883 || n = 4885 || n = 4887 || n = 4892 || n = 4899 || n = 4911)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4884)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4886 || n = 4888 || n = 4891 || n = 4893 || n = 4894 || n = 4897 || n = 4898 || n = 4900 || n = 4905)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4889)
	; 7 bytes
	php
	lda #19
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4890 || n = 4895 || n = 4902)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4896)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4901)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4903)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4904)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 4906)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 4907)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4908)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4909)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4910)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4912)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4913)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 4913 && n < 4918))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 4917 && n < 4922))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 4922 to 5000
.if(n = 4922 || (n > 4923 && n < 4928) || (n > 4928 && n < 5001))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 4923)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 4928)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if((n > 5078 && n < 5090))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5090 || n = 5091)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5092 || n = 5097)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5093 || n = 5095 || n = 5096)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5094 || n = 5115)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5098)
	; 14 bytes
	php
	sec
	lda #(n-10)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5099)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5100)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5101)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5102)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5103)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5104)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5105)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5106)
	; 14 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5107)
	; 14 bytes
	php
	clc
	lda #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-8
	plp
.elseif(n = 5108)
	; 14 bytes
	php
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5109)
	; 14 bytes
	php
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5110)
	; 14 bytes
	php
	sec
	lda #(n-10)/20
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5111)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 11
	iny
	Jne *-8
	tay
	plp
.elseif(n = 5112)
	; 14 bytes
	php
	tya
	ldy #(n-12)/20
	cmp $C5
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5113)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5114)
	; 14 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	cmp $C5
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5116)
	; 14 bytes
	php
	tya
	ldy #(n-12)/22
	cmp $C5
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5117)
	; 14 bytes
	delay_n (n-2)
	nop
.endif
.else
; 5118 to 5156
.if(n = 5118 || n = 5141)
	; 14 bytes
	php
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5119 || n = 5126 || n = 5142 || n = 5144 || n = 5147 || n = 5149 || n = 5150 || n = 5153 || n = 5154 || n = 5156)
	; 8..15 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5121 || n = 5146 || n = 5151)
	; 9..15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5122)
	; 14 bytes
	php
	sec
	lda #(n-10)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5123)
	; 15 bytes
	php
	tya
	pha
	ldy #176
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 5124)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5125)
	; 15 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 5127)
	; 15 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5128)
	; 16 bytes
	php
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
	plp
.elseif(n = 5129)
	; 16 bytes
	php
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
	plp
.elseif(n = 5130)
	; 14 bytes
	php
	sec
	lda #0
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5131)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 11
	dey
	Jne *-8
	tay
	plp
.elseif(n = 5132)
	; 13 bytes
	php
	sec
	lda #244 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5133)
	; 14 bytes
	php
	clc
	lda #12 ;hides 'nop abs'
	nop $0
	jsr @nearby_rts
	adc #1
	Jne *-8
	plp
.elseif(n = 5134)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5135)
	; 14 bytes
	php
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5136)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5137 || n = 5143 || n = 5148 || n = 5155)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5138)
	; 14 bytes
	php
	tya
	ldy #(n-12)/22
	cmp $C5
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5139)
	; 14 bytes
	php
	sec
	lda #(n-10)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5140)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5145)
	; 7 bytes
	php
	lda #20
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5152)
	; 9 bytes
	php
	pha
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 5157 to 5312
.if(n = 5157)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5158)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5159)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5160)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 5161)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5162)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 5163)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5164)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5165)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5166)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5167)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5168)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5169)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 5169 && n < 5174))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 5173 && n < 5179) || (n > 5179 && n < 5184) || (n > 5184 && n < 5313))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5179)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5184)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
.if((n > 5312 && n < 5346))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5346 || n = 5347)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5348)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 5348 && n < 5352))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5354)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-254)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5355)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5356)
	; 14 bytes
	php
	sec
	lda #(n-10)/22
	bit $24
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5357)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5358)
	; 14 bytes
	php
	tya
	ldy #(n-12)/22
	cmp $C5
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5359)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5360)
	; 14 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5361 || n = 5363 || n = 5370)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5362)
	; 14 bytes
	php
	sec
	lda #(n-10)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5364)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5365)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5366)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5367)
	; 13 bytes
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5368)
	; 14 bytes
	php
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5369)
	; 14 bytes
	php
	sec
	lda #(n-10)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.endif
.else
; 5371 to 5390
.if(n = 5371)
	; 14 bytes
	php
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
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
	; 15 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 5374)
	; 15 bytes
	php
	pha
	tya
	ldy #255
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5375)
	; 15 bytes
	php
	sec
	lda #185
	delay_n 24
	sbc #1
	Jne *-8
	plp
.elseif(n = 5376)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5377)
	; 15 bytes
	php
	tya
	ldy #185
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5378)
	; 14 bytes
	php
	sec
	lda #(n-10)/22
	bit $24
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5379)
	; 16 bytes
	php
	tya
	pha
	ldy #134
	lda #6
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 5380)
	; 14 bytes
	php
	tya
	ldy #(n-12)/22
	cmp $C5
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5381 || n = 5389)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5382)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5383)
	; 15 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-8
	tay
	plp
.elseif(n = 5384)
	; 15 bytes
	php
	tya
	pha
	ldy #185
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 5385)
	; 15 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 4
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 5386)
	; 13 bytes
	php
	sec
	lda #0
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5387)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5388)
	; 13 bytes
	php
	tya
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5390)
	; 13 bytes
	php
	sec
	lda #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.endif
.endif
.endif
.else
; 5391 to 5468
.if(n = 5391 || n = 5393 || n = 5395 || n = 5399 || n = 5404 || n = 5411 || n = 5423)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5392)
	; 13 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5394)
	; 14 bytes
	php
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5396)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
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
.elseif(n = 5398 || n = 5400 || n = 5403 || n = 5405 || n = 5406 || n = 5409 || n = 5410 || n = 5412 || n = 5417)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5401)
	; 7 bytes
	php
	lda #21
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5402 || n = 5407 || n = 5414)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5408)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5413)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5415)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5416)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 5418)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 5419)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5420)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5421)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5422)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5424)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5425)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 5425 && n < 5430))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 5429 && n < 5435) || (n > 5435 && n < 5440) || (n > 5440 && n < 5469))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5435)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5440)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 5469 to 5625
.if((n > 5468 && n < 5602))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5602 || n = 5603)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5604 || n = 5618 || n = 5625)
	; 13..16 bytes
	pha
	delay_n (n-3)
.elseif((n > 5604 && n < 5609))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-254)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5610)
	; 14 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5611)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5612)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5613)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5614)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5615)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5616)
	; 15 bytes
	php
	tya
	pha
	ldy #193
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 5617)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5619)
	; 14 bytes
	php
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-8
	plp
.elseif(n = 5620)
	; 13 bytes
	php
	sec
	lda #244 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5621)
	; 14 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5622)
	; 13 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5623)
	; 14 bytes
	php
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5624)
	; 14 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-8
	plp
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
	; 14 bytes
	php
	sec
	lda #(n-10)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5627)
	; 15 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5628)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5629)
	; 15 bytes
	php
	tya
	pha
	ldy #245 ;hides 'sbc zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 5630)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5631)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5632)
	; 16 bytes
	php
	tya
	ldy #19
	pha
	lda #240
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 5633)
	; 16 bytes
	php
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
	plp
.elseif(n = 5634)
	; 16 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #105
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	plp
.elseif(n = 5635)
	; 15 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 4
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 5636)
	; 15 bytes
	php
	sec
	lda #194
	delay_n 24
	sbc #1
	Jne *-8
	plp
.elseif(n = 5637)
	; 15 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 4
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5638)
	; 15 bytes
	php
	tya
	ldy #194
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5639)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-300)/281
	lda #247
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 5640)
	; 15 bytes
	php
	tya
	ldy #201
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5641)
	; 16 bytes
	php
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #1
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 5642)
	; 14 bytes
	php
	sec
	lda #0
	bit $24
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5643)
	; 16 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	nop
	delay_n 11
	dey
	Jne *-9
	tay
	plp
.elseif(n = 5644)
	; 14 bytes
	php
	tya
	ldy #0
	cmp $C5
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.endif
.else
; 5645 to 5664
.if(n = 5645)
	; 13 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5646 || n = 5649 || n = 5654 || n = 5656 || n = 5659 || n = 5661 || n = 5662)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5647)
	; 14 bytes
	php
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5648 || n = 5655 || n = 5660)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5650)
	; 14 bytes
	php
	sec
	lda #(n-10)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5651)
	; 15 bytes
	php
	tya
	pha
	ldy #127
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jpl *-5
	pla
	tay
	plp
.elseif(n = 5652)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5653)
	; 15 bytes
	php
	sec
	lda #171
	delay_n 28
	sbc #1
	Jne *-8
	plp
.elseif(n = 5657)
	; 7 bytes
	php
	lda #22
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5658 || n = 5663)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.if(n = 5665 || n = 5666 || n = 5668 || n = 5673)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5667 || n = 5679)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5669)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5670)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5671)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5672)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 5674)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 5675)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5676)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5677)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5678)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5680)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5681)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 5681 && n < 5686))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 5685 && n < 5691) || (n > 5691 && n < 5696) || (n > 5696 && n < 5704))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5691)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5696)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 5704 to 5781
.if((n > 5703 && n < 5782))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if((n > 5781 && n < 5858))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5858 || n = 5859)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 5860) && (n < 5899))
.if((n >= 5860) && (n < 5879))
; 5860 to 5878
.if(n = 5860)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 5860 && n < 5865))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5866)
	; 14 bytes
	php
	sec
	lda #(n-10)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5867)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5868)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5869)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5870)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5871)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5872)
	; 16 bytes
	php
	tya
	ldy #19
	pha
	lda #252
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 5873)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5874)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5875)
	; 14 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5876)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5877)
	; 14 bytes
	php
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5878)
	; 15 bytes
	delay_n (n-2)
	nop
.endif
.else
; 5879 to 5898
.if(n = 5879 || n = 5882 || n = 5894)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5880)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5881)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	plp
.elseif(n = 5883)
	; 16 bytes
	php
	tya
	ldy #57
	pha
	lda #62
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 5884)
	; 15 bytes
	php
	sec
	lda #178
	delay_n 28
	sbc #1
	Jne *-8
	plp
.elseif(n = 5885)
	; 14 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5886)
	; 14 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5887)
	; 15 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 4
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5888)
	; 14 bytes
	php
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif((n .mod 2 = 1) && ((n > 5888 && n < 5896)))
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5890)
	; 14 bytes
	php
	sec
	lda #(n-10)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5892)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5896)
	; 15 bytes
	php
	tya
	ldy #218 ;hides 'nop'
	delay_n 4
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	plp
.elseif(n = 5897)
	; 14 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
	tax
	plp
.elseif(n = 5898)
	; 14 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	plp
.endif
.endif
.else
; 5899 to 5937
.if(n = 5899)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5900)
	; 14 bytes
	php
	tya
	ldy #0
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5901 || n = 5910 || n = 5912 || n = 5915 || n = 5917 || n = 5918 || n = 5921 || n = 5922 || n = 5924 || n = 5929)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5902)
	; 15 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5903 || n = 5911 || n = 5916 || n = 5923 || n = 5935)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5904)
	; 16 bytes
	php
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
	plp
.elseif(n = 5905)
	; 16 bytes
	php
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
	plp
.elseif(n = 5906)
	; 15 bytes
	php
	tya
	pha
	ldy #203
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 5907)
	; 14 bytes
	php
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-8
	plp
.elseif(n = 5908)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5909)
	; 14 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5913)
	; 7 bytes
	php
	lda #23
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5914 || n = 5919 || n = 5926)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5920)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5925)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5927)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5928)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 5930)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 5931)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5932)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5933)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5934)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5936)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5937)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if((n > 5937 && n < 5942))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 5941 && n < 5947) || (n > 5947 && n < 5952) || (n > 5952 && n < 6094))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 5947)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 5952)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if((n > 6093 && n < 6114))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6114 || n = 6115)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6116)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 6116 && n < 6121))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6122)
	; 14 bytes
	php
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6123)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6124)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6125)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6126)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6127)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6128)
	; 15 bytes
	php
	clc
	lda #52 ;hides 'nop zp,x'
	delay_n 9
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-9
	plp
.elseif(n = 6129)
	; 15 bytes
	php
	sec
	lda #211
	delay_n 24
	sbc #1
	Jne *-8
	plp
.elseif(n = 6130)
	; 14 bytes
	php
	sec
	lda #(n-10)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 6131)
	; 15 bytes
	php
	tya
	ldy #211
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6132)
	; 14 bytes
	php
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.endif
.else
.if((n >= 6133) && (n < 6152))
; 6133 to 6151
.if(n = 6133)
	; 15 bytes
	php
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #202 ;hides 'dex'
	Jne *-1
	Jcc *-5
	pla
	tax
	plp
.elseif(n = 6134)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6135)
	; 14 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6136)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6137)
	; 15 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 4
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6138)
	; 15 bytes
	php
	tya
	pha
	ldy #211
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6139)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6140)
	; 15 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6141)
	; 16 bytes
	php
	tya
	ldy #27
	pha
	lda #186
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6142)
	; 15 bytes
	php
	sec
	lda #219
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 6143)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6144)
	; 15 bytes
	php
	tya
	ldy #219
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6145)
	; 16 bytes
	php
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
	plp
.elseif(n = 6146)
	; 14 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 6147)
	; 16 bytes
	php
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
	plp
.elseif(n = 6148)
	; 14 bytes
	php
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6149 || n = 6151)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6150)
	; 15 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.endif
.else
; 6152 to 6171
.if(n = 6152)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6153)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 6154)
	; 14 bytes
	php
	sec
	lda #0
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 6155)
	; 15 bytes
	php
	tya
	pha
	ldy #58 ;hides 'nop'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	iny
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6156)
	; 14 bytes
	php
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6157 || n = 6159 || n = 6167)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6158)
	; 15 bytes
	php
	sec
	lda #212
	delay_n 24
	sbc #1
	Jne *-8
	plp
.elseif(n = 6160)
	; 15 bytes
	php
	tya
	ldy #212
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6161)
	; 16 bytes
	php
	tya
	ldy #143
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6162)
	; 15 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 4
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6163)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #30
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6164)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6165)
	; 16 bytes
	php
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
	plp
.elseif(n = 6166 || n = 6168 || n = 6171)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 6169)
	; 7 bytes
	php
	lda #24
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6170)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172 || n = 6179 || n = 6191)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6173 || n = 6174 || n = 6177 || n = 6178 || n = 6180 || n = 6185)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6175 || n = 6182)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6176)
	; 9 bytes
	php
	pha
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6181)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6183)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6184)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 6186)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 6187)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6188)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6189)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6190)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6192)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6193)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 6193 && n < 6198))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 6197 && n < 6203) || (n > 6203 && n < 6208) || (n > 6208 && n < 6251))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6203)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6208)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if((n > 6328 && n < 6368))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 6368) && (n < 6387))
; 6368 to 6386
.if(n = 6368 || n = 6369)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6370 || n = 6371)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6372)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 6372 && n < 6377))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6378)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6379)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6380)
	; 14 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 6381)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6382)
	; 14 bytes
	php
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6383)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6384)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6385)
	; 15 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	plp
.elseif(n = 6386)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	plp
.endif
.else
; 6387 to 6406
.if(n = 6387)
	; 15 bytes
	php
	tya
	ldy #124
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jpl *-7
	tay
	plp
.elseif(n = 6388)
	; 16 bytes
	php
	tya
	pha
	ldy #32
	lda #159
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 6389)
	; 15 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	delay_n 9
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-9
	plp
.elseif(n = 6390)
	; 15 bytes
	php
	sec
	lda #220
	delay_n 24
	sbc #1
	Jne *-8
	plp
.elseif(n = 6391)
	; 15 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	plp
.elseif(n = 6392)
	; 15 bytes
	php
	tya
	ldy #220
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6393)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6394)
	; 15 bytes
	php
	sec
	lda #228
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 6395)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6396)
	; 15 bytes
	php
	tya
	ldy #228
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6397)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #254
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 6398)
	; 15 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6399)
	; 15 bytes
	php
	tya
	pha
	ldy #220
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6400)
	; 15 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6401)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6402)
	; 16 bytes
	php
	tya
	ldy #142
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6403)
	; 16 bytes
	php
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
	plp
.elseif(n = 6404)
	; 16 bytes
	php
	tya
	ldy #136
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6405)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6406)
	; 14 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
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
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6408)
	; 14 bytes
	php
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6409)
	; 15 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	plp
.elseif(n = 6410 || n = 6422 || n = 6424)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6411)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6412)
	; 15 bytes
	php
	tya
	ldy #0
	delay_n 4
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6413)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6414)
	; 15 bytes
	php
	tya
	ldy #194
	delay_n 28
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6415)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6416)
	; 16 bytes
	php
	tya
	pha
	ldy #26 ;hides 'nop'
	lda #201
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 6417)
	; 16 bytes
	php
	tya
	ldy #61
	pha
	lda #64
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6418)
	; 16 bytes
	php
	tya
	pha
	ldy #81
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6419)
	; 15 bytes
	php
	sec
	lda #221
	delay_n 24
	sbc #1
	Jne *-8
	plp
.elseif(n = 6420)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6421)
	; 15 bytes
	php
	tya
	ldy #221
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6423)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6425)
	; 7 bytes
	php
	lda #25
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 6426 to 6445
.if(n = 6426 || n = 6431 || n = 6438)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6427 || n = 6429 || n = 6430 || n = 6433 || n = 6434 || n = 6436 || n = 6441)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6428 || n = 6435)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6432)
	; 9 bytes
	php
	pha
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6437)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6439)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6440)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 6442)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 6443)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6444)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6445)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 6446 to 6484
.if(n = 6446)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6447)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6448)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6449)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 6449 && n < 6454))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 6453 && n < 6459) || (n > 6459 && n < 6464) || (n > 6464 && n < 6485))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6459)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6464)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 6485 to 6562
.if((n > 6484 && n < 6563))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if((n > 6562 && n < 6626))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6626 || n = 6627)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6628)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 6628 && n < 6633))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-258)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6634)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-259)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6635)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6636)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6637)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6638)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6639)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6640)
	; 14 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.endif
.else
.if((n >= 6641) && (n < 6680))
.if((n >= 6641) && (n < 6660))
; 6641 to 6659
.if(n = 6641)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6642)
	; 14 bytes
	php
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6643)
	; 15 bytes
	php
	sec
	lda #201
	delay_n 28
	sbc #1
	Jne *-8
	plp
.elseif(n = 6644)
	; 15 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 6645)
	; 15 bytes
	php
	tya
	ldy #201
	delay_n 28
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6646)
	; 15 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6647)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6648)
	; 15 bytes
	php
	tya
	ldy #237
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6649)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6650)
	; 16 bytes
	php
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
	plp
.elseif(n = 6651)
	; 15 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	iny
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6652)
	; 15 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6653)
	; 15 bytes
	php
	tya
	ldy #229
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6654)
	; 16 bytes
	php
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
	plp
.elseif(n = 6655)
	; 16 bytes
	php
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
	plp
.elseif(n = 6656)
	; 16 bytes
	php
	tya
	ldy #151
	pha
	lda #3
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6657)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #130
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 6658)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #234
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 6659)
	; 16 bytes
	php
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
	plp
.endif
.else
; 6660 to 6679
.if(n = 6660)
	; 15 bytes
	php
	tya
	pha
	ldy #229
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6661)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6662)
	; 16 bytes
	php
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
	plp
.elseif(n = 6663)
	; 15 bytes
	php
	tya
	pha
	ldy #151
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6664)
	; 16 bytes
	php
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
	plp
.elseif(n = 6665 || n = 6670 || n = 6678)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6666)
	; 14 bytes
	php
	sec
	lda #0
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 6667)
	; 16 bytes
	php
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
	plp
.elseif(n = 6668)
	; 14 bytes
	php
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6669 || n = 6671 || n = 6679)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6672)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6673)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #28
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 6674)
	; 15 bytes
	php
	sec
	lda #238
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 6675)
	; 15 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 6676)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6677)
	; 15 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.endif
.endif
.else
; 6680 to 6718
.if(n = 6680 || n = 6683 || n = 6685 || n = 6686 || n = 6689 || n = 6690 || n = 6692 || n = 6697)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6681)
	; 7 bytes
	php
	lda #26
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6682 || n = 6687 || n = 6694)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6684 || n = 6691 || n = 6703)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6688)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6693)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6695)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6696)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 6698)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 6699)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6700)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6701)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6702)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6704)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6705)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 6705 && n < 6710))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 6709 && n < 6715) || (n > 6715 && n < 6719))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6715)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.endif
.else
; 6719 to 6875
.if(n = 6719 || (n > 6720 && n < 6876))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6720)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
.if((n > 6875 && n < 6882))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6882 || n = 6883)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6884)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 6884 && n < 6889))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-259)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6890)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-260)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6891)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6892)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6893)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6894)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 6895 to 6914
.if(n = 6895)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6896)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6897)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6898)
	; 15 bytes
	php
	sec
	lda #246
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 6899)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6900)
	; 15 bytes
	php
	tya
	ldy #246
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6901)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6902)
	; 16 bytes
	php
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
	plp
.elseif(n = 6903)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6904)
	; 16 bytes
	php
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
	plp
.elseif(n = 6905)
	; 16 bytes
	php
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
	plp
.elseif(n = 6906)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #37
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 6907)
	; 15 bytes
	php
	sec
	lda #209
	delay_n 28
	sbc #1
	Jne *-8
	plp
.elseif(n = 6908)
	; 15 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
	Jcs *-10
	plp
.elseif(n = 6909)
	; 15 bytes
	php
	tya
	ldy #209
	delay_n 28
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6910)
	; 15 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	plp
.elseif(n = 6911)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6912)
	; 15 bytes
	php
	sec
	lda #238
	delay_n 24
	sbc #1
	Jne *-8
	plp
.elseif(n = 6913)
	; 16 bytes
	php
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
	plp
.elseif(n = 6914)
	; 15 bytes
	php
	tya
	ldy #238
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.endif
.endif
.else
.if((n >= 6915) && (n < 6934))
; 6915 to 6933
.if(n = 6915)
	; 16 bytes
	php
	tya
	ldy #58
	pha
	lda #76
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 6916)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6917)
	; 15 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6918)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6919)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6920)
	; 16 bytes
	php
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
	plp
.elseif(n = 6921)
	; 15 bytes
	php
	tya
	pha
	ldy #238
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6922)
	; 16 bytes
	php
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
	plp
.elseif(n = 6923)
	; 15 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 6924)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6925)
	; 15 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6926)
	; 15 bytes
	php
	sec
	lda #247
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 6927)
	; 15 bytes
	php
	tya
	pha
	ldy #157
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6928)
	; 15 bytes
	php
	tya
	ldy #247
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6929)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6930)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #130
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 6931)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6932)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6933)
	; 16 bytes
	php
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
	plp
.endif
.else
; 6934 to 6953
.if(n = 6934 || n = 6936 || n = 6939 || n = 6941 || n = 6942 || n = 6945 || n = 6946 || n = 6948 || n = 6953)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6935 || n = 6940 || n = 6947)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6937)
	; 7 bytes
	php
	lda #27
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6938 || n = 6943 || n = 6950)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6944)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6949)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6951)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6952)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.endif
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 6955)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6956)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6957)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6958)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6959)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6960)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6961)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 6961 && n < 6966))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 6965 && n < 6971) || (n > 6971 && n < 6976) || (n > 6976 && n < 7032))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 6971)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 6976)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
; 7032 to 7109
.if((n > 7031 && n < 7110))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if((n > 7109 && n < 7138))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7138 || n = 7139)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7140)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 7140 && n < 7145))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-260)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7146)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-261)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7147)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7148)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 7149) && (n < 7168))
; 7149 to 7167
.if(n = 7149)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7150)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7151)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7152)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7153)
	; 15 bytes
	php
	tya
	pha
	ldy #246
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7154)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 7155)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7156)
	; 16 bytes
	php
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
	plp
.elseif(n = 7157)
	; 16 bytes
	php
	tya
	pha
	ldy #166
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7158)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-140)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7159)
	; 16 bytes
	php
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
	plp
.elseif(n = 7160)
	; 16 bytes
	php
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
	plp
.elseif(n = 7161)
	; 16 bytes
	php
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #11
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 7162)
	; 16 bytes
	php
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
	plp
.elseif(n = 7163)
	; 16 bytes
	php
	tya
	pha
	ldy #188
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7164)
	; 16 bytes
	php
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
	plp
.elseif(n = 7165)
	; 16 bytes
	php
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
	plp
.elseif(n = 7166)
	; 16 bytes
	php
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
	plp
.elseif(n = 7167)
	; 16 bytes
	php
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
	plp
.endif
.else
; 7168 to 7187
.if(n = 7168)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7169)
	; 16 bytes
	php
	tya
	pha
	ldy #64
	lda #76
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7170)
	; 16 bytes
	php
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
	plp
.elseif(n = 7171)
	; 15 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 7172)
	; 16 bytes
	php
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
	plp
.elseif(n = 7173)
	; 15 bytes
	php
	sec
	lda #247
	delay_n 24
	sbc #1
	Jne *-8
	plp
.elseif(n = 7174)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #(n-618)/596
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7175)
	; 15 bytes
	php
	tya
	ldy #247
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7176)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7177)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #143
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 7178)
	; 15 bytes
	php
	sec
	lda #0
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 7179)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #6
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7180)
	; 15 bytes
	php
	tya
	ldy #0
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7181)
	; 16 bytes
	php
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
	plp
.elseif(n = 7182)
	; 15 bytes
	php
	tya
	pha
	ldy #247
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7183)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7184)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7185)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #92
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 7186)
	; 16 bytes
	php
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
	plp
.elseif(n = 7187)
	; 16 bytes
	php
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
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
; 7188 to 7343
.if(n = 7188)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7189)
	; 16 bytes
	php
	tya
	pha
	ldy #29
	lda #205
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7190 || n = 7192 || n = 7195 || n = 7197 || n = 7198 || n = 7201 || n = 7202 || n = 7204 || n = 7209)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7191 || n = 7196 || n = 7203 || n = 7215)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7193)
	; 7 bytes
	php
	lda #28
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7194 || n = 7199 || n = 7206)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7200)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7205)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7207)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7208)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 7210)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 7211)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7212)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7213)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7214)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7216)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7217)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 7217 && n < 7222))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 7221 && n < 7227) || (n > 7227 && n < 7232) || (n > 7232 && n < 7344))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7227)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7232)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if((n > 7343 && n < 7394))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7394 || n = 7395)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7396)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 7396 && n < 7401))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-261)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7402)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-262)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7403)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7404)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7405)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7406)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7407)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7408)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7409)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7410)
	; 16 bytes
	php
	tya
	ldy #54
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7411)
	; 15 bytes
	php
	tya
	pha
	ldy #168
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7412)
	; 16 bytes
	php
	tya
	ldy #148
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7413 || n = 7416)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7414)
	; 15 bytes
	php
	tya
	pha
	ldy #255
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7415)
	; 16 bytes
	php
	tya
	pha
	ldy #172
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7417)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7418)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7419)
	; 15 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 7420)
	; 16 bytes
	php
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
	plp
.elseif(n = 7421)
	; 15 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.endif
.else
.if((n >= 7422) && (n < 7461))
.if((n >= 7422) && (n < 7441))
; 7422 to 7440
.if(n = 7422)
	; 16 bytes
	php
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
	plp
.elseif(n = 7423)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7424)
	; 16 bytes
	php
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
	plp
.elseif(n = 7425)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7426)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 7427)
	; 16 bytes
	php
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
	plp
.elseif(n = 7428)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7429)
	; 16 bytes
	php
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
	plp
.elseif(n = 7430)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7431)
	; 16 bytes
	php
	tya
	pha
	ldy #218
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7432)
	; 16 bytes
	php
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
	plp
.elseif(n = 7433)
	; 16 bytes
	php
	tya
	ldy #<((n-12)/41)
	pha
	lda #>((n-12)/41)
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7434)
	; 15 bytes
	php
	sec
	lda #0
	delay_n 24
	sbc #1
	Jne *-8
	plp
.elseif(n = 7435)
	; 15 bytes
	php
	sec
	lda #225
	delay_n 28
	sbc #1
	Jne *-8
	plp
.elseif(n = 7436)
	; 15 bytes
	php
	tya
	ldy #0
	delay_n 24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7437)
	; 15 bytes
	php
	tya
	ldy #225
	delay_n 28
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7438)
	; 16 bytes
	php
	tya
	ldy #158
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7439)
	; 16 bytes
	php
	tya
	pha
	ldy #212
	lda #1
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7440)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 7441 to 7460
.if(n = 7441)
	; 16 bytes
	php
	tya
	pha
	ldy #58 ;hides 'nop'
	lda #92
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 7442)
	; 16 bytes
	php
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
	plp
.elseif(n = 7443)
	; 15 bytes
	php
	tya
	pha
	ldy #0
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7444)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7445 || n = 7446 || n = 7448 || n = 7451 || n = 7453 || n = 7454 || n = 7457 || n = 7458 || n = 7460)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7447 || n = 7452 || n = 7459)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7449)
	; 7 bytes
	php
	lda #29
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7450 || n = 7455)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7456)
	; 9 bytes
	php
	pha
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 7461 to 7500
.if(n = 7461)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7462)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7463)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7464)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 7465)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7466)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 7467)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7468)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7469)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7470)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7471)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7472)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7473)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 7473 && n < 7478))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 7477 && n < 7483) || (n > 7483 && n < 7488) || (n > 7488 && n < 7501))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7483)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7488)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
.if((n > 7500 && n < 7650))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7650 || n = 7651)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7652)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 7652 && n < 7657))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-262)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7658)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-263)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7659)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7660)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7661)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7662)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7663)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7664)
	; 15 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 26
	adc #1
	Jne *-9
	plp
.elseif(n = 7665)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	php
	pha
	lda #199
	jsr delay_a_27_clocks
	pla
	plp
	Jne *-11
	plp
.elseif(n = 7666)
	; 15 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	delay_n 26
	iny
	Jne *-8
	tay
	plp
.elseif(n = 7667)
	; 15 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 7668)
	; 15 bytes
	php
	tya
	ldy #232
	delay_n 28
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7669)
	; 15 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7670)
	; 16 bytes
	php
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
	plp
.elseif(n = 7671)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7672)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7673)
	; 16 bytes
	php
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
	plp
.elseif(n = 7674)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7675)
	; 15 bytes
	php
	tya
	pha
	ldy #174
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.endif
.else
; 7676 to 7695
.if(n = 7676)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7677 || n = 7682)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7678)
	; 16 bytes
	php
	tya
	pha
	ldy #207
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7679)
	; 16 bytes
	php
	tya
	ldy #<((n-12)/41)
	pha
	lda #>((n-12)/41)
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7680)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7681)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7683)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7684)
	; 16 bytes
	php
	tya
	pha
	ldy #219
	lda #1
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7685)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #106
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 7686)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7687)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7688)
	; 16 bytes
	php
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
	plp
.elseif(n = 7689)
	; 16 bytes
	php
	tya
	pha
	ldy #64
	lda #84
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7690)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #149
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 7691)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7692)
	; 16 bytes
	php
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
	plp
.elseif(n = 7693)
	; 16 bytes
	php
	txa
	pha
	ldx #202 ;hides 'dex'
	lda #166
	jsr delay_a_27_clocks
	dex
	Jmi *-7
	pla
	tax
	plp
.elseif(n = 7694)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7695)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
; 7696 to 7734
.if(n = 7696)
	; 15 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 24
	sbc #1
	Jne *-9
	plp
.elseif(n = 7697)
	; 16 bytes
	php
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
	plp
.elseif(n = 7698)
	; 15 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 24
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7699)
	; 15 bytes
	php
	sec
	lda #233
	delay_n 28
	sbc #1
	Jne *-8
	plp
.elseif(n = 7700)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7701)
	; 15 bytes
	php
	tya
	ldy #233
	delay_n 28
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7702 || n = 7704 || n = 7707 || n = 7709 || n = 7710 || n = 7713 || n = 7714 || n = 7716 || n = 7721)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7703 || n = 7708 || n = 7715 || n = 7727)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7705)
	; 7 bytes
	php
	lda #30
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7706 || n = 7711 || n = 7718)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7712)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7717)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7719)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7720)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 7722)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 7723)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7724)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7725)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7726)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7728)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7729)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 7729 && n < 7734))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7734)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 7735 to 7812
.if((n > 7734 && n < 7739) || (n > 7739 && n < 7744) || (n > 7744 && n < 7813))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7739)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7744)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
; 7813 to 7890
.if((n > 7812 && n < 7891))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if((n > 7890 && n < 7906))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7906 || n = 7907)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7908)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 7908 && n < 7913))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-263)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7914)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-264)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7915)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7916)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7917)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7918)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7919)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7920)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7921)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #40
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 7922)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 7923)
	; 16 bytes
	php
	tya
	pha
	ldy #208
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7924)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7925)
	; 16 bytes
	php
	tya
	pha
	ldy #66
	lda #84
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7926)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7927)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7928)
	; 16 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 7929)
	; 16 bytes
	php
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
	plp
.endif
.else
.if((n >= 7930) && (n < 7949))
; 7930 to 7948
.if(n = 7930)
	; 15 bytes
	php
	sec
	lda #240
	delay_n 28
	sbc #1
	Jne *-8
	plp
.elseif(n = 7931)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7932)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7933)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 7934 || n = 7941 || n = 7945)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7935)
	; 16 bytes
	php
	tya
	ldy #57
	pha
	lda #98
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7936)
	; 16 bytes
	php
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
	plp
.elseif(n = 7937)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7938)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #53
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 7939)
	; 15 bytes
	php
	tya
	pha
	ldy #180
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7940)
	; 16 bytes
	php
	tya
	pha
	ldy #88
	lda #55
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7942)
	; 16 bytes
	php
	tya
	ldy #64
	pha
	lda #81
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 7943)
	; 16 bytes
	php
	tya
	ldy #76
	pha
	lda #62
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 7944)
	; 16 bytes
	php
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
	plp
.elseif(n = 7946)
	; 15 bytes
	php
	sec
	lda #0
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 7947)
	; 16 bytes
	php
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
	plp
.elseif(n = 7948)
	; 15 bytes
	php
	tya
	ldy #0
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.endif
.else
; 7949 to 7968
.if(n = 7949)
	; 16 bytes
	php
	tya
	pha
	ldy #64
	lda #88
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7950)
	; 16 bytes
	php
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
	plp
.elseif(n = 7951 || n = 7959 || n = 7964)
	; 8..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 7952)
	; 16 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #166
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	plp
.elseif(n = 7953)
	; 17 bytes
	php
	lda #91
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7954)
	; 16 bytes
	php
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
	plp
.elseif(n = 7955)
	; 16 bytes
	php
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
	plp
.elseif(n = 7956)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7957)
	; 16 bytes
	php
	tya
	pha
	ldy #189
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7958 || n = 7960 || n = 7963 || n = 7965 || n = 7966)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 7961)
	; 7 bytes
	php
	lda #31
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7962 || n = 7967)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.if(n = 7969 || n = 7970 || n = 7972 || n = 7977)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7971 || n = 7983)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7973)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7974)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7975)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7976)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 7978)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 7979)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7980)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 7981)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7982)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7984)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7985)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 7985 && n < 7990))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 7989 && n < 7995) || (n > 7995 && n < 8000) || (n > 8000 && n < 8126))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 7995)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8000)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
.if((n > 8125 && n < 8162))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8162 || n = 8163)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8164)
	; 13 bytes
	pha
	delay_n (n-3)
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-264)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8170)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-265)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8171)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8172)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8173)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8174)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8175)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8176)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8177)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8178)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 8179)
	; 16 bytes
	php
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
	plp
.elseif(n = 8180)
	; 16 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #172
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	plp
.elseif(n = 8181)
	; 16 bytes
	php
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
	plp
.elseif(n = 8182)
	; 16 bytes
	php
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
	plp
.elseif(n = 8183)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 8184 to 8203
.if(n = 8184)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8185)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8186)
	; 16 bytes
	php
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
	plp
.elseif(n = 8187)
	; 16 bytes
	php
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
	plp
.elseif(n = 8188)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #71
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8189)
	; 16 bytes
	php
	tya
	pha
	ldy #215
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8190)
	; 16 bytes
	php
	tya
	ldy #174
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8191)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8192)
	; 15 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 26
	sbc #1
	Jne *-9
	plp
.elseif(n = 8193)
	; 16 bytes
	php
	tya
	pha
	ldy #66
	lda #88
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8194)
	; 15 bytes
	php
	sec
	lda #248
	delay_n 28
	sbc #1
	Jne *-8
	plp
.elseif(n = 8195)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8196)
	; 15 bytes
	php
	tya
	ldy #248
	delay_n 28
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8197)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #53
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8198)
	; 15 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 28
	sbc #1
	Jne *-9
	plp
.elseif(n = 8199)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8200)
	; 15 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	delay_n 28
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8201)
	; 16 bytes
	php
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
	plp
.elseif(n = 8202)
	; 16 bytes
	php
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
	plp
.elseif(n = 8203)
	; 15 bytes
	php
	tya
	pha
	ldy #186
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
; 8204 to 8242
.if(n = 8204)
	; 16 bytes
	php
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
	plp
.elseif(n = 8205 || n = 8214 || n = 8216 || n = 8219 || n = 8221 || n = 8222 || n = 8225 || n = 8226 || n = 8228 || n = 8233)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8206)
	; 16 bytes
	php
	tya
	ldy #33
	pha
	lda #200
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8207)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8208)
	; 17 bytes
	php
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8209)
	; 16 bytes
	php
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
	plp
.elseif(n = 8210)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #162
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 8211)
	; 16 bytes
	php
	tya
	pha
	ldy #63
	lda #94
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8212)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8213)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8215 || n = 8220 || n = 8227 || n = 8239)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8217)
	; 7 bytes
	php
	lda #32
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8218 || n = 8223 || n = 8230)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8224)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8229)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8231)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8232)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 8234)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 8235)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8236)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8237)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8238)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8240)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8241)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8242)
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.else
; 8243 to 8281
.if((n > 8242 && n < 8246))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 8245 && n < 8251) || (n > 8251 && n < 8256) || (n > 8256 && n < 8282))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8251)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8256)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
; 8282 to 8437
.if((n > 8281 && n < 8418))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8418 || n = 8419)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8420)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 8420 && n < 8425))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-265)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8426)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-266)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8427)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8428)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8429)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8430)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8431)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8432)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8433)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8434)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #51
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8435)
	; 16 bytes
	php
	tya
	pha
	ldy #31
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8436)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8437)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
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
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8439)
	; 16 bytes
	php
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
	plp
.elseif(n = 8440)
	; 16 bytes
	php
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
	plp
.elseif(n = 8441)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 8442)
	; 16 bytes
	php
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
	plp
.elseif(n = 8443)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8444)
	; 16 bytes
	php
	tya
	ldy #68
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8445)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #62
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 8446)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8447)
	; 16 bytes
	php
	tya
	pha
	ldy #196
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8448)
	; 16 bytes
	php
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
	plp
.elseif(n = 8449)
	; 16 bytes
	php
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
	plp
.elseif(n = 8450)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #168
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 8451)
	; 16 bytes
	php
	tya
	pha
	ldy #248
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8452)
	; 16 bytes
	php
	tya
	ldy #39
	pha
	lda #170
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8453)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #112
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 8454)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_b_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 8455)
	; 16 bytes
	php
	tya
	pha
	ldy #228
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8456)
	; 16 bytes
	php
	tya
	pha
	ldy #143
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 8457 to 8476
.if(n = 8457)
	; 16 bytes
	php
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
	plp
.elseif(n = 8458)
	; 15 bytes
	php
	sec
	lda #0
	delay_n 28
	sbc #1
	Jne *-8
	plp
.elseif(n = 8459)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #6
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8460)
	; 15 bytes
	php
	tya
	ldy #0
	delay_n 28
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8461)
	; 16 bytes
	php
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
	plp
.elseif(n = 8462)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8463)
	; 16 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #36
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 8464)
	; 16 bytes
	php
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
	plp
.elseif(n = 8465)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8466)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #108
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 8467)
	; 15 bytes
	php
	tya
	pha
	ldy #192
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 8468)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8469 || n = 8470 || n = 8472 || n = 8475)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8471 || n = 8476)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8473)
	; 7 bytes
	php
	lda #33
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8474)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8478 || n = 8481 || n = 8482 || n = 8484 || n = 8489)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8479 || n = 8486)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8480)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8483 || n = 8495)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8485)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8487)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8488)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 8490)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 8491)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8492)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8493)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8494)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8496)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8497)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 8497 && n < 8502))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 8501 && n < 8507) || (n > 8507 && n < 8512) || (n > 8512 && n < 8516))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8507)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8512)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 8516 to 8593
.if((n > 8515 && n < 8594))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
; 8594 to 8671
.if((n > 8593 && n < 8672))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 8672) && (n < 8711))
.if((n >= 8672) && (n < 8691))
; 8672 to 8690
.if(n = 8672 || n = 8673)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8674 || n = 8675)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8676)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 8676 && n < 8681))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-266)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8682)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-267)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8683)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8684)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8685)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8686)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8687)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8688)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8689)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8690)
	; 15 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 28
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 8691 to 8710
.if(n = 8691)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8692)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8693)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8694)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8695)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8696)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8697)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8698)
	; 16 bytes
	php
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
	plp
.elseif(n = 8699)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8700)
	; 16 bytes
	php
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
	plp
.elseif(n = 8701)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8702)
	; 16 bytes
	php
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
	plp
.elseif(n = 8703)
	; 16 bytes
	php
	tya
	pha
	ldy #167
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8704)
	; 16 bytes
	php
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
	plp
.elseif(n = 8705)
	; 16 bytes
	php
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
	plp
.elseif(n = 8706)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8707)
	; 16 bytes
	php
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
	plp
.elseif(n = 8708)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8709)
	; 16 bytes
	php
	tya
	pha
	ldy #79
	lda #76
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8710)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 8711) && (n < 8731))
; 8711 to 8730
.if(n = 8711)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8712)
	; 16 bytes
	php
	tya
	ldy #174
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8713)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8714)
	; 16 bytes
	php
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
	plp
.elseif(n = 8715)
	; 17 bytes
	php
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8716)
	; 16 bytes
	php
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
	plp
.elseif(n = 8717)
	; 16 bytes
	php
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
	plp
.elseif(n = 8718)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8719)
	; 16 bytes
	php
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
	plp
.elseif(n = 8720)
	; 16 bytes
	php
	tya
	pha
	ldy #76
	lda #79
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8721)
	; 16 bytes
	php
	tya
	pha
	ldy #229
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8722)
	; 16 bytes
	php
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
	plp
.elseif(n = 8723)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8724)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8725)
	; 16 bytes
	php
	sec
	lda #249
	nop
	delay_n 28
	sbc #1
	Jne *-9
	plp
.elseif(n = 8726 || n = 8728)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 8727)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8729)
	; 7 bytes
	php
	lda #34
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8730)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 8731 to 8750
.if(n = 8731 || n = 8733 || n = 8734 || n = 8737 || n = 8738 || n = 8740 || n = 8745)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8732 || n = 8739)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8735 || n = 8742)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8736)
	; 9 bytes
	php
	pha
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8741)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8743)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8744)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 8746)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 8747)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8748)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8749)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8750)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
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
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8752)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8753)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 8753 && n < 8758))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 8757 && n < 8763) || (n > 8763 && n < 8768) || (n > 8768 && n < 8907))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8763)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8768)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 8907) && (n < 8985))
.if((n >= 8907) && (n < 8946))
; 8907 to 8945
.if((n > 8906 && n < 8930))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8930 || n = 8931)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8932)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 8932 && n < 8937))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-267)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8938)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-268)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8939)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8940)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8941)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8942)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8943)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8944)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8945)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #88
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.endif
.else
.if((n >= 8946) && (n < 8965))
; 8946 to 8964
.if(n = 8946)
	; 16 bytes
	php
	tya
	pha
	ldy #79
	lda #79
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8947)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8948)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8949)
	; 16 bytes
	php
	tya
	pha
	ldy #235
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8950)
	; 16 bytes
	php
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
	plp
.elseif(n = 8951)
	; 15 bytes
	php
	tya
	pha
	ldy #203
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 8952)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8953)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8954)
	; 16 bytes
	php
	tya
	ldy #33
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8955)
	; 16 bytes
	php
	tya
	ldy #(n-283)/271
	pha
	lda #230
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8956)
	; 16 bytes
	php
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
	plp
.elseif(n = 8957)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8958)
	; 16 bytes
	php
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
	plp
.elseif(n = 8959)
	; 16 bytes
	php
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
	plp
.elseif(n = 8960)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8961)
	; 16 bytes
	php
	tya
	pha
	ldy #33
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8962)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8963)
	; 16 bytes
	php
	tya
	pha
	ldy #208
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8964)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.endif
.else
; 8965 to 8984
.if(n = 8965)
	; 16 bytes
	php
	tya
	pha
	ldy #213
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8966)
	; 16 bytes
	php
	tya
	ldy #73
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8967)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8968)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8969)
	; 16 bytes
	php
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
	plp
.elseif(n = 8970)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #117
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 8971)
	; 16 bytes
	php
	tya
	ldy #31
	pha
	lda #248
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8972)
	; 16 bytes
	php
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
	plp
.elseif(n = 8973)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-140)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8974)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #11
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8975)
	; 17 bytes
	php
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8976)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8977)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #14
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8978 || n = 8982 || n = 8984)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8979)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8980)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 8981)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #130
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 8983)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.endif
.endif
.else
; 8985 to 9062
.if(n = 8985)
	; 7 bytes
	php
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 8986 || n = 8991 || n = 8998)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8987 || n = 8989 || n = 8990 || n = 8993 || n = 8994 || n = 8996 || n = 9001)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988 || n = 8995 || n = 9007)
	; 8..12 bytes
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
.elseif(n = 8997)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8999)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9000)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 9002)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 9003)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9004)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 9005)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9006)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9008)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9009)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 9009 && n < 9014))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 9013 && n < 9019) || (n > 9019 && n < 9024) || (n > 9024 && n < 9063))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9019)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 9024)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
; 9063 to 9140
.if((n > 9062 && n < 9141))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if((n > 9140 && n < 9180))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 9180) && (n < 9199))
; 9180 to 9198
.if((n > 9179 && n < 9186))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9186 || n = 9187)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9188)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 9188 && n < 9193))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-268)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9194)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-269)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9195)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9196)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9197)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9198)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 9199 to 9218
.if(n = 9199)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9200)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9201)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #128
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9202)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9203)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9204)
	; 16 bytes
	php
	tya
	pha
	ldy #167
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9205)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9206)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 9207)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9208)
	; 16 bytes
	php
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
	plp
.elseif(n = 9209)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9210)
	; 16 bytes
	php
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
	plp
.elseif(n = 9211)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9212)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9213)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #40
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 9214)
	; 16 bytes
	php
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
	plp
.elseif(n = 9215)
	; 15 bytes
	php
	tya
	pha
	ldy #209
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 9216)
	; 16 bytes
	php
	tya
	ldy #77
	pha
	lda #77
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9217)
	; 15 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9218)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
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
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9220)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9221)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9222)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9223)
	; 16 bytes
	php
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
	plp
.elseif(n = 9224)
	; 16 bytes
	php
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
	plp
.elseif(n = 9225)
	; 16 bytes
	php
	tya
	ldy #(n-123)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9226)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #59
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9227)
	; 16 bytes
	php
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
	plp
.elseif(n = 9228)
	; 15 bytes
	php
	tya
	ldy #0
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 9229)
	; 17 bytes
	php
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9230)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9231)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9232)
	; 16 bytes
	php
	tya
	pha
	ldy #249
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9233)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9234)
	; 16 bytes
	php
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
	plp
.elseif(n = 9235)
	; 16 bytes
	php
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
	plp
.elseif(n = 9236)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 9237)
	; 16 bytes
	php
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
	plp
.endif
.else
; 9238 to 9257
.if(n = 9238 || n = 9240 || n = 9243 || n = 9245 || n = 9246 || n = 9249 || n = 9250 || n = 9252 || n = 9257)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9239 || n = 9244 || n = 9251)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 9241)
	; 7 bytes
	php
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9242 || n = 9247 || n = 9254)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9248)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9253)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9255)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9256)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.endif
.endif
.else
; 9258 to 9296
.if(n = 9258)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 9259)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9260)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 9261)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9262)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9263)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 9264)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9265)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 9265 && n < 9270))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 9269 && n < 9275) || (n > 9275 && n < 9280) || (n > 9280 && n < 9297))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9275)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 9280)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 9297 to 9375
.if((n > 9296 && n < 9376))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
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
.if((n > 9375 && n < 9442))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9442 || n = 9443)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9444)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 9444 && n < 9449))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-269)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9450)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-270)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9451)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9452)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9453)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 9454) && (n < 9493))
.if((n >= 9454) && (n < 9473))
; 9454 to 9472
.if(n = 9454)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9455)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9456)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9457)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9458)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9459)
	; 16 bytes
	php
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
	plp
.elseif(n = 9460)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9461)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9462)
	; 16 bytes
	php
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
	plp
.elseif(n = 9463)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9464)
	; 16 bytes
	php
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
	plp
.elseif(n = 9465)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9466)
	; 16 bytes
	php
	tya
	pha
	ldy #201
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9467)
	; 16 bytes
	php
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
	plp
.elseif(n = 9468)
	; 16 bytes
	php
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
	plp
.elseif(n = 9469)
	; 16 bytes
	php
	tya
	pha
	ldy #225
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9470)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9471)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #(n-257)/271
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9472)
	; 16 bytes
	php
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
	plp
.endif
.else
; 9473 to 9492
.if(n = 9473)
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9474)
	; 16 bytes
	php
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
	plp
.elseif(n = 9475)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #14
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9476)
	; 16 bytes
	php
	tya
	ldy #91
	pha
	lda #63
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9477)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #74
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 9478)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9479)
	; 15 bytes
	php
	tya
	pha
	ldy #215
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 9480)
	; 16 bytes
	php
	tya
	ldy #35
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9481)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9482)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 9483)
	; 16 bytes
	php
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
	plp
.elseif(n = 9484)
	; 16 bytes
	php
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
	plp
.elseif(n = 9485)
	; 17 bytes
	php
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9486)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9487)
	; 16 bytes
	php
	tya
	pha
	ldy #35
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9488)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9489)
	; 16 bytes
	php
	tya
	ldy #81
	pha
	lda #76
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9490)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9491)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9492)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.else
; 9493 to 9531
.if(n = 9493 || n = 9494 || n = 9496 || n = 9499 || n = 9501 || n = 9502 || n = 9505 || n = 9506 || n = 9508 || n = 9513)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9495 || n = 9500 || n = 9507 || n = 9519)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 9497)
	; 7 bytes
	php
	lda #37
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9498 || n = 9503 || n = 9510)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9504)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9509)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9511)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9512)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 9514)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 9515)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9516)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 9517)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9518)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9520)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9521)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 9521 && n < 9526))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 9525 && n < 9531))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9531)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.endif
.else
; 9532 to 9687
.if((n > 9531 && n < 9536) || (n > 9536 && n < 9688))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9536)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 9688) && (n < 9844))
.if((n >= 9688) && (n < 9766))
.if((n >= 9688) && (n < 9727))
; 9688 to 9726
.if((n > 9687 && n < 9698))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9698 || n = 9699)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9700 || n = 9714)
	; 13..16 bytes
	pha
	delay_n (n-3)
.elseif((n > 9700 && n < 9705))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-270)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9706)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-271)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9707)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9708)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9709)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9710)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9711)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9712)
	; 16 bytes
	php
	tya
	ldy #194
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9713)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9715)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9716)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9717)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9718)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9719)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #66
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9720)
	; 16 bytes
	php
	tya
	pha
	ldy #88
	lda #75
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9721)
	; 16 bytes
	php
	tya
	pha
	ldy #231
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9722)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9723)
	; 16 bytes
	php
	tya
	ldy #82
	pha
	lda #76
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9724)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9725)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9726)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 9727) && (n < 9746))
; 9727 to 9745
.if(n = 9727)
	; 16 bytes
	php
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
	plp
.elseif(n = 9728)
	; 16 bytes
	php
	tya
	pha
	ldy #72
	lda #99
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9729)
	; 16 bytes
	php
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
	plp
.elseif(n = 9730)
	; 16 bytes
	php
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
	plp
.elseif(n = 9731)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9732)
	; 16 bytes
	php
	tya
	ldy #216
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9733)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9734)
	; 16 bytes
	php
	tya
	pha
	ldy #145
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9735)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #77
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 9736)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9737)
	; 16 bytes
	php
	tya
	pha
	ldy #226
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9738)
	; 16 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #213
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	plp
.elseif(n = 9739)
	; 16 bytes
	php
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
	plp
.elseif(n = 9740)
	; 16 bytes
	php
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
	plp
.elseif(n = 9741)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9742)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #98
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9743)
	; 15 bytes
	php
	tya
	pha
	ldy #221
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 9744)
	; 16 bytes
	php
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
	plp
.elseif(n = 9745)
	; 16 bytes
	delay_n (n-2)
	nop
.endif
.else
; 9746 to 9765
.if(n = 9746)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9747)
	; 16 bytes
	php
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
	plp
.elseif(n = 9748)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 9749)
	; 16 bytes
	php
	tya
	ldy #91
	pha
	lda #66
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9750 || n = 9752 || n = 9755 || n = 9757 || n = 9758 || n = 9761 || n = 9762 || n = 9764)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9751 || n = 9756 || n = 9763)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 9753)
	; 7 bytes
	php
	lda #38
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9754 || n = 9759)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9760)
	; 9 bytes
	php
	pha
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9765)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.endif
.else
; 9766 to 9843
.if(n = 9766)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9767)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9768)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 9769)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9770)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 9771)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9772)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 9773)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9774)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9775)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 9776)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9777)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 9777 && n < 9782))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 9781 && n < 9787) || (n > 9787 && n < 9792) || (n > 9792 && n < 9844))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9787)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 9792)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
; 9844 to 9921
.if((n > 9843 && n < 9922))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 9922) && (n < 9961))
; 9922 to 9960
.if((n > 9921 && n < 9954))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9954 || n = 9955)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9956)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 9956 && n < 9961))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-271)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9962)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-272)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9963)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9964)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9965)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9966)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9967)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9968)
	; 16 bytes
	php
	tya
	ldy #(n-12)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9969)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #16
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9970)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9971)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #201
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9972)
	; 16 bytes
	php
	tya
	ldy #82
	pha
	lda #79
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9973)
	; 16 bytes
	php
	tya
	pha
	ldy #237
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9974)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9975)
	; 16 bytes
	php
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
	plp
.elseif(n = 9976)
	; 16 bytes
	php
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
	plp
.elseif(n = 9977)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #213
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9978)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #25
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9979)
	; 16 bytes
	php
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
	plp
.elseif(n = 9980)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #71
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.endif
.else
; 9981 to 10000
.if(n = 9981)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #58
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9982)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9983)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-125)/106
	lda #72
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9984)
	; 16 bytes
	php
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
	plp
.elseif(n = 9985)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9986)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #0
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 9987)
	; 16 bytes
	php
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
	plp
.elseif(n = 9988)
	; 16 bytes
	php
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
	plp
.elseif(n = 9989)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9990)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9991)
	; 16 bytes
	php
	tya
	pha
	ldy #35
	lda #243
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9992)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #139
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 9993)
	; 16 bytes
	php
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
	plp
.elseif(n = 9994)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9995)
	; 16 bytes
	php
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
	plp
.elseif(n = 9996)
	; 16 bytes
	php
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
	plp
.elseif(n = 9997)
	; 17 bytes
	php
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9998)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 9999)
	; 17 bytes
	php
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
	plp
.elseif(n = 10000)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	lda #3
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
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
; 10001 to 10156
.if(n = 10001)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10002)
	; 16 bytes
	php
	tya
	ldy #222
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10003)
	; 16 bytes
	php
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
	plp
.elseif(n = 10004)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10005)
	; 17 bytes
	php
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10006 || n = 10008 || n = 10011 || n = 10013 || n = 10014 || n = 10017 || n = 10018 || n = 10020 || n = 10025)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10007 || n = 10012 || n = 10019 || n = 10031)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 10009)
	; 7 bytes
	php
	lda #39
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10010 || n = 10015 || n = 10022)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10016)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10021)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10023)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10024)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 10026)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 10027)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10028)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10029)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10030)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10032)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10033)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 10033 && n < 10038))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 10037 && n < 10043) || (n > 10043 && n < 10048) || (n > 10048 && n < 10157))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10043)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10048)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 10157) && (n < 10235))
; 10157 to 10234
.if((n > 10156 && n < 10210))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10210 || n = 10211)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10212)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 10212 && n < 10217))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-272)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10218)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-273)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10219)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10220)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10221)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10222)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10223)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10224)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10225)
	; 16 bytes
	php
	tya
	pha
	ldy #243
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10226)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10227)
	; 15 bytes
	php
	tya
	pha
	ldy #232
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 10228)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10229)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 10230)
	; 16 bytes
	php
	tya
	ldy #(n-143)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10231)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10232)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #99
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10233)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10234)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #11
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 10235) && (n < 10274))
.if((n >= 10235) && (n < 10254))
; 10235 to 10253
.if(n = 10235)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10236)
	; 16 bytes
	php
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
	plp
.elseif(n = 10237)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-150)/131
	lda #97
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10238)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10239)
	; 16 bytes
	php
	tya
	pha
	ldy #146
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10240)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10241)
	; 16 bytes
	php
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
	plp
.elseif(n = 10242)
	; 16 bytes
	php
	tya
	ldy #93
	pha
	lda #69
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10243)
	; 16 bytes
	php
	tya
	pha
	ldy #213
	lda #14
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10244)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10245)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 10246)
	; 16 bytes
	php
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
	plp
.elseif(n = 10247)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #74
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10248)
	; 16 bytes
	php
	tya
	pha
	ldy #193
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10249)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #76
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10250)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #213
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 10251)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #83
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 10252)
	; 16 bytes
	php
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
	plp
.elseif(n = 10253)
	; 16 bytes
	php
	tya
	pha
	ldy #238
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 10254 to 10273
.if(n = 10254)
	; 16 bytes
	php
	tya
	pha
	ldy #88
	lda #81
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10255 || n = 10262 || n = 10264 || n = 10267 || n = 10269 || n = 10270 || n = 10273)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10256)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10257)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #220
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 10258)
	; 16 bytes
	php
	tya
	ldy #218
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10259)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #6
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10260)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10261)
	; 16 bytes
	php
	tya
	ldy #37
	pha
	lda #236
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10263 || n = 10268)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10265)
	; 7 bytes
	php
	lda #40
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10266 || n = 10271)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.if(n = 10274 || n = 10276 || n = 10281)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275 || n = 10287)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 10277)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10278)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10279)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10280)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 10282)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 10283)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10284)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10285)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10286)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10288)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10289)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 10289 && n < 10294))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 10293 && n < 10299) || (n > 10299 && n < 10304) || (n > 10304 && n < 10313))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10299)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10304)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
; 10313 to 10468
.if((n > 10312 && n < 10466))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10466 || n = 10467)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10468)
	; 13 bytes
	pha
	delay_n (n-3)
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-273)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10474)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-274)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10475)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10476)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10477)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10478)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10479)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10480)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10481)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10482)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #144
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 10483)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #75
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10484)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10485)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10486)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10487)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 10488 to 10507
.if(n = 10488)
	; 16 bytes
	php
	tya
	ldy #96
	pha
	lda #67
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10489)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #151
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 10490)
	; 16 bytes
	php
	tya
	pha
	ldy #37
	lda #249
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10491)
	; 15 bytes
	php
	tya
	pha
	ldy #238
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 10492)
	; 16 bytes
	php
	tya
	ldy #(n-143)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10493)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 10494)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10495)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #74
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10496)
	; 16 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #55
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 10497)
	; 16 bytes
	php
	tya
	ldy #233
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10498)
	; 16 bytes
	php
	tya
	ldy #214
	pha
	lda #8
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10499)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-150)/131
	lda #97
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10500)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10501)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10502)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #5
	jsr delay_256a_11_clocks_b_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 10503)
	; 16 bytes
	php
	tya
	ldy #39
	pha
	lda #228
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10504)
	; 15 bytes
	php
	tya
	ldy #58 ;hides 'nop'
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	iny
	Jne *-8
	tay
	plp
.elseif(n = 10505)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10506)
	; 16 bytes
	php
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
	plp
.elseif(n = 10507)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 10508) && (n < 10527))
; 10508 to 10526
.if(n = 10508)
	; 16 bytes
	php
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
	plp
.elseif(n = 10509)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #86
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 10510)
	; 16 bytes
	php
	tya
	pha
	ldy #39
	lda #235
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10511)
	; 16 bytes
	php
	tya
	pha
	ldy #244
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10512)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #1
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10513)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-125)/106
	lda #72
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10514)
	; 16 bytes
	php
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
	plp
.elseif(n = 10515)
	; 16 bytes
	php
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
	plp
.elseif(n = 10516)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10517)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10518 || n = 10520 || n = 10523 || n = 10525 || n = 10526)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 10519 || n = 10524)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10521)
	; 7 bytes
	php
	lda #41
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10522)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 10527 to 10546
.if(n = 10527 || n = 10534)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10528)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10529 || n = 10530 || n = 10532 || n = 10537)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10531 || n = 10543)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 10533)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10535)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10536)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 10538)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 10539)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10540)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10541)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10542)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10544)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10545)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10546)
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.endif
.else
; 10547 to 10625
.if((n > 10546 && n < 10550))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 10549 && n < 10555) || (n > 10555 && n < 10560) || (n > 10560 && n < 10626))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10555)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10560)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 10704) && (n < 10743))
; 10704 to 10742
.if((n > 10703 && n < 10722))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10722 || n = 10723)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10724)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 10724 && n < 10729))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-274)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10730)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-275)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10731)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10732)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10733)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10734)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10735)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10736)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10737)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10738)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10739)
	; 16 bytes
	php
	tya
	pha
	ldy #160
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10740)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10741)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10742)
	; 16 bytes
	php
	tya
	ldy #185
	pha
	lda #17
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
.if((n >= 10743) && (n < 10762))
; 10743 to 10761
.if(n = 10743)
	; 16 bytes
	php
	tya
	ldy #219
	pha
	lda #8
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10744)
	; 16 bytes
	php
	tya
	pha
	ldy #195
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10745)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10746)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10747)
	; 16 bytes
	php
	tya
	ldy #95
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10748)
	; 16 bytes
	php
	tya
	ldy #244
	pha
	lda #3
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10749)
	; 16 bytes
	php
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
	plp
.elseif(n = 10750)
	; 16 bytes
	php
	tya
	ldy #91
	pha
	lda #77
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10751)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10752)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10753)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10754)
	; 16 bytes
	php
	tya
	pha
	ldy #95
	lda #79
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10755)
	; 15 bytes
	php
	tya
	pha
	ldy #244
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 10756)
	; 16 bytes
	php
	tya
	ldy #79
	pha
	lda #95
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10757)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 10758)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #13
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10759)
	; 16 bytes
	php
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
	plp
.elseif(n = 10760)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10761)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/131
	lda #97
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 10762 to 10781
.if(n = 10762)
	; 16 bytes
	php
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
	plp
.elseif(n = 10763)
	; 16 bytes
	php
	tya
	pha
	ldy #158
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10764)
	; 16 bytes
	php
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
	plp
.elseif(n = 10765)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10766)
	; 16 bytes
	php
	tya
	ldy #37
	pha
	lda #242
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10767)
	; 16 bytes
	php
	tya
	ldy #239
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10768 || n = 10774 || n = 10776 || n = 10779 || n = 10781)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10769)
	; 16 bytes
	php
	tya
	pha
	ldy #250
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10770)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #25
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10771)
	; 16 bytes
	php
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
	plp
.elseif(n = 10772)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10773)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 10775 || n = 10780)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10777)
	; 7 bytes
	php
	lda #42
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10778)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.endif
.else
; 10782 to 10937
.if(n = 10782 || n = 10785 || n = 10786 || n = 10788 || n = 10793)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10783 || n = 10790)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10784)
	; 9 bytes
	php
	pha
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10787 || n = 10799)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 10789)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10791)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10792)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 10794)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 10795)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10796)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10797)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10798)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10800)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10801)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 10801 && n < 10806))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 10805 && n < 10811) || (n > 10811 && n < 10816) || (n > 10816 && n < 10938))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10811)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 10816)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if((n > 10937 && n < 10977))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 10977) && (n < 10996))
; 10977 to 10995
.if(n = 10977)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10978 || n = 10979)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10980)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 10980 && n < 10985))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-275)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10986)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-276)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10987)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10988)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10989)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10990)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10991)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10992)
	; 16 bytes
	php
	tya
	ldy #244
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10993)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #84
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10994)
	; 16 bytes
	php
	tya
	ldy #37
	pha
	lda #248
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10995)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10997)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10998)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 10999)
	; 16 bytes
	php
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
	plp
.elseif(n = 11000)
	; 16 bytes
	php
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
	plp
.elseif(n = 11001)
	; 16 bytes
	php
	tya
	ldy #(n-123)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11002)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #29
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11003)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11004)
	; 16 bytes
	php
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
	plp
.elseif(n = 11005)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11006)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11007)
	; 16 bytes
	php
	tya
	pha
	ldy #164
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11008)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-130)/111
	lda #77
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11009)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11010)
	; 16 bytes
	php
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
	plp
.elseif(n = 11011)
	; 16 bytes
	php
	tya
	pha
	ldy #229
	lda #14
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11012)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #3
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11013)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11014)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11015)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #11
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
; 11016 to 11054
.if(n = 11016)
	; 16 bytes
	php
	tya
	ldy #(n-12)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11017)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #83
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11018)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #157
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 11019)
	; 15 bytes
	php
	tya
	pha
	ldy #250
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 11020)
	; 16 bytes
	php
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
	plp
.elseif(n = 11021 || n = 11029 || n = 11030 || n = 11032 || n = 11035 || n = 11037 || n = 11038 || n = 11041 || n = 11042 || n = 11044 || n = 11049)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11022 || n = 11031 || n = 11036 || n = 11043)
	; 8..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 11023)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/131
	lda #97
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11024)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11025)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #92
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 11026)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #110
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 11027)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11028)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11033)
	; 7 bytes
	php
	lda #43
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11034 || n = 11039 || n = 11046)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11040)
	; 9 bytes
	php
	pha
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11045)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11047)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11048)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 11050)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 11051)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11052)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11053)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11054)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 11055 to 11093
.if(n = 11055)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 11056)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11057)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 11057 && n < 11062))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 11061 && n < 11067) || (n > 11067 && n < 11072) || (n > 11072 && n < 11094))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11067)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11072)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
; 11094 to 11250
.if((n > 11093 && n < 11234))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11234 || n = 11235)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11236)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 11236 && n < 11241))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-276)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11242)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-277)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11243)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11244)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11245)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11246)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11247)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11248)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11249)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11250)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #238
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
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
	plp
.elseif(n = 11252)
	; 16 bytes
	php
	tya
	pha
	ldy #239
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11253)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11254)
	; 16 bytes
	php
	tya
	pha
	ldy #107
	lda #71
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11255)
	; 16 bytes
	php
	tya
	pha
	ldy #212
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11256)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11257)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11258)
	; 16 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #253
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	plp
.elseif(n = 11259)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-300)/281
	lda #247
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11260)
	; 16 bytes
	php
	tya
	ldy #152
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11261)
	; 16 bytes
	php
	tya
	pha
	ldy #154
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11262)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11263)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #50
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 11264)
	; 16 bytes
	php
	tya
	ldy #96
	pha
	lda #75
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11265)
	; 16 bytes
	php
	tya
	ldy #(n-12)/121
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11266)
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11267)
	; 16 bytes
	php
	tya
	pha
	ldy #152
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11268)
	; 16 bytes
	php
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
	plp
.elseif(n = 11269)
	; 16 bytes
	php
	tya
	pha
	ldy #90
	lda #91
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 11270 to 11289
.if(n = 11270)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #57
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 11271)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-135)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11272)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11273)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #165
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 11274)
	; 17 bytes
	php
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11275)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11276)
	; 16 bytes
	php
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
	plp
.elseif(n = 11277)
	; 17 bytes
	php
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11278)
	; 16 bytes
	php
	tya
	ldy #(n-143)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11279)
	; 16 bytes
	php
	tya
	ldy #58 ;hides 'nop'
	pha
	lda #148
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	plp
.elseif(n = 11280)
	; 16 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 11281)
	; 16 bytes
	php
	tya
	ldy #191
	pha
	lda #18
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11282)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11283)
	; 15 bytes
	php
	tya
	pha
	ldy #0
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 11284)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11285 || n = 11286 || n = 11288)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 11287)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11289)
	; 7 bytes
	php
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 11290 to 11328
.if(n = 11290 || n = 11295 || n = 11302)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11291 || n = 11293 || n = 11294 || n = 11297 || n = 11298 || n = 11300 || n = 11305)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11292 || n = 11299 || n = 11311)
	; 8..12 bytes
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
.elseif(n = 11301)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11303)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11304)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 11306)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 11307)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11308)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11309)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11310)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11312)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11313)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 11313 && n < 11318))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 11317 && n < 11323) || (n > 11323 && n < 11328))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11323)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11328)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 11329 to 11406
.if((n > 11328 && n < 11407))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
; 11407 to 11484
.if((n > 11406 && n < 11485))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 11485) && (n < 11524))
.if((n >= 11485) && (n < 11504))
; 11485 to 11503
.if((n > 11484 && n < 11490))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11490 || n = 11491)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11492)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 11492 && n < 11497))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-277)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11498)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-278)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11499)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11500)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11501)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11502)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11503)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 11504 to 11523
.if(n = 11504)
	; 16 bytes
	php
	tya
	ldy #221
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11505)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 11506)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11507)
	; 16 bytes
	php
	tya
	ldy #(n-12)/121
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11508)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11509)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11510)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11511)
	; 16 bytes
	php
	tya
	pha
	ldy #221
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11512)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11513)
	; 16 bytes
	php
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
	plp
.elseif(n = 11514)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11515)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11516)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11517)
	; 15 bytes
	php
	tya
	pha
	ldy #250 ;hides 'nop'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11518)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11519)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11520)
	; 16 bytes
	php
	tya
	ldy #84
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11521)
	; 16 bytes
	php
	tya
	pha
	ldy #213
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11522)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11523)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 11524) && (n < 11543))
; 11524 to 11542
.if(n = 11524)
	; 16 bytes
	php
	tya
	pha
	ldy #195
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11525)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #238
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 11526)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	lda #16
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11527)
	; 16 bytes
	php
	tya
	ldy #245
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11528)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11529 || n = 11542)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11530)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #117
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 11531)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #166
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 11532)
	; 16 bytes
	php
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
	plp
.elseif(n = 11533)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11534)
	; 16 bytes
	php
	tya
	pha
	ldy #245
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11535)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11536)
	; 16 bytes
	php
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
	plp
.elseif(n = 11537)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #124
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 11538)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 11539)
	; 16 bytes
	php
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
	plp
.elseif(n = 11540)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11541)
	; 16 bytes
	php
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
	plp
.endif
.else
; 11543 to 11562
.if(n = 11543 || n = 11548 || n = 11555)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 11544 || n = 11547 || n = 11549 || n = 11550 || n = 11553 || n = 11554 || n = 11556 || n = 11561)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11545)
	; 7 bytes
	php
	lda #45
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11546 || n = 11551 || n = 11558)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11552)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11557)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11559)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11560)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 11562)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
; 11563 to 11718
.if(n = 11563)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11564)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11565)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11566)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11567)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 11568)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11569)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 11569 && n < 11574))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 11573 && n < 11579) || (n > 11579 && n < 11584) || (n > 11584 && n < 11719))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11579)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11584)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 11719) && (n < 11797))
.if((n >= 11719) && (n < 11758))
; 11719 to 11757
.if((n > 11718 && n < 11746))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11746 || n = 11747)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11748)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 11748 && n < 11753))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-278)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11754)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-279)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11755)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11756)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11757)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 11758) && (n < 11777))
; 11758 to 11776
.if(n = 11758)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11759)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11760)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #91
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11761)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11762)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11763)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11764)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11765)
	; 16 bytes
	php
	tya
	ldy #161
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11766)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11767)
	; 16 bytes
	php
	tya
	pha
	ldy #88
	lda #98
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11768)
	; 16 bytes
	php
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
	plp
.elseif(n = 11769)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #91
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11770)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #167
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 11771)
	; 16 bytes
	php
	tya
	pha
	ldy #226
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11772)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11773)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11774)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11775)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11776)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #27
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 11777 to 11796
.if(n = 11777)
	; 16 bytes
	php
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
	plp
.elseif(n = 11778)
	; 16 bytes
	php
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
	plp
.elseif(n = 11779)
	; 16 bytes
	php
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
	plp
.elseif(n = 11780)
	; 16 bytes
	php
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
	plp
.elseif(n = 11781)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11782)
	; 16 bytes
	php
	tya
	ldy #214
	pha
	lda #14
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11783)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11784)
	; 16 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	Jne *-9
	tax
	plp
.elseif(n = 11785)
	; 16 bytes
	php
	tya
	pha
	ldy #222
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11786)
	; 16 bytes
	php
	tya
	pha
	ldy #40
	lda #253
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11787)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11788)
	; 16 bytes
	php
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
	plp
.elseif(n = 11789)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11790)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #(n-270)/256
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11791)
	; 16 bytes
	php
	tya
	pha
	ldy #218
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11792)
	; 16 bytes
	php
	tya
	ldy #95
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11793)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 11794)
	; 16 bytes
	php
	tya
	ldy #85
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11795)
	; 16 bytes
	php
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
	plp
.elseif(n = 11796)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.endif
.else
; 11797 to 11875
.if(n = 11797)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11798 || n = 11800 || n = 11803 || n = 11805 || n = 11806 || n = 11809 || n = 11810 || n = 11812 || n = 11817)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11799 || n = 11804 || n = 11811 || n = 11823)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 11801)
	; 7 bytes
	php
	lda #46
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11802 || n = 11807 || n = 11814)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11808)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11813)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11815)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11816)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 11818)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 11819)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11820)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11821)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11822)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11824)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11825)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 11825 && n < 11830))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 11829 && n < 11835) || (n > 11835 && n < 11840) || (n > 11840 && n < 11876))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 11835)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 11840)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 11876) && (n < 12188))
.if((n >= 11876) && (n < 12032))
.if((n >= 11876) && (n < 11954))
; 11876 to 11953
.if((n > 11875 && n < 11954))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 11954) && (n < 11993))
; 11954 to 11992
.if((n > 11953 && n < 11993))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 11993) && (n < 12012))
; 11993 to 12011
.if((n > 11992 && n < 12002))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12002 || n = 12003)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12004)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 12004 && n < 12009))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-279)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12010)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-280)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12011)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 12012 to 12031
.if(n = 12012)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12013)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12014)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12015)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12016)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #95
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12017)
	; 16 bytes
	php
	tya
	ldy #245
	pha
	lda #8
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12018)
	; 16 bytes
	php
	tya
	ldy #86
	pha
	lda #97
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12019)
	; 16 bytes
	php
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
	plp
.elseif(n = 12020)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #35
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12021)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12022)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12023)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #51
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12024)
	; 16 bytes
	php
	tya
	ldy #91
	pha
	lda #91
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12025)
	; 16 bytes
	php
	tya
	pha
	ldy #207
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12026)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12027)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #94
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12028)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12029)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12030)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12031)
	; 16 bytes
	php
	tya
	pha
	ldy #91
	lda #98
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
; 12032 to 12070
.if(n = 12032)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #99
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 12033)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12034)
	; 16 bytes
	php
	tya
	pha
	ldy #135
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12035)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12036)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12037)
	; 16 bytes
	php
	tya
	ldy #185
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12038)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12039)
	; 16 bytes
	php
	tya
	ldy #211
	pha
	lda #16
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12040)
	; 16 bytes
	php
	tya
	ldy #96
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12041)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #131
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12042 || n = 12049 || n = 12050 || n = 12054 || n = 12056 || n = 12059 || n = 12061 || n = 12062 || n = 12065 || n = 12066 || n = 12068)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12043)
	; 16 bytes
	php
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
	plp
.elseif(n = 12044)
	; 16 bytes
	php
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
	plp
.elseif(n = 12045)
	; 16 bytes
	php
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
	plp
.elseif(n = 12046)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12047)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #90
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12048)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 12051)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12052)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12053)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #66
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12055 || n = 12060 || n = 12067)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12057)
	; 7 bytes
	php
	lda #47
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12058 || n = 12063 || n = 12070)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12064)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12069)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.else
; 12071 to 12109
.if(n = 12071)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12072)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 12073)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12074)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 12075)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12076)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12077)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12078)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12079)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 12080)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12081)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 12081 && n < 12086))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 12085 && n < 12091) || (n > 12091 && n < 12096) || (n > 12096 && n < 12110))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12091)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12096)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 12110 to 12187
.if((n > 12109 && n < 12188))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
; 12188 to 12265
.if((n > 12187 && n < 12258))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12258 || n = 12259)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12260)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 12260 && n < 12265))
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
	delay_n 5
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
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-281)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12267)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12268)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12269)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12270)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12271)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12272)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #179
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12273)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12274)
	; 16 bytes
	php
	tya
	pha
	ldy #95
	lda #95
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12275)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #191
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12276)
	; 16 bytes
	php
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
	plp
.elseif(n = 12277)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12278)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12279)
	; 16 bytes
	php
	tya
	ldy #141
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12280)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12281)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #183
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12282)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #75
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 12283)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12284)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 12285 to 12304
.if(n = 12285)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12286)
	; 16 bytes
	php
	tya
	pha
	ldy #141
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12287)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12288)
	; 16 bytes
	php
	tya
	ldy #98
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12289)
	; 16 bytes
	php
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
	plp
.elseif(n = 12290)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12291)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12292)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #95
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 12293)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12294)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #97
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12295)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #90
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12296)
	; 16 bytes
	php
	tya
	ldy #166
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12297)
	; 16 bytes
	php
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
	plp
.elseif(n = 12298)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12299)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12300)
	; 16 bytes
	php
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
	plp
.elseif(n = 12301)
	; 16 bytes
	php
	tya
	pha
	ldy #178
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12302)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12303)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 12304)
	; 16 bytes
	php
	tya
	pha
	ldy #195
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
; 12305 to 12343
.if(n = 12305 || n = 12310 || n = 12312 || n = 12315 || n = 12317 || n = 12318 || n = 12321 || n = 12322 || n = 12324 || n = 12329)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 12306)
	; 15 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	iny
	Jne *-8
	tay
	plp
.elseif(n = 12307)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #14
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12308)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12309 || n = 12311 || n = 12316 || n = 12323 || n = 12335)
	; 8..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 12313)
	; 7 bytes
	php
	lda #48
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12314 || n = 12319 || n = 12326)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12320)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12325)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12327)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12328)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 12330)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 12331)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12332)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12333)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12334)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12336)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12337)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 12337 && n < 12342))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12342 || n = 12343)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
; 12344 to 12500
.if((n > 12343 && n < 12347) || (n > 12347 && n < 12352) || (n > 12352 && n < 12501))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12347)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12352)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
.if((n > 12500 && n < 12514))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12514 || n = 12515)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12516)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 12516 && n < 12521))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-281)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12522)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-282)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12523)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12524)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12525)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12526)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12527)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12528)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #43
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12529)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12530)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #255
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12531)
	; 16 bytes
	php
	tya
	pha
	ldy #184
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12532)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #95
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12533)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12534)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12535)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #50
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12536)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12537)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12538)
	; 16 bytes
	php
	tya
	pha
	ldy #117
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12539)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 12540) && (n < 12559))
; 12540 to 12558
.if(n = 12540)
	; 16 bytes
	php
	tya
	ldy #232
	pha
	lda #13
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12541)
	; 16 bytes
	php
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
	plp
.elseif(n = 12542)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #29
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12543)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12544)
	; 16 bytes
	php
	tya
	pha
	ldy #167
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12545)
	; 16 bytes
	php
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
	plp
.elseif(n = 12546)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12547)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12548)
	; 16 bytes
	php
	tya
	pha
	ldy #187
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12549)
	; 16 bytes
	php
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
	plp
.elseif(n = 12550)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_b_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 12551)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12552)
	; 16 bytes
	php
	tya
	ldy #95
	pha
	lda #91
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12553)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 12554)
	; 16 bytes
	php
	tya
	pha
	ldy #114
	lda #75
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12555)
	; 16 bytes
	php
	tya
	ldy #(n-123)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12556)
	; 16 bytes
	php
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
	plp
.elseif(n = 12557)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12558)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.endif
.else
; 12559 to 12578
.if(n = 12559)
	; 16 bytes
	php
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
	plp
.elseif(n = 12560 || n = 12566 || n = 12568 || n = 12571 || n = 12573 || n = 12574 || n = 12577 || n = 12578)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 12561)
	; 16 bytes
	php
	tya
	ldy #141
	pha
	lda #(n-273)/256
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12562)
	; 16 bytes
	php
	tya
	ldy #251
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12563)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12564)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12565)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #17
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12567 || n = 12572)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12569)
	; 7 bytes
	php
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12570 || n = 12575)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.if(n = 12579 || n = 12591)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 12580 || n = 12585)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12581)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12582)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12583)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12584)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 12586)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 12587)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12588)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12589)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12590)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12592)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12593)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 12593 && n < 12598))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 12597 && n < 12603) || (n > 12603 && n < 12608) || (n > 12608 && n < 12657))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12603)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12608)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
; 12657 to 12734
.if((n > 12656 && n < 12735))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 12735) && (n < 12774))
; 12735 to 12773
.if((n > 12734 && n < 12770))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12770 || n = 12771)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12772)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 12773)
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-282)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12778)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-283)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12779)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12780)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12781)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12782)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12783)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12784)
	; 16 bytes
	php
	tya
	pha
	ldy #185
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12785)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12786)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12787)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12788)
	; 16 bytes
	php
	tya
	pha
	ldy #112
	lda #79
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12789)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12790)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #95
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12791)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12792)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 12793 to 12812
.if(n = 12793)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12794)
	; 16 bytes
	php
	tya
	pha
	ldy #175
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12795)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12796)
	; 16 bytes
	php
	tya
	ldy #93
	pha
	lda #95
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12797)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12798)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12799)
	; 16 bytes
	php
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
	plp
.elseif(n = 12800)
	; 16 bytes
	php
	tya
	ldy #91
	pha
	lda #98
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12801)
	; 16 bytes
	php
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
	plp
.elseif(n = 12802)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 12803)
	; 16 bytes
	php
	tya
	pha
	ldy #188
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12804)
	; 16 bytes
	php
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
	plp
.elseif(n = 12805)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12806)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12807)
	; 16 bytes
	php
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
	plp
.elseif(n = 12808)
	; 16 bytes
	php
	tya
	pha
	ldy #203
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12809)
	; 16 bytes
	php
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
	plp
.elseif(n = 12810)
	; 16 bytes
	php
	tya
	ldy #237
	pha
	lda #13
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12811)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12812)
	; 16 bytes
	php
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
; 12813 to 12851
.if(n = 12813)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 12814)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12815 || n = 12818 || n = 12822 || n = 12824 || n = 12827 || n = 12829 || n = 12830 || n = 12833 || n = 12834 || n = 12836 || n = 12841)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12816)
	; 16 bytes
	php
	tya
	ldy #96
	pha
	lda #91
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12817)
	; 16 bytes
	php
	tya
	pha
	ldy #237
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12819)
	; 16 bytes
	php
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
	plp
.elseif(n = 12820)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12821)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12823 || n = 12828 || n = 12835 || n = 12847)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 12825)
	; 7 bytes
	php
	lda #50
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12826 || n = 12831 || n = 12838)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12832)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12837)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12839)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12840)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 12842)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 12843)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12844)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12845)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12846)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12848)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12849)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12850 || n = 12851)
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.else
; 12852 to 12890
.if(n = 12852 || n = 12853)
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 12853 && n < 12859) || (n > 12859 && n < 12864) || (n > 12864 && n < 12891))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 12859)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 12864)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 12891 to 12968
.if((n > 12890 && n < 12969))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 12969) && (n < 13047))
; 12969 to 13046
.if((n > 12968 && n < 13026))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13026 || n = 13027)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13028)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 13028 && n < 13033))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-283)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13034)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-284)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13035)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13036)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13037)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13038)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13039)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13040)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13041)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13042)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #138
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 13043)
	; 16 bytes
	php
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
	plp
.elseif(n = 13044)
	; 16 bytes
	php
	tya
	ldy #181
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13045)
	; 16 bytes
	php
	tya
	pha
	ldy #167
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13046)
	; 16 bytes
	php
	tya
	ldy #98
	pha
	lda #92
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
.if((n >= 13047) && (n < 13086))
.if((n >= 13047) && (n < 13066))
; 13047 to 13065
.if(n = 13047)
	; 16 bytes
	php
	tya
	ldy #237
	pha
	lda #14
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13048)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #67
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13049)
	; 16 bytes
	php
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
	plp
.elseif(n = 13050)
	; 16 bytes
	php
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
	plp
.elseif(n = 13051)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13052)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13053)
	; 16 bytes
	php
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
	plp
.elseif(n = 13054)
	; 16 bytes
	php
	tya
	pha
	ldy #237
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13055)
	; 16 bytes
	php
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
	plp
.elseif(n = 13056)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13057)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13058)
	; 16 bytes
	php
	tya
	pha
	ldy #221
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13059)
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13060)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13061)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13062)
	; 16 bytes
	php
	tya
	ldy #225
	pha
	lda #17
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13063)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #186
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	plp
.elseif(n = 13064)
	; 16 bytes
	php
	tya
	ldy #251
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13065)
	; 16 bytes
	php
	tya
	ldy #229
	pha
	lda #16
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13067)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13068)
	; 15 bytes
	php
	tya
	ldy #0
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 13069)
	; 16 bytes
	php
	tya
	pha
	ldy #225
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13070 || n = 13078 || n = 13080 || n = 13083 || n = 13085)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 13071)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13072)
	; 16 bytes
	php
	tya
	pha
	ldy #229
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13073)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13074)
	; 16 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 13075)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #17
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13076)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13077)
	; 16 bytes
	php
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
	plp
.elseif(n = 13079 || n = 13084)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13081)
	; 7 bytes
	php
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13082)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086 || n = 13089 || n = 13090 || n = 13092 || n = 13097)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13087 || n = 13094)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13088)
	; 9 bytes
	php
	pha
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13091 || n = 13103)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 13093)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13095)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13096)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 13098)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 13099)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13100)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13101)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13102)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13104)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13105)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 13105 && n < 13110))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 13109 && n < 13115) || (n > 13115 && n < 13120) || (n > 13120 && n < 13126))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13115)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13120)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
.if((n > 13125 && n < 13282))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 13282) && (n < 13360))
.if((n >= 13282) && (n < 13321))
.if((n >= 13282) && (n < 13301))
; 13282 to 13300
.if(n = 13282 || n = 13283)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13284)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 13284 && n < 13289))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-284)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13290)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-285)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13291)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13292)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13293)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13294)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13295)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13296)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13297)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13298)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13299)
	; 16 bytes
	php
	tya
	pha
	ldy #166
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13300)
	; 16 bytes
	php
	tya
	pha
	ldy #233
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 13301 to 13320
.if(n = 13301)
	; 16 bytes
	php
	tya
	ldy #96
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13302)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13303)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13304)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13305)
	; 16 bytes
	php
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
	plp
.elseif(n = 13306)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #69
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13307)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13308)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #63
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13309)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13310)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13311)
	; 16 bytes
	php
	tya
	ldy #143
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13312)
	; 16 bytes
	php
	tya
	ldy #95
	pha
	lda #99
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13313)
	; 16 bytes
	php
	tya
	pha
	ldy #45
	lda #255
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13314)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13315)
	; 16 bytes
	php
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
	plp
.elseif(n = 13316)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13317)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13318)
	; 16 bytes
	php
	tya
	pha
	ldy #143
	lda #59
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13319)
	; 16 bytes
	php
	tya
	pha
	ldy #190
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13320)
	; 16 bytes
	php
	tya
	pha
	ldy #46
	lda #249
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.endif
.endif
.else
; 13321 to 13359
.if(n = 13321 || n = 13334 || n = 13336 || n = 13339 || n = 13341 || n = 13342 || n = 13345 || n = 13346 || n = 13348 || n = 13353)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13322)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13323)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13324)
	; 16 bytes
	php
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
	plp
.elseif(n = 13325)
	; 17 bytes
	php
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13326)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13327)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #74
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 13328)
	; 17 bytes
	php
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
	plp
.elseif(n = 13329)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13330)
	; 16 bytes
	php
	tya
	pha
	ldy #153
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13331)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13332)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13333)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #36
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13335 || n = 13340 || n = 13347 || n = 13359)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 13337)
	; 7 bytes
	php
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13338 || n = 13343 || n = 13350)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13344)
	; 9 bytes
	php
	pha
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13349)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13351)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13352)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 13354)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 13355)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13356)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13357)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13358)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 13360 to 13437
.if(n = 13360)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13361)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 13361 && n < 13366))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 13365 && n < 13371) || (n > 13371 && n < 13376) || (n > 13376 && n < 13438))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13371)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13376)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
; 13438 to 13515
.if((n > 13437 && n < 13516))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 13516) && (n < 13555))
; 13516 to 13554
.if((n > 13515 && n < 13538))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13538 || n = 13539)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13540)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 13540 && n < 13545))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-285)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13546)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-286)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13547)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13548)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13549)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13550)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13551)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13552)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13553)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13554)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
.if((n >= 13555) && (n < 13574))
; 13555 to 13573
.if(n = 13555)
	; 16 bytes
	php
	tya
	pha
	ldy #188
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13556)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13557)
	; 16 bytes
	php
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
	plp
.elseif(n = 13558)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13559)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #147
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 13560)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13561)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13562)
	; 16 bytes
	php
	tya
	ldy #(n-283)/271
	pha
	lda #230
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13563)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13564)
	; 16 bytes
	php
	tya
	pha
	ldy #215
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13565)
	; 16 bytes
	php
	tya
	pha
	ldy #13
	lda #4
	jsr delay_256a_11_clocks_b_
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13566)
	; 16 bytes
	php
	tya
	ldy #251
	pha
	lda #13
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13567)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13568)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13569)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13570)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13571)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13572)
	; 16 bytes
	php
	tya
	ldy #226
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13573)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 13574 to 13593
.if(n = 13574)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13575)
	; 16 bytes
	php
	tya
	ldy #98
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13576)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #195
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	plp
.elseif(n = 13577)
	; 17 bytes
	php
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13578)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13579)
	; 16 bytes
	php
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
	plp
.elseif(n = 13580)
	; 16 bytes
	php
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
	plp
.elseif(n = 13581)
	; 17 bytes
	php
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13582)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #65
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13583)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #202
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 13584)
	; 16 bytes
	php
	tya
	ldy #234
	pha
	lda #17
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13585)
	; 16 bytes
	php
	tya
	pha
	ldy #238
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13586 || n = 13590 || n = 13592)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13587)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13588)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13589)
	; 16 bytes
	php
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
	plp
.elseif(n = 13591)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13593)
	; 7 bytes
	php
	lda #53
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.endif
.else
; 13594 to 13750
.if(n = 13594 || n = 13599 || n = 13606)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13595 || n = 13597 || n = 13598 || n = 13601 || n = 13602 || n = 13604 || n = 13609)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13596 || n = 13603 || n = 13615)
	; 8..12 bytes
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
.elseif(n = 13605)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13607)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13608)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 13610)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 13611)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13612)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13613)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13614)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13616)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13617)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 13617 && n < 13622))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 13621 && n < 13627) || (n > 13627 && n < 13632) || (n > 13632 && n < 13751))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13627)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13632)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
.if((n > 13750 && n < 13790))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 13790) && (n < 13809))
; 13790 to 13808
.if((n > 13789 && n < 13794))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13794 || n = 13795)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13796)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 13796 && n < 13801))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-286)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13802)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-287)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13803)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13804)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13805)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13806)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13807)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13808)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13810)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 13811)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #206
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 13812)
	; 16 bytes
	php
	tya
	ldy #230
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13813)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-140)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13814)
	; 16 bytes
	php
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
	plp
.elseif(n = 13815)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #66
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13816)
	; 16 bytes
	php
	tya
	pha
	ldy #219
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13817)
	; 16 bytes
	php
	tya
	ldy #251
	pha
	lda #14
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13818)
	; 16 bytes
	php
	tya
	ldy #234
	pha
	lda #18
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13819)
	; 16 bytes
	php
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
	plp
.elseif(n = 13820)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #93
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 13821)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13822)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #73
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13823)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13824)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13825)
	; 16 bytes
	php
	tya
	pha
	ldy #234
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13826)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13827)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13828)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #47
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 13829) && (n < 13868))
.if((n >= 13829) && (n < 13848))
; 13829 to 13847
.if(n = 13829)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13830)
	; 16 bytes
	php
	tya
	ldy #147
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13831)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13832)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13833)
	; 16 bytes
	php
	tya
	ldy #51
	pha
	lda #230
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13834)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #149
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 13835)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13836)
	; 16 bytes
	php
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
	plp
.elseif(n = 13837)
	; 16 bytes
	php
	tya
	pha
	ldy #147
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13838)
	; 16 bytes
	php
	tya
	ldy #223
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13839 || n = 13846)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13840)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13841)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #156
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 13842)
	; 16 bytes
	php
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
	plp
.elseif(n = 13843)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13844)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13845)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13847)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 13848 to 13867
.if(n = 13848 || n = 13851 || n = 13853 || n = 13854 || n = 13857 || n = 13858 || n = 13860 || n = 13865)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13849)
	; 7 bytes
	php
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13850 || n = 13855 || n = 13862)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13852 || n = 13859)
	; 8..10 bytes
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
.elseif(n = 13861)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13863)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13864)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 13866)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 13867)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 13868 to 13906
.if(n = 13868)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13869)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13870)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13871)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 13872)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13873)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 13873 && n < 13878))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 13877 && n < 13883) || (n > 13883 && n < 13888) || (n > 13888 && n < 13907))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 13883)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 13888)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
; 13907 to 14062
.if((n > 13906 && n < 14050))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14050 || n = 14051)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14052)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 14052 && n < 14057))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-287)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14058)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-288)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14059)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14060)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14061)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14062)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
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
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14064)
	; 16 bytes
	php
	tya
	pha
	ldy #52
	lda #231
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14065)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14066)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #208
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 14067)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14068)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14069)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-300)/281
	lda #247
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14070)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #25
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14071)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14072)
	; 16 bytes
	php
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
	plp
.elseif(n = 14073)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #68
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 14074)
	; 16 bytes
	php
	tya
	ldy #178
	pha
	lda #38
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14075)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14076)
	; 16 bytes
	php
	tya
	ldy #47
	pha
	lda #252
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 14077)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14078)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14079)
	; 16 bytes
	php
	tya
	pha
	ldy #190
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14080)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #75
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14081)
	; 16 bytes
	php
	tya
	pha
	ldy #178
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 14082 to 14101
.if(n = 14082)
	; 16 bytes
	php
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
	plp
.elseif(n = 14083)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14084)
	; 16 bytes
	php
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
	plp
.elseif(n = 14085)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14086)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14087)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14088)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #27
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14089)
	; 16 bytes
	php
	tya
	pha
	ldy #210
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14090)
	; 17 bytes
	php
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14091)
	; 16 bytes
	php
	tya
	ldy #247
	pha
	lda #16
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14092)
	; 16 bytes
	php
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
	plp
.elseif(n = 14093)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14094)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14095)
	; 16 bytes
	php
	tya
	pha
	ldy #207
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14096)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #211
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 14097)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14098)
	; 16 bytes
	php
	tya
	pha
	ldy #247
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14099)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14100)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14101)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.else
; 14102 to 14140
.if(n = 14102 || n = 14104 || n = 14107 || n = 14109 || n = 14110 || n = 14113 || n = 14114 || n = 14116 || n = 14121)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14103 || n = 14108 || n = 14115 || n = 14127)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 14105)
	; 7 bytes
	php
	lda #55
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14106 || n = 14111 || n = 14118)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14112)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14117)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14119)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14120)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 14122)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 14123)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14124)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14125)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14126)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14128)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14129)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 14129 && n < 14134))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 14133 && n < 14139) || n = 14140)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14139)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.else
; 14141 to 14218
.if((n > 14140 && n < 14144) || (n > 14144 && n < 14219))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14144)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
; 14219 to 14296
.if((n > 14218 && n < 14297))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 14297) && (n < 14336))
.if((n >= 14297) && (n < 14316))
; 14297 to 14315
.if((n > 14296 && n < 14306))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14306 || n = 14307)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14308)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 14308 && n < 14313))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-288)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14314)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-289)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14315)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 14316 to 14335
.if(n = 14316)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14317)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14318)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14319)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14320)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14321)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14322)
	; 16 bytes
	php
	tya
	ldy #159
	pha
	lda #49
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14323)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #62
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14324)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #215
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 14325)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14326)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #(n-618)/596
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14327)
	; 16 bytes
	php
	tya
	pha
	ldy #196
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14328)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14329)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14330)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14331)
	; 16 bytes
	php
	tya
	ldy #(n-123)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 14332)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14333)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14334)
	; 16 bytes
	php
	tya
	ldy #231
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14335)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 14336) && (n < 14356))
; 14336 to 14355
.if(n = 14336)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #69
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14337)
	; 16 bytes
	php
	tya
	ldy #191
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14338)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-130)/111
	lda #77
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14339)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14340)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14341)
	; 16 bytes
	php
	tya
	pha
	ldy #231
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14342)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #155
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 14343)
	; 16 bytes
	php
	tya
	ldy #(n-12)/281
	pha
	lda #240
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14344)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14345)
	; 16 bytes
	php
	tya
	pha
	ldy #247
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14346)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #213
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 14347)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14348)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14349)
	; 16 bytes
	php
	tya
	ldy #243
	pha
	lda #18
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14350)
	; 16 bytes
	php
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
	plp
.elseif(n = 14351)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14352)
	; 16 bytes
	php
	tya
	ldy #239
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14353)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #220
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 14354)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14355)
	; 16 bytes
	php
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
	plp
.endif
.else
; 14356 to 14375
.if(n = 14356)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14357)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14358 || n = 14360 || n = 14363 || n = 14365 || n = 14366 || n = 14369 || n = 14370 || n = 14372)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14359 || n = 14364 || n = 14371)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 14361)
	; 7 bytes
	php
	lda #56
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14362 || n = 14367 || n = 14374)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14368)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14373)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14375)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 14377)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14378)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 14379)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14380)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14381)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14382)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14383)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 14384)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14385)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 14385 && n < 14390))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 14389 && n < 14395) || (n > 14395 && n < 14400) || (n > 14400 && n < 14532))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14395)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14400)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
; 14532 to 14570
.if((n > 14531 && n < 14562))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14562 || n = 14563)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14564)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 14564 && n < 14569))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-289)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14570)
	; 15 bytes
	php
	delay_n 5
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
.if(n = 14571)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14572)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14573)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14574)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14575)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14576)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14577)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14578)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14579)
	; 16 bytes
	php
	tya
	pha
	ldy #224
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14580)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14581)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14582)
	; 16 bytes
	php
	tya
	ldy #235
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14583)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14584)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14585)
	; 16 bytes
	php
	tya
	ldy #247
	pha
	lda #18
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14586)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14587)
	; 16 bytes
	php
	tya
	ldy #55
	pha
	lda #224
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14588)
	; 16 bytes
	php
	tya
	ldy #7
	pha
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 14589)
	; 16 bytes
	php
	tya
	pha
	ldy #235
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 14590 to 14609
.if(n = 14590)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14591)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14592)
	; 16 bytes
	php
	tya
	pha
	ldy #247
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14593)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14594)
	; 16 bytes
	php
	tya
	pha
	ldy #55
	lda #231
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14595)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #161
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 14596)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #79
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14597)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14598)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_b_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 14599)
	; 16 bytes
	php
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
	plp
.elseif(n = 14600)
	; 16 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_b_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 14601)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 14602)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #213
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	plp
.elseif(n = 14603)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	delay_n 2080
	Jcs *-11
	plp
.elseif(n = 14604)
	; 16 bytes
	php
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
	plp
.elseif(n = 14605)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14606)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #121
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 14607)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #64
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14608)
	; 16 bytes
	php
	tya
	ldy #178
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14609)
	; 16 bytes
	php
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.endif
.endif
.endif
.else
; 14610 to 14687
.if(n = 14610)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #30
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 14611)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14612)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14613)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14614 || n = 14616 || n = 14619 || n = 14621 || n = 14622 || n = 14625 || n = 14626 || n = 14628 || n = 14633)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14615 || n = 14620 || n = 14627 || n = 14639)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 14617)
	; 7 bytes
	php
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14618 || n = 14623 || n = 14630)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14624)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14629)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14631)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14632)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 14634)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 14635)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14636)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14637)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14638)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14640)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14641)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 14641 && n < 14646))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 14645 && n < 14651) || (n > 14651 && n < 14656) || (n > 14656 && n < 14688))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14651)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14656)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
; 14688 to 14843
.if((n > 14687 && n < 14818))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14818 || n = 14819)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14820)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 14820 && n < 14825))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-290)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14826)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-291)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14827)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14828)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14829)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14830)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14831)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14832)
	; 16 bytes
	php
	tya
	ldy #247
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14833)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14834)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14835)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14836)
	; 16 bytes
	php
	tya
	ldy #218
	pha
	lda #27
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14837)
	; 16 bytes
	php
	tya
	pha
	ldy #<((n-23)/2)
	lda #>((n-23)/2)
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14838)
	; 16 bytes
	php
	tya
	pha
	ldy #203
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14839)
	; 16 bytes
	php
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
	plp
.elseif(n = 14840)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14841)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14842)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14843)
	; 16 bytes
	php
	tya
	pha
	ldy #218
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 14844) && (n < 14922))
.if((n >= 14844) && (n < 14883))
.if((n >= 14844) && (n < 14863))
; 14844 to 14862
.if(n = 14844)
	; 16 bytes
	php
	tya
	ldy #206
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14845)
	; 16 bytes
	php
	tya
	ldy #(n-12)/91
	pha
	lda #50
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14846)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14847)
	; 16 bytes
	php
	tya
	ldy #215
	pha
	lda #(n-539)/511
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14848)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14849)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #170
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 14850)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 14851)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14852)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14853)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #56
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14854)
	; 16 bytes
	php
	tya
	pha
	ldy #215
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14855)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14856)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14857)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #229
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 14858)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14859)
	; 16 bytes
	php
	tya
	pha
	ldy #212
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14860)
	; 16 bytes
	php
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
	plp
.elseif(n = 14861)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14862)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #25
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 14863 to 14882
.if(n = 14863 || n = 14870 || n = 14872 || n = 14875 || n = 14877 || n = 14878 || n = 14881 || n = 14882)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14864)
	; 16 bytes
	php
	tya
	ldy #188
	pha
	lda #38
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14865)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #28
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 14866)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14867)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-135)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14868)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14869)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14871 || n = 14876)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14873)
	; 7 bytes
	php
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14874 || n = 14879)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.if(n = 14883 || n = 14895)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 14884 || n = 14889)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14885)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14886)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14887)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14888)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 14890)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 14891)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14892)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14893)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14894)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14896)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14897)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 14897 && n < 14902))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 14901 && n < 14907) || (n > 14907 && n < 14912) || (n > 14912 && n < 14922))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 14907)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 14912)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 14922 to 15000
.if((n > 14921 && n < 15001))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
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
.if((n > 15000 && n < 15074))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15074 || n = 15075)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15076)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 15077 || n = 15078)
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-291)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15082)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-292)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15083)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15084)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15085)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15086)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15087)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15088)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15089)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #76
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15090)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 15091)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #62
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15092)
	; 16 bytes
	php
	tya
	ldy #232
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15093)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15094)
	; 16 bytes
	php
	tya
	pha
	ldy #225
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15095)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15096)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15097)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #29
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.endif
.else
; 15098 to 15117
.if(n = 15098)
	; 16 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 15099)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15100)
	; 16 bytes
	php
	tya
	ldy #184
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15101)
	; 16 bytes
	php
	tya
	ldy #191
	pha
	lda #38
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15102)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15103)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15104)
	; 16 bytes
	php
	tya
	ldy #196
	pha
	lda #36
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15105)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #76
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15106)
	; 16 bytes
	php
	tya
	pha
	ldy #141
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15107)
	; 16 bytes
	php
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
	plp
.elseif(n = 15108)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15109)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15110)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15111)
	; 16 bytes
	php
	tya
	pha
	ldy #196
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15112)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #83
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15113)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #168
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15114)
	; 16 bytes
	php
	tya
	ldy #58 ;hides 'nop'
	pha
	lda #213
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	plp
.elseif(n = 15115)
	; 16 bytes
	php
	tya
	pha
	ldy #222
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15116)
	; 16 bytes
	php
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
	plp
.elseif(n = 15117)
	; 16 bytes
	php
	tya
	ldy #159
	pha
	lda #54
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.else
; 15118 to 15156
.if(n = 15118)
	; 16 bytes
	php
	tya
	ldy #182
	pha
	lda #42
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15119)
	; 16 bytes
	php
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
	plp
.elseif(n = 15120 || n = 15126 || n = 15128 || n = 15131 || n = 15133 || n = 15134 || n = 15137 || n = 15138 || n = 15140 || n = 15145)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 15121)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #92
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15122)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 15123)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15124)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15125)
	; 16 bytes
	php
	tya
	pha
	ldy #182
	lda #49
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15127 || n = 15132 || n = 15139 || n = 15151)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 15129)
	; 7 bytes
	php
	lda #59
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15130 || n = 15135 || n = 15142)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15136)
	; 9 bytes
	php
	pha
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15141)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15143)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15144)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 15146)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 15147)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15148)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15149)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15150)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15152)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15153)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 15153 && n < 15157))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.endif
.else
; 15157 to 15312
.if(n = 15157)
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 15157 && n < 15163) || (n > 15163 && n < 15168) || (n > 15168 && n < 15313))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15163)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15168)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 15313) && (n < 15469))
.if((n >= 15313) && (n < 15391))
.if((n >= 15313) && (n < 15352))
; 15313 to 15351
.if((n > 15312 && n < 15330))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15330 || n = 15331)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15332)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 15332 && n < 15337))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-292)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15338)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-293)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15339)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15340)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15341)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15342)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15343)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15344)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15345)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15346)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #83
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15347)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15348)
	; 16 bytes
	php
	tya
	ldy #(n-12)/71
	pha
	lda #30
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15349)
	; 16 bytes
	php
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
	plp
.elseif(n = 15350)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #233
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 15351)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 15352) && (n < 15371))
; 15352 to 15370
.if(n = 15352)
	; 16 bytes
	php
	tya
	pha
	ldy #57
	lda #235
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15353)
	; 16 bytes
	php
	tya
	pha
	ldy #187
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15354)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #231
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 15355)
	; 16 bytes
	php
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
	plp
.elseif(n = 15356)
	; 16 bytes
	php
	tya
	ldy #137
	pha
	lda #71
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15357)
	; 16 bytes
	php
	tya
	ldy #165
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15358)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15359)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15360)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15361)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #238
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15362)
	; 16 bytes
	php
	tya
	pha
	ldy #229
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15363)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #78
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15364)
	; 16 bytes
	php
	tya
	pha
	ldy #165
	lda #59
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15365)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15366)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #69
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15367)
	; 16 bytes
	php
	tya
	ldy #185
	pha
	lda #42
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15368)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15369)
	; 16 bytes
	php
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #65
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15370)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #85
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.endif
.else
; 15371 to 15390
.if(n = 15371)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #42
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15372)
	; 16 bytes
	php
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
	plp
.elseif(n = 15373 || n = 15382 || n = 15384 || n = 15387 || n = 15389 || n = 15390)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 15374)
	; 16 bytes
	php
	tya
	pha
	ldy #185
	lda #49
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15375)
	; 17 bytes
	php
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
	plp
.elseif(n = 15376)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #51
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15377)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #84
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 15378)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #37
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15379)
	; 16 bytes
	php
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
	plp
.elseif(n = 15380)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15381)
	; 16 bytes
	php
	tya
	ldy #141
	pha
	lda #68
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15383 || n = 15388)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15385)
	; 7 bytes
	php
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15386)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
; 15391 to 15468
.if(n = 15391 || n = 15398)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15392)
	; 9 bytes
	php
	pha
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15393 || n = 15394 || n = 15396 || n = 15401)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15395 || n = 15407)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 15397)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15399)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15400)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 15402)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 15403)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15404)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15405)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15406)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15408)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15409)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 15409 && n < 15414))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 15413 && n < 15419) || (n > 15419 && n < 15424) || (n > 15424 && n < 15469))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15419)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15424)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 15469) && (n < 15547))
; 15469 to 15546
.if((n > 15468 && n < 15547))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if((n > 15546 && n < 15586))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 15586) && (n < 15606))
; 15586 to 15605
.if(n = 15586 || n = 15587)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15588)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 15588 && n < 15593))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-293)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15594)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-294)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15595)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15596)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15597)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15598)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15599)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15600)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15601)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #64
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15602)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15603)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15604)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15605)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 15606 to 15625
.if(n = 15606)
	; 16 bytes
	php
	tya
	pha
	ldy #143
	lda #75
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15607)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15608)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #85
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15609)
	; 16 bytes
	php
	tya
	pha
	ldy #9
	lda #6
	jsr delay_256a_16_clocks_b
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15610)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #35
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 15611)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #77
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15612)
	; 16 bytes
	php
	tya
	ldy #240
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15613)
	; 16 bytes
	php
	tya
	pha
	ldy #226
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15614)
	; 16 bytes
	php
	tya
	ldy #57
	pha
	lda #228
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15615)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15616)
	; 16 bytes
	php
	tya
	ldy #188
	pha
	lda #42
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15617)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #42
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15618)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #61
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15619)
	; 16 bytes
	php
	tya
	pha
	ldy #240
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15620)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15621)
	; 16 bytes
	php
	tya
	pha
	ldy #57
	lda #235
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15622)
	; 16 bytes
	php
	tya
	ldy #223
	pha
	lda #29
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15623)
	; 16 bytes
	php
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
	plp
.elseif(n = 15624)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15625)
	; 16 bytes
	php
	tya
	pha
	ldy #153
	lda #68
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
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
	; 16 bytes
	php
	tya
	ldy #211
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15627)
	; 16 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #6
	jsr delay_256a_16_clocks_b
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15628)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-140)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15629)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15630)
	; 16 bytes
	php
	tya
	pha
	ldy #233
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15631)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #175
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15632)
	; 16 bytes
	php
	tya
	ldy #(n-12)/71
	pha
	lda #30
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15633)
	; 16 bytes
	php
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
	plp
.elseif(n = 15634)
	; 16 bytes
	php
	tya
	ldy #214
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15635)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15636)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15637)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #80
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15638 || n = 15640 || n = 15643)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 15639 || n = 15644)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15641)
	; 7 bytes
	php
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15642)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 15645 to 15664
.if(n = 15645 || n = 15646 || n = 15649 || n = 15650 || n = 15652 || n = 15657)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15647 || n = 15654)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15648)
	; 9 bytes
	php
	pha
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15651 || n = 15663)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 15653)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15655)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15656)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 15658)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 15659)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15660)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15661)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15662)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15664)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 15665 to 15703
.if(n = 15665)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 15665 && n < 15670))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 15669 && n < 15675) || (n > 15675 && n < 15680) || (n > 15680 && n < 15704))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15675)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15680)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 15704 to 15781
.if((n > 15703 && n < 15782))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 15782) && (n < 15860))
; 15782 to 15859
.if((n > 15781 && n < 15842))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15842 || n = 15843)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15844)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 15844 && n < 15849))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-294)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15850)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-295)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15851)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15852)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15853)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15854)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15855)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15856)
	; 16 bytes
	php
	tya
	ldy #233
	pha
	lda #27
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15857)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #184
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 15858)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #73
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15859)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 15860) && (n < 15899))
.if((n >= 15860) && (n < 15879))
; 15860 to 15878
.if(n = 15860)
	; 16 bytes
	php
	tya
	pha
	ldy #217
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15861)
	; 16 bytes
	php
	tya
	pha
	ldy #178
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15862)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15863)
	; 16 bytes
	php
	tya
	pha
	ldy #233
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15864)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15865)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #80
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15866)
	; 15 bytes
	php
	tya
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldy #136 ;hides 'dey'
	Jne *-1
	Jcs *-6
	pla
	tay
	plp
.elseif(n = 15867)
	; 16 bytes
	php
	tya
	pha
	ldy #55
	lda #249
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15868)
	; 16 bytes
	php
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
	plp
.elseif(n = 15869)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15870)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15871)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15872)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #49
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15873)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15874)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #71
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15875)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15876)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15877)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #54
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15878)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 15879 to 15898
.if(n = 15879)
	; 16 bytes
	php
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
	plp
.elseif(n = 15880)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15881)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15882)
	; 16 bytes
	php
	tya
	ldy #230
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15883)
	; 16 bytes
	php
	tya
	ldy #58
	pha
	lda #228
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15884)
	; 16 bytes
	php
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
	plp
.elseif(n = 15885)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15886)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #89
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15887)
	; 16 bytes
	php
	tya
	ldy #126
	pha
	lda #84
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15888)
	; 16 bytes
	php
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
	plp
.elseif(n = 15889)
	; 16 bytes
	php
	tya
	pha
	ldy #230
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15890)
	; 16 bytes
	php
	tya
	pha
	ldy #58
	lda #235
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15891)
	; 16 bytes
	php
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
	plp
.elseif(n = 15892)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15893)
	; 17 bytes
	php
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15894 || n = 15896)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 15895)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15897)
	; 7 bytes
	php
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15898)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 15899 to 15937
.if(n = 15899 || n = 15901 || n = 15902 || n = 15905 || n = 15906 || n = 15908 || n = 15913)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15900 || n = 15907 || n = 15919)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 15903 || n = 15910)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15904)
	; 9 bytes
	php
	pha
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15909)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15911)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15912)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 15914)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 15915)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15916)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15917)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15918)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15920)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15921)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 15921 && n < 15926))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 15925 && n < 15931) || (n > 15931 && n < 15936) || n = 15937)
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 15931)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 15936)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 15938) && (n < 16094))
; 15938 to 16093
.if((n > 15937 && n < 16094))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 16094) && (n < 16172))
.if((n >= 16094) && (n < 16133))
.if((n >= 16094) && (n < 16113))
; 16094 to 16112
.if((n > 16093 && n < 16098))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16098 || n = 16099)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16100)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16100 && n < 16105))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-295)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16106)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-296)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16107)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16108)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16109)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16110)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16111)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16112)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16114)
	; 16 bytes
	php
	tya
	pha
	ldy #185
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16115)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16116)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #25
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16117)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16118)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16119)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16120)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16121)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #49
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16122)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #49
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16123)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16124)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #35
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16125)
	; 16 bytes
	php
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
	plp
.elseif(n = 16126)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16127)
	; 16 bytes
	php
	tya
	ldy #55
	pha
	lda #252
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16128)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16129)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16130)
	; 16 bytes
	php
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
	plp
.elseif(n = 16131)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #42
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16132)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #89
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 16133) && (n < 16152))
; 16133 to 16151
.if(n = 16133)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16134)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16135)
	; 16 bytes
	php
	tya
	pha
	ldy #237
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16136)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16137)
	; 16 bytes
	php
	tya
	ldy #215
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16138)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16139)
	; 16 bytes
	php
	tya
	pha
	ldy #248
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16140)
	; 16 bytes
	php
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
	plp
.elseif(n = 16141)
	; 16 bytes
	php
	tya
	ldy #126
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 16142)
	; 16 bytes
	php
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
	plp
.elseif(n = 16143)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16144)
	; 16 bytes
	php
	tya
	pha
	ldy #215
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16145)
	; 16 bytes
	php
	tya
	ldy #221
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16146)
	; 17 bytes
	php
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16147)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16148)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16149)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #58
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16150)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 16151)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 16152 to 16171
.if(n = 16152 || n = 16155 || n = 16157 || n = 16158 || n = 16161 || n = 16162 || n = 16164 || n = 16169)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16153)
	; 7 bytes
	php
	lda #63
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16154 || n = 16159 || n = 16166)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16156 || n = 16163)
	; 8..10 bytes
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
.elseif(n = 16165)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16167)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16168)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 16170)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 16171)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
; 16172 to 16250
.if(n = 16172)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16173)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16174)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16175)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 16176)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16177)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 16177 && n < 16182))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 16181 && n < 16187) || (n > 16187 && n < 16192) || (n > 16192 && n < 16251))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16187)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16192)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 16329) && (n < 16368))
; 16329 to 16367
.if((n > 16328 && n < 16354))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16354 || n = 16355)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16356)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16356 && n < 16361))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-296)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16362)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-297)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16363)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16364)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16365)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16366)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16367)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 16368) && (n < 16387))
; 16368 to 16386
.if(n = 16368)
	; 16 bytes
	php
	tya
	ldy #188
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16369)
	; 16 bytes
	php
	tya
	pha
	ldy #218
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16370)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #49
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16371)
	; 16 bytes
	php
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
	plp
.elseif(n = 16372)
	; 16 bytes
	php
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
	plp
.elseif(n = 16373)
	; 16 bytes
	php
	tya
	pha
	ldy #221
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16374)
	; 16 bytes
	php
	tya
	ldy #(n-12)/81
	pha
	lda #40
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16375)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16376)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #251
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 16377)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16378)
	; 16 bytes
	php
	tya
	pha
	ldy #133
	lda #89
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16379)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16380)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #25
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16381)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16382)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16383)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #66
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16384)
	; 16 bytes
	php
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
	plp
.elseif(n = 16385)
	; 16 bytes
	php
	tya
	pha
	ldy #167
	lda #64
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16386)
	; 16 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.endif
.else
; 16387 to 16406
.if(n = 16387)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16388)
	; 16 bytes
	php
	tya
	ldy #184
	pha
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16389)
	; 16 bytes
	php
	tya
	ldy #159
	pha
	lda #62
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16390)
	; 16 bytes
	php
	tya
	pha
	ldy #153
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16391)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16392)
	; 16 bytes
	php
	tya
	ldy #252
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16393)
	; 16 bytes
	php
	tya
	pha
	ldy #184 ;hides 'clv'
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 16394)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #91
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16395)
	; 16 bytes
	php
	tya
	pha
	ldy #184
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16396)
	; 16 bytes
	php
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
	plp
.elseif(n = 16397)
	; 16 bytes
	php
	tya
	ldy #145
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16398)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16399)
	; 16 bytes
	php
	tya
	pha
	ldy #<((n-23)/2)
	lda #>((n-23)/2)
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16400)
	; 16 bytes
	php
	tya
	ldy #241
	pha
	lda #27
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16401 || n = 16406)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 16402)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #93
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 16403)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #30
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16404)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16405)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #56
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.endif
.endif
.else
; 16407 to 16562
.if(n = 16407 || n = 16412 || n = 16419 || n = 16431)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 16408 || n = 16411 || n = 16413 || n = 16414 || n = 16417 || n = 16418 || n = 16420 || n = 16425)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16409)
	; 7 bytes
	php
	lda #64
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16410 || n = 16415 || n = 16422)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16416)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16421)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16423)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16424)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 16426)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 16427)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16428)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16429)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16430)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16432)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16433)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 16433 && n < 16438))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 16437 && n < 16443) || (n > 16443 && n < 16448) || (n > 16448 && n < 16563))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16443)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16448)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
.if((n >= 16563) && (n < 16602))
; 16563 to 16601
.if((n > 16562 && n < 16602))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 16602) && (n < 16621))
; 16602 to 16620
.if((n > 16601 && n < 16610))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16610 || n = 16611)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16612)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16612 && n < 16617))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-297)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16618)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-298)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16619)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16620)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 16621 to 16640
.if(n = 16621)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16622)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16623)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16624)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16625)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16626)
	; 16 bytes
	php
	tya
	ldy #(n-12)/71
	pha
	lda #30
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16627)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #62
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16628)
	; 16 bytes
	php
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
	plp
.elseif(n = 16629)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #76
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16630)
	; 16 bytes
	php
	tya
	pha
	ldy #147
	lda #79
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16631)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 16632)
	; 16 bytes
	php
	tya
	ldy #60
	pha
	lda #236
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16633)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16634)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16635)
	; 16 bytes
	php
	tya
	pha
	ldy #248
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16636)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16637)
	; 16 bytes
	php
	tya
	ldy #175
	pha
	lda #54
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16638)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #61
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16639)
	; 16 bytes
	php
	tya
	pha
	ldy #60
	lda #243
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16640)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 16641) && (n < 16680))
.if((n >= 16641) && (n < 16660))
; 16641 to 16659
.if(n = 16641)
	; 16 bytes
	php
	tya
	ldy #241
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16642)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #188
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 16643)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16644)
	; 16 bytes
	php
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
	plp
.elseif(n = 16645)
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #68
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16646)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #8
	jsr delay_256a_11_clocks_b_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 16647)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16648)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16649)
	; 16 bytes
	php
	tya
	ldy #131
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16650)
	; 16 bytes
	php
	tya
	ldy #177
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16651)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16652)
	; 16 bytes
	php
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
	plp
.elseif(n = 16653)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #88
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 16654)
	; 16 bytes
	php
	tya
	ldy #(n-12)/106
	pha
	lda #65
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16655)
	; 16 bytes
	php
	tya
	ldy #187
	pha
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16656)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16657)
	; 16 bytes
	php
	tya
	pha
	ldy #177
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16658)
	; 16 bytes
	php
	tya
	ldy #203
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16659)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 16660 to 16679
.if(n = 16660)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16661)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/106
	lda #72
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16662 || n = 16664 || n = 16667 || n = 16669 || n = 16670 || n = 16673 || n = 16674 || n = 16676)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16663 || n = 16668 || n = 16675)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 16665)
	; 7 bytes
	php
	lda #65
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16666 || n = 16671 || n = 16678)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16672)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16677)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16679)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 16680 to 16718
.if(n = 16680)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 16681)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16682)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 16683)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16684)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16685)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16686)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16687)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 16688)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16689)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 16689 && n < 16694))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 16693 && n < 16699) || (n > 16699 && n < 16704) || (n > 16704 && n < 16719))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16699)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16704)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
; 16719 to 16875
.if((n > 16718 && n < 16866))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16866 || n = 16867)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16868)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16868 && n < 16873))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-298)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16874)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-299)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16875)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	plp
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
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16877)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16878)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16879)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16880)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16881)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 16882)
	; 16 bytes
	php
	tya
	pha
	ldy #231
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16883)
	; 16 bytes
	php
	tya
	pha
	ldy #248
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16884)
	; 16 bytes
	php
	tya
	ldy #228
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16885)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16886)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16887)
	; 16 bytes
	php
	tya
	ldy #225
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16888)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16889)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16890)
	; 16 bytes
	php
	tya
	ldy #194
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16891)
	; 16 bytes
	php
	tya
	pha
	ldy #228
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16892)
	; 16 bytes
	php
	tya
	ldy #211
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16893)
	; 16 bytes
	php
	tya
	pha
	ldy #143
	lda #84
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16894)
	; 16 bytes
	php
	tya
	pha
	ldy #225
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 16895 to 16914
.if(n = 16895)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16896)
	; 16 bytes
	php
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
	plp
.elseif(n = 16897)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16898)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16899)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16900)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16901)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16902)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16903)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16904)
	; 16 bytes
	php
	tya
	ldy #206
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16905 || n = 16906 || n = 16913)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 16907)
	; 16 bytes
	php
	tya
	ldy #155
	pha
	lda #68
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16908)
	; 16 bytes
	php
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
	plp
.elseif(n = 16909)
	; 16 bytes
	php
	tya
	ldy #61
	pha
	lda #236
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16910)
	; 16 bytes
	php
	tya
	pha
	ldy #133
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16911)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16912)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #59
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16914)
	; 16 bytes
	php
	tya
	pha
	ldy #155
	lda #75
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
; 16915 to 16953
.if(n = 16915)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16916)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16917)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16918 || n = 16920 || n = 16923 || n = 16925 || n = 16926 || n = 16929 || n = 16930 || n = 16932 || n = 16937)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16919 || n = 16924 || n = 16931 || n = 16943)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 16921)
	; 7 bytes
	php
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16922 || n = 16927 || n = 16934)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16928)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16933)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16935)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16936)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 16938)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 16939)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16940)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16941)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16942)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16944)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16945)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 16945 && n < 16950))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 16949 && n < 16954))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 16954 to 17031
.if(n = 16954 || (n > 16955 && n < 16960) || (n > 16960 && n < 17032))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 16955)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 16960)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
; 17032 to 17109
.if((n > 17031 && n < 17110))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 17110) && (n < 17149))
; 17110 to 17148
.if((n > 17109 && n < 17122))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17122 || n = 17123)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17124)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 17124 && n < 17129))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-299)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17130)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-300)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17131)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17132)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17133)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17134)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17135)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17136)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17137)
	; 16 bytes
	php
	tya
	ldy #137
	pha
	lda #84
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17138)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17139)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17140)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17141)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17142)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17143)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17144)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #91
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17145)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17146)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #65
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17147)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #74
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17148)
	; 16 bytes
	php
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
	plp
.endif
.else
.if((n >= 17149) && (n < 17168))
; 17149 to 17167
.if(n = 17149)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17150)
	; 16 bytes
	php
	tya
	ldy #209
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17151)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17152)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17153)
	; 16 bytes
	php
	tya
	ldy #(n-12)/281
	pha
	lda #240
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17154)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #81
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17155)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17156)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17157)
	; 16 bytes
	php
	tya
	pha
	ldy #209
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17158)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17159)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17160)
	; 16 bytes
	php
	tya
	pha
	ldy #61
	lda #247
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17161)
	; 17 bytes
	php
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
	plp
.elseif(n = 17162)
	; 16 bytes
	php
	tya
	pha
	ldy #217
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17163)
	; 17 bytes
	php
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17164)
	; 16 bytes
	php
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
	plp
.elseif(n = 17165)
	; 17 bytes
	php
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17166)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17167)
	; 16 bytes
	php
	tya
	ldy #235
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 17168 to 17187
.if(n = 17168)
	; 16 bytes
	php
	tya
	pha
	ldy #10
	lda #6
	jsr delay_256a_16_clocks_b
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 17169)
	; 16 bytes
	php
	tya
	pha
	ldy #245
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17170)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #122
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 17171)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17172)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17173)
	; 16 bytes
	php
	tya
	ldy #(n-12)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17174 || n = 17176 || n = 17179 || n = 17181 || n = 17182 || n = 17185 || n = 17186)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17175 || n = 17180 || n = 17187)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 17177)
	; 7 bytes
	php
	lda #67
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17178 || n = 17183)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17184)
	; 9 bytes
	php
	pha
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
.if(n = 17188 || n = 17193)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17189)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17190)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17191)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17192)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 17194)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 17195)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17196)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17197)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17198)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17199)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 17200)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17201)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 17201 && n < 17206))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 17205 && n < 17211) || (n > 17211 && n < 17216) || (n > 17216 && n < 17344))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17211)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17216)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
; 17344 to 17382
.if((n > 17343 && n < 17378))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17378 || n = 17379)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17380)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 17381 || n = 17382)
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-300)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17386)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-301)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17387)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17388)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17389)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17390)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17391)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17392)
	; 16 bytes
	php
	tya
	ldy #220
	pha
	lda #38
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17393)
	; 16 bytes
	php
	tya
	pha
	ldy #238
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17394)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #91
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17395)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #62
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17396)
	; 16 bytes
	php
	tya
	ldy #212
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17397)
	; 16 bytes
	php
	tya
	ldy #183
	pha
	lda #54
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17398)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17399)
	; 16 bytes
	php
	tya
	pha
	ldy #220
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17400)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17401)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #128
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.endif
.else
; 17402 to 17421
.if(n = 17402)
	; 16 bytes
	php
	tya
	ldy #235
	pha
	lda #(n-539)/511
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17403)
	; 16 bytes
	php
	tya
	pha
	ldy #212
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17404)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #61
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17405)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #90
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 17406)
	; 16 bytes
	php
	tya
	ldy #223
	pha
	lda #37
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17407)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17408)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17409)
	; 16 bytes
	php
	tya
	pha
	ldy #235
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17410)
	; 16 bytes
	php
	tya
	pha
	ldy #187
	lda #59
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17411)
	; 16 bytes
	php
	tya
	ldy #137
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17412)
	; 16 bytes
	php
	tya
	ldy #232
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17413)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17414)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17415)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17416)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #35
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17417)
	; 16 bytes
	php
	tya
	ldy #58
	pha
	lda #254
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 17418)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17419)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17420)
	; 16 bytes
	php
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
	plp
.elseif(n = 17421)
	; 16 bytes
	php
	tya
	pha
	ldy #226
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.endif
.else
; 17422 to 17500
.if(n = 17422)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17423)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #42
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17424)
	; 17 bytes
	php
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17425 || n = 17429 || n = 17430 || n = 17432 || n = 17435 || n = 17437 || n = 17438 || n = 17441 || n = 17442 || n = 17444 || n = 17449)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 17426)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #69
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17427)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17428)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17431 || n = 17436 || n = 17443 || n = 17455)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 17433)
	; 7 bytes
	php
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17434 || n = 17439 || n = 17446)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17440)
	; 9 bytes
	php
	pha
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17445)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17447)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17448)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 17450)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 17451)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17452)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17453)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17454)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17456)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17457)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 17457 && n < 17462))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 17461 && n < 17467) || (n > 17467 && n < 17472) || (n > 17472 && n < 17501))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17467)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17472)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
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
.if((n > 17500 && n < 17634))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17634 || n = 17635)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17636)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 17636 && n < 17641))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-301)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17642)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-302)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17643)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17644)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17645)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17646)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17647)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17648)
	; 16 bytes
	php
	tya
	pha
	ldy #61
	lda #255
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17649)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17650)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #202
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 17651)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17652)
	; 16 bytes
	php
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
	plp
.elseif(n = 17653)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17654)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17655)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17656)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 17657) && (n < 17735))
.if((n >= 17657) && (n < 17696))
.if((n >= 17657) && (n < 17676))
; 17657 to 17675
.if(n = 17657)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #209
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 17658)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #61
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17659)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17660)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17661)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17662)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17663)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17664)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17665)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #68
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17666)
	; 16 bytes
	php
	tya
	ldy #(n-12)/91
	pha
	lda #50
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17667)
	; 16 bytes
	php
	tya
	ldy #165
	pha
	lda #66
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17668)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #77
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17669)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17670)
	; 16 bytes
	php
	tya
	ldy #(n-12)/81
	pha
	lda #40
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17671)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #61
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 17672)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17673)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17674)
	; 16 bytes
	php
	tya
	pha
	ldy #165
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17675)
	; 16 bytes
	php
	tya
	pha
	ldy #218 ;hides 'nop'
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.endif
.else
; 17676 to 17695
.if(n = 17676)
	; 16 bytes
	php
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
	plp
.elseif(n = 17677)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17678)
	; 16 bytes
	php
	tya
	ldy #242
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17679)
	; 16 bytes
	php
	tya
	ldy #151
	pha
	lda #76
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17680)
	; 16 bytes
	php
	tya
	pha
	ldy #203
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17681)
	; 16 bytes
	php
	tya
	pha
	ldy #184 ;hides 'clv'
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 17682)
	; 16 bytes
	php
	tya
	ldy #190
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17683)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17684)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17685)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17686 || n = 17688 || n = 17691 || n = 17693 || n = 17694)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 17687 || n = 17692)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17689)
	; 7 bytes
	php
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17690 || n = 17695)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 17697 || n = 17698 || n = 17700 || n = 17705)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17699 || n = 17711)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 17701)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17702)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17703)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17704)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 17706)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 17707)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17708)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17709)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17710)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17712)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17713)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 17713 && n < 17718))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 17717 && n < 17723) || (n > 17723 && n < 17728) || (n > 17728 && n < 17735))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17723)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17728)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 17735 to 17812
.if((n > 17734 && n < 17813))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
; 17813 to 17890
.if((n > 17812 && n < 17890))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17890)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 17891) && (n < 17930))
.if((n >= 17891) && (n < 17910))
; 17891 to 17909
.if(n = 17891)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17892)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 17892 && n < 17897))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-302)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17898)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-303)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17899)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17900)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17901)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17902)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17903)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17904)
	; 16 bytes
	php
	tya
	pha
	ldy #245
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17905)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 17906)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #165
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 17907)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17908)
	; 16 bytes
	php
	tya
	pha
	ldy #201
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17909)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 17910 to 17929
.if(n = 17910)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #73
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17911)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17912)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #59
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17913)
	; 16 bytes
	php
	tya
	ldy #(n-12)/81
	pha
	lda #40
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17914)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17915)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17916)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17917)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #80
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17918)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #105
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 17919)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #66
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17920)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17921)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17922)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #49
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17923)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17924)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17925)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #206
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 17926)
	; 16 bytes
	php
	tya
	pha
	ldy #141
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17927)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17928)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17929)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
; 17930 to 17968
.if(n = 17930)
	; 16 bytes
	php
	tya
	ldy #62
	pha
	lda #248
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17931)
	; 16 bytes
	php
	tya
	ldy #181
	pha
	lda #58
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17932)
	; 16 bytes
	php
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
	plp
.elseif(n = 17933)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/106
	lda #72
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17934)
	; 16 bytes
	php
	tya
	ldy #206
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17935)
	; 17 bytes
	php
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17936 || n = 17942 || n = 17944 || n = 17947 || n = 17949 || n = 17950 || n = 17953 || n = 17954 || n = 17956 || n = 17961)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 17937)
	; 16 bytes
	php
	tya
	pha
	ldy #62
	lda #255
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17938)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #65
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17939)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17940)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17941)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17943 || n = 17948 || n = 17955 || n = 17967)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 17945)
	; 7 bytes
	php
	lda #70
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17946 || n = 17951 || n = 17958)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17952)
	; 9 bytes
	php
	pha
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17957)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17959)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17960)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 17962)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 17963)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17964)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17965)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17966)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17968)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
; 17969 to 18125
.if(n = 17969)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 17969 && n < 17974))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 17973 && n < 17979) || (n > 17979 && n < 17984) || (n > 17984 && n < 18126))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 17979)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 17984)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
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
.if((n > 18125 && n < 18146))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18146 || n = 18147)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18148)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 18148 && n < 18153))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-303)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18154)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-304)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18155)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18156)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18157)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18158)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18159)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18160)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #107
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 18161)
	; 16 bytes
	php
	tya
	pha
	ldy #193
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18162)
	; 16 bytes
	php
	tya
	ldy #242
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18163)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18164)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #61
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 18165) && (n < 18184))
; 18165 to 18183
.if(n = 18165)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18166)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #229
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18167)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #85
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18168)
	; 16 bytes
	php
	tya
	ldy #204
	pha
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18169)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18170)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18171)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18172)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18173)
	; 16 bytes
	php
	tya
	ldy #143
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18174)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18175)
	; 16 bytes
	php
	tya
	pha
	ldy #204
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18176)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18177)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #64
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18178)
	; 16 bytes
	php
	tya
	ldy #62
	pha
	lda #252
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18179)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18180)
	; 16 bytes
	php
	tya
	pha
	ldy #143
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18181)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18182)
	; 16 bytes
	php
	tya
	ldy #230
	pha
	lda #38
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18183)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #42
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 18184 to 18203
.if(n = 18184)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #71
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18185)
	; 17 bytes
	php
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18186)
	; 16 bytes
	php
	tya
	ldy #233
	pha
	lda #37
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18187)
	; 16 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #7
	jsr delay_256a_16_clocks_b
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18188)
	; 16 bytes
	php
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
	plp
.elseif(n = 18189)
	; 16 bytes
	php
	tya
	pha
	ldy #230
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18190)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #81
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18191)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18192)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	lda #49
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18193)
	; 16 bytes
	php
	tya
	pha
	ldy #233
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18194 || n = 18198 || n = 18200 || n = 18203)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 18195)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18196)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 18197)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #88
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18199)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18201)
	; 7 bytes
	php
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18202)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
; 18204 to 18281
.if(n = 18204 || n = 18211 || n = 18223)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 18205 || n = 18206 || n = 18209 || n = 18210 || n = 18212 || n = 18217)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18207 || n = 18214)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18208)
	; 9 bytes
	php
	pha
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18213)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18215)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18216)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 18218)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 18219)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18220)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 18221)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18222)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18224)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18225)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 18225 && n < 18230))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 18229 && n < 18235) || (n > 18235 && n < 18240) || (n > 18240 && n < 18282))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18235)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 18240)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
; 18282 to 18359
.if((n > 18281 && n < 18360))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if((n > 18359 && n < 18399))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 18399) && (n < 18418))
; 18399 to 18417
.if((n > 18398 && n < 18402))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18402 || n = 18403)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18404)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 18404 && n < 18409))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-304)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18410)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-305)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18411)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18412)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18413)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18414)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18415)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18416)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #45
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18417)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #56
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.endif
.else
; 18418 to 18437
.if(n = 18418)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #134
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 18419)
	; 16 bytes
	php
	tya
	pha
	ldy #230
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18420)
	; 16 bytes
	php
	tya
	ldy #236
	pha
	lda #37
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18421)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #50
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18422)
	; 16 bytes
	php
	tya
	pha
	ldy #239
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18423)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18424)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18425)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #141
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 18426)
	; 16 bytes
	php
	tya
	pha
	ldy #233
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18427)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18428)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18429)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18430)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18431)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18432)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18433)
	; 16 bytes
	php
	tya
	pha
	ldy #198
	lda #59
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18434)
	; 16 bytes
	php
	tya
	pha
	ldy #145
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18435)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18436)
	; 16 bytes
	php
	tya
	ldy #196
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18437)
	; 16 bytes
	php
	tya
	ldy #66
	pha
	lda #234
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
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
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #79
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18439)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18440)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #75
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18441)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #88
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18442)
	; 16 bytes
	php
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
	plp
.elseif(n = 18443)
	; 16 bytes
	php
	tya
	pha
	ldy #196
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18444)
	; 16 bytes
	php
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
	plp
.elseif(n = 18445)
	; 16 bytes
	php
	tya
	pha
	ldy #249
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18446)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18447)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18448)
	; 17 bytes
	php
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18449)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #61
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18450)
	; 17 bytes
	php
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18451)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18452)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 18453)
	; 17 bytes
	php
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18454 || n = 18456)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 18455)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 18457 to 18476
.if(n = 18457)
	; 7 bytes
	php
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18458 || n = 18463 || n = 18470)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18459 || n = 18461 || n = 18462 || n = 18465 || n = 18466 || n = 18468 || n = 18473)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18460 || n = 18467)
	; 8..10 bytes
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
.elseif(n = 18469)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18471)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18472)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 18474)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 18475)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18476)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.else
; 18477 to 18515
.if(n = 18477)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18478)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18479)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 18480)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18481)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 18481 && n < 18486))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 18485 && n < 18491) || (n > 18491 && n < 18496) || (n > 18496 && n < 18516))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18491)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 18496)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 18516 to 18593
.if((n > 18515 && n < 18594))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
; 18594 to 18671
.if((n > 18593 && n < 18658))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18658 || n = 18659)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18660)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 18660 && n < 18665))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-305)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18666)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-306)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18667)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18668)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18669)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18670)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18671)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 18672) && (n < 18711))
.if((n >= 18672) && (n < 18691))
; 18672 to 18690
.if(n = 18672)
	; 16 bytes
	php
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18673)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #140
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18674)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18675)
	; 16 bytes
	php
	tya
	pha
	ldy #212
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18676)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18677)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18678)
	; 16 bytes
	php
	tya
	ldy #183
	pha
	lda #61
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18679)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #181
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 18680)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18681)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18682)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18683)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18684)
	; 16 bytes
	php
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
	plp
.elseif(n = 18685)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #(n-257)/271
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18686)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18687)
	; 16 bytes
	php
	tya
	ldy #249
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18688)
	; 16 bytes
	php
	tya
	pha
	ldy #147
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18689)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18690)
	; 16 bytes
	php
	tya
	ldy #65
	pha
	lda #242
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.endif
.else
; 18691 to 18710
.if(n = 18691)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18692)
	; 16 bytes
	php
	tya
	pha
	ldy #70
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18693)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18694)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_b_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 18695)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18696)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #67
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18697)
	; 16 bytes
	php
	tya
	pha
	ldy #65
	lda #249
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18698)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 18699 || n = 18710)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 18700)
	; 16 bytes
	php
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
	plp
.elseif(n = 18701)
	; 17 bytes
	php
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
	plp
.elseif(n = 18702)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #85
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18703)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #74
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18704)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18705)
	; 16 bytes
	php
	tya
	ldy #201
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18706)
	; 16 bytes
	php
	tya
	ldy #122 ;hides 'nop'
	pha
	lda #109
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	plp
.elseif(n = 18707)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18708)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 18709)
	; 16 bytes
	php
	tya
	pha
	ldy #210
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
; 18711 to 18750
.if(n = 18711 || n = 18716 || n = 18723 || n = 18735)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 18712 || n = 18715 || n = 18717 || n = 18718 || n = 18721 || n = 18722 || n = 18724 || n = 18729)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18713)
	; 7 bytes
	php
	lda #73
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18714 || n = 18719 || n = 18726)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18720)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18725)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18727)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18728)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 18730)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 18731)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18732)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 18733)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18734)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18736)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18737)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 18737 && n < 18742))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 18741 && n < 18747) || (n > 18747 && n < 18751))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18747)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
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
.if(n = 18751 || (n > 18752 && n < 18907))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18752)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
.if((n >= 18907) && (n < 18926))
; 18907 to 18925
.if((n > 18906 && n < 18914))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18914 || n = 18915)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18916)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 18916 && n < 18921))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-306)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18922)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-307)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18923)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18924)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18925)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
; 18926 to 18945
.if(n = 18926)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18927)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18928)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #65
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18929)
	; 16 bytes
	php
	tya
	pha
	ldy #155
	lda #88
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18930)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18931)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #62
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18932)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #45
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18933)
	; 16 bytes
	php
	tya
	pha
	ldy #193
	lda #64
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18934)
	; 16 bytes
	php
	tya
	pha
	ldy #195
	lda #63
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18935)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18936)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #35
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18937)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #184
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 18938)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #139
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 18939)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18940)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #85
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18941)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18942)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18943)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #42
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18944)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18945)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #146
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 18946) && (n < 18965))
; 18946 to 18964
.if(n = 18946)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #220
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 18947)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18948)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18949)
	; 16 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #134
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 18950)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18951)
	; 16 bytes
	php
	tya
	ldy #177
	pha
	lda #66
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18952)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18953)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #227
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 18954)
	; 16 bytes
	php
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
	plp
.elseif(n = 18955)
	; 16 bytes
	php
	tya
	pha
	ldy #72
	lda #229
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18956)
	; 16 bytes
	php
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
	plp
.elseif(n = 18957)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 18958)
	; 16 bytes
	php
	tya
	pha
	ldy #177
	lda #(n-270)/256
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18959)
	; 16 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #41
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18960)
	; 17 bytes
	php
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18961)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18962)
	; 16 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 18963)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18964)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.endif
.else
; 18965 to 18984
.if(n = 18965 || n = 18966 || n = 18968 || n = 18971 || n = 18973 || n = 18974 || n = 18977 || n = 18978 || n = 18980)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 18967 || n = 18972 || n = 18979)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 18969)
	; 7 bytes
	php
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18970 || n = 18975 || n = 18982)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18976)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18981)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18983)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18984)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18986)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 18987)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18988)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 18989)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18990)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18991)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 18992)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 18993)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 18993 && n < 18998))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 18997 && n < 19003) || (n > 19003 && n < 19008) || (n > 19008 && n < 19063))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19003)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19008)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
; 19063 to 19140
.if((n > 19062 && n < 19141))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 19141) && (n < 19180))
; 19141 to 19179
.if((n > 19140 && n < 19170))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19170 || n = 19171)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19172)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 19172 && n < 19177))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-307)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19178)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-308)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19179)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 19180) && (n < 19199))
; 19180 to 19198
.if(n = 19180)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19181)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19182)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19183)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19184)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19185)
	; 16 bytes
	php
	tya
	pha
	ldy #73
	lda #225
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19186)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19187)
	; 16 bytes
	php
	tya
	ldy #64
	pha
	lda #254
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 19188)
	; 16 bytes
	php
	tya
	ldy #204
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19189)
	; 16 bytes
	php
	tya
	pha
	ldy #213
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19190)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #45
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19191)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19192)
	; 16 bytes
	php
	tya
	pha
	ldy #249
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19193)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19194)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #97
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19195)
	; 16 bytes
	php
	tya
	pha
	ldy #204
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19196)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19197)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19198)
	; 16 bytes
	php
	tya
	ldy #(n-12)/106
	pha
	lda #65
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 19199 to 19218
.if(n = 19199)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #240
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19200)
	; 16 bytes
	php
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
	plp
.elseif(n = 19201)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #244
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19202)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	lda #54
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19203)
	; 16 bytes
	php
	tya
	pha
	ldy #218
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19204)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19205)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/106
	lda #72
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19206)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19207)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19208)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19209)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #61
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19210)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 19211)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 19212)
	; 16 bytes
	php
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
	plp
.elseif(n = 19213)
	; 16 bytes
	php
	tya
	ldy #(n-12)/91
	pha
	lda #50
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19214)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19215)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19216)
	; 16 bytes
	php
	tya
	pha
	ldy #243
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19217)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #(n-273)/256
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19218)
	; 16 bytes
	php
	tya
	pha
	ldy #72
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.endif
.endif
.endif
.endif
.else
; 19219 to 19375
.if(n = 19219)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19220)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19221)
	; 17 bytes
	php
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19222 || n = 19224 || n = 19227 || n = 19229 || n = 19230 || n = 19233 || n = 19234 || n = 19236 || n = 19241)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19223 || n = 19228 || n = 19235 || n = 19247)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 19225)
	; 7 bytes
	php
	lda #75
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19226 || n = 19231 || n = 19238)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19232)
	; 9 bytes
	php
	pha
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19237)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19239)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19240)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 19242)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 19243)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19244)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19245)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19246)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19248)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19249)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 19249 && n < 19254))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 19253 && n < 19259) || (n > 19259 && n < 19264) || (n > 19264 && n < 19376))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19259)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19264)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
.if((n >= 19376) && (n < 19688))
.if((n >= 19376) && (n < 19532))
.if((n >= 19376) && (n < 19454))
; 19376 to 19453
.if((n > 19375 && n < 19426))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19426 || n = 19427)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19428)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 19428 && n < 19433))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-308)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19434)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-309)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19435)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19436)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19437)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19438)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19439)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19440)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19441)
	; 16 bytes
	php
	tya
	pha
	ldy #249
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19442)
	; 16 bytes
	php
	tya
	ldy #145
	pha
	lda #93
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19443)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19444)
	; 16 bytes
	php
	tya
	pha
	ldy #185
	lda #71
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19445)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19446)
	; 16 bytes
	php
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
	plp
.elseif(n = 19447)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #74
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19448)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #45
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19449)
	; 16 bytes
	php
	tya
	ldy #209
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19450)
	; 16 bytes
	php
	tya
	pha
	ldy #153
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19451)
	; 16 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #43
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19452)
	; 16 bytes
	php
	tya
	ldy #243
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19453)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 19454) && (n < 19493))
.if((n >= 19454) && (n < 19473))
; 19454 to 19472
.if(n = 19454)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #81
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19455)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19456)
	; 16 bytes
	php
	tya
	pha
	ldy #209
	lda #59
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19457)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #234
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 19458)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #144
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 19459)
	; 16 bytes
	php
	tya
	pha
	ldy #243
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19460)
	; 16 bytes
	php
	tya
	ldy #221
	pha
	lda #47
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19461)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19462)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19463)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19464)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19465)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #151
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 19466)
	; 16 bytes
	php
	tya
	ldy #142
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19467)
	; 16 bytes
	php
	tya
	pha
	ldy #221
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19468)
	; 16 bytes
	php
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
	plp
.elseif(n = 19469)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 19470)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19471)
	; 17 bytes
	php
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 137
	Jne *-8
	pla
	tay
	plp
.elseif(n = 19472)
	; 16 bytes
	php
	tya
	ldy #140
	pha
	lda #98
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 19473 to 19492
.if(n = 19473)
	; 16 bytes
	php
	tya
	pha
	ldy #70
	lda #240
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19474)
	; 16 bytes
	php
	tya
	ldy #73
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 19475)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #42
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19476)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19477)
	; 16 bytes
	php
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
	plp
.elseif(n = 19478 || n = 19480 || n = 19483 || n = 19485 || n = 19486 || n = 19489 || n = 19490 || n = 19492)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19479 || n = 19484 || n = 19491)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 19481)
	; 7 bytes
	php
	lda #76
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19482 || n = 19487)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19488)
	; 9 bytes
	php
	pha
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 19493 to 19531
.if(n = 19493)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19494)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19495)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19496)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 19497)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19498)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 19499)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19500)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19501)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19502)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19503)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 19504)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19505)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 19505 && n < 19510))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.elseif((n > 19509 && n < 19515) || (n > 19515 && n < 19520) || (n > 19520 && n < 19532))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19515)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19520)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.endif
.else
; 19532 to 19687
.if((n > 19531 && n < 19682))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19682 || n = 19683)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19684)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 19684 && n < 19688))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-309)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19690)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #(n-310)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19691)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19692)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19693)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19694)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19695)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19696)
	; 16 bytes
	php
	tya
	ldy #148
	pha
	lda #92
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19697)
	; 16 bytes
	php
	tya
	ldy #155
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19698)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #61
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19699)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19700)
	; 16 bytes
	php
	tya
	ldy #214
	pha
	lda #51
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19701)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #230
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19702)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19703)
	; 16 bytes
	php
	tya
	pha
	ldy #148
	lda #99
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19704)
	; 16 bytes
	php
	tya
	pha
	ldy #155
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19705)
	; 16 bytes
	php
	tya
	pha
	ldy #193
	lda #68
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19706)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #45
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 19707 to 19726
.if(n = 19707)
	; 16 bytes
	php
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
	plp
.elseif(n = 19708)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19709)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #76
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19710)
	; 16 bytes
	php
	tya
	pha
	ldy #203
	lda #63
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19711)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #193
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 19712)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #59
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19713)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19714)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19715)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19716)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19717)
	; 16 bytes
	php
	tya
	pha
	ldy #201
	lda #64
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19718)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #77
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19719)
	; 16 bytes
	php
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
	plp
.elseif(n = 19720)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #65
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19721)
	; 17 bytes
	php
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19722)
	; 16 bytes
	php
	tya
	ldy #219
	pha
	lda #49
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19723)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #131
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19724)
	; 16 bytes
	php
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
	plp
.elseif(n = 19725)
	; 16 bytes
	php
	tya
	pha
	ldy #167
	lda #84
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19726)
	; 17 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
; 19727 to 19765
.if(n = 19727)
	; 16 bytes
	php
	tya
	pha
	ldy #90 ;hides 'nop'
	lda #183
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19728)
	; 16 bytes
	php
	tya
	ldy #212
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19729)
	; 16 bytes
	php
	tya
	pha
	ldy #219
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19730)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #42
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 19731)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19732)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19733)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #150
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19734 || n = 19736 || n = 19739 || n = 19741 || n = 19742 || n = 19745 || n = 19746 || n = 19748 || n = 19753)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19735 || n = 19740 || n = 19747 || n = 19759)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 19737)
	; 7 bytes
	php
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19738 || n = 19743 || n = 19750)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19744)
	; 9 bytes
	php
	pha
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19749)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19751)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19752)
	; 11 bytes
	delay_n 15
	delay_n (n-15)
.elseif(n = 19754)
	; 11 bytes
	delay_n 17
	delay_n (n-17)
.elseif(n = 19755)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks_b
	lda #>(n-43)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19756)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19757)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19758)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19760)
	; 13 bytes
	php
	clc
	clc
	lda #<(n-6) ;hides 'rol a'
	Jcc *-1
	lda #>(n-6)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19761)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks_b
	plp
.elseif((n > 19761 && n < 19766))
	; 12 bytes
	php
	lda #<(n-49)
	jsr delay_a_27_clocks
	lda #>(n-49)
	jsr delay_256a_11_clocks_b_
	plp
.endif
.endif
.else
; 19766 to 19843
.if((n > 19765 && n < 19771) || (n > 19771 && n < 19776) || (n > 19776 && n < 19844))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19771)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19776)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks_b
	plp
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
; 19844 to 19921
.if((n > 19843 && n < 19922))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.endif
.else
.if((n >= 19922) && (n < 19961))
; 19922 to 19960
.if((n > 19921 && n < 19938))
	; 12 bytes
	php
	lda #<(n-54)
	jsr delay_a_27_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19938 || n = 19939)
	; 13 bytes
	php
	nop
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19940)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 19940 && n < 19945))
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
	delay_n 5
	lda #170
	jsr delay_a_27_clocks
	lda #(n-310)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19946)
	; 15 bytes
	php
	delay_n 5
	lda #171
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19947)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19948)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19949)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19950)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19951)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19952)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19953)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #248
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19954)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #77
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19955)
	; 16 bytes
	php
	tya
	pha
	ldy #224
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19956)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #236
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19957)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19958)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19959)
	; 16 bytes
	php
	tya
	ldy #183
	pha
	lda #68
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19960)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #255
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 19961) && (n < 19981))
; 19961 to 19980
.if(n = 19961)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #84
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19962)
	; 16 bytes
	php
	tya
	pha
	ldy #76
	lda #225
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19963)
	; 16 bytes
	php
	tya
	pha
	ldy #72
	lda #243
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19964)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #45
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19965)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19966)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #75
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19967)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #97
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19968)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19969)
	; 16 bytes
	php
	tya
	pha
	ldy #210
	lda #61
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19970)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-300)/281
	lda #247
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19971)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19972)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19973)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19974)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19975)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #122
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 19976)
	; 16 bytes
	php
	tya
	ldy #217
	pha
	lda #51
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19977)
	; 16 bytes
	php
	tya
	ldy #(n-12)/121
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19978)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #93
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19979)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	delay_n 2848
	Jcs *-11
	plp
.elseif(n = 19980)
	; 16 bytes
	php
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
	plp
.endif
.else
; 19981 to 20000
.if(n = 19981)
	; 17 bytes
	php
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
	plp
.elseif(n = 19982)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #129
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 19983)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19984)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19985)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #100
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19986)
	; 17 bytes
	php
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19987)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19988)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_b_
	plp
.elseif(n = 19989)
	; 17 bytes
	php
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19990 || n = 19992 || n = 19995 || n = 19997 || n = 19998)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 19991 || n = 19996)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19993)
	; 7 bytes
	php
	lda #78
	jsr delay_256a_16_clocks_b
	plp
.elseif(n = 19994 || n = 19999)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
