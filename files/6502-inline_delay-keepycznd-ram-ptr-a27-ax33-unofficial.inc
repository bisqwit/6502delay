; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, and V
; Will preserve Y, S, Z+N, C, D, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 19
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 33
; - 8 bytes or less when n <= 293
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.2, max=6
; - When 21 <= n <= 30: min=5, avg=5.9, max=7
; - When 31 <= n <= 50: min=6, avg=7.0, max=8
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.1, max=9
; - When 301 <= n <= 500: min=6, avg=8.2, max=9
; - When 501 <= n <= 750: min=7, avg=8.2, max=9
; - When 751 <= n <= 1000: min=7, avg=8.2, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.6, max=9
; - When 2001 <= n <= 3000: min=7, avg=9.0, max=9
; - When 3001 <= n <= 5000: min=7, avg=9.0, max=9
; - When 5001 <= n <= 7500: min=7, avg=9.0, max=9
; - When 7501 <= n <= 10000: min=7, avg=9.0, max=9
; - When 10001 <= n <= 20000: min=7, avg=9.0, max=9
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
	sta @zptemp
.elseif(n = 4 || n = 6 || (n > 15 && n < 19) || n = 20 || n = 32)
	; 2..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 3 bytes
	clv
	Jvc *+2
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || n = 13 || n = 19 || (n > 20 && n < 24))
	; 3..6 bytes
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
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 25 || n = 27 || n = 29 || n = 30)
	; 6..7 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
	; 8 bytes
	php
	lda $A5
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 35)
	; 8 bytes
	php
	clc
	clc
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
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
.if(n = 40 || n = 42 || n = 44 || n = 45 || n = 47 || (n > 48 && n < 58) || (n > 58 && n < 62) || (n > 62 && n < 79))
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
.if((n > 78 && n < 118) || (n > 118 && n < 123) || (n > 123 && n < 131) || (n > 131 && n < 157))
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
.elseif(n = 131)
	; 6 bytes
	php
	ldx #232 ;hides 'inx'
	Jne *-1
	plp
.endif
.endif
.else
; 157 to 312
.if((n > 156 && n < 163) || (n > 163 && n < 292))
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
.elseif(n = 292 || n = 301 || n = 305 || n = 310)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 293 || n = 298 || n = 303)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 294 || n = 296 || n = 297)
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 295)
	; 8 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	plp
.elseif(n = 299)
	; 7 bytes
	php
	lax #1
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 300)
	; 7 bytes
	php
	ldx #42 ;hides 'rol a'
	dex
	Jne *-2
	plp
.elseif(n = 302 || n = 309)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 304)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 306)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 307)
	; 7 bytes
	php
	ldx #42 ;hides 'rol a'
	dex
	Jpl *-2
	plp
.elseif(n = 308)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 311)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	dex
	Jpl *-3
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
.if((n .mod 5 = 3) && ((n > 312 && n < 324) || n = 338 || (n > 347 && n < 359) || (n > 367 && n < 389)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 314)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 315 || n = 320 || n = 325 || n = 335 || n = 340 || n = 345 || n = 350 || n = 355 || n = 360 || n = 370 || n = 374 || n = 375 || n = 380 || n = 385)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 316 || n = 330 || n = 337 || n = 351 || n = 372)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 317 || n = 322 || n = 327 || n = 352 || n = 357)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 319)
	; 8 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	dex
	Jpl *-3
	plp
.elseif(n = 321)
	; 8 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	inx
	Jpl *-3
	plp
.elseif(n = 324 || n = 329 || n = 334 || n = 349 || n = 354 || n = 364 || n = 369)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 326 || n = 341 || n = 356 || n = 361 || n = 371 || n = 381)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 328 || n = 333 || n = 343 || n = 363)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 331)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 332 || n = 342 || n = 347 || n = 362 || n = 367 || n = 377 || n = 382 || n = 387)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 336 || n = 376)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 339)
	; 8 bytes
	php
	ldx #74 ;hides 'lsr a'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 344 || n = 365 || n = 379)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
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
.elseif(n = 359)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 366)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 384)
	; 7 bytes
	php
	ldx #74 ;hides 'lsr a'
	inx
	Jpl *-2
	plp
.elseif(n = 386)
	; 6 bytes
	php
	ldx #202 ;hides 'dex'
	Jmi *-1
	plp
.elseif(n = 389)
	; 8 bytes
	php
	ldx $A6
	ldx #202 ;hides 'dex'
	Jmi *-1
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
.if(n = 391)
	; 8 bytes
	php
	inc @zptemp
	ldx #202 ;hides 'dex'
	Jmi *-1
	plp
.elseif(n = 392 || n = 417 || n = 422 || n = 432 || n = 462 || n = 467)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && ((n > 392 && n < 409) || n = 418 || n = 433 || n = 448 || (n > 457 && n < 469)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 394 || n = 399 || n = 429 || n = 434 || n = 439 || n = 454 || n = 459)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 395 || n = 410 || n = 415 || n = 420 || n = 425 || n = 430 || n = 440 || n = 444 || n = 445 || n = 450 || n = 455 || n = 460 || n = 465)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 396 || n = 401 || n = 406 || n = 436 || n = 451 || n = 461 || n = 466)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 397 || n = 427 || n = 437 || n = 447 || n = 452 || n = 457)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 400 || n = 456)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 402)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 404)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 405)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
	plp
.elseif(n = 407 || n = 414 || n = 421 || n = 435 || n = 442 || n = 449)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
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
.elseif(n = 411)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	dex
	Jmi *-3
	plp
.elseif(n = 412)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 413 || n = 423 || n = 428 || n = 438 || n = 443 || n = 453)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 416)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 419)
	; 7 bytes
	php
	ldx #58 ;hides 'nop'
	dex
	Jpl *-2
	plp
.elseif(n = 424)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	plp
.elseif(n = 426 || n = 431 || n = 441 || n = 446)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 464)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.endif
.endif
.else
.if((n >= 469) && (n < 547))
.if((n >= 469) && (n < 508))
; 469 to 507
.if(n = 469 || n = 499)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 470 || n = 484 || n = 491)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 471)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 472 || n = 482 || n = 497 || n = 507)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 473 || n = 478)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 474)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	inx
	Jpl *-3
	plp
.elseif(n = 475 || n = 480 || n = 485 || n = 489 || n = 490 || n = 495 || n = 500)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 476 || n = 481)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 477 || n = 505)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 479)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	inx
	Jne *-3
	plp
