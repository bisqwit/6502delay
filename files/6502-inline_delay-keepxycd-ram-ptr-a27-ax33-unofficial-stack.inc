; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, S, Z+N, and V
; Will preserve X, Y, C, D, and I (interrupt-disable flag)
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
; - 3 bytes or less when n <= 12
; - 4 bytes or less when n <= 19
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 31
; - 7 bytes or less when n <= 292
; - 8 bytes or less when n <= 310
; - 9 bytes or less when n <= 1305
; -10 bytes or less when n <= 1981
; -11 bytes or less when n <= 3001
; -12 bytes or less when n <= 3995
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=3, avg=3.7, max=5
; - When 21 <= n <= 30: min=5, avg=5.7, max=6
; - When 31 <= n <= 50: min=6, avg=6.9, max=7
; - When 51 <= n <= 75: min=6, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=6, avg=7.0, max=8
; - When 301 <= n <= 500: min=6, avg=7.9, max=9
; - When 501 <= n <= 750: min=6, avg=7.9, max=9
; - When 751 <= n <= 1000: min=7, avg=7.9, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.6, max=11
; - When 2001 <= n <= 3000: min=8, avg=9.7, max=11
; - When 3001 <= n <= 5000: min=9, avg=11.0, max=13
; - When 5001 <= n <= 7500: min=10, avg=12.6, max=13
; - When 7501 <= n <= 10000: min=11, avg=13.0, max=13
; - When 10001 <= n <= 20000: min=11, avg=13.0, max=13
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
.elseif(n = 4)
	; 1 bytes
	pla
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
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
.elseif(n = 9 || n = 10 || n = 13 || n = 21 || n = 22 || n = 25)
	; 3..6 bytes
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
.elseif(n = 16 || n = 17 || n = 32)
	; 4..7 bytes
	delay_n (n-2)
	nop
.elseif((n > 17 && n < 21) || n = 23 || n = 27 || n = 29 || n = 30)
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
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 33)
	; 7 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 34)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 35)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 36 || n = 37 || n = 39)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 38)
	; 7 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	plp
.endif
.else
; 40 to 78
.if(n = 40 || n = 42 || n = 44 || n = 45 || n = 47 || (n > 48 && n < 53) || (n > 53 && n < 58) || (n > 58 && n < 62) || (n > 62 && n < 79))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 41)
	; 7 bytes
	php
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	plp
.elseif(n = 43)
	; 7 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	plp
.elseif(n = 46)
	; 6 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	plp
.elseif(n = 48)
	; 7 bytes
	php
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
	plp
.elseif(n = 53)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
.elseif(n = 58)
	; 7 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	plp
.elseif(n = 62)
	; 7 bytes
	php
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	plp
.endif
.endif
.else
; 79 to 156
.if((n > 78 && n < 118) || (n > 118 && n < 123) || (n > 123 && n < 128) || (n > 128 && n < 157))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 118)
	; 7 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	plp
.elseif(n = 123)
	; 7 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	plp
.elseif(n = 128)
	; 6 bytes
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
.endif
.endif
.else
; 157 to 312
.if((n > 156 && n < 163) || (n > 163 && n < 288) || (n > 288 && n < 292))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 163)
	; 7 bytes
	php
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	plp
.elseif(n = 288)
	; 6 bytes
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
.elseif(n = 292 || n = 304 || n = 309)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 293 || n = 297 || n = 302 || n = 307 || n = 312)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 294 || n = 298)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 295 || n = 300)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 296)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 299 || n = 306)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 301)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 303)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 305 || n = 310)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 308)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	dey
	Jpl *-3
	tay
.elseif(n = 311)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.endif
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 348 || n = 362 || n = 369 || n = 404 || n = 432 || n = 439 || n = 446)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 314 || n = 319 || n = 324 || n = 329 || n = 339 || n = 344 || n = 349 || n = 354 || n = 359 || n = 364 || n = 374 || n = 379 || n = 384 || n = 387 || n = 389 || n = 394 || n = 406 || n = 414 || n = 419 || n = 424 || n = 429 || n = 434 || n = 444 || n = 449 || n = 454 || n = 464)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385 || n = 400 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 316 || n = 326 || n = 331 || n = 351 || n = 366 || n = 396 || n = 426 || n = 431 || n = 451 || n = 456 || n = 466)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif(n = 317 || n = 322 || n = 332 || n = 336 || n = 337 || n = 342 || n = 347 || n = 352 || n = 357 || n = 367 || n = 371 || n = 372 || n = 377 || n = 382 || n = 392 || n = 407 || n = 412 || n = 417 || n = 422 || n = 427 || n = 437 || n = 441 || n = 442 || n = 447 || n = 452 || n = 457 || n = 462)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 318)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	iny
	Jpl *-3
	tay
.elseif(n = 321 || n = 361 || n = 436)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 323 || n = 328 || n = 338 || n = 343 || n = 353 || n = 358 || n = 363 || n = 368 || n = 378 || n = 386 || n = 393 || n = 398 || n = 403 || n = 423 || n = 428 || n = 433 || n = 438 || n = 443 || n = 448 || n = 458 || n = 463 || n = 468)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 0) && ((n > 324 && n < 341) || n = 355 || n = 360 || (n > 369 && n < 381) || n = 390 || n = 395 || n = 405 || n = 420 || (n > 429 && n < 451) || n = 465))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 327 || n = 334 || n = 341 || n = 376 || n = 397 || n = 411 || n = 418 || n = 453 || n = 467)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 333 || n = 373 || n = 381)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 346)
	; 9 bytes
	php
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
	plp
.elseif(n = 356 || n = 401)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 383)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
.elseif(n = 388)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 391)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 399)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
.elseif(n = 408)
	; 8 bytes
	tya
	ldy #90 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 409)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 413 || n = 421 || n = 461)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 416)
	; 7 bytes
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
.elseif(n = 459)
	; 7 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
	tay
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 479 || n = 494 || n = 499 || n = 504 || n = 511 || n = 514 || n = 519 || n = 524 || n = 529 || n = 539)
	; 8 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 472 || n = 477 || n = 482 || n = 487 || n = 492 || n = 497 || n = 512 || n = 517 || n = 522 || n = 527 || n = 532 || n = 542 || n = 546)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 473 || n = 478 || n = 483 || n = 496 || n = 498 || n = 503 || n = 508 || n = 513 || n = 518 || n = 528 || n = 538 || n = 543)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 474 || n = 502 || n = 516 || n = 523 || n = 544)
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
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 481 || n = 488 || n = 509 || n = 537)
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
	ldy #68 ;hides 'nop zp'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 486)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 493)
	; 7 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	Jpl *-2
	tay
.elseif(n = 501)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 506 || n = 526 || n = 531)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif(n = 507)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
.elseif(n = 521)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 533 || n = 541)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 534)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 536)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.endif
.else
; 547 to 625
.if(n = 547 || n = 552 || n = 557 || n = 562 || n = 567 || n = 577 || n = 582 || n = 587 || n = 592 || n = 597 || n = 602 || n = 612 || n = 616 || n = 617 || n = 622)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 548)
	; 8 bytes
	tya
	ldy #(n-4)/8 ;hides 'nop zp'
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 5 = 4) && ((n > 548 && n < 560) || n = 569 || n = 574 || (n > 583 && n < 610) || n = 619 || n = 624))
	; 8 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 553 || n = 563 || n = 568 || n = 578 || n = 583 || n = 588 || n = 598 || n = 603 || n = 608 || n = 618 || n = 623)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 556)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 558 || n = 572 || n = 579 || n = 593)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
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
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	dey
	Jpl *-3
	tay
.elseif(n = 566 || n = 571 || n = 576 || n = 591 || n = 596 || n = 601)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif((n .mod 5 = 0) && ((n > 569 && n < 586) || n = 595 || (n > 614 && n < 626)))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 573)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 581 || n = 613)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 606)
	; 8 bytes
	tya
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 611)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
	tax
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if(n = 626)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 627 || n = 632 || n = 637 || n = 647 || n = 652 || n = 657 || n = 662 || n = 667 || n = 672 || n = 682 || n = 686 || n = 687 || n = 697 || n = 702)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 628 || n = 649 || n = 663 || n = 684 || n = 698)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 629 || n = 634 || n = 639 || n = 644 || n = 654 || n = 659 || n = 664 || n = 669 || n = 674 || n = 679 || n = 689 || n = 692 || n = 699)
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
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jmi *-3
	plp
.elseif(n = 633)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 701)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 636 || n = 638 || n = 643 || n = 648 || n = 658 || n = 668 || n = 673 || n = 683 || n = 691 || n = 703)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 641)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 642 || n = 656 || n = 677)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 646 || n = 666)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif(n = 651)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 653 || n = 661)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 671)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 676 || n = 681)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 678)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
.elseif(n = 693)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 694)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 696)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 701)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.endif
.else
; 704 to 781
.if(n = 704 || n = 709 || n = 714 || n = 721 || n = 724 || n = 729 || n = 734 || n = 739 || n = 744 || n = 749 || n = 756 || n = 764 || n = 769 || n = 774 || n = 779)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 0) && ((n > 704 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 706)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif((n .mod 5 = 2) && (n = 707 || (n > 721 && n < 743) || (n > 756 && n < 778)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 707 && n < 729) || n = 738 || n = 743 || (n > 752 && n < 764) || n = 778))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 711 || n = 746 || n = 751 || n = 766 || n = 771 || n = 776)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif(n = 712 || n = 719 || n = 733 || n = 747 || n = 754 || n = 761 || n = 768)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 716)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 717)
	; 7 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	tay
.elseif(n = 726)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 731)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
.elseif(n = 736)
	; 9 bytes
	php
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jpl *-3
	plp
.elseif(n = 741 || n = 781)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 748)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 752)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
.elseif(n = 759)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 773)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.endif
.endif
.else
; 782 to 937
.if(n = 782 || n = 789 || n = 824 || n = 838 || n = 852 || n = 887 || n = 922 || n = 929 || n = 936)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && ((n > 782 && n < 799) || n = 808 || (n > 817 && n < 834) || n = 843 || n = 848 || (n > 857 && n < 869) || (n > 877 && n < 889) || n = 898 || (n > 912 && n < 929)))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 4) && (n = 784 || n = 794 || n = 799 || (n > 808 && n < 820) || (n > 828 && n < 855) || (n > 868 && n < 890) || (n > 898 && n < 925) || n = 934))
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
.elseif(n = 786 || n = 811 || n = 816 || n = 841 || n = 846 || n = 856 || n = 871 || n = 876 || n = 891 || n = 906 || n = 911 || n = 916 || n = 926)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif(n = 787 || n = 791 || n = 792 || n = 797 || n = 802 || n = 807 || n = 812 || n = 822 || n = 826 || n = 827 || n = 832 || n = 837 || n = 842 || n = 847 || n = 862 || n = 867 || n = 872 || n = 877 || n = 882 || n = 892 || n = 897 || n = 902 || n = 907 || n = 912 || n = 917 || n = 927 || n = 931 || n = 932 || n = 937)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 796 || n = 803 || n = 817 || n = 831 || n = 859 || n = 866 || n = 873 || n = 894 || n = 901 || n = 908)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 801)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	dey
	Jpl *-4
	tay
.elseif(n = 804)
	; 8 bytes
	tya
	ldy #(n-4)/8 ;hides 'nop zp'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 806 || n = 851 || n = 896)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 813)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 821 || n = 853 || n = 861 || n = 893)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 836)
	; 9 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	iny
	Jne *-2
	pla
	tay
.elseif(n = 857)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 864)
	; 7 bytes
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
.elseif(n = 881)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 886)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 903)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 921)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 933)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 948 || n = 953 || n = 958 || n = 963 || n = 968 || n = 983 || n = 988 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1021 || n = 1028 || n = 1033 || n = 1043 || n = 1058 || n = 1063 || n = 1068 || n = 1073 || n = 1078 || n = 1088)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 939)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jpl *-3
	tay
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
.elseif(n = 942 || n = 947 || n = 952 || n = 967 || n = 972 || n = 977 || n = 982 || n = 987 || n = 997 || n = 1001 || n = 1002 || n = 1007 || n = 1012 || n = 1017 || n = 1032 || n = 1036 || n = 1037 || n = 1042 || n = 1047 || n = 1052 || n = 1057 || n = 1067 || n = 1071 || n = 1072 || n = 1077 || n = 1082 || n = 1087 || n = 1092)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && (n = 943 || (n > 956 && n < 979) || (n > 991 && n < 1014) || (n > 1026 && n < 1049) || (n > 1061 && n < 1084)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 969 || n = 974 || n = 979 || n = 989 || n = 994 || n = 1004 || n = 1009 || n = 1014 || n = 1019 || n = 1022 || n = 1029 || n = 1039 || n = 1044 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1079 || n = 1084 || n = 1089)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 946 || n = 951 || n = 956 || n = 991 || n = 1016 || n = 1081 || n = 1086 || n = 1091)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif(n = 961 || n = 996 || n = 1046 || n = 1051 || n = 1066)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
.elseif(n = 973 || n = 1053 || n = 1061 || n = 1093)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 976)
	; 9 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	dey
	Jne *-2
	pla
	tay
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
.elseif(n = 986)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1011)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jpl *-4
	tay
.elseif(n = 1018)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
.elseif(n = 1023)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1024)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	inc @zptemp,x
	tax
.elseif(n = 1026)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1031)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 1038)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 1056)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
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
.if((n .mod 5 = 4) && (n = 1094 || (n > 1108 && n < 1135) || (n > 1143 && n < 1170) || (n > 1178 && n < 1205) || (n > 1213 && n < 1240) || n = 1249))
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
.elseif((n .mod 5 = 1) && (n = 1096 || n = 1116 || (n > 1125 && n < 1137) || n = 1151 || n = 1156 || n = 1171 || (n > 1190 && n < 1207) || n = 1226 || n = 1231))
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif((n .mod 7 = 5) && ((n > 1096 && n < 1119) || (n > 1131 && n < 1154) || (n > 1166 && n < 1189) || (n > 1201 && n < 1224) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && ((n > 1097 && n < 1114) || n = 1123 || (n > 1137 && n < 1149) || (n > 1157 && n < 1169) || n = 1178 || n = 1183 || (n > 1192 && n < 1209) || n = 1218 || (n > 1227 && n < 1249)))
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1102 || n = 1106 || n = 1107 || n = 1112 || n = 1117 || n = 1122 || n = 1127 || n = 1137 || n = 1142 || n = 1147 || n = 1152 || n = 1157 || n = 1162 || n = 1172 || n = 1176 || n = 1177 || n = 1182 || n = 1187 || n = 1192 || n = 1197 || n = 1207 || n = 1212 || n = 1217 || n = 1222 || n = 1227 || n = 1232 || n = 1242 || n = 1246 || n = 1247)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1121 || n = 1166)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 1128)
	; 8 bytes
	tya
	ldy #250 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1133 || n = 1141 || n = 1173 || n = 1213 || n = 1221)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1161 || n = 1186)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 1211)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1236)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	nop
	Jne *-3
	tay
.elseif(n = 1241)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
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
.if(n = 1251 || n = 1258 || n = 1272 || n = 1279 || n = 1286)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1252 || n = 1257 || n = 1262 || n = 1267 || n = 1277 || n = 1281 || n = 1282 || n = 1287)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1253 || n = 1261)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
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
.elseif(n = 1256)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 5 = 4) && ((n > 1258 && n < 1275) || n = 1284 || n = 1289))
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 1263)
	; 8 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1266 || n = 1271 || n = 1276)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif((n .mod 5 = 3) && ((n > 1267 && n < 1289)))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1285)
	; 7 bytes
	tya
	ldy #0
	dey
	Jne *-1
	tay
.endif
.else
; 1290 to 1328
.if(n = 1290)
	; 9 bytes
	php
	sec
	lda #0
	sbc #1
	Jne *-2
	plp
.elseif(n = 1291)
	; 7 bytes
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
.elseif(n = 1292 || n = 1298)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif((n .mod 7 = 5) && ((n > 1292 && n < 1329)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1294 || n = 1320 || n = 1324)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1295 || n = 1304 || n = 1311)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1296)
	; 9 bytes
	php
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1297)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1299)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $70A0'
	ldy #112
	iny
	Jne *-4
	tay
