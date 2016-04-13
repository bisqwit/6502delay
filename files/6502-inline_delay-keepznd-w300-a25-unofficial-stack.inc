; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, Y, and S
; Will preserve P
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; WARNING: May write garbage into $300-$3FF.
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
; - 8 bytes or less when n <= 298
; - 9 bytes or less when n <= 1306
; -10 bytes or less when n <= 1892
; -11 bytes or less when n <= 2324
; -12 bytes or less when n <= 4044
; -13 bytes or less when n <= 6252
; -14 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=3, avg=3.9, max=5
; - When 21 <= n <= 30: min=5, avg=5.8, max=6
; - When 31 <= n <= 50: min=6, avg=6.9, max=7
; - When 51 <= n <= 75: min=6, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=6, avg=7.1, max=9
; - When 301 <= n <= 500: min=6, avg=8.0, max=9
; - When 501 <= n <= 750: min=6, avg=8.0, max=9
; - When 751 <= n <= 1000: min=7, avg=8.1, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.7, max=11
; - When 2001 <= n <= 3000: min=8, avg=9.9, max=12
; - When 3001 <= n <= 5000: min=9, avg=11.1, max=13
; - When 5001 <= n <= 7500: min=10, avg=12.0, max=14
; - When 7501 <= n <= 10000: min=11, avg=12.2, max=14
; - When 10001 <= n <= 20000: min=11, avg=12.3, max=14
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
.elseif(n = 4 || n = 5 || n = 8 || n = 16 || n = 17 || n = 20 || n = 32)
	; 2..7 bytes
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
.elseif((n > 8 && n < 12) || n = 13 || n = 19 || (n > 20 && n < 24) || n = 25)
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
.elseif(n = 18 || n = 27 || n = 29 || n = 30)
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
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif((n > 33 && n < 46) || (n > 46 && n < 56) || (n > 56 && n < 131) || (n > 131 && n < 157))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 46)
	; 6 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	plp
.elseif(n = 56)
	; 6 bytes
	php
	ldy #136 ;hides 'dey'
	Jmi *-1
	plp
.elseif(n = 131)
	; 6 bytes
	php
	ldx #232 ;hides 'inx'
	Jne *-1
	plp
.endif
.else
; 157 to 312
.if((n > 156 && n < 290))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 290 || n = 305 || n = 310)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 291)
	; 6 bytes
	php
	ldy #200 ;hides 'iny'
	Jmi *-1
	plp
.elseif(n = 292 || n = 294 || n = 296 || n = 301)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 293 || n = 298 || n = 303)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 295)
	; 8 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	plp
.elseif(n = 297 || n = 299)
	; 8..9 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 315 || n = 320 || n = 325 || n = 329 || n = 335 || n = 340 || n = 345 || n = 350 || n = 355 || n = 360 || n = 370 || n = 374 || n = 375 || n = 380 || n = 385)
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
.elseif(n = 317 || n = 322 || n = 352 || n = 357)
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
	ldy #186 ;hides 'tsx'
	iny
	iny
	Jne *-3
	plp
.elseif(n = 324 || n = 334 || n = 354 || n = 364 || n = 369)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 326 || n = 331 || n = 336 || n = 341 || n = 346 || n = 356 || n = 361 || n = 371 || n = 376 || n = 381 || n = 387 || n = 389)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 327)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jmi *-3
	plp
.elseif(n = 328 || n = 333 || n = 343 || n = 363)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 332 || n = 342 || n = 347 || n = 362 || n = 367 || n = 377 || n = 382)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
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
.elseif(n = 349)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	inx
	Jne *-4
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
.if(n = 391 || n = 396 || n = 401 || n = 406 || n = 416 || n = 422 || n = 426 || n = 431 || n = 436 || n = 441 || n = 446 || n = 451 || n = 461 || n = 466)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 392 || n = 399 || n = 434 || n = 454 || n = 459)
	; 8..9 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 5 = 3) && ((n > 392 && n < 409) || n = 418 || n = 433 || n = 448 || (n > 457 && n < 469)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 394)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	inx
	Jne *-4
	plp
.elseif(n = 395 || n = 410 || n = 415 || n = 420 || n = 425 || n = 430 || n = 440 || n = 444 || n = 445 || n = 450 || n = 455 || n = 460 || n = 465)
	; 8..9 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 417 || n = 432 || n = 467)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 419)
	; 7 bytes
	php
	ldy #186 ;hides 'tsx'
	dey
	Jmi *-2
	plp
.elseif(n = 424)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	plp
.elseif(n = 429)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $AEA2'
	ldx #174
	dex
	Jmi *-4
	plp
.elseif(n = 439)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $AFA2'
	ldx #175
	dex
	Jmi *-4
	plp
.elseif(n = 462)
	; 8 bytes
	php
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
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
.if(n = 469)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 476 || n = 481 || n = 486 || n = 499 || n = 506)
	; 8 bytes
	pha
	delay_n (n-3)
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
	ldy #186 ;hides 'tsx'
	iny
	Jne *-2
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
.elseif(n = 509)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 510)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
	plp
.elseif(n = 511 || n = 516 || n = 521 || n = 527 || n = 534 || n = 536 || n = 541)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 517)
	; 7 bytes
	php
	ldx #200 ;hides 'iny'
	dex
	Jmi *-2
	plp
.elseif(n = 522 || n = 532 || n = 542)
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
.elseif(n = 529)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B9A2'
	ldx #185
	dex
	Jmi *-4
	plp
.elseif(n = 531)
	; 7 bytes
	php
	ldx #74 ;hides 'lsr a'
	dex
	Jpl *-2
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
.elseif(n = 546)
	; 8 bytes
	php
	ldx #136 ;hides 'dey'
	inx
	inx
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
.elseif((n .mod 5 = 1) && (n = 556 || (n > 565 && n < 592) || (n > 600 && n < 622)))
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 567)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 574)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	inx
	Jne *-4
	plp
.elseif(n = 575 || n = 589 || n = 596 || n = 610 || n = 617)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 577 || n = 587 || n = 592 || n = 602)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 579 || n = 604)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
	nop $04
	Jmi *-3
	plp
.elseif(n = 622)
	; 7 bytes
	php
	ldx #168 ;hides 'tay'
	inx
	Jne *-2
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
.if(n = 626 || n = 632 || n = 634 || n = 639 || n = 641 || n = 646 || n = 651 || n = 656 || n = 661 || n = 669 || n = 671 || n = 676 || n = 686 || n = 694 || n = 696)
	; 7..9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 636)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 637 || n = 647 || n = 657)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 638 || n = 653)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 642 || n = 662 || n = 667 || n = 677 || n = 682 || n = 692 || n = 702)
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
.elseif(n = 645 || n = 652 || n = 680 || n = 687 || n = 701)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 649 || n = 679 || n = 684 || n = 697)
	; 8..9 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 691)
	; 6 bytes
	php
	ldy #136 ;hides 'dey'
	Jne *-1
	plp
.elseif(n = 695)
	; 8 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	plp
.elseif(n = 699)
	; 8 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	dex ;first loop only
	inx
	Jne *-3
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
.elseif(n = 706 || n = 716 || n = 721 || n = 726 || n = 731 || n = 737 || n = 744 || n = 746 || n = 756 || n = 761 || n = 766 || n = 776 || n = 781)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && ((n > 706 && n < 718) || n = 732 || (n > 741 && n < 753) || (n > 766 && n < 778)))
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
.elseif(n = 709)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	inx
	Jne *-4
	plp
.elseif(n = 711)
	; 8 bytes
	php
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	iny
	Jne *-3
	plp
.elseif(n = 714 || n = 739 || n = 749 || n = 769 || n = 774 || n = 779)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 7 = 1) && ((n > 714 && n < 730) || (n > 749 && n < 772)))
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 719)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	iny
	Jne *-3
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
.elseif(n = 741)
	; 7 bytes
	php
	ldy #232 ;hides 'inx'
	dey
	Jmi *-2
	plp
.elseif(n = 751)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 754)
	; 9 bytes
	php
	ldx #136 ;hides 'dey'
	nop
	dex
	dex
	Jne *-4
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
.if((n >= 782) && (n < 860))
; 782 to 859
.if((n .mod 5 = 2) && (n = 782 || n = 787 || n = 797 || n = 802 || (n > 811 && n < 823) || (n > 836 && n < 858)))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif((n .mod 5 = 3) && ((n > 782 && n < 859)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 784 || n = 824)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 785 || n = 792 || n = 806 || n = 820 || n = 841 || n = 855)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 786 || n = 791 || n = 796 || n = 801 || n = 811 || n = 821 || n = 826 || n = 836 || n = 849 || n = 851 || n = 856)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 789)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 790 || n = 794 || n = 795 || n = 800 || n = 805 || n = 810 || n = 815 || n = 825 || n = 829 || n = 830 || n = 835 || n = 840 || n = 845 || n = 850)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 799 || n = 827 || n = 834)
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
.elseif(n = 814 || n = 859)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 816)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 819)
	; 9 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	iny
	Jne *-4
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
.elseif(n = 844)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $A3A2'
	ldx #163
	inx
	Jne *-4
	plp
.elseif(n = 846)
	; 7 bytes
	php
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
	plp
.endif
.else
; 860 to 937
.if(n = 860)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif((n .mod 5 = 1) && ((n > 860 && n < 872) || (n > 880 && n < 902) || (n > 915 && n < 937)))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 862 || n = 869 || n = 890 || n = 911)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif((n .mod 5 = 3) && ((n > 862 && n < 934)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 864)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 865 || n = 870 || n = 875 || n = 880 || n = 885 || n = 895 || n = 900 || n = 905 || n = 910 || n = 915 || n = 920 || n = 930 || n = 934 || n = 935)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 867)
	; 7 bytes
	php
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
	plp
.elseif((n .mod 5 = 2) && ((n > 871 && n < 893) || (n > 901 && n < 928) || n = 937))
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 874)
	; 9 bytes
	php
	pha
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
	pla
	plp
.elseif(n = 876 || n = 897 || n = 904 || n = 925 || n = 932)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 879)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $61A2'
	ldx #97
	dex
	Jne *-4
	plp
.elseif(n = 884)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 889)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jpl *-4
	plp
.elseif(n = 894 || n = 909 || n = 914)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 919)
	; 9 bytes
	php
	ldx #90 ;hides 'nop'
	nop
	inx
	inx
	Jne *-4
	plp
.elseif(n = 924)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 929)
	; 9 bytes
	php
	ldx #(n-5)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.endif
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
.elseif((n .mod 5 = 0) && ((n > 939 && n < 956) || (n > 969 && n < 991) || (n > 999 && n < 1016)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 941 || n = 947 || n = 956 || n = 961 || n = 966 || n = 971 || n = 976 || n = 986 || n = 996 || n = 1001 || n = 1006 || n = 1011)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 949 || n = 954 || n = 959 || n = 964 || n = 994)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 951)
	; 8 bytes
	php
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
	plp
.elseif(n = 952 || n = 957 || n = 962 || n = 977 || n = 982 || n = 992 || n = 997 || n = 1007 || n = 1012)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 965)
	; 7 bytes
	php
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
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
.elseif(n = 979)
	; 9 bytes
	php
	pha
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 984)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
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
.elseif(n = 1019 || n = 1027 || n = 1049 || n = 1054 || n = 1084)
	; 8..9 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 1024 || n = 1026 || n = 1031 || n = 1036 || n = 1046 || n = 1061 || n = 1066 || n = 1071 || n = 1076 || n = 1081 || n = 1091)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1025)
	; 8 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	plp
.elseif(n = 1029)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	nop
	dey
	dey
	Jne *-4
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
.elseif(n = 1069)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jpl *-4
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
.elseif(n = 1089)
	; 9 bytes
	php
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	Jmi *-4
	plp
.endif
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1099 || n = 1119 || n = 1129 || n = 1134 || n = 1139 || n = 1154 || n = 1164 || n = 1174 || n = 1209 || n = 1229 || n = 1234)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1095 || n = 1105 || n = 1109 || n = 1110 || n = 1115 || n = 1120 || n = 1125 || n = 1130 || n = 1140 || n = 1145 || n = 1150 || n = 1155 || n = 1160 || n = 1165 || n = 1175 || n = 1179 || n = 1180 || n = 1185 || n = 1190 || n = 1195 || n = 1200 || n = 1210 || n = 1215 || n = 1220 || n = 1225 || n = 1230 || n = 1235 || n = 1245 || n = 1249 || n = 1250)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1096 || n = 1101 || n = 1106 || n = 1111 || n = 1116 || n = 1126 || n = 1136 || n = 1141 || n = 1146 || n = 1151 || n = 1161 || n = 1166 || n = 1171 || n = 1176 || n = 1181 || n = 1186 || n = 1199 || n = 1201 || n = 1206 || n = 1211 || n = 1216 || n = 1221 || n = 1231 || n = 1236 || n = 1241 || n = 1246)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && (n = 1097 || n = 1112 || n = 1117 || (n > 1126 && n < 1138) || (n > 1146 && n < 1173) || (n > 1186 && n < 1208) || (n > 1216 && n < 1243)))
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
.elseif(n = 1102)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
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
.elseif(n = 1144 || n = 1224)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1159)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $80A2'
	ldx #128
	dex
	Jne *-4
	plp
.elseif(n = 1169)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 1182)
	; 7 bytes
	php
	ldx #168 ;hides 'tay'
	dex
	Jne *-2
	plp
.elseif(n = 1189)
	; 9 bytes
	php
	pha
	ldx #168 ;hides 'tay'
	dex
	Jne *-2
	pla
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
.elseif(n = 1196)
	; 7 bytes
	php
	ldy #170 ;hides 'tax'
	dey
	Jne *-2
	plp
.elseif(n = 1204)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $85A2'
	ldx #133
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
.if((n >= 1251) && (n < 1290))
; 1251 to 1289
.if((n .mod 5 = 1) && (n = 1251 || n = 1256 || (n > 1270 && n < 1287)))
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1254 || n = 1261 || n = 1275 || n = 1282 || n = 1289)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1255 || n = 1260 || n = 1265 || n = 1270 || n = 1284 || n = 1285)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1257)
	; 8 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1259)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1264)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
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
.elseif(n = 1269 || n = 1274 || n = 1279)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.endif
.else
; 1290 to 1328
.if(n = 1290 || n = 1305 || n = 1321 || n = 1326)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1291 || n = 1297 || n = 1299 || n = 1306 || n = 1325)
	; 8..10 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 7 = 1) && ((n > 1295 && n < 1325)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
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
.elseif(n = 1300 || n = 1314)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1301)
	; 9 bytes
	php
	pha
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1302)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $90A2'
	ldx #144
	dex
	Jne *-4
	plp
.elseif(n = 1304 || n = 1313 || n = 1322)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1307)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1308)
	; 7 bytes
	php
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
	plp
.elseif(n = 1309)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1311)
	; 8 bytes
	php
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1312 || n = 1320 || n = 1328)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
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
.elseif(n = 1318)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1319)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1323)
	; 10 bytes
	php
	tya
	ldx #164 ;hides 'ldy zp'
	nop
	dex
	Jne *-3
	tay
	plp
.elseif(n = 1327)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	nop
	dex
	Jne *-3
	plp
.endif
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1333 || n = 1347 || n = 1353 || n = 1354 || n = 1361)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1330)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $6DA2'
	ldx #109
	inx
	Jne *-4
	plp
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1367)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1332)
	; 10 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	nop
	inx
	inx
	Jne *-5
	plp
.elseif(n = 1334 || n = 1341 || n = 1355 || n = 1362)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1335)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1336 || n = 1344 || n = 1360)
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
.elseif(n = 1339)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $6CA2'
	ldx #108
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1339 && n < 1368)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1342 || n = 1363)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1343)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
	plp
.elseif(n = 1346)
	; 9 bytes
	php
	ldx #122 ;hides 'nop'
	nop $04
	inx
	Jne *-4
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
.elseif(n = 1350)
	; 10 bytes
	php
	ldx #121
	delay_n 4
	nop
	dex
	Jpl *-4
	plp
.elseif(n = 1351)
	; 8 bytes
	php
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	dey
	Jne *-3
	plp
.elseif(n = 1356)
	; 8 bytes
	php
	ldx #106 ;hides 'ror a'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1357)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $6AA2'
	ldx #106
	inx
	Jne *-4
	plp
.elseif(n = 1364)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1365)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $97A2'
	ldx #151
	dex
	Jne *-4
	plp
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1384 || n = 1400)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1369 || n = 1377 || n = 1390 || n = 1395 || n = 1397 || n = 1404)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1370)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 1371)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	inx
	Jpl *-5
	plp
.elseif(n = 1372)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif((n .mod 7 = 1) && ((n > 1372 && n < 1402)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1374)
	; 8 bytes
	php
	ldx #152 ;hides 'tya'
	nop
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 3) && ((n > 1374 && n < 1397)))
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1376 || n = 1385 || n = 1403)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1378)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1379 || n = 1398)
	; 9..10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1381)
	; 9 bytes
	php
	ldx #250 ;hides 'nop'
	nop
	dex
	dex
	Jne *-4
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
.elseif(n = 1391 || n = 1405)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1392)
	; 7 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif(n = 1393)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $9AA2'
	ldx #154
	dex
	Jne *-4
	plp
.elseif(n = 1399)
	; 9 bytes
	php
	pha
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
	pla
	plp
.elseif(n = 1402)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $9BA2'
	ldx #155
	dex
	Jne *-4
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
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1435)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1444)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1409 || n = 1418 || n = 1423 || n = 1425 || n = 1430)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1410)
	; 9 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
	plp
.elseif(n = 1411)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $9CA2'
	ldx #156
	dex
	Jne *-4
	plp
.elseif(n = 1412 || n = 1421 || n = 1439)
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
.elseif(n = 1414 || n = 1417 || n = 1438 || n = 1441 || n = 1442 || n = 1445)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 0) && ((n > 1415 && n < 1441)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1419)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $63A2'
	ldx #99
	inx
	Jne *-4
	plp
.elseif(n = 1420)
	; 7 bytes
	php
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
	plp
.elseif(n = 1426 || n = 1433)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1427)
	; 7 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	plp
.elseif(n = 1428)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $62A2'
	ldx #98
	inx
	Jne *-4
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
.elseif(n = 1437)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $61A2'
	ldx #97
	inx
	Jne *-4
	plp
.elseif(n = 1444)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.endif
.else
; 1446 to 1484
.if(n = 1446)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
	plp
.elseif(n = 1447)
	; 9 bytes
	php
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	plp
.elseif(n = 1448 || n = 1466 || n = 1475 || n = 1484)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1449)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif((n .mod 7 = 1) && ((n > 1449 && n < 1479)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1451 || n = 1453 || n = 1467 || n = 1479 || n = 1481)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1452 || n = 1459 || n = 1473 || n = 1477)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1454 || n = 1461 || n = 1468)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1455)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $A1A2'
	ldx #161
	dex
	Jne *-4
	plp
.elseif(n = 1456 || n = 1472 || n = 1480)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1458)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 1463)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1465)
	; 9 bytes
	php
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
	plp
.elseif(n = 1469)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $7BA2'
	ldx #123
	nop
	inx
	Jne *-5
	plp
.elseif(n = 1470)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $85A2'
	ldx #133
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1474)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $A3A2'
	ldx #163
	dex
	Jne *-4
	plp
.elseif(n = 1476)
	; 9 bytes
	php
	ldy #232 ;hides 'inx'
	delay_n 7
	dey
	Jmi *-4
	plp
.elseif(n = 1482)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $A4A2'
	ldx #164
	dex
	Jne *-4
	plp
.elseif(n = 1483)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $A4A2'
	ldx #164
	dex
	Jne *-4
	plp
.endif
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
.elseif(n = 1486 || n = 1487 || n = 1494 || n = 1497 || n = 1501 || n = 1505 || n = 1515)
	; 9..10 bytes
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
.elseif(n = 1489 || n = 1517)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
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
.elseif(n = 1491)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $A5A2'
	ldx #165
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
.elseif(n = 1498 || n = 1507 || n = 1514 || n = 1516 || n = 1521 || n = 1523)
	; 8..9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1508)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1509)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $A7A2'
	ldx #167
	dex
	Jne *-4
	plp
.elseif(n = 1510)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $A7A2'
	ldx #167
	dex
	Jne *-4
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
.elseif(n = 1530 || n = 1535 || n = 1551 || n = 1558)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1531 || n = 1559)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1532)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1533)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1536)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1537)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $AAA2'
	ldx #170
	dex
	Jne *-4
	plp
.elseif(n = 1539)
	; 9 bytes
	php
	pha
	ldx #(n-13)/7 ;hides 'nop'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1540 || n = 1543 || n = 1549 || n = 1550 || n = 1557)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1542)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 1553)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jmi *-5
	plp
.elseif(n = 1554)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1561)
	; 10 bytes
	php
	pha
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
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
.if(n = 1563)
	; 8 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1564 || n = 1571 || n = 1577 || n = 1578 || n = 1585 || n = 1593 || n = 1599)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1574 || n = 1592 || n = 1601)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1566)
	; 9 bytes
	php
	ldx #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 1567)
	; 8 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1568 || n = 1584 || n = 1600)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1598)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1570 || n = 1579 || n = 1586 || n = 1594)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1572)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $AEA2'
	ldx #174
	dex
	Jne *-4
	plp
.elseif(n = 1573)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $AEA2'
	ldx #174
	dex
	Jne *-4
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
.elseif(n = 1581)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $AFA2'
	ldx #175
	dex
	Jne *-4
	plp
.elseif(n = 1582)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $AFA2'
	ldx #175
	dex
	Jne *-4
	plp
.elseif(n = 1587)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1588 || n = 1598)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1589)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 1591)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
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
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.endif
.else
; 1602 to 1640
.if(n = 1602 || n = 1606 || n = 1612 || n = 1613 || n = 1620 || n = 1627)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1603)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif((n .mod 7 = 1) && ((n > 1603 && n < 1640)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1605)
	; 10 bytes
	php
	ldx #10 ;hides 'asl a'
	delay_n 4
	inx
	inx
	Jne *-5
	plp
.elseif(n = 1607 || n = 1614 || n = 1619 || n = 1621 || n = 1629 || n = 1633 || n = 1638)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1608 || n = 1624 || n = 1640)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1609)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	dex
	Jne *-4
	plp
.elseif(n = 1610 || n = 1628 || n = 1637)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1615)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
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
.elseif(n = 1622)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1623)
	; 9 bytes
	php
	pha
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
	pla
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
.elseif(n = 1631)
	; 10 bytes
	php
	ldx #250 ;hides 'nop'
	delay_n 4
	dex
	dex
	Jne *-5
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
.elseif(n = 1636)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B5A2'
	ldx #181
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641 || n = 1659 || n = 1669 || n = 1675 || n = 1679)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1642 || n = 1647 || n = 1649 || n = 1665 || n = 1670)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1643)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1644)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1645)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B6A2'
	ldx #182
	dex
	Jne *-4
	plp
.elseif((n .mod 7 = 1) && ((n > 1645 && n < 1675)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
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
.elseif(n = 1650)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 1651)
	; 9 bytes
	php
	pha
	ldx #(n-13)/7 ;hides 'nop'
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
.elseif(n = 1655 || n = 1664 || n = 1673)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1656 || n = 1672)
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
.elseif(n = 1661)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
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
.elseif(n = 1663)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B8A2'
	ldx #184
	dex
	Jne *-4
	plp
.elseif(n = 1666)
	; 9 bytes
	php
	ldy #166 ;hides 'ldx zp'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
	plp
.elseif(n = 1668)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1671)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $B9A2'
	ldx #185
	dex
	Jne *-4
	plp
.elseif(n = 1677)
	; 9 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	plp
.elseif(n = 1678)
	; 9 bytes
	php
	ldx #152 ;hides 'tya'
	delay_n 4
	dex
	Jne *-4
	plp
.endif
.else
; 1680 to 1718
.if(n = 1680)
	; 8 bytes
	php
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1680 && n < 1717)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1682 || n = 1691 || n = 1700 || n = 1718)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1683 || n = 1690 || n = 1693 || n = 1697 || n = 1710 || n = 1711)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1703 || n = 1705 || n = 1715)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1685 || n = 1692 || n = 1713)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1686)
	; 9 bytes
	php
	ldx #168 ;hides 'tay'
	ldy $A4
	dex
	Jne *-4
	plp
.elseif(n = 1687)
	; 9 bytes
	php
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	nop
	dey
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
.elseif(n = 1694)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1696 || n = 1704 || n = 1712)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1698)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $BCA2'
	ldx #188
	dex
	Jne *-4
	plp
.elseif(n = 1699)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $BCA2'
	ldx #188
	dex
	Jne *-4
	plp
.elseif(n = 1701)
	; 10 bytes
	php
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	lda $A5
	inx
	Jne *-5
	plp
.elseif(n = 1706)
	; 9 bytes
	php
	ldy #170 ;hides 'tax'
	ldx $A6
	dey
	Jne *-4
	plp
.elseif(n = 1707)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jne *-4
	plp
.elseif(n = 1708)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jne *-4
	plp