.elseif((n .mod 5 = 3) && ((n > 482 && n < 504)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 486 || n = 506)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 487)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 492)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
	plp
.elseif(n = 494)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 496)
	; 7 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	Jpl *-2
	plp
.elseif(n = 501)
	; 8 bytes
	php
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	dex
	Jpl *-3
	plp
.elseif(n = 502)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 504)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.endif
.else
; 508 to 546
.if(n = 508 || n = 528)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 509 || n = 529)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 510)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
	plp
.elseif(n = 511)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 512 || n = 519 || n = 526 || n = 540)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 513 || n = 518 || n = 523 || n = 533 || n = 538 || n = 543)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 514)
	; 9 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
	plp
.elseif((n .mod 5 = 0) && ((n > 514 && n < 536) || n = 545))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 516 || n = 521 || n = 541 || n = 546)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 517 || n = 522 || n = 532 || n = 542)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 524)
	; 7 bytes
	php
	ldx #74 ;hides 'lsr a'
	dex
	Jne *-2
	plp
.elseif(n = 527)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 531)
	; 7 bytes
	php
	ldx #74 ;hides 'lsr a'
	dex
	Jpl *-2
	plp
.elseif(n = 534)
	; 9 bytes
	php
	ldx $A6
	ldx #74 ;hides 'lsr a'
	dex
	Jpl *-2
	plp
.elseif(n = 536)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 537)
	; 8 bytes
	php
	ldx #138 ;hides 'txa'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 539)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 544)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.endif
.endif
.else
; 547 to 625
.if(n = 547 || n = 554 || n = 561 || n = 582 || n = 624)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 548 || n = 553 || n = 563 || n = 568 || n = 578 || n = 583 || n = 588 || n = 603 || n = 613 || n = 618 || n = 623)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 549 || n = 550 || n = 555 || n = 560 || n = 565 || n = 569 || n = 570 || n = 580 || n = 585 || n = 590 || n = 595 || n = 600 || n = 605 || n = 615 || n = 619 || n = 620 || n = 625)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 551)
	; 8 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 552 || n = 557 || n = 562 || n = 572 || n = 597 || n = 607 || n = 612)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 556)
	; 7 bytes
	php
	lax #2
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 558 || n = 573 || n = 593 || n = 598 || n = 608)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 559)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 564)
	; 8 bytes
	php
	ldx #250 ;hides 'nop'
	dex
	dex
	Jmi *-3
	plp
.elseif(n = 566 || n = 571 || n = 581 || n = 586 || n = 591 || n = 606 || n = 621)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 567)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 574 || n = 579 || n = 604)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 575 || n = 589 || n = 596 || n = 610 || n = 617)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 576)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 577 || n = 587 || n = 592 || n = 602 || n = 622)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 584)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 594)
	; 9 bytes
	php
	ldx #42 ;hides 'rol a'
	pla
	pha
	dex
	Jne *-4
	plp
.elseif(n = 599)
	; 9 bytes
	php
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	dex
	Jpl *-4
	plp
.elseif(n = 601 || n = 611)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 609)
	; 8 bytes
	php
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 614)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	sta @zptemp
	Jmi *-3
	plp
.elseif(n = 616)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
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
.if(n = 626 || n = 641 || n = 656)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 627)
	; 8 bytes
	php
	ldx #138 ;hides 'txa'
	dex
	dex
	Jne *-3
	plp
.elseif((n .mod 5 = 3) && (n = 628 || n = 633 || n = 643 || n = 648 || (n > 657 && n < 704)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 629)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
	plp
.elseif(n = 630 || n = 635 || n = 640 || n = 650 || n = 655 || n = 660 || n = 665 || n = 670 || n = 675 || n = 685 || n = 689 || n = 690 || n = 700)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 631 || n = 659 || n = 666)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 632 || n = 637 || n = 647 || n = 657)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 634 || n = 639 || n = 649 || n = 669 || n = 679 || n = 684)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 636)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 638 || n = 653)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 642 || n = 662 || n = 667 || n = 677 || n = 682 || n = 692 || n = 697 || n = 702)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 644)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 645 || n = 652 || n = 680 || n = 687 || n = 694 || n = 701)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 646 || n = 651 || n = 661 || n = 671 || n = 676 || n = 686 || n = 691)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 654)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 664)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 672)
	; 8 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 674)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 681)
	; 8 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 695)
	; 8 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	plp
.elseif(n = 696)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 699)
	; 8 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.endif
.else
; 704 to 781
.if(n = 704)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 705 || n = 710 || n = 724 || n = 725 || n = 730 || n = 735 || n = 740 || n = 745 || n = 760 || n = 765 || n = 770 || n = 775 || n = 780)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 705 && n < 732) || n = 741 || n = 746 || (n > 755 && n < 767) || n = 781))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 5 = 2) && ((n > 706 && n < 718) || (n > 731 && n < 753) || (n > 766 && n < 778)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && ((n > 707 && n < 779)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 709 || n = 714 || n = 739 || n = 749 || n = 754 || n = 769 || n = 774 || n = 779)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 7 = 1) && ((n > 714 && n < 730) || (n > 749 && n < 772)))
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 719)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 720)
	; 7 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
	plp
.elseif(n = 727)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 734)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
	plp
.elseif(n = 736)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 744 || n = 776)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 751)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 755)
	; 7 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
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
.elseif(n = 762)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.endif
.endif
.else
; 782 to 937
.if((n .mod 5 = 2) && (n = 782 || n = 787 || n = 797 || n = 802 || (n > 811 && n < 823) || (n > 836 && n < 858) || (n > 871 && n < 893) || (n > 901 && n < 928) || n = 937))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && ((n > 782 && n < 934)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 784 || n = 824 || n = 856 || n = 864 || n = 896 || n = 936)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 785 || n = 792 || n = 806 || n = 820 || n = 841 || n = 855 || n = 862 || n = 869 || n = 890 || n = 911)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif((n .mod 5 = 1) && ((n > 785 && n < 802) || n = 811 || n = 821 || n = 826 || n = 836 || n = 846 || n = 851 || (n > 860 && n < 872) || (n > 880 && n < 892) || n = 901 || (n > 915 && n < 932)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 789)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 790 || n = 794 || n = 795 || n = 800 || n = 805 || n = 810 || n = 815 || n = 825 || n = 829 || n = 830 || n = 835 || n = 840 || n = 845 || n = 850 || n = 865 || n = 870 || n = 875 || n = 880 || n = 885 || n = 895 || n = 900 || n = 905 || n = 910 || n = 915 || n = 920 || n = 930 || n = 934 || n = 935)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 799 || n = 827 || n = 834 || n = 876 || n = 897 || n = 904 || n = 925 || n = 932)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 804)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 7
	dex
	Jpl *-4
	plp