.elseif((n .mod 8 = 5) && ((n > 1300 && n < 1326)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1302 || n = 1316 || n = 1323)
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
.elseif(n = 1305 || n = 1315)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1306)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1308)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $91A0'
	ldy #145
	dey
	Jne *-4
	tay
.elseif(n = 1310 || n = 1319)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1312)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	iny
	iny
	Jne *-4
	tay
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
	bit <$2C ;hides 'bit $6EA0'
	ldy #110
	iny
	Jne *-4
	tay
.elseif(n = 1322)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1326)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
.elseif(n = 1327)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $93A0'
	ldy #147
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1332 || n = 1339 || n = 1360 || n = 1367)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1330 || n = 1344 || n = 1348 || n = 1351 || n = 1358 || n = 1359)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1338 || n = 1352 || n = 1366)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1333 || n = 1341 || n = 1357 || n = 1365)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1334)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	iny
	Jpl *-5
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
	bit <$2C ;hides 'bit $6CA0'
	ldy #108
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1336 && n < 1365)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1340)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
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
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1347)
	; 9 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-3
	tay
.elseif(n = 1350)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1353)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $6AA0'
	ldy #106
	iny
	Jne *-4
	tay
.elseif(n = 1354)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $96A0'
	ldy #150
	dey
	Jne *-4
	tay
.elseif(n = 1361)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1362)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $97A0'
	ldy #151
	dey
	Jne *-4
	tay
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1376 || n = 1387 || n = 1392 || n = 1401 || n = 1406)
	; 8..9 bytes
	pha
	delay_n (n-3)
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
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $68A0'
	ldy #104
	iny
	Jne *-4
	tay
.elseif(n = 1372 || n = 1379 || n = 1383 || n = 1386)
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
.elseif(n = 1374 || n = 1388 || n = 1393 || n = 1402)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1375 || n = 1395)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1378)
	; 9 bytes
	tya
	ldy #250 ;hides 'nop'
	nop
	dey
	dey
	Jne *-4
	tay
.elseif(n = 1380)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	tax
.elseif(n = 1382 || n = 1400)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1385)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1389)
	; 7 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	tay
.elseif(n = 1390)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $9AA0'
	ldy #154
	dey
	Jne *-4
	tay
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
	delay_n (n-4)
	pla
.elseif(n = 1399)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
.elseif(n = 1403)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
.elseif(n = 1404)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1423 || n = 1428 || n = 1441)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1408)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1409 || n = 1418 || n = 1436)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1410 || n = 1431)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1411 || n = 1432)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1441)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 8 = 5) && ((n > 1412 && n < 1446)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1414 || n = 1420 || n = 1435 || n = 1439 || n = 1442)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1415 || n = 1425)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1416)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $63A0'
	ldy #99
	iny
	Jne *-4
	tay
.elseif(n = 1417)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $63A0'
	ldy #99
	iny
	Jne *-4
	tay
.elseif(n = 1422 || n = 1427)
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
.elseif(n = 1430)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	sta (@ptrtemp,x)
	tax
.elseif(n = 1434)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $61A0'
	ldy #97
	iny
	Jne *-4
	tay
.elseif(n = 1438)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1443)
	; 9 bytes
	txa
	ldx #<$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
	tax
.elseif(n = 1444)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	tay
.endif
.else
; 1446 to 1484
.if(n = 1446)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1446 && n < 1483)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1448 || n = 1450 || n = 1464 || n = 1478)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1449 || n = 1456 || n = 1470 || n = 1474 || n = 1483 || n = 1484)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1451 || n = 1458 || n = 1473)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1452)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $A1A0'
	ldy #161
	dey
	Jne *-4
	tay
.elseif(n = 1453 || n = 1469 || n = 1477)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1455 || n = 1465)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1457)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1459)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jpl *-4
	tay
.elseif(n = 1460)
	; 10 bytes
	txa
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	inc @zptemp,x
	tax
.elseif(n = 1462)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
.elseif(n = 1463 || n = 1472 || n = 1481)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1466)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1467)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	inc @zptemp
	dey
	Jne *-4
	tay
.elseif(n = 1471)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $A3A0'
	ldy #163
	dey
	Jne *-4
	tay
.elseif(n = 1476)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1479)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
.elseif(n = 1480)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if((n .mod 8 = 5) && ((n > 1484 && n < 1510)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1486 || n = 1514 || n = 1521)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1487 || n = 1491 || n = 1498 || n = 1502 || n = 1505 || n = 1511 || n = 1512 || n = 1518 || n = 1519)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $A5A0'
	ldy #165
	dey
	Jne *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1518)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1490 || n = 1499)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1492 || n = 1504 || n = 1513)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1494)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1495 || n = 1515)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1497)
	; 8 bytes
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1500)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1506)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $A7A0'
	ldy #167
	dey
	Jne *-4
	tay
.elseif(n = 1507)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $A7A0'
	ldy #167
	dey
	Jne *-4
	tay
.elseif(n = 1508)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	iny
	Jne *-3
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
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
	tax
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
.elseif((n .mod 8 = 5) && ((n > 1524 && n < 1558)))
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
.elseif(n = 1527 || n = 1540 || n = 1547 || n = 1554 || n = 1561)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1528 || n = 1537 || n = 1546 || n = 1556)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1529)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1530)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1532 || n = 1548)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1534)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $AAA0'
	ldy #170
	dey
	Jne *-4
	tay
.elseif(n = 1536)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1539)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1542)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1543)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $ABA0'
	ldy #171
	dey
	Jne *-4
	tay
.elseif(n = 1550)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1551)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1555)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1558)
	; 10 bytes
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
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
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif(n = 1564)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 1565 || n = 1581 || n = 1589 || n = 1597)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1602)))
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
.elseif(n = 1568 || n = 1575 || n = 1582 || n = 1583 || n = 1596 || n = 1599)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1569)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
.elseif(n = 1570)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
.elseif(n = 1571 || n = 1598)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1572)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1574)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 1576 || n = 1590 || n = 1592)
	; 9 bytes
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
.elseif(n = 1578)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
.elseif(n = 1579)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
.elseif(n = 1584 || n = 1591 || n = 1593)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1585 || n = 1595)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1586)
	; 10 bytes
	txa
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	inc @zptemp,x
	tax
.elseif(n = 1588)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B0A0'
	ldy #176
	dey
	Jne *-4
	tay
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 1603 || n = 1610 || n = 1624 || n = 1638 || n = 1639)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1604 || n = 1616 || n = 1630)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1605 || n = 1621 || n = 1637)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1606)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	dey
	Jne *-4
	tay
.elseif(n = 1607 || n = 1625)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1607 && n < 1637)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1609 || n = 1612 || n = 1617 || n = 1626 || n = 1631 || n = 1640)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1611)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	inc @zptemp
	iny
	Jne *-4
	tay
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
	php
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
	plp
.elseif(n = 1619)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1620 || n = 1634)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1623)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $B4A0'
	ldy #180
	dey
	Jne *-4
	tay
.elseif(n = 1627)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
.elseif(n = 1628)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1632)
	; 9 bytes
	php
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
	plp
.elseif(n = 1633)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1635)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1642)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B6A0'
	ldy #182
	dey
	Jne *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1679)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1644 || n = 1656 || n = 1662 || n = 1667 || n = 1672 || n = 1674)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1645 || n = 1654)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1646)
	; 9 bytes
	php
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1647)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
.elseif(n = 1648)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1649)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1651)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B7A0'
	ldy #183
	dey
	Jne *-4
	tay
.elseif(n = 1652 || n = 1670 || n = 1679)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 8 = 5) && ((n > 1652 && n < 1678)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1655 || n = 1665 || n = 1675)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1658)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
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
	bit <$2C ;hides 'bit $B8A0'
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
.elseif(n = 1666 || n = 1673)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1668)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $B9A0'
	ldy #185
	dey
	Jne *-4
	tay
.elseif(n = 1676)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.endif
.else
; 1680 to 1718
.if(n = 1680 || n = 1687 || n = 1694 || n = 1707 || n = 1708)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1681 || n = 1702 || n = 1712 || n = 1716)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1682 || n = 1689 || n = 1703 || n = 1710)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1683)
	; 9 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 1684 || n = 1698)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif((n .mod 7 = 5) && ((n > 1684 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
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
.elseif(n = 1688 || n = 1697 || n = 1715)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1690)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1691)
	; 10 bytes
	txa
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	inc @zptemp,x
	tax
.elseif((n .mod 8 = 5) && ((n > 1692 && n < 1718)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1695 || n = 1705)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1696)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
.elseif(n = 1700)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1704)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
.elseif(n = 1711)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 1714)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	dey
	Jne *-4
	tay
.elseif(n = 1718)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop $48 ;hides 'pha'
	iny
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719 || n = 1722 || n = 1726 || n = 1729 || n = 1736 || n = 1743 || n = 1747 || n = 1750)
	; 9..10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1723)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $BFA0'
	ldy #191
	dey
	Jne *-4
	tay
.elseif(n = 1724 || n = 1742 || n = 1751)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1725 || n = 1733 || n = 1749 || n = 1757)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1728 || n = 1730 || n = 1737)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1731)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
.elseif(n = 1732)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
.elseif(n = 1735)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1738 || n = 1752)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1739)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1740)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	iny
	Jne *-4
	tay
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
.elseif(n = 1753)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1754)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
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
.if(n = 1758 || n = 1766 || n = 1771 || n = 1774 || n = 1791 || n = 1792)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1759)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $3DA0'
	ldy #61
	iny
	Jne *-4
	tay
.elseif(n = 1760 || n = 1778 || n = 1787 || n = 1796)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
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
.elseif(n = 1763)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1764)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	tay
.elseif((n .mod 8 = 5) && ((n > 1764 && n < 1790)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1767)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1768)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1770)
	; 10 bytes
	txa
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 1772)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1775 || n = 1795)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1777)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	iny
	Jne *-4
	tay
.elseif(n = 1779 || n = 1784 || n = 1788)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1782)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	iny
	Jne *-4
	tay
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
.elseif(n = 1798 || n = 1801 || n = 1807 || n = 1833 || n = 1834)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1799 || n = 1819 || n = 1826)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1808 || n = 1816 || n = 1824)
	; 9 bytes
	pha
	delay_n (n-3)
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
	bit <$2C ;hides 'bit $C8A0'
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
.elseif(n = 1809 || n = 1820)
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
.elseif(n = 1811)
	; 10 bytes
	tya
	ldy #128
	delay_n 9
	dey
	Jpl *-4
	tay
.elseif(n = 1812)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $C9A0'
	ldy #201
	dey
	Jne *-4
	tay
.elseif(n = 1814 || n = 1823 || n = 1832)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1815 || n = 1825 || n = 1835)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1817)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1818)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $A5A0'
	ldy #165
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1822)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	dey
	Jne *-4
	tay
.elseif(n = 1827)
	; 10 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1828)
	; 10 bytes
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	iny ;first loop only
	iny
	Jne *-3
	pla
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
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	iny
	Jne *-4
	tay
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	nop
	Jne *-3
	sta (@ptrtemp,x)
	tax
.elseif((n .mod 8 = 5) && ((n > 1836 && n < 1870)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1838)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
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
	tya
	bit <$2C ;hides 'bit $CCA0'
	ldy #204
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1840 && n < 1869)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
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
.elseif(n = 1844)
	; 10 bytes
	php
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 1846 || n = 1851 || n = 1856 || n = 1872)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1847 || n = 1860 || n = 1863 || n = 1870 || n = 1871)
	; 9..10 bytes
	delay_n (n-2)
	nop
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
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
	dey
	Jne *-4
	tay
.elseif(n = 1852 || n = 1873)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1854)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 1855 || n = 1875)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1857)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	dey
	Jne *-4
	tay
.elseif(n = 1858)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	dey
	Jne *-4
	tay
.elseif(n = 1862)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $A9A0'
	ldy #169
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1866)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	dey
	Jne *-4
	tay
.elseif(n = 1867)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	iny
	Jne *-4
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
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	dey
	Jne *-4
	tay
.elseif((n .mod 8 = 5) && ((n > 1876 && n < 1910)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1878 || n = 1882 || n = 1887 || n = 1891 || n = 1898)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 1880 || n = 1888 || n = 1896 || n = 1900)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1881 || n = 1890 || n = 1899)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1883)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif(n = 1884)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1885 && n < 1914)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1889)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1892)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 1894)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	iny
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
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
.elseif(n = 1905)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
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
.elseif(n = 1907)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	iny
	iny
	Jne *-5
	tay
.elseif(n = 1910)
	; 10 bytes
	tya
	ldy #126
	delay_n 10
	dey
	Jpl *-4
	tay
.elseif(n = 1911)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1912)
	; 9 bytes
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	tax
.elseif(n = 1914)
	; 9 bytes
	tya
	ldy #44 ;hides 'bit abs'
	inc @zptemp
	iny
	Jne *-4
	tay
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1935 || n = 1945)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1916 || n = 1927 || n = 1932 || n = 1942 || n = 1943 || n = 1946 || n = 1950 || n = 1951)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 5) && ((n > 1916 && n < 1950)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
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
	ldy #(n-12)/9 ;hides 'nop zp,x'
	iny ;first loop only
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
.elseif(n = 1922 || n = 1931 || n = 1940)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1923 || n = 1928 || n = 1934 || n = 1936 || n = 1944)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1924 || n = 1953)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1926)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
.elseif(n = 1929)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
.elseif(n = 1930)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	dey
	Jne *-4
	tay
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
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	iny
	Jne *-4
	tay
.elseif(n = 1939)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	iny
	Jne *-4
	tay
.elseif(n = 1947)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $D8A0'
	ldy #216
	dey
	Jne *-4
	tay
.elseif(n = 1948)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	iny
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
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
.elseif(n = 1955 || n = 1975)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1956)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	dey
	Jne *-4
	tay
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
.elseif(n = 1959 || n = 1978 || n = 1991)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1960 || n = 1964 || n = 1986 || n = 1988 || n = 1990)
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
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1963)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
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
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
.elseif(n = 1968)
	; 9 bytes
	pha
	tya
	ldy #245
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
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 1971)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 9
	dey
	Jpl *-5
	tay
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
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
.elseif(n = 1977 || n = 1980 || n = 1987)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1979)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1982)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
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
	bit <$2C ;hides 'bit $24A0'
	ldy #36
	iny
	Jne *-4
	tay
.elseif(n = 1992)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $DDA0'
	ldy #221
	dey
	Jne *-4
	tay
.endif
.else
; 1993 to 2031
.if(n = 1993)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	iny
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
.elseif(n = 1995 || n = 2015 || n = 2025)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1996 || n = 2007 || n = 2018)
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
.elseif(n = 1998 || n = 2017)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1999 || n = 2004 || n = 2014 || n = 2023 || n = 2026 || n = 2031)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2000 || n = 2006 || n = 2008 || n = 2016 || n = 2024)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2001)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	dey
	Jne *-4
	tay
.elseif(n = 2002)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	dey
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
.elseif(n = 2009)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2010)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	iny
	Jne *-4
	tay
.elseif(n = 2011)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	iny
	Jne *-4
	tay
.elseif(n = 2019)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	iny
	Jne *-4
	tay
.elseif(n = 2020)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	dey
	Jne *-4
	tay
.elseif(n = 2022)
	; 10 bytes
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2027)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2028)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	iny
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2036 || n = 2044 || n = 2054)
	; 9..10 bytes
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
.elseif(n = 2034 || n = 2041 || n = 2043 || n = 2049 || n = 2052)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2035 || n = 2055 || n = 2065)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
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
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
	tax
.elseif((n .mod 9 = 5) && ((n > 2038 && n < 2067)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2040 || n = 2051 || n = 2062)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2042 || n = 2050 || n = 2059 || n = 2068)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2046)
	; 9 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
.elseif(n = 2047)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	iny
	Jne *-4
	tay
.elseif(n = 2053)
	; 8 bytes
	tya
	ldy #0
	pha
	dey
	Jne *-2
	tay
.elseif(n = 2056)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	dey
	Jne *-4
	tay
.elseif(n = 2058)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-4
	tay
.elseif(n = 2060)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $BBA0'
	ldy #187
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2061)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2063)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2064)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	iny
	Jne *-4
	tay