.elseif(n = 1714)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 1717)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $BEA2'
	ldx #190
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719 || n = 1731 || n = 1733 || n = 1740 || n = 1745 || n = 1747)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1720 || n = 1752)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1721 || n = 1722 || n = 1725 || n = 1729 || n = 1732 || n = 1739 || n = 1750 || n = 1753)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1752)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1724 || n = 1748)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1726)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $BFA2'
	ldx #191
	dex
	Jne *-4
	plp
.elseif(n = 1727 || n = 1736 || n = 1754)
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
.elseif(n = 1734)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C0A2'
	ldx #192
	dex
	Jne *-4
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
.elseif(n = 1738)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1741 || n = 1755)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1742)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
	plp
.elseif(n = 1743)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	dex
	Jne *-4
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
.elseif(n = 1757)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.endif
.else
; 1758 to 1796
.if((n .mod 7 = 1) && ((n > 1757 && n < 1794)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
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
.elseif((n .mod 8 = 0) && ((n > 1759 && n < 1793)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1761 || n = 1769 || n = 1773 || n = 1774 || n = 1777 || n = 1794 || n = 1795)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1762)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C3A2'
	ldx #195
	dex
	Jne *-4
	plp
.elseif(n = 1763 || n = 1781 || n = 1790)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1764)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.elseif(n = 1766)
	; 10 bytes
	php
	pha
	ldx #37 ;hides 'and zp'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 1767)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1770)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C4A2'
	ldx #196
	dex
	Jne *-4
	plp
.elseif(n = 1771)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C4A2'
	ldx #196
	dex
	Jne *-4
	plp
.elseif(n = 1775)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1778 || n = 1782 || n = 1791)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1780)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	dex
	Jne *-4
	plp
.elseif(n = 1783)
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1785)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1787)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	.repeat 4
	 nop
	.endrepeat
	Jne *-5
	plp
.elseif(n = 1788)
	; 8 bytes
	php
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1789)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C6A2'
	ldx #198
	dex
	Jne *-4
	plp
.elseif(n = 1796)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
.if((n >= 1797) && (n < 1816))
; 1797 to 1815
.if(n = 1797)
	; 8 bytes
	php
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	plp
.elseif(n = 1798)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C7A2'
	ldx #199
	dex
	Jne *-4
	plp
.elseif(n = 1799 || n = 1808)
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
.elseif(n = 1802 || n = 1805)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1803 || n = 1809)
	; 9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1806)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1807)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C8A2'
	ldx #200
	dex
	Jne *-4
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
	; 10 bytes
	php
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	lda $A5
	dex
	Jne *-5
	plp
.elseif(n = 1812)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1813)
	; 10 bytes
	php
	pha
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 1814)
	; 10 bytes
	php
	ldx #128
	delay_n 9
	dex
	Jpl *-4
	plp
.elseif(n = 1815)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C9A2'
	ldx #201
	dex
	Jne *-4
	plp
.endif
.else
; 1816 to 1835
.if(n = 1816 || n = 1832)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1817 || n = 1826 || n = 1835)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1818 || n = 1828)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1819 || n = 1822 || n = 1825)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1820)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1821)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $A5A2'
	ldx #165
	nop
	dex
	Jne *-5
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
.elseif(n = 1827 || n = 1829)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1830)
	; 10 bytes
	php
	pha
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
	pla
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
.elseif(n = 1834)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	dex
	Jne *-4
	plp
.endif
.endif
.else
; 1836 to 1875
.if(n = 1836 || n = 1845 || n = 1850 || n = 1859)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1837)
	; 10 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
	plp
.elseif(n = 1838 || n = 1858)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1839)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif((n .mod 8 = 0) && ((n > 1839 && n < 1873)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1841 || n = 1849 || n = 1855 || n = 1863 || n = 1873 || n = 1874)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1842)
	; 8 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1843)
	; 9 bytes
	php
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1843 && n < 1872)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
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
.elseif(n = 1852)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $CDA2'
	ldx #205
	dex
	Jne *-4
	plp
.elseif(n = 1854)
	; 9 bytes
	php
	ldx #168 ;hides 'tay'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 1857)
	; 10 bytes
	php
	ldx #(n-1)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 1860)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $CEA2'
	ldx #206
	dex
	Jne *-4
	plp
.elseif(n = 1861)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	dex
	Jne *-4
	plp
.elseif(n = 1865)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $A9A2'
	ldx #169
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1866)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	ldx $A6
	dey
	Jne *-4
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
.elseif(n = 1868)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1869)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $CFA2'
	ldx #207
	dex
	Jne *-4
	plp
.elseif(n = 1870)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $CFA2'
	ldx #207
	dex
	Jne *-4
	plp
.elseif(n = 1875)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
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
.if((n >= 1876) && (n < 1915))
; 1876 to 1914
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
.elseif(n = 1878)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $D0A2'
	ldx #208
	dex
	Jne *-4
	plp
.elseif(n = 1879)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D0A2'
	ldx #208
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1879 && n < 1908)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1881 || n = 1890 || n = 1891 || n = 1894)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1882)
	; 9 bytes
	php
	ldx #122 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 1883 || n = 1885 || n = 1901 || n = 1908 || n = 1913)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1884)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
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
.elseif((n .mod 8 = 0) && ((n > 1887 && n < 1913)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1892)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1893 || n = 1902)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1895)
	; 10 bytes
	php
	ldx #(n-25)/17
	jsr @nearby_rts
	dex
	Jpl *-4
	plp
.elseif(n = 1897)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D2A2'
	ldx #210
	dex
	Jne *-4
	plp
.elseif(n = 1899)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $ACA2'
	ldx #172
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1900 || n = 1911)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1903)
	; 11 bytes
	php
	pha
	ldx #(n-15)/8
	bit $24
	dex
	Jne *-3
	pla
	plp
.elseif(n = 1905)
	; 8 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1906)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D3A2'
	ldx #211
	dex
	Jne *-4
	plp
.elseif(n = 1909)
	; 9 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 1910)
	; 9 bytes
	php
	ldx #136 ;hides 'dey'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 1914)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.endif
.else
; 1915 to 1953
.if(n = 1915)
	; 9 bytes
	php
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1915 && n < 1953)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1917 || n = 1919 || n = 1926 || n = 1935 || n = 1937 || n = 1953)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 8 = 0) && ((n > 1919 && n < 1945)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1921)
	; 10 bytes
	php
	pha
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	pla
	plp
.elseif(n = 1922 || n = 1930 || n = 1945 || n = 1946)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1923)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1924)
	; 9 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
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
.elseif(n = 1929)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 1931)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $AFA2'
	ldx #175
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1932)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1933)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	dex
	Jne *-4
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
.elseif(n = 1939)
	; 10 bytes
	php
	pha
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	pla
	plp
.elseif(n = 1940)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1941)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
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
.elseif(n = 1947)
	; 10 bytes
	php
	ldx #194 ;hides 'nop #imm'
	nop $FF44 ;hides 'nop zp'
	dex
	Jne *-5
	plp
.elseif(n = 1948)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1949)
	; 10 bytes
	php
	ldx #139 ;hides 'ane #imm'
	nop $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
	plp
.elseif(n = 1950)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1951)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
.if((n >= 1954) && (n < 1973))
; 1954 to 1972
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
.elseif(n = 1955)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1956)
	; 10 bytes
	php
	ldx #106 ;hides 'ror a'
	delay_n 4
	nop
	inx
	Jne *-5
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
.elseif(n = 1958)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1959)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	dex
	Jne *-4
	plp
.elseif(n = 1960)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1961 || n = 1970)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1962 || n = 1963 || n = 1972)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1964)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1965)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	nop
	dex
	Jne *-5
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
.elseif(n = 1969)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	dex
	Jne *-4
	plp
.elseif(n = 1971)
	; 9 bytes
	pha
	delay_n (n-3)
.endif
.else
; 1973 to 1992
.if(n = 1973)
	; 10 bytes
	php
	ldx #164 ;hides 'ldy zp'
	ldy $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 1974)
	; 10 bytes
	php
	ldx #10 ;hides 'asl a'
	pla
	pha
	inx
	inx
	Jne *-5
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
.elseif(n = 1976 || n = 1984 || n = 1992)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1977)
	; 9 bytes
	php
	ldx #197 ;hides 'cmp zp'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 1978)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DBA2'
	ldx #219
	dex
	Jne *-4
	plp
.elseif(n = 1979 || n = 1988)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1980)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.elseif(n = 1981 || n = 1989 || n = 1990)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1982)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 1983)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1985)
	; 9 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
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
.elseif(n = 1991)
	; 10 bytes
	pha
	delay_n (n-3)
.endif
.endif
.else
.if((n >= 1993) && (n < 2012))
; 1993 to 2011
.if(n = 1993)
	; 11 bytes
	php
	pha
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	pla
	plp
.elseif(n = 1994 || n = 2002 || n = 2009 || n = 2011)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1995)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $DDA2'
	ldx #221
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
.elseif(n = 1997 || n = 2006)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1998)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1999)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2000 || n = 2008)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2001)
	; 10 bytes
	php
	ldy #232 ;hides 'inx'
	jsr @nearby_rts
	dey
	Jmi *-5
	plp
.elseif(n = 2003)
	; 10 bytes
	php
	ldx #104
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
	plp
.elseif(n = 2004)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	dex
	Jne *-4
	plp
.elseif(n = 2005)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
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
.elseif(n = 2010)
	; 9 bytes
	pha
	delay_n (n-3)
.endif
.else
; 2012 to 2031
.if(n = 2012)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2013)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	dex
	Jne *-4
	plp
.elseif(n = 2014)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	dex
	Jne *-4
	plp
.elseif(n = 2015)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2016)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2017 || n = 2025 || n = 2027)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2018)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2019)
	; 9 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	nop
	dey
	Jne *-4
	plp
.elseif(n = 2020)
	; 10 bytes
	php
	ldx #73 ;hides 'eor #imm'
	lda $EA ;hides 'nop'
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2021)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2022)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $E0A2'
	ldx #224
	dex
	Jne *-4
	plp
.elseif(n = 2023)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2024)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2026)
	; 9 bytes
	php
	ldy #202 ;hides 'dex'
	ldx $A6
	dey
	Jne *-4
	plp
.elseif(n = 2028)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2029)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2030)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2031)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	inx
	Jne *-4
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032 || n = 2040 || n = 2048)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2033 || n = 2042)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2034 || n = 2035 || n = 2044)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2036)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2037)
	; 10 bytes
	php
	ldy #203 ;hides 'sbx #imm'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	plp
.elseif(n = 2038)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2039)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
	plp
.elseif(n = 2041)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
	plp
.elseif(n = 2043)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2045)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2046)
	; 10 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	lda $A5
	inx
	Jne *-5
	plp
.elseif(n = 2047)
	; 11 bytes
	php
	pha
	ldx #(n-15)/8
	bit $24
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2049)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
	plp
.elseif(n = 2050)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
	plp
.endif
.else
; 2051 to 2070
.if(n = 2051 || n = 2060 || n = 2069)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2052)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	delay_n 4
	dey
	Jne *-4
	plp
.elseif(n = 2053 || n = 2062)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2054 || n = 2065)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2055)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2056)
	; 9 bytes
	php
	ldx #0
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2057)
	; 10 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	lda $A5A5 ;hides 'lda zp'
	inx
	Jne *-5
	plp
.elseif(n = 2058)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	inx
	Jne *-4
	plp
.elseif(n = 2059)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	dex
	Jne *-4
	plp
.elseif(n = 2061)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	php
	cmp $C5
	plp
	Jne *-5
	plp
.elseif(n = 2063)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $BBA2'
	ldx #187
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2064)
	; 10 bytes
	php
	ldx #69 ;hides 'eor zp'
	nop
	lda $A5
	inx
	Jne *-5
	plp
.elseif(n = 2066)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2067)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
	Jne *-4
	plp
.elseif(n = 2068)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	inx
	Jne *-4
	plp
.elseif(n = 2070)
	; 10 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	lda $A5
	inx
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
.if(n = 2071 || n = 2074)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2072 || n = 2084)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2073)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2075)
	; 10 bytes
	php
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	lda $A5
	inx
	Jne *-5
	plp
.elseif(n = 2076)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2077)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inx
	Jne *-4
	plp
.elseif(n = 2078 || n = 2087)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2079 || n = 2081 || n = 2088)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2080)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2082)
	; 10 bytes
	php
	ldx #(n-25)/17
	jsr @nearby_rts
	dex
	Jpl *-4
	plp
.elseif(n = 2083)
	; 10 bytes
	php
	pha
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 2085)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 2086)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
	plp
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
.endif
.else
; 2090 to 2109
.if(n = 2090 || n = 2097)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2091 || n = 2100)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2092)
	; 10 bytes
	php
	pha
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2093)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 2094)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2095)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	inx
	Jne *-4
	plp
.elseif(n = 2096 || n = 2105)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
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
.elseif(n = 2099)
	; 10 bytes
	php
	ldx #(n-25)/17
	jsr @nearby_rts
	dex
	Jpl *-4
	plp
.elseif(n = 2101)
	; 10 bytes
	php
	pha
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 2102)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 7
	dey
	dey
	Jne *-5
	plp
.elseif(n = 2103)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $17A2'
	ldx #23
	inx
	Jne *-4
	plp
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
.elseif(n = 2107)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2108)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2109)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2125 || n = 2143)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2111 || n = 2115 || n = 2124 || n = 2126 || n = 2133 || n = 2135 || n = 2142 || n = 2147)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2112)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2113)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 2113 && n < 2142)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2116)
	; 10 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2117)
	; 10 bytes
	php
	ldx #105 ;hides 'adc #imm'
	bit $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 2118 || n = 2128 || n = 2138)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2119)
	; 10 bytes
	php
	pha
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2120 || n = 2144)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
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
.elseif(n = 2122)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	inx
	Jne *-4
	plp
.elseif(n = 2127 || n = 2145)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2129)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2130)
	; 8 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2131)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	inx
	Jne *-4
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
.elseif(n = 2136)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2137)
	; 10 bytes
	php
	pha
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
	plp
.elseif(n = 2139)
	; 8 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2140)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	dex
	Jne *-4
	plp
.elseif(n = 2146)
	; 9 bytes
	php
	ldx #42 ;hides 'rol a'
	lda $A5
	inx
	Jne *-4
	plp
.elseif(n = 2148)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $12A2'
	ldx #18
	inx
	Jne *-4
	plp
.endif
.else
; 2149 to 2187
.if(n = 2149)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 2149 && n < 2187)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2151 || n = 2152 || n = 2160 || n = 2161 || n = 2169 || n = 2170 || n = 2179 || n = 2182)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2153 || n = 2164)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2154)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	bit $24
	inx
	inx
	Jne *-5
	plp
.elseif(n = 2155)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2156 || n = 2180)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2157)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	inx
	Jne *-4
	plp
.elseif(n = 2158)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	dex
	Jne *-4
	plp
.elseif(n = 2162)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $C4A2'
	ldx #196
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2163)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2165)
	; 10 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $A4A4 ;hides 'ldy zp'
	dex
	Jne *-5
	plp
.elseif(n = 2166)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2167)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	inx
	Jne *-4
	plp
.elseif(n = 2171 || n = 2183)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2172 || n = 2181)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2173)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $C5A2'
	ldx #197
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2174)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2175)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	dex
	Jne *-4
	plp
.elseif(n = 2176)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	inx
	Jne *-4
	plp
.elseif(n = 2178)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2184)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	plp
.elseif(n = 2185)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	inx
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
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188 || n = 2198 || n = 2218)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2189 || n = 2191 || n = 2199 || n = 2209 || n = 2215 || n = 2224)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2190)
	; 10 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	lda $A5
	dex
	Jne *-5
	plp
.elseif(n = 2192 || n = 2216)
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
.elseif(n = 2194)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 2194 && n < 2223)))
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
.elseif(n = 2200)
	; 10 bytes
	php
	pha
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2201)
	; 10 bytes
	php
	ldx #(n-25)/17
	jsr @nearby_rts
	dex
	Jpl *-4
	plp
.elseif(n = 2202)
	; 8 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 2203)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	inx
	Jne *-4
	plp
.elseif(n = 2205 || n = 2210 || n = 2214 || n = 2223)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2208 || n = 2219)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
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
.elseif(n = 2212)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	inx
	Jne *-4
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
.elseif(n = 2221)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	inx
	Jne *-4
	plp
.elseif(n = 2225)
	; 10 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 2226)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227)
	; 10 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 2228)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2229)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	inx
	Jne *-4
	plp
.elseif(n = 2230)
	; 9 bytes
	php
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 2231 || n = 2240)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2232 || n = 2233 || n = 2236)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2234)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2235)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2237)
	; 11 bytes
	php
	pha
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
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
	delay_n 4
	nop
	Jne *-4
	plp
.elseif(n = 2241)
	; 9 bytes
	pha
	delay_n (n-3)
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
	php
	ldx #160 ;hides 'ldy #imm'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2244)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2245)
	; 10 bytes
	php
	pha
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	pla
	plp
.endif
.else
; 2246 to 2265
.if(n = 2246)
	; 10 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
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
.elseif(n = 2249 || n = 2258)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
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
.elseif(n = 2251 || n = 2254 || n = 2260)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2252 || n = 2264)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2253)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
	plp
.elseif(n = 2255)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
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
.elseif(n = 2259)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2261)
	; 9 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2262)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2263)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
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
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
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
.elseif(n = 2268 || n = 2278 || n = 2298)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2269 || n = 2279 || n = 2289 || n = 2295 || n = 2299)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2270 || n = 2280 || n = 2291)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2271)
	; 10 bytes
	php
	ldy #162 ;hides 'ldx #imm'
	ldx $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	plp
.elseif(n = 2272)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2273)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
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
	; 11 bytes
	php
	pha
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
	pla
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
.elseif(n = 2288 || n = 2300)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2290 || n = 2304)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
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
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305 || n = 2309 || n = 2313 || n = 2314)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2306)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	nop $04
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
.elseif(n = 2308)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
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
.elseif(n = 2315)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2316)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2317)
	; 10 bytes
	php
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 2318)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2319)
	; 11 bytes
	php
	pha
	ldx #0
	delay_n 4
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2320)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2321)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 2322)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2323)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	bit $24
	dex
	Jne *-6
	plp
.endif
.else
; 2324 to 2343
.if(n = 2324 || n = 2336)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2325)
	; 12 bytes
	php
	pha
	ldx #210
	delay_n 4
	nop
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2326)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	bit $24
	inx
	Jne *-4
	plp
.elseif(n = 2327)
	; 9 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2328)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2329 || n = 2340)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2330)
	; 9 bytes
	php
	ldx #90 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 2331)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2332)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2333)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 4
	inx
	inx
	Jne *-5
	plp
.elseif(n = 2334)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2335)
	; 9 bytes
	php
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	plp
.elseif(n = 2337)
	; 9 bytes
	php
	ldx #(n-5)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
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
.elseif(n = 2339)
	; 10 bytes
	php
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2341)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2342)
	; 10 bytes
	php
	ldx #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 2343)
	; 10 bytes
	php
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
.if((n >= 2344) && (n < 2363))
; 2344 to 2362
.if(n = 2344)
	; 10 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 2345)
	; 10 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2346)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2347)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2348 || n = 2360)
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
.elseif(n = 2350)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2351 || n = 2362)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2352)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2354)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2355)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2356)
	; 11 bytes
	php
	pha
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2357)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 2358)
	; 9 bytes
	php
	ldx #168 ;hides 'tay'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 2359)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2361)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	nop
	dex
	Jne *-5
	plp
.endif
.else
; 2363 to 2382
.if(n = 2363 || n = 2375)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2364)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2365)
	; 11 bytes
	php
	pha
	ldx #168 ;hides 'tay'
	delay_n 7
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2366 || n = 2377)
	; 11 bytes
	php
	tsx
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-3
	txs
	plp
.elseif(n = 2367 || n = 2379)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2368 || n = 2378)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2369)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 2370)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2371)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2372)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2373)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2374)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2376)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2380)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2381)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 2382)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
	plp
.endif
.endif
.else
; 2383 to 2421
.if(n = 2383)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif((n .mod 12 = 8) && ((n > 2383 && n < 2421)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2385)
	; 10 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	plp
.elseif(n = 2386)
	; 9 bytes
	php
	ldy #170 ;hides 'tax'
	delay_n 7
	dey
	Jne *-4
	plp
.elseif(n = 2387 || n = 2389 || n = 2399 || n = 2401 || n = 2407 || n = 2411)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2388 || n = 2398 || n = 2418)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2390 || n = 2410 || n = 2419)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2391 || n = 2403)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2392)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 2393)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2394)
	; 10 bytes
	php
	ldy #200 ;hides 'iny'
	bit $24
	dey
	dey
	Jne *-5
	plp
.elseif(n = 2395 || n = 2406 || n = 2417)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2397)
	; 10 bytes
	php
	ldy #171 ;hides 'lax #imm'
	lda $4868 ;hides 'pla' and 'pha'
	dey
	Jne *-5
	plp
.elseif(n = 2400 || n = 2412)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2402 || n = 2416)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2404)
	; 9 bytes
	php
	ldx #(n-6)/11 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2405)
	; 10 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	lda $A5
	inx
	Jne *-5
	plp
.elseif(n = 2409)
	; 11 bytes
	php
	tsx
	ldy #37 ;hides 'and zp'
	iny ;first loop only
	pha
	iny
	Jne *-4
	txs
	plp
.elseif(n = 2413)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2414)
	; 11 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	nop
	lda $A5
	inx
	Jne *-6
	plp
.elseif(n = 2415)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2421)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
.if((n >= 2422) && (n < 2441))
; 2422 to 2440
.if(n = 2422 || n = 2429 || n = 2434)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423 || n = 2433 || n = 2435)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2424)
	; 10 bytes
	php
	ldy #186 ;hides 'tsx'
	delay_n 4
	nop
	dey
	Jne *-5
	plp
.elseif(n = 2425)
	; 11 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	nop
	lda $A5
	inx
	Jne *-6
	plp
.elseif(n = 2426)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	inx
	Jne *-5
	plp
.elseif(n = 2427)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	inx
	Jne *-5
	plp
.elseif(n = 2428 || n = 2439)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2430)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2431)
	; 11 bytes
	php
	tsx
	ldy #36 ;hides 'bit zp'
	nop
	pha
	iny
	Jne *-4
	txs
	plp
.elseif(n = 2432)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2436)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2437)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $DDA2'
	ldx #221
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2438)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2440)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.endif
.else
; 2441 to 2460
.if(n = 2441)
	; 10 bytes
	php
	ldx #53 ;hides 'and zp,x'
	lda $A5A5 ;hides 'lda zp'
	inx
	Jne *-5
	plp
.elseif(n = 2442)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 2443)
	; 11 bytes
	php
	tsx
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-3
	txs
	plp
.elseif(n = 2444 || n = 2456)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2445 || n = 2447)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2446)
	; 10 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	plp
.elseif(n = 2448 || n = 2458)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2449)
	; 10 bytes
	php
	pha
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	pla
	plp
.elseif(n = 2450)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2451)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2452)
	; 10 bytes
	php
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 2453)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	inx
	Jne *-5
	plp
.elseif(n = 2454)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
	plp
.elseif(n = 2455)
	; 11 bytes
	php
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	cmp $C5
	dex
	Jne *-6
	plp
.elseif(n = 2457)
	; 10 bytes
	php
	ldx #11 ;hides 'anc #imm'
	nop $FF44 ;hides 'nop zp'
	inx
	Jne *-5
	plp
.elseif(n = 2459)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2460)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
	plp
.endif
.endif
.else
; 2461 to 2500
.if((n .mod 11 = 8) && ((n > 2460 && n < 2495)))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2462)
	; 10 bytes
	php
	ldx #164 ;hides 'ldy zp'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif((n .mod 12 = 3) && ((n > 2462 && n < 2500)))
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2464)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $BDA2'
	ldx #189
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 2465 || n = 2491)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2466)
	; 9 bytes
	php
	ldx #10 ;hides 'asl a'
	lda $A5
	inx
	Jne *-4
	plp
.elseif(n = 2467)
	; 10 bytes
	php
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	pla
	pha
	dex
	Jne *-5
	plp
.elseif(n = 2468 || n = 2480 || n = 2492)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2469 || n = 2489 || n = 2495)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2470 || n = 2474 || n = 2482 || n = 2485)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2471)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2473)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2476)
	; 11 bytes
	php
	tsx
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-3
	txs
	plp
.elseif(n = 2477)
	; 10 bytes
	php
	ldx #9 ;hides 'ora #imm'
	lda $A5A5 ;hides 'lda zp'
	inx
	Jne *-5
	plp
.elseif(n = 2478 || n = 2488 || n = 2498)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2479)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	bit $24
	dex
	Jne *-6
	plp
.elseif(n = 2481)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2484)
	; 10 bytes
	php
	ldx #138 ;hides 'txa'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 2486)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2490)
	; 10 bytes
	php
	tsx
	ldy #248 ;hides 'sed'
	pha
	dey
	Jne *-3
	txs
	plp
.elseif(n = 2493)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2496)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2497)
	; 10 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	pla
	pha
	dex
	Jne *-5
	plp