.elseif(n = 807)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 809 || n = 854)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 814 || n = 819 || n = 844 || n = 849 || n = 859 || n = 874 || n = 879 || n = 889 || n = 894 || n = 909 || n = 914 || n = 919 || n = 929)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 816)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 831)
	; 8 bytes
	php
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	dex
	Jpl *-3
	plp
.elseif(n = 832)
	; 7 bytes
	php
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
	plp
.elseif(n = 839)
	; 9 bytes
	php
	pha
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
	pla
	plp
.elseif(n = 860)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 867)
	; 7 bytes
	php
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
	plp
.elseif(n = 884)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 899)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 906)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 924)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	nop
	inx
	Jpl *-3
	plp
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if((n .mod 5 = 3) && ((n > 937 && n < 1014)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 7 = 1) && (n = 939 || n = 946 || (n > 959 && n < 982) || (n > 994 && n < 1010)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif((n .mod 5 = 0) && ((n > 939 && n < 956) || (n > 964 && n < 991) || (n > 999 && n < 1016)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && (n = 941 || (n > 950 && n < 972) || n = 986 || (n > 995 && n < 1012)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 942)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 944)
	; 7 bytes
	php
	ldx #122 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif(n = 947 || n = 952 || n = 957 || n = 962 || n = 977 || n = 982 || n = 992 || n = 997 || n = 1007 || n = 1012)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 949 || n = 954 || n = 959 || n = 964 || n = 994)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 969)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 972)
	; 7 bytes
	php
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
	plp
.elseif(n = 976 || n = 984)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 979)
	; 9 bytes
	php
	pha
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 987)
	; 8 bytes
	php
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 989)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 991)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 999)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 1004)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1014)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jpl *-4
	plp
.endif
.else
; 1016 to 1093
.if((n .mod 7 = 1) && (n = 1016 || (n > 1029 && n < 1052) || (n > 1064 && n < 1087)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif((n .mod 5 = 2) && (n = 1017 || n = 1022 || n = 1032 || (n > 1041 && n < 1068) || (n > 1081 && n < 1093)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && ((n > 1017 && n < 1094)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 1019 || n = 1049 || n = 1054 || n = 1069 || n = 1084 || n = 1089)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1020 || n = 1035 || n = 1039 || n = 1040 || n = 1045 || n = 1050 || n = 1055 || n = 1060 || n = 1074 || n = 1075 || n = 1080 || n = 1085 || n = 1090)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1021)
	; 6 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	plp
.elseif(n = 1024)
	; 8 bytes
	php
	ldx $A6
	ldx #202 ;hides 'dex'
	Jne *-1
	plp
.elseif(n = 1025)
	; 8 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	plp
.elseif(n = 1026)
	; 8 bytes
	php
	inc @zptemp
	ldx #202 ;hides 'dex'
	Jne *-1
	plp
.elseif(n = 1027)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 1029)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	slo (@ptrtemp,x)
	plp
.elseif((n .mod 5 = 1) && (n = 1031 || n = 1036 || n = 1046 || (n > 1060 && n < 1082) || n = 1091))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 1034)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 1041)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 1056)
	; 7 bytes
	php
	ldx #106 ;hides 'ror a'
	inx
	Jne *-2
	plp
.elseif(n = 1059)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1064)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1070)
	; 7 bytes
	php
	ldx #152 ;hides 'tya'
	dex
	Jne *-2
	plp
.elseif(n = 1077)
	; 8 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	inx ;first loop only
	dex
	Jmi *-3
	plp
.endif
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && (n = 1094 || n = 1099 || n = 1119 || (n > 1128 && n < 1140) || (n > 1153 && n < 1165) || n = 1174 || n = 1189 || (n > 1198 && n < 1210) || n = 1229 || n = 1234))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1095 || n = 1105 || n = 1109 || n = 1110 || n = 1115 || n = 1120 || n = 1125 || n = 1130 || n = 1140 || n = 1145 || n = 1150 || n = 1155 || n = 1160 || n = 1165 || n = 1175 || n = 1179 || n = 1180 || n = 1185 || n = 1190 || n = 1195 || n = 1200 || n = 1210 || n = 1215 || n = 1220 || n = 1225 || n = 1230 || n = 1235 || n = 1245 || n = 1249 || n = 1250)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1096 || n = 1136 || n = 1144 || n = 1176 || n = 1216 || n = 1224)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 5 = 2) && (n = 1097 || n = 1102 || n = 1112 || n = 1117 || (n > 1126 && n < 1138) || (n > 1146 && n < 1173) || (n > 1181 && n < 1208) || (n > 1216 && n < 1243)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && ((n > 1097 && n < 1249)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1122) || (n > 1134 && n < 1157) || (n > 1169 && n < 1192) || (n > 1204 && n < 1227) || n = 1240 || n = 1247))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif((n .mod 5 = 1) && ((n > 1100 && n < 1117) || n = 1126 || (n > 1140 && n < 1152) || (n > 1160 && n < 1172) || n = 1181 || n = 1186 || (n > 1195 && n < 1212) || n = 1221 || (n > 1230 && n < 1247)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 1104)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1122)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1124 || n = 1214)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1131)
	; 8 bytes
	php
	ldx #250 ;hides 'nop'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1169)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 1194)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 1239)
	; 8 bytes
	php
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1244)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
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
.if((n .mod 5 = 1) && (n = 1251 || (n > 1270 && n < 1287)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 5 = 2) && (n = 1252 || (n > 1261 && n < 1278) || n = 1287))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && ((n > 1252 && n < 1284)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 7 = 1) && (n = 1254 || n = 1261 || (n > 1274 && n < 1325)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1255 || n = 1260 || n = 1265 || n = 1270 || n = 1284 || n = 1285 || n = 1290 || n = 1305 || n = 1321 || n = 1326)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1256 || n = 1264 || n = 1312 || n = 1320 || n = 1328)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1257)
	; 8 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1259 || n = 1304 || n = 1313 || n = 1322)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1266)
	; 8 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1269 || n = 1274 || n = 1279 || n = 1299 || n = 1300 || n = 1302 || (n > 1305 && n < 1310) || n = 1311 || n = 1314 || n = 1318 || n = 1323 || n = 1325)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1280)
	; 7 bytes
	php
	ldx #74 ;hides 'lsr a'
	inx
	Jne *-2
	plp