.elseif(n = 2067)
	; 9 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	pha
	iny
	Jne *-4
	tay
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
.else
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
.if(n = 2071)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop $48,X ;hides 'pha'
	iny
	Jne *-4
	tay
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
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inx
	Jne *-4
	tax
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
.elseif(n = 2077 || n = 2086)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 2078)
	; 10 bytes
	php
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2079)
	; 9 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	iny
	Jne *-4
	tay
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
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
	tax
.elseif(n = 2085)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2087)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2088)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	sta (@ptrtemp,x)
	tax
.elseif(n = 2089)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
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
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	dey
	Jne *-4
	tay
.elseif(n = 2093 || n = 2102)
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
.elseif(n = 2095)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2096)
	; 10 bytes
	php
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2097 || n = 2103 || n = 2104 || n = 2107 || n = 2108)
	; 10 bytes
	delay_n (n-2)
	nop
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
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	dey
	Jne *-4
	tay
.elseif(n = 2101)
	; 9 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	inx
	Jne *-4
	tax
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
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	iny
	Jne *-4
	tay
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
.elseif((n .mod 9 = 7) && ((n > 2112 && n < 2141)))
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 2114)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 10 = 5) && ((n > 2114 && n < 2146)))
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
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
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	iny
	Jne *-4
	tay
.elseif(n = 2123)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2124)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2126)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	nop
	iny
	Jne *-5
	tay
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
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	iny
	Jne *-4
	tay
.elseif(n = 2132)
	; 10 bytes
	php
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2133)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
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
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
.elseif(n = 2139)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2142)
	; 10 bytes
	txa
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 2143 || n = 2148)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2146)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	iny
	Jne *-4
	tay
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2157 || n = 2158 || n = 2166 || n = 2167 || n = 2176 || n = 2179 || n = 2184)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2150 || n = 2161)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2151)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	pha
	inx
	inx
	Jne *-4
	tax
.elseif(n = 2152)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2153 || n = 2177)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2154)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	iny
	Jne *-4
	tay
.elseif(n = 2155 || n = 2175 || n = 2185)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2156 || n = 2165 || n = 2174)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2159)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2160)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2162)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2163)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	dey
	Jne *-4
	tay
.elseif(n = 2164)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	iny
	Jne *-4
	tay
.elseif(n = 2168 || n = 2186)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2169 || n = 2178 || n = 2187)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2170)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $3BA0'
	ldy #59
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2171)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2172)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2173)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2180)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2181)
	; 9 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	tax
.elseif(n = 2182)
	; 9 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	tax
.elseif(n = 2183)
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
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2189 || n = 2213 || n = 2225)
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
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2191 && n < 2220)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2193 || n = 2222)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2194)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2195 || n = 2205 || n = 2215)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2196 || n = 2197 || n = 2206 || n = 2207 || n = 2220 || n = 2221)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2198)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
.elseif(n = 2199)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2200)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
.elseif(n = 2202)
	; 9 bytes
	tya
	ldy #12 ;hides 'nop abs'
	inc @zptemp
	iny
	Jne *-4
	tay
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
.elseif(n = 2208)
	; 8 bytes
	tya
	ldy #(n-12)/9 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 2209)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	dey
	Jne *-4
	tay
.elseif(n = 2211)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	inc @zptemp
	dey
	Jne *-4
	tay
.elseif(n = 2212 || n = 2223)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2214)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2216)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2217)
	; 9 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jne *-4
	tax
.elseif(n = 2218)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
.elseif(n = 2224)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $B9A0'
	ldy #185
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2226)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	tay
.endif
.else
; 2227 to 2265
.if(n = 2227)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2227 && n < 2265)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2229 || n = 2230 || n = 2248 || n = 2251)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2231 || n = 2259)
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
	; 11 bytes
	php
	sec
	lda #247
	nop
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2234)
	; 11 bytes
	tya
	pha
	ldy #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	pla
	tay
.elseif(n = 2235 || n = 2265)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
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
.elseif(n = 2238 || n = 2260)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2239 || n = 2256)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2240 || n = 2241 || n = 2257)
	; 9..10 bytes
	pla
	delay_n (n-4)
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
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 2244)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	tay
.elseif(n = 2245)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	tay
.elseif(n = 2247)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	nop
	iny
	Jne *-4
	tay
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
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	iny
	Jne *-4
	tay
.elseif(n = 2258)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2262)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	tay
.elseif(n = 2263)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266 || n = 2267 || n = 2274 || n = 2277 || n = 2283 || n = 2284 || n = 2292 || n = 2302)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2268 || n = 2286)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2269)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2270)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2271)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
.elseif(n = 2272)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2272 && n < 2301)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2275 || n = 2295)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2276 || n = 2288 || n = 2294 || n = 2296)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2278)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2279)
	; 10 bytes
	pla
	tya
	ldy #(n-9)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2280)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	tay
.elseif(n = 2281)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	tay
.elseif(n = 2285 || n = 2297)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2287 || n = 2301)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2289)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
	tay
.elseif(n = 2290)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
	tay
.elseif(n = 2293 || n = 2304)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2298)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	tay
.elseif(n = 2299)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	tay
.elseif(n = 2303)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	pha
	iny
	Jne *-3
	tay
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305 || n = 2315)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2306 || n = 2312)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2307)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	iny
	Jne *-4
	tay
.elseif(n = 2308)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
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
.elseif(n = 2310 || n = 2311 || n = 2317)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2313)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2314)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2316)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2318)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2319)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2320)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2321)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2322)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 2325 || n = 2335)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
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
	; 10 bytes
	delay_n 8
	delay_n (n-8)
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
.elseif(n = 2336)
	; 10 bytes
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2338)
	; 10 bytes
	tya
	ldy #44 ;hides 'bit abs'
	inc @zptemp
	nop
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
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jpl *-5
	tay
.elseif(n = 2341)
	; 10 bytes
	pla
	delay_n (n-4)
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
; 2344 to 2382
.if(n = 2344)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif((n .mod 12 = 5) && ((n > 2344 && n < 2382)))
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
	ldy #(n-14)/11 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2347 || n = 2349 || n = 2352 || n = 2363 || n = 2374)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2348 || n = 2359 || n = 2370)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2350 || n = 2367 || n = 2372 || n = 2377)
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
	; 11 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2354)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2355 || n = 2365 || n = 2375)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2356)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2358)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2360)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2361)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2362 || n = 2373)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2364 || n = 2376)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2366)
	; 11 bytes
	php
	txa
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 2368)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2371)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2378)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2379)
	; 9 bytes
	tya
	ldy #58 ;hides 'nop'
	inc @zptemp
	iny
	Jne *-4
	tay
.elseif(n = 2380)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2382)
	; 10 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	nop
	pha
	iny
	Jne *-4
	tay
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2387 || n = 2394 || n = 2397 || n = 2407 || n = 2416 || n = 2419)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2410)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2385 || n = 2395 || n = 2415)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2386)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2388 || n = 2398)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2389)
	; 10 bytes
	pla
	tya
	ldy #(n-9)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2390)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2391)
	; 9 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	dey
	dey
	Jne *-4
	tay
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
.elseif(n = 2396)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2399)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2400)
	; 11 bytes
	delay_n (n-4)
	pla
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
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2403)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	inc @zptemp
	iny
	Jne *-4
	tay
.elseif(n = 2404)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2406)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2408)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	iny
	Jne *-5
	tay
.elseif(n = 2409 || n = 2421)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2411)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
.elseif(n = 2412)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2413)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2418)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2420)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
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
	ldy #36 ;hides 'bit zp'
	nop $48,X ;hides 'pha'
	iny
	Jne *-4
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
.elseif((n .mod 10 = 5) && ((n > 2424 && n < 2456)))
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2426 || n = 2431 || n = 2437 || n = 2438 || n = 2457 || n = 2460)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2427)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2428)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2429 || n = 2441 || n = 2453)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2430)
	; 11 bytes
	php
	sec
	lda #242
	inc @zptemp
	sbc #1
	Jne *-4
	plp
.elseif(n = 2432 || n = 2442 || n = 2454)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2433 || n = 2440 || n = 2443)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2434)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $DDA0'
	ldy #221
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2436 || n = 2447 || n = 2458)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2439)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 2444)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2446)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 2448)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
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
.elseif(n = 2451)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2452)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $CCA0'
	ldy #204
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2456)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2459)
	; 10 bytes
	pla
	tya
	ldy #(n-9)/10
	nop
	pha
	dey
	Jne *-3
	tay
.endif
.else
; 2461 to 2500
.if(n = 2461)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
.elseif(n = 2462 || n = 2488)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2463)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2464)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
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
.elseif(n = 2466 || n = 2472 || n = 2492)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2467 || n = 2471 || n = 2479 || n = 2482 || n = 2487 || n = 2493)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2468)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2469 || n = 2480 || n = 2491)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
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
.elseif(n = 2473 || n = 2481 || n = 2484)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2474)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	nop
	nop
	dey
	Jne *-6
	tay
.elseif(n = 2475 || n = 2485 || n = 2495)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2476)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2478)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2483 || n = 2497)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2486)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $BFA0'
	ldy #191
	nop
	nop
	dey
	Jne *-6
	tay
.elseif(n = 2490)
	; 10 bytes
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2494)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2496)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2498)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2499)
	; 10 bytes
	pla
	tya
	ldy #(n-9)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2500)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	inx
	Jne *-5
	tax
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
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2502 || n = 2524)
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
.elseif(n = 2504 || n = 2521 || n = 2538)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2505 || n = 2515 || n = 2535)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2506 || n = 2516 || n = 2528)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2507 || n = 2529)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2508 || n = 2518)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2509)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	inc @zptemp
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2510)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2511 || n = 2539)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2512)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2514)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2517 || n = 2526 || n = 2527)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2519)
	; 10 bytes
	pla
	tya
	ldy #(n-9)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2520)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2522)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2523)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2530)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2531)
	; 11 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	pla
	tay
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
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2536)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	pha
	iny
	Jne *-5
	tay
.endif
.else
; 2540 to 2578
.if(n = 2540)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2541 || n = 2548 || n = 2550)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2542 || n = 2554)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2543)
	; 11 bytes
	txa
	ldx #(n-11)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
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
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2546 || n = 2557 || n = 2568)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2547)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop zp,x'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2549 || n = 2561 || n = 2573)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2551 || n = 2563 || n = 2574 || n = 2575)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2552 || n = 2558 || n = 2560 || n = 2562 || n = 2564 || n = 2571 || n = 2576)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2553 || n = 2567)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
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
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2559)
	; 9 bytes
	tya
	ldy #(n-15)/12 ;hides 'nop zp,x'
	iny ;first loop only
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2565)
	; 9 bytes
	tya
	ldy #0
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2566)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2569)
	; 10 bytes
	pla
	tya
	ldy #0
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2570)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2572)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
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
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	nop
	iny
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 2580 || n = 2591 || n = 2593 || n = 2604)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2581 || n = 2595)
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
.elseif(n = 2583)
	; 10 bytes
	pla
	tya
	ldy #234
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2584)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2585 || n = 2597 || n = 2609)
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
.elseif(n = 2587 || n = 2611 || n = 2614)
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
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2590 || n = 2601 || n = 2612)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2592)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2594 || n = 2602 || n = 2607 || n = 2613 || n = 2616)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2596)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2598)
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
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	nop
	iny
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
.elseif(n = 2608)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 2615)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2617)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C9A0'
	ldy #201
	nop
	nop
	dey
	Jne *-6
	tay
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2631)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2619)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	inc @zptemp
	dey
	Jne *-4
	tay
.elseif(n = 2620)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	pha
	dey
	Jne *-5
	tay
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
	bit <$2C ;hides 'bit $12A0'
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
.elseif(n = 2628 || n = 2652)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2629)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 2630)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2632)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2635 || n = 2636 || n = 2639 || n = 2647)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2637 || n = 2651)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2638 || n = 2649)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2640)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2641)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2642)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 6
	Jne *-4
	tax
.elseif(n = 2643)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2650)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 2653)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
.elseif(n = 2654)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2655)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2658 || n = 2659 || n = 2671 || n = 2680 || n = 2683 || n = 2695)
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
.elseif(n = 2661 || n = 2673 || n = 2682)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2662)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2663)
	; 11 bytes
	txa
	ldx #(n-11)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 2664)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2665 || n = 2679)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2666)
	; 10 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2667 || n = 2678 || n = 2689)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
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
.elseif(n = 2672 || n = 2692)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2674 || n = 2691)
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
.elseif(n = 2677)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2684)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2685)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
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
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2690)
	; 10 bytes
	tya
	ldy #12 ;hides 'nop abs'
	inc @zptemp
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2694)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	nop
	nop
	dey
	Jne *-6
	tay
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
.elseif(n = 2697 || n = 2704 || n = 2715 || n = 2726)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2698)
	; 9 bytes
	tya
	ldy #(n-14)/11 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2699)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	nop
	dey
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
	pha
	delay_n (n-3)
.elseif(n = 2702 || n = 2713 || n = 2718 || n = 2719 || n = 2723 || n = 2724 || n = 2731)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2703)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2705 || n = 2717 || n = 2729)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2706)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 2707 || n = 2721)
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
.elseif(n = 2709)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2710)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2712)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2714)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dey
	Jne *-5
	tay
.elseif(n = 2716)
	; 10 bytes
	tya
	ldy #n/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 2720)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2727)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	pha
	inx
	Jne *-5
	tax
.elseif(n = 2728)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2730)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2732)
	; 10 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2734)
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
.elseif(n = 2736 || n = 2747)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2737)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2738)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	nop
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2739)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 2740)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	pha
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
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2748)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2750)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 2751)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2752)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	pha
	iny
	Jne *-5
	tay
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
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
.elseif(n = 2756 || n = 2769)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2757)
	; 10 bytes
	tya
	ldy #(n-1)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2758)
	; 10 bytes
	tya
	ldy #(n-2)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2759)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2762)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 2763)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	inc @zptemp
	iny
	Jne *-4
	tay
.elseif(n = 2764)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2770)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2771)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 2772)
	; 10 bytes
	tya
	ldy #(n-16)/13 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2773)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2800)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2775)
	; 9 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 2776)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
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
.elseif(n = 2780 || n = 2795)
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
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2783)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2784)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2785)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2786)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2787)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	inc @zptemp
	iny
	Jne *-4
	tay
.elseif(n = 2788 || n = 2799 || n = 2810)
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
.elseif(n = 2792 || n = 2802 || n = 2804 || n = 2806)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2793)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2796)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2797)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2798)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2803)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 2808)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2809)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2811)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	inc @zptemp
	dey
	Jne *-4
	tay
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
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2814 || n = 2829 || n = 2841)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2815 || n = 2822 || n = 2827 || n = 2839 || n = 2851)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2828 || n = 2838 || n = 2848 || n = 2850)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2817 || n = 2836)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2818 || n = 2830 || n = 2842)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2819)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2820)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
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
.elseif(n = 2823)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	pha
	iny
	Jne *-4
	tay
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
.elseif(n = 2831 || n = 2843)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2832)
	; 11 bytes
	delay_n (n-4)
	pla
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
.elseif(n = 2835)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2840)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
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
	tya
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 2847)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	iny
	Jne *-4
	tay
.endif
.else
.if((n >= 2852) && (n < 2871))
; 2852 to 2870
.if(n = 2852)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2853)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2854 || n = 2866)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2855)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2856)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	iny
	Jne *-5
	tay
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
.elseif(n = 2859)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
.elseif(n = 2860)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2861)
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
.elseif(n = 2863)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2864)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
.elseif(n = 2865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2867)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2868)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2869)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 2870)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.endif
.else
; 2871 to 2890
.if(n = 2871)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2872)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2873 || n = 2885)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2874)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2875 || n = 2889)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2876 || n = 2888)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2877)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
.elseif(n = 2878)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2879)
	; 11 bytes
	txa
	ldx #(n-11)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 2880)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	tay