.elseif(n = 2500)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
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
.if(n = 2501 || n = 2520 || n = 2531 || n = 2539)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2502 || n = 2509 || n = 2519 || n = 2521 || n = 2529 || n = 2530 || n = 2533)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2503)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2504 || n = 2516 || n = 2528)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2505 || n = 2538)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2506)
	; 9 bytes
	php
	ldx #250 ;hides 'nop'
	nop $04
	dex
	Jne *-4
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
.elseif(n = 2508 || n = 2518)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2510)
	; 10 bytes
	php
	tsx
	ldy #250 ;hides 'nop'
	pha
	dey
	Jne *-3
	txs
	plp
.elseif(n = 2511 || n = 2523)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2512)
	; 10 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	plp
.elseif(n = 2513)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 2514)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2515)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2517)
	; 9 bytes
	php
	ldx #5 ;hides 'ora zp'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2522)
	; 11 bytes
	php
	tsx
	ldy #251
	nop
	pha
	dey
	Jne *-3
	txs
	plp
.elseif(n = 2524)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2525)
	; 10 bytes
	php
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2526)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
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
.elseif(n = 2532)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2534)
	; 11 bytes
	php
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
	pla
	plp
.elseif(n = 2535)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2536)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2537)
	; 10 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	lda $A5
	dex
	Jne *-5
	plp
.endif
.else
; 2540 to 2578
.if((n .mod 12 = 8) && ((n > 2539 && n < 2577)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2541 || n = 2575)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2542 || n = 2556 || n = 2570)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2543 || n = 2555 || n = 2557 || n = 2567)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2544)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2545 || n = 2566 || n = 2572 || n = 2577 || n = 2578)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2546)
	; 11 bytes
	php
	ldx #46
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
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
.elseif(n = 2548)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2549 || n = 2560 || n = 2571)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2550)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	cmp $C5
	dex
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
.elseif(n = 2553)
	; 11 bytes
	php
	tsx
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-3
	txs
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
.elseif(n = 2559)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2561)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dex
	Jne *-5
	plp
.elseif(n = 2562)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2563)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	bit $24
	dex
	Jne *-6
	plp
.elseif(n = 2565)
	; 10 bytes
	php
	ldx #73 ;hides 'eor #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 2568)
	; 10 bytes
	php
	ldx #0
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2569)
	; 10 bytes
	php
	cmp #<$CD ;hides 'cmp $17A2'
	ldx #23
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2573)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2574)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579 || n = 2583 || n = 2585 || n = 2594 || n = 2596 || n = 2605 || n = 2616)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2580)
	; 9 bytes
	php
	ldx #(n-6)/11 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2581)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2582)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 2584 || n = 2598)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2586)
	; 11 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2587)
	; 11 bytes
	php
	pha
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2588 || n = 2600 || n = 2612)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2589)
	; 10 bytes
	php
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
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
.elseif(n = 2592 || n = 2614 || n = 2617)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2593 || n = 2604 || n = 2615)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2595 || n = 2607)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2597)
	; 11 bytes
	php
	pha
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
	plp
.elseif(n = 2599)
	; 11 bytes
	php
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	plp
.elseif(n = 2601)
	; 9 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
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
.elseif(n = 2603)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2606)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 4
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2608)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2609)
	; 9 bytes
	php
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	Jne *-4
	plp
.elseif(n = 2610)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	delay_n 7
	dey
	Jne *-4
	plp
.elseif(n = 2611)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2613)
	; 9 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.endif
.else
.if((n >= 2618) && (n < 2637))
; 2618 to 2636
.if(n = 2618)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 2619 || n = 2631)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2620)
	; 11 bytes
	php
	ldx #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 2621)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2622)
	; 10 bytes
	php
	ldx #(n-6)/12 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
	plp
.elseif(n = 2623)
	; 10 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif(n = 2624 || n = 2636)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2625)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2626)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2627)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2628)
	; 10 bytes
	php
	ldx #138 ;hides 'txa'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 2629)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2630)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2632)
	; 10 bytes
	php
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif(n = 2633)
	; 10 bytes
	php
	ldx #37 ;hides 'and zp'
	lda $EAEA ;hides two 'nop's
	inx
	Jne *-5
	plp
.elseif(n = 2634)
	; 10 bytes
	php
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2635)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.endif
.else
; 2637 to 2656
.if(n = 2637)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2638 || n = 2642 || n = 2650)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2639 || n = 2651)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2640 || n = 2654)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2641)
	; 11 bytes
	php
	ldx #101 ;hides 'adc zp'
	bit $EA ;hides 'nop'
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 2643)
	; 10 bytes
	php
	ldx #53 ;hides 'and zp,x'
	lda $EAEA ;hides two 'nop's
	inx
	Jne *-5
	plp
.elseif(n = 2644)
	; 10 bytes
	php
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2645)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	.repeat 4
	 nop
	.endrepeat
	Jne *-5
	plp
.elseif(n = 2646)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2647)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 4
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2648)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2649)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2652)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2653)
	; 11 bytes
	php
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 4
	nop
	inx
	Jne *-6
	plp
.elseif(n = 2655)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2656)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	inx
	Jne *-5
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if(n = 2657)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2658)
	; 10 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif((n .mod 11 = 8) && ((n > 2658 && n < 2693)))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2660 || n = 2672 || n = 2684)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2661 || n = 2662 || n = 2665 || n = 2674 || n = 2683 || n = 2686)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2663 || n = 2675 || n = 2693)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2664 || n = 2676)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2666)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2667)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2668 || n = 2682)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2669)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2671)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif(n = 2673)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2677)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2678)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 2679)
	; 9 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2680)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2685)
	; 11 bytes
	php
	tsx
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-3
	txs
	plp
.elseif(n = 2687)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2688)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2689)
	; 9 bytes
	php
	ldx #(n-5)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2690)
	; 9 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2691)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2694)
	; 10 bytes
	php
	ldx #168 ;hides 'tay'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 2695)
	; 11 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	cmp $C5
	dex
	Jne *-6
	plp
.endif
.else
; 2696 to 2734
.if((n .mod 12 = 8) && ((n > 2695 && n < 2733)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2697)
	; 11 bytes
	php
	pha
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2698 || n = 2702 || n = 2705 || n = 2716 || n = 2722 || n = 2733)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2699 || n = 2704 || n = 2709 || n = 2715)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2703 || n = 2714 || n = 2725)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2706)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2707)
	; 11 bytes
	php
	pha
	ldx #245 ;hides 'sbc zp,x'
	cmp $EA ;hides 'nop'
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2710 || n = 2724)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2711 || n = 2728)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
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
.elseif(n = 2713)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2717)
	; 11 bytes
	php
	ldy #21
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.elseif(n = 2718 || n = 2729)
	; 11 bytes
	php
	tsx
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-3
	txs
	plp
.elseif(n = 2719)
	; 10 bytes
	php
	ldx #(n-3)/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2721)
	; 11 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 2723)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2726)
	; 10 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 2727)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2730)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2731)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2734)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
.if((n >= 2735) && (n < 2754))
; 2735 to 2753
.if(n = 2735)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2736 || n = 2747)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2737)
	; 10 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 2738 || n = 2752)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2739 || n = 2751)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2740)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2741)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 2742)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2743)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2744)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2745)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2746)
	; 9 bytes
	php
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_15cyc
	Jne *-4
	plp
.elseif(n = 2748)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2749)
	; 10 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2750)
	; 11 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 2753)
	; 10 bytes
	php
	ldx #229 ;hides 'sbc zp'
	lda $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 10 bytes
	php
	ldx #229 ;hides 'sbc zp'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2755)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2756 || n = 2768)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2757)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2758 || n = 2769)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2759 || n = 2771)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2760)
	; 10 bytes
	php
	ldx #(n-4)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 2761)
	; 10 bytes
	php
	ldx #(n-5)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2762)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2763)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2764)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2765)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 2766)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2767)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2770)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2772)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2773)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 2774) && (n < 2793))
; 2774 to 2792
.if(n = 2774)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2775)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2776)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 2777)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2778)
	; 9 bytes
	php
	ldx #58 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 2779)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2780 || n = 2792)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2781 || n = 2783)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2782)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2784)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2785)
	; 11 bytes
	php
	pha
	ldx #58 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
	pla
	plp
.elseif(n = 2786)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2787)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2788)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2789)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2790)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	cmp $C5
	inx
	Jne *-5
	plp
.elseif(n = 2791)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.endif
.else
; 2793 to 2812
.if(n = 2793)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2794 || n = 2808)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2795 || n = 2805 || n = 2809)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2796)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2797)
	; 10 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 2798)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 2799)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2800)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2801)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2802)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2803)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2804)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2806)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 2807)
	; 10 bytes
	php
	ldx #69 ;hides 'eor zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
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
.elseif(n = 2811)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2812)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	inx
	Jne *-5
	plp
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2814)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2815 || n = 2818 || n = 2830 || n = 2833 || n = 2842 || n = 2849)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2828 || n = 2840)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2817)
	; 10 bytes
	php
	ldx #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2819 || n = 2831 || n = 2843 || n = 2851)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2820 || n = 2839)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2821)
	; 11 bytes
	php
	ldx #96
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 2822 || n = 2836 || n = 2850)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2823)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2824)
	; 10 bytes
	php
	ldx #0
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2825)
	; 10 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $A5A5 ;hides 'lda zp'
	inx
	Jne *-5
	plp
.elseif(n = 2826)
	; 10 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	nop
	lda $A5
	inx
	Jne *-5
	plp
.elseif(n = 2827)
	; 10 bytes
	php
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 2829)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2832 || n = 2844)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2834)
	; 9 bytes
	php
	ldy #202 ;hides 'dex'
	delay_n 7
	dey
	Jne *-4
	plp
.elseif(n = 2835)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2837)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	inx
	Jne *-5
	plp
.elseif(n = 2838)
	; 10 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	lda $A5
	inx
	Jne *-5
	plp
.elseif(n = 2841)
	; 11 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	nop
	lda $A5
	inx
	Jne *-6
	plp
.elseif(n = 2845)
	; 10 bytes
	php
	ldy #203 ;hides 'sbx #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	plp
.elseif(n = 2846)
	; 11 bytes
	php
	ldy #21
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 2847)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2848)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	plp
.endif
.else
; 2852 to 2890
.if((n .mod 12 = 8) && ((n > 2851 && n < 2889)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2853)
	; 11 bytes
	php
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	lda $A5
	inx
	Jne *-6
	plp
.elseif(n = 2854 || n = 2857 || n = 2860 || n = 2866 || n = 2869 || n = 2890)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2855 || n = 2865 || n = 2867 || n = 2879 || n = 2884 || n = 2889)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2856)
	; 10 bytes
	php
	ldx #106 ;hides 'ror a'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 2858 || n = 2877)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2859 || n = 2871)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2861)
	; 11 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	inx
	Jne *-6
	plp
.elseif(n = 2862)
	; 10 bytes
	php
	ldx #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 2863)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2868)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2870)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	plp
.elseif(n = 2872)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 2873)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 2874)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2875)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	bit $24
	dex
	Jne *-6
	plp
.elseif(n = 2878)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2880)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2881)
	; 10 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2882)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2883)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	plp
.elseif(n = 2885)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 2886)
	; 10 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	pla
	pha
	dex
	Jne *-5
	plp
.elseif(n = 2887)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	cmp $C5
	inx
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891 || n = 2901 || n = 2903 || n = 2909 || n = 2921 || n = 2927)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2892 || n = 2906 || n = 2920)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2893)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 2894)
	; 10 bytes
	php
	ldx #152 ;hides 'tya'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 2895 || n = 2907 || n = 2919)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2896)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2897)
	; 10 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
	plp
.elseif(n = 2898 || n = 2915)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2899)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2900 || n = 2912 || n = 2924)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2902 || n = 2914 || n = 2917 || n = 2926 || n = 2929)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2904)
	; 10 bytes
	php
	ldx #138 ;hides 'txa'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 2905)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 2908 || n = 2928)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2910)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2911)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2913)
	; 10 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	ldx $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	plp
.elseif(n = 2916)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2918)
	; 10 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	pla
	pha
	dex
	Jne *-5
	plp
.elseif(n = 2922)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	dex
	Jne *-5
	plp
.elseif(n = 2923)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2925)
	; 11 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.endif
.else
; 2930 to 2968
.if(n = 2930 || n = 2938 || n = 2941 || n = 2950 || n = 2965)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 3) && ((n > 2930 && n < 2968)))
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2932)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 2933)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	dex
	Jne *-5
	plp
.elseif(n = 2934 || n = 2962)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2935)
	; 11 bytes
	php
	ldx #73 ;hides 'eor #imm'
	lda $EA ;hides 'nop'
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 2936 || n = 2948 || n = 2960)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2937)
	; 11 bytes
	php
	ldx #100
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 2939 || n = 2951 || n = 2961 || n = 2963)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2940 || n = 2952 || n = 2964)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2942)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2944)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 2945)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dex
	Jne *-5
	plp
.elseif(n = 2946)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	nop
	lda $A5
	dex
	Jne *-5
	plp
.elseif(n = 2947)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 2949 || n = 2966)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2953)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2954)
	; 10 bytes
	php
	ldx #122 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 2956)
	; 11 bytes
	php
	ldx #67
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 2957)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2958)
	; 10 bytes
	php
	ldx #10 ;hides 'asl a'
	nop
	cmp $C5
	inx
	Jne *-5
	plp
.elseif(n = 2959)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2968)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if(n = 2969)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 2970)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	cmp $C5
	dex
	Jne *-6
	plp
.elseif(n = 2971)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 4
	bit $24
	dex
	Jne *-6
	plp
.elseif(n = 2972 || n = 2984 || n = 2996)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2973)
	; 11 bytes
	php
	ldx #(n-5)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dex
	Jne *-6
	plp
.elseif(n = 2974 || n = 2986 || n = 2989 || n = 2992 || n = 2998 || n = 3001)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2975 || n = 2980 || n = 2987 || n = 2999 || n = 3005)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2976 || n = 2990 || n = 3004)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2977)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 2978 || n = 2993)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 2979 || n = 3003)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2981)
	; 12 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2982)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2983 || n = 3000)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2985)
	; 10 bytes
	php
	ldx #43 ;hides 'anc #imm'
	cmp $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 2988)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2991)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2994)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2995)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2997)
	; 11 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	nop
	bit $24
	dex
	Jne *-6
	plp
.elseif(n = 3002)
	; 9 bytes
	php
	ldx #42 ;hides 'rol a'
	pla
	pha
	inx
	Jne *-4
	plp
.elseif(n = 3006)
	; 10 bytes
	php
	ldx #(n-6)/12 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
	plp
.elseif(n = 3007)
	; 10 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.endif
.else
; 3008 to 3046
.if((n .mod 12 = 8) && ((n > 3007 && n < 3045)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 3009)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
	plp
.elseif(n = 3010 || n = 3021 || n = 3022 || n = 3025 || n = 3034 || n = 3040 || n = 3041)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3011 || n = 3023 || n = 3033 || n = 3035 || n = 3045)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3012 || n = 3024 || n = 3036)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3013)
	; 10 bytes
	php
	ldx #41 ;hides 'and #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 3014)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $BCA2'
	ldx #188
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3015 || n = 3027 || n = 3039)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 3016)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3017)
	; 10 bytes
	php
	ldx #5 ;hides 'ora zp'
	lda $EAEA ;hides two 'nop's
	inx
	Jne *-5
	plp
.elseif(n = 3018 || n = 3046)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3019)
	; 10 bytes
	php
	ldx #5 ;hides 'ora zp'
	delay_n 4
	nop
	inx
	Jne *-5
	plp
.elseif(n = 3026)
	; 11 bytes
	php
	tsx
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-4
	txs
	plp
.elseif(n = 3028)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3029)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
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
.elseif(n = 3031)
	; 10 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif(n = 3037)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3038)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3042)
	; 10 bytes
	php
	ldx #138 ;hides 'txa'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 3043)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	inx
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
.if(n = 3047)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3048)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 3049)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3050)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3051)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	php
	cmp $C5
	plp
	Jne *-5
	plp
.elseif(n = 3052)
	; 11 bytes
	php
	tsx
	ldy #(n-10)/13 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	txs
	plp
.elseif(n = 3053)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3054)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3055)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3056)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 3057 || n = 3062)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3058)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3059)
	; 10 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $EAEA ;hides two 'nop's
	inx
	Jne *-5
	plp
.elseif(n = 3060)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3061)
	; 10 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	delay_n 4
	nop
	inx
	Jne *-5
	plp
.elseif(n = 3063)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 3064)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3065)
	; 11 bytes
	php
	clc
	lda #$4A ;hides 'lsr a'
	ldx #202 ;hides 'dex'
	Jne *-1
	Jcc *-5
	plp
.endif
.else
; 3066 to 3085
.if(n = 3066)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3067)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3068)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 3069 || n = 3070 || n = 3082)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3071 || n = 3077 || n = 3083)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3072)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	inx
	Jne *-5
	plp
.elseif(n = 3073)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
	plp
.elseif(n = 3074)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3075)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 3076)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3078)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	dex
	Jne *-6
	plp
.elseif(n = 3079)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3080)
	; 9 bytes
	php
	ldx #0
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 3081)
	; 11 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	nop
	inx
	Jne *-6
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
.elseif(n = 3085)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.endif
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3105 || n = 3124)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3087)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif(n = 3088 || n = 3102 || n = 3116)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3089 || n = 3115)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3090 || n = 3103 || n = 3104 || n = 3117 || n = 3118 || n = 3121)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3091 || n = 3122)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3092 || n = 3106)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3093)
	; 12 bytes
	php
	ldx #136 ;hides 'dey'
	ldy #66
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 3094)
	; 11 bytes
	php
	ldy #248 ;hides 'sed'
	ldx #202 ;hides 'dex'
	Jmi *-1
	iny
	Jne *-6
	plp
.elseif(n = 3095)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3096 || n = 3112)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3097)
	; 11 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3098 || n = 3113)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3099)
	; 12 bytes
	php
	ldx #(n-289)/281
	lda #249
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3100)
	; 11 bytes
	php
	ldx #74 ;hides 'lsr a'
	rol $26
	ror $26
	inx
	Jne *-6
	plp
.elseif(n = 3101)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 4
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3107)
	; 11 bytes
	php
	ldx #101 ;hides 'adc zp'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 3108)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3109 || n = 3123)
	; 12 bytes
	php
	pha
	ldx #(n-15)/14
	nop
	pla
	pha
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3110)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $C2A2'
	ldx #194
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3111)
	; 11 bytes
	php
	ldx #106
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3114)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 3119)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3120)
	; 11 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
	plp
.elseif(n = 3125)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
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
	php
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3127)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3128 || n = 3148)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3129)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $DFA2'
	ldx #223
	nop
	cmp $C5
	dex
	Jne *-7
	plp
.elseif(n = 3130 || n = 3144 || n = 3158)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3131 || n = 3133 || n = 3151 || n = 3161 || n = 3163)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3132 || n = 3138 || n = 3145 || n = 3146 || n = 3149 || n = 3155 || n = 3164)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3134)
	; 11 bytes
	php
	ldx #184 ;hides 'clv'
	rol $26
	ror $26
	dex
	Jne *-6
	plp
.elseif(n = 3135)
	; 12 bytes
	php
	ldx #58
	lda #21
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3136 || n = 3153)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3137)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3139)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3140)
	; 11 bytes
	php
	ldx #107
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3141 || n = 3154)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3142)
	; 11 bytes
	php
	ldx #56 ;hides 'sec'
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-6
	plp
.elseif(n = 3143 || n = 3162)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3147)
	; 11 bytes
	php
	ldx #100 ;hides 'nop zp'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 3150)
	; 11 bytes
	php
	ldy #24 ;hides 'clc'
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-6
	plp
.elseif(n = 3152)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3156)
	; 10 bytes
	php
	ldx #106 ;hides 'ror a'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 3157)
	; 11 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
	plp
.elseif(n = 3159)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3160)
	; 10 bytes
	php
	ldx #90 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.endif
.else
.if((n >= 3165) && (n < 3184))
; 3165 to 3183
.if(n = 3165)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 3166)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3167)
	; 10 bytes
	php
	ldx #(n-3)/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3168)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3169)
	; 11 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	nop
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 3170)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3171)
	; 11 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3172)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3173)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3174)
	; 10 bytes
	php
	ldx #58 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3175)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $C6A2'
	ldx #198
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3176)
	; 10 bytes
	php
	ldx #(n-4)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 3177)
	; 10 bytes
	php
	ldx #(n-5)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 3178)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 3179)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3180)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3181)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3182 || n = 3183)
	; 11..12 bytes
	delay_n (n-2)
	nop
.endif
.else
; 3184 to 3203
.if(n = 3184)
	; 11 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3185)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 4
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3186 || n = 3200)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3187)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3188)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3189)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	bit $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 3190)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 3191)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 3192)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3193)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3194)
	; 11 bytes
	php
	ldx #58
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3195 || n = 3202)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3196)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #111
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 3197)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	cmp $EAEA ;hides two 'nop's
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3198)
	; 10 bytes
	php
	ldx #168 ;hides 'tay'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 3199)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3201)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3203)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
.if((n >= 3204) && (n < 3223))
; 3204 to 3222
.if(n = 3204)
	; 10 bytes
	php
	ldx #10 ;hides 'asl a'
	delay_n 4
	nop
	inx
	Jne *-5
	plp
.elseif(n = 3205)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3206)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3207)
	; 11 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jmi *-6
	plp
.elseif(n = 3208)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3209)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3210 || n = 3215 || n = 3216)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3211)
	; 11 bytes
	php
	ldx #229 ;hides 'sbc zp'
	lda $EAEA ;hides two 'nop's
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3212)
	; 11 bytes
	php
	ldx #229 ;hides 'sbc zp'
	bit $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 3213)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 4
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3214)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3217)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3218)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3219)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3220)
	; 11 bytes
	php
	ldx #72
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3221)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3222)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $C9A2'
	ldx #201
	delay_n 7
	dex
	Jne *-6
	plp
.endif
.else
; 3223 to 3242
.if(n = 3223)
	; 11 bytes
	php
	ldx #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3224 || n = 3240)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3225 || n = 3235)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3226)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 3227)
	; 11 bytes
	php
	ldx #110
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3228 || n = 3242)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3229)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3230)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 3231)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3232)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3233)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3234)
	; 11 bytes
	php
	tsx
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-4
	txs
	plp
.elseif(n = 3236)
	; 10 bytes
	php
	ldy #170 ;hides 'tax'
	jsr @nearby_rts
	dey
	Jne *-5
	plp
.elseif(n = 3237)
	; 12 bytes
	php
	pha
	ldx #248 ;hides 'sed'
	delay_n 4
	nop
	dex
	Jne *-5
	pla
	plp
.elseif(n = 3238)
	; 10 bytes
	php
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 3239)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3241)
	; 11 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
	plp
.endif
.endif
.else
; 3243 to 3281
.if(n = 3243)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3244 || n = 3247 || n = 3250 || n = 3251 || n = 3267 || n = 3274 || n = 3279)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3245 || n = 3271)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3246)
	; 11 bytes
	php
	ldx #216 ;hides 'cld'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3248 || n = 3268)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3249)
	; 10 bytes
	php
	ldx #53 ;hides 'and zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 3252 || n = 3273 || n = 3275 || n = 3280)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3253)
	; 12 bytes
	php
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	nop
	lda $A5
	inx
	Jne *-7
	plp
.elseif(n = 3254)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 3255 || n = 3272)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3256 || n = 3270)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3257 || n = 3276)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.elseif(n = 3259)
	; 11 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3260)
	; 11 bytes
	php
	tsx
	ldy #(n-10)/13 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	txs
	plp
.elseif(n = 3261)
	; 11 bytes
	php
	pha
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
	pla
	plp
.elseif(n = 3262)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #10
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 3263 || n = 3278)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3264)
	; 11 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	delay_n 4
	pla
	pha
	dex
	Jne *-6
	plp
.elseif(n = 3265)
	; 10 bytes
	php
	ldx #233 ;hides 'sbc #imm'
	bit $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
	plp
.elseif(n = 3266)
	; 12 bytes
	php
	ldx #(n-8)/18
	nop
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 3269)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3277)
	; 10 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 3281)
	; 11 bytes
	php
	ldy #24 ;hides 'clc'
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3283)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3284 || n = 3298 || n = 3312)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3285 || n = 3301 || n = 3311 || n = 3315 || n = 3317)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3286)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3287)
	; 10 bytes
	php
	ldx #37 ;hides 'and zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 3288 || n = 3308)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3289 || n = 3306)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3290 || n = 3291 || n = 3294 || n = 3299 || n = 3300 || n = 3316)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3292)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3293)
	; 10 bytes
	php
	ldx #235 ;hides 'sbc #imm'
	lda $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
	plp
.elseif(n = 3295 || n = 3314)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3296)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3297 || n = 3310)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3302)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 3303)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3304 || n = 3320)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3305)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3307)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3309)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3313)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 3318)
	; 11 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 3319)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $CFA2'
	ldx #207
	delay_n 7
	dex
	Jne *-6
	plp
.endif
.else
; 3321 to 3359
.if(n = 3321)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3322)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3323 || n = 3357)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3324 || n = 3325 || n = 3337 || n = 3341 || n = 3342 || n = 3345 || n = 3356 || n = 3359)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3326 || n = 3340 || n = 3354)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3327)
	; 11 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 3328 || n = 3348)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3329 || n = 3331)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3330)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 3332)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3333 || n = 3352)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3334)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 3335)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3336)
	; 11 bytes
	php
	ldx #0
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3338 || n = 3353)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3339)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3343)
	; 11 bytes
	php
	ldx #114
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3344 || n = 3358)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3346)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 3347)
	; 11 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	plp