.elseif(n = 1288)
	; 7 bytes
	php
	ldx #0
	dex
	Jne *-1
	plp
.elseif(n = 1291)
	; 9 bytes
	php
	ldx $A6
	ldx #0
	dex
	Jne *-1
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
.elseif(n = 1293)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 1294)
	; 7 bytes
	php
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1295)
	; 9 bytes
	php
	pha
	ldx #0
	dex
	Jne *-1
	pla
	plp
.elseif(n = 1297)
	; 9 bytes
	php
	ldx $A6
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
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
.elseif(n = 1301)
	; 9 bytes
	php
	pha
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1315)
	; 9 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	inx ;first loop only
	nop
	dex
	Jmi *-4
	plp
.elseif(n = 1316)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 1319)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1327)
	; 7 bytes
	php
	lax #5
	jsr delay_256a_x_33_clocks
	plp
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1333 || n = 1347 || n = 1354 || n = 1361 || n = 1375 || n = 1382 || n = 1389 || n = 1396)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1330 || n = 1332 || n = 1334 || n = 1339 || (n > 1340 && n < 1344) || n = 1346 || n = 1350 || n = 1351 || n = 1353 || n = 1355 || n = 1357 || n = 1362 || n = 1363 || n = 1365 || n = 1369 || n = 1371 || n = 1372 || (n > 1376 && n < 1380) || n = 1381 || n = 1390 || n = 1391 || n = 1393 || n = 1395 || (n > 1396 && n < 1400) || n = 1402 || n = 1404 || n = 1405)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1402)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1335)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1336 || n = 1344 || n = 1360 || n = 1368 || n = 1384 || n = 1400)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1337)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jmi *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1339 && n < 1386) || n = 1403))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1348)
	; 9 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	dex
	dex
	Jne *-4
	plp
.elseif(n = 1356)
	; 8 bytes
	php
	ldx #106 ;hides 'ror a'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1364)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1370)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 1374)
	; 8 bytes
	php
	ldx #152 ;hides 'tya'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1383)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1386)
	; 9 bytes
	php
	ldx #138 ;hides 'txa'
	lda $A5
	dex
	Jne *-4
	plp
.elseif(n = 1388)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 1392)
	; 7 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif(n = 1406)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
	plp
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1411 || n = 1414 || (n > 1417 && n < 1421) || n = 1423 || n = 1425 || n = 1426 || n = 1428 || n = 1433 || n = 1435 || n = 1437 || n = 1441 || n = 1442 || n = 1444 || n = 1446 || n = 1447 || n = 1449 || n = 1451 || (n > 1452 && n < 1456) || n = 1458 || n = 1461 || n = 1463 || n = 1465 || (n > 1466 && n < 1471) || n = 1474 || n = 1476 || n = 1477 || n = 1479 || (n > 1480 && n < 1484))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1479)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1409)
	; 9 bytes
	php
	ldx $A6
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
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
.elseif((n .mod 9 = 8) && ((n > 1411 && n < 1449) || (n > 1465 && n < 1485)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1413)
	; 9 bytes
	php
	pha
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
	pla
	plp
.elseif((n .mod 8 = 0) && ((n > 1415 && n < 1441) || n = 1456 || n = 1472 || n = 1480))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 3) && (n = 1417 || (n > 1437 && n < 1460) || n = 1473))
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1427)
	; 7 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	plp
.elseif(n = 1431)
	; 9 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
	plp
.elseif(n = 1434)
	; 9 bytes
	php
	pha
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	pla
	plp
.elseif(n = 1460)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 1462)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jpl *-4
	plp
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if((n .mod 7 = 1) && ((n > 1484 && n < 1521)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1486 || n = 1489 || n = 1491 || n = 1498 || n = 1509 || n = 1510 || n = 1514 || n = 1516 || n = 1517 || n = 1523)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1487 || n = 1494 || n = 1497 || n = 1501 || n = 1505 || n = 1515)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488 || n = 1496 || n = 1512)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1490)
	; 9 bytes
	php
	ldx #106 ;hides 'ror a'
	pla
	pha
	dex
	Jne *-4
	plp
.elseif(n = 1493 || n = 1502)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1495)
	; 8 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1500)
	; 8 bytes
	php
	ldx #90 ;hides 'nop'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1503)
	; 8 bytes
	php
	ldx #69 ;hides 'eor zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1504)
	; 7 bytes
	php
	ldx #42 ;hides 'rol a'
	inx
	Jne *-2
	plp
.elseif(n = 1507)
	; 9 bytes
	php
	ldx $A6
	ldx #42 ;hides 'rol a'
	inx
	Jne *-2
	plp
.elseif(n = 1508)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1511)
	; 8 bytes
	php
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1518)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1519)
	; 8 bytes
	php
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1521)
	; 9 bytes
	php
	ldx $A6
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
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
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 9 bytes
	php
	ldx #138 ;hides 'txa'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 1525)
	; 9 bytes
	php
	pha
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1526)
	; 9 bytes
	php
	ldx #152 ;hides 'tya'
	lda $A5
	dex
	Jne *-4
	plp
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1528 || n = 1544 || n = 1552 || n = 1560)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 1528 && n < 1557)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1530 || n = 1531 || n = 1533 || n = 1537 || n = 1539 || n = 1540 || n = 1542 || n = 1549 || n = 1551 || n = 1553 || n = 1558 || n = 1559 || n = 1561)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1532)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1535)
	; 9 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 1536)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1543 || n = 1550 || n = 1557)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1545)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1546)
	; 9 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 1554)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563)
	; 8 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1564 || n = 1571 || n = 1577 || n = 1578 || n = 1585 || n = 1586 || n = 1593 || n = 1599 || n = 1606 || n = 1613 || n = 1620 || n = 1627)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 8) && (n = 1565 || n = 1574 || (n > 1591 && n < 1638)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1566 || n = 1570 || n = 1572 || n = 1573 || n = 1579 || n = 1581 || n = 1582 || (n > 1586 && n < 1590) || (n > 1593 && n < 1597) || n = 1598 || n = 1602 || n = 1603 || n = 1605 || n = 1607 || n = 1609 || n = 1612 || n = 1614 || n = 1615 || (n > 1620 && n < 1624) || n = 1629 || n = 1631 || n = 1636 || n = 1638)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1567)
	; 8 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1568 || n = 1600 || n = 1608 || n = 1624 || n = 1640)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1640)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1575)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1580)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1584)
	; 7 bytes
	php
	lax #6
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1591)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1616)
	; 7 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif(n = 1617)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 1626)
	; 8 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1630)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
	plp