.elseif(n = 2881)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2882)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 2883)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2884)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2886 || n = 2887)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2890)
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
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891 || n = 2904)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2892 || n = 2928)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2893)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2894 || n = 2918 || n = 2922 || n = 2924)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2895 || n = 2912)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2896)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0FA0'
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
.elseif(n = 2898 || n = 2899 || n = 2906 || n = 2911 || n = 2923)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2900 || n = 2915)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2901 || n = 2913)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2902 || n = 2914 || n = 2926)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2903 || n = 2917)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
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
.elseif(n = 2907)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	pha
	inx
	Jne *-5
	tax
.elseif(n = 2908)
	; 10 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	pha
	inx
	Jne *-5
	tax
.elseif(n = 2910)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	dex
	dex
	Jne *-6
	tax
.elseif(n = 2916)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2919)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2920)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2927)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
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
.elseif(n = 2931)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop zp,x'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2932)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
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
.elseif(n = 2934 || n = 2936 || n = 2948)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2935 || n = 2947)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2937)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2938)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2939)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $4848 ;hides two 'pha's
	dey
	Jne *-5
	tay
.elseif(n = 2940)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2941)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2942)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2943)
	; 9 bytes
	tya
	ldy #(n-15)/12 ;hides 'nop zp,x'
	iny ;first loop only
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2944)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	pha
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
.endif
.else
; 2949 to 2968
.if(n = 2949 || n = 2961)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2952)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2953)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 2954)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	nop
	inx
	Jne *-6
	tax
.elseif(n = 2955)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	pha
	inx
	Jne *-5
	tax
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
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	inc @zptemp
	iny
	Jne *-6
	tay
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
.elseif(n = 2962)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2963)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2964)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2965)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2966)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 2967)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2968)
	; 10 bytes
	tya
	ldy #n/14 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
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
.elseif(n = 2970)
	; 10 bytes
	tya
	ldy #(n-2)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2971 || n = 2983 || n = 2989 || n = 2995 || n = 3002 || n = 3007)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2972 || n = 2978 || n = 2984 || n = 2996)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2973 || n = 2987 || n = 3001)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2974 || n = 2986 || n = 2998)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2975 || n = 2990)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2976 || n = 3000)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2977)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2979)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	pha
	dey
	Jne *-5
	tay
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
.elseif(n = 2985)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 2991)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2992)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2994)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	nop
	inx
	Jne *-6
	tax
.elseif(n = 2999)
	; 11 bytes
	txa
	ldx #(n-11)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 3003)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	inc @zptemp
	dey
	Jne *-4
	tay
.elseif(n = 3004)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
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
.elseif(n = 3009 || n = 3023 || n = 3033)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3010 || n = 3046)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 3011)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 3012 || n = 3024)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3013)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3014)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3015 || n = 3043)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3016)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
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
.elseif(n = 3018 || n = 3022 || n = 3032 || n = 3038 || n = 3044)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3025)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3026)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3027)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3028)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3030 || n = 3031)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3036)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3037)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3039)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3040)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3042)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3045)
	; 9 bytes
	tya
	ldy #(n-3)/13 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047 || n = 3060 || n = 3073)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3048)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
	tax
.elseif(n = 3049 || n = 3052 || n = 3054 || n = 3059 || n = 3081)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3050)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3051 || n = 3056 || n = 3068 || n = 3074 || n = 3080)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3053 || n = 3065)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3055)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3057 || n = 3071 || n = 3085)
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
.elseif(n = 3061)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3062)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3063)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3064 || n = 3083)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3066 || n = 3067 || n = 3079)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3069)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
.elseif(n = 3070)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 3072)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3075)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3076)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
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
.elseif(n = 3078)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3082)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3087 || n = 3088 || n = 3096 || n = 3100 || n = 3101 || n = 3104 || n = 3114 || n = 3115 || n = 3118)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3089)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3090 || n = 3103 || n = 3106 || n = 3120)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3091)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3092)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3093 || n = 3109)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3094)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3095 || n = 3110)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3097)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	nop
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3098)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3099 || n = 3113)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3102 || n = 3121)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3105 || n = 3125)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3107)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $3EA0'
	ldy #62
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3108)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C2A0'
	ldy #194
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3111)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	nop
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3116)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3117)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3119)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3122)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3123)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	nop
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3124)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	nop
	nop
	iny
	Jne *-6
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
.if(n = 3126)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3127 || n = 3141 || n = 3155)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3128)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3129 || n = 3135 || n = 3142 || n = 3143 || n = 3147 || n = 3152 || n = 3161)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3130 || n = 3148 || n = 3158 || n = 3160)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3131)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 3132)
	; 11 bytes
	php
	stx @zptemp
	lax #12
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 3133 || n = 3150)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3134)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3136)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3137)
	; 11 bytes
	php
	txa
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3138 || n = 3151)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3139)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3140 || n = 3159)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3144)
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 3145)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3146)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3149)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	nop
	inx
	Jne *-6
	tax
.elseif(n = 3153)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3154)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 18
	Jne *-5
	tay
.elseif(n = 3156)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3157)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts
	iny
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
.elseif(n = 3163)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	nop
	nop
	iny
	Jne *-6
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
.if(n = 3165)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3166 || n = 3180)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3167)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3168)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
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
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3172)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3173)
	; 10 bytes
	tya
	ldy #(n-1)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 3174)
	; 10 bytes
	tya
	ldy #(n-2)/13 ;hides 'nop zp,x'
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
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	nop
	nop
	iny
	Jne *-6
	tay
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
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	dey
	Jne *-5
	tay
.elseif(n = 3181)
	; 10 bytes
	tya
	ldy #(n-1)/15 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3182)
	; 10 bytes
	tya
	ldy #(n-2)/15 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
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
.elseif(n = 3186 || n = 3192 || n = 3199)
	; 11 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-16)/13 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3189)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
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
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3193)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3194)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3195)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3196)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	inc @zptemp
	dey
	Jne *-6
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
	ldy #(n-18)/15 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3200)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3202)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	nop
	nop
	iny
	Jne *-6
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
.if(n = 3204)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3206 || n = 3208 || n = 3226 || n = 3233)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3207 || n = 3212 || n = 3213 || n = 3231 || n = 3232 || n = 3241)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3209)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3210)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3211 || n = 3225 || n = 3239)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3214)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3215 || n = 3230)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3216)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3217)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3218 || n = 3235)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3219)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $C9A0'
	ldy #201
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3220)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C9A0'
	ldy #201
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3221)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3222 || n = 3227 || n = 3234)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3223)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3224)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inc @zptemp
	inx
	Jne *-6
	tax
.elseif(n = 3228)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	nop
	inx
	Jne *-6
	tax
.elseif(n = 3229 || n = 3242)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3236)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3237)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	inc @zptemp
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3238)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inc @zptemp
	inx
	Jne *-6
	tax
.elseif(n = 3240)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	nop
	iny
	Jne *-6
	tay
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3257 || n = 3259 || n = 3272)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3244 || n = 3247 || n = 3264 || n = 3270 || n = 3271 || n = 3277)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3245 || n = 3265)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3246)
	; 11 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3248)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3249)
	; 11 bytes
	pla
	tya
	ldy #(n-9)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3250)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3251)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3252 || n = 3269)
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
	ldy #(n-3)/13 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3254 || n = 3273)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3255 || n = 3268)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3256 || n = 3276 || n = 3278)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3258)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3260 || n = 3275)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3261)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3262)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3263)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3266)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3267 || n = 3281)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3274)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3279)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3280)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	nop
	nop
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3298 || n = 3312 || n = 3314)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3283)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3284)
	; 11 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	plp
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
.elseif(n = 3287 || n = 3288 || n = 3291 || n = 3296 || n = 3297 || n = 3313)
	; 11 bytes
	delay_n (n-2)
	nop
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
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3292 || n = 3311)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3293)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
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
.elseif(n = 3299)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3300)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3301 || n = 3317)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3302)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
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
.elseif(n = 3307)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
.elseif(n = 3308)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3310)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3315)
	; 11 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3316)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3318)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	nop
	nop
	iny
	Jne *-6
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
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
.elseif(n = 3322)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	inc @zptemp
	dey
	Jne *-6
	tay
.elseif(n = 3323 || n = 3337 || n = 3351)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3324)
	; 11 bytes
	pla
	tya
	ldy #255
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3325 || n = 3345)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3326 || n = 3328 || n = 3338 || n = 3340 || n = 3357)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3327)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3329 || n = 3341 || n = 3355 || n = 3358)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3330 || n = 3349)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3331)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3332)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	nop
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3333)
	; 10 bytes
	tya
	ldy #0
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3334)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3335 || n = 3350)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3336)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3339 || n = 3346 || n = 3352 || n = 3353 || n = 3356)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3342)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3343)
	; 12 bytes
	txa
	ldx #238
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3344)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3347)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3348)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3354)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3359)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
.if((n >= 3360) && (n < 3379))
; 3360 to 3378
.if(n = 3360)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3361)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3362)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $6EA0'
	ldy #110
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 3363)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3364)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3365)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3366 || n = 3367 || n = 3370 || n = 3373)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3368)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3369)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3371)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3372 || n = 3375)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3374)
	; 11 bytes
	pha
	delay_n (n-3)
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
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3378)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	inc @zptemp
	iny
	Jne *-6
	tay
.endif
.else
; 3379 to 3398
.if(n = 3379 || n = 3393)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3380)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3381 || n = 3397)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3382)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3383)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3384)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3385)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3386)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3387)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3388)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3389)
	; 11 bytes
	php
	stx @zptemp
	lax #13
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 3390)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3391)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inc @zptemp
	inx
	Jne *-6
	tax
.elseif(n = 3392)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	inc @zptemp
	inx
	Jne *-6
	tax
.elseif(n = 3394)
	; 11 bytes
	php
	txa
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3395)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3396)
	; 11 bytes
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3398)
	; 11 bytes
	tya
	ldy #44 ;hides 'bit abs'
	inc @zptemp
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 3399) && (n < 3418))
; 3399 to 3417
.if(n = 3399)
	; 11 bytes
	pla
	tya
	ldy #(n-9)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3400)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3401)
	; 12 bytes
	txa
	ldx #212 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
	inc @zptemp,x
	tax
.elseif(n = 3402)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3403)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3404)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 10
	inx
	inx
	Jne *-6
	tax
.elseif(n = 3405)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3406)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3407)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3408 || n = 3409)
	; 11 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-19)/16 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3412)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3413)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3414)
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3415)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3416)
	; 10 bytes
	tya
	ldy #n/14 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dey
	Jne *-5
	tay
.elseif(n = 3417)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	dey
	Jne *-5
	tay
.endif
.else
; 3418 to 3437
.if(n = 3418)
	; 10 bytes
	tya
	ldy #(n-2)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3419)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	inc @zptemp
	dey
	Jne *-5
	tay
.elseif(n = 3420)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	inc @zptemp
	iny
	Jne *-6
	tay
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
.elseif(n = 3423 || n = 3424 || n = 3430 || n = 3437)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3425)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3426)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3427)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3428)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3429)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3431)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3432)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 3433)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	inc @zptemp
	dey
	Jne *-5
	tay
.elseif(n = 3434)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	inc @zptemp
	dey
	Jne *-6
	tay
.elseif(n = 3436)
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3452 || n = 3457 || n = 3466 || n = 3471)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3439 || n = 3456 || n = 3473)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3440 || n = 3455 || n = 3470)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3441 || n = 3442 || n = 3446 || n = 3450 || n = 3451 || n = 3467 || n = 3472)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3443)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3447)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inc @zptemp
	inx
	Jne *-6
	tax
.elseif(n = 3448)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3449 || n = 3463)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3453)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3454)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
.elseif(n = 3458 || n = 3474)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3459)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $D8A0'
	ldy #216
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3460)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3461)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3462)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3464)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 3468)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3469)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 6
	inx
	Jne *-6
	tax
.elseif(n = 3475)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3476)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.else
.if((n >= 3477) && (n < 3496))
; 3477 to 3495
.if(n = 3477 || n = 3491)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3478)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 3479 || n = 3484 || n = 3487)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3480)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3481)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
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
	tya
	ldy #24 ;hides 'clc'
	nop
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3485)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3486 || n = 3494)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3488)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3489)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 3490)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3492)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3493)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3495)
	; 11 bytes
	pla
	delay_n (n-4)
.endif
.else
; 3496 to 3515
.if(n = 3496)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3497)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3498)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3499)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 3500 || n = 3515)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3501)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3502)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3503)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3504)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3505)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3506)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3507)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3508)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DBA0'
	ldy #219
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3509)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3510)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3511)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3512)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3513)
	; 10 bytes
	tya
	ldy #(n-3)/15 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3514)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	delay_n 6
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516 || n = 3536 || n = 3550)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3517)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3518)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3519 || n = 3533 || n = 3547)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3520 || n = 3539)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3521 || n = 3522 || n = 3526 || n = 3527 || n = 3535 || n = 3549)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3523)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $24A0'
	ldy #36
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3524 || n = 3541)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3525 || n = 3545)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3528)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3529)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3530)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3531)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3532)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3534 || n = 3537)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3538 || n = 3552)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3540)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3542)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3543)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3544)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3546)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3548)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3551)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3553)
	; 12 bytes
	tya
	ldy #253
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3554)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.endif
.else
; 3555 to 3593
.if(n = 3555)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3556)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3557 || n = 3573)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3558 || n = 3592)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3559)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3560 || n = 3590)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3561 || n = 3575)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3562 || n = 3563 || n = 3567 || n = 3576 || n = 3579 || n = 3591)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3564 || n = 3568 || n = 3578 || n = 3580)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3565 || n = 3585)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3566)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3569)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3570)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 3571)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3572)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3574)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3577)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3581 || n = 3593)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3582 || n = 3584)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3583)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	delay_n 9
	inx
	inx
	Jne *-7
	tax
.elseif(n = 3586)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $39A0'
	ldy #57
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3587)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3588)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	inc @zptemp
	dey
	Jne *-6
	tay
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
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3598 || n = 3607 || n = 3611 || n = 3622 || n = 3627 || n = 3628)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3599 || n = 3612 || n = 3629)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3596 || n = 3615)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3597 || n = 3614 || n = 3631)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3600 || n = 3624 || n = 3630)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3601)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3602)
	; 12 bytes
	tya
	ldy #(n-11)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3603)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3604)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3605 || n = 3625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3606)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3608)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3609 || n = 3626)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3610)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3613)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3616)
	; 11 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3617)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3618)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3619)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3620)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3621)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3623)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3632)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.endif
.else
.if((n >= 3633) && (n < 3652))
; 3633 to 3651
.if(n = 3633)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 6
	inx
	Jne *-6
	tax
.elseif(n = 3634)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3635 || n = 3650)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3636)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3637)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3638)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3639)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3640)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 3641)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3642)
	; 12 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 3643)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3644)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3645)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3646)
	; 11 bytes
	php
	stx @zptemp
	lax #14
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 3647)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3648)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3649)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3651)
	; 11 bytes
	php
	txa
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.else
; 3652 to 3671
.if(n = 3652)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3653)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3654)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3655)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 3656 || n = 3658 || n = 3666)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3657)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
	tax
.elseif(n = 3659)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	dey
	Jne *-5
	tay
.elseif(n = 3660)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3661)
	; 10 bytes
	tya
	ldy #(n-1)/15 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3662)
	; 10 bytes
	tya
	ldy #(n-2)/15 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3663)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3664)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3665)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3667)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3668)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3669)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3670)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3671)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	pha
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672 || n = 3691 || n = 3710)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3673)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3674 || n = 3679 || n = 3682 || n = 3687 || n = 3693 || n = 3696 || n = 3697 || n = 3707)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3675)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	tax
.elseif(n = 3676 || n = 3681 || n = 3689 || n = 3703)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3677 || n = 3694)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3678)
	; 10 bytes
	tya
	ldy #(n-18)/15 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3680 || n = 3695)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3683)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3684)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3685 || n = 3705)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3686 || n = 3688 || n = 3698)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3690)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3692)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3699)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3700)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3701)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3702)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tay
.elseif(n = 3704)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3706)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3708)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3709)
	; 10 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	tay
.endif
.else
; 3711 to 3750
.if(n = 3711 || n = 3728 || n = 3745)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3712 || n = 3713 || n = 3726 || n = 3730 || n = 3734 || n = 3735 || n = 3741 || n = 3742 || n = 3746 || n = 3750)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3714)
	; 11 bytes
	pla
	tya
	ldy #(n-9)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3716)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3717 || n = 3733 || n = 3749)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3718)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3719 || n = 3737)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3720)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 9
	adc #1
	Jne *-6
	plp