.elseif(n = 3349)
	; 12 bytes
	php
	ldx #(n-265)/257
	lda #225
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3350)
	; 10 bytes
	php
	ldx #152 ;hides 'tya'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 3351)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $D1A2'
	ldx #209
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3355)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
.if((n >= 3360) && (n < 3379))
; 3360 to 3378
.if(n = 3360 || n = 3377)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3361 || n = 3375)
	; 12 bytes
	php
	pha
	ldx #(n-15)/14
	nop
	pla
	pha
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3362)
	; 11 bytes
	php
	ldy #26
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.elseif(n = 3363)
	; 12 bytes
	php
	ldx #60
	lda #23
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3364)
	; 11 bytes
	php
	ldx #200 ;hides 'iny'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jmi *-6
	plp
.elseif(n = 3365)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $92A2'
	ldx #146
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 3366)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $D2A2'
	ldx #210
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3367)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $D2A2'
	ldx #210
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3368)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3369)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #25
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 3370 || n = 3376)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3371)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3372)
	; 11 bytes
	php
	ldx #115
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3373)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3374)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3378)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.endif
.else
; 3379 to 3398
.if(n = 3379 || n = 3392)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3380)
	; 12 bytes
	php
	ldx #(n-8)/281
	lda #249
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 3381)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #101
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 3382 || n = 3396)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3383)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3384)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3385 || n = 3394)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3386)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3387)
	; 12 bytes
	php
	ldx #(n-39)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 3388)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3389)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3390)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3391)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3393)
	; 10 bytes
	php
	ldx #(n-1)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3395)
	; 11 bytes
	php
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 3397)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 4
	nop
	inx
	Jne *-7
	plp
.elseif(n = 3398)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 3399) && (n < 3418))
; 3399 to 3417
.if(n = 3399)
	; 11 bytes
	php
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3400 || n = 3416)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3401)
	; 11 bytes
	php
	ldx #117
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
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
	; 12 bytes
	php
	pha
	ldx #(n-15)/14
	nop
	pla
	pha
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3404)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #111
	dex
	Jpl *-1
	Jcc *-6
	plp
.elseif(n = 3405)
	; 12 bytes
	php
	pha
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	pla
	plp
.elseif(n = 3406)
	; 11 bytes
	php
	ldx #56 ;hides 'sec'
	rol $26
	ror $26
	inx
	Jne *-6
	plp
.elseif(n = 3407)
	; 12 bytes
	php
	ldy #102
	lda #1
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	plp
.elseif(n = 3408)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3409)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3410)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3411)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3412)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3413)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3414)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3415)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3417)
	; 11 bytes
	php
	ldx #101 ;hides 'adc zp'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.endif
.else
; 3418 to 3437
.if(n = 3418)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 3419)
	; 12 bytes
	php
	pha
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	pla
	plp
.elseif(n = 3420)
	; 12 bytes
	php
	ldx #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	cmp $C5
	dex
	Jne *-7
	plp
.elseif(n = 3421)
	; 11 bytes
	php
	ldx #(n-5)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dex
	Jne *-6
	plp
.elseif(n = 3422)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3423)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	nop
	cmp $C5
	inx
	Jne *-7
	plp
.elseif(n = 3424)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3425)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3426 || n = 3429)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3427)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3428)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3430)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3431)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3432)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3433)
	; 10 bytes
	php
	ldx #11 ;hides 'anc #imm'
	cmp $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 3434)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3435)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	cmp $EA ;hides 'nop'
	lda $A5
	dex
	Jne *-6
	plp
.elseif(n = 3436)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	delay_n 4
	bit $24
	dex
	Jne *-6
	plp
.elseif(n = 3437)
	; 10 bytes
	php
	ldx #229 ;hides 'sbc zp'
	cmp $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
	plp
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
.if(n = 3438 || n = 3452 || n = 3466)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3439 || n = 3440 || n = 3444 || n = 3449 || n = 3454 || n = 3470 || n = 3475)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3441 || n = 3445 || n = 3451 || n = 3453 || n = 3455 || n = 3460 || n = 3465 || n = 3467 || n = 3469)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3442)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3443 || n = 3458 || n = 3473)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3446)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3447)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3448 || n = 3468)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
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
.elseif(n = 3456)
	; 11 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 3457)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	plp
.elseif(n = 3459 || n = 3476)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3461)
	; 10 bytes
	php
	ldx #9 ;hides 'ora #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 3462)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3463)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3464)
	; 10 bytes
	php
	ldx #74 ;hides 'lsr a'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 3471)
	; 11 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 4
	nop
	dex
	dex
	Jne *-6
	plp
.elseif(n = 3472)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3474)
	; 12 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	.repeat 4
	 nop
	.endrepeat
	inx
	Jne *-7
	plp
.endif
.else
.if((n >= 3477) && (n < 3496))
; 3477 to 3495
.if(n = 3477)
	; 12 bytes
	php
	ldx #216 ;hides 'cld'
	lda #5
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 3478)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3479)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3480 || n = 3494)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3481 || n = 3483)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3484)
	; 11 bytes
	php
	ldx #78
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3485)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3486)
	; 11 bytes
	php
	ldx #24 ;hides 'clc'
	.repeat 4
	 nop
	.endrepeat
	inx
	Jne *-6
	plp
.elseif(n = 3487 || n = 3490)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3488)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3489)
	; 12 bytes
	php
	ldx #58
	lda #27
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3491)
	; 11 bytes
	php
	ldy #27
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.elseif(n = 3492)
	; 10 bytes
	php
	ldx #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 3493)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3495)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	delay_n 7
	dex
	Jne *-6
	plp
.endif
.else
; 3496 to 3515
.if(n = 3496 || n = 3512)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3497 || n = 3505 || n = 3509)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3498)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3499)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $C2A2'
	ldx #194
	delay_n 9
	dex
	Jne *-7
	plp
.elseif(n = 3500)
	; 12 bytes
	php
	ldx #(n-8)/18
	nop
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 3501)
	; 12 bytes
	php
	pha
	ldx #(n-15)/14
	nop
	pla
	pha
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3502)
	; 10 bytes
	php
	ldx #184 ;hides 'clv'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 3503)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3504)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3506)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3507)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	delay_n 4
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3508)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3510)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3511)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $DBA2'
	ldx #219
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3513)
	; 11 bytes
	php
	pha
	ldx #250 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3514)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3515)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
.if((n >= 3516) && (n < 3535))
; 3516 to 3534
.if(n = 3516)
	; 11 bytes
	php
	ldx #234 ;hides 'nop'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3517)
	; 11 bytes
	php
	ldx #120
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3518 || n = 3533)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3519)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	lda $EAEA ;hides two 'nop's
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3520)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	lda $EA ;hides 'nop'
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3521)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	.repeat 4
	 nop
	.endrepeat
	inx
	Jne *-6
	plp
.elseif(n = 3522)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3523)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3524 || n = 3525)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3526)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $DCA2'
	ldx #220
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3527)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3528)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3529)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $EAEA ;hides two 'nop's
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3530)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3531)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	.repeat 4
	 nop
	.endrepeat
	inx
	Jne *-6
	plp
.elseif(n = 3532)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3534)
	; 10 bytes
	php
	ldx #168 ;hides 'tay'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.endif
.else
; 3535 to 3554
.if(n = 3535)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	delay_n 4
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3536 || n = 3550)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3537 || n = 3539 || n = 3547 || n = 3551 || n = 3553)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3538 || n = 3552)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3540)
	; 10 bytes
	php
	ldy #186 ;hides 'tsx'
	jsr @nearby_rts
	dey
	Jne *-5
	plp
.elseif(n = 3541)
	; 12 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	delay_n 4
	pla
	pha
	inx
	Jne *-7
	plp
.elseif(n = 3542)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3543)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $DDA2'
	ldx #221
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3544)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3545)
	; 11 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3546)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	delay_n 4
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3548)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3549)
	; 11 bytes
	php
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
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
.endif
.else
.if((n >= 3555) && (n < 3574))
; 3555 to 3573
.if(n = 3555)
	; 12 bytes
	php
	ldx #218 ;hides 'nop'
	lda #5
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 3556)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #108
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 3557)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3558)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3559)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3560)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3561)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3562 || n = 3565 || n = 3566 || n = 3569 || n = 3570)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3563)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3564)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3567 || n = 3571)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3568)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3572)
	; 11 bytes
	php
	ldx #81
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 3573)
	; 12 bytes
	php
	ldx #(n-39)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-6
	plp
.endif
.else
; 3574 to 3593
.if(n = 3574)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3575)
	; 11 bytes
	php
	ldx #122
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3576)
	; 10 bytes
	php
	ldy #170 ;hides 'tax'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	plp
.elseif(n = 3577)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	nop
	cmp $C5
	inx
	Jne *-7
	plp
.elseif(n = 3578)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3579 || n = 3581 || n = 3583)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3580)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3582)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3584)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #118
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3585)
	; 12 bytes
	php
	pha
	ldx #(n-15)/14
	nop
	pla
	pha
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3586)
	; 12 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
	plp
.elseif(n = 3587)
	; 11 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3588)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3589)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $C7A2'
	ldx #199
	delay_n 9
	dex
	Jne *-7
	plp
.elseif(n = 3590)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $E0A2'
	ldx #224
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3591)
	; 11 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3592)
	; 10 bytes
	php
	ldx #0
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3593)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3597 || n = 3600 || n = 3601 || n = 3610 || n = 3614 || n = 3625 || n = 3630 || n = 3631)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3612 || n = 3629)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.elseif(n = 3598 || n = 3602 || n = 3611 || n = 3621 || n = 3632)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3599 || n = 3618)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
	; 11 bytes
	php
	ldx #124
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 3605)
	; 12 bytes
	php
	ldx #108
	lda #1
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3606)
	; 11 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-6
	plp
.elseif(n = 3607)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $1FA2'
	ldx #31
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3608 || n = 3628)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3609)
	; 12 bytes
	php
	ldx #12
	lda #245
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3613)
	; 12 bytes
	php
	ldx #102
	lda #3
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3615)
	; 11 bytes
	php
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 3616)
	; 11 bytes
	php
	ldx #81
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3617)
	; 12 bytes
	php
	ldx #(n-409)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 3619)
	; 11 bytes
	php
	ldx #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3620)
	; 11 bytes
	php
	ldy #27
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 3622)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3623)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3624)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3626)
	; 11 bytes
	php
	ldx #67
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 3627)
	; 12 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	.repeat 5
	 nop
	.endrepeat
	dex
	Jne *-7
	plp
.endif
.else
.if((n >= 3633) && (n < 3652))
; 3633 to 3651
.if(n = 3633)
	; 11 bytes
	php
	ldx #125
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 3634)
	; 12 bytes
	php
	ldx #97
	lda #5
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3635)
	; 12 bytes
	php
	ldx #93
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 3636)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #131
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 3637)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3638)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3639)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3640)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3641)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3642)
	; 11 bytes
	php
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	nop
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 3643)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	delay_n 9
	dex
	Jne *-7
	plp
.elseif(n = 3644)
	; 11 bytes
	php
	ldx #42 ;hides 'rol a'
	rol $26
	ror $26
	inx
	Jne *-6
	plp
.elseif(n = 3645)
	; 12 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
	plp
.elseif(n = 3646)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3647 || n = 3649)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3648)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3650)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3651)
	; 11 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	nop
	dex
	Jne *-6
	plp
.endif
.else
; 3652 to 3671
.if(n = 3652)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3653)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3654)
	; 11 bytes
	php
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3655)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3656)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3657)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 3658)
	; 10 bytes
	php
	ldx #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 3659)
	; 11 bytes
	php
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 3660)
	; 11 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	delay_n 11
	Jne *-6
	plp
.elseif(n = 3661)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3662)
	; 11 bytes
	php
	ldx #125
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3663)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3664)
	; 11 bytes
	php
	ldx #138 ;hides 'txa'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-6
	plp
.elseif(n = 3665)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3666)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	delay_n 4
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3667)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	delay_n 4
	nop
	inx
	Jne *-7
	plp
.elseif(n = 3668)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3669)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #64
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 3670)
	; 11 bytes
	php
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3671)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	delay_n 7
	inx
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
.if((n >= 3672) && (n < 3691))
; 3672 to 3690
.if(n = 3672)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3673)
	; 11 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3674 || n = 3677 || n = 3682 || n = 3685 || n = 3690)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3675)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3676)
	; 12 bytes
	php
	ldx #(n-139)/131
	lda #99
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3678)
	; 11 bytes
	php
	ldx #216 ;hides 'cld'
	rol $26
	ror $26
	dex
	Jne *-6
	plp
.elseif(n = 3679)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	bit $EAEA ;hides two 'nop's
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3680)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3681)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3683)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3684)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3686)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3687)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3688)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3689)
	; 11 bytes
	pha
	delay_n (n-3)
.endif
.else
; 3691 to 3710
.if(n = 3691)
	; 11 bytes
	php
	ldx #127
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 3692)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3693 || n = 3699 || n = 3710)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3694)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3695)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3696)
	; 11 bytes
	php
	ldx #10 ;hides 'asl a'
	.repeat 4
	 nop
	.endrepeat
	inx
	Jne *-6
	plp
.elseif(n = 3697)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3698)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3700)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3701)
	; 12 bytes
	php
	clc
	lda #74 ;hides 'lsr a'
	ldx #(n-26)/15
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3702)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	plp
.elseif(n = 3703)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3704)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3705)
	; 11 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	plp
.elseif(n = 3706)
	; 12 bytes
	php
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
	plp
.elseif(n = 3707)
	; 12 bytes
	php
	ldx #26
	lda #105
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3708)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3709)
	; 12 bytes
	php
	pha
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	pla
	plp
.endif
.endif
.else
.if((n >= 3711) && (n < 3731))
; 3711 to 3730
.if(n = 3711 || n = 3717 || n = 3721)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3712)
	; 11 bytes
	php
	ldx #(n-6)/17 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
	plp
.elseif(n = 3713)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3714)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3715 || n = 3716)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3718)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3719)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3720)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3722)
	; 12 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3723)
	; 12 bytes
	php
	ldx #58 ;hides 'nop'
	lda #29
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 3724)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #135
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 3725)
	; 11 bytes
	php
	ldx #37 ;hides 'and zp'
	lda $4868 ;hides 'pla' and 'pha'
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3726)
	; 11 bytes
	php
	ldx #248 ;hides 'sed'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3727)
	; 11 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 3728)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3729)
	; 11 bytes
	php
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 3730)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 9
	inx
	Jne *-6
	plp
.endif
.else
; 3731 to 3750
.if(n = 3731 || n = 3748)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3732)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3733 || n = 3737 || n = 3738 || n = 3745)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3734)
	; 11 bytes
	php
	ldx #68
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3735)
	; 11 bytes
	php
	ldx #233 ;hides 'sbc #imm'
	lda $EA ;hides 'nop'
	pla
	pha
	dex
	Jne *-6
	plp
.elseif(n = 3736)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3739)
	; 12 bytes
	php
	ldx #(n-49)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jpl *-6
	plp
.elseif(n = 3740)
	; 12 bytes
	php
	ldx #(n-8)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
	plp
.elseif(n = 3741)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	nop
	inx
	Jne *-7
	plp
.elseif(n = 3742)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3743)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3744)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #55
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 3746)
	; 10 bytes
	php
	ldy #170 ;hides 'tax'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	plp
.elseif(n = 3747)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 11
	inx
	Jne *-6
	plp
.elseif(n = 3749)
	; 11 bytes
	php
	ldx #128
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jpl *-5
	plp
.elseif(n = 3750)
	; 10 bytes
	php
	ldx #(n-6)/16 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
	plp
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
.if(n = 3751 || n = 3770 || n = 3789)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3752 || n = 3784)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3753 || n = 3759 || n = 3763 || n = 3769 || n = 3774)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3754 || n = 3760 || n = 3771 || n = 3775 || n = 3780 || n = 3785)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3755)
	; 12 bytes
	php
	pha
	ldx #(n-15)/17
	jsr @nearby_rts
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3756)
	; 11 bytes
	php
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 3757)
	; 10 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 3758 || n = 3773)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3761)
	; 10 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 3762)
	; 12 bytes
	php
	tsx
	ldy #(n-12)/15
	delay_n 10
	dey
	Jne *-4
	txs
	plp
.elseif(n = 3764)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
	plp
.elseif(n = 3765)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3766)
	; 10 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	nop
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 3767)
	; 10 bytes
	php
	ldx #5 ;hides 'ora zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	plp
.elseif(n = 3768 || n = 3788)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3772)
	; 10 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 3776)
	; 12 bytes
	php
	clc
	lda #74 ;hides 'lsr a'
	ldx #(n-26)/15
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3777)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 3778)
	; 11 bytes
	php
	ldx #130
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 3779)
	; 11 bytes
	php
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 3781)
	; 12 bytes
	php
	ldx #76
	lda #17
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3782)
	; 10 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 3783)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3786)
	; 11 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 3787)
	; 10 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
	plp
.endif
.else
.if((n >= 3790) && (n < 3809))
; 3790 to 3808
.if(n = 3790 || n = 3791 || n = 3801 || n = 3805)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3792)
	; 11 bytes
	php
	ldx #86
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 3793)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3794)
	; 12 bytes
	php
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
	pla
	plp
.elseif(n = 3795)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #106
	dex
	Jpl *-1
	Jcs *-7
	plp
.elseif(n = 3796)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 3797)
	; 12 bytes
	php
	ldx #(n-429)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 3798)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3799)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3800)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3802)
	; 11 bytes
	php
	ldx #165 ;hides 'lda zp'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 3803)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3804)
	; 11 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3806)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 3807)
	; 11 bytes
	php
	ldx #131
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 3808)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.endif
.else
; 3809 to 3828
.if(n = 3809)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3810 || n = 3820)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3811)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3812)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3813)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #107
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 3814)
	; 11 bytes
	php
	cmp #<$CD ;hides 'cmp $12A2'
	ldx #18
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3815)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3816)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3817)
	; 11 bytes
	php
	ldx #(n-1)/18 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3818)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3819)
	; 11 bytes
	php
	ldx #250 ;hides 'nop'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jmi *-6
	plp
.elseif(n = 3821)
	; 11 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3822)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3823)
	; 12 bytes
	php
	ldx #108
	lda #3
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3824)
	; 11 bytes
	php
	ldx #90 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 3825)
	; 11 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 3826)
	; 12 bytes
	php
	ldx #(n-8)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jne *-6
	plp
.elseif(n = 3827)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3828)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
.if((n >= 3829) && (n < 3848))
; 3829 to 3847
.if(n = 3829 || n = 3834 || n = 3838 || n = 3841)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3830)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3831)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3832)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3833)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3835)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
	plp
.elseif(n = 3836)
	; 11 bytes
	php
	ldx #87
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 3837)
	; 12 bytes
	php
	tsx
	ldy #(n-12)/15
	delay_n 10
	dey
	Jne *-4
	txs
	plp
.elseif(n = 3839)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3840)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 3842)
	; 11 bytes
	php
	ldx #71
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 3843)
	; 12 bytes
	php
	ldx #65
	lda #27
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 3844)
	; 10 bytes
	php
	ldy #202 ;hides 'dex'
	jsr @nearby_rts
	dey
	Jne *-5
	plp
.elseif(n = 3845)
	; 12 bytes
	php
	ldx #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dex
	Jne *-7
	plp
.elseif(n = 3846)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3847)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	delay_n 7
	inx
	Jne *-6
	plp
.endif
.else
; 3848 to 3867
.if(n = 3848)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3849)
	; 11 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	plp
.elseif(n = 3850 || n = 3867)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3851)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3852)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3853 || n = 3861 || n = 3866)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3854)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #127
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3855)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3856)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 3857)
	; 12 bytes
	php
	pha
	ldx #(n-15)/17
	jsr @nearby_rts
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3858)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3859)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	delay_n 9
	dex
	Jne *-7
	plp
.elseif(n = 3860)
	; 12 bytes
	php
	ldx #(n-8)/18
	nop
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 3862)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3863)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	plp
.elseif(n = 3864)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3865)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 3868) && (n < 3887))
; 3868 to 3886
.if(n = 3868)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3869 || n = 3885 || n = 3886)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3870)
	; 10 bytes
	php
	ldx #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3871)
	; 11 bytes
	php
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	plp
.elseif(n = 3872)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3873)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3874)
	; 12 bytes
	php
	pha
	ldx #(n-15)/17
	jsr @nearby_rts
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3875)
	; 12 bytes
	php
	ldx #200 ;hides 'iny'
	ldy #8
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 3876)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #11
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 3877)
	; 12 bytes
	php
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	nop
	pla
	pha
	inx
	Jne *-7
	plp
.elseif(n = 3878)
	; 11 bytes
	php
	ldy #29
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 3879)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3880)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3881)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #121
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 3882)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3883)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 3884)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.endif
.else
; 3887 to 3906
.if(n = 3887 || n = 3891 || n = 3897 || n = 3904 || n = 3906)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3888)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3889)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #69
	dey
	Jne *-1
	dex
	Jmi *-7
	plp
.elseif(n = 3890 || n = 3898)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3892)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 3893)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3894)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3895)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3896)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3899)
	; 11 bytes
	php
	ldx #229 ;hides 'sbc zp'
	bit $EA ;hides 'nop'
	pla
	pha
	dex
	Jne *-6
	plp
.elseif(n = 3900)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3901)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3902)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 3903)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3905)
	; 10 bytes
	php
	ldx #(n-1)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
.if((n >= 3907) && (n < 3926))
; 3907 to 3925
.if(n = 3907)
	; 11 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 3908)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3909)
	; 12 bytes
	php
	ldx #83
	lda #15
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 3910)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	pla
	pha
	dex
	Jne *-5
	plp
.elseif(n = 3911)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3912)
	; 10 bytes
	php
	ldy #186 ;hides 'tsx'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	plp
.elseif(n = 3913 || n = 3915 || n = 3925)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3914)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3916)
	; 11 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 3917)
	; 11 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	nop
	pla
	pha
	dex
	Jne *-6
	plp
.elseif(n = 3918)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3919 || n = 3920)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3921)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	cmp $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
	plp
.elseif(n = 3922)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3923)
	; 11 bytes
	php
	ldx #135
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 3924)
	; 11 bytes
	php
	ldx #89
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.endif
.else
; 3926 to 3945
.if(n = 3926)
	; 10 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	nop
	pla
	pha
	dex
	Jne *-5
	plp
.elseif(n = 3927)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3928)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3929 || n = 3931 || n = 3938 || n = 3945)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3930 || n = 3937)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3932)
	; 12 bytes
	php
	ldx #(n-8)/18
	nop
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 3933)
	; 12 bytes
	php
	ldx #232 ;hides 'inx'
	.repeat 4
	 nop
	.endrepeat
	dex
	dex
	Jne *-7
	plp
.elseif(n = 3934)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 3935)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3936)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #96
	dex
	Jpl *-1
	Jcc *-7
	plp
.elseif(n = 3939)
	; 11 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 3940)
	; 12 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jmi *-7
	plp
.elseif(n = 3941)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3942)
	; 10 bytes
	php
	ldx #10 ;hides 'asl a'
	nop
	pla
	pha
	inx
	Jne *-5
	plp
.elseif(n = 3943)
	; 11 bytes
	php
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3944)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 3946) && (n < 3965))
; 3946 to 3964
.if(n = 3946)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 3947)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 3948)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3949)
	; 12 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	nop
	pla
	pha
	inx
	Jne *-5
	pla
	plp
.elseif(n = 3950)
	; 11 bytes
	php
	ldx #24 ;hides 'clc'
	rol $26
	ror $26
	inx
	Jne *-6
	plp
.elseif(n = 3951 || n = 3963)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3952)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3953)
	; 12 bytes
	php
	ldx #15
	lda #231
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 3954 || n = 3957 || n = 3961)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3955)
	; 11 bytes
	php
	ldx #9 ;hides 'ora #imm'
	lda $4868 ;hides 'pla' and 'pha'
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3956)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3958)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3959)
	; 11 bytes
	php
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 3960)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3962)
	; 11 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	inx
	Jpl *-6
	plp
.elseif(n = 3964)
	; 12 bytes
	php
	ldx #(n-8)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jne *-6
	plp
.endif
.else
; 3965 to 3984
.if(n = 3965)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #3
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 3966)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #21
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 3967)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 3968)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3969)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3970 || n = 3971)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3972 || n = 3982)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3973)
	; 12 bytes
	php
	ldx #(n-8)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 3974)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3975)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3976)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3977)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3978)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3979)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3980)
	; 12 bytes
	php
	ldx #(n-8)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 3981)
	; 11 bytes
	php
	ldx #137
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 3983)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 3984)
	; 11 bytes
	php
	ldx #(n-6)/17 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
.if((n >= 3985) && (n < 4004))
; 3985 to 4003
.if(n = 3985)
	; 11 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 3986 || n = 4003)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3987)
	; 12 bytes
	php
	ldx #22
	lda #141
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 3988)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3989 || n = 4001)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3990)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3991)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3992)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3993 || n = 3994 || n = 4000)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3995)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 3996)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #36
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 3997)
	; 12 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jmi *-7
	plp