.elseif(n = 1633)
	; 9 bytes
	php
	ldx $A6
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
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
.elseif(n = 1635)
	; 8 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	nop
	dex
	Jne *-3
	plp
.endif
.else
; 1641 to 1718
.if(n = 1641 || n = 1669 || n = 1683 || n = 1690 || n = 1697 || n = 1711)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1642 || n = 1643 || n = 1645 || n = 1649 || n = 1650 || n = 1659 || n = 1661 || n = 1663 || n = 1665 || n = 1666 || n = 1668 || n = 1670 || n = 1671 || n = 1675 || n = 1677 || n = 1679 || (n > 1683 && n < 1688) || (n > 1691 && n < 1695) || n = 1698 || n = 1699 || n = 1701 || n = 1703 || (n > 1704 && n < 1709) || n = 1710 || (n > 1712 && n < 1716) || n = 1717)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1644)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif((n .mod 7 = 1) && ((n > 1645 && n < 1717)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1647)
	; 9 bytes
	php
	ldx $A6
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1648)
	; 9 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1651)
	; 9 bytes
	php
	pha
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1652 || n = 1676)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1654)
	; 9 bytes
	php
	ldx #73 ;hides 'eor #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1654 && n < 1701) || n = 1718))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1656 || n = 1672 || n = 1680 || n = 1696 || n = 1704 || n = 1712)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1657)
	; 9 bytes
	php
	ldx #165 ;hides 'lda zp'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 1658)
	; 9 bytes
	php
	ldx #138 ;hides 'txa'
	pla
	pha
	inx
	Jne *-4
	plp
.elseif(n = 1662)
	; 8 bytes
	php
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1678)
	; 9 bytes
	php
	ldx #152 ;hides 'tya'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 1689)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1721 || n = 1722 || n = 1726 || n = 1729 || n = 1733 || n = 1734 || n = 1738 || n = 1740 || n = 1741 || n = 1743 || n = 1747 || n = 1750 || n = 1755 || n = 1757 || n = 1762 || n = 1764 || n = 1766 || n = 1770 || n = 1771 || n = 1773 || n = 1778 || n = 1780 || n = 1782 || n = 1783 || n = 1785 || n = 1787 || n = 1789 || n = 1791 || n = 1794)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 8 = 0) && (n = 1720 || (n > 1751 && n < 1793)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1794)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1724 || n = 1748 || n = 1796)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1725 || n = 1732 || n = 1739 || n = 1753 || n = 1761 || n = 1769 || n = 1774 || n = 1777 || n = 1795)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 8) && ((n > 1726 && n < 1764) || n = 1781 || n = 1790))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1728)
	; 7 bytes
	php
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	plp
.elseif(n = 1731)
	; 9 bytes
	php
	ldx $A6
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	plp
.elseif(n = 1735)
	; 9 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	pla
	plp
.elseif(n = 1742)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
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
.elseif(n = 1749)
	; 9 bytes
	php
	pha
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1756)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1759)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1767)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1775)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1788)
	; 8 bytes
	php
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
	plp
.endif
.else
; 1797 to 1875
.if(n = 1797 || n = 1798 || n = 1801 || (n > 1802 && n < 1806) || n = 1807 || (n > 1808 && n < 1816) || n = 1818 || n = 1819 || n = 1821 || n = 1822 || (n > 1826 && n < 1831) || n = 1834 || (n > 1835 && n < 1839) || n = 1845 || n = 1850 || n = 1852 || n = 1854 || n = 1855 || (n > 1856 && n < 1862) || n = 1863 || n = 1865 || n = 1866 || n = 1869 || n = 1870 || (n > 1872 && n < 1876))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 9 = 8) && ((n > 1798 && n < 1872)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1800)
	; 8 bytes
	php
	ldx #0
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1802 || n = 1825 || n = 1843 || n = 1849)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1806)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
	plp
.elseif((n .mod 8 = 0) && (n = 1816 || (n > 1831 && n < 1873)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1820 || n = 1868)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1823)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1824)
	; 8 bytes
	php
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1831)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1833)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	plp
.elseif(n = 1839)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1841)
	; 7 bytes
	php
	lax #7
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1842)
	; 8 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1846)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 1847)
	; 8 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1851)
	; 8 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1867)
	; 9 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
; 1876 to 1953
.if(n = 1876)
	; 9 bytes
	php
	ldx #69 ;hides 'eor zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 1877)
	; 9 bytes
	php
	ldx #69 ;hides 'eor zp'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 1878 || n = 1879 || (n > 1880 && n < 1886) || n = 1890 || n = 1891 || (n > 1892 && n < 1896) || n = 1897 || (n > 1898 && n < 1904) || n = 1906 || (n > 1907 && n < 1912) || n = 1913 || n = 1915 || (n > 1916 && n < 1920) || n = 1921 || n = 1922 || n = 1924 || n = 1926 || n = 1927 || (n > 1928 && n < 1932) || n = 1933 || n = 1935 || n = 1937 || n = 1939 || n = 1941 || (n > 1944 && n < 1950) || n = 1951 || n = 1953)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 9 = 8) && ((n > 1879 && n < 1953)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1886)
	; 9 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 1887)
	; 9 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif((n .mod 8 = 0) && ((n > 1887 && n < 1945)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1892 || n = 1940)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1905)
	; 8 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1914)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1923)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1932)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1938)
	; 9 bytes
	php
	ldx #138 ;hides 'txa'
	pla
	pha
	dex
	Jne *-4
	plp
.elseif(n = 1942)
	; 9 bytes
	php
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 1950)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
	plp
.endif
.else
; 1954 to 2031
.if((n > 1953 && n < 1957) || n = 1958 || n = 1959 || n = 1962 || n = 1963 || n = 1965 || n = 1969 || (n > 1970 && n < 1975) || n = 1978 || (n > 1979 && n < 1984) || n = 1985 || (n > 1988 && n < 1992) || (n > 1992 && n < 1996) || n = 1998 || n = 1999 || (n > 2000 && n < 2006) || n = 2010 || n = 2011 || n = 2013 || n = 2014 || (n > 2017 && n < 2023) || (n > 2025 && n < 2030) || n = 2031)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1957)
	; 9 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif((n .mod 8 = 0) && (n = 1960 || (n > 1975 && n < 2017)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 1960 && n < 2007) || n = 2024))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1964 || n = 2012)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1966)
	; 9 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 1967)
	; 9 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1968)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1975)
	; 9 bytes
	php
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
	plp