.elseif(n = 3721)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 3722)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3723)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3724)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 6
	inx
	Jne *-6
	tax
.elseif(n = 3725)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3727)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3729 || n = 3748)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3731)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3732)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3736)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3738)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3739)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3740)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3743)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3744)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3747)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop'
	delay_n 9
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
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
.if((n >= 3751) && (n < 3790))
; 3751 to 3789
.if(n = 3751 || n = 3752 || n = 3759 || n = 3766 || n = 3771)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3753)
	; 10 bytes
	tya
	ldy #(n-3)/15 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3754)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3755 || n = 3770)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3756 || n = 3758 || n = 3773 || n = 3782)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3757 || n = 3769 || n = 3772 || n = 3776 || n = 3787 || n = 3788)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3760)
	; 12 bytes
	sty @zptemp
	ldy #21 ;hides 'ora zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3761)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3762 || n = 3779)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3763)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3764)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3765 || n = 3785)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3767 || n = 3786)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3768)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3774)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3775)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 3777)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 3778)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inc @zptemp
	iny
	Jne *-6
	tay
.elseif(n = 3780)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3781)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3783)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3784)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 3789)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.endif
.else
.if((n >= 3790) && (n < 3809))
; 3790 to 3808
.if(n = 3790 || n = 3804)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3791)
	; 12 bytes
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
.elseif(n = 3792 || n = 3793)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3794)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	nop
	delay_n 6
	iny
	Jne *-7
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
.elseif(n = 3796)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3797)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3798 || n = 3802)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3799)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3800)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3801)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3803)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 3805)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3806)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3807)
	; 11 bytes
	tya
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	nop
.elseif(n = 3808)
	; 11 bytes
	pha
	delay_n (n-3)
.endif
.else
; 3809 to 3828
.if(n = 3809)
	; 11 bytes
	pla
	tya
	ldy #200
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3810)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3811)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3812)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3813)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3814)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3815)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3816)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3817 || n = 3826)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3818)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3819)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop zp,x'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3820)
	; 12 bytes
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 9
	inx
	Jne *-7
	tax
.elseif(n = 3821)
	; 11 bytes
	tya
	ldy #90 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3822)
	; 12 bytes
	tya
	ldy #44 ;hides 'bit abs'
	inc @zptemp
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3823)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 3824)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3827)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3828)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	delay_n 7
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3861)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3830 || n = 3847 || n = 3864)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3831 || n = 3832 || n = 3841 || n = 3849 || n = 3866)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3833 || n = 3855)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3834)
	; 11 bytes
	pla
	tya
	ldy #(n-9)/15
	delay_n 10
	dey
	Jne *-4
	tay
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
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3837)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3838)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3839 || n = 3857)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3840 || n = 3846 || n = 3850 || n = 3858 || n = 3863)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3842)
	; 11 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	dey
	Jne *-6
	tay
.elseif(n = 3843 || n = 3862)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3844)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3845 || n = 3865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3848)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3851)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3853)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3854)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3856)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 3859)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3860)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	tax
.elseif(n = 3867)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3871 || n = 3889 || n = 3895)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3869 || n = 3890)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3870 || n = 3882 || n = 3884 || n = 3888 || n = 3894 || n = 3901)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3872)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3873)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3874)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	iny
	Jne *-6
	tay
.elseif(n = 3875)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3876)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3877 || n = 3893)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3878)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3879)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 3880)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 3881 || n = 3898)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3883 || n = 3887 || n = 3904)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3885 || n = 3905)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3886)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3891)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3892)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3896)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3897)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 3899)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3900)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3902)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3903)
	; 11 bytes
	php
	stx @zptemp
	lax #15
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 3906)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inc @zptemp
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
; 3907 to 3945
.if(n = 3907)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3908)
	; 11 bytes
	php
	txa
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3909 || n = 3941)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3910)
	; 11 bytes
	tya
	ldy #12 ;hides 'nop abs'
	inc @zptemp
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3911)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3912)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3913)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3914)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $AAA0'
	ldy #170
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 3915 || n = 3932)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3916 || n = 3918 || n = 3922 || n = 3926 || n = 3928 || n = 3935)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3917 || n = 3927 || n = 3934)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3919 || n = 3938)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3920 || n = 3937)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3921 || n = 3943)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3923)
	; 10 bytes
	tya
	ldy #(n-19)/16 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3924)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3925 || n = 3945)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3929)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3930)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	delay_n 6
	dex
	dex
	Jne *-6
	tax
.elseif(n = 3931)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $BBA0'
	ldy #187
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3933 || n = 3942)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3936)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3939)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3940)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3944)
	; 12 bytes
	tya
	ldy #(n-11)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.endif
.else
; 3946 to 3984
.if(n = 3946)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 3947)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3948 || n = 3950 || n = 3952 || n = 3960 || n = 3969 || n = 3979 || n = 3984)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3949 || n = 3966 || n = 3983)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3951 || n = 3958 || n = 3959 || n = 3967 || n = 3968 || n = 3975 || n = 3978)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3953 || n = 3974)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3954)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3955)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3956)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3957 || n = 3976)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3961 || n = 3970 || n = 3977 || n = 3980)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3962)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3963)
	; 11 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 13
	iny
	Jne *-6
	tay
.elseif(n = 3964)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 3965)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3971)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3972)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3973)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3981)
	; 10 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 3982)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985 || n = 4005)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3986 || n = 3998 || n = 4006 || n = 4008)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3987)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3988)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3989 || n = 4021)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3990 || n = 3991 || n = 3997 || n = 4002 || n = 4007 || n = 4022 || n = 4023)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3992 || n = 3996)
	; 12..13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3993)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3994)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3995 || n = 4014)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3999 || n = 4018)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4000 || n = 4017)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4001)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4003)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4004)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4009)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4010)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4011)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	nop
	iny
	Jne *-6
	tay
.elseif(n = 4012)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4013)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 4015)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	nop
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 4016)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4019)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4020)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4041 || n = 4043 || n = 4044 || n = 4059)
	; 12..13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4025 || n = 4045)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4026)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	dey
	Jne *-6
	tay
.elseif(n = 4027 || n = 4039 || n = 4047 || n = 4054 || n = 4060)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4028 || n = 4048)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4029 || n = 4038 || n = 4049 || n = 4056 || n = 4062)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4030)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4031)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4032)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 4033 || n = 4052)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4034 || n = 4051)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4035)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4036)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4037 || n = 4053)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4040)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4042)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4046)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4050)
	; 11 bytes
	tya
	ldy #(n-22)/19 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4055)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4057)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4058)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4061)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
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
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	tax
.elseif(n = 4064)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4065)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4066 || n = 4074 || n = 4088 || n = 4093)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4067 || n = 4072 || n = 4089 || n = 4094)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4068 || n = 4085)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4069)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4070 || n = 4073 || n = 4081 || n = 4086 || n = 4087 || n = 4092)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4071 || n = 4090)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4075 || n = 4097)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4076)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4077)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $3EA0'
	ldy #62
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4078)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $C2A0'
	ldy #194
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4079)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4080)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 4082)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4083)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4084)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4091)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4095)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4096)
	; 12 bytes
	txa
	ldx #(n-11)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4098)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $C3A0'
	ldy #195
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4099)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4100)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4101)
	; 11 bytes
	tya
	ldy #0
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.endif
.else
; 4102 to 4140
.if(n = 4102 || n = 4119 || n = 4136)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4103 || n = 4104 || n = 4107 || n = 4111 || n = 4121 || n = 4127 || n = 4130 || n = 4138)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4105 || n = 4125)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4106 || n = 4113 || n = 4123 || n = 4129 || n = 4132)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4108 || n = 4112 || n = 4122 || n = 4137 || n = 4139)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4109 || n = 4128)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4110)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
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
.elseif(n = 4115)
	; 12 bytes
	txa
	ldx #(n-11)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4116)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4117)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	delay_n 6
	inx
	Jne *-7
	tax
.elseif(n = 4118)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4120)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	inx
	inx
	Jne *-6
	tax
.elseif(n = 4124)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4126)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4131)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4133)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4134)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 4135)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4140)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4142 || n = 4163)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4143)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4144)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 9
	inx
	Jne *-7
	tax
.elseif(n = 4145 || n = 4165)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4146)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 4147 || n = 4166)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4148 || n = 4150 || n = 4156 || n = 4164 || n = 4169 || n = 4173)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4149 || n = 4155 || n = 4167 || n = 4168 || n = 4172)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4151 || n = 4157 || n = 4174)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4152 || n = 4154 || n = 4177)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4153 || n = 4170)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4158 || n = 4175)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4159)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4160)
	; 11 bytes
	php
	stx @zptemp
	lax #16
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 4161)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4162)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4171)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4176)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4178)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4179)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.endif
.else
; 4180 to 4218
.if(n = 4180)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4181 || n = 4199 || n = 4217)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4182)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts
	dey
	dey
	Jne *-6
	tay
.elseif(n = 4183)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4184)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4185 || n = 4205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4186 || n = 4189 || n = 4201 || n = 4206)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4187 || n = 4204)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4188 || n = 4190 || n = 4202 || n = 4218)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4191 || n = 4208 || n = 4209)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4192)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4193 || n = 4210)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4194)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4195)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4196 || n = 4200)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4197)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4198)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 9
	inx
	Jne *-7
	tax
.elseif(n = 4203)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4207)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4211 || n = 4212)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4213)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $B7A0'
	ldy #183
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4214)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4215)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 4216)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	delay_n 9
	iny
	Jne *-7
	tay
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
.if((n >= 4219) && (n < 4258))
; 4219 to 4257
.if(n = 4219 || n = 4227 || n = 4231 || n = 4240 || n = 4244 || n = 4257)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4220 || n = 4224 || n = 4228 || n = 4241 || n = 4248 || n = 4256)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4221 || n = 4238 || n = 4255)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4222)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 4223 || n = 4242)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4225 || n = 4245)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4226 || n = 4247)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4229 || n = 4251)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4230)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4232)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4233)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4234)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4235)
	; 11 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4236)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $B8A0'
	ldy #184
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4237)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4239)
	; 12 bytes
	pla
	tya
	ldy #235
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4243)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4246)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.elseif(n = 4249)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4250)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4252)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4253)
	; 10 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 4254)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	nop
	delay_n 6
	iny
	Jne *-7
	tay
.endif
.else
; 4258 to 4296
.if(n = 4258 || n = 4264 || n = 4275 || n = 4288 || n = 4292)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4259 || n = 4270 || n = 4276 || n = 4284 || n = 4293)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4260 || n = 4283)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4261 || n = 4280)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4262 || n = 4279 || n = 4296)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4263 || n = 4267 || n = 4274 || n = 4282 || n = 4287 || n = 4291)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4265 || n = 4285)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4266)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	tax
.elseif(n = 4268)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4269)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4271)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4272 || n = 4289)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4273 || n = 4295)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4277 || n = 4294)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4278)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4281)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $BAA0'
	ldy #186
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4286)
	; 12 bytes
	tya
	ldy #(n-11)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4290)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if(n = 4297 || n = 4301 || n = 4307 || n = 4308 || n = 4319 || n = 4320 || n = 4327 || n = 4333)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4298)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4299 || n = 4318)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4300)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $B3A0'
	ldy #179
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4302 || n = 4309 || n = 4321 || n = 4326 || n = 4334)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4303 || n = 4322)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4304)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4305 || n = 4325)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4306 || n = 4323)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4310 || n = 4331)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4311)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4312)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4313 || n = 4330)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4314)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4315)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4316)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 10
	inx
	Jne *-7
	tax
.elseif(n = 4317)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4324)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4328)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4329)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4332)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4335)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	delay_n 10
	iny
	Jne *-7
	tay
.endif
.else
; 4336 to 4375
.if(n = 4336)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 4337 || n = 4356 || n = 4375)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4338)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	nop
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 4339 || n = 4361)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4340)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4341 || n = 4344 || n = 4349 || n = 4369)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4342 || n = 4347 || n = 4353 || n = 4354 || n = 4358 || n = 4367)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4343)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4345 || n = 4365)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4346)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4348 || n = 4360 || n = 4362 || n = 4368)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4350)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4351)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4352)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4355)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 4357)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4359)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 4363)
	; 11 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4364 || n = 4366)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4370)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4371)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4372)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4373)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 4374)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
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
.if(n = 4376 || n = 4378 || n = 4388 || n = 4406 || n = 4408 || n = 4414)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4377 || n = 4386 || n = 4387 || n = 4396 || n = 4407)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4379 || n = 4397)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4380)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4381 || n = 4400)
	; 12 bytes
	tya
	ldy #(n-11)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4382 || n = 4410 || n = 4412)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4383)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4384)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4385 || n = 4405)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4389 || n = 4398 || n = 4409)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4390)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 4391)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	pha
	dey
	Jne *-6
	tay
.elseif(n = 4392)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 10
	dex
	dex
	Jne *-6
	tax
.elseif(n = 4393)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 4394 || n = 4413)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4395)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop zp,x'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 4399)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	inc @zptemp
	iny
	Jne *-7
	tay
.elseif(n = 4401)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	delay_n 7
	iny
	Jne *-7
	tay
.elseif(n = 4402)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4403)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 4404)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4411)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4436)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4416 || n = 4420 || n = 4428 || n = 4435 || n = 4448 || n = 4452)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4417)
	; 11 bytes
	php
	stx @zptemp
	lax #17
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 4418 || n = 4439)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4419)
	; 11 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4421 || n = 4444)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4422)
	; 11 bytes
	php
	txa
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4423 || n = 4434 || n = 4438 || n = 4446 || n = 4447 || n = 4453)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4424)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4425 || n = 4445)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4426 || n = 4429)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4427)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4430)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4431)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 9
	inx
	Jne *-7
	tax
.elseif(n = 4432 || n = 4451)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4433)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4437)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4440)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4441)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4442)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4443)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4449)
	; 10 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4450)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 9
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454 || n = 4468 || n = 4479 || n = 4481 || n = 4488 || n = 4492)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4455)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4456)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4457 || n = 4478)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4458)
	; 12 bytes
	tya
	ldy #44 ;hides 'bit abs'
	inc @zptemp
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4459 || n = 4467 || n = 4472 || n = 4480 || n = 4487 || n = 4491)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4460)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4461 || n = 4473 || n = 4474)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif((n > 4461 && n < 4465))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4465 || n = 4485)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4466)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $C2A0'
	ldy #194
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4469)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
	tax
.elseif(n = 4470 || n = 4489)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4471)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4475)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4476)
	; 11 bytes
	tya
	ldy #(n-24)/21 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4477)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4482)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	iny
	Jne *-6
	tay
.elseif(n = 4483)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	nop
	nop
	iny
	Jne *-7
	tay
.elseif(n = 4484)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inc @zptemp
	iny
	Jne *-7
	tay
.elseif(n = 4486)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4490)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.endif
.else
; 4493 to 4531
.if(n = 4493 || n = 4515)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4494)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4495 || n = 4501 || n = 4507 || n = 4510 || n = 4529)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4496)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4497)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4498)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4499 || n = 4520)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4500)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4502 || n = 4511 || n = 4526 || n = 4528 || n = 4530)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4503)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 4504)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4505 || n = 4525)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4506)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 4508 || n = 4527)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4509 || n = 4512 || n = 4531)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4513)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4514)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4516)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $BCA0'
	ldy #188
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4517)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4518)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4519)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4521)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4522)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4523)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4524)
	; 12 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 28
	Jne *-7
	tay
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
.if((n >= 4532) && (n < 4551))
; 4532 to 4550
.if(n = 4532 || n = 4534)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4533)
	; 12 bytes
	txa
	ldx #(n-11)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4535)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4536)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4537)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4538 || n = 4543 || n = 4547 || n = 4548)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4539)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4540)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4541)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4542)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4544)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 4545)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4546)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4549)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4550)
	; 11 bytes
	pla
	delay_n (n-4)