.elseif(n = 3998)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3999)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4002)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #20
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.endif
.else
; 4004 to 4023
.if(n = 4004)
	; 11 bytes
	php
	ldx #73
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4005 || n = 4014)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4006)
	; 10 bytes
	php
	ldx #(n-6)/16 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 4007)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4008)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4009 || n = 4011 || n = 4013)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4010)
	; 10 bytes
	php
	ldx #74 ;hides 'lsr a'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 4012)
	; 11 bytes
	php
	ldx #90
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4015)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4016)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #98
	dex
	Jpl *-1
	Jcc *-7
	plp
.elseif(n = 4017)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4018)
	; 12 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	.repeat 4
	 nop
	.endrepeat
	inx
	Jne *-7
	plp
.elseif(n = 4019)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4020)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4021)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 4022)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4023)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
	plp
.endif
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4040 || n = 4056)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 4025 || n = 4026 || n = 4030 || n = 4041 || n = 4042 || n = 4050 || n = 4057 || n = 4060)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4027 || n = 4044)
	; 12 bytes
	php
	pha
	ldx #(n-15)/17
	jsr @nearby_rts
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4028 || n = 4048)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4029)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #115
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4031 || n = 4051)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4032 || n = 4052)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4033)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4034)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4035)
	; 12 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	dex
	Jne *-7
	plp
.elseif(n = 4036 || n = 4055)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4037 || n = 4054)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4038)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4039)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4043)
	; 12 bytes
	php
	ldx #15
	lda #237
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4045)
	; 13 bytes
	php
	pha
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
	pla
	plp
.elseif(n = 4046)
	; 12 bytes
	php
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	delay_n 4
	pla
	pha
	inx
	Jne *-7
	plp
.elseif(n = 4047)
	; 13 bytes
	php
	pha
	ldx #252
	delay_n 4
	pla
	pha
	dex
	Jne *-5
	pla
	plp
.elseif(n = 4049)
	; 13 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4053)
	; 12 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	rol $26
	ror $26
	dex
	Jne *-7
	plp
.elseif(n = 4058)
	; 11 bytes
	php
	ldx #74
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4059)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	jsr @nearby_rts
	dex
	Jne *-7
	plp
.elseif(n = 4061)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4062)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #5
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
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
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4064)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4065)
	; 12 bytes
	php
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	nop
	pla
	pha
	inx
	Jne *-7
	plp
.elseif(n = 4066)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	plp
.elseif(n = 4067)
	; 12 bytes
	php
	ldx #(n-49)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4068)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4069 || n = 4075 || n = 4077)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4070)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4071)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4072)
	; 10 bytes
	php
	ldx #42 ;hides 'rol a'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 4073 || n = 4076)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4074)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4078)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4079)
	; 12 bytes
	php
	ldx #68
	lda #27
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4080)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #63
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 4081)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dex
	Jpl *-7
	plp
.endif
.else
; 4082 to 4101
.if(n = 4082)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4083)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4084)
	; 11 bytes
	php
	clc
	lda #$2A ;hides 'rol a'
	ldx #202 ;hides 'dex'
	Jne *-1
	Jcc *-5
	plp
.elseif(n = 4085)
	; 12 bytes
	php
	ldx #26
	lda #119
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4086)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4087)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4088)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4089 || n = 4090 || n = 4095)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4091 || n = 4096 || n = 4101)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4092)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4093)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4094)
	; 11 bytes
	php
	ldx #200 ;hides 'iny'
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jmi *-6
	plp
.elseif(n = 4097)
	; 11 bytes
	php
	ldx #141
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4098)
	; 10 bytes
	php
	ldy #186 ;hides 'tsx'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	plp
.elseif(n = 4099)
	; 11 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4100)
	; 11 bytes
	php
	ldx #93
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 4102) && (n < 4121))
; 4102 to 4120
.if(n = 4102)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 4103)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4104)
	; 11 bytes
	php
	ldx #0
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 4105)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4106 || n = 4107 || n = 4114)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4108)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4109)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #72
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4110)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4111 || n = 4113 || n = 4115)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4112)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4116)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4117)
	; 12 bytes
	php
	pha
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	dex
	Jne *-5
	pla
	plp
.elseif(n = 4118)
	; 12 bytes
	php
	ldx #30
	lda #105
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4119)
	; 12 bytes
	php
	pha
	ldx #(n-15)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4120)
	; 12 bytes
	php
	ldx #(n-8)/257
	lda #225
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.endif
.else
; 4121 to 4140
.if(n = 4121)
	; 11 bytes
	php
	ldx #69 ;hides 'eor zp'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4122 || n = 4139)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4123)
	; 11 bytes
	php
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	inx
	inx
	Jne *-6
	plp
.elseif((n .mod 3 = 2) && ((n > 4123 && n < 4134)))
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4125)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4126)
	; 11 bytes
	php
	ldx #142
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4128)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4129)
	; 12 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
	plp
.elseif(n = 4131)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4132)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4134)
	; 11 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 4135)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4136)
	; 11 bytes
	php
	ldy #31
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 4137)
	; 12 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-7
	plp
.elseif(n = 4138)
	; 12 bytes
	php
	ldx #118
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4140)
	; 12 bytes
	php
	ldx #106 ;hides 'ror a'
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.endif
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4152 || n = 4157 || n = 4158 || n = 4161 || n = 4170 || n = 4171 || n = 4174 || n = 4175)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4142)
	; 11 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldy #136 ;hides 'dey'
	Jne *-1
	Jcc *-5
	plp
.elseif(n = 4143)
	; 11 bytes
	php
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4144)
	; 11 bytes
	php
	ldx #94
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 4145)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4146)
	; 11 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 4147)
	; 12 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
	plp
.elseif(n = 4148 || n = 4168)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4149)
	; 12 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	delay_n 4
	pla
	pha
	inx
	Jne *-7
	plp
.elseif(n = 4150 || n = 4169)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4151 || n = 4153 || n = 4159 || n = 4167 || n = 4172 || n = 4176)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4154)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #137
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4155)
	; 11 bytes
	php
	ldx #143
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4156 || n = 4173)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4160)
	; 11 bytes
	php
	ldx #122 ;hides 'nop'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-6
	plp
.elseif(n = 4162)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4163)
	; 12 bytes
	php
	ldx #(n-248)/261
	lda #245
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4164)
	; 10 bytes
	php
	ldx #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 4165)
	; 11 bytes
	php
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4166)
	; 11 bytes
	php
	ldx #76
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4177)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #63
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4178)
	; 12 bytes
	php
	ldx #30
	lda #107
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4179)
	; 12 bytes
	php
	ldx #96
	lda #11
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.endif
.else
.if((n >= 4180) && (n < 4199))
; 4180 to 4198
.if(n = 4180)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #118
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 4181)
	; 12 bytes
	php
	ldx #106
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4182)
	; 11 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-6
	plp
.elseif(n = 4183)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4184)
	; 11 bytes
	php
	ldx #144
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4185)
	; 11 bytes
	php
	ldy #200 ;hides 'iny'
	jsr @nearby_rts
	dey
	dey
	Jne *-6
	plp
.elseif(n = 4186)
	; 12 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4187)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4188)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4189)
	; 12 bytes
	php
	ldx #113
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4190)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4191)
	; 11 bytes
	pha
	php
	ldx #209
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4192)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4193)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4194)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4195)
	; 12 bytes
	php
	ldx #78
	lda #21
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4196)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 4197)
	; 12 bytes
	php
	ldx #71
	lda #27
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4198)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	ldy #74 ;hides 'lsr a'
	dey
	Jne *-2
	inx
	Jne *-7
	plp
.endif
.else
; 4199 to 4218
.if(n = 4199)
	; 12 bytes
	php
	ldx #127
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4200)
	; 12 bytes
	php
	ldx #131
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4201)
	; 12 bytes
	php
	ldx #233 ;hides 'sbc #imm'
	lda $EA ;hides 'nop'
	nop
	pla
	pha
	dex
	Jne *-7
	plp
.elseif(n = 4202)
	; 12 bytes
	php
	ldx #(n-8)/18
	nop
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 4203 || n = 4204 || n = 4209)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4205)
	; 13 bytes
	php
	clc
	lda #74 ;hides 'lsr a'
	ldx #(n-26)/21
	nop
	dex
	Jne *-2
	Jcc *-7
	plp
.elseif(n = 4206)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 4207)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4208)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4210)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4211)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4212)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4213)
	; 11 bytes
	php
	ldx #145
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4214)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #139
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4215)
	; 11 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	plp
.elseif(n = 4216)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #104
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 4217)
	; 12 bytes
	php
	ldx #(n-69)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4218)
	; 11 bytes
	php
	ldx #(n-6)/18 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
.if((n >= 4219) && (n < 4258))
.if((n >= 4219) && (n < 4238))
; 4219 to 4237
.if(n = 4219)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #74
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4220)
	; 11 bytes
	php
	ldx #77
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4221)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 4222)
	; 11 bytes
	php
	ldx #248 ;hides 'sed'
	rol $26
	ror $26
	dex
	Jne *-6
	plp
.elseif(n = 4223)
	; 12 bytes
	php
	ldx #(n-8)/281
	lda #249
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4224)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4225 || n = 4227)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4226)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4228)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4229)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4230 || n = 4234 || n = 4237)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4231)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	nop
	inx
	Jne *-6
	plp
.elseif(n = 4232)
	; 11 bytes
	php
	ldx #95
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4233)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4235)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 4236)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	delay_n 4
	pla
	pha
	inx
	Jne *-6
	plp
.endif
.else
; 4238 to 4257
.if(n = 4238)
	; 11 bytes
	php
	ldx #184 ;hides 'clv'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 4239)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $B8A2'
	ldx #184
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 4240)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4241)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4242)
	; 11 bytes
	php
	ldx #146
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4243 || n = 4247 || n = 4250)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4244 || n = 4251)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4245)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4246)
	; 12 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 11
	dex
	Jne *-7
	plp
.elseif(n = 4248)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4249)
	; 11 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
	plp
.elseif(n = 4252)
	; 11 bytes
	php
	ldy #138 ;hides 'txa'
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jmi *-6
	plp
.elseif(n = 4253)
	; 11 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4254)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 4255)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4256)
	; 11 bytes
	php
	ldx #(n-6)/17 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
	plp
.elseif(n = 4257)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	nop
	pla
	pha
	inx
	Jne *-6
	plp
.endif
.endif
.else
; 4258 to 4296
.if(n = 4258 || n = 4275 || n = 4292)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4259)
	; 12 bytes
	php
	ldx #108
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4260 || n = 4266 || n = 4270 || n = 4277 || n = 4285 || n = 4293 || n = 4294)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4261 || n = 4267 || n = 4278 || n = 4286 || n = 4295)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4262)
	; 12 bytes
	php
	ldx #152 ;hides 'tya'
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-7
	plp
.elseif(n = 4263)
	; 12 bytes
	php
	ldx #114
	lda #5
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4264 || n = 4283)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4265)
	; 11 bytes
	php
	ldy #32
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 4268 || n = 4288)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4269)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	plp
.elseif(n = 4271)
	; 11 bytes
	php
	ldx #147
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4272)
	; 11 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
	plp
.elseif(n = 4273)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	lda $EA ;hides 'nop'
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 4274)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	delay_n 4
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 4276)
	; 11 bytes
	php
	ldx #96
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4279)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4280)
	; 12 bytes
	php
	ldx #89
	lda #(n-264)/251
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4281)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #41
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4282)
	; 12 bytes
	php
	pha
	ldx #(n-15)/17
	jsr @nearby_rts
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4284)
	; 11 bytes
	php
	ldx #138 ;hides 'txa'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4287)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4289)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4290)
	; 11 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 4291)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 4296)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #105
	dex
	Jpl *-1
	Jcc *-7
	plp
.endif
.endif
.else
.if((n >= 4297) && (n < 4336))
.if((n >= 4297) && (n < 4316))
; 4297 to 4315
.if(n = 4297)
	; 11 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 4298)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4299)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #125
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4300)
	; 11 bytes
	php
	ldx #148
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4301)
	; 12 bytes
	php
	ldx #81
	lda #21
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4302)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4303)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $B3A2'
	ldx #179
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
	plp
.elseif(n = 4304 || n = 4310 || n = 4311)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4305 || n = 4312)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4306)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #138
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4307)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $BBA2'
	ldx #187
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 4308)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4309)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4313)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4314)
	; 12 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dex
	Jne *-7
	plp
.elseif(n = 4315)
	; 12 bytes
	php
	ldx #72
	lda #27
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.endif
.else
; 4316 to 4335
.if(n = 4316)
	; 12 bytes
	php
	pha
	ldx #(n-15)/17
	jsr @nearby_rts
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4317)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4318)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #43
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 4319)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 4320)
	; 11 bytes
	php
	ldx #97
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4321)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4322 || n = 4323 || n = 4330)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4324)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4325)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4326)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4327)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4328)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4329)
	; 11 bytes
	php
	ldx #149
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4331)
	; 11 bytes
	php
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 4332)
	; 12 bytes
	php
	ldx #(n-8)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jne *-6
	plp
.elseif(n = 4333)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4334)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4335)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.endif
.endif
.else
.if((n >= 4336) && (n < 4356))
; 4336 to 4355
.if(n = 4336)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #106
	dex
	Jpl *-1
	Jcc *-7
	plp
.elseif(n = 4337)
	; 12 bytes
	php
	ldx #110
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4338)
	; 12 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	delay_n 9
	dex
	Jne *-7
	plp
.elseif(n = 4339)
	; 12 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	delay_n 11
	dex
	Jne *-7
	plp
.elseif(n = 4340)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4341)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 4342)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4343)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4344)
	; 12 bytes
	php
	ldx #(n-8)/271
	lda #239
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4345 || n = 4350)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4346 || n = 4351)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4347)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4348)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4349)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #67
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4352)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4353)
	; 12 bytes
	php
	ldx #78
	lda #23
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4354)
	; 11 bytes
	php
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 4355)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.endif
.else
; 4356 to 4375
.if(n = 4356)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dex
	Jpl *-7
	plp
.elseif(n = 4357)
	; 12 bytes
	php
	ldx #229 ;hides 'sbc zp'
	bit $EA ;hides 'nop'
	nop
	pla
	pha
	dex
	Jne *-7
	plp
.elseif(n = 4358)
	; 11 bytes
	php
	ldx #150
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4359)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4360)
	; 10 bytes
	php
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4361 || n = 4366 || n = 4370)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4362)
	; 10 bytes
	php
	ldx #58 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 4363)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 4364)
	; 11 bytes
	php
	ldx #98
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4365 || n = 4371)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4367)
	; 11 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 4368)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4369)
	; 12 bytes
	php
	ldx #89
	lda #n/257
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4372)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4373)
	; 12 bytes
	php
	ldx #96
	lda #13
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4374)
	; 11 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 4375)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $D0A2'
	ldx #208
	jsr @nearby_rts
	dex
	Jne *-7
	plp
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
.if(n = 4376)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 4377)
	; 12 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	delay_n 4
	pla
	pha
	dex
	Jne *-7
	plp
.elseif(n = 4378)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4379 || n = 4381 || n = 4391)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4380 || n = 4389 || n = 4390)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4382)
	; 11 bytes
	php
	ldx #81
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 4383)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4384)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #165
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 4385)
	; 12 bytes
	php
	pha
	ldx #(n-15)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4386)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4387)
	; 11 bytes
	php
	ldx #151
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4388)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4392)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4393)
	; 11 bytes
	php
	ldx #(n-1)/18 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
	plp
.elseif(n = 4394)
	; 11 bytes
	php
	ldy #33
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.endif
.else
; 4395 to 4414
.if(n = 4395)
	; 12 bytes
	php
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
	plp
.elseif(n = 4396 || n = 4399 || n = 4410)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4397)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4398)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	nop
	pla
	pha
	dex
	Jne *-6
	plp
.elseif(n = 4400 || n = 4409)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4401)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #36
	dex
	Jpl *-1
	Jcs *-7
	plp
.elseif(n = 4402)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4403)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4404)
	; 12 bytes
	php
	ldx #27
	lda #125
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4405)
	; 13 bytes
	php
	pha
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	nop
	pla
	pha
	dex
	Jne *-6
	pla
	plp
.elseif(n = 4406)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 4407)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 4408)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4411)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	cmp $4868 ;hides 'pla' and 'pha'
	nop
	dex
	Jne *-6
	plp
.elseif(n = 4412)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4413)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4414)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 4415) && (n < 4434))
; 4415 to 4433
.if(n = 4415)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	cmp $EA ;hides 'nop'
	pla
	pha
	dex
	Jne *-6
	plp
.elseif(n = 4416)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4417 || n = 4419 || n = 4431)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4418)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4420)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4421)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 4422)
	; 11 bytes
	php
	ldx #184 ;hides 'clv'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 4423)
	; 12 bytes
	php
	pha
	ldx #(n-15)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4424)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4425)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #125
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 4426)
	; 12 bytes
	php
	ldx #94
	lda #15
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4427)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 4428)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4429)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4430)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4432)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4433)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.endif
.else
; 4434 to 4453
.if(n = 4434)
	; 11 bytes
	php
	ldx #10 ;hides 'asl a'
	delay_n 4
	pla
	pha
	inx
	Jne *-6
	plp
.elseif(n = 4435)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4436)
	; 11 bytes
	php
	ldx #81
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4437 || n = 4447)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4438 || n = 4451 || n = 4453)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4439)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4440)
	; 12 bytes
	php
	ldx #16
	lda #245
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4441)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4442)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldy #232 ;hides 'inx'
	dey
	Jmi *-2
	Jcc *-6
	plp
.elseif(n = 4443)
	; 12 bytes
	php
	ldx #136 ;hides 'dey'
	ldy #96
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4444)
	; 13 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	lda #118
	jsr delay_a_25_clocks
	inx
	Jne *-8
	plp
.elseif(n = 4445)
	; 11 bytes
	php
	ldx #153
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4446)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #151
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 4448)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4449)
	; 12 bytes
	php
	ldx #9 ;hides 'ora #imm'
	lda $4868 ;hides 'pla' and 'pha'
	delay_n 4
	inx
	Jne *-7
	plp
.elseif(n = 4450)
	; 10 bytes
	php
	ldy #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	plp
.elseif(n = 4452)
	; 10 bytes
	php
	ldx #(n-6)/19 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
.if((n >= 4454) && (n < 4473))
; 4454 to 4472
.if(n = 4454)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4455 || n = 4457 || n = 4471)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4456)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4458)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 4459)
	; 12 bytes
	php
	ldx #200 ;hides 'iny'
	lda #27
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 4460)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4461)
	; 12 bytes
	php
	ldx #(n-69)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4462)
	; 12 bytes
	php
	ldx #131
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4463)
	; 12 bytes
	php
	ldx #135
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4464)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #147
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 4465)
	; 12 bytes
	php
	ldx #218 ;hides 'nop'
	lda #15
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 4466)
	; 12 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	lda $A5
	inx
	Jne *-7
	plp
.elseif(n = 4467)
	; 12 bytes
	php
	ldx #90
	lda #17
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4468)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4469)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $C2A2'
	ldx #194
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 4470)
	; 11 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 4472)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
	plp
.endif
.else
; 4473 to 4492
.if(n = 4473 || n = 4492)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4474)
	; 11 bytes
	php
	ldx #154
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4475)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4476 || n = 4482 || n = 4491)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4477)
	; 12 bytes
	php
	ldx #(n-49)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4478)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	inx
	Jpl *-7
	plp
.elseif(n = 4479)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 4480)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4481)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4483)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4484)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #148
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4485)
	; 12 bytes
	php
	ldx #(n-129)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4486)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #46
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 4487)
	; 13 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	delay_n 11
	dex
	Jne *-8
	plp
.elseif(n = 4488)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4489)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4490)
	; 11 bytes
	php
	ldx #83
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 4493) && (n < 4512))
; 4493 to 4511
.if(n = 4493)
	; 12 bytes
	php
	ldx #114
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4494 || n = 4498 || n = 4505 || n = 4510)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4495)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4496)
	; 11 bytes
	php
	ldx #101
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4497)
	; 12 bytes
	php
	ldx #67
	lda #35
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4499)
	; 12 bytes
	php
	pha
	ldx #(n-15)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4500)
	; 10 bytes
	php
	ldx #42 ;hides 'rol a'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 4501)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	jsr @nearby_rts
	dex
	Jne *-7
	plp
.elseif(n = 4502)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4503)
	; 11 bytes
	php
	ldx #155
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4504)
	; 12 bytes
	php
	ldx #(n-8)/281
	lda #249
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4506)
	; 11 bytes
	php
	ldx #(n-6)/18 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 4507)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4508)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4509)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #45
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4511)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.endif
.else
; 4512 to 4531
.if(n = 4512)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4513)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4514 || n = 4531)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4515)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 4516)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4517)
	; 12 bytes
	php
	ldx #26
	lda #135
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4518)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4519)
	; 12 bytes
	php
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	plp
.elseif(n = 4520)
	; 12 bytes
	php
	ldx #141
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4521)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #71
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4522)
	; 12 bytes
	php
	ldx #121
	lda #5
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4523)
	; 11 bytes
	php
	ldy #35
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.elseif(n = 4524)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4525)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4526)
	; 12 bytes
	php
	ldx #(n-8)/18
	nop
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 4527)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 28
	Jne *-7
	plp
.elseif(n = 4528)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4529)
	; 12 bytes
	php
	ldx #137
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4530)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
.if((n >= 4532) && (n < 4551))
; 4532 to 4550
.if(n = 4532)
	; 11 bytes
	php
	ldx #156
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4533 || n = 4541 || n = 4545)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4534)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4535)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4536)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #111
	dex
	Jpl *-1
	Jcc *-7
	plp
.elseif(n = 4537)
	; 12 bytes
	php
	pha
	ldx #(n-15)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4538)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 4539)
	; 12 bytes
	php
	ldx #22
	lda #165
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4540)
	; 11 bytes
	php
	ldx #102
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4542)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4543)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4544)
	; 11 bytes
	php
	ldx #84
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 4546 || n = 4550)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4547)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 4548)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4549)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.endif
.else
; 4551 to 4570
.if(n = 4551 || n = 4563 || n = 4570)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4552)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4553)
	; 12 bytes
	php
	ldx #(n-8)/101
	lda #69
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4554)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #150
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 4555)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4556)
	; 12 bytes
	php
	clc
	lda #$4A ;hides 'lsr a'
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	Jcc *-6
	plp
.elseif(n = 4557)
	; 12 bytes
	php
	ldx #168 ;hides 'tay'
	lda #77
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 4558)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4559)
	; 12 bytes
	php
	ldx #(n-49)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4560)
	; 11 bytes
	php
	ldx #58 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4561)
	; 11 bytes
	php
	ldx #157
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4562)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4564)
	; 12 bytes
	php
	ldx #106 ;hides 'ror a'
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-7
	plp
.elseif(n = 4565)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4566)
	; 12 bytes
	php
	ldx #86
	lda #21
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4567)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 4568)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4569)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #135
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.endif
.endif
.else
.if((n >= 4571) && (n < 4590))
; 4571 to 4589
.if(n = 4571)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4572)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4573)
	; 12 bytes
	php
	ldx #83
	lda #(n-180)/191
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4574)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #151
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4575)
	; 12 bytes
	php
	pha
	ldx #(n-15)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4576)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #113
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 4577)
	; 13 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	ldy #181
	dey
	Jne *-1
	dex
	Jne *-8
	plp
.elseif(n = 4578)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4579)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4580)
	; 12 bytes
	php
	ldx #(n-8)/18
	nop
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 4581)
	; 12 bytes
	php
	ldx #16
	lda #237
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4582)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #10
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 4583)
	; 11 bytes
	php
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-6
	plp
.elseif(n = 4584)
	; 10 bytes
	php
	ldx #(n-6)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4585)
	; 12 bytes
	php
	ldx #22
	lda #167
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4586)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4587)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4588)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4589)
	; 11 bytes
	delay_n (n-2)
	nop
.endif
.else
; 4590 to 4609
.if(n = 4590)
	; 11 bytes
	php
	ldx #158
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4591 || n = 4601 || n = 4609)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4592)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4593)
	; 12 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	nop
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 4594)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jne *-7
	plp
.elseif(n = 4595)
	; 12 bytes
	php
	ldx #139
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4596)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4597)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4598)
	; 11 bytes
	php
	ldx #84
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4599)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4600)
	; 12 bytes
	php
	ldx #(n-64)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4602)
	; 13 bytes
	php
	tya
	ldx #84
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	tay
	plp
.elseif(n = 4603)
	; 13 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	ldy #228
	dey
	Jne *-1
	dex
	Jne *-8
	plp
.elseif(n = 4604)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #152
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4605)
	; 12 bytes
	php
	ldx #200 ;hides 'iny'
	ldy #10
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4606)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4607)
	; 11 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 4608)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
.if((n >= 4610) && (n < 4629))
; 4610 to 4628
.if(n = 4610 || n = 4617 || n = 4621 || n = 4627)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4611 || n = 4622)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4612)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4613)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4614)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 9
	dex
	Jne *-7
	plp
.elseif(n = 4615)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 9
	inx
	Jne *-7
	plp