.elseif(n = 1977)
	; 9 bytes
	php
	ldx #197 ;hides 'cmp zp'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 1986)
	; 9 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1987)
	; 9 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1996)
	; 9 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	lda $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2007)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2009 || n = 2017 || n = 2025)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2015)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2023)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2030)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
	plp
.endif
.endif
.else
; 2032 to 2187
.if(n = 2032 || n = 2040 || n = 2048)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 2032 && n < 2187)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2034 || n = 2035 || (n > 2036 && n < 2040) || n = 2041 || (n > 2042 && n < 2048) || n = 2049 || n = 2050 || (n > 2051 && n < 2056) || (n > 2056 && n < 2060) || (n > 2060 && n < 2069) || n = 2070 || n = 2071 || (n > 2072 && n < 2076) || n = 2077 || (n > 2078 && n < 2084) || n = 2086 || (n > 2087 && n < 2094) || n = 2095 || n = 2097 || n = 2099 || (n > 2100 && n < 2104) || n = 2107 || (n > 2108 && n < 2112) || n = 2113 || (n > 2114 && n < 2120) || n = 2122 || (n > 2123 && n < 2130) || n = 2131 || n = 2133 || (n > 2134 && n < 2139) || n = 2140 || n = 2142 || n = 2143 || n = 2145 || (n > 2146 && n < 2150) || (n > 2150 && n < 2156) || n = 2157 || n = 2158 || (n > 2159 && n < 2166) || n = 2167 || (n > 2168 && n < 2177) || n = 2178 || n = 2179 || (n > 2180 && n < 2186))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2036 || n = 2072 || n = 2084 || n = 2108 || n = 2120 || n = 2144 || n = 2156 || n = 2180)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2056)
	; 9 bytes
	php
	ldx #0
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2076)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2085)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 2094)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2098)
	; 7 bytes
	php
	lax #8
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2100)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2104)
	; 9 bytes
	php
	ldx #233 ;hides 'sbc #imm'
	lda $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2106)
	; 9 bytes
	php
	ldx #106 ;hides 'ror a'
	pla
	pha
	inx
	Jne *-4
	plp
.elseif(n = 2112)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2121)
	; 8 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2130)
	; 8 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2134)
	; 9 bytes
	php
	ldx #152 ;hides 'tya'
	pla
	pha
	dex
	Jne *-4
	plp
.elseif(n = 2139)
	; 8 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2146)
	; 9 bytes
	php
	ldx #42 ;hides 'rol a'
	lda $A5
	inx
	Jne *-4
	plp
.elseif(n = 2166)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2187)
	; 9 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if((n > 2187 && n < 2192) || n = 2194 || (n > 2197 && n < 2202) || n = 2203 || n = 2205 || (n > 2207 && n < 2211) || n = 2212 || n = 2214 || n = 2215 || n = 2218 || n = 2219 || n = 2221 || (n > 2222 && n < 2228) || n = 2229 || (n > 2231 && n < 2238) || (n > 2240 && n < 2247) || n = 2251 || (n > 2252 && n < 2256) || (n > 2258 && n < 2264))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2192 || n = 2216 || n = 2228 || n = 2252 || n = 2264)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2193)
	; 8 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 2194 && n < 2259)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2196)
	; 9 bytes
	php
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 2197)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2202)
	; 8 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 2206)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2207)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2211)
	; 8 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2217)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2220)
	; 8 bytes
	php
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2230)
	; 9 bytes
	php
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 2238)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2239)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
	plp
.elseif(n = 2247)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	inx
	Jne *-4
	plp
.elseif(n = 2248)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	inx
	Jne *-4
	plp
.elseif(n = 2250)
	; 9 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2256)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2257)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	inx
	Jne *-4
	plp
.elseif(n = 2265)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	inx
	Jne *-4
	plp
.endif
.else
; 2266 to 2343
.if(n = 2266)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 2266 && n < 2304)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif((n > 2267 && n < 2274) || (n > 2277 && n < 2283) || (n > 2288 && n < 2292) || n = 2295 || n = 2298 || n = 2299 || (n > 2303 && n < 2307) || n = 2308 || n = 2309 || (n > 2312 && n < 2324) || n = 2325 || (n > 2326 && n < 2336) || n = 2337 || (n > 2338 && n < 2344))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2274)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
	plp
.elseif(n = 2275)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
	plp
.elseif(n = 2277)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2283)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	inx
	Jne *-4
	plp
.elseif(n = 2284)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	inx
	Jne *-4
	plp
.elseif(n = 2286)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2287)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2288 || n = 2300 || n = 2324 || n = 2336)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2292)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	inx
	Jne *-4
	plp
.elseif(n = 2293)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	inx
	Jne *-4
	plp
.elseif(n = 2296)
	; 9 bytes
	php
	ldx #229 ;hides 'sbc zp'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2297)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2301)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	inx
	Jne *-4
	plp
.elseif(n = 2302)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	inx
	Jne *-4
	plp
.elseif(n = 2307)
	; 9 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2310)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	dex
	Jne *-4
	plp
.elseif(n = 2311)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	inx
	Jne *-4
	plp
.elseif(n = 2312)
	; 9 bytes
	php
	ldx #0
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2326)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	bit $24
	inx
	Jne *-4
	plp
.elseif(n = 2338)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.endif
.endif
.else
; 2344 to 2500
.if(n = 2344 || n = 2345 || n = 2347 || (n > 2349 && n < 2355) || n = 2356 || n = 2358 || n = 2359 || (n > 2360 && n < 2372) || (n > 2372 && n < 2382) || n = 2383 || (n > 2384 && n < 2396) || (n > 2396 && n < 2408) || (n > 2408 && n < 2420) || (n > 2420 && n < 2432) || (n > 2432 && n < 2442) || n = 2443 || (n > 2444 && n < 2456) || (n > 2456 && n < 2466) || n = 2467 || (n > 2468 && n < 2480) || (n > 2480 && n < 2486) || (n > 2486 && n < 2492) || (n > 2492 && n < 2501))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2346)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	bit $24
	dex
	Jne *-4
	plp
.elseif((n .mod 12 = 8) && ((n > 2347 && n < 2493)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2349)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2355)
	; 7 bytes
	php
	lax #9
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2357)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2382)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2442)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 2466)
	; 9 bytes
	php
	ldx #10 ;hides 'asl a'
	lda $A5
	inx
	Jne *-4
	plp