.endif
.else
; 4551 to 4570
.if(n = 4551)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4552)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4553)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4554)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4555)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4556)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4557)
	; 11 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4558)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4559)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4560)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4561)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4562)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4563)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $BEA0'
	ldy #190
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4564)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4565)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4566 || n = 4569)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4567)
	; 11 bytes
	tya
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	nop
.elseif(n = 4568)
	; 11 bytes
	pha
	tya
	ldy #240
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4570)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.endif
.endif
.else
; 4571 to 4609
.if(n = 4571)
	; 12 bytes
	tya
	ldy #(n-11)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4572)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4573)
	; 12 bytes
	delay_n (n-4565)
	tya
	ldy #240
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4574 || n = 4578 || n = 4596 || n = 4600)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4575)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4576)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4577 || n = 4595)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4579 || n = 4586 || n = 4587 || n = 4597 || n = 4607)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4580)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-6
	tay
.elseif(n = 4581)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4582)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4583 || n = 4604)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4584 || n = 4603)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4585 || n = 4605)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4588 || n = 4589 || n = 4599 || n = 4609)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4590)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4591 || n = 4592)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4593)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4594)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4598 || n = 4606 || n = 4608)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4601)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 10
	inx
	Jne *-7
	tax
.elseif(n = 4602)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
.if((n >= 4610) && (n < 4629))
; 4610 to 4628
.if(n = 4610)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4611)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4612)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4613)
	; 11 bytes
	tya
	ldy #0
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4614 || n = 4618)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4615 || n = 4624 || n = 4627)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4616)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4617 || n = 4626)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4619)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 4620)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4621)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4622)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4623)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	delay_n 9
	dex
	dex
	Jne *-7
	tax
.elseif(n = 4625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4628)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.endif
.else
; 4629 to 4648
.if(n = 4629)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4630)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4631)
	; 12 bytes
	txa
	ldx #(n-11)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4632)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4633)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4634)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	dey
	Jne *-6
	tay
.elseif(n = 4635)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 4636)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 4637)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4638)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4639)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4640)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4641)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4642 || n = 4644 || n = 4648)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4643)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4645)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4646)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4647)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4664 || n = 4676 || n = 4678)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4650 || n = 4670)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4651)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4652)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4653)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4654 || n = 4663 || n = 4668 || n = 4675 || n = 4677 || n = 4682)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4655 || n = 4680)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 4656)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4657)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	inx
	inx
	Jne *-7
	tax
.elseif(n = 4658)
	; 11 bytes
	tya
	ldy #(n-22)/19 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4659)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4660 || n = 4679)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4661)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4662 || n = 4667 || n = 4671 || n = 4681 || n = 4687)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4665 || n = 4685)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4666)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4669)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4672)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-5
	tax
.elseif(n = 4673)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4674)
	; 11 bytes
	php
	stx @zptemp
	lax #18
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 4683)
	; 11 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4684)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4686)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if(n = 4688 || n = 4709)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4689 || n = 4702 || n = 4721)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4690 || n = 4710)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4691 || n = 4713)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4692 || n = 4724)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4693 || n = 4699 || n = 4700 || n = 4707 || n = 4715 || n = 4719)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4694)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4695)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4696)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $CCA0'
	ldy #204
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4697 || n = 4720)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4698 || n = 4717)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4701 || n = 4708)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4703)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4704)
	; 12 bytes
	txa
	ldx #(n-11)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4705 || n = 4725)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4706)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4711)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4712)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4714 || n = 4726)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4716)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 10
	inx
	Jne *-7
	tax
.elseif(n = 4718)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4722)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4723)
	; 12 bytes
	tya
	ldy #(n-11)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.endif
.else
; 4727 to 4765
.if(n = 4727 || n = 4737 || n = 4738 || n = 4747 || n = 4757)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4728 || n = 4739 || n = 4748 || n = 4756 || n = 4758)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4729 || n = 4740 || n = 4749 || n = 4759)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4730 || n = 4751)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4731)
	; 12 bytes
	txa
	ldx #(n-11)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4732 || n = 4752 || n = 4762)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4733)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4734)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4735)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4736 || n = 4755)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4741 || n = 4760)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4742 || n = 4761)
	; 12 bytes
	txa
	ldx #(n-11)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4743)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4744)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4745 || n = 4765)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4746)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 4750)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 4753)
	; 10 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4754)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4763)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4764)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
; 4766 to 4804
.if(n = 4766 || n = 4789)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4767 || n = 4776 || n = 4787 || n = 4795)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4768 || n = 4775 || n = 4777 || n = 4788 || n = 4796 || n = 4802)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4769 || n = 4778 || n = 4797)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4770)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 4771)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4772)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4773)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4774 || n = 4793)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4779 || n = 4801)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4780)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 4781)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4782)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4783)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4784)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4785)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4786 || n = 4794)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4790)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4791)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4792)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4798)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4799)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4800)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4803)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4804)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.else
; 4805 to 4843
.if(n = 4805 || n = 4825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4806 || n = 4819 || n = 4832 || n = 4838 || n = 4840)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4807 || n = 4824 || n = 4827)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4808 || n = 4815 || n = 4828 || n = 4834 || n = 4836)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4809 || n = 4816 || n = 4829 || n = 4837)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4810 || n = 4830)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4811)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4812 || n = 4831)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4813)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4814 || n = 4835)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4817)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4818)
	; 12 bytes
	txa
	ldx #(n-11)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4820)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4821 || n = 4842)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4822)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4823)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4826)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
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
.elseif(n = 4839)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
.elseif(n = 4841)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4843)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4845 || n = 4865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4846 || n = 4847 || n = 4852 || n = 4871)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4848 || n = 4853 || n = 4859 || n = 4861 || n = 4868 || n = 4872 || n = 4878)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4849 || n = 4860 || n = 4862 || n = 4873)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4850)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4851)
	; 12 bytes
	txa
	ldx #(n-11)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4854)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	tax
.elseif(n = 4855)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 4856 || n = 4877)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4857 || n = 4882)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4858 || n = 4881)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4863)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4864)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4866)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4867)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4869)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4870)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
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
.elseif(n = 4875)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4876)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4879)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4880)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-6
	plp
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	nop
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4884 || n = 4888 || n = 4908 || n = 4914 || n = 4920)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4885 || n = 4905)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4886 || n = 4894 || n = 4895 || n = 4916)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4887 || n = 4891 || n = 4900 || n = 4906 || n = 4907 || n = 4913)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4889 || n = 4911)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4890 || n = 4910)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4892 || n = 4912)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4893)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4896)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4897)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 4898 || n = 4919)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4899)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4901)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4902)
	; 11 bytes
	tya
	ldy #(n-26)/23 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4903)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4904)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4909 || n = 4915 || n = 4921)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4917)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4918)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
; 4922 to 4960
.if(n = 4922)
	; 12 bytes
	php
	sec
	lda #(n-8)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 4923)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4924 || n = 4932 || n = 4952)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4925 || n = 4945)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4926)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4927 || n = 4942 || n = 4947 || n = 4957)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4928 || n = 4934 || n = 4939 || n = 4941 || n = 4943 || n = 4948)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4929 || n = 4935 || n = 4937 || n = 4944 || n = 4949)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4930)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4931)
	; 11 bytes
	php
	stx @zptemp
	lax #19
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 4933 || n = 4955)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4936)
	; 11 bytes
	php
	txa
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4938)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4940)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4946)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4950)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4951)
	; 12 bytes
	tya
	ldy #(n-11)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4953)
	; 11 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 18
	iny
	Jne *-6
	tay
.elseif(n = 4954)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inc @zptemp
	iny
	Jne *-7
	tay
.elseif(n = 4956)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	delay_n 7
	iny
	Jne *-7
	tay
.elseif(n = 4958)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4959)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4960)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.else
; 4961 to 5000
.if(n = 4961 || n = 4982)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4962 || n = 4964 || n = 4968 || n = 4976 || n = 4983 || n = 4988)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4963 || n = 4967 || n = 4975 || n = 4979 || n = 4984 || n = 4987 || n = 4998)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4965 || n = 4985)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4966 || n = 4974)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4969 || n = 4986 || n = 4989)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4970 || n = 4990)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4971)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4972 || n = 4992)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4973 || n = 4996)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4977 || n = 4999)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4978)
	; 12 bytes
	tya
	ldy #200 ;hides 'iny'
	nop
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-7
	tay
.elseif(n = 4980)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4981)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4991)
	; 12 bytes
	tya
	ldy #(n-11)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4993)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4994)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4995)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4997)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5000)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
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
.if(n = 5001)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5002)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5003 || n = 5024)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5004 || (n > 5013 && n < 5017) || (n > 5033 && n < 5040))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5005 || n = 5025)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5006)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 18
	iny
	Jne *-6
	tay
.elseif(n = 5007 || n = 5026 || n = 5027)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5008 || n = 5028)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5009 || n = 5029)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 5010 || n = 5030)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5011 || n = 5031)
	; 12 bytes
	txa
	ldx #(n-11)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 5012 || n = 5032)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5013 || n = 5033)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5017)
	; 11 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5018)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5019)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5020)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5021)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5022)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5023)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.endif
.else
; 5040 to 5078
.if(n = 5040)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5041)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5042)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5043)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5044)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5045 || n = 5065)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5046 || n = 5048 || n = 5068)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5047 || n = 5067)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5049 || n = 5069)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 5050 || n = 5070)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5051 || n = 5071)
	; 12 bytes
	tya
	ldy #(n-11)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 5052 || n = 5072)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5053 || n = 5073)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5054)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5055)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif((n > 5055 && n < 5062) || n = 5074 || n = 5076 || n = 5077)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5062)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5063)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 5064)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5066)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5075)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5078)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 18
	Jne *-6
	tax
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5083 || n = 5084 || (n > 5095 && n < 5101) || n = 5104)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5080)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5081)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5082)
	; 12 bytes
	pla
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	jsr @nearby_rts_14cyc
	Jne *-6
	tax
.elseif(n = 5085 || n = 5105)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5086)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 5087 || n = 5109)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5088 || n = 5111)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5089 || n = 5095)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5090)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5091)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5092 || n = 5112)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5093 || n = 5117)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5094)
	; 12 bytes
	tya
	ldy #44 ;hides 'bit abs'
	delay_n 20
	iny
	Jne *-7
	tay
.elseif(n = 5101)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5102)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5103)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5106)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5107)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 5108)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5110)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5113)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 5114)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5115)
	; 11 bytes
	tya
	ldy #(n-27)/24 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5116)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.endif
.else
; 5118 to 5156
.if(n = 5118 || n = 5128 || n = 5137 || n = 5154)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5119 || n = 5138 || n = 5155)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5120 || n = 5121 || (n > 5141 && n < 5148))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5122)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inc @zptemp
	dey
	Jne *-7
	tay
.elseif(n = 5123)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	delay_n 6
	dey
	Jne *-7
	tay
.elseif(n = 5124)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 5125)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5126)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 5127 || n = 5133 || n = 5136 || n = 5152)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5129 || n = 5150)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5130)
	; 12 bytes
	php
	sec
	lda #0
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5131 || n = 5153)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5132)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5134)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5135)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5139)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5140)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5141)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5148)
	; 11 bytes
	tya
	ldy #(n-24)/21 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5149)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5151)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5156)
	; 12 bytes
	php
	sec
	lda #(n-8)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
; 5157 to 5195
.if(n = 5157 || n = 5180)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5158)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5159 || n = 5173 || n = 5177 || n = 5182 || n = 5190 || n = 5194 || n = 5195)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5160 || n = 5174 || n = 5183 || n = 5191)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5161 || n = 5184)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5162)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5163)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5164)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5165 || n = 5189)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif((n > 5165 && n < 5169) || n = 5172 || n = 5176 || n = 5181 || n = 5185 || n = 5186)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5169)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 5170)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5171 || n = 5192)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5175)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5178)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5179)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5187)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $D8A0'
	ldy #216
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5188)
	; 11 bytes
	php
	stx @zptemp
	lax #20
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 5193)
	; 11 bytes
	php
	txa
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.else
; 5196 to 5234
.if(n = 5196 || n = 5206 || n = 5229)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5197 || n = 5219)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5198 || n = 5204 || (n > 5207 && n < 5211) || n = 5214 || n = 5220 || n = 5227)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5199 || n = 5215 || n = 5221 || n = 5228)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5200 || n = 5222)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5201)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5202)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5203 || n = 5226)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5205 || n = 5231)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5207 || n = 5217)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5211)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5212)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 5213 || n = 5234)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5216)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5218)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5223)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5224)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5225)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5230)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5232)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5233)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
; 5235 to 5273
.if(n = 5235)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5236 || n = 5251 || n = 5265)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5237 || n = 5261)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5238 || n = 5257)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5239)
	; 12 bytes
	pla
	tya
	ldy #(n-7)/24 ;hides 'nop'
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5240 || n = 5242 || n = 5246 || n = 5250 || n = 5262 || n = 5264 || n = 5268 || n = 5269 || n = 5273)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5241 || n = 5263)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5243 || n = 5270)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5244)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5245 || n = 5267)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5247)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5248)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5249 || n = 5272)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5252 || n = 5256 || n = 5266)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5253)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5254)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5255)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5258)
	; 12 bytes
	php
	sec
	lda #(n-8)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5259)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5260)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5271)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.else
; 5274 to 5312
.if(n = 5274)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5275)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5276 || n = 5297)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5277 || n = 5282 || n = 5286 || (n > 5289 && n < 5293) || n = 5304 || n = 5312)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5278 || n = 5287 || n = 5299)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5279 || n = 5298 || n = 5300)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5280 || n = 5305)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5281)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
	tax
.elseif(n = 5283)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5284)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5285 || n = 5307)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5288 || n = 5310)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5289 || n = 5311)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5293)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5294)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5295)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5296)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5301)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5302)
	; 12 bytes
	tya
	ldy #(n-2)/25 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5303)
	; 12 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop zp,x'
	nop
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5306)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5308)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5309)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
.if((n > 5312 && n < 5316) || n = 5323 || (n > 5324 && n < 5328) || n = 5334 || n = 5336 || (n > 5345 && n < 5349) || n = 5350)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5316)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
	tax
.elseif(n = 5317)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5318 || n = 5339)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5319 || n = 5342 || n = 5344)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5320 || n = 5331 || n = 5343)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5321)
	; 12 bytes
	pha
	tya
	ldy #231
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5322)
	; 12 bytes
	pla
	tya
	ldy #231
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5324)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5328)
	; 12 bytes
	tya
	ldy #(n-28)/25 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5329 || n = 5351)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5330)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5332)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5333)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5335)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5337)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5338)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5340)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5341)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5345)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5349)
	; 12 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 20
	iny
	Jne *-7
	tay
.endif
.else
; 5352 to 5390
.if(n = 5352 || n = 5365 || n = 5372 || n = 5374 || n = 5382)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5353 || n = 5362 || n = 5366 || n = 5375 || n = 5383 || n = 5389)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5354 || n = 5376)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5355)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5356)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5357)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5358)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5359)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5360)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5361)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5363)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5364 || n = 5387)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5367 || n = 5384 || n = 5388 || n = 5390)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5368)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5369)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 5370)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 5371)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 13
	dey
	Jne *-7
	tay
.elseif(n = 5373)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5377)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5378)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5379)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5380)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5381)
	; 11 bytes
	tya
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5385)
	; 11 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5386)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
; 5391 to 5429
.if(n = 5391 || n = 5399 || n = 5414 || n = 5421)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5392 || n = 5394 || n = 5396 || (n > 5399 && n < 5403) || n = 5406 || n = 5407 || n = 5415 || n = 5416 || n = 5418 || (n > 5421 && n < 5426))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5393)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 13
	dey
	Jne *-7
	tay
.elseif(n = 5395 || n = 5417)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5397 || n = 5412 || n = 5419)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5398 || n = 5420)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5403)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 20
	iny
	Jne *-7
	tay