.elseif(n = 4616)
	; 11 bytes
	php
	ldy #12
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	plp
.elseif(n = 4618)
	; 12 bytes
	php
	ldx #(n-469)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4619)
	; 11 bytes
	php
	ldx #159
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4620)
	; 13 bytes
	php
	tya
	ldy #12
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4623)
	; 12 bytes
	php
	ldx #71
	lda #33
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4624)
	; 13 bytes
	php
	clc
	lda #$2A ;hides 'rol a'
	ldx #164
	nop
	dex
	Jne *-2
	Jcc *-7
	plp
.elseif(n = 4625)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4626)
	; 12 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
	plp
.elseif(n = 4628)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.endif
.else
; 4629 to 4648
.if(n = 4629)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4630 || n = 4645 || n = 4646)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4631 || n = 4647)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4632)
	; 11 bytes
	php
	ldx #16
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 4633)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4634)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #153
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4635)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #131
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 4636)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4637)
	; 12 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dex
	Jne *-7
	plp
.elseif(n = 4638)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #159
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 4639)
	; 12 bytes
	php
	ldx #(n-429)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4640)
	; 12 bytes
	php
	ldx #24
	lda #161
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4641)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 4642)
	; 12 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
	plp
.elseif(n = 4643)
	; 12 bytes
	php
	ldx #45
	lda #71
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4644)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4648)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 4649) && (n < 4668))
; 4649 to 4667
.if(n = 4649)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4650)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4651 || n = 4666)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4652)
	; 11 bytes
	php
	ldx #86
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 4653)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 4654)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4655)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4656)
	; 11 bytes
	php
	ldx #106 ;hides 'ror a'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-6
	plp
.elseif(n = 4657)
	; 12 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	plp
.elseif(n = 4658)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4659)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #82
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4660)
	; 12 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	pla
	pha
	dex
	Jne *-7
	plp
.elseif(n = 4661)
	; 12 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	nop
	rol $26
	ror $26
	dex
	Jne *-7
	plp
.elseif(n = 4662)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4663)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4664)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #154
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4665)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4667)
	; 12 bytes
	php
	ldy #186 ;hides 'tsx'
	lda #45
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	plp
.endif
.else
; 4668 to 4687
.if(n = 4668)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4669)
	; 12 bytes
	php
	ldx #78
	lda #27
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4670)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4671 || n = 4683 || n = 4685)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4672)
	; 11 bytes
	php
	ldx #105
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4673)
	; 12 bytes
	php
	ldx #(n-8)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
	plp
.elseif(n = 4674 || n = 4679 || n = 4684)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4675)
	; 11 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
	plp
.elseif(n = 4676)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 4677)
	; 11 bytes
	php
	ldx #161
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4678)
	; 12 bytes
	php
	ldx #200 ;hides 'iny'
	lda #30
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 4680)
	; 10 bytes
	php
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 4681)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #153
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4682)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4686)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	delay_n 11
	dex
	Jne *-7
	plp
.elseif(n = 4687)
	; 12 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts
	inx
	Jne *-5
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
.if((n >= 4688) && (n < 4707))
; 4688 to 4706
.if(n = 4688)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4689)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4690 || n = 4702 || n = 4703)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4691)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4692)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4693)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #75
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4694)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4695)
	; 12 bytes
	php
	ldx #42
	lda #77
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4696)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #115
	dex
	Jpl *-1
	Jcc *-7
	plp
.elseif(n = 4697)
	; 12 bytes
	php
	ldx #(n-529)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4698)
	; 11 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4699)
	; 12 bytes
	php
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 4700)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4701)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4704)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4705)
	; 12 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	nop
	pla
	pha
	inx
	Jne *-7
	plp
.elseif(n = 4706)
	; 11 bytes
	php
	ldx #162
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.endif
.else
; 4707 to 4726
.if(n = 4707)
	; 12 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4708)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4709 || n = 4711 || n = 4717 || n = 4719 || n = 4723)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4710 || n = 4722)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4712)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4713)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4714)
	; 10 bytes
	php
	ldx #42 ;hides 'rol a'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 4715)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4716)
	; 11 bytes
	php
	ldx #106
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4718)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4720)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4721)
	; 11 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4724)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4725)
	; 12 bytes
	php
	ldx #89
	lda #21
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4726)
	; 12 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	delay_n 4
	pla
	pha
	inx
	Jne *-7
	plp
.endif
.endif
.else
.if((n >= 4727) && (n < 4746))
; 4727 to 4745
.if(n = 4727)
	; 12 bytes
	php
	ldx #120
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4728)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4729 || n = 4730 || n = 4737 || n = 4741)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4731 || n = 4742)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4732)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4733)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4734)
	; 12 bytes
	php
	ldx #139
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4735)
	; 11 bytes
	php
	ldx #163
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4736)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #117
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 4738)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4739)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4740)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4743)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4744)
	; 12 bytes
	php
	ldx #148
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4745)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.endif
.else
; 4746 to 4765
.if(n = 4746)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4747)
	; 13 bytes
	php
	ldx #196 ;hides 'cpy zp'
	nop
	lda #44
	jsr delay_a_25_clocks
	inx
	Jne *-8
	plp
.elseif(n = 4748)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4749)
	; 12 bytes
	php
	ldx #(n-439)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4750 || n = 4762)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4751 || n = 4759 || n = 4761)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4752)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4753)
	; 12 bytes
	php
	ldx #72
	lda #33
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4754)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4755)
	; 12 bytes
	php
	ldx #(n-109)/101
	lda #69
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4756)
	; 10 bytes
	php
	ldx #(n-6)/19 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4757)
	; 12 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	delay_n 4
	pla
	pha
	inx
	Jne *-7
	plp
.elseif(n = 4758)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4760)
	; 11 bytes
	php
	ldx #107
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4763)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4764)
	; 11 bytes
	php
	ldx #164
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4765)
	; 12 bytes
	php
	ldx #71
	lda #35
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
.if((n >= 4766) && (n < 4785))
; 4766 to 4784
.if(n = 4766)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4767)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $CFA2'
	ldx #207
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 4768)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4769)
	; 12 bytes
	php
	ldx #68
	lda #37
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4770 || n = 4779)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4771 || n = 4780)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4772)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4773)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts
	inx
	Jne *-7
	plp
.elseif(n = 4774)
	; 12 bytes
	php
	ldx #5 ;hides 'ora zp'
	lda $EAEA ;hides two 'nop's
	pla
	pha
	inx
	Jne *-7
	plp
.elseif(n = 4775)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4776)
	; 12 bytes
	php
	ldx #5 ;hides 'ora zp'
	delay_n 4
	nop
	pla
	pha
	inx
	Jne *-7
	plp
.elseif(n = 4777)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4778)
	; 12 bytes
	php
	ldx #89
	lda #21
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4781)
	; 11 bytes
	php
	ldy #37
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.elseif(n = 4782)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4783)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4784)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #158
	dex
	Jne *-1
	Jcc *-6
	plp
.endif
.else
; 4785 to 4804
.if(n = 4785)
	; 12 bytes
	php
	ldx #(n-289)/281
	lda #249
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4786)
	; 12 bytes
	php
	ldx #10 ;hides 'asl a'
	ldy #93
	dey
	Jpl *-1
	dex
	Jne *-7
	plp
.elseif(n = 4787)
	; 12 bytes
	php
	ldx #81
	lda #27
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4788)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4789)
	; 13 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	nop
	ldx #135
	dex
	Jne *-1
	Jcs *-8
	plp
.elseif(n = 4790)
	; 11 bytes
	php
	ldx #24 ;hides 'clc'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	inx
	Jpl *-6
	plp
.elseif(n = 4791 || n = 4799)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4792)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4793)
	; 11 bytes
	php
	ldx #165
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4794)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #50
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4795 || n = 4798)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4796)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4797)
	; 13 bytes
	php
	pha
	ldx #24 ;hides 'clc'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	inx
	Jpl *-6
	pla
	plp
.elseif(n = 4800)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #13
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 4801)
	; 12 bytes
	php
	ldx #229 ;hides 'sbc zp'
	nop
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4802)
	; 10 bytes
	php
	ldx #(n-6)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4803)
	; 11 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4804)
	; 11 bytes
	php
	ldx #108
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.endif
.endif
.else
.if((n >= 4805) && (n < 4824))
; 4805 to 4823
.if(n = 4805 || n = 4811 || n = 4818)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4806)
	; 11 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 4807)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $C8A2'
	ldx #200
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
	plp
.elseif(n = 4808)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4809)
	; 12 bytes
	php
	pha
	ldx #(n-13)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	pla
	plp
.elseif(n = 4810)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4812)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4813)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4814)
	; 11 bytes
	php
	ldx #89
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 4815)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4816)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #118
	dex
	Jpl *-1
	Jcc *-7
	plp
.elseif(n = 4817)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4819)
	; 12 bytes
	php
	ldx #16
	lda #251
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4820)
	; 12 bytes
	php
	ldx #58 ;hides 'nop'
	lda #49
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 4821)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #11
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 4822)
	; 11 bytes
	php
	ldx #166
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4823)
	; 12 bytes
	php
	ldx #45
	lda #75
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.endif
.else
; 4824 to 4843
.if(n = 4824 || n = 4830)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4825)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4826)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4827)
	; 12 bytes
	php
	ldx #(n-69)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4828)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4829)
	; 12 bytes
	php
	ldx #218 ;hides 'nop'
	lda #19
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 4831)
	; 11 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldy #136 ;hides 'dey'
	Jne *-1
	Jcs *-6
	plp
.elseif(n = 4832)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4833)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4834)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4835)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4836)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 4837 || n = 4839)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4838)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4840)
	; 12 bytes
	php
	ldx #151
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4841)
	; 12 bytes
	php
	ldx #26
	lda #147
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4842)
	; 12 bytes
	php
	ldx #(n-6)/186 ;hides 'nop'
	lda #152
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 4843)
	; 12 bytes
	php
	pha
	ldx #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
.if((n >= 4844) && (n < 4863))
; 4844 to 4862
.if(n = 4844)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4845)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #137
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 4846)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 4847)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4848)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4849)
	; 12 bytes
	php
	ldx #46
	lda #71
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4850 || n = 4855)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4851)
	; 11 bytes
	php
	ldx #167
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4852)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4853)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4854)
	; 11 bytes
	php
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 4856 || n = 4862)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4857)
	; 11 bytes
	php
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	plp
.elseif(n = 4858)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4859)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4860)
	; 12 bytes
	php
	pha
	ldx #(n-15)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4861)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #40
	dex
	Jpl *-1
	Jcs *-7
	plp
.endif
.else
; 4863 to 4882
.if(n = 4863)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4864)
	; 13 bytes
	php
	pha
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	pla
	plp
.elseif(n = 4865)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #79
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4866)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #56
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 4867)
	; 12 bytes
	php
	ldx #42
	lda #81
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4868)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4869)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 4870)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4871 || n = 4875 || n = 4881)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4872)
	; 10 bytes
	php
	ldx #0
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4873)
	; 12 bytes
	php
	ldx #139
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4874)
	; 11 bytes
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
.elseif(n = 4877)
	; 12 bytes
	php
	ldx #(n-549)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4878)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 4879)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #86
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4880)
	; 11 bytes
	php
	ldx #168
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4882)
	; 11 bytes
	php
	ldx #106 ;hides 'ror a'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 4883) && (n < 4902))
; 4883 to 4901
.if(n = 4883)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4884)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4885)
	; 12 bytes
	php
	ldx #168 ;hides 'tay'
	lda #85
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 4886)
	; 12 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	delay_n 4
	pla
	pha
	dex
	Jne *-7
	plp
.elseif(n = 4887)
	; 12 bytes
	php
	ldx #(n-49)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jpl *-6
	plp
.elseif(n = 4888)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4889)
	; 13 bytes
	php
	pha
	ldx #106 ;hides 'ror a'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-6
	pla
	plp
.elseif(n = 4890 || n = 4894)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4891)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4892)
	; 11 bytes
	php
	ldx #110
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4893)
	; 12 bytes
	php
	ldx #(n-187)/181 ;hides 'nop'
	lda #147
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4895)
	; 12 bytes
	php
	ldx #(n-189)/181
	lda #149
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4896)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4897)
	; 12 bytes
	php
	ldx #200 ;hides 'iny'
	lda #33
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 4898)
	; 12 bytes
	php
	ldx #30
	lda #131
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4899)
	; 12 bytes
	php
	ldx #72
	lda #35
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4900)
	; 12 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dex
	Jne *-7
	plp
.elseif(n = 4901)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.endif
.else
; 4902 to 4921
.if(n = 4902)
	; 11 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 4903)
	; 12 bytes
	php
	ldx #89
	lda #23
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4904)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 4905)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 4906)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4907)
	; 12 bytes
	php
	ldx #71
	lda #37
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4908)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4909)
	; 11 bytes
	php
	ldx #169
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4910)
	; 11 bytes
	php
	ldy #37
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 4911 || n = 4917)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4912)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4913)
	; 12 bytes
	php
	ldx #(n-143)/106
	lda #77
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4914)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4915)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #139
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 4916)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4918)
	; 12 bytes
	php
	ldx #(n-499)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4919)
	; 13 bytes
	php
	clc
	lda #74 ;hides 'lsr a'
	ldx #(n-26)/21
	nop
	dex
	Jne *-2
	Jcc *-7
	plp
.elseif(n = 4920)
	; 10 bytes
	php
	ldx #(n-6)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4921)
	; 11 bytes
	php
	ldx #16
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jpl *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
.if((n >= 4922) && (n < 4941))
; 4922 to 4940
.if(n = 4922)
	; 11 bytes
	php
	ldx #90
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4923 || n = 4931 || n = 4939)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4924 || n = 4930)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4925)
	; 12 bytes
	php
	ldx #149
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4926)
	; 11 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 4927)
	; 12 bytes
	php
	ldy #74 ;hides 'lsr a'
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	dey
	Jne *-7
	plp
.elseif(n = 4928)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4929)
	; 12 bytes
	php
	ldx #133
	lda #(n-824)/821
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4932)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4933)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4934)
	; 11 bytes
	php
	ldx #168 ;hides 'tay'
	ldy #136 ;hides 'dey'
	Jmi *-1
	inx
	Jne *-6
	plp
.elseif(n = 4935)
	; 12 bytes
	php
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 4936)
	; 11 bytes
	php
	ldx #111
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4937)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4938)
	; 11 bytes
	php
	ldx #170
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4940)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.endif
.else
; 4941 to 4960
.if(n = 4941)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 4942)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	jsr @nearby_rts
	inx
	Jne *-7
	plp
.elseif(n = 4943)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4944)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #163
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 4945 || n = 4950 || n = 4959 || n = 4960)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4946)
	; 12 bytes
	php
	ldx #202 ;hides 'dex'
	lda #96
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 4947)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #149
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4948)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4949)
	; 12 bytes
	php
	ldx #(n-8)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4951)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4952)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4953)
	; 12 bytes
	php
	ldx #42
	lda #83
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4954)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4955)
	; 12 bytes
	php
	ldx #96
	lda #19
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4956)
	; 12 bytes
	php
	ldx #106 ;hides 'ror a'
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 4957)
	; 12 bytes
	php
	ldx #(n-109)/101
	lda #69
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 4958)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 4961) && (n < 4981))
; 4961 to 4980
.if(n = 4961)
	; 12 bytes
	php
	ldx #127
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 4962)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 4963)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 4964)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4965)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #53
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4966)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	delay_n 11
	dex
	Jne *-7
	plp
.elseif(n = 4967)
	; 11 bytes
	php
	ldx #171
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4968)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4969 || n = 4970)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4971 || n = 4977 || n = 4979)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4972)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4973)
	; 12 bytes
	php
	ldx #(n-8)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 4974)
	; 11 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 4975)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4976)
	; 11 bytes
	php
	ldx #91
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 4978)
	; 12 bytes
	php
	ldx #142
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4980)
	; 11 bytes
	php
	ldx #113
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.endif
.else
; 4981 to 5000
.if(n = 4981)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #165
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 4982 || n = 4987 || n = 4998)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4983)
	; 11 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 4984)
	; 11 bytes
	php
	ldy #218 ;hides 'nop'
	ldx #232 ;hides 'inx'
	Jne *-1
	iny
	Jne *-6
	plp
.elseif(n = 4985)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4986)
	; 12 bytes
	php
	ldx #131
	lda #6
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 4988)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4989)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #88
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 4990)
	; 11 bytes
	php
	ldx #216 ;hides 'cld'
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jmi *-6
	plp
.elseif(n = 4991 || n = 4993)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4992)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4994)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #165
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 4995)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 4996)
	; 11 bytes
	php
	ldx #172
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 4997)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 4999)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5000)
	; 11 bytes
	php
	ldy #13
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
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
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
.if((n >= 5001) && (n < 5626))
.if((n >= 5001) && (n < 5313))
.if((n >= 5001) && (n < 5157))
.if((n >= 5001) && (n < 5079))
.if((n >= 5001) && (n < 5040))
.if((n >= 5001) && (n < 5020))
; 5001 to 5019
.if(n = 5001)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 5002)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5003)
	; 12 bytes
	php
	ldx #135
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5004)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #85
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5005)
	; 12 bytes
	php
	ldx #19
	lda #231
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5006)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5007)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 5008)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 5009 || n = 5010 || n = 5017)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5011)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5012)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5013)
	; 12 bytes
	php
	ldx #143
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5014)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5015)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 5016)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #124
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5018)
	; 12 bytes
	php
	ldx #30
	lda #135
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5019)
	; 13 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	lda #144
	jsr delay_a_25_clocks
	inx
	Jne *-8
	plp
.endif
.else
; 5020 to 5039
.if(n = 5020)
	; 11 bytes
	php
	ldx #(n-6)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 5021)
	; 11 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5022)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5023)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 5024)
	; 11 bytes
	php
	ldx #113
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5025)
	; 11 bytes
	php
	ldx #173
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5026)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5027)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 5028)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 5029)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5030)
	; 11 bytes
	php
	ldx #93
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 5031)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5032)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5033)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5034)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
	Jcs *-7
	plp
.elseif(n = 5035)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #13
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5036)
	; 12 bytes
	php
	ldx #10 ;hides 'asl a'
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-7
	plp
.elseif(n = 5037)
	; 12 bytes
	php
	ldx #46
	lda #75
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5038)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #34
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5039)
	; 11 bytes
	php
	ldy #39
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 5040) && (n < 5059))
; 5040 to 5058
.if(n = 5040)
	; 12 bytes
	php
	ldx #148
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5041 || n = 5050 || n = 5053 || n = 5056)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5042 || n = 5051)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5043)
	; 12 bytes
	php
	ldx #94
	lda #21
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5044)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5045)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 5046)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5047)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
	plp
.elseif(n = 5048)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 5049)
	; 12 bytes
	php
	ldx #(n-8)/71
	lda #39
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5052)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5054)
	; 11 bytes
	php
	ldx #174
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5055)
	; 12 bytes
	php
	ldx #202 ;hides 'dex'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	dex
	Jne *-7
	plp
.elseif(n = 5057)
	; 12 bytes
	php
	ldx #153
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5058)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.endif
.else
; 5059 to 5078
.if(n = 5059 || n = 5070)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5060)
	; 12 bytes
	php
	ldx #202 ;hides 'dex'
	lda #99
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5061)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5062)
	; 12 bytes
	php
	ldx #133
	lda #6
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5063)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5064)
	; 12 bytes
	php
	ldx #158
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5065)
	; 13 bytes
	php
	clc
	lda #233 ;hides 'sbc #$F8'
	sed ;first loop only
	ldx #(n-40)/25
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5066)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 5067)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5068)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 5069)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5071)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5072)
	; 12 bytes
	php
	clc
	lda #42 ;hides 'rol a'
	ldx #(n-32)/20
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5073)
	; 12 bytes
	php
	ldx #136 ;hides 'dey'
	ldy #111
	dey
	Jne *-1
	dex
	Jmi *-7
	plp
.elseif(n = 5074)
	; 12 bytes
	php
	ldx #149
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5075)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 5076)
	; 12 bytes
	php
	ldx #(n-189)/181
	lda #149
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5077)
	; 12 bytes
	php
	ldx #137
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5078)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
.if((n >= 5079) && (n < 5098))
; 5079 to 5097
.if(n = 5079)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #55
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5080)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	inx
	Jne *-7
	plp
.elseif(n = 5081)
	; 12 bytes
	php
	ldx #89
	lda #(n-181)/196
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5082)
	; 12 bytes
	php
	ldx #86
	lda #27
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5083)
	; 11 bytes
	php
	ldx #175
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5084)
	; 11 bytes
	php
	ldx #94
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 5085)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5086)
	; 12 bytes
	php
	ldx #216 ;hides 'cld'
	lda #93
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5087 || n = 5091 || n = 5097)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5088)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 5089)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts
	inx
	Jne *-7
	plp
.elseif(n = 5090)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5092)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5093)
	; 12 bytes
	php
	ldx #45
	lda #81
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5094)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5095)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 5096)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #125
	dex
	Jpl *-1
	Jcc *-7
	plp
.endif
.else
; 5098 to 5117
.if(n = 5098)
	; 12 bytes
	php
	ldx #202 ;hides 'dex'
	lda #100
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5099)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #91
	dey
	Jne *-1
	dex
	Jmi *-7
	plp
.elseif(n = 5100)
	; 12 bytes
	php
	ldx #134
	lda #6
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5101 || n = 5113)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5102)
	; 11 bytes
	php
	ldx #218 ;hides 'nop'
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jmi *-6
	plp
.elseif(n = 5103)
	; 11 bytes
	php
	ldy #4
	ldx #202 ;hides 'dex'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 5104)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5105)
	; 12 bytes
	php
	clc
	lda #$E9 ;hides 'sbc #$F8'
	sed ;first loop only
	ldx #202 ;hides 'dex'
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5106)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #170
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5107)
	; 13 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 11
	inx
	Jne *-8
	plp
.elseif(n = 5108)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 5109)
	; 11 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 5110)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 5111)
	; 11 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #202 ;hides 'dex'
	Jne *-1
	dey
	Jmi *-6
	plp
.elseif(n = 5112)
	; 11 bytes
	php
	ldx #176
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5114)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5115)
	; 12 bytes
	php
	pha
	ldx #(n-15)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 5116)
	; 12 bytes
	php
	ldx #58 ;hides 'nop'
	lda #39
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 5117)
	; 12 bytes
	php
	ldx #131
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 5118) && (n < 5137))
; 5118 to 5136
.if(n = 5118)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5119)
	; 12 bytes
	php
	ldx #19
	lda #237
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5120)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5121)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5122)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5123)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5124)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #169
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 5125)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #145
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5126)
	; 12 bytes
	php
	ldx #216 ;hides 'cld'
	lda #94
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5127)
	; 13 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 11
	inx
	Jne *-8
	plp
.elseif(n = 5128)
	; 10 bytes
	php
	ldx #0
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 5129)
	; 13 bytes
	php
	clc
	lda #74 ;hides 'lsr a'
	ldx #(n-26)/21
	nop
	dex
	Jne *-2
	Jcc *-7
	plp
.elseif(n = 5130)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 5131)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5132)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5133)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5134)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5135)
	; 12 bytes
	php
	pha
	ldx #0
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	pla
	plp
.elseif(n = 5136)
	; 12 bytes
	php
	lda #$0A ;hides 'asl a'
	delay_n 1021
	Jpl *-7
	plp
.endif
.else
; 5137 to 5156
.if(n = 5137)
	; 12 bytes
	php
	ldx #22
	lda #191
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5138)
	; 11 bytes
	php
	ldx #94
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5139)
	; 12 bytes
	php
	ldy #186 ;hides 'tsx'
	lda #53
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	plp
.elseif(n = 5140 || n = 5143)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5141)
	; 11 bytes
	php
	ldx #177
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5142)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5144)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #170
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5145)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5146)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5147)
	; 12 bytes
	php
	ldx #(n-579)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5148)
	; 12 bytes
	php
	ldx #(n-265)/257
	lda #225
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5149)
	; 12 bytes
	php
	ldx #96
	lda #21
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5150)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	ldy #126
	dey
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 5151)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	nop
	jsr @nearby_rts
	dex
	Jne *-6
	plp
.elseif(n = 5152)
	; 11 bytes
	php
	ldx #90 ;hides 'nop'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-6
	plp
.elseif(n = 5153)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5154)
	; 10 bytes
	php
	ldx #(n-6)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5155)
	; 12 bytes
	php
	ldy #166 ;hides 'ldx zp'
	nop
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5156)
	; 11 bytes
	php
	ldx #117
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
.if((n >= 5157) && (n < 5176))
; 5157 to 5175
.if(n = 5157 || n = 5175)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5158)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5159)
	; 12 bytes
	php
	ldx #(n-8)/101
	lda #69
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5160)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5161)
	; 12 bytes
	php
	pha
	ldx #(n-13)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	pla
	plp
.elseif(n = 5162 || n = 5169)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5163)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #157
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5164)
	; 13 bytes
	php
	ldx #229 ;hides 'sbc zp'
	nop
	lda #156
	jsr delay_a_25_clocks
	inx
	Jne *-8
	plp
.elseif(n = 5165)
	; 12 bytes
	php
	ldx #(n-199)/191
	lda #159
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5166)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #181
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5167)
	; 12 bytes
	php
	ldx #76
	lda #35
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5168)
	; 11 bytes
	php
	ldy #39
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 5170)
	; 11 bytes
	php
	ldx #178
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5171)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5172)
	; 10 bytes
	php
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 5173)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
	plp