.elseif(n = 2486)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	bit $24
	dex
	Jne *-4
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
; 2501 to 2812
.if((n > 2500 && n < 2504) || n = 2505 || n = 2506 || (n > 2507 && n < 2516) || (n > 2517 && n < 2527) || (n > 2528 && n < 2540) || (n > 2540 && n < 2547) || (n > 2547 && n < 2552) || n = 2553 || (n > 2554 && n < 2558) || (n > 2558 && n < 2564) || (n > 2564 && n < 2576) || (n > 2576 && n < 2580) || n = 2581 || (n > 2582 && n < 2588) || n = 2589 || (n > 2591 && n < 2600) || n = 2601 || (n > 2602 && n < 2612) || n = 2613 || (n > 2614 && n < 2624) || (n > 2624 && n < 2636) || (n > 2636 && n < 2648) || (n > 2648 && n < 2660) || (n > 2660 && n < 2672) || (n > 2672 && n < 2684) || (n > 2684 && n < 2690) || (n > 2690 && n < 2696) || (n > 2696 && n < 2700) || (n > 2701 && n < 2708) || (n > 2708 && n < 2712) || (n > 2712 && n < 2720) || (n > 2720 && n < 2732) || n = 2733 || (n > 2734 && n < 2744) || (n > 2744 && n < 2756) || (n > 2756 && n < 2768) || (n > 2768 && n < 2780) || (n > 2780 && n < 2792) || (n > 2792 && n < 2804) || n = 2805 || (n > 2806 && n < 2813))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 12 = 8) && ((n > 2503 && n < 2601) || (n > 2623 && n < 2805)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2507)
	; 9 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2517)
	; 9 bytes
	php
	ldx #5 ;hides 'ora zp'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2527)
	; 9 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2547)
	; 9 bytes
	php
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 2554)
	; 9 bytes
	php
	ldx #74 ;hides 'lsr a'
	pla
	pha
	inx
	Jne *-4
	plp
.elseif(n = 2558)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2580)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2582)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 2590)
	; 9 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 2591)
	; 9 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2602)
	; 9 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2612)
	; 7 bytes
	php
	lax #10
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2614)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2690)
	; 9 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2700)
	; 9 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	cmp $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2701)
	; 9 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2712)
	; 9 bytes
	php
	ldx #10 ;hides 'asl a'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2734)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2806)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
	plp
.endif
.else
; 2813 to 3125
.if((n > 2812 && n < 2816) || (n > 2816 && n < 2828) || (n > 2828 && n < 2840) || (n > 2840 && n < 2848) || (n > 2848 && n < 2852) || (n > 2852 && n < 2864) || (n > 2864 && n < 2869) || n = 2870 || (n > 2871 && n < 2876) || (n > 2876 && n < 2888) || (n > 2888 && n < 2900) || (n > 2900 && n < 2912) || (n > 2912 && n < 2924) || (n > 2924 && n < 2936) || (n > 2936 && n < 2948) || (n > 2948 && n < 2960) || (n > 2960 && n < 2972) || (n > 2972 && n < 2984) || (n > 2984 && n < 2996) || (n > 2996 && n < 3002) || (n > 3002 && n < 3008) || (n > 3008 && n < 3020) || (n > 3020 && n < 3030) || n = 3031 || (n > 3032 && n < 3044) || (n > 3044 && n < 3056) || (n > 3056 && n < 3068) || (n > 3068 && n < 3080) || (n > 3080 && n < 3126))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 12 = 8) && ((n > 2815 && n < 3069)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2848)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	plp
.elseif(n = 2869)
	; 7 bytes
	php
	lax #11
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2871)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3002)
	; 9 bytes
	php
	ldx #42 ;hides 'rol a'
	pla
	pha
	inx
	Jne *-4
	plp
.elseif(n = 3030)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3080)
	; 9 bytes
	php
	ldx #0
	delay_n 7
	dex
	Jne *-3
	plp
.endif
.endif
.else
; 3126 to 3750
.if(n = 3126)
	; 7 bytes
	php
	lax #12
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3127 || (n > 3128 && n < 3254) || (n > 3254 && n < 3282) || (n > 3282 && n < 3383) || n = 3384 || (n > 3385 && n < 3450) || (n > 3450 && n < 3478) || (n > 3478 && n < 3640) || n = 3641 || (n > 3642 && n < 3751))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3128 || n = 3385 || n = 3642)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3254)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 3282)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3383)
	; 7 bytes
	php
	lax #13
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3450)
	; 9 bytes
	php
	ldx #10 ;hides 'asl a'
	pla
	pha
	inx
	Jne *-4
	plp