.elseif(n = 5404)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5405 || n = 5429)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5408)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5409)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5410)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5411)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5413)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5426)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
.elseif(n = 5427)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5428)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.endif
.else
; 5430 to 5468
.if(n = 5430 || n = 5455)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5431)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5432 || n = 5459)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5433 || n = 5456)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5434 || n = 5436 || n = 5448)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5435 || n = 5441 || n = 5447 || n = 5452 || n = 5458 || n = 5463)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5437 || n = 5443 || n = 5449 || n = 5457 || n = 5460)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5438 || n = 5440 || n = 5444 || n = 5446 || n = 5462 || n = 5467 || n = 5468)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5439 || n = 5461)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5442)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5445)
	; 11 bytes
	php
	stx @zptemp
	lax #21
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 5450)
	; 11 bytes
	php
	txa
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5451)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5453)
	; 11 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop'
	delay_n 18
	dey
	Jne *-6
	tay
.elseif(n = 5454)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5464)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5465)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5466)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
.if((n >= 5469) && (n < 5508))
; 5469 to 5507
.if((n > 5468 && n < 5475) || n = 5490 || (n > 5491 && n < 5499))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5475)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5476)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5477 || n = 5501)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5478)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5479 || n = 5502)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5480)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5481 || n = 5485)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5482 || n = 5506)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5483 || n = 5505)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5484)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tax
.elseif(n = 5486)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5487)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5488 || n = 5507)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5489)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5491)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5499)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5500)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5503)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5504)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.endif
.else
; 5508 to 5546
.if(n = 5508)
	; 12 bytes
	php
	sec
	lda #(n-8)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5509)
	; 12 bytes
	tya
	ldy #250 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 5510)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5511)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5512 || n = 5516 || (n > 5517 && n < 5523) || (n > 5531 && n < 5538) || n = 5539 || n = 5542 || n = 5544 || n = 5545)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5513 || n = 5540)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5514)
	; 12 bytes
	tya
	ldy #(n-2)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5515)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop zp,x'
	nop
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5517 || n = 5543)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5523)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5524)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5525)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5526 || n = 5528)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5527)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5529)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5530)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5531)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5538)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 5541)
	; 12 bytes
	tya
	ldy #(n-29)/26 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5546)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.endif
.else
; 5547 to 5625
.if(n = 5547)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	tax
.elseif(n = 5548 || n = 5594 || n = 5617)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5549 || n = 5571 || n = 5593 || n = 5615)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5550 || n = 5551)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5552 || n = 5574 || n = 5622)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5553 || n = 5598)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5554 || (n > 5555 && n < 5567) || n = 5568 || (n > 5575 && n < 5580) || (n > 5580 && n < 5592) || (n > 5598 && n < 5603) || n = 5604 || (n > 5605 && n < 5610) || (n > 5610 && n < 5614) || n = 5624 || n = 5625)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5555 || n = 5580 || n = 5605)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5567)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5569 || n = 5595)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5570)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5572)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5573 || n = 5597 || n = 5621)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5575)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5592)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5596)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5603)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5610)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 5614)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5616)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5618)
	; 12 bytes
	tya
	ldy #12 ;hides 'nop abs'
	inc @zptemp
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5619)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5620)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5623)
	; 12 bytes
	pla
	tya
	ldy #(n-7)/24 ;hides 'nop'
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
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
.if((n > 5625 && n < 5630) || (n > 5630 && n < 5637) || (n > 5648 && n < 5653) || (n > 5655 && n < 5661) || n = 5664)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5630 || n = 5655)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5637)
	; 11 bytes
	tya
	ldy #0
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5638)
	; 11 bytes
	tya
	ldy #(n-26)/23 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5639 || n = 5642)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5640 || n = 5663)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5641)
	; 12 bytes
	pla
	tya
	ldy #0
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5643)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5644)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5645)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5646)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5647)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5648)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5653)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5654)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5661)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5662)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.else
; 5665 to 5703
.if(n = 5665 || n = 5688)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5666 || n = 5670 || n = 5689 || n = 5694)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5667)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5668)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5669 || n = 5693)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5671)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5672 || n = 5674 || (n > 5675 && n < 5680) || (n > 5680 && n < 5684) || n = 5696 || n = 5697 || n = 5701 || n = 5703)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5673 || n = 5699)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5675)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5680)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5684)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5685)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5686)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5687)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	jsr @nearby_rts_14cyc
	Jne *-7
	tax
.elseif(n = 5690 || n = 5695)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5691)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5692)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5698)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 5700)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5702)
	; 11 bytes
	php
	stx @zptemp
	lax #22
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.endif
.endif
.else
.if((n >= 5704) && (n < 5743))
; 5704 to 5742
.if(n = 5704 || n = 5734)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5705 || n = 5710 || n = 5712 || n = 5735)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5706 || n = 5711 || n = 5713 || n = 5736)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5707)
	; 11 bytes
	php
	txa
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5708)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5709 || n = 5732)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5714 || (n > 5717 && n < 5722) || n = 5728 || n = 5733 || n = 5737 || n = 5738 || n = 5742)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5715)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5716)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5717 || n = 5741)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5722)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5723)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5724)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 5725)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5726)
	; 12 bytes
	tya
	ldy #(n-2)/27 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5727)
	; 12 bytes
	tya
	ldy #(n-3)/27 ;hides 'nop zp,x'
	nop
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5729)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5730)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5731)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5739)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5740)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.else
; 5743 to 5781
.if(n = 5743 || n = 5744 || (n > 5745 && n < 5750) || n = 5752 || (n > 5759 && n < 5763) || (n > 5765 && n < 5776) || n = 5779)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5745)
	; 12 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5750)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 5751 || n = 5777)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5753)
	; 11 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5754)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5755 || n = 5778)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5756)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5757)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5758 || n = 5781)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5759)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5763)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5764)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5765)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5776)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5780)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if(n = 5782 || n = 5807 || n = 5828 || n = 5839 || n = 5851 || n = 5857)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5783 || n = 5810)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif((n > 5783 && n < 5787) || (n > 5789 && n < 5799) || n = 5802 || n = 5808 || n = 5809 || (n > 5813 && n < 5822) || n = 5825 || (n > 5830 && n < 5835) || (n > 5839 && n < 5845) || n = 5848 || n = 5852 || n = 5854)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5787)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5788)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5789 || n = 5813 || n = 5837)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5799)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5800)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5801 || n = 5824 || n = 5847)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5803)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 18
	iny
	Jne *-6
	tay
.elseif(n = 5804 || n = 5806 || n = 5827 || n = 5838 || n = 5850 || n = 5856)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5805 || n = 5830)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5811)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5812)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5822)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5823)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5826 || n = 5849)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5829 || n = 5855)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5835)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5836)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5845)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5846)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5853)
	; 11 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop'
	delay_n 18
	dey
	Jne *-6
	tay
.elseif(n = 5858)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5859)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5861 || n = 5885)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5862)
	; 12 bytes
	tya
	ldy #12 ;hides 'nop abs'
	delay_n 20
	iny
	Jne *-7
	tay
.elseif(n = 5863 || n = 5874 || n = 5887 || n = 5897)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5864 || n = 5891)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif((n > 5864 && n < 5868) || n = 5871 || (n > 5874 && n < 5878) || n = 5879 || n = 5882 || n = 5888 || n = 5894 || n = 5898)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5868)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5869)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5870)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5872 || n = 5895)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5873 || n = 5886 || n = 5896)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5878)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5880)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5881)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5883)
	; 11 bytes
	tya
	ldy #(n-27)/24 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5884)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5889)
	; 12 bytes
	tya
	ldy #(n-3)/27 ;hides 'nop'
	delay_n 20
	dey
	Jne *-7
	tay
.elseif(n = 5890)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
	tax
.elseif(n = 5892)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5893)
	; 11 bytes
	tya
	ldy #0
	delay_n 18
	dey
	Jne *-5
	tay
.endif
.else
; 5899 to 5937
.if((n > 5898 && n < 5902) || n = 5904 || n = 5906 || (n > 5909 && n < 5918) || (n > 5918 && n < 5928) || n = 5929 || (n > 5933 && n < 5938))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5902)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5903)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5905 || n = 5930)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5907)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5908)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5909 || n = 5933)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5918)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5928)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5931)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5932)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
; 5938 to 6015
.if((n > 5937 && n < 5945) || (n > 5945 && n < 5955) || n = 5958 || n = 5960 || n = 5965 || (n > 5968 && n < 5972) || (n > 5972 && n < 5979) || n = 5982 || n = 5984 || (n > 5985 && n < 5999) || (n > 5999 && n < 6003) || n = 6008 || n = 6010 || (n > 6011 && n < 6016))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5945 || n = 5972 || n = 5999)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5955)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 5956)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5957 || n = 5981 || n = 6005)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5959)
	; 11 bytes
	php
	stx @zptemp
	lax #23
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 5961 || n = 5966)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5962 || n = 5967 || n = 6006)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5963 || n = 5968)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5964)
	; 11 bytes
	php
	txa
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5979)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5980)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5983)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5985 || n = 6011)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6003)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6004)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6007)
	; 12 bytes
	pla
	tya
	ldy #(n-7)/24 ;hides 'nop'
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6009)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-7
	tax
.endif
.else
; 6016 to 6093
.if((n > 6015 && n < 6026) || (n > 6030 && n < 6035) || n = 6036 || (n > 6037 && n < 6051) || n = 6054 || (n > 6055 && n < 6063) || (n > 6063 && n < 6075) || n = 6079 || (n > 6080 && n < 6087) || n = 6088 || (n > 6089 && n < 6093))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6026 || n = 6080)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6027)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6028)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6029 || n = 6053 || n = 6077)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 6030 || n = 6055)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 6035)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 6037 || n = 6063 || n = 6089)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6051)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6052)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6075)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6076)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6078)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6087)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6093)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	jsr @nearby_rts_15cyc
	Jne *-7
	tax
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
; 6094 to 6171
.if((n > 6093 && n < 6099) || n = 6104 || n = 6106 || (n > 6107 && n < 6113) || n = 6114 || (n > 6115 && n < 6123) || n = 6126 || n = 6127 || n = 6129 || (n > 6130 && n < 6134) || n = 6135 || n = 6136 || n = 6140 || (n > 6141 && n < 6147) || (n > 6149 && n < 6155) || (n > 6155 && n < 6161) || (n > 6161 && n < 6165) || n = 6166 || (n > 6167 && n < 6172))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6099)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6100)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6101 || n = 6125)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 6102)
	; 12 bytes
	tya
	ldy #(n-2)/25 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6103)
	; 12 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop zp,x'
	nop
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6105 || n = 6130 || n = 6155)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 6107 || n = 6134 || n = 6161)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6113)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6115 || n = 6141 || n = 6167)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6123)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6124)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6128)
	; 12 bytes
	tya
	ldy #(n-28)/25 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6137)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 6138)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6139)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6147)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6148)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6149)
	; 12 bytes
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 6165)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.endif
.else
; 6172 to 6250
.if((n > 6171 && n < 6180) || (n > 6180 && n < 6188) || (n > 6188 && n < 6193) || (n > 6193 && n < 6205) || (n > 6205 && n < 6213) || n = 6214 || n = 6217 || n = 6222 || (n > 6225 && n < 6230) || (n > 6230 && n < 6240) || n = 6241 || n = 6243 || n = 6244 || (n > 6245 && n < 6251))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6180 || n = 6205 || n = 6230)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 6188 || n = 6215 || n = 6242)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6193 || n = 6245)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6213)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 20
	iny
	Jne *-7
	tay
.elseif(n = 6216)
	; 11 bytes
	php
	stx @zptemp
	lax #24
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 6218 || n = 6223)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6219 || n = 6224)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6220 || n = 6225)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6221)
	; 11 bytes
	php
	txa
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6240)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 18
	dex
	dex
	Jne *-7
	tax
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 6251) && (n < 6876))
.if((n >= 6251) && (n < 6563))
; 6251 to 6562
.if(n = 6251 || n = 6252 || n = 6254 || (n > 6257 && n < 6267) || n = 6268 || n = 6270 || (n > 6271 && n < 6280) || (n > 6280 && n < 6296) || (n > 6297 && n < 6305) || (n > 6305 && n < 6321) || n = 6322 || n = 6324 || (n > 6325 && n < 6330) || (n > 6330 && n < 6345) || (n > 6350 && n < 6355) || (n > 6355 && n < 6373) || n = 6376 || n = 6378 || n = 6379 || (n > 6380 && n < 6401) || n = 6403 || (n > 6405 && n < 6427) || (n > 6427 && n < 6431) || (n > 6431 && n < 6443) || (n > 6443 && n < 6453) || (n > 6453 && n < 6458) || (n > 6458 && n < 6473) || n = 6474 || n = 6483 || n = 6484 || (n > 6485 && n < 6499) || (n > 6499 && n < 6503) || n = 6504 || (n > 6505 && n < 6512) || (n > 6512 && n < 6531) || (n > 6531 && n < 6539) || (n > 6539 && n < 6555) || n = 6556 || (n > 6557 && n < 6563))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6253)
	; 11 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop'
	delay_n 18
	dey
	Jne *-6
	tay
.elseif(n = 6255 || n = 6280 || n = 6305 || n = 6330 || n = 6355 || n = 6380)
	; 12 bytes
	tya
	ldy #(n-5)/25
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 6256 || n = 6476 || n = 6481)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6257 || n = 6477 || n = 6482)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6267)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 20
	iny
	Jne *-7
	tay
.elseif(n = 6269 || n = 6296 || n = 6350 || n = 6377 || n = 6404 || n = 6431 || n = 6458 || n = 6485 || n = 6512 || n = 6539)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6271 || n = 6297 || n = 6323 || n = 6349 || n = 6375 || n = 6401 || n = 6427 || n = 6453 || n = 6479 || n = 6505 || n = 6531 || n = 6557)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6321)
	; 12 bytes
	tya
	ldy #(n-3)/27 ;hides 'nop'
	delay_n 20
	dey
	Jne *-7
	tay
.elseif(n = 6325)
	; 12 bytes
	tya
	ldy #218 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6345)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6346)
	; 12 bytes
	tya
	ldy #(n-2)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6347)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop zp,x'
	nop
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6348)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 6373)
	; 12 bytes
	tya
	ldy #(n-29)/26 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6374)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6402)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 6405)
	; 12 bytes
	tya
	ldy #0
	delay_n 20
	dey
	Jne *-6
	tay
.elseif(n = 6443)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 6473)
	; 11 bytes
	php
	stx @zptemp
	lax #25
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 6475 || n = 6480)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6478)
	; 11 bytes
	php
	txa
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6499)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 6503)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6555)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.endif
.else
; 6563 to 6875
.if(n = 6563 || n = 6565 || (n > 6566 && n < 6583) || (n > 6583 && n < 6589) || n = 6592 || (n > 6593 && n < 6609) || (n > 6609 && n < 6618) || n = 6619 || (n > 6620 && n < 6635) || (n > 6635 && n < 6647) || (n > 6647 && n < 6661) || (n > 6661 && n < 6673) || (n > 6674 && n < 6701) || (n > 6702 && n < 6728) || n = 6729 || n = 6736 || (n > 6739 && n < 6753) || n = 6754 || (n > 6755 && n < 6782) || (n > 6782 && n < 6789) || (n > 6789 && n < 6809) || (n > 6809 && n < 6836) || (n > 6836 && n < 6863) || (n > 6863 && n < 6876))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6564)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 6566 || n = 6593 || n = 6620 || n = 6647 || n = 6674 || n = 6701 || n = 6728 || n = 6755 || n = 6782 || n = 6809 || n = 6836 || n = 6863)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6583 || n = 6609 || n = 6635)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6589)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6590)
	; 12 bytes
	tya
	ldy #(n-2)/27 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6591)
	; 12 bytes
	tya
	ldy #(n-3)/27 ;hides 'nop zp,x'
	nop
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 6618)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 6661)
	; 12 bytes
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6673)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6702)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 28
	Jne *-7
	tax
.elseif(n = 6730)
	; 11 bytes
	php
	stx @zptemp
	lax #26
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 6731)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6732 || n = 6737)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6733 || n = 6738)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6734 || n = 6739)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6735)
	; 11 bytes
	php
	txa
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6753)
	; 12 bytes
	tya
	ldy #(n-3)/27 ;hides 'nop'
	delay_n 20
	dey
	Jne *-7
	tay
.elseif(n = 6789)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.endif
.else
; 6876 to 7500
.if((n > 6875 && n < 6890) || (n > 6890 && n < 6905) || (n > 6905 && n < 6917) || (n > 6917 && n < 6987) || n = 6988 || n = 6993 || (n > 6996 && n < 7003) || (n > 7003 && n < 7108) || (n > 7108 && n < 7244) || n = 7245 || n = 7250 || (n > 7253 && n < 7501))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6890)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6905)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 29
	Jne *-7
	tax