.elseif(n = 5174)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.endif
.else
; 5176 to 5195
.if(n = 5176)
	; 12 bytes
	php
	clc
	lda #$2A ;hides 'rol a'
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	Jcc *-6
	plp
.elseif(n = 5177)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5178)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5179)
	; 12 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	pla
	plp
.elseif(n = 5180)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #165
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5181)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #173
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5182)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5183)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5184)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #171
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 5185)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5186)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #36
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5187 || n = 5194)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5188)
	; 12 bytes
	php
	ldx #148
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5189)
	; 12 bytes
	php
	ldx #157
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5190)
	; 11 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5191)
	; 12 bytes
	php
	ldx #(n-79)/71
	lda #39
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5192)
	; 11 bytes
	php
	ldx #95
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5193)
	; 12 bytes
	php
	ldx #(n-69)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5195)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 5196) && (n < 5215))
; 5196 to 5214
.if(n = 5196)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5197 || n = 5201 || n = 5202)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5198)
	; 12 bytes
	php
	ldx #30
	lda #141
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5199)
	; 11 bytes
	php
	ldx #179
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5200)
	; 11 bytes
	php
	ldx #118
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5203)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5204)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #172
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5205)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	plp
.elseif(n = 5206)
	; 12 bytes
	php
	ldx #(n-8)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jne *-6
	plp
.elseif(n = 5207)
	; 13 bytes
	php
	pha
	ldx #118
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	pla
	plp
.elseif(n = 5208)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5209)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jmi *-7
	plp
.elseif(n = 5210)
	; 11 bytes
	php
	ldx #17
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5211)
	; 12 bytes
	php
	ldx #(n-129)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5212)
	; 12 bytes
	php
	ldx #202 ;hides 'dex'
	lda #103
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5213)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5214)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.endif
.else
; 5215 to 5234
.if(n = 5215)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5216)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5217)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5218)
	; 12 bytes
	php
	ldx #(n-529)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5219)
	; 12 bytes
	php
	ldx #26
	lda #161
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5220)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #203
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5221)
	; 12 bytes
	php
	ldx #(n-409)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5222)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5223)
	; 12 bytes
	php
	ldx #149
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5224)
	; 12 bytes
	php
	ldx #163
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5225)
	; 12 bytes
	php
	ldx #141
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5226)
	; 12 bytes
	php
	ldx #90 ;hides 'nop'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dex
	Jne *-7
	plp
.elseif(n = 5227)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	plp
.elseif(n = 5228)
	; 11 bytes
	php
	ldx #180
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5229)
	; 12 bytes
	php
	ldx #22
	lda #195
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5230)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5231)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #175
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5232)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	lda #5
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5233)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5234)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
.if((n >= 5235) && (n < 5254))
; 5235 to 5253
.if(n = 5235)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	jsr @nearby_rts
	inx
	Jne *-7
	plp
.elseif(n = 5236)
	; 12 bytes
	php
	ldx #10 ;hides 'asl a'
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-7
	plp
.elseif(n = 5237)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5238)
	; 11 bytes
	php
	ldx #(n-6)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5239)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5240)
	; 12 bytes
	php
	ldx #48
	lda #77
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5241)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5242)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #204
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5243)
	; 13 bytes
	php
	ldx #69 ;hides 'eor zp'
	nop
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	inx
	Jne *-8
	plp
.elseif(n = 5244)
	; 11 bytes
	php
	ldx #118
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5245)
	; 13 bytes
	php
	ldx #229 ;hides 'sbc zp'
	nop
	lda #159
	jsr delay_a_25_clocks
	inx
	Jne *-8
	plp
.elseif(n = 5246)
	; 11 bytes
	php
	ldx #96
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5247)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5248)
	; 12 bytes
	php
	ldx #131
	lda #n/656
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5249)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #15
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5250)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-7
	plp
.elseif(n = 5251)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5252)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5253)
	; 12 bytes
	php
	ldx #136 ;hides 'dey'
	ldy #114
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.endif
.else
; 5254 to 5273
.if(n = 5254)
	; 12 bytes
	php
	ldx #(n-8)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5255)
	; 12 bytes
	php
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-7
	plp
.elseif(n = 5256)
	; 10 bytes
	php
	ldx #(n-6)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5257)
	; 11 bytes
	php
	ldx #181
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5258)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5259 || n = 5269)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5260)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5261)
	; 12 bytes
	php
	ldx #51
	lda #71
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5262)
	; 12 bytes
	php
	ldx #142
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5263)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5264)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5265)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #149
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5266)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5267)
	; 13 bytes
	php
	clc
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #149
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5268)
	; 12 bytes
	php
	ldx #19
	lda #231
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5270)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #60
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5271)
	; 12 bytes
	php
	ldx #19
	lda #245
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5272)
	; 12 bytes
	php
	ldx #(n-8)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dex
	Jne *-6
	plp
.elseif(n = 5273)
	; 12 bytes
	php
	ldx #135
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 5274) && (n < 5293))
; 5274 to 5292
.if(n = 5274)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5275)
	; 12 bytes
	php
	ldx #22
	lda #197
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5276)
	; 11 bytes
	php
	ldy #170 ;hides 'tax'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	plp
.elseif(n = 5277)
	; 12 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	jsr @nearby_rts
	inx
	Jne *-7
	plp
.elseif(n = 5278)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5279)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5280 || n = 5285 || n = 5290 || n = 5292)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5281)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #177
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5282)
	; 12 bytes
	php
	ldx #(n-594)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5283)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5284)
	; 11 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
	plp
.elseif(n = 5286)
	; 11 bytes
	php
	ldx #182
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5287)
	; 12 bytes
	php
	ldx #165 ;hides 'lda zp'
	nop
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-7
	plp
.elseif(n = 5288)
	; 11 bytes
	php
	ldx #120
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5289 || n = 5291)
	; 12 bytes
	pha
	delay_n (n-3)
.endif
.else
; 5293 to 5312
.if(n = 5293)
	; 12 bytes
	php
	ldx #151
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5294)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #175
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5295)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dex
	Jpl *-7
	plp
.elseif(n = 5296)
	; 11 bytes
	php
	ldx #26 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 5297)
	; 11 bytes
	php
	ldy #40
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 5298)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5299)
	; 12 bytes
	php
	ldx #143
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5300)
	; 11 bytes
	php
	ldx #97
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5301)
	; 12 bytes
	php
	ldx #78
	lda #35
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5302)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5303)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5304)
	; 12 bytes
	php
	ldx #(n-8)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5305)
	; 12 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 5306)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #178
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5307)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #59
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5308)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5309)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5310)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5311)
	; 12 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	plp
.elseif(n = 5312)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
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
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5314 || n = 5317 || n = 5323 || n = 5329)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5315)
	; 11 bytes
	php
	ldx #183
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5316)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	lda #11
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5318)
	; 12 bytes
	php
	ldx #89
	lda #27
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5319)
	; 11 bytes
	php
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
	plp
.elseif(n = 5320)
	; 12 bytes
	php
	ldx #166
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5321)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5322)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5324)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #176
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5325)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #163
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5326)
	; 12 bytes
	php
	ldx #152 ;hides 'tya'
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5327)
	; 12 bytes
	php
	ldx #26
	lda #165
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5328)
	; 12 bytes
	php
	ldx #(n-64)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5330)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #208
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5331)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #179
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.endif
.else
; 5332 to 5351
.if(n = 5332)
	; 11 bytes
	php
	ldx #120
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5333)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5334 || n = 5346)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5335)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #151
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5336)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5337)
	; 12 bytes
	php
	ldx #72
	lda #41
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5338)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5339)
	; 13 bytes
	php
	pha
	ldx #120
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	pla
	plp
.elseif(n = 5340)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5341)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	jsr @nearby_rts
	inx
	Jne *-7
	plp
.elseif(n = 5342)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5343)
	; 12 bytes
	php
	ldx #96
	lda #23
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5344)
	; 11 bytes
	php
	ldx #184
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5345)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5347)
	; 12 bytes
	php
	ldx #(n-8)/281
	lda #249
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5348)
	; 12 bytes
	php
	ldx #89
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jne *-6
	plp
.elseif(n = 5349)
	; 12 bytes
	php
	ldx #48
	lda #77
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5350)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	ldy #132
	dey
	Jne *-1
	inx
	Jne *-7
	plp
.elseif(n = 5351)
	; 12 bytes
	php
	ldx #137
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 5352) && (n < 5371))
; 5352 to 5370
.if(n = 5352)
	; 12 bytes
	php
	ldx #167
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5353 || n = 5365 || n = 5369)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5354)
	; 11 bytes
	php
	ldx #98
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5355)
	; 13 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	inx
	Jpl *-8
	plp
.elseif(n = 5356)
	; 12 bytes
	php
	ldx #(n-199)/191
	lda #159
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5357)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5358)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5359)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
	plp
.elseif(n = 5360)
	; 12 bytes
	php
	ldx #24
	lda #191
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5361)
	; 12 bytes
	php
	ldx #(n-8)/101
	lda #69
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5362)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #172
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5363)
	; 11 bytes
	php
	ldx #(n-8)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5364)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5366)
	; 12 bytes
	php
	ldx #141
	lda #6
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5367)
	; 12 bytes
	php
	ldx #22
	lda #201
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5368)
	; 12 bytes
	php
	ldx #134
	lda #8
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5370)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #152
	dex
	Jne *-1
	Jcs *-7
	plp
.endif
.else
; 5371 to 5390
.if(n = 5371)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5372)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5373)
	; 11 bytes
	php
	ldx #185
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5374)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #210
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5375 || n = 5378 || n = 5386)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5376)
	; 11 bytes
	php
	ldx #121
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5377)
	; 12 bytes
	php
	ldx #90
	lda #27
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5379)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5380)
	; 12 bytes
	php
	ldx #158
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5381)
	; 12 bytes
	php
	ldx #26
	lda #167
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5382)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #62
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5383)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5384)
	; 11 bytes
	php
	ldy #13
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	plp
.elseif(n = 5385)
	; 12 bytes
	php
	ldx #19
	lda #251
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5387)
	; 12 bytes
	php
	ldx #163
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5388)
	; 11 bytes
	php
	ldx #(n-6)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 5389)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	plp
.elseif(n = 5390)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
.if((n >= 5391) && (n < 5410))
; 5391 to 5409
.if(n = 5391 || n = 5399 || n = 5401)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5392 || n = 5404)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5393)
	; 13 bytes
	php
	clc
	lda #$4A ;hides 'lsr a'
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	Jcc *-7
	plp
.elseif(n = 5394)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #178
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 5395)
	; 13 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	cmp $EA ;hides 'nop'
	delay_n 4
	pla
	pha
	dex
	Jne *-8
	plp
.elseif(n = 5396)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #211
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5397)
	; 13 bytes
	php
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	lda #$4A ;hides 'lsr a'
	Jne *-1
	inx
	inx
	Jne *-8
	plp
.elseif(n = 5398)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5400)
	; 12 bytes
	php
	ldx #58 ;hides 'nop'
	lda #59
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5402)
	; 11 bytes
	php
	ldx #186
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5403)
	; 12 bytes
	php
	ldx #83
	lda #33
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5405)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #153
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5406)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	inx
	Jne *-7
	plp
.elseif(n = 5407)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $1FA2'
	ldx #31
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	plp
.elseif(n = 5408)
	; 11 bytes
	php
	ldx #100
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 5409)
	; 12 bytes
	php
	ldx #(n-499)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.endif
.else
; 5410 to 5429
.if(n = 5410)
	; 12 bytes
	php
	ldx #146
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5411)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 5412)
	; 12 bytes
	php
	ldx #27
	lda #161
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5413)
	; 12 bytes
	php
	ldx #46
	lda #83
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5414)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #179
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5415)
	; 13 bytes
	php
	clc
	lda #233 ;hides 'sbc #$F8'
	sed ;first loop only
	ldx #(n-40)/25
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5416)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5417)
	; 12 bytes
	php
	ldx #(n-609)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5418)
	; 10 bytes
	php
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 5419)
	; 12 bytes
	php
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5420)
	; 11 bytes
	php
	ldx #122
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5421)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5422)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5423)
	; 12 bytes
	php
	ldx #94
	lda #25
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5424)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #29
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 5425)
	; 12 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	pla
	plp
.elseif(n = 5426)
	; 11 bytes
	php
	ldy #42
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.elseif(n = 5427)
	; 13 bytes
	php
	pha
	ldx #122
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	pla
	plp
.elseif(n = 5428)
	; 12 bytes
	php
	ldx #(n-279)/271
	lda #239
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5429)
	; 12 bytes
	php
	ldx #139
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 5430) && (n < 5449))
; 5430 to 5448
.if(n = 5430)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #192
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5431)
	; 11 bytes
	php
	ldx #187
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5432)
	; 12 bytes
	php
	ldx #48
	lda #81
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5433)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5434)
	; 11 bytes
	php
	ldx #138 ;hides 'txa'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	inx
	Jne *-6
	plp
.elseif(n = 5435)
	; 12 bytes
	php
	ldx #81
	lda #35
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5436)
	; 12 bytes
	php
	ldx #(n-8)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jne *-6
	plp
.elseif(n = 5437)
	; 12 bytes
	php
	ldx #(n-8)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5438)
	; 12 bytes
	php
	ldx #30
	lda #149
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5439)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5440)
	; 12 bytes
	php
	ldx #(n-64)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5441)
	; 13 bytes
	php
	pha
	ldx #138 ;hides 'txa'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	inx
	Jne *-6
	pla
	plp
.elseif(n = 5442)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5443)
	; 13 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	lda #116
	jsr delay_a_25_clocks
	dex
	Jne *-8
	plp
.elseif(n = 5444)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5445)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5446)
	; 12 bytes
	php
	ldx #216 ;hides 'cld'
	lda #102
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5447)
	; 12 bytes
	php
	ldx #147
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5448)
	; 12 bytes
	php
	ldx #170
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.endif
.else
; 5449 to 5468
.if(n = 5449 || n = 5450 || n = 5466)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5451)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	lda #11
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5452)
	; 13 bytes
	php
	ldx #100 ;hides 'nop zp'
	dex ;first loop only
	lda #20
	jsr delay_a_25_clocks
	dex
	Jne *-8
	plp
.elseif(n = 5453)
	; 12 bytes
	php
	ldx #(n-8)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5454)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #193
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5455)
	; 12 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 5456)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #135
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5457)
	; 11 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 5458)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5459)
	; 12 bytes
	php
	ldx #78
	lda #37
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5460)
	; 11 bytes
	php
	ldx #188
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5461)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5462)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5463)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #17
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5464)
	; 11 bytes
	php
	ldx #124
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5465)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5467)
	; 12 bytes
	php
	ldx #53
	lda #71
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5468)
	; 12 bytes
	php
	ldx #90
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jpl *-6
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
.if((n >= 5469) && (n < 5508))
.if((n >= 5469) && (n < 5488))
; 5469 to 5487
.if(n = 5469)
	; 12 bytes
	php
	ldx #42
	lda #95
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5470)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	ldy #135
	dey
	Jne *-1
	inx
	Jne *-7
	plp
.elseif(n = 5471)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5472)
	; 13 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	nop
	ldx #135
	dex
	Jne *-1
	Jcc *-8
	plp
.elseif(n = 5473)
	; 13 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	lda #35
	jsr delay_a_25_clocks
	inx
	Jpl *-8
	plp
.elseif(n = 5474)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #181
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5475)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #155
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5476)
	; 13 bytes
	php
	clc
	clc
	lda #$0A ;hides 'asl a'
	ldx #181
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5477)
	; 13 bytes
	php
	clc
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #155
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5478)
	; 12 bytes
	php
	ldx #152 ;hides 'tya'
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5479)
	; 12 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 5480)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5481)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #185
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5482)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5483)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5484)
	; 12 bytes
	php
	ldx #148
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5485)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5486)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5487)
	; 11 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	plp
.endif
.else
; 5488 to 5507
.if(n = 5488)
	; 12 bytes
	php
	ldx #137
	lda #8
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5489)
	; 11 bytes
	php
	ldx #189
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5490)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5491 || n = 5497 || n = 5501)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5492)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #177
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5493)
	; 12 bytes
	php
	ldx #58 ;hides 'nop'
	lda #59
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5494)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5495)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5496)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #136
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5498)
	; 12 bytes
	php
	ldx #(n-69)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5499)
	; 11 bytes
	php
	ldx #19
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 5500)
	; 12 bytes
	php
	ldx #168 ;hides 'tay'
	lda #100
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5502)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5503)
	; 12 bytes
	php
	ldx #157
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5504)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #182
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5505)
	; 12 bytes
	php
	ldx #22
	lda #207
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5506)
	; 10 bytes
	php
	ldx #(n-6)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5507)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 5508) && (n < 5527))
; 5508 to 5526
.if(n = 5508)
	; 11 bytes
	php
	ldx #125
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5509 || n = 5511)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5510)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5512)
	; 12 bytes
	php
	ldx #172
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5513)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5514)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #68
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 5515)
	; 13 bytes
	php
	pha
	ldx #125
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	pla
	plp
.elseif(n = 5516)
	; 11 bytes
	php
	ldx #101
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5517)
	; 12 bytes
	php
	ldx #(n-5)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
	plp
.elseif(n = 5518)
	; 11 bytes
	php
	ldx #190
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5519)
	; 12 bytes
	php
	ldx #167
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5520)
	; 11 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldy #136 ;hides 'dey'
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5521)
	; 12 bytes
	php
	ldx #149
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5522)
	; 12 bytes
	php
	sec
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldy #136 ;hides 'dey'
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5523)
	; 12 bytes
	php
	ldx #136 ;hides 'dey'
	ldy #121
	dey
	Jne *-1
	dex
	Jmi *-7
	plp
.elseif(n = 5524)
	; 12 bytes
	php
	ldx #216 ;hides 'cld'
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jmi *-7
	plp
.elseif(n = 5525)
	; 13 bytes
	php
	pha
	ldx #190
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	pla
	plp
.elseif(n = 5526)
	; 11 bytes
	php
	ldx #136 ;hides 'dey'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	inx
	Jne *-6
	plp
.endif
.else
; 5527 to 5546
.if(n = 5527)
	; 12 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	plp
.elseif(n = 5528)
	; 12 bytes
	php
	ldx #91
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5529)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 5530)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5531)
	; 12 bytes
	php
	ldx #(n-11)/276
	lda #231
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5532)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5533)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5534)
	; 11 bytes
	php
	ldx #248 ;hides 'sed'
	ldy #136 ;hides 'dey'
	Jne *-1
	inx
	Jne *-6
	plp
.elseif(n = 5535)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #63
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5536)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #137
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5537)
	; 12 bytes
	php
	ldx #96
	lda #25
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5538)
	; 12 bytes
	php
	ldx #158
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5539)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	ldy #98
	dey
	Jpl *-1
	dex
	Jmi *-7
	plp
.elseif(n = 5540)
	; 12 bytes
	php
	ldx #(n-8)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
	plp
.elseif(n = 5541)
	; 12 bytes
	php
	ldx #168 ;hides 'tay'
	lda #101
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5542)
	; 13 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	lda #10
	jsr delay_a_25_clocks
	inx
	Jpl *-8
	plp
.elseif(n = 5543)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5544)
	; 12 bytes
	php
	ldx #173
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5545)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #157
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5546)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 5547) && (n < 5586))
.if((n >= 5547) && (n < 5566))
; 5547 to 5565
.if(n = 5547)
	; 11 bytes
	php
	ldx #191
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5548)
	; 12 bytes
	php
	ldx #(n-19)/291
	lda #245
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5549 || n = 5554 || n = 5560 || n = 5561)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5550)
	; 11 bytes
	php
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	plp
.elseif(n = 5551)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.elseif(n = 5552)
	; 11 bytes
	php
	ldx #125
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5553)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #95
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5555)
	; 11 bytes
	php
	ldy #43
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.elseif(n = 5556)
	; 12 bytes
	php
	ldx #146
	lda #(n-810)/791
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5557)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5558)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5559)
	; 12 bytes
	php
	ldx #(n-69)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5562)
	; 13 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	lda #20
	jsr delay_a_25_clocks
	dex
	Jmi *-8
	plp
.elseif(n = 5563)
	; 12 bytes
	php
	ldx #(n-8)/101
	lda #69
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5564)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #184
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5565)
	; 13 bytes
	php
	clc
	lda #233 ;hides 'sbc #$F8'
	sed ;first loop only
	ldx #(n-40)/25
	dex
	Jne *-1
	Jcc *-7
	plp
.endif
.else
; 5566 to 5585
.if(n = 5566)
	; 12 bytes
	php
	ldx #216 ;hides 'cld'
	lda #105
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5567)
	; 12 bytes
	php
	ldx #(n-161)/106
	lda #77
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5568)
	; 12 bytes
	php
	ldx #139
	lda #(n-640)/616
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5569)
	; 12 bytes
	php
	ldx #83
	lda #35
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5570)
	; 11 bytes
	php
	ldx #102
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5571)
	; 12 bytes
	php
	ldy #232 ;hides 'inx'
	ldx #8
	dex
	Jpl *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5572)
	; 11 bytes
	php
	ldx #248 ;hides 'sed'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jmi *-6
	plp
.elseif(n = 5573)
	; 12 bytes
	php
	ldx #159
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5574)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5575)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	plp
.elseif(n = 5576)
	; 11 bytes
	php
	ldx #192
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5577 || n = 5579)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5578)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5580)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #158
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5581)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #189
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5582)
	; 12 bytes
	php
	ldx #136 ;hides 'dey'
	ldy #136 ;hides 'dey'
	dey
	Jmi *-2
	dex
	Jne *-7
	plp
.elseif(n = 5583)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5584)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5585)
	; 12 bytes
	php
	ldx #143
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 5586) && (n < 5606))
; 5586 to 5605
.if(n = 5586)
	; 12 bytes
	php
	ldx #10 ;hides 'asl a'
	ldy #110
	dey
	Jne *-1
	dex
	Jne *-7
	plp
.elseif(n = 5587 || n = 5602)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5588)
	; 12 bytes
	php
	ldx #93
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jne *-6
	plp
.elseif(n = 5589)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5590)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	ldy #138
	dey
	Jne *-1
	inx
	Jne *-7
	plp
.elseif(n = 5591)
	; 13 bytes
	php
	ldx #<$9D ;hides 'sta $13A2,X'
	ldx #19
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jne *-8
	plp
.elseif(n = 5592)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #64
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5593)
	; 13 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	dex
	Jne *-8
	plp
.elseif(n = 5594)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #185
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5595)
	; 11 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 5596)
	; 11 bytes
	php
	ldx #127
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5597)
	; 12 bytes
	php
	ldx #81
	lda #37
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5598)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5599)
	; 12 bytes
	php
	ldx #233 ;hides 'sbc #imm'
	lda $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
	plp
.elseif(n = 5600)
	; 12 bytes
	php
	ldx #24
	lda #201
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5601)
	; 12 bytes
	php
	ldx #46
	lda #87
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5603)
	; 13 bytes
	php
	pha
	ldx #127
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	pla
	plp
.elseif(n = 5604)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #185
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 5605)
	; 11 bytes
	php
	ldx #193
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.endif
.else
; 5606 to 5625
.if(n = 5606)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	inx
	Jne *-7
	plp
.elseif(n = 5607 || n = 5609 || n = 5610)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5608)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5611)
	; 12 bytes
	php
	ldx #58 ;hides 'nop'
	lda #61
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5612)
	; 13 bytes
	php
	pha
	ldx #193
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	pla
	plp
.elseif(n = 5613)
	; 12 bytes
	php
	ldx #94
	lda #27
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5614)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5615)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #159
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5616)
	; 12 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 5617)
	; 12 bytes
	php
	ldx #(n-79)/71
	lda #39
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5618)
	; 11 bytes
	php
	ldx #(n-8)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5619)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5620)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5621)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5622)
	; 11 bytes
	php
	ldx #(n-6)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5623)
	; 12 bytes
	php
	ldx #168 ;hides 'tay'
	lda #103
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5624)
	; 11 bytes
	php
	ldx #104
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	plp
.elseif(n = 5625)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
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
	php
	ldx #(n-8)/106
	lda #74
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5627)
	; 12 bytes
	php
	ldx #200 ;hides 'iny'
	lda #43
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5628)
	; 12 bytes
	php
	ldx #(n-289)/281
	lda #249
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5629)
	; 12 bytes
	php
	ldx #76
	lda #41
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5630)
	; 12 bytes
	php
	ldx #152 ;hides 'tya'
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5631)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #191
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5632)
	; 12 bytes
	php
	ldx #152
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5633)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5634)
	; 11 bytes
	php
	ldx #194
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5635)
	; 12 bytes
	php
	ldx #(n-339)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5636 || n = 5642)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5637 || n = 5644)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5638)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #222
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5639)
	; 11 bytes
	php
	ldy #170 ;hides 'tax'
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jmi *-6
	plp
.elseif(n = 5640)
	; 11 bytes
	php
	ldx #128
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5641)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 5643)
	; 12 bytes
	php
	ldx #(n-169)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dex
	Jpl *-6
	plp