.elseif(n = 3478)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3640)
	; 7 bytes
	php
	lax #14
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.else
; 3751 to 5000
.if((n > 3750 && n < 3897) || n = 3898 || (n > 3899 && n < 4154) || n = 4155 || (n > 4156 && n < 4411) || n = 4412 || (n > 4413 && n < 4668) || n = 4669 || (n > 4670 && n < 4925) || n = 4926 || (n > 4927 && n < 5001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3897)
	; 7 bytes
	php
	lax #15
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3899 || n = 4156 || n = 4413 || n = 4670 || n = 4927)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4154)
	; 7 bytes
	php
	lax #16
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4411)
	; 7 bytes
	php
	lax #17
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4668)
	; 7 bytes
	php
	lax #18
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4925)
	; 7 bytes
	php
	lax #19
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5182) || n = 5183 || (n > 5184 && n < 5439) || n = 5440 || (n > 5441 && n < 5696) || n = 5697 || (n > 5698 && n < 5953) || n = 5954 || (n > 5955 && n < 6210) || n = 6211 || (n > 6212 && n < 6467) || n = 6468 || (n > 6469 && n < 6724) || n = 6725 || (n > 6726 && n < 6981) || n = 6982 || (n > 6983 && n < 7238) || n = 7239 || (n > 7240 && n < 7495) || n = 7496 || (n > 7497 && n < 7752) || n = 7753 || (n > 7754 && n < 8009) || n = 8010 || (n > 8011 && n < 8266) || n = 8267 || (n > 8268 && n < 8523) || n = 8524 || (n > 8525 && n < 8780) || n = 8781 || (n > 8782 && n < 9037) || n = 9038 || (n > 9039 && n < 9294) || n = 9295 || (n > 9296 && n < 9551) || n = 9552 || (n > 9553 && n < 9808) || n = 9809 || (n > 9810 && n < 10001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5182)
	; 7 bytes
	php
	lax #20
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5184 || n = 5441 || n = 5698 || n = 5955 || n = 6212 || n = 6469 || n = 6726 || n = 6983 || n = 7240 || n = 7497 || n = 7754 || n = 8011 || n = 8268 || n = 8525 || n = 8782 || n = 9039 || n = 9296 || n = 9553 || n = 9810)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5439)
	; 7 bytes
	php
	lax #21
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5696)
	; 7 bytes
	php
	lax #22
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5953)
	; 7 bytes
	php
	lax #23
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6210)
	; 7 bytes
	php
	lax #24
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6467)
	; 7 bytes
	php
	lax #25
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6724)
	; 7 bytes
	php
	lax #26
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6981)
	; 7 bytes
	php
	lax #27
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7238)
	; 7 bytes
	php
	lax #28
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7495)
	; 7 bytes
	php
	lax #29
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7752)
	; 7 bytes
	php
	lax #30
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8009)
	; 7 bytes
	php
	lax #31
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8266)
	; 7 bytes
	php
	lax #32
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8523)
	; 7 bytes
	php
	lax #33
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8780)
	; 7 bytes
	php
	lax #34
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9037)
	; 7 bytes
	php
	lax #35
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9294)
	; 7 bytes
	php
	lax #36
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9551)
	; 7 bytes
	php
	lax #37
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9808)
	; 7 bytes
	php
	lax #38
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10065) || n = 10066 || (n > 10067 && n < 10322) || n = 10323 || (n > 10324 && n < 10579) || n = 10580 || (n > 10581 && n < 10836) || n = 10837 || (n > 10838 && n < 11093) || n = 11094 || (n > 11095 && n < 11350) || n = 11351 || (n > 11352 && n < 11607) || n = 11608 || (n > 11609 && n < 11864) || n = 11865 || (n > 11866 && n < 12121) || n = 12122 || (n > 12123 && n < 12378) || n = 12379 || (n > 12380 && n < 12635) || n = 12636 || (n > 12637 && n < 12892) || n = 12893 || (n > 12894 && n < 13149) || n = 13150 || (n > 13151 && n < 13406) || n = 13407 || (n > 13408 && n < 13663) || n = 13664 || (n > 13665 && n < 13920) || n = 13921 || (n > 13922 && n < 14177) || n = 14178 || (n > 14179 && n < 14434) || n = 14435 || (n > 14436 && n < 14691) || n = 14692 || (n > 14693 && n < 14948) || n = 14949 || (n > 14950 && n < 15001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10065)
	; 7 bytes
	php
	lax #39
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10067 || n = 10324 || n = 10581 || n = 10838 || n = 11095 || n = 11352 || n = 11609 || n = 11866 || n = 12123 || n = 12380 || n = 12637 || n = 12894 || n = 13151 || n = 13408 || n = 13665 || n = 13922 || n = 14179 || n = 14436 || n = 14693 || n = 14950)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10322)
	; 7 bytes
	php
	lax #40
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10579)
	; 7 bytes
	php
	lax #41
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10836)
	; 7 bytes
	php
	lax #42
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11093)
	; 7 bytes
	php
	lax #43
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11350)
	; 7 bytes
	php
	lax #44
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11607)
	; 7 bytes
	php
	lax #45
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11864)
	; 7 bytes
	php
	lax #46
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12121)
	; 7 bytes
	php
	lax #47
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12378)
	; 7 bytes
	php
	lax #48
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12635)
	; 7 bytes
	php
	lax #49
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12892)
	; 7 bytes
	php
	lax #50
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13149)
	; 7 bytes
	php
	lax #51
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13406)
	; 7 bytes
	php
	lax #52
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13663)
	; 7 bytes
	php
	lax #53
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13920)
	; 7 bytes
	php
	lax #54
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14177)
	; 7 bytes
	php
	lax #55
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14434)
	; 7 bytes
	php
	lax #56
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14691)
	; 7 bytes
	php
	lax #57
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14948)
	; 7 bytes
	php
	lax #58
	jsr delay_256a_x_33_clocks
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15205) || n = 15206 || (n > 15207 && n < 15462) || n = 15463 || (n > 15464 && n < 15719) || n = 15720 || (n > 15721 && n < 15976) || n = 15977 || (n > 15978 && n < 16233) || n = 16234 || (n > 16235 && n < 16490) || n = 16491 || (n > 16492 && n < 16747) || n = 16748 || (n > 16749 && n < 17004) || n = 17005 || (n > 17006 && n < 17261) || n = 17262 || (n > 17263 && n < 17518) || n = 17519 || (n > 17520 && n < 17775) || n = 17776 || (n > 17777 && n < 18032) || n = 18033 || (n > 18034 && n < 18289) || n = 18290 || (n > 18291 && n < 18546) || n = 18547 || (n > 18548 && n < 18803) || n = 18804 || (n > 18805 && n < 19060) || n = 19061 || (n > 19062 && n < 19317) || n = 19318 || (n > 19319 && n < 19574) || n = 19575 || (n > 19576 && n < 19831) || n = 19832 || (n > 19833 && n < 20001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15205)
	; 7 bytes
	php
	lax #59
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15207 || n = 15464 || n = 15721 || n = 15978 || n = 16235 || n = 16492 || n = 16749 || n = 17006 || n = 17263 || n = 17520 || n = 17777 || n = 18034 || n = 18291 || n = 18548 || n = 18805 || n = 19062 || n = 19319 || n = 19576 || n = 19833)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15462)
	; 7 bytes
	php
	lax #60
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15719)
	; 7 bytes
	php
	lax #61
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15976)
	; 7 bytes
	php
	lax #62
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 16233)
	; 7 bytes
	php
	lax #63
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 16490)
	; 7 bytes
	php
	lax #64
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 16747)
	; 7 bytes
	php
	lax #65
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17004)
	; 7 bytes
	php
	lax #66
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17261)
	; 7 bytes
	php
	lax #67
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17518)
	; 7 bytes
	php
	lax #68
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17775)
	; 7 bytes
	php
	lax #69
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18032)
	; 7 bytes
	php
	lax #70
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18289)
	; 7 bytes
	php
	lax #71
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18546)
	; 7 bytes
	php
	lax #72
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18803)
	; 7 bytes
	php
	lax #73
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19060)
	; 7 bytes
	php
	lax #74
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19317)
	; 7 bytes
	php
	lax #75
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19574)
	; 7 bytes
	php
	lax #76
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19831)
	; 7 bytes
	php
	lax #77
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