.elseif(n = 6917)
	; 12 bytes
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6987)
	; 11 bytes
	php
	stx @zptemp
	lax #27
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 6989 || n = 6994 || n = 7246 || n = 7251)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6990 || n = 6995 || n = 7247 || n = 7252)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6991 || n = 6996 || n = 7248 || n = 7253)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6992)
	; 11 bytes
	php
	txa
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7003)
	; 12 bytes
	tya
	ldy #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 7108)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 30
	Jne *-7
	tax
.elseif(n = 7244)
	; 11 bytes
	php
	stx @zptemp
	lax #28
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 7249)
	; 11 bytes
	php
	txa
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.endif
.else
; 7501 to 10000
.if(n = 7501)
	; 11 bytes
	php
	stx @zptemp
	lax #29
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 7502 || n = 7507 || (n > 7510 && n < 7758) || n = 7759 || n = 7764 || (n > 7767 && n < 8015) || n = 8016 || n = 8021 || (n > 8024 && n < 8272) || n = 8273 || n = 8278 || (n > 8281 && n < 8529) || n = 8530 || n = 8535 || (n > 8538 && n < 8786) || n = 8787 || n = 8792 || (n > 8795 && n < 9043) || n = 9044 || n = 9049 || (n > 9052 && n < 9300) || n = 9301 || n = 9306 || (n > 9309 && n < 9557) || n = 9558 || n = 9563 || (n > 9566 && n < 9814) || n = 9815 || n = 9820 || (n > 9823 && n < 10001))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7503 || n = 7508 || n = 7760 || n = 7765 || n = 8017 || n = 8022 || n = 8274 || n = 8279 || n = 8531 || n = 8536 || n = 8788 || n = 8793 || n = 9045 || n = 9050 || n = 9302 || n = 9307 || n = 9559 || n = 9564 || n = 9816 || n = 9821)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7504 || n = 7509 || n = 7761 || n = 7766 || n = 8018 || n = 8023 || n = 8275 || n = 8280 || n = 8532 || n = 8537 || n = 8789 || n = 8794 || n = 9046 || n = 9051 || n = 9303 || n = 9308 || n = 9560 || n = 9565 || n = 9817 || n = 9822)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7505 || n = 7510 || n = 7762 || n = 7767 || n = 8019 || n = 8024 || n = 8276 || n = 8281 || n = 8533 || n = 8538 || n = 8790 || n = 8795 || n = 9047 || n = 9052 || n = 9304 || n = 9309 || n = 9561 || n = 9566 || n = 9818 || n = 9823)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7506)
	; 11 bytes
	php
	txa
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7758)
	; 11 bytes
	php
	stx @zptemp
	lax #30
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 7763)
	; 11 bytes
	php
	txa
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8015)
	; 11 bytes
	php
	stx @zptemp
	lax #31
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 8020)
	; 11 bytes
	php
	txa
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8272)
	; 11 bytes
	php
	stx @zptemp
	lax #32
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 8277)
	; 11 bytes
	php
	txa
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8529)
	; 11 bytes
	php
	stx @zptemp
	lax #33
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 8534)
	; 11 bytes
	php
	txa
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8786)
	; 11 bytes
	php
	stx @zptemp
	lax #34
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 8791)
	; 11 bytes
	php
	txa
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 9043)
	; 11 bytes
	php
	stx @zptemp
	lax #35
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 9048)
	; 11 bytes
	php
	txa
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 9300)
	; 11 bytes
	php
	stx @zptemp
	lax #36
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 9305)
	; 11 bytes
	php
	txa
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 9557)
	; 11 bytes
	php
	stx @zptemp
	lax #37
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 9562)
	; 11 bytes
	php
	txa
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 9814)
	; 11 bytes
	php
	stx @zptemp
	lax #38
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 9819)
	; 11 bytes
	php
	txa
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
; 10001 to 12500
.if((n > 10000 && n < 10071) || n = 10072 || n = 10077 || (n > 10080 && n < 10328) || n = 10329 || n = 10334 || (n > 10337 && n < 10585) || n = 10586 || n = 10591 || (n > 10594 && n < 10842) || n = 10843 || n = 10848 || (n > 10851 && n < 11099) || n = 11100 || n = 11105 || (n > 11108 && n < 11356) || n = 11357 || n = 11362 || (n > 11365 && n < 11613) || n = 11614 || n = 11619 || (n > 11622 && n < 11870) || n = 11871 || n = 11876 || (n > 11879 && n < 12127) || n = 12128 || n = 12133 || (n > 12136 && n < 12384) || n = 12385 || n = 12390 || (n > 12393 && n < 12501))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 10071)
	; 11 bytes
	php
	stx @zptemp
	lax #39
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 10073 || n = 10078 || n = 10330 || n = 10335 || n = 10587 || n = 10592 || n = 10844 || n = 10849 || n = 11101 || n = 11106 || n = 11358 || n = 11363 || n = 11615 || n = 11620 || n = 11872 || n = 11877 || n = 12129 || n = 12134 || n = 12386 || n = 12391)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10074 || n = 10079 || n = 10331 || n = 10336 || n = 10588 || n = 10593 || n = 10845 || n = 10850 || n = 11102 || n = 11107 || n = 11359 || n = 11364 || n = 11616 || n = 11621 || n = 11873 || n = 11878 || n = 12130 || n = 12135 || n = 12387 || n = 12392)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 10075 || n = 10080 || n = 10332 || n = 10337 || n = 10589 || n = 10594 || n = 10846 || n = 10851 || n = 11103 || n = 11108 || n = 11360 || n = 11365 || n = 11617 || n = 11622 || n = 11874 || n = 11879 || n = 12131 || n = 12136 || n = 12388 || n = 12393)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 10076)
	; 11 bytes
	php
	txa
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 10328)
	; 11 bytes
	php
	stx @zptemp
	lax #40
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 10333)
	; 11 bytes
	php
	txa
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 10585)
	; 11 bytes
	php
	stx @zptemp
	lax #41
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 10590)
	; 11 bytes
	php
	txa
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 10842)
	; 11 bytes
	php
	stx @zptemp
	lax #42
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 10847)
	; 11 bytes
	php
	txa
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 11099)
	; 11 bytes
	php
	stx @zptemp
	lax #43
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 11104)
	; 11 bytes
	php
	txa
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 11356)
	; 11 bytes
	php
	stx @zptemp
	lax #44
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 11361)
	; 11 bytes
	php
	txa
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 11613)
	; 11 bytes
	php
	stx @zptemp
	lax #45
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 11618)
	; 11 bytes
	php
	txa
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 11870)
	; 11 bytes
	php
	stx @zptemp
	lax #46
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 11875)
	; 11 bytes
	php
	txa
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 12127)
	; 11 bytes
	php
	stx @zptemp
	lax #47
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 12132)
	; 11 bytes
	php
	txa
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 12384)
	; 11 bytes
	php
	stx @zptemp
	lax #48
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 12389)
	; 11 bytes
	php
	txa
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.else
; 12501 to 15000
.if((n > 12500 && n < 12641) || n = 12642 || n = 12647 || (n > 12650 && n < 12898) || n = 12899 || n = 12904 || (n > 12907 && n < 13155) || n = 13156 || n = 13161 || (n > 13164 && n < 13412) || n = 13413 || n = 13418 || (n > 13421 && n < 13669) || n = 13670 || n = 13675 || (n > 13678 && n < 13926) || n = 13927 || n = 13932 || (n > 13935 && n < 14183) || n = 14184 || n = 14189 || (n > 14192 && n < 14440) || n = 14441 || n = 14446 || (n > 14449 && n < 14697) || n = 14698 || n = 14703 || (n > 14706 && n < 14954) || n = 14955 || n = 14960 || (n > 14963 && n < 15001))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 12641)
	; 11 bytes
	php
	stx @zptemp
	lax #49
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 12643 || n = 12648 || n = 12900 || n = 12905 || n = 13157 || n = 13162 || n = 13414 || n = 13419 || n = 13671 || n = 13676 || n = 13928 || n = 13933 || n = 14185 || n = 14190 || n = 14442 || n = 14447 || n = 14699 || n = 14704 || n = 14956 || n = 14961)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12644 || n = 12649 || n = 12901 || n = 12906 || n = 13158 || n = 13163 || n = 13415 || n = 13420 || n = 13672 || n = 13677 || n = 13929 || n = 13934 || n = 14186 || n = 14191 || n = 14443 || n = 14448 || n = 14700 || n = 14705 || n = 14957 || n = 14962)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 12645 || n = 12650 || n = 12902 || n = 12907 || n = 13159 || n = 13164 || n = 13416 || n = 13421 || n = 13673 || n = 13678 || n = 13930 || n = 13935 || n = 14187 || n = 14192 || n = 14444 || n = 14449 || n = 14701 || n = 14706 || n = 14958 || n = 14963)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 12646)
	; 11 bytes
	php
	txa
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 12898)
	; 11 bytes
	php
	stx @zptemp
	lax #50
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 12903)
	; 11 bytes
	php
	txa
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 13155)
	; 11 bytes
	php
	stx @zptemp
	lax #51
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 13160)
	; 11 bytes
	php
	txa
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 13412)
	; 11 bytes
	php
	stx @zptemp
	lax #52
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 13417)
	; 11 bytes
	php
	txa
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 13669)
	; 11 bytes
	php
	stx @zptemp
	lax #53
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 13674)
	; 11 bytes
	php
	txa
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 13926)
	; 11 bytes
	php
	stx @zptemp
	lax #54
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 13931)
	; 11 bytes
	php
	txa
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 14183)
	; 11 bytes
	php
	stx @zptemp
	lax #55
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 14188)
	; 11 bytes
	php
	txa
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 14440)
	; 11 bytes
	php
	stx @zptemp
	lax #56
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 14445)
	; 11 bytes
	php
	txa
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 14697)
	; 11 bytes
	php
	stx @zptemp
	lax #57
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 14702)
	; 11 bytes
	php
	txa
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 14954)
	; 11 bytes
	php
	stx @zptemp
	lax #58
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 14959)
	; 11 bytes
	php
	txa
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
; 15001 to 17500
.if((n > 15000 && n < 15211) || n = 15212 || n = 15217 || (n > 15220 && n < 15468) || n = 15469 || n = 15474 || (n > 15477 && n < 15725) || n = 15726 || n = 15731 || (n > 15734 && n < 15982) || n = 15983 || n = 15988 || (n > 15991 && n < 16239) || n = 16240 || n = 16245 || (n > 16248 && n < 16496) || n = 16497 || n = 16502 || (n > 16505 && n < 16753) || n = 16754 || n = 16759 || (n > 16762 && n < 17010) || n = 17011 || n = 17016 || (n > 17019 && n < 17267) || n = 17268 || n = 17273 || (n > 17276 && n < 17501))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 15211)
	; 11 bytes
	php
	stx @zptemp
	lax #59
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 15213 || n = 15218 || n = 15470 || n = 15475 || n = 15727 || n = 15732 || n = 15984 || n = 15989 || n = 16241 || n = 16246 || n = 16498 || n = 16503 || n = 16755 || n = 16760 || n = 17012 || n = 17017 || n = 17269 || n = 17274)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15214 || n = 15219 || n = 15471 || n = 15476 || n = 15728 || n = 15733 || n = 15985 || n = 15990 || n = 16242 || n = 16247 || n = 16499 || n = 16504 || n = 16756 || n = 16761 || n = 17013 || n = 17018 || n = 17270 || n = 17275)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 15215 || n = 15220 || n = 15472 || n = 15477 || n = 15729 || n = 15734 || n = 15986 || n = 15991 || n = 16243 || n = 16248 || n = 16500 || n = 16505 || n = 16757 || n = 16762 || n = 17014 || n = 17019 || n = 17271 || n = 17276)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 15216)
	; 11 bytes
	php
	txa
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 15468)
	; 11 bytes
	php
	stx @zptemp
	lax #60
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 15473)
	; 11 bytes
	php
	txa
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 15725)
	; 11 bytes
	php
	stx @zptemp
	lax #61
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 15730)
	; 11 bytes
	php
	txa
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 15982)
	; 11 bytes
	php
	stx @zptemp
	lax #62
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 15987)
	; 11 bytes
	php
	txa
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 16239)
	; 11 bytes
	php
	stx @zptemp
	lax #63
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 16244)
	; 11 bytes
	php
	txa
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 16496)
	; 11 bytes
	php
	stx @zptemp
	lax #64
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 16501)
	; 11 bytes
	php
	txa
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 16753)
	; 11 bytes
	php
	stx @zptemp
	lax #65
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 16758)
	; 11 bytes
	php
	txa
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 17010)
	; 11 bytes
	php
	stx @zptemp
	lax #66
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 17015)
	; 11 bytes
	php
	txa
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 17267)
	; 11 bytes
	php
	stx @zptemp
	lax #67
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 17272)
	; 11 bytes
	php
	txa
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.else
; 17501 to 20000
.if((n > 17500 && n < 17524) || n = 17525 || n = 17530 || (n > 17533 && n < 17781) || n = 17782 || n = 17787 || (n > 17790 && n < 18038) || n = 18039 || n = 18044 || (n > 18047 && n < 18295) || n = 18296 || n = 18301 || (n > 18304 && n < 18552) || n = 18553 || n = 18558 || (n > 18561 && n < 18809) || n = 18810 || n = 18815 || (n > 18818 && n < 19066) || n = 19067 || n = 19072 || (n > 19075 && n < 19323) || n = 19324 || n = 19329 || (n > 19332 && n < 19580) || n = 19581 || n = 19586 || (n > 19589 && n < 19837) || n = 19838 || n = 19843 || (n > 19846 && n < 20001))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 17524)
	; 11 bytes
	php
	stx @zptemp
	lax #68
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 17526 || n = 17531 || n = 17783 || n = 17788 || n = 18040 || n = 18045 || n = 18297 || n = 18302 || n = 18554 || n = 18559 || n = 18811 || n = 18816 || n = 19068 || n = 19073 || n = 19325 || n = 19330 || n = 19582 || n = 19587 || n = 19839 || n = 19844)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17527 || n = 17532 || n = 17784 || n = 17789 || n = 18041 || n = 18046 || n = 18298 || n = 18303 || n = 18555 || n = 18560 || n = 18812 || n = 18817 || n = 19069 || n = 19074 || n = 19326 || n = 19331 || n = 19583 || n = 19588 || n = 19840 || n = 19845)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 17528 || n = 17533 || n = 17785 || n = 17790 || n = 18042 || n = 18047 || n = 18299 || n = 18304 || n = 18556 || n = 18561 || n = 18813 || n = 18818 || n = 19070 || n = 19075 || n = 19327 || n = 19332 || n = 19584 || n = 19589 || n = 19841 || n = 19846)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 17529)
	; 11 bytes
	php
	txa
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 17781)
	; 11 bytes
	php
	stx @zptemp
	lax #69
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 17786)
	; 11 bytes
	php
	txa
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 18038)
	; 11 bytes
	php
	stx @zptemp
	lax #70
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 18043)
	; 11 bytes
	php
	txa
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 18295)
	; 11 bytes
	php
	stx @zptemp
	lax #71
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 18300)
	; 11 bytes
	php
	txa
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 18552)
	; 11 bytes
	php
	stx @zptemp
	lax #72
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 18557)
	; 11 bytes
	php
	txa
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 18809)
	; 11 bytes
	php
	stx @zptemp
	lax #73
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 18814)
	; 11 bytes
	php
	txa
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 19066)
	; 11 bytes
	php
	stx @zptemp
	lax #74
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 19071)
	; 11 bytes
	php
	txa
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 19323)
	; 11 bytes
	php
	stx @zptemp
	lax #75
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 19328)
	; 11 bytes
	php
	txa
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 19580)
	; 11 bytes
	php
	stx @zptemp
	lax #76
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 19585)
	; 11 bytes
	php
	txa
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 19837)
	; 11 bytes
	php
	stx @zptemp
	lax #77
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 19842)
	; 11 bytes
	php
	txa
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