.endif
.else
; 5645 to 5664
.if(n = 5645 || n = 5653 || n = 5659)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5646)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5647)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	plp
.elseif(n = 5648)
	; 11 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-6
	plp
.elseif(n = 5649)
	; 12 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	nop
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 5650)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5651)
	; 12 bytes
	php
	ldx #171
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5652)
	; 12 bytes
	php
	ldx #166
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5654)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #187
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5655)
	; 13 bytes
	php
	pha
	ldx #74 ;hides 'lsr a'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-6
	pla
	plp
.elseif(n = 5656)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #140
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5657)
	; 12 bytes
	php
	ldx #20
	lda #237
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5658)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5660)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5661)
	; 13 bytes
	php
	ldx #56 ;hides 'sec'
	lda #159
	jsr delay_a_25_clocks
	dex
	dex
	Jpl *-8
	plp
.elseif(n = 5662)
	; 12 bytes
	php
	ldx #(n-265)/257
	lda #225
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5663)
	; 11 bytes
	php
	ldx #195
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5664)
	; 11 bytes
	php
	ldx #10 ;hides 'asl a'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 5665) && (n < 5684))
; 5665 to 5683
.if(n = 5665 || n = 5680)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5666)
	; 12 bytes
	php
	ldx #(n-54)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5667)
	; 12 bytes
	php
	ldx #136 ;hides 'dey'
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	dex
	Jmi *-7
	plp
.elseif(n = 5668)
	; 12 bytes
	php
	ldx #19
	lda #251
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5669)
	; 12 bytes
	php
	ldx #153
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5670)
	; 11 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5671)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5672)
	; 12 bytes
	php
	ldx #177
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5673)
	; 12 bytes
	php
	ldx #55
	lda #71
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5674)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #184
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5675)
	; 13 bytes
	php
	ldy #55
	lda #71
	jsr delay_a_25_clocks
	dey
	Jne *-6
	plp
	nop
.elseif(n = 5676)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5677)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #19
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5678)
	; 11 bytes
	php
	ldx #104
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5679)
	; 12 bytes
	php
	ldx #53
	lda #75
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5681)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5682)
	; 12 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 5683)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.endif
.else
; 5684 to 5703
.if(n = 5684)
	; 11 bytes
	php
	ldx #128
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5685)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #161
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5686 || n = 5698 || n = 5701)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5687)
	; 12 bytes
	php
	ldx #(n-639)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5688)
	; 12 bytes
	php
	ldx #142
	lda #8
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5689)
	; 12 bytes
	php
	ldx #22
	lda #215
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5690)
	; 12 bytes
	php
	ldx #58 ;hides 'nop'
	lda #64
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5691)
	; 13 bytes
	php
	pha
	ldx #128
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jpl *-5
	pla
	plp
.elseif(n = 5692)
	; 11 bytes
	php
	ldx #196
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5693)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	lda #13
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5694)
	; 11 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5695)
	; 12 bytes
	php
	ldx #(n-129)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5696)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5697)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5699)
	; 12 bytes
	php
	ldx #(n-279)/271
	lda #239
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5700)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #185
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5702)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5703)
	; 12 bytes
	php
	ldx #84
	lda #35
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 5704) && (n < 5743))
.if((n >= 5704) && (n < 5723))
; 5704 to 5722
.if(n = 5704)
	; 12 bytes
	php
	ldx #178
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5705)
	; 12 bytes
	php
	ldx #(n-219)/211
	lda #179
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5706)
	; 12 bytes
	php
	ldx #106 ;hides 'ror a'
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	inx
	Jne *-7
	plp
.elseif(n = 5707)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5708)
	; 12 bytes
	php
	ldx #94
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5709)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5710)
	; 11 bytes
	php
	ldx #184 ;hides 'clv'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5711)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	plp
.elseif(n = 5712)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5713)
	; 12 bytes
	php
	ldx #163
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5714)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #189
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5715)
	; 12 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 5716)
	; 13 bytes
	php
	clc
	clc
	lda #$0A ;hides 'asl a'
	ldx #189
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5717)
	; 12 bytes
	php
	ldx #173
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5718)
	; 11 bytes
	php
	ldx #26 ;hides 'nop'
	ldy #136 ;hides 'dey'
	Jmi *-1
	inx
	Jpl *-6
	plp
.elseif(n = 5719)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	plp
.elseif(n = 5720)
	; 12 bytes
	php
	ldx #(n-64)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5721)
	; 11 bytes
	php
	ldx #197
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5722)
	; 13 bytes
	php
	clc
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #162
	dex
	Jne *-1
	Jcs *-7
	plp
.endif
.else
; 5723 to 5742
.if(n = 5723 || n = 5730 || n = 5734 || n = 5737)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5724)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #189
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 5725)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #99
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5726)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #21
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 5727)
	; 12 bytes
	php
	ldx #133
	lda #11
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5728)
	; 11 bytes
	php
	ldx #130
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5729)
	; 12 bytes
	php
	ldy #186 ;hides 'tsx'
	lda #63
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	plp
.elseif(n = 5731)
	; 12 bytes
	php
	ldx #96
	lda #27
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5732)
	; 11 bytes
	php
	ldx #105
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5733)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5735)
	; 12 bytes
	php
	ldx #83
	lda #37
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5736)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #142
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5738)
	; 12 bytes
	php
	ldx #30
	lda #159
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5739)
	; 12 bytes
	php
	ldx #218 ;hides 'nop'
	lda #29
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5740)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	lda #13
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5741)
	; 12 bytes
	php
	ldx #147
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5742)
	; 12 bytes
	php
	ldx #(n-8)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 5743) && (n < 5762))
; 5743 to 5761
.if(n = 5743)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5744)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #190
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5745 || n = 5752 || n = 5761)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5746)
	; 12 bytes
	php
	ldx #151
	lda #6
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5747)
	; 13 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dex
	Jne *-8
	plp
.elseif(n = 5748)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5749)
	; 13 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-8
	plp
.elseif(n = 5750)
	; 11 bytes
	php
	ldx #198
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5751)
	; 13 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #163
	nop
	dex
	Jne *-2
	dey
	Jmi *-8
	plp
.elseif(n = 5753)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5754)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5755)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #163
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5756)
	; 11 bytes
	php
	ldx #(n-6)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	plp
.elseif(n = 5757)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5758)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5759)
	; 12 bytes
	php
	ldx #(n-8)/71
	lda #39
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5760)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #103
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.endif
.else
; 5762 to 5781
.if(n = 5762)
	; 12 bytes
	php
	ldx #137
	lda #10
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5763)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #67
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5764)
	; 12 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jmi *-7
	plp
.elseif(n = 5765)
	; 12 bytes
	php
	ldx #(n-109)/101
	lda #69
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5766)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #46
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 5767)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	plp
.elseif(n = 5768)
	; 11 bytes
	php
	ldy #15
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	plp
.elseif(n = 5769)
	; 13 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	nop
	ldx #163
	dex
	Jne *-1
	Jcs *-8
	plp
.elseif(n = 5770)
	; 12 bytes
	php
	ldx #86
	lda #35
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5771)
	; 12 bytes
	php
	ldx #51
	lda #81
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5772)
	; 11 bytes
	php
	ldx #131
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5773)
	; 12 bytes
	php
	ldx #200 ;hides 'iny'
	lda #45
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5774)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5775)
	; 12 bytes
	php
	ldx #78
	lda #41
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5776)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #143
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5777)
	; 12 bytes
	php
	ldx #(n-649)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5778)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #44
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5779)
	; 11 bytes
	php
	ldx #199
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5780)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5781)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
.if((n >= 5782) && (n < 5821))
.if((n >= 5782) && (n < 5801))
; 5782 to 5800
.if(n = 5782)
	; 12 bytes
	php
	ldx #152 ;hides 'tya'
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-7
	plp
.elseif(n = 5783)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5784)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	inx
	Jne *-7
	plp
.elseif(n = 5785)
	; 12 bytes
	php
	ldx #53
	lda #77
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5786)
	; 11 bytes
	php
	ldx #106
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5787)
	; 12 bytes
	php
	ldx #168 ;hides 'tay'
	lda #107
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5788)
	; 11 bytes
	php
	ldx #19
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5789)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5790)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #164
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5791)
	; 12 bytes
	php
	ldx #216 ;hides 'cld'
	lda #31
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5792)
	; 12 bytes
	php
	ldx #(n-8)/241
	lda #209
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5793)
	; 12 bytes
	php
	ldx #89
	lda #33
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5794)
	; 12 bytes
	php
	ldx #21
	lda #231
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5795)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5796)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5797 || n = 5799)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5798)
	; 12 bytes
	php
	ldx #30
	lda #161
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5800)
	; 12 bytes
	php
	ldx #181
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.endif
.else
; 5801 to 5820
.if(n = 5801)
	; 13 bytes
	php
	ldx #248 ;hides 'sed'
	lda #59
	jsr delay_a_25_clocks
	dex
	dex
	Jmi *-8
	plp
.elseif(n = 5802)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	lda #8
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5803)
	; 11 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5804)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5805)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5806)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5807 || n = 5810 || n = 5812 || n = 5815 || n = 5818)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5808)
	; 11 bytes
	php
	ldx #200
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5809)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5811)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #101
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5813)
	; 11 bytes
	php
	ldy #45
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.elseif(n = 5814)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	dex
	Jmi *-7
	plp
.elseif(n = 5816)
	; 11 bytes
	php
	ldx #132
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5817)
	; 12 bytes
	php
	ldx #157
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5819)
	; 12 bytes
	php
	ldx #149
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5820)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #68
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.endif
.endif
.else
.if((n >= 5821) && (n < 5840))
; 5821 to 5839
.if(n = 5821)
	; 12 bytes
	php
	ldy #136 ;hides 'dey'
	ldx #(n-46)/25
	dex
	Jne *-1
	dey
	Jmi *-7
	plp
.elseif(n = 5822)
	; 12 bytes
	php
	ldx #171
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5823)
	; 13 bytes
	php
	pha
	ldx #132
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	pla
	plp
.elseif(n = 5824)
	; 13 bytes
	php
	lda #105 ;hides 'adc #$18'
	clc ;first loop only
	ldx #n/56-2
	nop
	dex
	Jpl *-2
	Jcc *-8
	plp
.elseif(n = 5825)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #165
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5826)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	plp
.elseif(n = 5827)
	; 12 bytes
	php
	ldx #22
	lda #221
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5828)
	; 12 bytes
	php
	ldx #96
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5829 || n = 5839)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5830)
	; 11 bytes
	php
	ldx #24 ;hides 'clc'
	ldy #136 ;hides 'dey'
	Jmi *-1
	inx
	Jpl *-6
	plp
.elseif(n = 5831)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #199
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5832)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5833)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5834)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #193
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5835)
	; 12 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	lda #$4A ;hides 'lsr a'
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 5836)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5837)
	; 11 bytes
	php
	ldx #201
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5838)
	; 11 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.endif
.else
; 5840 to 5859
.if(n = 5840)
	; 11 bytes
	php
	ldx #107
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5841 || n = 5843)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5842 || n = 5851)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5844)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #105
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5845)
	; 13 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	ldy #138 ;hides 'txa'
	dey
	Jne *-2
	dex
	Jpl *-8
	plp
.elseif(n = 5846)
	; 12 bytes
	php
	ldx #139
	lda #10
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5847)
	; 12 bytes
	php
	ldx #58 ;hides 'nop'
	lda #65
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5848)
	; 12 bytes
	php
	ldx #146
	lda #n/731
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5849)
	; 12 bytes
	php
	ldx #177
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5850)
	; 12 bytes
	php
	ldx #(n-8)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jne *-6
	plp
.elseif(n = 5852)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #45
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5853)
	; 12 bytes
	php
	ldx #167
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5854)
	; 12 bytes
	php
	ldx #158
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5855)
	; 13 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	lda $EA ;hides 'nop'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-8
	plp
.elseif(n = 5856)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #145
	dex
	Jne *-1
	Jcc *-7
	plp
.elseif(n = 5857)
	; 13 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	lda #30
	jsr delay_a_25_clocks
	inx
	Jne *-8
	plp
.elseif(n = 5858)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5859)
	; 13 bytes
	php
	ldx #165 ;hides 'lda zp'
	nop
	lda #119
	jsr delay_a_25_clocks
	dex
	Jmi *-8
	plp
.endif
.endif
.endif
.else
.if((n >= 5860) && (n < 5899))
.if((n >= 5860) && (n < 5879))
; 5860 to 5878
.if(n = 5860)
	; 11 bytes
	php
	ldx #133
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5861)
	; 13 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	jsr @nearby_rts
	dex
	Jne *-8
	plp
.elseif(n = 5862)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5863)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5864)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #194
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5865)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #183
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5866)
	; 11 bytes
	php
	ldx #202
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5867)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5868 || n = 5875)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5869)
	; 12 bytes
	php
	ldx #168 ;hides 'tay'
	ldy #27
	dey
	Jne *-1
	dex
	Jmi *-7
	plp
.elseif(n = 5870)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	ldy #145
	dey
	Jne *-1
	inx
	Jne *-7
	plp
.elseif(n = 5871)
	; 12 bytes
	php
	ldx #(n-8)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-6
	plp
.elseif(n = 5872)
	; 12 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	plp
.elseif(n = 5873)
	; 12 bytes
	php
	ldx #84
	lda #37
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5874)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #194
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 5876)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5877)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	lda #69
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5878)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	inx
	Jne *-7
	plp
.endif
.else
; 5879 to 5898
.if(n = 5879)
	; 12 bytes
	php
	ldx #56
	lda #71
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5880)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #233
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5881)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #201
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5882)
	; 12 bytes
	php
	ldx #178
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5883)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5884)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5885)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5886)
	; 11 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5887)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	plp
.elseif(n = 5888)
	; 12 bytes
	php
	ldx #97
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5889)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5890)
	; 12 bytes
	php
	ldx #173
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5891)
	; 12 bytes
	php
	ldx #159
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5892)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #75
	jsr delay_a_25_clocks
	inx
	Jpl *-7
	plp
.elseif(n = 5893)
	; 12 bytes
	php
	ldx #55
	lda #75
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5894)
	; 11 bytes
	php
	ldx #108
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 5895)
	; 11 bytes
	php
	ldx #203
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5896)
	; 12 bytes
	php
	ldx #(n-8)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dex
	Jne *-6
	plp
.elseif(n = 5897)
	; 12 bytes
	php
	ldx #151
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5898)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 5899) && (n < 5918))
; 5899 to 5917
.if(n = 5899)
	; 12 bytes
	php
	ldx #137
	lda #(n-3)/536
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5900)
	; 12 bytes
	php
	ldx #(n-8)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5901)
	; 12 bytes
	php
	ldx #(n-8)/71
	lda #39
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5902)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #234
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5903)
	; 12 bytes
	php
	ldx #(n-8)/131
	lda #99
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5904)
	; 11 bytes
	php
	ldx #134
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5905)
	; 12 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	plp
.elseif(n = 5906 || n = 5914)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5907)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5908)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5909)
	; 12 bytes
	php
	ldx #(n-289)/281
	lda #249
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5910)
	; 11 bytes
	php
	ldx #10 ;hides 'asl a'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
	plp
.elseif(n = 5911)
	; 12 bytes
	php
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	plp
.elseif(n = 5912)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5913)
	; 12 bytes
	php
	ldx #232 ;hides 'inx'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	inx
	Jne *-7
	plp
.elseif(n = 5915)
	; 12 bytes
	php
	ldx #179
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5916)
	; 12 bytes
	php
	ldx #(n-219)/211
	lda #179
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5917)
	; 12 bytes
	php
	ldx #(n-8)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
	plp
.endif
.else
; 5918 to 5937
.if(n = 5918)
	; 12 bytes
	php
	ldx #30
	lda #165
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5919)
	; 12 bytes
	php
	ldx #(n-265)/257
	lda #225
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5920)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldy #232 ;hides 'inx'
	dey
	Jmi *-2
	Jcc *-7
	plp
.elseif(n = 5921)
	; 12 bytes
	php
	ldx #81
	lda #41
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5922)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	inx
	Jpl *-7
	plp
.elseif(n = 5923)
	; 12 bytes
	php
	ldx #169
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5924)
	; 11 bytes
	php
	ldx #204
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5925)
	; 12 bytes
	php
	ldx #(n-69)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5926)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5927)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5928)
	; 12 bytes
	php
	ldx #185
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5929)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5930)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #168
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5931)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #203
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5932)
	; 13 bytes
	php
	clc
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #168
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 5933)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5934)
	; 12 bytes
	php
	ldx #152 ;hides 'tya'
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5935)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	lda #15
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5936)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5937)
	; 12 bytes
	php
	ldx #(n-129)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dex
	Jpl *-6
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
.if((n >= 5938) && (n < 5977))
.if((n >= 5938) && (n < 5957))
; 5938 to 5956
.if(n = 5938)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5939 || n = 5950 || n = 5955)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5940)
	; 12 bytes
	php
	ldx #138 ;hides 'txa'
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dex
	Jne *-7
	plp
.elseif(n = 5941)
	; 13 bytes
	php
	ldx #69 ;hides 'eor zp'
	nop
	lda #51
	jsr delay_a_25_clocks
	dex
	Jne *-8
	plp
.elseif(n = 5942)
	; 11 bytes
	php
	ldy #45
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	plp
.elseif(n = 5943)
	; 13 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	lda #177
	jsr delay_a_25_clocks
	inx
	Jne *-8
	plp
.elseif(n = 5944)
	; 12 bytes
	php
	ldx #(n-64)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5945)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5946)
	; 12 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-7
	plp
.elseif(n = 5947)
	; 13 bytes
	php
	ldx #165 ;hides 'lda zp'
	nop
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-8
	plp
.elseif(n = 5948)
	; 11 bytes
	php
	ldx #135
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 5949)
	; 13 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #247
	jsr delay_a_25_clocks
	dex
	dex
	Jne *-8
	plp
.elseif(n = 5951)
	; 12 bytes
	php
	ldx #20
	lda #251
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5952)
	; 13 bytes
	php
	tya
	ldx #110
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	plp
.elseif(n = 5953)
	; 11 bytes
	php
	ldx #205
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5954)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #197
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 5956)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #204
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.endif
.else
; 5957 to 5976
.if(n = 5957)
	; 12 bytes
	php
	ldx #(n-669)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5958)
	; 11 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	plp
.elseif(n = 5959)
	; 12 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 5960)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 5961)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5962)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5963)
	; 12 bytes
	php
	ldx #4
	ldy #(n-38)/25
	dey
	Jne *-1
	dex
	Jpl *-6
	plp
.elseif(n = 5964)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	ldy #197
	dey
	Jne *-1
	inx
	Jpl *-7
	plp
.elseif(n = 5965)
	; 12 bytes
	php
	ldx #(n-169)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dex
	Jpl *-6
	plp
.elseif(n = 5966)
	; 12 bytes
	php
	ldx #216 ;hides 'cld'
	lda #115
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5967)
	; 12 bytes
	php
	ldx #(n-109)/101
	lda #69
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5968)
	; 12 bytes
	php
	ldx #149
	lda #8
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5969)
	; 12 bytes
	php
	ldx #216 ;hides 'cld'
	lda #33
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 5970)
	; 12 bytes
	php
	ldx #(n-279)/271
	lda #239
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5971)
	; 12 bytes
	php
	ldx #89
	lda #35
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5972)
	; 12 bytes
	php
	ldx #(n-8)/71
	lda #39
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5973)
	; 12 bytes
	php
	ldx #26 ;hides 'nop'
	lda #187
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 5974)
	; 12 bytes
	php
	ldx #157
	lda #6
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5975)
	; 12 bytes
	php
	ldx #153
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5976)
	; 12 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #148
	dex
	Jne *-1
	Jcc *-7
	plp
.endif
.endif
.else
.if((n >= 5977) && (n < 5996))
; 5977 to 5995
.if(n = 5977)
	; 12 bytes
	php
	ldx #46
	lda #95
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5978)
	; 12 bytes
	php
	ldx #30
	lda #167
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5979)
	; 12 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	nop
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-7
	plp
.elseif(n = 5980)
	; 12 bytes
	php
	ldx #58 ;hides 'nop'
	lda #69
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 5981)
	; 12 bytes
	php
	ldx #181
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5982)
	; 11 bytes
	php
	ldx #206
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 5983)
	; 12 bytes
	php
	ldx #(n-8)/25
	lda #42 ;hides 'rol a'
	clc
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 5984)
	; 12 bytes
	php
	ldx #(n-8)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5985)
	; 12 bytes
	php
	ldx #139
	lda #11
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 5986)
	; 12 bytes
	php
	ldx #(n-69)/61
	lda #29
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 5987 || n = 5994)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5988)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 5989)
	; 13 bytes
	php
	pha
	ldx #206
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	pla
	plp
.elseif(n = 5990)
	; 12 bytes
	php
	ldx #234 ;hides 'nop'
	lda #238
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 5991)
	; 12 byte                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
	lda #49
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 6570)
	; 12 bytes
	php
	ldx #193
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 6571)
	; 13 bytes
	php
	pha
	ldx #149
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	pla
	plp
.elseif(n = 6572)
	; 12 bytes
	php
	ldx #122 ;hides 'nop'
	lda #15
	jsr delay_a_25_clocks
	inx
	Jne *-7
	plp
.elseif(n = 6573)
	; 12 bytes
	php
	ldx #(n-8)/101
	lda #69
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 6574)
	; 12 bytes
	php
	ldx #97
	lda #35
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 6575)
	; 12 bytes
	php
	ldx #199
	lda #1
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 6576)
	; 12 bytes
	php
	clc
	lda #$2A ;hides 'rol a'
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	Jcc *-6
	plp
.elseif(n = 6578)
	; 12 bytes
	php
	ldx #106 ;hides 'ror a'
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dex
	Jne *-7
	plp
.elseif(n = 6579)
	; 13 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	lda #18
	jsr delay_a_25_clocks
	inx
	Jpl *-8
	plp
.elseif(n = 6580)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 6581)
	; 12 bytes
	php
	ldx #24 ;hides 'clc'
	lda #229
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.endif
.else
; 6582 to 6601
.if(n = 6582)
	; 12 bytes
	php
	ldx #173
	lda #6
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 6583)
	; 12 bytes
	php
	ldx #24
	lda #231
	jsr delay_a_25_clocks
	dex
	Jpl *-6
	plp
.elseif(n = 6584)
	; 12 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	ldx #218
	dex
	Jne *-1
	Jcc *-6
	plp
.elseif(n = 6585)
	; 12 bytes
	php
	ldx #42 ;hides 'rol a'
	lda #119
	jsr delay_a_25_clocks
	dex
	Jpl *-7
	plp
.elseif(n = 6586)
	; 12 bytes
	php
	ldx #(n-8)/26
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dex
	Jne *-6
	plp
.elseif(n = 6587)
	; 11 bytes
	php
	ldy #51
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	plp
.elseif(n = 6588)
	; 12 bytes
	php
	ldx #188
	lda #3
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 6589 || n = 6593 || n = 6598 || n = 6601)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6590)
	; 12 bytes
	php
	ldx #248 ;hides 'sed'
	ldy #163
	dey
	Jne *-1
	inx
	Jne *-7
	plp
.elseif(n = 6591)
	; 11 bytes
	php
	ldx #227
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-5
	plp
.elseif(n = 6592)
	; 12 bytes
	php
	ldx #74 ;hides 'lsr a'
	lda #55
	jsr delay_a_25_clocks
	dex
	Jne *-7
	plp
.elseif(n = 6594)
	; 12 bytes
	php
	ldx #178
	lda #5
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 6595)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #187
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 6596)
	; 11 bytes
	php
	ldx #121
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	plp
.elseif(n = 6597)
	; 13 bytes
	php
	clc
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #187
	dex
	Jne *-1
	Jcs *-7
	plp
.elseif(n = 6599)
	; 12 bytes
	php
	ldx #169
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dex
	Jne *-6
	plp
.elseif(n = 6600)
	; 12 bytes
	php
	ldx #206
	lda #0
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 6602) && (n < 6621))
; 6602 to 6620
.if(n = 6602)
	; 12 bytes
	php
	ldx #157
	lda #10
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 6603)
	; 12 bytes
	php
	ldx #136 ;hides 'dey'
	ldy #145
	dey
	Jne *-1
	dex
	Jmi *-7
	plp
.elseif(n = 6604)
	; 12 bytes
	php
	ldx #194
	lda #2
	jsr delay_a_25_clocks
	dex
	Jne *-6
	plp
.elseif(n = 6605 || n = 6617)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 6606)
	; 12 bytes
	php
	ldx #56 ;hides 'sec'
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	inx
	Jne *-7
	plp
.elseif(n = 6607)
	; 12 bytes
	php
	ldx #168 ;hides 'tay'
	lda #127
	jsr delay_a_25_clocks
	dex
	Jmi *-7
	plp
.elseif(n = 6608)
	; 11 bytes
	php
	ldx #150
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dex
	Jne *-5
	plp
.elseif(n = 6609)
	; 12 bytes
	php
	ldx #(n-169)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dex
	Jpl *-6
	plp
.elseif(n = 6610 || n = 6613)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6611)
	; 12 bytes
	php
	ldx #(n-8)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dex
	Jne *-6
	plp
.elseif(n = 6612)
	; 12 bytes
	php
	ldx #(