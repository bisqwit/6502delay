; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, and S
; Will preserve X, Y, and P
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
;
; Requires a function called "delay_256a_16_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A*256 clocks + overhead
;; Clobbers A. Preserves X,Y.
;; Time: A*256+16 clocks (including JSR)
;; Depends on delay_a_25_clocks
;;;;;;;;;;;;;;;;;;;;;;;;;
;delay_256a_16_clocks:
;	cmp #0
;	bne :+
;	rts
;delay_256a_11_clocks_:
;:       pha
;	 lda #(256-25-7-2-2-3)
;	 jsr delay_a_25_clocks
;	pla
;	sec
;	sbc #1
;	bne :-
;	rts
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
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 289
; - 8 bytes or less when n <= 292
; - 9 bytes or less when n <= 302
; -10 bytes or less when n <= 320
; -11 bytes or less when n <= 1326
; -12 bytes or less when n <= 2058
; -13 bytes or less when n <= 3826
; -14 bytes or less when n <= 4866
; -15 bytes or less when n <= 6664
; -16 bytes or less when n <= 9228
; -17 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=6, avg=6.8, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=9
; - When 301 <= n <= 500: min=8, avg=9.6, max=11
; - When 501 <= n <= 750: min=7, avg=9.6, max=11
; - When 751 <= n <= 1000: min=7, avg=9.6, max=11
; - When 1001 <= n <= 2000: min=7, avg=10.3, max=12
; - When 2001 <= n <= 3000: min=7, avg=11.4, max=13
; - When 3001 <= n <= 5000: min=7, avg=11.7, max=15
; - When 5001 <= n <= 7500: min=7, avg=12.1, max=16
; - When 7501 <= n <= 10000: min=7, avg=12.3, max=17
; - When 10001 <= n <= 20000: min=7, avg=12.4, max=17
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
.elseif(n = 4 || n = 5 || n = 8 || n = 17 || n = 28)
	; 2..5 bytes
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
.elseif((n > 8 && n < 12) || (n > 19 && n < 24) || n = 33)
	; 3..6 bytes
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
.elseif(n = 18 || n = 30)
	; 4..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 31)
	; 6 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif((n > 33 && n < 38) || n = 40 || n = 42 || n = 44 || n = 45 || n = 47 || (n > 48 && n < 58) || (n > 58 && n < 62) || (n > 62 && n < 79))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 38)
	; 7 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
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
.else
; 79 to 156
.if((n > 78 && n < 118) || (n > 118 && n < 123) || (n > 123 && n < 157))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
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
.endif
.endif
.else
; 157 to 312
.if((n > 156 && n < 163) || (n > 163 && n < 290))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 163)
	; 7 bytes
	php
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	plp
.elseif(n = 290)
	; 8 bytes
	php
	sec
	lda #$69 ;hides 'adc #$D8'
	cld ;first loop only
	Jne *-2
	plp
.elseif(n = 291 || n = 309)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 292 || n = 308)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 293 || n = 294 || n = 296)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
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
.elseif(n = 297 || n = 312)
	; 9 bytes
	php
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	plp
.elseif(n = 298 || n = 299 || n = 301)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 300 || n = 310)
	; 9 bytes
	php
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	plp
.elseif(n = 302 || n = 307)
	; 9 bytes
	php
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	plp
.elseif(n = 303)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 304)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
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
.elseif(n = 393 || n = 398 || n = 408 || n = 413 || n = 428 || n = 433 || n = 438 || n = 443 || n = 463)
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
.elseif(n = 403)
	; 10 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 404)
	; 9 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	brk 0
	Jne *-4
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
.elseif(n = 418)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
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
.elseif(n = 448)
	; 10 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_15cyc
	Jne *-5
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
.elseif(n = 468)
	; 10 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_15cyc
	Jcs *-5
	plp
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 471 || n = 484 || n = 489 || n = 494 || n = 499 || n = 504 || n = 506 || n = 519 || n = 524 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 541 || n = 546)
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
.elseif(n = 477 || n = 487 || n = 492 || n = 502 || n = 512 || n = 517 || n = 522 || n = 542)
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 537)
	; 7 bytes
	php
	lda #2
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
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
.elseif(n = 549 || n = 556 || n = 591 || n = 619)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif((n .mod 5 = 0) && ((n > 549 && n < 561) || n = 575 || (n > 589 && n < 601) || (n > 609 && n < 626)))
	; 9 bytes
	php
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	plp
.elseif(n = 551 || n = 558 || n = 593 || n = 614 || n = 621)
	; 10 bytes
	php
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	plp
.elseif(n = 553)
	; 10 bytes
	php
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 554 || n = 559 || n = 564 || n = 569 || n = 574 || n = 581 || n = 589 || n = 594 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 566)
	; 9 bytes
	php
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	brk 0
	Jne *-4
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
.elseif(n = 584)
	; 9 bytes
	php
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	brk 0
	Jne *-4
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
.elseif(n = 636 || n = 641 || n = 643 || n = 653 || n = 673 || n = 678 || n = 683 || n = 688 || n = 693 || n = 698)
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
.elseif(n = 648)
	; 10 bytes
	php
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	jsr @nearby_rts_15cyc
	Jne *-5
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
.elseif(n = 708)
	; 10 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 713 || n = 723 || n = 728 || n = 741 || n = 743 || n = 748 || n = 753 || n = 758 || n = 763 || n = 778)
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
.if((n .mod 5 = 2) && ((n > 781 && n < 858)))
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
.elseif(n = 784 || n = 790 || n = 795 || n = 798 || n = 804 || n = 809 || n = 814 || n = 819 || n = 821 || n = 834 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854)
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
	jsr delay_256a_11_clocks_
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 794 || n = 799)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 800)
	; 9 bytes
	php
	pha
	lda #3
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 801 || n = 806)
	; 9 bytes
	delay_n (n-13)
	brk 0
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
.if(n = 938 || n = 951 || n = 953 || n = 958 || n = 963 || n = 968 || n = 988 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1023 || n = 1028 || n = 1033 || n = 1043 || n = 1047 || n = 1052 || n = 1059 || n = 1068 || n = 1073 || n = 1078 || n = 1093)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 939 || n = 944 || n = 949 || n = 954 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 989 || n = 994 || n = 1009 || n = 1014 || n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1029 || n = 1046 || n = 1048 || n = 1051 || n = 1053 || n = 1054 || n = 1058 || n = 1064 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1021) || (n > 1029 && n < 1091)))
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
.elseif(n = 956 || n = 996)
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
.elseif(n = 1031)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1036)
	; 10 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	brk 0
	Jmi *-3
	tay
	plp
.elseif(n = 1038)
	; 10 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1044)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1049)
	; 7 bytes
	php
	lda #4
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1056)
	; 9 bytes
	php
	pha
	lda #4
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1061)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1063)
	; 10 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1066)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
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
.elseif(n = 1253 || n = 1273 || n = 1278 || n = 1283 || n = 1288 || n = 1295 || n = 1299 || n = 1301 || n = 1303 || n = 1308)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1254 || n = 1259 || n = 1264 || n = 1269 || n = 1271 || n = 1274 || n = 1289 || n = 1294 || n = 1302 || n = 1304 || n = 1307 || n = 1309 || n = 1310 || n = 1314 || n = 1323)
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
.elseif(n = 1256 || n = 1263 || n = 1284 || n = 1291 || n = 1319 || n = 1326)
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
.elseif(n = 1266 || n = 1322)
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1305)
	; 7 bytes
	php
	lda #5
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1306 || n = 1311)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1312)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1313 || n = 1318 || n = 1325)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1315 || n = 1320)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1316)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 1317)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1327)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1330 || n = 1338 || n = 1346)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1331)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1332 || n = 1348 || n = 1364)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1362)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1334)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $93A0'
	ldy #147
	dey
	Jne *-4
	tay
	plp
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1364)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1336 || n = 1345 || n = 1357 || n = 1359)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1337 || n = 1355 || n = 1358 || n = 1365 || n = 1367)
	; 11..12 bytes
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
.elseif(n = 1343)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $6CA0'
	ldy #108
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1344 || n = 1353 || n = 1362)
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
.elseif(n = 1351 || n = 1360)
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
	bit <$2C ;hides 'bit $6BA0'
	ldy #107
	iny
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
.endif
.else
; 1368 to 1406
.if((n .mod 7 = 3) && ((n > 1367 && n < 1390) || n = 1403))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1369)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 7 = 5) && ((n > 1369 && n < 1406)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1371 || n = 1380)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1372 || n = 1388 || n = 1404)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1373 || n = 1381 || n = 1397 || n = 1399)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1376)
	; 12 bytes
	php
	tya
	ldy #123
	delay_n 4
	nop
	dey
	Jpl *-4
	tay
	plp
.elseif(n = 1378)
	; 10 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	brk 0
	Jmi *-3
	tax
	plp
.elseif(n = 1379 || n = 1393 || n = 1395 || n = 1400)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1383)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 1386 || n = 1402)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1406)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
	plp
.endif
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
.elseif(n = 1408)
	; 11 bytes
	php
	clc
	lda #58 ;hides 'nop'
	brk 0
	adc #1
	Jpl *-5
	plp
.elseif(n = 1409)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1410)
	; 9 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
	plp
.elseif(n = 1411 || n = 1439 || n = 1467)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 1413 || n = 1422 || n = 1427 || n = 1429 || n = 1434 || n = 1455 || n = 1457 || n = 1471 || n = 1483)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442) || n = 1477))
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1415)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $64A0'
	ldy #100
	iny
	Jne *-4
	tay
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
.elseif(n = 1421 || n = 1435 || n = 1446 || n = 1449 || n = 1456 || n = 1463 || n = 1465 || n = 1481)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1430)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
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
.elseif(n = 1437)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1448)
	; 11 bytes
	php
	clc
	lda #184 ;hides 'clv'
	brk 0
	adc #1
	Jne *-5
	plp
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
.elseif(n = 1453)
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
.elseif(n = 1469)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1472)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1478)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $A3A0'
	ldy #163
	dey
	Jne *-4
	tay
	plp
.endif
.else
; 1485 to 1562
.if(n = 1485 || n = 1499 || n = 1511 || n = 1518 || n = 1525 || n = 1537 || n = 1539 || n = 1553 || n = 1555 || n = 1559)
	; 8..11 bytes
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
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1516) || n = 1529 || (n > 1542 && n < 1558)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1488 || n = 1497 || n = 1506 || n = 1533 || n = 1542 || n = 1551)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1553)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1490 || n = 1498 || n = 1514 || n = 1530 || n = 1546 || n = 1554)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1491 || n = 1493 || n = 1505 || n = 1509 || n = 1519 || n = 1521 || n = 1526 || n = 1544 || n = 1547 || n = 1558 || n = 1560)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1492 || n = 1500 || n = 1516 || n = 1532 || n = 1540 || n = 1548)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
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
.elseif(n = 1534)
	; 9 bytes
	php
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1535)
	; 11 bytes
	delay_n (n-13)
	brk 0
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
	tya
	bit <$2C ;hides 'bit $AAA0'
	ldy #170
	dey
	Jne *-4
	tay
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1561)
	; 7 bytes
	php
	lda #6
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563 || n = 1565 || n = 1566 || n = 1570 || n = 1589 || n = 1591 || n = 1597)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1564 || n = 1571 || n = 1583 || n = 1590 || n = 1598)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1567)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1568)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1569 || n = 1574 || n = 1581)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1572 || n = 1577)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif((n .mod 7 = 5) && ((n > 1572 && n < 1602)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1575)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1576)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.elseif(n = 1593)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
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
.elseif(n = 1607)
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
.elseif(n = 1617 || n = 1619 || n = 1624 || n = 1631 || n = 1638)
	; 11..12 bytes
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
.elseif(n = 1633)
	; 11 bytes
	delay_n (n-13)
	brk 0
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
	php
	tya
	bit <$2C ;hides 'bit $B5A0'
	ldy #181
	dey
	Jne *-4
	tay
	plp
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
.elseif(n = 1645 || n = 1673 || n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1715 || n = 1717)
	; 11..12 bytes
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
.elseif(n = 1647 || n = 1661)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1648)
	; 9 bytes
	php
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1649 || n = 1651 || n = 1665 || n = 1672 || n = 1679 || n = 1681 || n = 1688 || n = 1707 || n = 1709)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1663 || n = 1691 || n = 1705)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 1667)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $B8A0'
	ldy #184
	dey
	Jne *-4
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
.elseif(n = 1703)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1710)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
.elseif(n = 1712)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
	plp
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1756)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1721)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1722 || n = 1731 || n = 1740)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1723 || n = 1735 || n = 1737 || n = 1742 || n = 1747 || n = 1749)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1724 || n = 1756)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1725 || n = 1732 || n = 1739 || n = 1753)
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
.elseif(n = 1729)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1730 || n = 1738 || n = 1754)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1733 || n = 1736 || n = 1743 || n = 1745 || n = 1757)
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
.endif
.else
; 1758 to 1796
.if(n = 1758)
	; 9 bytes
	php
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1759 || n = 1789)
	; 11..12 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1760)
	; 9 bytes
	php
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1761 || n = 1777 || n = 1782 || n = 1791)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 1761 && n < 1791)))
	; 10 bytes
	php
	tya
	ldy #(n-12)/7
	nop
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
.elseif(n = 1764 || n = 1772 || n = 1780 || n = 1796)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1765)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1766)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $3DA0'
	ldy #61
	iny
	Jne *-4
	tay
	plp
.elseif((n .mod 7 = 3) && ((n > 1766 && n < 1796)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1768 || n = 1773)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1770 || n = 1778 || n = 1786)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1775)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	dey
	Jne *-4
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
.elseif(n = 1784)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1785 || n = 1794)
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
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
.elseif(n = 1793)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	iny
	Jne *-4
	tay
	plp
.endif
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
.elseif(n = 1799 || n = 1806 || n = 1814 || n = 1816 || n = 1819 || n = 1821 || n = 1822 || n = 1826)
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
	; 11 bytes
	php
	sec
	lda #(n-8)/20 ;hides 'nop'
	brk 0
	sbc #1
	Jne *-5
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
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1812)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1817)
	; 7 bytes
	php
	lda #7
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1818 || n = 1823)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1824)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1825 || n = 1830)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1828 || n = 1833)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 1829)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1831)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1832)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1834)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
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
.elseif(n = 1838)
	; 11 bytes
	delay_n (n-13)
	brk 0
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
.elseif(n = 1840 || n = 1849)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1841)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
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
.elseif(n = 1845 || n = 1854 || n = 1859)
	; 11..12 bytes
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
.elseif(n = 1847)
	; 11 bytes
	php
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
	tax
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1853)
	; 12 bytes
	php
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1861 || n = 1863 || n = 1867)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1862)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
	dey
	Jne *-4
	tay
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
.elseif(n = 1865)
	; 11 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1869)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
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
.elseif(n = 1871)
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
.if((n >= 1876) && (n < 1915))
; 1876 to 1914
.if(n = 1876 || n = 1892 || n = 1900 || n = 1908)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1877 || n = 1878 || n = 1895)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879 || n = 1885 || n = 1887 || n = 1897 || n = 1903 || n = 1905 || n = 1907)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1880)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
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
.elseif((n .mod 8 = 2) && ((n > 1881 && n < 1915)))
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1883)
	; 11 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	tay
	plp
.elseif((n .mod 9 = 3) && ((n > 1883 && n < 1912)))
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
.elseif(n = 1891)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1894)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1896)
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
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1904)
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
.elseif(n = 1910)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1912)
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
.endif
.else
; 1915 to 1953
.if(n = 1915)
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
.elseif(n = 1916)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 1917)
	; 11 bytes
	php
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	adc #1
	Jne *-5
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
.elseif(n = 1919)
	; 11 bytes
	php
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	tax
	plp
.elseif((n .mod 9 = 3) && ((n > 1919 && n < 1948)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1921 || n = 1939)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1922 || n = 1930 || n = 1946)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1923 || n = 1941 || n = 1951 || n = 1953)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 8 = 4) && ((n > 1923 && n < 1949)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
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
.elseif(n = 1926 || n = 1931 || n = 1949)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1933)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
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
.elseif(n = 1937)
	; 11 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
	plp
.elseif(n = 1942)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1945)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1950)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
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
.elseif(n = 1955)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	iny
	Jne *-4
	tay
	plp
.elseif((n .mod 9 = 3) && ((n > 1955 && n < 1993)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1957 || n = 1975)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1958)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
.elseif(n = 1960)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
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
.elseif(n = 1962 || n = 1978 || n = 1986)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1963 || n = 1990)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1964 || n = 1980 || n = 1988)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1966)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1967 || n = 1976 || n = 1985)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1968)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1969 || n = 1977 || n = 1987 || n = 1989)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 1973)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	dey
	Jne *-4
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
.elseif(n = 1982)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1984)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 2006 || n = 2021 || n = 2029)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1994 || n = 2002 || n = 2018 || n = 2026)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1995 || n = 1997 || n = 2005 || n = 2007 || n = 2011 || n = 2013 || n = 2015)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 8 = 4) && ((n > 1995 && n < 2021)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1998)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 1999 || n = 2008 || n = 2017)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2000)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	iny
	Jne *-4
	tay
	plp
.elseif((n .mod 9 = 3) && ((n > 2000 && n < 2029)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2003 || n = 2025)
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
.elseif(n = 2009)
	; 11 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2014)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2016)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2022)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
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
	tya
	bit <$2C ;hides 'bit $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
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
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
.elseif(n = 2031)
	; 12 bytes
	pha
	delay_n (n-3)
.endif
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
.elseif(n = 2039 || n = 2057 || n = 2066 || n = 2070)
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
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	iny
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
.elseif(n = 2048)
	; 11 bytes
	php
	clc
	lda #26 ;hides 'nop'
	brk 0
	adc #1
	Jpl *-5
	plp
.elseif(n = 2049 || n = 2051 || n = 2061)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2054)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	iny
	Jne *-4
	tay
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
.elseif(n = 2059)
	; 13 bytes
	php
	pha
	tya
	ldy #255
	cmp $C5
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 2063)
	; 11 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	iny
	Jne *-4
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
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2076 || n = 2101)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2072 || n = 2075 || n = 2077 || n = 2078 || n = 2082 || n = 2090)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2073)
	; 7 bytes
	php
	lda #8
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2074 || n = 2079)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2080)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2081 || n = 2086 || n = 2093 || n = 2094 || n = 2099)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 2083 || n = 2088)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2084)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2085)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2087)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2095)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
	jsr delay_256a_16_clocks
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
.elseif(n = 2102)
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
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
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
	; 12 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
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
.elseif(n = 2108)
	; 11 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	tay
	plp
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2122 || n = 2146)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2111 || n = 2133)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	plp
.elseif(n = 2112 || n = 2124 || n = 2148)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2113)
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
.elseif(n = 2114)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2115 || n = 2131 || n = 2139)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 2117)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	iny
	Jne *-4
	tay
	plp
.elseif((n .mod 9 = 3) && ((n > 2117 && n < 2146)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2119 || n = 2128 || n = 2137)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2120 || n = 2130 || n = 2140)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
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
.elseif(n = 2126)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2129 || n = 2147)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2135)
	; 11 bytes
	php
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2138)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2141)
	; 12 bytes
	php
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
	plp
.elseif(n = 2142)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
	dey
	Jne *-4
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
.elseif(n = 2144)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
	plp
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2151 || n = 2159 || n = 2164 || n = 2169 || n = 2173 || n = 2175 || n = 2176 || n = 2185 || n = 2187)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2150)
	; 11 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	brk 0
	dey
	Jmi *-4
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
.elseif(n = 2153)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	iny
	Jne *-4
	tay
	plp
.elseif((n .mod 9 = 3) && ((n > 2153 && n < 2182)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2155 || n = 2166 || n = 2177)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	plp
.elseif((n .mod 9 = 5) && ((n > 2155 && n < 2184)))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2157)
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
.elseif(n = 2158 || n = 2182)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2160 || n = 2184)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2162)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2167)
	; 12 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
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
.elseif(n = 2171)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2178)
	; 12 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2180)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
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
.elseif(n = 2189)
	; 11 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	tay
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
.elseif(n = 2191 || n = 2193 || n = 2195 || n = 2214)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2192 || n = 2202 || n = 2222)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
	dey
	Jne *-4
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
.elseif(n = 2198)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	iny
	Jne *-4
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
.elseif(n = 2203 || n = 2213 || n = 2219)
	; 12 bytes
	delay_n (n-2)
	nop
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
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
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
.elseif(n = 2212 || n = 2223)
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
.elseif(n = 2216)
	; 11 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
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
.elseif(n = 2225)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
	plp
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2239 || n = 2241 || n = 2248 || n = 2257 || n = 2259)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 9 = 5) && ((n > 2227 && n < 2256)))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2229)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
.elseif(n = 2231)
	; 12 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	nop
	iny
	Jne *-3
	pla
	tay
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
.elseif(n = 2234)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	tay
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
.elseif(n = 2236 || n = 2264)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2238)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
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
; 2266 to 2304
.if(n = 2266 || n = 2302)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2267 || n = 2300)
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
.elseif(n = 2268 || n = 2292 || n = 2304)
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
.elseif((n .mod 9 = 3) && ((n > 2270 && n < 2299)))
	; 11 bytes
	php
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2272 || n = 2282)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2273 || n = 2283 || n = 2293 || n = 2299 || n = 2303)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2274 || n = 2284 || n = 2295)
	; 12 bytes
	pha
	delay_n (n-3)
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
	; 12 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2285)
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
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
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
	clv
	Jvc *+2
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2313 || n = 2319)
	; 12 bytes
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
.elseif(n = 2317 || n = 2318)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2320)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
.elseif(n = 2321)
	; 12 bytes
	php
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	nop
	iny
	Jne *-5
	tay
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
.if(n = 2324)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2325)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	delay_n 4
	dey
	Jne *-6
	tay
	plp
.elseif(n = 2326 || n = 2328 || n = 2331 || n = 2333 || n = 2334 || n = 2338)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2327 || n = 2332)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2329)
	; 7 bytes
	php
	lda #9
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2330 || n = 2335)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2336)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2337 || n = 2342)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 2339)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2340)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
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
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2351)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2345)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2346 || n = 2354)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 2348)
	; 11 bytes
	php
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2349 || n = 2355)
	; 11 bytes
	delay_n (n-13)
	brk 0
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
.elseif(n = 2352 || n = 2364 || n = 2376)
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
.elseif((n > 2355 && n < 2360) || n = 2361 || n = 2367 || n = 2369 || n = 2371 || n = 2373 || n = 2379 || n = 2381)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2360 || n = 2380)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
.elseif(n = 2362 || n = 2374)
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2365)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2366 || n = 2377)
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
.elseif(n = 2368)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2370)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2372 || n = 2382)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2375)
	; 12 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2378)
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
; 2383 to 2421
.if(n = 2383 || n = 2385 || n = 2389 || n = 2391 || (n > 2392 && n < 2397) || n = 2401 || n = 2403 || n = 2405 || n = 2407 || n = 2409 || n = 2411 || n = 2413 || (n > 2414 && n < 2418))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 2386 || n = 2398 || n = 2410)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2387)
	; 12 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2388 || n = 2400 || n = 2412)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2390)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
.elseif(n = 2392 || n = 2402)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2397 || n = 2419)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	plp
.elseif(n = 2399 || n = 2421)
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
.elseif(n = 2404)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
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
	ldy #218 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2414)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2418)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2420)
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
; 2422 to 2500
.if(n = 2422 || n = 2434 || n = 2458 || n = 2470 || n = 2482 || n = 2494)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2423 || n = 2425 || n = 2427 || n = 2433 || n = 2435 || n = 2437 || n = 2439 || n = 2444 || n = 2447 || n = 2451 || (n > 2454 && n < 2458) || n = 2459 || n = 2461 || n = 2464 || n = 2466 || n = 2467 || n = 2469 || n = 2471 || n = 2473 || n = 2477 || n = 2481 || n = 2483 || n = 2489 || n = 2491 || n = 2493 || n = 2495 || n = 2497 || n = 2499)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 12 = 0) && ((n > 2423 && n < 2497)))
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2426 || n = 2438 || n = 2486)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2428)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	brk 0
	sbc #1
	Jmi *-5
	plp
.elseif(n = 2429)
	; 12 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	nop
	bit $24
	adc #1
	Jne *-6
	plp
.elseif(n = 2430)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jmi *-4
	tay
	plp
.elseif(n = 2431)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	nop
	bit $24
	iny
	Jne *-5
	tay
	plp
.elseif((n .mod 11 = 1) && ((n > 2431 && n < 2466) || n = 2487 || n = 2498))
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
.elseif(n = 2440)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 2441 || n = 2463 || n = 2474 || n = 2485)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	plp
.elseif(n = 2442 || n = 2478)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2445)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
.elseif(n = 2449 || n = 2479)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2450)
	; 11 bytes
	php
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2452 || n = 2462 || n = 2492)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
	dey
	Jne *-4
	tay
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
.elseif(n = 2468)
	; 11 bytes
	php
	sec
	lda #250 ;hides 'nop'
	brk 0
	sbc #1
	Jmi *-5
	plp
.elseif(n = 2475)
	; 12 bytes
	php
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2476)
	; 10 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	brk 0
	Jne *-3
	tay
	plp
.elseif(n = 2480 || n = 2500)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
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
.endif
.else
.if((n >= 2501) && (n < 3751))
.if((n >= 2501) && (n < 3126))
.if((n >= 2501) && (n < 2813))
.if((n >= 2501) && (n < 2657))
.if((n >= 2501) && (n < 2579))
.if((n >= 2501) && (n < 2540))
; 2501 to 2539
.if(n = 2501 || n = 2503 || n = 2505 || n = 2511 || n = 2513 || n = 2515 || n = 2517 || n = 2521 || (n > 2522 && n < 2529))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2502 || n = 2522)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	clv
	Jvc *+2
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2504)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
.elseif(n = 2507 || n = 2529)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
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
.elseif(n = 2509 || n = 2531)
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
.elseif(n = 2512)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 2514)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2516)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2519)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dey
	Jne *-5
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
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
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
.elseif(n = 2541)
	; 12 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	tay
	plp
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
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 2544 || n = 2556)
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
.elseif(n = 2546)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2547 || n = 2557)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2548)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
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
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
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
	clv
	Jvc *+2
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2553)
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
.elseif(n = 2555)
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
.endif
.else
; 2559 to 2578
.if(n = 2559)
	; 12 bytes
	php
	sec
	lda #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	sbc #1
	Jne *-6
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
	; 11 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	iny
	iny
	Jne *-4
	tay
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
.elseif(n = 2563)
	; 12 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2564 || n = 2575)
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
.elseif(n = 2565)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2566 || n = 2578)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2569)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2570)
	; 12 bytes
	php
	sec
	lda #0
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
.elseif(n = 2571)
	; 12 bytes
	php
	sec
	lda #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	sbc #1
	Jpl *-6
	plp
.elseif(n = 2572)
	; 12 bytes
	php
	tya
	ldy #0
	clv
	Jvc *+2
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
.elseif(n = 2574)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2576)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2577)
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
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2581)
	; 12 bytes
	php
	tya
	ldy #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dey
	Jpl *-5
	tay
	plp
.elseif(n = 2582 || n = 2584 || n = 2587 || n = 2589 || n = 2590 || n = 2594)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2583 || n = 2588)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2585)
	; 7 bytes
	php
	lda #10
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2586 || n = 2591)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2592)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2593 || n = 2598 || n = 2605 || n = 2606 || n = 2611)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 2595 || n = 2600)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2596 || n = 2601 || n = 2608)
	; 10..12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2597)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2599)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2602 || n = 2614)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2603)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 2604 || n = 2616)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2607)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2609)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2610)
	; 12 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2612 || n = 2615)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2654)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2619)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2620 || n = 2656)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 2621)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2622)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2623 || n = 2625 || n = 2627 || n = 2629 || (n > 2630 && n < 2636) || n = 2637 || n = 2643 || (n > 2644 && n < 2648) || n = 2653 || n = 2655)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2624)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2626 || n = 2638 || n = 2650)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2628 || n = 2640 || n = 2652)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2630 || n = 2641)
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
.elseif(n = 2636)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2639)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2642)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2644)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2648)
	; 12 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	plp
.elseif(n = 2649)
	; 12 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	.repeat 4
	 nop
	.endrepeat
	Jne *-5
	tax
	plp
.elseif(n = 2651)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	nop
	iny
	Jne *-5
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if(n = 2657 || n = 2659 || n = 2660 || n = 2665 || (n > 2666 && n < 2670) || n = 2671 || n = 2673 || n = 2675 || n = 2677 || (n > 2678 && n < 2683) || n = 2687 || n = 2689 || n = 2695 || n = 2697 || n = 2699 || n = 2703 || n = 2704 || n = 2706 || n = 2708 || n = 2709 || n = 2711 || n = 2713 || n = 2715 || n = 2717 || (n > 2718 && n < 2722) || n = 2723 || n = 2725 || (n > 2730 && n < 2734))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2658 || n = 2730)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2661 || n = 2672 || n = 2716 || n = 2727)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
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
.elseif((n .mod 11 = 1) && (n = 2663 || (n > 2684 && n < 2730)))
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
.elseif(n = 2666 || n = 2678 || n = 2702)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2670 || n = 2684 || n = 2726)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 2690)
	; 11 bytes
	php
	tya
	ldy #122 ;hides 'nop'
	brk 0
	iny
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
.elseif(n = 2701)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
.elseif(n = 2714)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2728)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	plp
.elseif((n .mod 12 = 0) && ((n > 2735 && n < 2773)))
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2737 || n = 2739 || (n > 2742 && n < 2746) || n = 2752 || n = 2753 || n = 2755 || n = 2757 || n = 2763 || n = 2765 || n = 2767 || n = 2769)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif((n .mod 11 = 1) && ((n > 2739 && n < 2774)))
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
.elseif(n = 2741)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2742 || n = 2756)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2746 || n = 2758 || n = 2770)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2747)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2749)
	; 12 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2750)
	; 11 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_15cyc
	Jne *-4
	tay
	plp
.elseif(n = 2754 || n = 2768)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2759)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2761)
	; 12 bytes
	php
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2764)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 2766)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2771)
	; 12 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
	plp
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2786 || n = 2788)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n > 2774 && n < 2778) || n = 2781 || n = 2785)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2778)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2779)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
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
.elseif(n = 2782 || n = 2794 || n = 2806)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2783)
	; 12 bytes
	php
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2784 || n = 2796 || n = 2808)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2787 || n = 2797 || n = 2799 || n = 2809 || n = 2811)
	; 12 bytes
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
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
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
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 2802)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
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
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2823 || n = 2833 || n = 2835 || n = 2839 || n = 2844)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2814 || n = 2819 || n = 2822 || n = 2834 || n = 2838 || n = 2840 || n = 2843 || n = 2845 || n = 2846 || n = 2850)
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
.elseif(n = 2821)
	; 12 bytes
	php
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2841)
	; 7 bytes
	php
	lda #11
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2849)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 2851)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2857)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2853)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2854 || n = 2861 || n = 2862 || n = 2867)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 2855)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2856)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2858 || n = 2870)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2859)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 2860)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2863 || n = 2864)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2865)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
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
.elseif(n = 2869 || n = 2871 || n = 2873 || n = 2876 || n = 2879 || n = 2881 || (n > 2882 && n < 2890))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2872)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 2874)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2875)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2877)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2882)
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
; 2891 to 2968
.if(n = 2891 || n = 2893 || n = 2895 || (n > 2896 && n < 2902) || n = 2903 || n = 2905 || n = 2907 || n = 2909 || (n > 2910 && n < 2914) || n = 2915 || n = 2917 || (n > 2918 && n < 2922) || n = 2923 || n = 2925 || n = 2927 || n = 2929 || (n > 2930 && n < 2936) || n = 2937 || n = 2939 || n = 2941 || n = 2943 || n = 2945 || (n > 2946 && n < 2950) || n = 2953 || n = 2955 || n = 2956 || (n > 2957 && n < 2962) || n = 2963 || n = 2965 || n = 2967 || n = 2968)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 2894 || n = 2922 || n = 2936)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2896 || n = 2924 || n = 2966)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif((n .mod 12 = 2) && ((n > 2905 && n < 2955)))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2908 || n = 2944)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 2910 || n = 2946)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2951)
	; 12 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2957)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n > 2968 && n < 2974) || n = 2975 || n = 2977 || n = 2979 || n = 2983 || n = 2985 || n = 2987 || n = 2989 || n = 2991 || n = 2993 || n = 2995 || n = 2996 || n = 2999 || n = 3001 || (n > 3002 && n < 3006) || n = 3007 || n = 3009 || n = 3011 || n = 3015 || n = 3017 || n = 3019 || n = 3021 || n = 3023 || n = 3025 || (n > 3026 && n < 3032) || n = 3033 || n = 3035 || n = 3037 || (n > 3038 && n < 3042))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 2978 || n = 2992 || n = 3006 || n = 3020)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2980 || n = 3016)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
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
.elseif(n = 2982 || n = 3018)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2984)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	clv
	Jvc *+2
	sbc #1
	Jne *-6
	plp
.elseif((n .mod 12 = 2) && ((n > 2989 && n < 3039)))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2994 || n = 3008)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2997)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3013)
	; 12 bytes
	php
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
	tax
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
.elseif(n = 3042)
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
.elseif(n = 3043)
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
.elseif(n = 3044)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3045)
	; 13 bytes
	delay_n (n-13)
	brk 0
.endif
.else
.if((n >= 3047) && (n < 3086))
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
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
.elseif(n = 3048 || n = 3060)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3049 || n = 3051 || n = 3057 || n = 3061 || n = 3063)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3050 || n = 3064)
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
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
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
.elseif(n = 3054)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 3056)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3058)
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
.endif
.else
; 3066 to 3085
.if(n = 3066)
	; 13 bytes
	php
	sec
	lda #(n-10)/16
	delay_n 11
	sbc #1
	Jne *-6
	plp
.elseif(n = 3067)
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
	; 12 bytes
	php
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	brk 0
	iny
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
.elseif(n = 3070)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 3072)
	; 11 bytes
	php
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3073 || n = 3085)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3078)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3079)
	; 12 bytes
	php
	tya
	ldy #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3080)
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
.elseif(n = 3081)
	; 13 bytes
	php
	tya
	ldy #98
	delay_n 26
	dey
	Jpl *-5
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
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3094 || n = 3096 || n = 3099 || n = 3101 || n = 3102 || n = 3106 || n = 3114)
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
.elseif(n = 3088 || n = 3124)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
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
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3093 || n = 3095 || n = 3100)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3097)
	; 7 bytes
	php
	lda #12
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3098 || n = 3103)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3104)
	; 9 bytes
	php
	pha
	lda #12
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3105 || n = 3110 || n = 3117 || n = 3118 || n = 3123)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3107 || n = 3112)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3108 || n = 3113 || n = 3120)
	; 10..12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3109)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3111)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3115)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 3116)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3119 || n = 3122)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3121)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3125)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.if(n = 3126 || n = 3144 || n = 3162 || n = 3180 || n = 3198 || n = 3216 || n = 3234 || n = 3252 || n = 3270)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3127 || n = 3129 || n = 3130 || n = 3135 || n = 3137 || (n > 3138 && n < 3142) || n = 3143 || n = 3145 || n = 3147 || (n > 3148 && n < 3152) || (n > 3152 && n < 3160) || n = 3161 || (n > 3162 && n < 3174) || n = 3175 || n = 3177 || n = 3179 || (n > 3180 && n < 3188) || n = 3189 || (n > 3190 && n < 3194) || n = 3195 || n = 3197 || (n > 3198 && n < 3202) || n = 3203 || (n > 3204 && n < 3208) || n = 3209 || n = 3211 || n = 3212 || n = 3215 || n = 3217 || (n > 3218 && n < 3228) || n = 3229 || n = 3231 || n = 3233 || (n > 3234 && n < 3244) || n = 3245 || (n > 3246 && n < 3250) || n = 3251 || n = 3254 || n = 3255 || n = 3257 || n = 3259 || (n > 3260 && n < 3268) || n = 3269 || n = 3271 || n = 3273 || (n > 3274 && n < 3282))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3128)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3131)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3132 || n = 3146 || n = 3174 || n = 3188 || n = 3202 || n = 3244 || n = 3272)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3133)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif((n .mod 14 = 12) && (n = 3134 || n = 3148 || (n > 3175 && n < 3219) || (n > 3245 && n < 3275)))
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3136)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3138 || n = 3152 || n = 3213 || n = 3253)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3142 || n = 3160 || n = 3178 || n = 3196 || n = 3214 || n = 3232 || n = 3250 || n = 3268)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3194)
	; 12 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3208)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #12
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3210)
	; 12 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	tay
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
.if(n = 3282 || n = 3283 || n = 3285 || n = 3287 || (n > 3288 && n < 3296) || n = 3297)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 3288 || n = 3306)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3296)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3298)
	; 13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3303 || n = 3305 || n = 3307 || n = 3315)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3304)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
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
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 3317 || n = 3319)
	; 13 bytes
	delay_n (n-13)
	brk 0
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
.if(n = 3321 || n = 3331 || n = 3333 || n = 3343 || n = 3345 || n = 3351 || n = 3356)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3322 || n = 3340)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3323)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	delay_n 7
	iny
	Jne *-6
	tay
	plp
.elseif(n = 3324 || n = 3342)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 3328)
	; 11 bytes
	php
	clc
	lda #90 ;hides 'nop'
	brk 0
	adc #1
	Jne *-5
	plp
.elseif(n = 3329 || n = 3346 || n = 3350 || n = 3352 || n = 3355 || n = 3357 || n = 3358)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3330)
	; 11 bytes
	php
	tya
	ldy #90 ;hides 'nop'
	brk 0
	iny
	Jne *-4
	tay
	plp
.elseif(n = 3332)
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3334)
	; 13 bytes
	php
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
	; 9..13 bytes
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
.elseif(n = 3341)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3344)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3347)
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
.elseif(n = 3348)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
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
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3361 || n = 3366 || n = 3373 || n = 3374 || n = 3379)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3362 || n = 3370)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363 || n = 3368)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3364 || n = 3369)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3365)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3367)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3371)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 3372)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3375 || n = 3404 || n = 3408)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3376 || n = 3394 || n = 3412 || n = 3430)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3377)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3378 || n = 3396 || n = 3414 || n = 3432)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif((n > 3379 && n < 3384) || n = 3385 || n = 3388 || n = 3390 || n = 3391 || n = 3393 || n = 3395 || n = 3397 || n = 3399 || n = 3401 || n = 3403 || (n > 3404 && n < 3408) || (n > 3408 && n < 3412) || n = 3413 || n = 3415 || n = 3417 || (n > 3418 && n < 3426) || n = 3427 || n = 3429 || n = 3433 || (n > 3434 && n < 3438))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3384 || n = 3398 || n = 3426)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3386 || n = 3400 || n = 3428)
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
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3389)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3392)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
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
.elseif(n = 3416)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3418)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	plp
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
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if(n = 3438 || n = 3439 || n = 3441 || (n > 3442 && n < 3446) || n = 3447 || n = 3449 || (n > 3450 && n < 3454) || n = 3455 || (n > 3456 && n < 3461) || n = 3462 || n = 3463 || n = 3465 || n = 3467 || (n > 3470 && n < 3480) || n = 3481 || n = 3483 || n = 3485 || (n > 3486 && n < 3496) || n = 3497 || (n > 3498 && n < 3502) || n = 3503 || (n > 3504 && n < 3508) || n = 3511 || (n > 3512 && n < 3516))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3440 || n = 3454 || n = 3496)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3442 || n = 3456 || n = 3470 || n = 3498 || n = 3512)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3446)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3448 || n = 3466 || n = 3484 || n = 3502)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3450 || n = 3468 || n = 3486 || n = 3504)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
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
.elseif(n = 3464)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 3469 || n = 3509)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
; 3516 to 3593
.if((n > 3515 && n < 3520) || n = 3521 || n = 3523 || n = 3525 || (n > 3526 && n < 3538) || n = 3539 || (n > 3540 && n < 3552) || n = 3553)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3520 || n = 3538 || n = 3556 || n = 3574 || n = 3592)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3522 || n = 3540 || n = 3558 || n = 3576)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3524 || n = 3552 || n = 3566 || n = 3580)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3526 || n = 3554 || n = 3568 || n = 3582)
	; 12 bytes
	php
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3555)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3557 || n = 3559 || n = 3561 || n = 3573 || n = 3577 || n = 3579 || n = 3593)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3560 || n = 3578 || n = 3586)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3569 || n = 3571 || n = 3587 || n = 3589)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3570 || n = 3590)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3572)
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3575)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 3581)
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
.elseif(n = 3583)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
	plp
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
.elseif(n = 3585)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 3588)
	; 13 bytes
	php
	tya
	ldy #100 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	iny
	Jne *-6
	tay
	plp
.elseif(n = 3591)
	; 13 bytes
	php
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	plp
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3630)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3595 || n = 3600 || n = 3602 || n = 3607 || n = 3612)
	; 8..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3598 || n = 3605 || n = 3606 || n = 3608 || n = 3611 || n = 3613 || n = 3614 || n = 3618 || n = 3626)
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3609)
	; 7 bytes
	php
	lda #14
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3610 || n = 3615)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3616)
	; 9 bytes
	php
	pha
	lda #14
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3617 || n = 3622 || n = 3629)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3619 || n = 3624)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3620 || n = 3625)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3621)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3623)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3627)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 3628)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3631 || n = 3632)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3634 || n = 3650 || n = 3660)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3635)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif((n > 3635 && n < 3640) || n = 3641 || n = 3642 || n = 3644 || n = 3647 || n = 3649 || (n > 3650 && n < 3660) || (n > 3660 && n < 3664) || n = 3665 || (n > 3667 && n < 3672))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3640)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3643)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3645)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3646)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3648)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3664)
	; 10 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	brk 0
	Jne *-3
	tax
	plp
.elseif(n = 3666)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3667)
	; 11 bytes
	pha
	delay_n (n-3)
.endif
.endif
.else
; 3672 to 3750
.if((n > 3671 && n < 3682) || n = 3683 || (n > 3684 && n < 3688) || n = 3689 || (n > 3690 && n < 3700) || n = 3701 || (n > 3702 && n < 3706) || (n > 3706 && n < 3718) || n = 3719 || n = 3721 || n = 3723 || n = 3724 || (n > 3725 && n < 3736) || n = 3737 || (n > 3738 && n < 3751))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3682 || n = 3700 || n = 3718 || n = 3736)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3684 || n = 3702 || n = 3720 || n = 3738)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3688)
	; 11 bytes
	php
	sec
	lda #184 ;hides 'clv'
	brk 0
	sbc #1
	Jne *-5
	plp
.elseif(n = 3690)
	; 11 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	brk 0
	dey
	Jne *-4
	tay
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
.elseif(n = 3725)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
.if(n = 3751 || n = 3753 || n = 3755 || (n > 3756 && n < 3765) || (n > 3765 && n < 3772) || n = 3773 || (n > 3774 && n < 3790) || n = 3791 || (n > 3792 && n < 3808) || n = 3809)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3752)
	; 12 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 3754 || n = 3772 || n = 3790 || n = 3808 || n = 3826)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3756 || n = 3774 || n = 3792 || n = 3810 || n = 3828)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3765)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3811 || n = 3813)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3812)
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3814)
	; 13 bytes
	php
	tya
	ldy #200
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
	nop
.elseif(n = 3815)
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
.elseif(n = 3816)
	; 12 bytes
	php
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	brk 0
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3817)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk 0
	dey
	Jne *-3
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
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
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
.elseif(n = 3821)
	; 13 bytes
	php
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
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
.elseif(n = 3827)
	; 14 bytes
	php
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 9
	inx
	Jne *-7
	tax
	plp
.endif
.else
.if((n >= 3829) && (n < 3868))
.if((n >= 3829) && (n < 3848))
; 3829 to 3847
.if(n = 3829)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3830)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3831)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3832)
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3833 || n = 3845)
	; 13..14 bytes
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
.elseif(n = 3835)
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
.elseif(n = 3837)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3838)
	; 13 bytes
	php
	sec
	lda #(n-10)/22
	nop
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 3839)
	; 13 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3840)
	; 13 bytes
	php
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3841)
	; 12 bytes
	php
	tya
	ldy #192 ;hides 'cpy #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3842)
	; 14 bytes
	php
	pha
	sec
	lda #225
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
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3846)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3847)
	; 12 bytes
	pha
	delay_n (n-3)
.endif
.else
; 3848 to 3867
.if(n = 3848)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3849 || n = 3863)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3850)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3851)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 7
	iny
	Jne *-6
	tay
	plp
.elseif(n = 3852)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3853)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 3854)
	; 12 bytes
	php
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	plp
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
.elseif(n = 3856 || n = 3858 || n = 3862 || n = 3864 || n = 3867)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3857 || n = 3859)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3860)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3861)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $AFA0'
	ldy #175
	brk 0
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3865)
	; 7 bytes
	php
	lda #15
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3866)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 3868 to 3906
.if(n = 3868)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3869 || n = 3870 || n = 3874 || n = 3882)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3871)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3872)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3873 || n = 3878 || n = 3885 || n = 3886 || n = 3891)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3875 || n = 3880)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3876 || n = 3881)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3877)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3879)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3883)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 3884)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3887 || n = 3888 || n = 3890 || n = 3906)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3889)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 3891 && n < 3896) || n = 3897 || n = 3902 || n = 3903 || n = 3905)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3896)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3898)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3899)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3900)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3901)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3904)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 3907 to 4062
.if((n > 3906 && n < 3916) || n = 3917 || n = 3919 || (n > 3920 && n < 3934) || n = 3935 || (n > 3936 && n < 3952) || n = 3953 || (n > 3954 && n < 3968) || n = 3969 || n = 3971 || (n > 3972 && n < 3976) || n = 3977 || n = 3979 || n = 3980 || (n > 3981 && n < 3988) || n = 3989 || (n > 3990 && n < 4006) || n = 4007 || n = 4009 || (n > 4010 && n < 4021) || n = 4022 || n = 4023 || n = 4025 || (n > 4026 && n < 4042) || n = 4043 || (n > 4044 && n < 4060) || n = 4061)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3916 || n = 3934 || n = 3952 || n = 3970 || n = 3988 || n = 4006 || n = 4024 || n = 4042 || n = 4060)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 3918 || n = 3936 || n = 3954 || n = 3972 || n = 3990 || n = 4008 || n = 4026 || n = 4044 || n = 4062)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 3920 || n = 3981 || n = 4021)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3968)
	; 11 bytes
	php
	clc
	lda #58 ;hides 'nop'
	brk 0
	adc #1
	Jne *-5
	plp
.elseif(n = 3976)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 9
	sbc #1
	Jne *-6
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
.elseif(n = 4010)
	; 11 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	brk 0
	iny
	Jne *-4
	tay
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4066 || n = 4067)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4068 || n = 4079 || n = 4081 || n = 4083 || n = 4099 || n = 4101)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4069 || n = 4087)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4070)
	; 11 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	tax
	plp
.elseif(n = 4071)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 4072)
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
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
.elseif(n = 4077 || n = 4082 || n = 4088 || n = 4094 || n = 4100)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4078 || n = 4096)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 4080 || n = 4098)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
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
	php
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4089)
	; 14 bytes
	php
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
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
.elseif(n = 4093)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4097)
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
; 4102 to 4140
.if(n = 4102)
	; 13 bytes
	php
	sec
	lda #(n-10)/22
	nop
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 4103)
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
.elseif(n = 4104)
	; 13 bytes
	php
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
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
	; 12 bytes
	php
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	brk 0
	dey
	Jne *-5
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
.elseif(n = 4110)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4111 || n = 4118 || n = 4120 || n = 4123 || n = 4125 || n = 4126 || n = 4130 || n = 4138)
	; 8..13 bytes
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
.elseif(n = 4113 || n = 4115 || n = 4117 || n = 4119 || n = 4124)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4114)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 4116)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4121)
	; 7 bytes
	php
	lda #16
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4122 || n = 4127)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4128)
	; 9 bytes
	php
	pha
	lda #16
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4129 || n = 4134)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4131 || n = 4136)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4132 || n = 4137)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4133)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4135)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4139)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 4140)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141 || n = 4142 || n = 4147)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4143 || n = 4144 || n = 4146 || n = 4162 || n = 4172 || n = 4176)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4145)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4148 || n = 4149 || n = 4151 || n = 4153 || n = 4154 || n = 4156 || n = 4158 || n = 4159 || n = 4161 || (n > 4162 && n < 4168) || n = 4169 || n = 4171 || (n > 4172 && n < 4176) || (n > 4176 && n < 4186) || n = 4187 || (n > 4188 && n < 4204) || n = 4205 || (n > 4206 && n < 4219))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4150 || n = 4168 || n = 4186 || n = 4204)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 4152 || n = 4170 || n = 4188 || n = 4206)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 4155)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4157)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4160)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if((n > 4218 && n < 4222) || n = 4223 || (n > 4224 && n < 4232) || (n > 4232 && n < 4237) || n = 4238 || n = 4239 || n = 4241 || (n > 4242 && n < 4258) || n = 4259 || (n > 4260 && n < 4276) || (n > 4278 && n < 4294) || n = 4295)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4222 || n = 4240 || n = 4258 || n = 4276 || n = 4294)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 4224 || n = 4242 || n = 4260 || n = 4278 || n = 4296)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 4232)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #16
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4237 || n = 4277)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if((n > 4296 && n < 4312) || n = 4313 || (n > 4314 && n < 4322))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4312 || n = 4330)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 4314 || n = 4332)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 4322)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4323)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4324 || n = 4326 || n = 4331 || n = 4333 || n = 4335)
	; 12..13 bytes
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
.elseif(n = 4327 || n = 4329 || n = 4334)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4328)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	brk 0
	sbc #1
	Jne *-5
	plp
.endif
.else
; 4336 to 4375
.if(n = 4336 || n = 4357)
	; 13 bytes
	php
	sec
	lda #(n-10)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 4337 || n = 4355)
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
.elseif(n = 4339)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4340)
	; 14 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	plp
.elseif(n = 4341 || n = 4343)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4342 || n = 4361)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4344 || n = 4363)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4345)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4346 || n = 4349 || n = 4354 || n = 4365 || n = 4374)
	; 8..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4348 || n = 4366)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 4350 || n = 4368)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 2 = 1) && (n = 4351 || n = 4353 || (n > 4368 && n < 4376)))
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4352)
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4356)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4358)
	; 13 bytes
	php
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	plp
.elseif(n = 4360)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
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
.elseif(n = 4370)
	; 11 bytes
	php
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4372)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4379 || n = 4381 || n = 4382 || n = 4386 || n = 4394)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377)
	; 7 bytes
	php
	lda #17
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4378 || n = 4383)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4380)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4384)
	; 9 bytes
	php
	pha
	lda #17
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4385 || n = 4390 || n = 4397 || n = 4398 || n = 4403)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4387 || n = 4392)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4388 || n = 4393)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4389)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4391)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4395)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 4396)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4399 || n = 4400 || n = 4418 || n = 4428 || n = 4432 || n = 4493)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4401)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4402 || n = 4420 || n = 4438 || n = 4456 || n = 4474 || n = 4492 || n = 4510 || n = 4528)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 4404 || n = 4422 || n = 4440 || n = 4458 || n = 4476 || n = 4494 || n = 4512 || n = 4530)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif((n > 4404 && n < 4408) || n = 4409 || n = 4410 || n = 4412 || n = 4414 || n = 4415 || n = 4417 || n = 4419 || n = 4421 || (n > 4422 && n < 4428) || (n > 4428 && n < 4432) || (n > 4432 && n < 4438) || n = 4439 || (n > 4440 && n < 4456) || n = 4457 || (n > 4458 && n < 4474) || n = 4475 || (n > 4476 && n < 4488) || (n > 4488 && n < 4492) || (n > 4494 && n < 4510) || n = 4511 || (n > 4512 && n < 4528) || n = 4529 || n = 4531)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4408)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4411)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4413)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4416)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4488)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #17
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if(n = 4532 || (n > 4533 && n < 4546) || n = 4547 || (n > 4548 && n < 4564) || n = 4565 || (n > 4566 && n < 4578))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4533)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4546 || n = 4564 || n = 4582 || n = 4600)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 4548 || n = 4566 || n = 4584 || n = 4602)
	; 11 bytes
	php
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	plp
.elseif(n = 4578 || n = 4579)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4580 || n = 4585 || n = 4587 || n = 4594 || n = 4601 || n = 4603 || n = 4605)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4581 || n = 4583)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
.elseif(n = 4589)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4590)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4591)
	; 10 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 4592)
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4593 || n = 4595 || n = 4596 || n = 4599 || n = 4604 || n = 4606)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4597)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4598)
	; 12 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4607)
	; 10 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 4608)
	; 11 bytes
	php
	clc
	lda #26 ;hides 'nop'
	brk 0
	adc #1
	Jne *-5
	plp
.elseif(n = 4609)
	; 10 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	tay
	plp
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610)
	; 11 bytes
	php
	tya
	ldy #26 ;hides 'nop'
	brk 0
	iny
	Jne *-4
	tay
	plp
.elseif(n = 4611 || n = 4613 || n = 4622 || n = 4624 || n = 4630 || n = 4632 || n = 4635 || n = 4637 || n = 4638 || n = 4642)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4621 || n = 4623 || n = 4631 || n = 4636)
	; 8..12 bytes
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
.elseif(n = 4615 || n = 4626 || n = 4634 || n = 4639)
	; 9..13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4616)
	; 12 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4617)
	; 13 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	brk 0
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4618)
	; 11 bytes
	php
	sec
	lda #0
	brk 0
	sbc #1
	Jne *-4
	plp
.elseif(n = 4619 || n = 4645)
	; 10..13 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4620)
	; 11 bytes
	php
	tya
	ldy #0
	brk 0
	dey
	Jne *-3
	tay
	plp
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4629)
	; 12 bytes
	php
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4633)
	; 7 bytes
	php
	lda #18
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4640)
	; 9 bytes
	php
	pha
	lda #18
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4641 || n = 4646)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4643 || n = 4648)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4644)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4647)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.else
; 4649 to 4687
.if(n = 4649)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4650)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4651)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 4652)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4653 || n = 4654 || n = 4659)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4655 || n = 4656 || n = 4658 || n = 4674 || n = 4684)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4657)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 4659 && n < 4664) || n = 4665 || n = 4666 || n = 4668 || n = 4670 || n = 4671 || n = 4673 || (n > 4674 && n < 4684) || (n > 4684 && n < 4688))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4664)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4667)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4669)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4672)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
; 4688 to 4843
.if(n = 4688)
	; 11 bytes
	php
	sec
	lda #(n-8)/20 ;hides 'nop'
	brk 0
	sbc #1
	Jne *-5
	plp
.elseif(n = 4689 || (n > 4690 && n < 4744) || (n > 4744 && n < 4749) || (n > 4749 && n < 4789) || (n > 4789 && n < 4834))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4690)
	; 11 bytes
	php
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4744)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #18
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4749 || n = 4789)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4834 || n = 4835)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
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
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
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
.if(n = 4844)
	; 14 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	adc #1
	Jne *-8
	plp
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
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
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
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
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
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
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
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4853 || n = 4858)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4854 || n = 4856 || n = 4859)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4855)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
.elseif(n = 4860)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
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
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
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
	cmp $C5
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 4866 || n = 4875)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4867)
	; 15 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4868)
	; 14 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4869)
	; 15 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4870)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4871)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 11
	iny
	Jne *-8
	tay
	plp
.elseif(n = 4872)
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 4878)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4879)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.if(n = 4883 || n = 4887 || n = 4892)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4884)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4885)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 4886 || n = 4888 || n = 4891 || n = 4893 || n = 4894 || n = 4898 || n = 4906)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4889)
	; 7 bytes
	php
	lda #19
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4890 || n = 4895)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4896)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4897 || n = 4902 || n = 4909 || n = 4910 || n = 4915)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4899 || n = 4904)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4900 || n = 4905)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4901)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4903)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4907)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 4908)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4911 || n = 4912 || n = 4914)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4913)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 4915 && n < 4920) || n = 4921)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4920)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 4922 to 5000
.if(n = 4922 || n = 4924 || n = 4926 || n = 4927 || n = 4929 || (n > 4930 && n < 4940) || (n > 4940 && n < 4944) || (n > 4944 && n < 4968) || n = 4969 || (n > 4970 && n < 5000))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4923)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4925)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4928)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4930 || n = 4940 || n = 4944)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4968)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	brk 0
	sbc #1
	Jne *-5
	plp
.elseif(n = 4970)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jne *-4
	tay
	plp
.elseif(n = 5000)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #19
	jsr delay_256a_16_clocks
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
.if((n > 5000 && n < 5005) || n = 5006 || n = 5007 || n = 5009 || (n > 5010 && n < 5045) || (n > 5045 && n < 5079))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5005 || n = 5045)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5008)
	; 11 bytes
	php
	sec
	lda #(n-8)/20 ;hides 'nop'
	brk 0
	sbc #1
	Jne *-5
	plp
.elseif(n = 5010)
	; 11 bytes
	php
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	tay
	plp
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if((n > 5078 && n < 5090))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5090 || n = 5110)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5091)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5092 || n = 5112)
	; 12 bytes
	php
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 5093)
	; 13 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	brk 0
	iny
	Jne *-6
	tay
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
.elseif(n = 5095)
	; 13 bytes
	php
	sec
	lda #212 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	brk 0
	sbc #1
	Jne *-7
	plp
.elseif(n = 5096)
	; 13 bytes
	php
	sec
	lda #(n-8)/24 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5097)
	; 13 bytes
	php
	tya
	ldy #(n-9)/24 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk 0
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5098)
	; 13 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5099)
	; 14 bytes
	php
	pha
	tya
	ldy #254
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
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
	; 11 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	plp
.elseif(n = 5102)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5103 || n = 5105 || n = 5106)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5104 || n = 5117)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5107)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5108)
	; 13 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 5111)
	; 13 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	nop
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
	; 12 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
	plp
.elseif(n = 5116)
	; 13 bytes
	php
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	plp
.endif
.else
; 5118 to 5156
.if(n = 5118)
	; 11 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	sbc #1
	Jne *-5
	plp
.elseif(n = 5119)
	; 11 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 5120)
	; 11 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	plp
.elseif(n = 5121)
	; 11 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 5122 || n = 5123 || n = 5129 || n = 5134 || n = 5142 || n = 5144 || n = 5147 || n = 5149 || n = 5150 || n = 5154)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5124 || n = 5143 || n = 5148)
	; 8..12 bytes
	pha
	delay_n (n-3)
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
	; 13 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 5128)
	; 13 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 5130)
	; 12 bytes
	php
	sec
	lda #0
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5131 || n = 5133 || n = 5153)
	; 9..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5132)
	; 12 bytes
	php
	tya
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	plp
.elseif(n = 5135)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 5136)
	; 13 bytes
	php
	sec
	lda #(n-10)/22
	nop
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5137)
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5138)
	; 13 bytes
	php
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5139)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 5140)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5141)
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
.elseif(n = 5145)
	; 7 bytes
	php
	lda #20
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5146 || n = 5151)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5152)
	; 9 bytes
	php
	pha
	lda #20
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5155)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5156)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.endif
.else
; 5157 to 5312
.if(n = 5157)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5158 || n = 5165 || n = 5166 || n = 5171)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5159)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5160)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5161)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5162)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5163)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 5164)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5167 || n = 5168 || n = 5170 || n = 5186 || n = 5196 || n = 5200 || n = 5261 || n = 5301)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5169)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 5171 && n < 5176) || n = 5177 || n = 5178 || n = 5180 || n = 5182 || n = 5183 || n = 5185 || (n > 5186 && n < 5196) || (n > 5196 && n < 5200) || (n > 5200 && n < 5256) || (n > 5256 && n < 5261) || (n > 5261 && n < 5301) || (n > 5301 && n < 5313))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5176)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5179)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5181)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5184)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5256)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #20
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5346 || n = 5347)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5348)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 5348 && n < 5352))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5353)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5354)
	; 13 bytes
	php
	clc
	lda #58 ;hides 'nop'
	delay_n 7
	brk 0
	adc #1
	Jne *-7
	plp
.elseif(n = 5355)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5356)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	bit $24
	iny
	Jne *-5
	tay
	plp
.elseif(n = 5357)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	brk 0
	iny
	Jne *-6
	tay
	plp
.elseif(n = 5358)
	; 13 bytes
	php
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5359 || n = 5361 || n = 5368 || n = 5370)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5360)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 5362)
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5363)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
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
	; 12 bytes
	php
	sec
	lda #244 ;hides 'nop zp,x'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
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
	; 12 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5369)
	; 13 bytes
	delay_n (n-2)
	nop
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
	; 14 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5373)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 5374)
	; 12 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
	plp
.elseif(n = 5375)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	bit $24
	sbc #1
	Jne *-6
	plp
.elseif(n = 5376)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5377)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	bit $24
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5378)
	; 12 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5379 || n = 5382)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5380)
	; 13 bytes
	php
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5381)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5383)
	; 14 bytes
	php
	sec
	lda #(n-10)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
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
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
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
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
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
.elseif(n = 5389)
	; 14 bytes
	php
	sec
	lda #163
	nop
	delay_n 26
	sbc #1
	Jne *-7
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
.if(n = 5391)
	; 14 bytes
	php
	tya
	ldy #163
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
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
.elseif(n = 5393)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
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
.elseif(n = 5395 || n = 5399 || n = 5404)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5396)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
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
.elseif(n = 5398 || n = 5400 || n = 5403 || n = 5405 || n = 5406 || n = 5410 || n = 5418)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5401)
	; 7 bytes
	php
	lda #21
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5402 || n = 5407)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5408)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5409 || n = 5414 || n = 5421 || n = 5422 || n = 5427)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5411 || n = 5416)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5412 || n = 5417)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5413)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5415)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5419)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 5420)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5423 || n = 5424 || n = 5426 || n = 5442 || n = 5452 || n = 5456)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5425)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 5427 && n < 5432) || n = 5433 || n = 5434 || n = 5436 || n = 5438 || n = 5439 || n = 5441 || (n > 5442 && n < 5452) || (n > 5452 && n < 5456) || (n > 5456 && n < 5469))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5432)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5435)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5437)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5440)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 5469 to 5625
.if((n > 5468 && n < 5512) || (n > 5512 && n < 5517) || (n > 5517 && n < 5557) || (n > 5557 && n < 5602))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5512)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #21
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5517 || n = 5557)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5602 || n = 5603)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5604 || n = 5614)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 5604 && n < 5609))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5609)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5610)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 5611)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	plp
.elseif(n = 5612)
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5613 || n = 5616 || n = 5625)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5615)
	; 14 bytes
	php
	sec
	lda #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	sbc #1
	Jne *-8
	plp
.elseif(n = 5617)
	; 14 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	plp
.elseif(n = 5618)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
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
	lda #(n-10)/22
	nop
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5621)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	brk 0
	iny
	Jne *-6
	tay
	plp
.elseif(n = 5622)
	; 13 bytes
	php
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5623)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5624)
	; 13 bytes
	php
	sec
	lda #234 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
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
	; 13 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5627)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
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
	; 12 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	adc #1
	Jne *-6
	plp
.elseif(n = 5630)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	plp
.elseif(n = 5631)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	nop
	sbc #1
	Jne *-6
	plp
.elseif(n = 5632)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5633)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5634 || n = 5641)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5635)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 5636)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5637)
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
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
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5640)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5642)
	; 13 bytes
	php
	sec
	lda #0
	nop
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5643)
	; 13 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	brk 0
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5644)
	; 13 bytes
	php
	tya
	ldy #0
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
.elseif(n = 5646 || n = 5654 || n = 5656 || n = 5659 || n = 5661 || n = 5662)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5647)
	; 14 bytes
	php
	tya
	ldy #(n-12)/23
	cmp $C5
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5648 || n = 5655 || n = 5660)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5649)
	; 15 bytes
	php
	pha
	sec
	lda #0
	nop
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5650)
	; 13 bytes
	php
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	plp
.elseif(n = 5651)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	plp
.elseif(n = 5652)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5653)
	; 14 bytes
	php
	sec
	lda #(n-10)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5657)
	; 7 bytes
	php
	lda #22
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 5665 to 5703
.if(n = 5665 || n = 5670 || n = 5677 || n = 5678 || n = 5683)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5666 || n = 5674)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5667 || n = 5672)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5668 || n = 5673)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5669)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5671)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5675)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 5676)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5679 || n = 5680 || n = 5682 || n = 5698)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5681)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 5683 && n < 5688) || n = 5689 || n = 5690 || n = 5692 || n = 5694 || n = 5695 || n = 5697 || (n > 5698 && n < 5704))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5688)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5691)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5693)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5696)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 5704 to 5781
.if((n > 5703 && n < 5708) || (n > 5708 && n < 5712) || (n > 5712 && n < 5768) || (n > 5768 && n < 5773) || (n > 5773 && n < 5782))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5708 || n = 5712 || n = 5773)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5768)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #22
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if((n > 5781 && n < 5813) || (n > 5813 && n < 5858))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5813)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5858 || n = 5859)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 5860) && (n < 5899))
.if((n >= 5860) && (n < 5879))
; 5860 to 5878
.if(n = 5860 || n = 5872 || n = 5874)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5861)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5862)
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5863)
	; 13 bytes
	php
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk 0
	sbc #1
	Jne *-7
	plp
.elseif(n = 5864)
	; 13 bytes
	php
	sec
	lda #(n-8)/24 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 5865)
	; 13 bytes
	php
	tya
	ldy #(n-9)/24 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk 0
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5866)
	; 13 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5867)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-257)/255
	jsr delay_256a_16_clocks
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
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 5870)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5871)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5873 || n = 5878)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5875)
	; 14 bytes
	php
	sec
	lda #255
	bit $24
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
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
	cmp $C5
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.endif
.else
; 5879 to 5898
.if(n = 5879)
	; 15 bytes
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
	; 15 bytes
	php
	sec
	lda #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts
	sbc #1
	Jne *-9
	plp
.elseif(n = 5882)
	; 15 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5883)
	; 15 bytes
	php
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts
	dey
	Jne *-8
	tay
	plp
.elseif(n = 5884)
	; 14 bytes
	php
	sec
	lda #178
	nop
	delay_n 26
	sbc #1
	Jne *-7
	plp
.elseif(n = 5885)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 5886)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	bit $24
	sbc #1
	Jne *-7
	plp
.elseif(n = 5887)
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5888)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	bit $24
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5889)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	clv
	Jvc *+2
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5890)
	; 13 bytes
	php
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 5891)
	; 14 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
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
.elseif(n = 5893)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5894)
	; 13 bytes
	php
	sec
	lda #218 ;hides 'nop'
	delay_n 7
	brk 0
	sbc #1
	Jne *-7
	plp
.elseif(n = 5895)
	; 15 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	cmp $C5
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5896)
	; 13 bytes
	php
	tya
	ldy #218 ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-6
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
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
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
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 5901 || n = 5904 || n = 5910 || n = 5912 || n = 5915 || n = 5917 || n = 5918 || n = 5922 || n = 5930)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5902)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5903 || n = 5905 || n = 5911 || n = 5916)
	; 8..14 bytes
	pha
	delay_n (n-3)
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
	jsr delay_256a_11_clocks_
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5914 || n = 5919)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5920)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5921 || n = 5926 || n = 5933 || n = 5934)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5923 || n = 5928)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5924 || n = 5929)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5925)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5927)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5931)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 5932)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5935 || n = 5936)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5937)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if(n = 5938 || n = 5954 || n = 5964 || n = 5968 || n = 6029 || n = 6069)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5939)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif((n > 5939 && n < 5944) || n = 5945 || n = 5946 || n = 5948 || n = 5950 || n = 5951 || n = 5953 || (n > 5954 && n < 5964) || (n > 5964 && n < 5968) || (n > 5968 && n < 6024) || (n > 6024 && n < 6029) || (n > 6029 && n < 6069) || (n > 6069 && n < 6094))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5944)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5947)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5949)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5952)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6024)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #23
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if((n > 6093 && n < 6114))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6114 || n = 6115)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6116)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 6117)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 6118 || n = 6120)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6119)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 6121)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	nop
	iny
	Jne *-6
	tay
	plp
.elseif(n = 6122)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-257)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6123)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6124)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6125)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6126)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6127)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6128)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6129)
	; 14 bytes
	php
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6130)
	; 14 bytes
	php
	sec
	lda #255
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 6131)
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
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
	; 14 bytes
	php
	clc
	lda #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	delay_n 4
	adc #1
	Jne *-8
	plp
.elseif(n = 6134 || n = 6147)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6135)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 6136)
	; 15 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	plp
.elseif(n = 6137)
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 6138 || n = 6140)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6139)
	; 14 bytes
	php
	sec
	lda #(n-10)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 6141)
	; 13 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	nop
	adc #1
	Jne *-7
	plp
.elseif(n = 6142)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-7
	plp
.elseif(n = 6143)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.repeat 4
	 nop
	.endrepeat
	sbc #1
	Jne *-7
	plp
.elseif(n = 6144)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 4
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6145)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	tay
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
.elseif(n = 6148)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
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
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 6151)
	; 14 bytes
	php
	sec
	lda #212 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk 0
	sbc #1
	Jne *-8
	plp
.endif
.else
; 6152 to 6171
.if(n = 6152 || n = 6157 || n = 6166 || n = 6168 || n = 6171)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6153)
	; 14 bytes
	php
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk 0
	dey
	Jne *-7
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
	; 14 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
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
.elseif(n = 6158)
	; 14 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6159)
	; 15 bytes
	php
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 7
	brk 0
	inx
	Jne *-8
	tax
	plp
.elseif(n = 6160)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 6161)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	plp
.elseif(n = 6162)
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 6163 || n = 6165 || n = 6167)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6164)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6169)
	; 7 bytes
	php
	lda #24
	jsr delay_256a_16_clocks
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
.if(n = 6172)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6173 || n = 6174 || n = 6178 || n = 6186)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6175)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6176)
	; 9 bytes
	php
	pha
	lda #24
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6177 || n = 6182 || n = 6189 || n = 6190 || n = 6195)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6179 || n = 6184)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6180 || n = 6185)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6181)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6183)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6187)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 6188)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6191 || n = 6192 || n = 6194 || n = 6210 || n = 6220 || n = 6224)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6193)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 6195 && n < 6200) || n = 6201 || n = 6202 || n = 6204 || n = 6206 || n = 6207 || n = 6209 || (n > 6210 && n < 6220) || (n > 6220 && n < 6224) || (n > 6224 && n < 6251))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6200)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6203)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6205)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6208)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
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
.if((n > 6250 && n < 6280) || (n > 6280 && n < 6285) || (n > 6285 && n < 6325) || (n > 6325 && n < 6329))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6280)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #24
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6285 || n = 6325)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 6329 to 6406
.if((n > 6328 && n < 6370))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6370 || n = 6371)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6372 || n = 6379 || n = 6388 || n = 6397 || n = 6402 || n = 6404)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif((n > 6372 && n < 6376))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6376)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	plp
.elseif(n = 6377)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-257)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6378 || n = 6389 || n = 6400 || n = 6403 || n = 6405)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6380 || n = 6406)
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
	; 14 bytes
	php
	tya
	ldy #193
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6382)
	; 14 bytes
	php
	sec
	lda #(n-10)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 6383)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 6384)
	; 14 bytes
	php
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6385)
	; 13 bytes
	php
	sec
	lda #(n-10)/25
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
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
.elseif(n = 6387)
	; 13 bytes
	php
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 6390)
	; 14 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk 0
	dey
	Jne *-7
	tay
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
	; 14 bytes
	php
	tya
	ldy #68 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	iny
	Jne *-7
	tay
	plp
.elseif(n = 6393)
	; 15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6394)
	; 12 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	plp
.elseif(n = 6395)
	; 14 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6396)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	plp
.elseif(n = 6398)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 6399)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 6401)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	dey
	Jne *-5
	tay
	plp
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
; 6407 to 6445
.if(n = 6407 || n = 6433 || n = 6438 || n = 6445)
	; 9..14 bytes
	delay_n (n-13)
	brk 0
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
	; 14 bytes
	php
	sec
	lda #(n-10)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 6410)
	; 13 bytes
	php
	sec
	lda #0
	delay_n 7
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 6411)
	; 14 bytes
	php
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6412)
	; 13 bytes
	php
	tya
	ldy #0
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 6413 || n = 6415 || n = 6423 || n = 6428)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6414)
	; 14 bytes
	php
	tya
	ldy #194
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6416 || n = 6422 || n = 6424 || n = 6427 || n = 6429 || n = 6430 || n = 6434 || n = 6442)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6417)
	; 15 bytes
	php
	tya
	ldy #183
	delay_n 30
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6418 || n = 6426 || n = 6431)
	; 9..15 bytes
	delay_n 6
	delay_n (n-6)
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
	jsr delay_256a_11_clocks_
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
.elseif(n = 6425)
	; 7 bytes
	php
	lda #25
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6432)
	; 9 bytes
	php
	pha
	lda #25
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6435 || n = 6440)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6436 || n = 6441)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6437)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6439)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6443)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 6444)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 6446 to 6484
.if(n = 6446 || n = 6451)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6447 || n = 6448 || n = 6450 || n = 6466 || n = 6476 || n = 6480)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6449)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 6451 && n < 6456) || n = 6457 || n = 6458 || n = 6460 || n = 6462 || n = 6463 || n = 6465 || (n > 6466 && n < 6476) || (n > 6476 && n < 6480) || (n > 6480 && n < 6485))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6456)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6459)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6461)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6464)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 6485 to 6562
.if((n > 6484 && n < 6536) || (n > 6536 && n < 6541) || (n > 6541 && n < 6563))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6536)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #25
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6541)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if((n > 6562 && n < 6581) || (n > 6581 && n < 6626))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6581)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6626 || n = 6627)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6628)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 6628 && n < 6633))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6633)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6634)
	; 14 bytes
	php
	tya
	ldy #201 ;hides 'cmp #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6635)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-260)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6636)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6637)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6638)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6639)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
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
	; 14 bytes
	php
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
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
	; 14 bytes
	php
	sec
	lda #201
	nop
	delay_n 26
	sbc #1
	Jne *-7
	plp
.elseif(n = 6644)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 6645)
	; 14 bytes
	php
	tya
	ldy #201
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6646)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 6647 || n = 6649)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6648 || n = 6650 || n = 6658)
	; 14..15 bytes
	delay_n (n-2)
	nop
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
	; 14 bytes
	php
	sec
	lda #(n-10)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 6653)
	; 14 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	delay_n 4
	adc #1
	Jne *-8
	plp
.elseif(n = 6654)
	; 14 bytes
	php
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6655)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.repeat 5
	 nop
	.endrepeat
	sbc #1
	Jne *-8
	plp
.elseif(n = 6656)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 4
	nop
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6657)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.repeat 5
	 nop
	.endrepeat
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6659)
	; 15 bytes
	delay_n (n-13)
	brk 0
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
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	delay_n 26
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6662)
	; 15 bytes
	php
	tya
	ldy #190
	delay_n 30
	dey
	Jne *-7
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
	; 14 bytes
	php
	tya
	ldy #196 ;hides 'cpy zp'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6665 || n = 6678)
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
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-8
	tay
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
.elseif(n = 6669 || n = 6671 || n = 6673 || n = 6679)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6670)
	; 14 bytes
	php
	sec
	lda #222
	jsr @nearby_rts
	brk 0
	sbc #1
	Jne *-7
	plp
.elseif(n = 6672)
	; 14 bytes
	php
	tya
	ldy #222
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
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
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 6676)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6677)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.endif
.endif
.else
; 6680 to 6718
.if(n = 6680 || n = 6683 || n = 6685 || n = 6686 || n = 6690 || n = 6698)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6681)
	; 7 bytes
	php
	lda #26
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6682 || n = 6687)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6684)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6688)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6689 || n = 6694 || n = 6701 || n = 6702 || n = 6707)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6691 || n = 6696)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6692 || n = 6697)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6693)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6695)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6699)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 6700)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6703 || n = 6704 || n = 6706)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6705)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 6707 && n < 6712) || n = 6713 || n = 6714 || n = 6716 || n = 6718)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6712)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6715)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6717)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
; 6719 to 6875
.if(n = 6719 || n = 6721 || (n > 6722 && n < 6732) || (n > 6732 && n < 6736) || (n > 6736 && n < 6792) || (n > 6792 && n < 6797) || (n > 6797 && n < 6837) || (n > 6837 && n < 6876))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6720)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6722 || n = 6732 || n = 6736 || n = 6797 || n = 6837)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6792)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #26
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6882 || n = 6883)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6884)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 6885 || n = 6887 || n = 6888)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6886)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
	plp
.elseif(n = 6889)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6890)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-260)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6891)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-261)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6892)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 6893)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6894)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.endif
.else
; 6895 to 6914
.if(n = 6895)
	; 14 bytes
	php
	sec
	lda #(n-10)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 6896 || n = 6899 || n = 6914)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6897)
	; 14 bytes
	php
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
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
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 6902)
	; 16 bytes
	php
	tya
	ldy #130
	pha
	sec
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
	lda #171
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
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
	; 15 bytes
	php
	sec
	lda #197
	delay_n 30
	sbc #1
	Jne *-8
	plp
.elseif(n = 6906)
	; 13 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	adc #1
	Jne *-7
	plp
.elseif(n = 6907)
	; 14 bytes
	php
	sec
	lda #209
	nop
	delay_n 26
	sbc #1
	Jne *-7
	plp
.elseif(n = 6908)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6909)
	; 14 bytes
	php
	tya
	ldy #209
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6910)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-7
	plp
.elseif(n = 6911)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 11
	sbc #1
	Jne *-7
	plp
.elseif(n = 6912)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6913)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 11
	dey
	Jne *-6
	tay
	plp
.endif
.endif
.else
; 6915 to 6953
.if(n = 6915 || n = 6927 || n = 6929 || n = 6934 || n = 6936 || n = 6939 || n = 6941 || n = 6942 || n = 6946)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6916 || n = 6926 || n = 6928 || n = 6935 || n = 6940)
	; 8..14 bytes
	pha
	delay_n (n-3)
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
	; 15 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6920)
	; 14 bytes
	php
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts
	brk 0
	sbc #1
	Jne *-8
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
	; 14 bytes
	php
	sec
	lda #0
	delay_n 7
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 6923)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 6924)
	; 14 bytes
	php
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 6925)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
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
.elseif(n = 6931 || n = 6938 || n = 6943)
	; 9..15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6932)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6933)
	; 15 bytes
	php
	clc
	lda #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	jsr @nearby_rts
	adc #1
	Jne *-9
	plp
.elseif(n = 6937)
	; 7 bytes
	php
	lda #27
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6944)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6945 || n = 6950)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6947 || n = 6952)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6948 || n = 6953)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6949)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6951)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6955)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 6956)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6957 || n = 6958 || n = 6963)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6959 || n = 6960 || n = 6962 || n = 6978 || n = 6988 || n = 6992)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6961)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 6963 && n < 6968) || n = 6969 || n = 6970 || n = 6972 || n = 6974 || n = 6975 || n = 6977 || (n > 6978 && n < 6988) || (n > 6988 && n < 6992) || (n > 6992 && n < 7032))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6968)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6971)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6973)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6976)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
; 7032 to 7109
.if((n > 7031 && n < 7048) || (n > 7048 && n < 7053) || (n > 7053 && n < 7093) || (n > 7093 && n < 7110))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7048)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #27
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7053 || n = 7093)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if((n > 7109 && n < 7138))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7138 || n = 7139)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7140)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 7141 || n = 7143 || n = 7144)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7142)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 7145)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-260)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7146)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-261)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7147)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-262)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7148)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 7149) && (n < 7168))
; 7149 to 7167
.if(n = 7149)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7150)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7151)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7152)
	; 14 bytes
	php
	tya
	ldy #238
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
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
	; 15 bytes
	php
	sec
	lda #188
	delay_n 7
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 7155)
	; 15 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7156)
	; 15 bytes
	php
	tya
	ldy #188
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7157)
	; 16 bytes
	php
	tya
	pha
	ldy #85
	lda #51
	jsr delay_a_25_clocks
	dey
	Jpl *-6
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
	lda #$4A ;hides 'lsr a'
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
	lda #13
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 7162)
	; 14 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	bit $24
	adc #1
	Jne *-8
	plp
.elseif(n = 7163)
	; 16 bytes
	php
	tya
	pha
	ldy #188
	lda #6
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7164)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	bit $24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7165)
	; 15 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	delay_n 4
	nop
	adc #1
	Jne *-9
	plp
.elseif(n = 7166)
	; 14 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	delay_n 7
	adc #1
	Jne *-8
	plp
.elseif(n = 7167)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	rol $26
	ror $26
	sbc #1
	Jne *-8
	plp
.endif
.else
; 7168 to 7187
.if(n = 7168)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	delay_n 7
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7169)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7170 || n = 7175 || n = 7177)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7171)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 7172 || n = 7174 || n = 7176 || n = 7181 || n = 7183)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7173)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 7178)
	; 14 bytes
	php
	sec
	lda #224
	delay_n 27
	sbc #1
	Jne *-7
	plp
.elseif(n = 7179)
	; 15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7180)
	; 14 bytes
	php
	sec
	lda #239
	jsr @nearby_rts
	brk 0
	sbc #1
	Jne *-7
	plp
.elseif(n = 7182)
	; 14 bytes
	php
	tya
	ldy #239
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	plp
.elseif(n = 7184)
	; 15 bytes
	php
	sec
	lda #211
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 7185)
	; 14 bytes
	php
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	plp
.elseif(n = 7186)
	; 15 bytes
	php
	tya
	ldy #211
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7187)
	; 15 bytes
	php
	tya
	ldy #205
	delay_n 30
	dey
	Jne *-7
	tay
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7189)
	; 16 bytes
	php
	tya
	pha
	ldy #29
	lda #207
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7190 || n = 7192 || n = 7195 || n = 7197 || n = 7198 || n = 7202 || n = 7210)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7191 || n = 7196)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 7193)
	; 7 bytes
	php
	lda #28
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7194 || n = 7199)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7200)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7201 || n = 7206 || n = 7213 || n = 7214 || n = 7219)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7203 || n = 7208)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7204 || n = 7209)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7205)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7207)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7211)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 7212)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7215 || n = 7216 || n = 7218 || n = 7234 || n = 7244 || n = 7248 || n = 7309)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7217)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 7219 && n < 7224) || n = 7225 || n = 7226 || n = 7228 || n = 7230 || n = 7231 || n = 7233 || (n > 7234 && n < 7244) || (n > 7244 && n < 7248) || (n > 7248 && n < 7304) || (n > 7304 && n < 7309) || (n > 7309 && n < 7344))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7224)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7227)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7229)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7232)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7304)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #28
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if((n > 7343 && n < 7349) || (n > 7349 && n < 7394))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7349)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7394 || n = 7395)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7396 || n = 7409)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 7396 && n < 7401))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7401)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-261)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7402)
	; 14 bytes
	php
	sec
	lda #224
	nop
	delay_n 26
	sbc #1
	Jne *-7
	plp
.elseif(n = 7403)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-263)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7404)
	; 14 bytes
	php
	tya
	ldy #224
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 7405)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7406)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7407)
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
.elseif(n = 7408)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7410)
	; 15 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	delay_n 30
	iny
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
	ldy #185
	pha
	lda #1
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7413)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
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
	ldy #85
	lda #54
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7416)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7417)
	; 14 bytes
	php
	sec
	lda #212 ;hides 'nop zp,x'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	brk 0
	sbc #1
	Jne *-8
	plp
.elseif(n = 7418)
	; 14 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	adc #1
	Jne *-8
	plp
.elseif(n = 7419)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 7420)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7421)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.endif
.else
.if((n >= 7422) && (n < 7461))
; 7422 to 7460
.if(n = 7422)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-8
	plp
.elseif(n = 7423 || n = 7427 || n = 7438 || n = 7439 || n = 7445 || n = 7446 || n = 7448 || n = 7451 || n = 7453 || n = 7454 || n = 7458)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7424)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7425)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 11
	dey
	Jne *-7
	tay
	plp
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
.elseif(n = 7428)
	; 14 bytes
	php
	sec
	lda #212 ;hides 'nop zp,x'
	nop
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 7429)
	; 15 bytes
	php
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	delay_n 26
	adc #1
	Jne *-9
	plp
.elseif(n = 7430)
	; 14 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7431)
	; 15 bytes
	php
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 26
	inx
	Jne *-8
	tax
	plp
.elseif(n = 7432)
	; 14 bytes
	php
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts
	brk 0
	adc #1
	Jne *-8
	plp
.elseif(n = 7433 || n = 7440 || n = 7447 || n = 7452)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7434)
	; 14 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	brk 0
	iny
	Jne *-7
	tay
	plp
.elseif(n = 7435)
	; 14 bytes
	php
	sec
	lda #225
	nop
	delay_n 26
	sbc #1
	Jne *-7
	plp
.elseif(n = 7436)
	; 14 bytes
	php
	tya
	ldy #232
	delay_n 27
	dey
	Jne *-6
	tay
	plp
.elseif(n = 7437)
	; 14 bytes
	php
	tya
	ldy #225
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 7441)
	; 16 bytes
	php
	tya
	pha
	ldy #58 ;hides 'nop'
	lda #94
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7449)
	; 7 bytes
	php
	lda #29
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7457)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7459)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7460)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.endif
.else
; 7461 to 7500
.if(n = 7461)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7462 || n = 7469 || n = 7470 || n = 7475)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7463)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7464)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7465)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7466)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7467)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 7468)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7471 || n = 7472 || n = 7474 || n = 7490 || n = 7500)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7473)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 7475 && n < 7480) || n = 7481 || n = 7482 || n = 7484 || n = 7486 || n = 7487 || n = 7489 || (n > 7490 && n < 7500))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7480)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7483)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7485)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7488)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
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
.if((n > 7500 && n < 7504) || (n > 7504 && n < 7560) || (n > 7560 && n < 7565) || (n > 7565 && n < 7605) || (n > 7605 && n < 7650))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7504 || n = 7565 || n = 7605)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7560)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7650 || n = 7651)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7652)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 7652 && n < 7657))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
.if((n >= 7657) && (n < 7676))
; 7657 to 7675
.if(n = 7657)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-262)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7658)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-263)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7659)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-264)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7660)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7661)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7662)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7663)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7664)
	; 13 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 26
	adc #1
	Jne *-7
	plp
.elseif(n = 7665 || n = 7670 || n = 7672)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7666)
	; 13 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	delay_n 26
	iny
	Jne *-6
	tay
	plp
.elseif(n = 7667)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 7668)
	; 14 bytes
	php
	tya
	ldy #232
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 7669)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 7671)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7673)
	; 15 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 26
	iny
	Jne *-6
	tay
	pla
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
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 7677)
	; 15 bytes
	php
	tya
	ldy #219
	delay_n 30
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7678)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	rol $26
	ror $26
	sbc #1
	Jne *-9
	plp
.elseif(n = 7679)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	sbc #1
	Jne *-7
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
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	plp
.elseif(n = 7682 || n = 7684 || n = 7693 || n = 7695)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7683 || n = 7685)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7686)
	; 15 bytes
	php
	sec
	lda #202
	delay_n 7
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 7687)
	; 15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7688)
	; 15 bytes
	php
	tya
	ldy #202
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7689)
	; 16 bytes
	php
	tya
	pha
	ldy #64
	lda #86
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7690)
	; 14 bytes
	php
	sec
	lda #0
	jsr @nearby_rts
	brk 0
	sbc #1
	Jne *-7
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
	; 14 bytes
	php
	tya
	ldy #0
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	plp
.elseif(n = 7694)
	; 15 bytes
	php
	sec
	lda #226
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-8
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
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7698)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 7699)
	; 14 bytes
	php
	sec
	lda #233
	nop
	delay_n 26
	sbc #1
	Jne *-7
	plp
.elseif(n = 7700)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7701)
	; 14 bytes
	php
	tya
	ldy #233
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 7702 || n = 7704 || n = 7707 || n = 7709 || n = 7710 || n = 7714 || n = 7722)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7703 || n = 7708)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 7705)
	; 7 bytes
	php
	lda #30
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7706 || n = 7711)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7712)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7713 || n = 7718 || n = 7725 || n = 7726 || n = 7731)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7715 || n = 7720)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7716 || n = 7721)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7717)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7719)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7723)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 7724)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7727 || n = 7728 || n = 7730)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7729)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 7731 && n < 7735))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 7735 to 7812
.if(n = 7735 || n = 7737 || n = 7738 || n = 7740 || n = 7742 || n = 7743 || n = 7745 || (n > 7746 && n < 7756) || (n > 7756 && n < 7760) || (n > 7760 && n < 7813))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7736)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7739)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7741)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7744)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7746 || n = 7756 || n = 7760)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
; 7813 to 7890
.if((n > 7812 && n < 7816) || (n > 7816 && n < 7821) || (n > 7821 && n < 7861) || (n > 7861 && n < 7891))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7816)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #30
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7821 || n = 7861)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if((n > 7890 && n < 7906))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7906)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk 0
	iny
	Jne *-5
	tay
	plp
.elseif(n = 7907)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7908 || n = 7909 || n = 7920 || n = 7927 || n = 7929)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif((n > 7909 && n < 7913))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7913)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk 0
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 7914)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-264)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7915)
	; 13 bytes
	php
	sec
	lda #(n-10)/31
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 7916)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7917)
	; 13 bytes
	php
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 7918)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7919 || n = 7921 || n = 7928)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7922)
	; 15 bytes
	php
	tya
	ldy #226
	delay_n 30
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7923)
	; 15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7924)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	sbc #1
	Jne *-6
	plp
.elseif(n = 7925)
	; 16 bytes
	php
	tya
	pha
	ldy #66
	lda #86
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7926)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	dey
	Jne *-5
	tay
	plp
.endif
.else
; 7930 to 7968
.if(n = 7930)
	; 14 bytes
	php
	sec
	lda #240
	nop
	delay_n 26
	sbc #1
	Jne *-7
	plp
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
	; 14 bytes
	php
	tya
	ldy #240
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
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
.elseif(n = 7934)
	; 15 bytes
	php
	tya
	ldy #233
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7935)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	brk 0
	sbc #1
	Jne *-6
	plp
.elseif(n = 7936)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 11
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7937)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
	dey
	Jne *-5
	tay
	plp
.elseif(n = 7938 || n = 7940 || n = 7945 || n = 7949 || n = 7951 || n = 7959 || n = 7964)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7939 || n = 7941 || n = 7950 || n = 7958 || n = 7960 || n = 7963 || n = 7965 || n = 7966)
	; 8..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7943 || n = 7962 || n = 7967)
	; 9..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7944)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 7946)
	; 13 bytes
	php
	sec
	lda #0
	delay_n 26
	sbc #1
	Jne *-6
	plp
.elseif(n = 7947)
	; 15 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7948)
	; 13 bytes
	php
	tya
	ldy #0
	delay_n 26
	dey
	Jne *-5
	tay
	plp
.elseif(n = 7952)
	; 15 bytes
	php
	sec
	lda #209
	delay_n 7
	delay_n 26
	sbc #1
	Jne *-8
	plp
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
	php
	tya
	ldy #209
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7955)
	; 15 bytes
	php
	sec
	lda #227
	delay_n 30
	sbc #1
	Jne *-8
	plp
.elseif(n = 7956)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7957)
	; 15 bytes
	php
	tya
	ldy #227
	delay_n 30
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7961)
	; 7 bytes
	php
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7968)
	; 9 bytes
	php
	pha
	lda #31
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
; 7969 to 8125
.if(n = 7969 || n = 7974 || n = 7981 || n = 7982 || n = 7987)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7970 || n = 7978)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7971 || n = 7976)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7972 || n = 7977)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7973)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7975)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7979)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 7980)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7983 || n = 7984 || n = 7986 || n = 8002 || n = 8012 || n = 8016 || n = 8077 || n = 8117)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7985)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 7987 && n < 7992) || n = 7993 || n = 7994 || n = 7996 || n = 7998 || n = 7999 || n = 8001 || (n > 8002 && n < 8012) || (n > 8012 && n < 8016) || (n > 8016 && n < 8072) || (n > 8072 && n < 8077) || (n > 8077 && n < 8117) || (n > 8117 && n < 8126))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7992)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7995)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7997)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8000)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8072)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #31
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8162 || n = 8163)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
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
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8169)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-264)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8170)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8171)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-266)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8172)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8173)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8174)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8175)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8176)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8177)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8178)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8179)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8180)
	; 15 bytes
	php
	sec
	lda #215
	delay_n 7
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 8181)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8182)
	; 15 bytes
	php
	tya
	ldy #215
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8183)
	; 15 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	delay_n 26
	dey
	Jne *-8
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
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8186)
	; 14 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	adc #1
	Jne *-8
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
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8189 || n = 8197)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8190)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-8
	plp
.elseif(n = 8191)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
.elseif(n = 8192)
	; 13 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 26
	sbc #1
	Jne *-7
	plp
.elseif(n = 8193)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	plp
.elseif(n = 8194)
	; 13 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 8195)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8196)
	; 14 bytes
	php
	tya
	ldy #248
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 8198)
	; 14 bytes
	php
	sec
	lda #234 ;hides 'nop'
	nop
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 8199)
	; 15 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	delay_n 26
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 8200)
	; 14 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8201)
	; 15 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 8202)
	; 14 bytes
	php
	sec
	lda #0
	delay_n 27
	sbc #1
	Jne *-7
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
	; 14 bytes
	php
	tya
	ldy #0
	delay_n 27
	dey
	Jne *-6
	tay
	plp
.elseif(n = 8205 || n = 8207 || n = 8225 || n = 8230 || n = 8237 || n = 8238)
	; 9..15 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8206)
	; 15 bytes
	php
	tya
	ldy #241
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8208)
	; 15 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	nop
	brk 0
	dey
	Jne *-8
	tay
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
	ldy #56 ;hides 'sec'
	pha
	lda #0
	jsr delay_a_25_clocks
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
	ldy #0
	lda #0
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8212)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8213)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #2
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8214 || n = 8216 || n = 8219 || n = 8221 || n = 8222 || n = 8226 || n = 8234)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8215 || n = 8220)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8217)
	; 7 bytes
	php
	lda #32
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8218 || n = 8223)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8224)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8227 || n = 8232)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8228 || n = 8233)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8229)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8231)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8235)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 8236)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8239 || n = 8240 || n = 8242)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8241)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 8243 to 8281
.if(n = 8243)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif((n > 8243 && n < 8248) || n = 8249 || n = 8250 || n = 8252 || n = 8254 || n = 8255 || n = 8257 || (n > 8258 && n < 8268) || (n > 8268 && n < 8272) || (n > 8272 && n < 8282))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8248)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8251)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8253)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8256)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8258 || n = 8268 || n = 8272)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
; 8282 to 8437
.if((n > 8281 && n < 8328) || (n > 8328 && n < 8333) || (n > 8333 && n < 8373) || (n > 8373 && n < 8418))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8328)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #32
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8333 || n = 8373)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8418 || n = 8419)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8420 || n = 8433)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 8420 && n < 8425))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8425)
	; 14 bytes
	php
	sec
	lda #255
	nop
	delay_n 26
	sbc #1
	Jne *-7
	plp
.elseif(n = 8426)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-266)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8427)
	; 14 bytes
	php
	tya
	ldy #255
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 8428)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8429)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8430)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8431)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8432)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8434)
	; 15 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8435)
	; 16 bytes
	php
	tya
	pha
	ldy #31
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8436)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	nop
	sbc #1
	Jne *-7
	plp
.elseif(n = 8437)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
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
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	nop
	dey
	Jne *-6
	tay
	plp
.elseif(n = 8439 || n = 8441 || n = 8452)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 8440 || n = 8450 || n = 8451)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8442)
	; 14 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 27
	dey
	Jne *-7
	tay
	plp
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
	; 15 bytes
	php
	tya
	ldy #248
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8445)
	; 15 bytes
	php
	sec
	lda #241
	delay_n 30
	sbc #1
	Jne *-8
	plp
.elseif(n = 8446)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk 0
	sbc #1
	Jne *-7
	plp
.elseif(n = 8447)
	; 13 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	brk 0
	sbc #1
	Jne *-7
	plp
.elseif(n = 8448)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk 0
	dey
	Jne *-6
	tay
	plp
.elseif(n = 8449)
	; 13 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	brk 0
	dey
	Jne *-6
	tay
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
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 8455)
	; 15 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	brk 0
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 8456)
	; 15 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	brk 0
	dey
	Jne *-6
	tay
	pla
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
	; 14 bytes
	php
	sec
	lda #0
	nop
	delay_n 26
	sbc #1
	Jne *-7
	plp
.elseif(n = 8459 || n = 8461)
	; 15 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8460)
	; 14 bytes
	php
	tya
	ldy #0
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	plp
.elseif(n = 8462 || n = 8469 || n = 8470 || n = 8472 || n = 8475)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8463 || n = 8471 || n = 8476)
	; 8..15 bytes
	pha
	delay_n (n-3)
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
	lda #110
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8473)
	; 7 bytes
	php
	lda #33
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8474)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8478 || n = 8482 || n = 8490)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8479)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8480)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8481 || n = 8486 || n = 8493 || n = 8494 || n = 8499)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8483 || n = 8488)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8484 || n = 8489)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8485)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8487)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8491)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 8492)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8495 || n = 8496 || n = 8498 || n = 8514)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8497)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 8499 && n < 8504) || n = 8505 || n = 8506 || n = 8508 || n = 8510 || n = 8511 || n = 8513 || n = 8515)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8504)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8507)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8509)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8512)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 8516 to 8593
.if((n > 8515 && n < 8524) || (n > 8524 && n < 8528) || (n > 8528 && n < 8584) || (n > 8584 && n < 8589) || (n > 8589 && n < 8594))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8524 || n = 8528 || n = 8589)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8584)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #33
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
; 8594 to 8671
.if((n > 8593 && n < 8629) || (n > 8629 && n < 8672))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8629)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 8672) && (n < 8711))
.if((n >= 8672) && (n < 8691))
; 8672 to 8690
.if(n = 8672 || n = 8673)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8674 || n = 8675)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8676 || n = 8689)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 8676 && n < 8681))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8681)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-266)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8682)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-267)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8683)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-268)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8684)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8685)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8686)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8687)
	; 15 bytes
	php
	tya
	pha
	ldy #197
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 8688)
	; 14 bytes
	php
	sec
	lda #248 ;hides 'sed'
	nop
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 8690)
	; 14 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.endif
.else
; 8691 to 8710
.if(n = 8691)
	; 15 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 26
	iny
	Jne *-8
	tay
	plp
.elseif(n = 8692)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	bit $24
	sbc #1
	Jne *-8
	plp
.elseif(n = 8693 || n = 8695 || n = 8697 || n = 8708)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8694)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	bit $24
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8696 || n = 8706 || n = 8707)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8698)
	; 15 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 9
	adc #1
	Jne *-9
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
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 9
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8701)
	; 15 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	jsr @nearby_rts
	adc #1
	Jne *-9
	plp
.elseif(n = 8702)
	; 14 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	brk 0
	adc #1
	Jne *-8
	plp
.elseif(n = 8703)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	bit $24
	brk 0
	sbc #1
	Jne *-8
	plp
.elseif(n = 8704)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8705)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	cmp $C5
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8709)
	; 16 bytes
	php
	tya
	pha
	ldy #79
	lda #78
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 8711 to 8750
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
	; 15 bytes
	php
	sec
	lda #229
	delay_n 7
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 8713)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	sec
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8714)
	; 15 bytes
	php
	sec
	lda #0
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-8
	plp
.elseif(n = 8715 || n = 8737 || n = 8742 || n = 8749 || n = 8750)
	; 9..16 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8716)
	; 15 bytes
	php
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
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
.elseif(n = 8718 || n = 8726 || n = 8728 || n = 8731 || n = 8733 || n = 8734 || n = 8738 || n = 8746)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8719)
	; 16 bytes
	php
	tya
	pha
	ldy #145
	lda #$4A ;hides 'lsr a'
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
	lda #81
	jsr delay_a_25_clocks
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
	lda #6
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8722)
	; 15 bytes
	php
	sec
	lda #198
	delay_n 39
	sbc #1
	Jne *-8
	plp
.elseif(n = 8723)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #2
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8724)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8725)
	; 15 bytes
	php
	sec
	lda #249
	delay_n 30
	sbc #1
	Jne *-8
	plp
.elseif(n = 8727 || n = 8732)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8729)
	; 7 bytes
	php
	lda #34
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8730 || n = 8735)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8736)
	; 9 bytes
	php
	pha
	lda #34
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8739 || n = 8744)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8740 || n = 8745)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8741)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8743)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8747)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 8748)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
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
.if(n = 8751 || n = 8752 || n = 8754 || n = 8770 || n = 8780 || n = 8784 || n = 8845 || n = 8885)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8753)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8755)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif((n > 8755 && n < 8760) || n = 8761 || n = 8762 || n = 8764 || n = 8766 || n = 8767 || n = 8769 || (n > 8770 && n < 8780) || (n > 8780 && n < 8784) || (n > 8784 && n < 8840) || (n > 8840 && n < 8845) || (n > 8845 && n < 8885) || (n > 8885 && n < 8907))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8760)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8763)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8765)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8768)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8840)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #34
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 8907) && (n < 8985))
.if((n >= 8907) && (n < 8946))
; 8907 to 8945
.if((n > 8906 && n < 8930))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8930 || n = 8931)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8932 || n = 8945)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 8932 && n < 8937))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8937)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-267)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8938)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-268)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8939)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-269)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8940)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8941)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8942)
	; 14 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
	tax
	plp
.elseif(n = 8943)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8944)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
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
	lda #81
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8947)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8948)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 4
	sbc #1
	Jne *-8
	plp
.elseif(n = 8949)
	; 16 bytes
	php
	tya
	pha
	ldy #235
	lda #6
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8950)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 4
	dey
	Jne *-7
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
.elseif(n = 8952 || n = 8962 || n = 8963)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8953 || n = 8964)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8954)
	; 16 bytes
	php
	tya
	ldy #33
	pha
	lda #224
	jsr delay_a_25_clocks
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
	lda #232
	jsr delay_a_25_clocks
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
	; 14 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	brk 0
	adc #1
	Jne *-8
	plp
.elseif(n = 8958)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	brk 0
	sbc #1
	Jne *-8
	plp
.elseif(n = 8959)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	nop
	brk 0
	sbc #1
	Jne *-8
	plp
.elseif(n = 8960)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8961)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	brk 0
	dey
	Jne *-7
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
	ldy #70
	lda #94
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8966)
	; 16 bytes
	php
	tya
	ldy #73
	pha
	lda #82
	jsr delay_a_25_clocks
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
	lda #6
	jsr delay_a_25_clocks
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
	lda #25
	jsr delay_a_25_clocks
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
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8970)
	; 15 bytes
	php
	sec
	lda #0
	delay_n 30
	sbc #1
	Jne *-8
	plp
.elseif(n = 8971)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 26
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8972)
	; 15 bytes
	php
	tya
	ldy #0
	delay_n 30
	dey
	Jne *-7
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
	lda #13
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8975)
	; 16 bytes
	php
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 27
	iny
	Jne *-9
	tay
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
	lda #16
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8978)
	; 15 bytes
	php
	sec
	lda #236
	delay_n 7
	delay_n 26
	sbc #1
	Jne *-8
	plp
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8981)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #132
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 8982 || n = 8984)
	; 8..9 bytes
	delay_n (n-2)
	nop
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8986 || n = 8991)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8987 || n = 8989 || n = 8990 || n = 8994 || n = 9002)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8992)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8993 || n = 8998 || n = 9005 || n = 9006 || n = 9011)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8995 || n = 9000)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8996 || n = 9001)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8997)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8999)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9003)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 9004)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9007 || n = 9008 || n = 9010 || n = 9026 || n = 9036 || n = 9040)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9009)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 9011 && n < 9016) || n = 9017 || n = 9018 || n = 9020 || n = 9022 || n = 9023 || n = 9025 || (n > 9026 && n < 9036) || (n > 9036 && n < 9040) || (n > 9040 && n < 9063))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9016)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9019)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9021)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9024)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
; 9063 to 9140
.if((n > 9062 && n < 9096) || (n > 9096 && n < 9101) || (n > 9101 && n < 9141))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9096)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9101)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if(n = 9141)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif((n > 9141 && n < 9180))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 9180) && (n < 9199))
; 9180 to 9198
.if((n > 9179 && n < 9186))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9186 || n = 9187)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9188)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 9188 && n < 9193))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9193)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-268)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9194)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-269)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9195)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-270)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9196)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9197)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9198)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 9199 to 9218
.if(n = 9199)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9200)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9201)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9202)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9203)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9204)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	clv
	Jvc *+2
	sbc #1
	Jne *-9
	plp
.elseif(n = 9205)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9206)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	clv
	Jvc *+2
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9207)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9208)
	; 15 bytes
	php
	tya
	ldy #209
	delay_n 39
	dey
	Jne *-7
	tay
	plp
.elseif(n = 9209)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9210)
	; 15 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	delay_n 39
	iny
	Jne *-8
	tay
	plp
.elseif(n = 9211)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
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
	; 15 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-9
	plp
.elseif(n = 9214)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	bit $24
	brk 0
	sbc #1
	Jne *-9
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
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	cmp $C5
	brk 0
	iny
	Jne *-8
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
	lda #167
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
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
	sec
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
	lda #169
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9221)
	; 16 bytes
	php
	tya
	pha
	ldy #85
	lda #75
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9222)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
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
	lda #72
	jsr delay_a_25_clocks
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
	lda #61
	jsr delay_a_25_clocks
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
	txa
	pha
	sec
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #229
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
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
	lda #62
	jsr delay_a_25_clocks
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
	lda #5
	jsr delay_a_25_clocks
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
	lda #239
	jsr delay_a_25_clocks
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
	sec
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
	jsr delay_256a_11_clocks_
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
.if(n = 9238 || n = 9240 || n = 9243 || n = 9245 || n = 9246 || n = 9250)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9239 || n = 9244)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 9241)
	; 7 bytes
	php
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9242 || n = 9247)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9248)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9249 || n = 9254)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9251 || n = 9256)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9252 || n = 9257)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9253)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9255)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 9258 to 9296
.if(n = 9258)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9259)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 9260)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9261 || n = 9262 || n = 9267)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9263 || n = 9264 || n = 9266 || n = 9282 || n = 9292 || n = 9296)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9265)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 9267 && n < 9272) || n = 9273 || n = 9274 || n = 9276 || n = 9278 || n = 9279 || n = 9281 || (n > 9282 && n < 9292) || (n > 9292 && n < 9296))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9272)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9275)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9277)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9280)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 9297 to 9375
.if((n > 9296 && n < 9352) || (n > 9352 && n < 9357) || (n > 9357 && n < 9376))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9352)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9357)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
.if((n > 9375 && n < 9397) || (n > 9397 && n < 9442))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9397)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9442 || n = 9443)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9444)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 9444 && n < 9449))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9449)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-269)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9450)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-270)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9451)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-271)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9452)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9453)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 9454) && (n < 9493))
.if((n >= 9454) && (n < 9473))
; 9454 to 9472
.if(n = 9454)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9455)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9456)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9457)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9458)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9459)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9460)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 4
	nop
	sbc #1
	Jne *-9
	plp
.elseif(n = 9461)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9462)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 4
	nop
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9463)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
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
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9466)
	; 15 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts
	adc #1
	Jne *-9
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
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9469)
	; 15 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	nop
	brk 0
	adc #1
	Jne *-9
	plp
.elseif(n = 9470)
	; 15 bytes
	php
	sec
	lda #215
	delay_n 39
	sbc #1
	Jne *-8
	plp
.elseif(n = 9471)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.repeat 4
	 nop
	.endrepeat
	brk 0
	sbc #1
	Jne *-9
	plp
.elseif(n = 9472)
	; 15 bytes
	php
	tya
	ldy #215
	delay_n 39
	dey
	Jne *-7
	tay
	plp
.endif
.else
; 9473 to 9492
.if(n = 9473)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.repeat 4
	 nop
	.endrepeat
	brk 0
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9474)
	; 15 bytes
	php
	tya
	ldy #249
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 9475)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #16
	jsr delay_a_25_clocks
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
	lda #65
	jsr delay_a_25_clocks
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
	lda #76
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
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
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9481 || n = 9485)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9482)
	; 16 bytes
	php
	sec
	lda #0
	nop
	delay_n 30
	sbc #1
	Jne *-9
	plp
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
.elseif(n = 9486)
	; 16 bytes
	php
	sec
	lda #206
	nop
	delay_n 39
	sbc #1
	Jne *-9
	plp
.elseif(n = 9487)
	; 16 bytes
	php
	tya
	pha
	ldy #35
	lda #231
	jsr delay_a_25_clocks
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
	ldy #243
	pha
	lda #0
	jsr delay_a_25_clocks
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
	lda #5
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9492)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 9493 to 9531
.if(n = 9493 || n = 9494 || n = 9496 || n = 9499 || n = 9501 || n = 9502 || n = 9506 || n = 9514)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9495 || n = 9500)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 9497)
	; 7 bytes
	php
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9498 || n = 9503)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9504)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9505 || n = 9510 || n = 9517 || n = 9518 || n = 9523)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9507 || n = 9512)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9508 || n = 9513)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9509)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9511)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9515)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 9516)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9519 || n = 9520 || n = 9522)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9521)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 9523 && n < 9528) || n = 9529 || n = 9530)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9528)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9531)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
; 9532 to 9687
.if(n = 9532 || n = 9534 || n = 9535 || n = 9537 || (n > 9538 && n < 9548) || (n > 9548 && n < 9552) || (n > 9552 && n < 9608) || (n > 9608 && n < 9613) || (n > 9613 && n < 9653) || (n > 9653 && n < 9688))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9533)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9536)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9538 || n = 9548 || n = 9552 || n = 9613 || n = 9653)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9608)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #37
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9698 || n = 9699)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9700 || n = 9713 || n = 9719 || n = 9721 || n = 9725)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 9700 && n < 9705))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9705)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-270)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9706)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-271)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9707)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-272)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9708)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9709)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9710)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9711)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9712)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9714)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9715)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9716)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 7
	sbc #1
	Jne *-8
	plp
.elseif(n = 9717)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9718)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 7
	dey
	Jne *-7
	tay
	plp
.elseif(n = 9720 || n = 9726)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 9722)
	; 14 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk 0
	adc #1
	Jne *-8
	plp
.elseif(n = 9723)
	; 16 bytes
	php
	tya
	ldy #249
	pha
	lda #0
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9724)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk 0
	dey
	Jne *-7
	tay
	plp
.endif
.else
.if((n >= 9727) && (n < 9746))
; 9727 to 9745
.if(n = 9727)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 9
	brk 0
	sbc #1
	Jne *-8
	plp
.elseif(n = 9728)
	; 16 bytes
	php
	tya
	pha
	ldy #133
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9729)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 9730 || n = 9732)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9731 || n = 9745)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9733)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9734)
	; 15 bytes
	php
	sec
	lda #221
	delay_n 39
	sbc #1
	Jne *-8
	plp
.elseif(n = 9735)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #79
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 9736)
	; 15 bytes
	php
	tya
	ldy #221
	delay_n 39
	dey
	Jne *-7
	tay
	plp
.elseif(n = 9737)
	; 16 bytes
	php
	tya
	pha
	ldy #85
	lda #81
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9738)
	; 15 bytes
	php
	sec
	lda #0
	delay_n 7
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 9739)
	; 16 bytes
	php
	tya
	pha
	ldy #162
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9740)
	; 15 bytes
	php
	tya
	ldy #0
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 9741)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #8
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9742)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #31
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
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
.endif
.else
; 9746 to 9765
.if(n = 9746)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9749)
	; 16 bytes
	php
	tya
	ldy #91
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9750 || n = 9752 || n = 9755 || n = 9757 || n = 9758 || n = 9762)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9751 || n = 9756)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 9753)
	; 7 bytes
	php
	lda #38
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9761)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9763)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9764)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9765)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.endif
.else
; 9766 to 9843
.if(n = 9766 || n = 9773 || n = 9774 || n = 9779)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9767)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9768)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9769)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9770)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9771)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 9772)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9775 || n = 9776 || n = 9778 || n = 9794 || n = 9804 || n = 9808)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9777)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 9779 && n < 9784) || n = 9785 || n = 9786 || n = 9788 || n = 9790 || n = 9791 || n = 9793 || (n > 9794 && n < 9804) || (n > 9804 && n < 9808) || (n > 9808 && n < 9844))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9784)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9787)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9789)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9792)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
; 9844 to 9921
.if((n > 9843 && n < 9864) || (n > 9864 && n < 9869) || (n > 9869 && n < 9909) || (n > 9909 && n < 9922))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9864)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #38
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9869 || n = 9909)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 9922) && (n < 9961))
; 9922 to 9960
.if((n > 9921 && n < 9954))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9954 || n = 9955)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9956)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 9956 && n < 9961))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 9961) && (n < 9981))
; 9961 to 9980
.if(n = 9961)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-271)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9962)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-272)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9963)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-273)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9964)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9965)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9966)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9967)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9968)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9969)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9970)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #75
	jsr delay_a_25_clocks
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
	lda #203
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9972)
	; 16 bytes
	php
	tya
	ldy #249
	pha
	lda #1
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9973)
	; 16 bytes
	php
	tya
	pha
	ldy #79
	lda #94
	jsr delay_a_25_clocks
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
	lda #23
	jsr delay_a_25_clocks
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
	sec
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
	lda #215
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9978)
	; 15 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-9
	plp
.elseif(n = 9979)
	; 16 bytes
	php
	tya
	pha
	ldy #166
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9980)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
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
	lda #60
	jsr delay_a_25_clocks
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
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-9
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
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9986)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #2
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9990)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #27
	jsr delay_a_25_clocks
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
	lda #245
	jsr delay_a_25_clocks
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
	lda #141
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	delay_n (n-2)
	nop
.elseif(n = 9998)
	; 15 bytes
	php
	sec
	lda #227
	delay_n 39
	sbc #1
	Jne *-8
	plp
.elseif(n = 9999)
	; 17 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	pha
	lda #2
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-10
	tay
	plp
.elseif(n = 10000)
	; 15 bytes
	php
	tya
	ldy #227
	delay_n 39
	dey
	Jne *-7
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
	sec
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
	lda #6
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10005)
	; 17 bytes
	php
	txa
	pha
	sec
	lda #26
	ldx #202 ;hides 'dex'
	Jmi *-1
	sbc #1
	Jne *-6
	pla
	tax
	plp
.elseif(n = 10006 || n = 10008 || n = 10011 || n = 10013 || n = 10014 || n = 10018 || n = 10026)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10007 || n = 10012)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10009)
	; 7 bytes
	php
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10010 || n = 10015)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10016)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10017 || n = 10022 || n = 10029 || n = 10030 || n = 10035)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10019 || n = 10024)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10020 || n = 10025)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10021)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10023)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10027)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 10028)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10031 || n = 10032 || n = 10034 || n = 10050 || n = 10060 || n = 10064 || n = 10125)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10033)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 10035 && n < 10040) || n = 10041 || n = 10042 || n = 10044 || n = 10046 || n = 10047 || n = 10049 || (n > 10050 && n < 10060) || (n > 10060 && n < 10064) || (n > 10064 && n < 10120) || (n > 10120 && n < 10125) || (n > 10125 && n < 10157))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10040)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10043)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10045)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10048)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10120)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #39
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 10157) && (n < 10235))
; 10157 to 10234
.if((n > 10156 && n < 10165) || (n > 10165 && n < 10210))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10165)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10210 || n = 10211)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10212 || n = 10225)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 10212 && n < 10217))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10217)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-272)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10218)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-273)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10219)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-274)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10220)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10221)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10222)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10223)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10224)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10226)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #27
	jsr delay_a_25_clocks
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
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 9
	sbc #1
	Jne *-9
	plp
.elseif(n = 10229)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10230)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 9
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10231)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	sec
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
	ldy #146
	pha
	lda #31
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10233)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10234)
	; 15 bytes
	php
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	brk 0
	adc #1
	Jne *-9
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
	lda #160
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10236)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	brk 0
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
	lda #99
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10238)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	brk 0
	sbc #1
	Jne *-9
	plp
.elseif(n = 10239)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 11
	brk 0
	sbc #1
	Jne *-9
	plp
.elseif(n = 10240)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10241)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 11
	brk 0
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
	lda #71
	jsr delay_a_25_clocks
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
	lda #16
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10245)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #74
	jsr delay_a_25_clocks
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
	lda #76
	jsr delay_a_25_clocks
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
	lda #21
	jsr delay_a_25_clocks
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
	lda #78
	jsr delay_a_25_clocks
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
	lda #215
	jsr delay_a_25_clocks
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
	lda #85
	jsr delay_a_25_clocks
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
	ldy #85
	lda #87
	jsr delay_a_25_clocks
	dey
	Jpl *-6
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
	lda #83
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10255 || n = 10262 || n = 10264 || n = 10267 || n = 10269 || n = 10270)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10256)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #13
	jsr delay_a_25_clocks
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
	lda #222
	jsr delay_a_25_clocks
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
	lda #8
	jsr delay_a_25_clocks
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
	lda #8
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10260)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10261)
	; 16 bytes
	php
	tya
	ldy #37
	pha
	lda #238
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10273)
	; 9 bytes
	delay_n (n-13)
	brk 0
.endif
.endif
.else
; 10274 to 10312
.if(n = 10274 || n = 10282)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275 || n = 10280)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10276 || n = 10281)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10277)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10278 || n = 10285 || n = 10286 || n = 10291)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10279)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10283)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 10284)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10287 || n = 10288 || n = 10290 || n = 10306)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10289)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 10291 && n < 10296) || n = 10297 || n = 10298 || n = 10300 || n = 10302 || n = 10303 || n = 10305 || (n > 10306 && n < 10313))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10296)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10299)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10301)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10304)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
; 10313 to 10468
.if((n > 10312 && n < 10316) || (n > 10316 && n < 10320) || (n > 10320 && n < 10376) || (n > 10376 && n < 10381) || (n > 10381 && n < 10421) || (n > 10421 && n < 10466))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10316 || n = 10320 || n = 10381 || n = 10421)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10376)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10466 || n = 10467)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
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
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10473)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-273)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10474)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-274)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10475)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-275)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10476)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10477)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10478)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10479)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10480)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10481)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 10482)
	; 15 bytes
	php
	sec
	lda #238
	delay_n 39
	sbc #1
	Jne *-8
	plp
.elseif(n = 10483)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10484)
	; 15 bytes
	php
	tya
	ldy #238
	delay_n 39
	dey
	Jne *-7
	tay
	plp
.elseif(n = 10485)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10486)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10487)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
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
	lda #69
	jsr delay_a_25_clocks
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
	lda #153
	jsr delay_a_25_clocks
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
	lda #251
	jsr delay_a_25_clocks
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
	lda #92
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10495)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #76
	jsr delay_a_25_clocks
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
	lda #57
	jsr delay_a_25_clocks
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
	lda #6
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10498)
	; 16 bytes
	php
	tya
	ldy #98
	pha
	lda #68
	jsr delay_a_25_clocks
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
	lda #99
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10502)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #5
	jsr delay_256a_11_clocks_
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
	lda #230
	jsr delay_a_25_clocks
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
	lda #75
	jsr delay_a_25_clocks
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
	sec
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
	sec
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
; 10508 to 10546
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
	lda #88
	jsr delay_a_25_clocks
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
	lda #237
	jsr delay_a_25_clocks
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
	ldy #85
	lda #90
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10512)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #3
	jsr delay_a_25_clocks
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
	ldy #98
	lda #74
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10517)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #26
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10518 || n = 10520 || n = 10523 || n = 10525 || n = 10526 || n = 10530 || n = 10538)
	; 8..11 bytes
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10522 || n = 10527)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10528)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10529 || n = 10534 || n = 10541 || n = 10542)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10531 || n = 10536)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10532 || n = 10537)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10533)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10535)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10539)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 10540)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10543 || n = 10544 || n = 10546)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10545)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 10547 to 10625
.if(n = 10547)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif((n > 10547 && n < 10552) || n = 10553 || n = 10554 || n = 10556 || n = 10558 || n = 10559 || n = 10561 || (n > 10562 && n < 10572) || (n > 10572 && n < 10576) || (n > 10576 && n < 10626))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10552)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10555)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10557)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10560)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10562 || n = 10572 || n = 10576)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
.if((n > 10625 && n < 10632) || (n > 10632 && n < 10637) || (n > 10637 && n < 10677) || (n > 10677 && n < 10704))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10632)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10637 || n = 10677)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 10704) && (n < 10743))
; 10704 to 10742
.if((n > 10703 && n < 10722))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10722 || n = 10723)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10724 || n = 10737)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 10724 && n < 10729))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10729)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-274)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10730)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-275)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10731)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-276)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10732)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10733)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10734)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10735)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10736)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10738)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #23
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10742)
	; 16 bytes
	php
	tya
	ldy #185
	pha
	lda #19
	jsr delay_a_25_clocks
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
	lda #10
	jsr delay_a_25_clocks
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
	lda #23
	jsr delay_a_25_clocks
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
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10746)
	; 15 bytes
	php
	sec
	lda #244
	delay_n 39
	sbc #1
	Jne *-8
	plp
.elseif(n = 10747)
	; 16 bytes
	php
	tya
	ldy #95
	pha
	lda #74
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10748)
	; 15 bytes
	php
	tya
	ldy #244
	delay_n 39
	dey
	Jne *-7
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
	lda #79
	jsr delay_a_25_clocks
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
	lda #164
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10752)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #21
	jsr delay_a_25_clocks
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
	lda #166
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10754)
	; 16 bytes
	php
	tya
	pha
	ldy #95
	lda #81
	jsr delay_a_25_clocks
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
	lda #97
	jsr delay_a_25_clocks
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
	lda #15
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10761)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/131
	lda #99
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #22
	jsr delay_a_25_clocks
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
	lda #244
	jsr delay_a_25_clocks
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
	lda #6
	jsr delay_a_25_clocks
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
	ldy #85
	lda #93
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10770)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #27
	jsr delay_a_25_clocks
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
	sec
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
	jsr delay_256a_11_clocks_
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
	jsr delay_256a_16_clocks
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
.if(n = 10782 || n = 10786 || n = 10794)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10783)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10784)
	; 9 bytes
	php
	pha
	lda #42
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10785 || n = 10790 || n = 10797 || n = 10798 || n = 10803)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10787 || n = 10792)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10788 || n = 10793)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10789)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10791)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10795)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 10796)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10799 || n = 10800 || n = 10802 || n = 10818 || n = 10828 || n = 10832 || n = 10893 || n = 10933)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10801)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 10803 && n < 10808) || n = 10809 || n = 10810 || n = 10812 || n = 10814 || n = 10815 || n = 10817 || (n > 10818 && n < 10828) || (n > 10828 && n < 10832) || (n > 10832 && n < 10888) || (n > 10888 && n < 10893) || (n > 10893 && n < 10933) || (n > 10933 && n < 10938))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10808)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10811)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10813)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10816)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10888)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #42
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 10977) && (n < 10996))
; 10977 to 10995
.if(n = 10977)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10978 || n = 10979)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10980 || n = 10993)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 10980 && n < 10985))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10985)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-275)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10986)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-276)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10987)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-277)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10988)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10989)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10990)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10991)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10992)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10994)
	; 16 bytes
	php
	tya
	ldy #37
	pha
	lda #250
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10995)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	jsr @nearby_rts
	sbc #1
	Jne *-9
	plp
.elseif(n = 10997)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #26
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10998)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	jsr @nearby_rts
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10999)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #$4A ;hides 'lsr a'
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
	lda #$4A ;hides 'lsr a'
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
	lda #72
	jsr delay_a_25_clocks
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
	lda #31
	jsr delay_a_25_clocks
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
	lda #160
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
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
	lda #162
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
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
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	brk 0
	sbc #1
	Jne *-9
	plp
.elseif(n = 11008)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-130)/111
	lda #79
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11009)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	brk 0
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11010)
	; 15 bytes
	php
	sec
	lda #250
	delay_n 39
	sbc #1
	Jne *-8
	plp
.elseif(n = 11011)
	; 16 bytes
	php
	tya
	pha
	ldy #229
	lda #16
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11012)
	; 15 bytes
	php
	tya
	ldy #250
	delay_n 39
	dey
	Jne *-7
	tay
	plp
.elseif(n = 11013)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	sec
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
	lda #171
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11015)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #13
	jsr delay_a_25_clocks
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
	lda #92
	jsr delay_a_25_clocks
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
	lda #85
	jsr delay_a_25_clocks
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
	lda #159
	jsr delay_a_25_clocks
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
.elseif(n = 11021 || n = 11029 || n = 11030 || n = 11032 || n = 11035 || n = 11037 || n = 11038 || n = 11042 || n = 11050)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11022 || n = 11031 || n = 11036)
	; 8..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 11023)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/131
	lda #99
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #94
	jsr delay_a_25_clocks
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
	lda #112
	jsr delay_a_25_clocks
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
	lda #11
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11028)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11033)
	; 7 bytes
	php
	lda #43
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11034 || n = 11039)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11040)
	; 9 bytes
	php
	pha
	lda #43
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11041 || n = 11046 || n = 11053 || n = 11054)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11043 || n = 11048)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11044 || n = 11049)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11045)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11047)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11051)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 11052)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 11055 to 11093
.if(n = 11055 || n = 11056 || n = 11058 || n = 11074 || n = 11084 || n = 11088)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11057)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11059)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif((n > 11059 && n < 11064) || n = 11065 || n = 11066 || n = 11068 || n = 11070 || n = 11071 || n = 11073 || (n > 11074 && n < 11084) || (n > 11084 && n < 11088) || (n > 11088 && n < 11094))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11064)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11067)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11069)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11072)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 11094 to 11250
.if((n > 11093 && n < 11144) || (n > 11144 && n < 11149) || (n > 11149 && n < 11189) || (n > 11189 && n < 11234))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11144)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #43
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11149 || n = 11189)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11234 || n = 11235)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11236 || n = 11249)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 11236 && n < 11241))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11241)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-276)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11242)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-277)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11243)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-278)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11244)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11245)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11246)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11247)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11248)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11250)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #240
	jsr delay_a_25_clocks
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
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11252)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	brk 0
	sbc #1
	Jne *-8
	plp
.elseif(n = 11253)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #50
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11254)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	tay
	plp
.elseif(n = 11255 || n = 11257 || n = 11266 || n = 11268)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11256 || n = 11267)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 11258)
	; 16 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #255
	jsr delay_a_25_clocks
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
	lda #249
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
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
	lda #6
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11263)
	; 14 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	delay_n 26
	sbc #1
	Jne *-8
	plp
.elseif(n = 11264)
	; 16 bytes
	php
	tya
	ldy #96
	pha
	lda #77
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11265)
	; 14 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	plp
.elseif(n = 11269)
	; 16 bytes
	php
	tya
	pha
	ldy #90
	lda #93
	jsr delay_a_25_clocks
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
	lda #59
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 11274)
	; 15 bytes
	php
	sec
	lda #0
	delay_n 39
	sbc #1
	Jne *-8
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
	; 15 bytes
	php
	tya
	ldy #0
	delay_n 39
	dey
	Jne *-7
	tay
	plp
.elseif(n = 11277 || n = 11287)
	; 8..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 11278)
	; 16 bytes
	php
	tya
	ldy #(n-143)/131
	pha
	lda #92
	jsr delay_a_25_clocks
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
	lda #150
	jsr delay_a_25_clocks
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
	lda #8
	jsr delay_a_25_clocks
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
	lda #(n-561)/536
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11285 || n = 11286 || n = 11288)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 11289)
	; 7 bytes
	php
	lda #44
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 11290 to 11328
.if(n = 11290 || n = 11295)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11291 || n = 11293 || n = 11294 || n = 11298 || n = 11306)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11292)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11296)
	; 9 bytes
	php
	pha
	lda #44
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11297 || n = 11302 || n = 11309 || n = 11310 || n = 11315)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11299 || n = 11304)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11300 || n = 11305)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11301)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11303)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11307)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 11308)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11311 || n = 11312 || n = 11314)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11313)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 11315 && n < 11320) || n = 11321 || n = 11322 || n = 11324 || n = 11326 || n = 11327)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11320)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11323)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11325)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11328)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 11329 to 11406
.if(n = 11329 || (n > 11330 && n < 11340) || (n > 11340 && n < 11344) || (n > 11344 && n < 11400) || (n > 11400 && n < 11405) || n = 11406)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11330 || n = 11340 || n = 11344 || n = 11405)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11400)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #44
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
; 11407 to 11484
.if((n > 11406 && n < 11445) || (n > 11445 && n < 11485))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11445)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 11485) && (n < 11524))
.if((n >= 11485) && (n < 11504))
; 11485 to 11503
.if((n > 11484 && n < 11490))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11490 || n = 11491)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11492)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 11492 && n < 11497))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11497)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-277)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11498)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-278)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11499)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-279)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11500)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11501)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11502)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11503)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 11504 to 11523
.if(n = 11504)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11505)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11506)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11507)
	; 16 bytes
	php
	tya
	ldy #(n-12)/121
	pha
	lda #82
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11508)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-9
	plp
.elseif(n = 11509)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11510)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11511)
	; 16 bytes
	php
	tya
	pha
	ldy #221
	lda #20
	jsr delay_a_25_clocks
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
	sec
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
	lda #160
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11516)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11519)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	delay_n 27
	sbc #1
	Jne *-9
	plp
.elseif(n = 11520)
	; 16 bytes
	php
	tya
	ldy #84
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11521)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 27
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11522)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11523)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
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
	lda #27
	jsr delay_a_25_clocks
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
	lda #240
	jsr delay_a_25_clocks
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
	lda #18
	jsr delay_a_25_clocks
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
	lda #8
	jsr delay_a_25_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
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
	lda #119
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
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
	lda #25
	jsr delay_a_25_clocks
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
	lda #15
	jsr delay_a_25_clocks
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
	lda #30
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #126
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	jsr delay_256a_11_clocks_
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
.if(n = 11543 || n = 11548)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11544 || n = 11547 || n = 11549 || n = 11550 || n = 11554 || n = 11562)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11545)
	; 7 bytes
	php
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11546 || n = 11551)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11552)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11553 || n = 11558)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11555 || n = 11560)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11556 || n = 11561)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11557)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11559)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
; 11563 to 11718
.if(n = 11563)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 11564)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11565 || n = 11566 || n = 11571)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11567 || n = 11568 || n = 11570 || n = 11586 || n = 11596 || n = 11600 || n = 11661 || n = 11701)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11569)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 11571 && n < 11576) || n = 11577 || n = 11578 || n = 11580 || n = 11582 || n = 11583 || n = 11585 || (n > 11586 && n < 11596) || (n > 11596 && n < 11600) || (n > 11600 && n < 11656) || (n > 11656 && n < 11661) || (n > 11661 && n < 11701) || (n > 11701 && n < 11719))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11576)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11579)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11581)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11584)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11656)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #45
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 11719) && (n < 11797))
.if((n >= 11719) && (n < 11758))
; 11719 to 11757
.if((n > 11718 && n < 11746))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11746 || n = 11747)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11748)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 11748 && n < 11753))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11753)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-278)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11754)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-279)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11755)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-280)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11756)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11757)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 11758) && (n < 11777))
; 11758 to 11776
.if(n = 11758)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11759)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11760)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11761)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11762)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #8
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11764)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	nop
	brk 0
	sbc #1
	Jne *-9
	plp
.elseif(n = 11765)
	; 16 bytes
	php
	tya
	ldy #161
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11766)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	nop
	brk 0
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11767)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
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
	lda #93
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
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
	lda #20
	jsr delay_a_25_clocks
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
	lda #162
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11774)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 26
	sbc #1
	Jne *-9
	plp
.elseif(n = 11775)
	; 15 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	delay_n 26
	sbc #1
	Jne *-9
	plp
.elseif(n = 11776)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 26
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 11777 to 11796
.if(n = 11777)
	; 15 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	delay_n 26
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11778)
	; 16 bytes
	php
	tya
	ldy #222
	pha
	sec
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
	lda #$4A ;hides 'lsr a'
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
	lda #170
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11782)
	; 16 bytes
	php
	tya
	ldy #214
	pha
	lda #16
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #21
	jsr delay_a_25_clocks
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
	lda #255
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #23
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #22
	jsr delay_a_25_clocks
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
	lda #85
	jsr delay_a_25_clocks
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
	lda #98
	jsr delay_a_25_clocks
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
	sec
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
	jsr delay_256a_11_clocks_
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
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11798 || n = 11800 || n = 11803 || n = 11805 || n = 11806 || n = 11810 || n = 11818)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11799 || n = 11804)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11801)
	; 7 bytes
	php
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11802 || n = 11807)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11808)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11809 || n = 11814 || n = 11821 || n = 11822 || n = 11827)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11811 || n = 11816)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11812 || n = 11817)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11813)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11815)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11819)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 11820)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11823 || n = 11824 || n = 11826 || n = 11842 || n = 11852 || n = 11856)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11825)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 11827 && n < 11832) || n = 11833 || n = 11834 || n = 11836 || n = 11838 || n = 11839 || n = 11841 || (n > 11842 && n < 11852) || (n > 11852 && n < 11856) || (n > 11856 && n < 11876))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11832)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11835)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11837)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11840)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
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
.if((n > 11875 && n < 11912) || (n > 11912 && n < 11917) || (n > 11917 && n < 11954))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11912)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11917)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 11954) && (n < 11993))
; 11954 to 11992
.if((n > 11953 && n < 11957) || (n > 11957 && n < 11993))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11957)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 11993) && (n < 12012))
; 11993 to 12011
.if((n > 11992 && n < 12002))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12002 || n = 12003)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12004)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 12004 && n < 12009))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12009)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-279)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12010)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-280)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12011)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-281)/255
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 12012 to 12031
.if(n = 12012)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12013)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12014)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12015)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12016)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12017)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 12018)
	; 16 bytes
	php
	tya
	ldy #86
	pha
	lda #99
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12019)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12020)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #37
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12022)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12023)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #53
	jsr delay_a_25_clocks
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
	lda #93
	jsr delay_a_25_clocks
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
	lda #26
	jsr delay_a_25_clocks
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
	lda #159
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12027)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #96
	jsr delay_a_25_clocks
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
	lda #161
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12029)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #22
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12031)
	; 16 bytes
	php
	tya
	pha
	ldy #231
	lda #20
	jsr delay_a_25_clocks
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
.if((n >= 12032) && (n < 12051))
; 12032 to 12050
.if(n = 12032)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #101
	jsr delay_a_25_clocks
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
	lda #166
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12034)
	; 16 bytes
	php
	tya
	pha
	ldy #135
	lda #57
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12036)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
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
	lda #26
	jsr delay_a_25_clocks
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
	lda #69
	jsr delay_a_25_clocks
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
	lda #18
	jsr delay_a_25_clocks
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
	lda #85
	jsr delay_a_25_clocks
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
	lda #133
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12042 || n = 12049 || n = 12050)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12043)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	sec
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
	lda #25
	jsr delay_a_25_clocks
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
	lda #92
	jsr delay_a_25_clocks
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
.endif
.else
; 12051 to 12070
.if(n = 12051)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #15
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12052)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12053)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #68
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12054 || n = 12056 || n = 12059 || n = 12061 || n = 12062 || n = 12066)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12055 || n = 12060)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12057)
	; 7 bytes
	php
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12058 || n = 12063)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12064)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12065 || n = 12070)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12067)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12068)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12069)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 12071 to 12109
.if(n = 12071)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12072)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12073)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12074)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12075)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 12076)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12077 || n = 12078 || n = 12083)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12079 || n = 12080 || n = 12082 || n = 12098 || n = 12108)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12081)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 12083 && n < 12088) || n = 12089 || n = 12090 || n = 12092 || n = 12094 || n = 12095 || n = 12097 || (n > 12098 && n < 12108) || n = 12109)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12088)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12091)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12093)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12096)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 12110 to 12187
.if(n = 12110 || n = 12111 || (n > 12112 && n < 12168) || (n > 12168 && n < 12173) || (n > 12173 && n < 12188))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12112 || n = 12173)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12168)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #47
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
; 12188 to 12265
.if((n > 12187 && n < 12213) || (n > 12213 && n < 12258))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12213)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12258 || n = 12259)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12260)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 12260 && n < 12265))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12265)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-280)/255
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 12266) && (n < 12305))
.if((n >= 12266) && (n < 12285))
; 12266 to 12284
.if(n = 12266)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-281)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12267)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-282)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12268)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12269)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12270)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12271)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12272)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12273)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 12274)
	; 16 bytes
	php
	tya
	pha
	ldy #95
	lda #97
	jsr delay_a_25_clocks
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
	lda #193
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12276)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12277)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #22
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12279)
	; 16 bytes
	php
	tya
	ldy #141
	pha
	lda #48
	jsr delay_a_25_clocks
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
	lda #29
	jsr delay_a_25_clocks
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
	lda #185
	jsr delay_a_25_clocks
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
	lda #77
	jsr delay_a_25_clocks
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
	lda #23
	jsr delay_a_25_clocks
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
	lda #162
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12286)
	; 16 bytes
	php
	tya
	pha
	ldy #141
	lda #55
	jsr delay_a_25_clocks
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
	lda #164
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12288)
	; 16 bytes
	php
	tya
	ldy #98
	pha
	lda #85
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12291)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #20
	jsr delay_a_25_clocks
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
	lda #97
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12294)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #99
	jsr delay_a_25_clocks
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
	lda #92
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	lda #37
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
; 12305 to 12343
.if(n = 12305 || n = 12310 || n = 12312 || n = 12315 || n = 12317 || n = 12318 || n = 12322 || n = 12330)
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
	lda #16
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12308)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12309 || n = 12311 || n = 12316)
	; 8..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 12313)
	; 7 bytes
	php
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12314 || n = 12319)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12320)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12321 || n = 12326 || n = 12333 || n = 12334 || n = 12339)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12323 || n = 12328)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12324 || n = 12329)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12325)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12327)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12331)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 12332)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12335 || n = 12336 || n = 12338)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12337)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 12339 && n < 12344))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 12344 to 12500
.if(n = 12344)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12345 || n = 12346 || n = 12348 || n = 12350 || n = 12351 || n = 12353 || (n > 12354 && n < 12364) || (n > 12364 && n < 12368) || (n > 12368 && n < 12424) || (n > 12424 && n < 12429) || (n > 12429 && n < 12469) || (n > 12469 && n < 12501))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12347)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12349)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12352)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12354 || n = 12364 || n = 12368 || n = 12429 || n = 12469)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12424)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #48
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12514 || n = 12515)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12516 || n = 12529)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 12516 && n < 12521))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12521)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-281)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12522)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-282)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12523)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-283)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12524)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12525)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12526)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12527)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12528)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12530)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12531)
	; 16 bytes
	php
	tya
	pha
	ldy #184
	lda #36
	jsr delay_a_25_clocks
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
	lda #97
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12534)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12535)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #52
	jsr delay_a_25_clocks
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
	lda #23
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #75
	jsr delay_a_25_clocks
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
	lda #160
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
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
	lda #15
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #31
	jsr delay_a_25_clocks
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
	lda #30
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
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
	lda #20
	jsr delay_a_25_clocks
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
	lda #93
	jsr delay_a_25_clocks
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
	lda #77
	jsr delay_a_25_clocks
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
	lda #72
	jsr delay_a_25_clocks
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
	lda #26
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12560 || n = 12566 || n = 12568 || n = 12571 || n = 12573 || n = 12574 || n = 12578)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 12561)
	; 16 bytes
	php
	tya
	ldy #141
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12562)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-130)/111
	lda #79
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12565)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #19
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12577)
	; 9 bytes
	delay_n (n-13)
	brk 0
.endif
.endif
.endif
.else
; 12579 to 12656
.if(n = 12579 || n = 12584)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12580 || n = 12585)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12581)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12582 || n = 12589 || n = 12590 || n = 12595)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12583)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12586)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12587)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 12588)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12591 || n = 12592 || n = 12594 || n = 12610 || n = 12620 || n = 12624)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12593)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 12595 && n < 12600) || n = 12601 || n = 12602 || n = 12604 || n = 12606 || n = 12607 || n = 12609 || (n > 12610 && n < 12620) || (n > 12620 && n < 12624) || (n > 12624 && n < 12657))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12600)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12603)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12605)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12608)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
; 12657 to 12734
.if((n > 12656 && n < 12680) || (n > 12680 && n < 12685) || (n > 12685 && n < 12725) || (n > 12725 && n < 12735))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12680)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12685 || n = 12725)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 12735) && (n < 12774))
; 12735 to 12773
.if((n > 12734 && n < 12770))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12770 || n = 12771)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12772)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 12773)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12777)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-282)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12778)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-283)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12779)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-284)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12780)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12781)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12782)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12783)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12784)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12785)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 12786)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12787)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12788)
	; 16 bytes
	php
	tya
	pha
	ldy #112
	lda #81
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12790)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #97
	jsr delay_a_25_clocks
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
	lda #30
	jsr delay_a_25_clocks
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
	lda #21
	jsr delay_a_25_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12794)
	; 16 bytes
	php
	tya
	pha
	ldy #175
	lda #41
	jsr delay_a_25_clocks
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
	lda #160
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12796)
	; 16 bytes
	php
	tya
	ldy #93
	pha
	lda #97
	jsr delay_a_25_clocks
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
	lda #162
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12798)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12799)
	; 16 bytes
	php
	tya
	pha
	ldy #213
	lda #$4A ;hides 'lsr a'
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
	ldy #139
	pha
	lda #53
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
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
	lda #82
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12806)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
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
	lda #31
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #15
	jsr delay_a_25_clocks
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
	lda #20
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12815 || n = 12818 || n = 12822 || n = 12824 || n = 12827 || n = 12829 || n = 12830 || n = 12834 || n = 12842)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12816)
	; 16 bytes
	php
	tya
	ldy #96
	pha
	lda #93
	jsr delay_a_25_clocks
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
	lda #22
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12821)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12823 || n = 12828)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12825)
	; 7 bytes
	php
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12826 || n = 12831)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12832)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12833 || n = 12838 || n = 12845 || n = 12846 || n = 12851)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12835 || n = 12840)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12836 || n = 12841)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12837)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12839)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12843)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 12844)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12847 || n = 12848 || n = 12850)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12849)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 12852 to 12890
.if((n > 12851 && n < 12856) || n = 12857 || n = 12858 || n = 12860 || n = 12862 || n = 12863 || n = 12865 || (n > 12866 && n < 12876) || (n > 12876 && n < 12880) || (n > 12880 && n < 12891))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12856)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12859)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12861)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12864)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12866 || n = 12876 || n = 12880)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 12891 to 12968
.if((n > 12890 && n < 12936) || (n > 12936 && n < 12941) || (n > 12941 && n < 12969))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12936)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12941)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
.if((n >= 12969) && (n < 13047))
; 12969 to 13046
.if((n > 12968 && n < 12981) || (n > 12981 && n < 13026))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12981)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13026 || n = 13027)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13028 || n = 13041)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 13028 && n < 13033))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13033)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-283)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13034)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-284)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13035)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-285)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13036)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13037)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13038)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13039)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13040)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13042)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #140
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 13043)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13044)
	; 16 bytes
	php
	tya
	ldy #181
	pha
	lda #33
	jsr delay_a_25_clocks
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
	lda #46
	jsr delay_a_25_clocks
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
	lda #94
	jsr delay_a_25_clocks
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
	lda #16
	jsr delay_a_25_clocks
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
	lda #69
	jsr delay_a_25_clocks
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
	sec
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
	lda #(n-11)/326
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
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
	lda #23
	jsr delay_a_25_clocks
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
	lda #165
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13057)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #21
	jsr delay_a_25_clocks
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
	lda #27
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13062)
	; 16 bytes
	php
	tya
	ldy #225
	pha
	lda #19
	jsr delay_a_25_clocks
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
	lda #188
	jsr delay_a_25_clocks
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
	lda #13
	jsr delay_a_25_clocks
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
	lda #18
	jsr delay_a_25_clocks
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
	lda #22
	jsr delay_a_25_clocks
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
	lda #26
	jsr delay_a_25_clocks
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
	lda #20
	jsr delay_a_25_clocks
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
	lda #25
	jsr delay_a_25_clocks
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
	lda #29
	jsr delay_a_25_clocks
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
	lda #30
	jsr delay_a_25_clocks
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
	lda #19
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13076)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13077)
	; 16 bytes
	php
	tya
	ldy #195
	pha
	lda #$4A ;hides 'lsr a'
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13082)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086 || n = 13090 || n = 13098)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13087)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13088)
	; 9 bytes
	php
	pha
	lda #51
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13089 || n = 13094 || n = 13101 || n = 13102 || n = 13107)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13091 || n = 13096)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13092 || n = 13097)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13093)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13095)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13099)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 13100)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13103 || n = 13104 || n = 13106 || n = 13122)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13105)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 13107 && n < 13112) || n = 13113 || n = 13114 || n = 13116 || n = 13118 || n = 13119 || n = 13121 || (n > 13122 && n < 13126))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13112)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13115)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13117)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13120)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
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
.if((n > 13125 && n < 13132) || (n > 13132 && n < 13136) || (n > 13136 && n < 13192) || (n > 13192 && n < 13197) || (n > 13197 && n < 13237) || (n > 13237 && n < 13282))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13132 || n = 13136 || n = 13197 || n = 13237)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13192)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #51
	jsr delay_256a_16_clocks
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
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13284 || n = 13297)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 13284 && n < 13289))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13289)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-284)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13290)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-285)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13291)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-286)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13292)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13293)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13294)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13295)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13296)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13298)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/271
	lda #239
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #25
	jsr delay_a_25_clocks
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
	lda #98
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13303)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #22
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
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
	lda #71
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #65
	jsr delay_a_25_clocks
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
	lda #162
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13310)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #22
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13312)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13313)
	; 16 bytes
	php
	tya
	ldy #46
	pha
	lda #244
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13314)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13315)
	; 16 bytes
	php
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
	plp
.elseif(n = 13316)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13317)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
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
	lda #61
	jsr delay_a_25_clocks
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
	lda #38
	jsr delay_a_25_clocks
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
	lda #251
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.endif
.endif
.else
; 13321 to 13359
.if(n = 13321 || n = 13326 || n = 13334 || n = 13336 || n = 13339 || n = 13341 || n = 13342 || n = 13346 || n = 13354)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13322)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #21
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	nop
	pha
	tya
	ldy #112
	dey
	Jne *-1
	tay
	pla
	Jcs *-12
	plp
.elseif(n = 13327)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #76
	jsr delay_a_25_clocks
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
	lda #51
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13332)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13333)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #38
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13335 || n = 13340)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13337)
	; 7 bytes
	php
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13338 || n = 13343)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13344)
	; 9 bytes
	php
	pha
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13345 || n = 13350 || n = 13357 || n = 13358)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13347 || n = 13352)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13348 || n = 13353)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13349)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13351)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13355)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 13356)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13359)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 13360 to 13437
.if(n = 13360 || n = 13362 || n = 13378 || n = 13388 || n = 13392)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13361)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13363)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif((n > 13363 && n < 13368) || n = 13369 || n = 13370 || n = 13372 || n = 13374 || n = 13375 || n = 13377 || (n > 13378 && n < 13388) || (n > 13388 && n < 13392) || (n > 13392 && n < 13438))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13368)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13371)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13373)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13376)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
; 13438 to 13515
.if((n > 13437 && n < 13448) || (n > 13448 && n < 13453) || (n > 13453 && n < 13493) || (n > 13493 && n < 13516))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13448)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13453 || n = 13493)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 13516) && (n < 13555))
; 13516 to 13554
.if((n > 13515 && n < 13538))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13538 || n = 13539)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13540 || n = 13553)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 13540 && n < 13545))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13545)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-285)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13546)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-286)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13547)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-287)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13548)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13549)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13550)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13551)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13552)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13554)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #22
	jsr delay_a_25_clocks
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
	lda #40
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13557)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13558)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13559)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #149
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13561)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
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
	lda #232
	jsr delay_a_25_clocks
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
	lda #160
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13564)
	; 16 bytes
	php
	tya
	pha
	ldy #215
	lda #31
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
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
	lda #15
	jsr delay_a_25_clocks
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
	lda #164
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13568)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13569)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #239
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
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
	lda #21
	jsr delay_a_25_clocks
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
	lda #22
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13575)
	; 16 bytes
	php
	tya
	ldy #98
	pha
	lda #98
	jsr delay_a_25_clocks
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
	lda #197
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-9
	tay
	plp
.elseif(n = 13577)
	; 17 bytes
	php
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
	plp
.elseif(n = 13578)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #59
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13581 || n = 13586 || n = 13590 || n = 13592)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13582)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #67
	jsr delay_a_25_clocks
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
	lda #204
	jsr delay_a_25_clocks
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
	lda #19
	jsr delay_a_25_clocks
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
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13587)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #21
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13588)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
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
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
; 13594 to 13750
.if(n = 13594 || n = 13599)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13595 || n = 13597 || n = 13598 || n = 13602 || n = 13610)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13596)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13600)
	; 9 bytes
	php
	pha
	lda #53
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13601 || n = 13606 || n = 13613 || n = 13614 || n = 13619)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13603 || n = 13608)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13604 || n = 13609)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13605)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13607)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13611)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 13612)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13615 || n = 13616 || n = 13618 || n = 13634 || n = 13644 || n = 13648 || n = 13709 || n = 13749)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13617)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 13619 && n < 13624) || n = 13625 || n = 13626 || n = 13628 || n = 13630 || n = 13631 || n = 13633 || (n > 13634 && n < 13644) || (n > 13644 && n < 13648) || (n > 13648 && n < 13704) || (n > 13704 && n < 13709) || (n > 13709 && n < 13749) || n = 13750)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13624)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13627)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13629)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13632)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13704)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #53
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 13790) && (n < 13809))
; 13790 to 13808
.if((n > 13789 && n < 13794))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13794 || n = 13795)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13796)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 13796 && n < 13801))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13801)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-286)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13802)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-287)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13803)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-288)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13804)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13805)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13806)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13807)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13808)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 13810)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13811)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #208
	jsr delay_a_25_clocks
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
	lda #21
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13813)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13814)
	; 16 bytes
	php
	tya
	ldy #206
	pha
	lda #$4A ;hides 'lsr a'
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
	lda #68
	jsr delay_a_25_clocks
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
	lda #31
	jsr delay_a_25_clocks
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
	lda #16
	jsr delay_a_25_clocks
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
	lda #20
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	lda #75
	jsr delay_a_25_clocks
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
	lda #23
	jsr delay_a_25_clocks
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
	lda #27
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13827)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13828)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #49
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13830)
	; 16 bytes
	php
	tya
	ldy #147
	pha
	lda #55
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13832)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13833)
	; 16 bytes
	php
	tya
	ldy #51
	pha
	lda #232
	jsr delay_a_25_clocks
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
	lda #151
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #62
	jsr delay_a_25_clocks
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
	lda #23
	jsr delay_a_25_clocks
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
	lda #239
	jsr delay_a_25_clocks
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
	lda #158
	jsr delay_a_25_clocks
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
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13844)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13845)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #30
	jsr delay_a_25_clocks
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
.if(n = 13848 || n = 13851 || n = 13853 || n = 13854 || n = 13858 || n = 13866)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13849)
	; 7 bytes
	php
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13850 || n = 13855)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13852)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13856)
	; 9 bytes
	php
	pha
	lda #54
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13857 || n = 13862)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13859 || n = 13864)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13860 || n = 13865)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13861)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13863)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13867)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13869 || n = 13870 || n = 13875)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13871 || n = 13872 || n = 13874 || n = 13890 || n = 13900 || n = 13904)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13873)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 13875 && n < 13880) || n = 13881 || n = 13882 || n = 13884 || n = 13886 || n = 13887 || n = 13889 || (n > 13890 && n < 13900) || (n > 13900 && n < 13904) || n = 13905 || n = 13906)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13880)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13883)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13885)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13888)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 13907 to 14062
.if((n > 13906 && n < 13960) || (n > 13960 && n < 13965) || (n > 13965 && n < 14005) || (n > 14005 && n < 14050))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13960)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13965 || n = 14005)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14050 || n = 14051)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14052)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 14052 && n < 14057))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14057)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-287)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14058)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-288)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14059)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-289)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14060)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14061)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14062)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
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
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14064)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14065)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14066)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #210
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14068)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #31
	jsr delay_a_25_clocks
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
	lda #249
	jsr delay_a_25_clocks
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
	lda #27
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
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
	lda #70
	jsr delay_a_25_clocks
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
	lda #40
	jsr delay_a_25_clocks
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
	lda #254
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14079)
	; 16 bytes
	php
	tya
	pha
	ldy #190
	lda #42
	jsr delay_a_25_clocks
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
	lda #77
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #168
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14086)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	lda #23
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14088)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #29
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14090)
	; 17 bytes
	php
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-10
	tay
	plp
.elseif(n = 14091)
	; 16 bytes
	php
	tya
	ldy #247
	pha
	lda #18
	jsr delay_a_25_clocks
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
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14094 || n = 14097)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14095)
	; 16 bytes
	php
	tya
	pha
	ldy #207
	lda #36
	jsr delay_a_25_clocks
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
	lda #213
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 14098)
	; 16 bytes
	php
	tya
	pha
	ldy #247
	lda #25
	jsr delay_a_25_clocks
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
	lda #23
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14100)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14101)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.else
; 14102 to 14140
.if(n = 14102 || n = 14104 || n = 14107 || n = 14109 || n = 14110 || n = 14114 || n = 14122)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14103 || n = 14108)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14105)
	; 7 bytes
	php
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14106 || n = 14111)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14112)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14113 || n = 14118 || n = 14125 || n = 14126 || n = 14131)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14115 || n = 14120)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14116 || n = 14121)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14117)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14119)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14123)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 14124)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14127 || n = 14128 || n = 14130)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14129)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 14131 && n < 14136) || n = 14137 || n = 14138 || n = 14140)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14136)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14139)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 14141 to 14218
.if(n = 14141)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14142 || n = 14143 || n = 14145 || (n > 14146 && n < 14156) || (n > 14156 && n < 14160) || (n > 14160 && n < 14216) || n = 14217 || n = 14218)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14144)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14146 || n = 14156 || n = 14160)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14216)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #55
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
; 14219 to 14296
.if(n = 14219 || n = 14220 || (n > 14221 && n < 14261) || (n > 14261 && n < 14297))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14221 || n = 14261)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 14297) && (n < 14336))
.if((n >= 14297) && (n < 14316))
; 14297 to 14315
.if((n > 14296 && n < 14306))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14306 || n = 14307)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14308)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 14308 && n < 14313))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14313)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-288)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14314)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-289)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14315)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-290)/255
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 14316 to 14335
.if(n = 14316)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14317)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14318)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14319)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14320)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14321)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14322)
	; 16 bytes
	php
	tya
	ldy #159
	pha
	lda #51
	jsr delay_a_25_clocks
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
	lda #64
	jsr delay_a_25_clocks
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
	lda #217
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14326)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #25
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14329)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #58
	jsr delay_a_25_clocks
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
	lda #159
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14331)
	; 16 bytes
	php
	tya
	ldy #(n-123)/111
	pha
	lda #72
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
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
	lda #162
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14334)
	; 16 bytes
	php
	tya
	ldy #231
	pha
	lda #23
	jsr delay_a_25_clocks
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
	lda #164
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
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
	lda #71
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #79
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #33
	jsr delay_a_25_clocks
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
	lda #30
	jsr delay_a_25_clocks
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
	lda #157
	jsr delay_a_25_clocks
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
	lda #242
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
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
	lda #26
	jsr delay_a_25_clocks
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
	lda #215
	jsr delay_a_25_clocks
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
	lda #40
	jsr delay_a_25_clocks
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
	lda #20
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #21
	jsr delay_a_25_clocks
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
	lda #222
	jsr delay_a_25_clocks
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
	lda #29
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14357)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14358 || n = 14360 || n = 14363 || n = 14365 || n = 14366 || n = 14370)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14359 || n = 14364)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14361)
	; 7 bytes
	php
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14362 || n = 14367)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14368)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14369 || n = 14374)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14371)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14372)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
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
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14377)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14378)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14379)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 14380)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14381 || n = 14382 || n = 14387)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14383 || n = 14384 || n = 14386 || n = 14402 || n = 14412 || n = 14416 || n = 14477 || n = 14517)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14385)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 14387 && n < 14392) || n = 14393 || n = 14394 || n = 14396 || n = 14398 || n = 14399 || n = 14401 || (n > 14402 && n < 14412) || (n > 14412 && n < 14416) || (n > 14416 && n < 14472) || (n > 14472 && n < 14477) || (n > 14477 && n < 14517) || (n > 14517 && n < 14532))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14392)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14395)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14397)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14400)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14472)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #56
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
; 14532 to 14570
.if((n > 14531 && n < 14562))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14562 || n = 14563)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14564)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 14564 && n < 14569))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14569)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-289)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14570)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-290)/255
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 14571) && (n < 14590))
; 14571 to 14589
.if(n = 14571)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-291)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14572)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14573)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14574)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14575)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14576)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14577)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14578)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #37
	jsr delay_a_25_clocks
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
	lda #33
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14581)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14582)
	; 16 bytes
	php
	tya
	ldy #235
	pha
	lda #23
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14584)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14585)
	; 16 bytes
	php
	tya
	ldy #247
	pha
	lda #20
	jsr delay_a_25_clocks
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
	lda #159
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14587)
	; 16 bytes
	php
	tya
	ldy #55
	pha
	lda #226
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
	lda #30
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	lda #22
	jsr delay_a_25_clocks
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
	lda #27
	jsr delay_a_25_clocks
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
	lda #166
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14594)
	; 16 bytes
	php
	tya
	pha
	ldy #55
	lda #233
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
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
	lda #81
	jsr delay_a_25_clocks
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
	lda #42
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
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
	lda #$4A ;hides 'lsr a'
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
	sec
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_
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
	lda #215
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
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
	lda #123
	jsr delay_a_25_clocks
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
	lda #66
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
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
	lda #62
	jsr delay_a_25_clocks
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
	lda #32
	jsr delay_a_25_clocks
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
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14612)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14613)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #54
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14614 || n = 14616 || n = 14619 || n = 14621 || n = 14622 || n = 14626 || n = 14634)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14615 || n = 14620)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14617)
	; 7 bytes
	php
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14618 || n = 14623)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14624)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14625 || n = 14630 || n = 14637 || n = 14638 || n = 14643)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14627 || n = 14632)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14628 || n = 14633)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14629)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14631)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14635)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 14636)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14639 || n = 14640 || n = 14642 || n = 14658 || n = 14668 || n = 14672)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14641)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 14643 && n < 14648) || n = 14649 || n = 14650 || n = 14652 || n = 14654 || n = 14655 || n = 14657 || (n > 14658 && n < 14668) || (n > 14668 && n < 14672) || (n > 14672 && n < 14688))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14648)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14651)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14653)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14656)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
; 14688 to 14843
.if((n > 14687 && n < 14728) || (n > 14728 && n < 14733) || (n > 14733 && n < 14773) || (n > 14773 && n < 14818))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14728)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14733 || n = 14773)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14818 || n = 14819)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14820 || n = 14833)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 14820 && n < 14825))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14825)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-290)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14826)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-291)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14827)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-292)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14828)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14829)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14830)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14831)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14832)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14834)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14835)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #22
	jsr delay_a_25_clocks
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
	lda #29
	jsr delay_a_25_clocks
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
	ldy #239
	lda #30
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #172
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14841)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14842)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #33
	jsr delay_a_25_clocks
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
	lda #52
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14847)
	; 16 bytes
	php
	tya
	ldy #215
	pha
	lda #30
	jsr delay_a_25_clocks
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
	lda #165
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14849)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #172
	jsr delay_a_25_clocks
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
	lda #224
	jsr delay_a_25_clocks
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
	lda #40
	jsr delay_a_25_clocks
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
	lda #59
	jsr delay_a_25_clocks
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
	lda #(n-5)/256
	jsr delay_a_25_clocks
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
	lda #37
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14856)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14857)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #231
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #38
	jsr delay_a_25_clocks
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
	lda #50
	jsr delay_a_25_clocks
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
	lda #27
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 14863 to 14882
.if(n = 14863 || n = 14870 || n = 14872 || n = 14875 || n = 14877 || n = 14878 || n = 14882)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14864)
	; 16 bytes
	php
	tya
	ldy #188
	pha
	lda #40
	jsr delay_a_25_clocks
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
	lda #30
	jsr delay_a_25_clocks
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
	lda #69
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14869)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #34
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14881)
	; 9 bytes
	delay_n (n-13)
	brk 0
.endif
.endif
.else
; 14883 to 14921
.if(n = 14883 || n = 14888)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14884 || n = 14889)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14885)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14886 || n = 14893 || n = 14894 || n = 14899)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14887)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14890)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14891)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 14892)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14895 || n = 14896 || n = 14898 || n = 14914)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14897)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 14899 && n < 14904) || n = 14905 || n = 14906 || n = 14908 || n = 14910 || n = 14911 || n = 14913 || (n > 14914 && n < 14922))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14904)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14907)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14909)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14912)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 14922 to 15000
.if(n = 14922 || n = 14923 || (n > 14924 && n < 14928) || (n > 14928 && n < 14984) || (n > 14984 && n < 14989) || (n > 14989 && n < 15001))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14924 || n = 14928 || n = 14989)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14984)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #58
	jsr delay_256a_16_clocks
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
.if((n > 15000 && n < 15029) || (n > 15029 && n < 15074))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15029)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15074 || n = 15075)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15076)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 15077 || n = 15078)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15081)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-291)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15082)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-292)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15083)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-293)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15084)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15085)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15086)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15087)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15088)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15089)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 15090)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15091)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #64
	jsr delay_a_25_clocks
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
	lda #26
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15094)
	; 16 bytes
	php
	tya
	pha
	ldy #225
	lda #35
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15096)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15097)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #31
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
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
	lda #40
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15103)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15104)
	; 16 bytes
	php
	tya
	ldy #196
	pha
	lda #38
	jsr delay_a_25_clocks
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
	lda #78
	jsr delay_a_25_clocks
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
	lda #75
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15110)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15111)
	; 16 bytes
	php
	tya
	pha
	ldy #196
	lda #45
	jsr delay_a_25_clocks
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
	lda #85
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
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
	lda #215
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #44
	jsr delay_a_25_clocks
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
.elseif(n = 15120 || n = 15126 || n = 15128 || n = 15131 || n = 15133 || n = 15134 || n = 15138 || n = 15146)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 15121)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #94
	jsr delay_a_25_clocks
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
	lda #231
	jsr delay_a_25_clocks
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
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15124)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15125)
	; 16 bytes
	php
	tya
	pha
	ldy #182
	lda #51
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15127 || n = 15132)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15129)
	; 7 bytes
	php
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15130 || n = 15135)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15136)
	; 9 bytes
	php
	pha
	lda #59
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15137 || n = 15142 || n = 15149 || n = 15150 || n = 15155)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 15139 || n = 15144)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15140 || n = 15145)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15141)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15143)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15147)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 15148)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15151 || n = 15152 || n = 15154)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15153)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15156)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 15157 to 15312
.if((n > 15156 && n < 15160) || n = 15161 || n = 15162 || n = 15164 || n = 15166 || n = 15167 || n = 15169 || (n > 15170 && n < 15180) || (n > 15180 && n < 15184) || (n > 15184 && n < 15240) || (n > 15240 && n < 15245) || (n > 15245 && n < 15285) || (n > 15285 && n < 15313))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15160)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15163)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15165)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15168)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15170 || n = 15180 || n = 15184 || n = 15245 || n = 15285)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15240)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #59
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15330 || n = 15331)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15332 || n = 15345)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 15332 && n < 15337))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15337)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-292)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15338)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-293)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15339)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-294)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15340)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15341)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15342)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15343)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15344)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15346)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #85
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15348)
	; 16 bytes
	php
	tya
	ldy #(n-12)/71
	pha
	lda #32
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15349)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15350)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #235
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
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
	lda #237
	jsr delay_a_25_clocks
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
	lda #50
	jsr delay_a_25_clocks
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
	lda #233
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #73
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15359)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #33
	jsr delay_a_25_clocks
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
	lda #165
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15361)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #240
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	lda #80
	jsr delay_a_25_clocks
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
	lda #61
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15366)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #71
	jsr delay_a_25_clocks
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
	lda #44
	jsr delay_a_25_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15369)
	; 16 bytes
	php
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #67
	jsr delay_a_25_clocks
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
	lda #87
	jsr delay_a_25_clocks
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
	lda #44
	jsr delay_a_25_clocks
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
	lda #51
	jsr delay_a_25_clocks
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
	lda #125
	jsr delay_a_25_clocks
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
	lda #53
	jsr delay_a_25_clocks
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
	lda #86
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15381)
	; 16 bytes
	php
	tya
	ldy #141
	pha
	lda #70
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
.if(n = 15391)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15392)
	; 9 bytes
	php
	pha
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15393 || n = 15398 || n = 15405 || n = 15406 || n = 15411)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 15394 || n = 15402)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15395 || n = 15400)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15396 || n = 15401)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15397)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15399)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15403)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 15404)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15407 || n = 15408 || n = 15410 || n = 15426 || n = 15436 || n = 15440)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15409)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 15411 && n < 15416) || n = 15417 || n = 15418 || n = 15420 || n = 15422 || n = 15423 || n = 15425 || (n > 15426 && n < 15436) || (n > 15436 && n < 15440) || (n > 15440 && n < 15469))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15416)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15419)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15421)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15424)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 15469) && (n < 15547))
; 15469 to 15546
.if((n > 15468 && n < 15496) || (n > 15496 && n < 15501) || (n > 15501 && n < 15541) || (n > 15541 && n < 15547))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15496)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15501 || n = 15541)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if((n > 15546 && n < 15586))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 15586) && (n < 15606))
; 15586 to 15605
.if(n = 15586 || n = 15587)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15588 || n = 15601)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 15588 && n < 15593))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15593)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-293)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15594)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-294)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15595)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-295)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15596)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15597)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15598)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15599)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15600)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15602)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15603)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15604)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15605)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
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
	lda #77
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15608)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #87
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
	lda #37
	jsr delay_a_25_clocks
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
	lda #79
	jsr delay_a_25_clocks
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
	lda #26
	jsr delay_a_25_clocks
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
	lda #37
	jsr delay_a_25_clocks
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
	lda #230
	jsr delay_a_25_clocks
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
	lda #164
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15616)
	; 16 bytes
	php
	tya
	ldy #188
	pha
	lda #44
	jsr delay_a_25_clocks
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
	lda #44
	jsr delay_a_25_clocks
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
	lda #63
	jsr delay_a_25_clocks
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
	lda #33
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15621)
	; 16 bytes
	php
	tya
	pha
	ldy #57
	lda #237
	jsr delay_a_25_clocks
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
	lda #31
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15625)
	; 16 bytes
	php
	tya
	pha
	ldy #153
	lda #70
	jsr delay_a_25_clocks
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
; 15626 to 15664
.if(n = 15626)
	; 16 bytes
	php
	tya
	ldy #211
	pha
	lda #35
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
	lda #38
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	lda #177
	jsr delay_a_25_clocks
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
	lda #32
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
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
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15636)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15637)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #82
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15638 || n = 15640 || n = 15643 || n = 15645 || n = 15646 || n = 15650 || n = 15658)
	; 8..11 bytes
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15642 || n = 15647)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15648)
	; 9 bytes
	php
	pha
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15649 || n = 15654 || n = 15661 || n = 15662)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 15651 || n = 15656)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15652 || n = 15657)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15653)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15655)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15659)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 15660)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15663 || n = 15664)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 15665 to 15703
.if(n = 15665)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15666 || n = 15682 || n = 15692 || n = 15696)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15667)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif((n > 15667 && n < 15672) || n = 15673 || n = 15674 || n = 15676 || n = 15678 || n = 15679 || n = 15681 || (n > 15682 && n < 15692) || (n > 15692 && n < 15696) || (n > 15696 && n < 15704))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15672)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15675)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15677)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15680)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 15704 to 15781
.if((n > 15703 && n < 15752) || (n > 15752 && n < 15757) || (n > 15757 && n < 15782))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15752)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15757)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
.if((n >= 15782) && (n < 15860))
; 15782 to 15859
.if((n > 15781 && n < 15797) || (n > 15797 && n < 15842))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15797)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15842 || n = 15843)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15844 || n = 15857)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 15844 && n < 15849))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15849)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-294)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15850)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-295)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15851)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-296)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15852)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15853)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15854)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15855)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15856)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15858)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #75
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
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
	lda #57
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15863)
	; 16 bytes
	php
	tya
	pha
	ldy #233
	lda #36
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15865)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #82
	jsr delay_a_25_clocks
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
	lda #251
	jsr delay_a_25_clocks
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
	sec
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
	lda #62
	jsr delay_a_25_clocks
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
	lda #164
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15872)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #51
	jsr delay_a_25_clocks
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
	lda #166
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15874)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #73
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15876)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15881)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #45
	jsr delay_a_25_clocks
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
	lda #30
	jsr delay_a_25_clocks
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
	lda #230
	jsr delay_a_25_clocks
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
	lda #72
	jsr delay_a_25_clocks
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
	lda #91
	jsr delay_a_25_clocks
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
	lda #86
	jsr delay_a_25_clocks
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
	lda #37
	jsr delay_a_25_clocks
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
	lda #237
	jsr delay_a_25_clocks
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
	lda #$4A ;hides 'lsr a'
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15893 || n = 15894 || n = 15896)
	; 8..17 bytes
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15898)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 15899 to 15937
.if(n = 15899 || n = 15901 || n = 15902 || n = 15906 || n = 15914)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15900)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15903)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15904)
	; 9 bytes
	php
	pha
	lda #62
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15905 || n = 15910 || n = 15917 || n = 15918 || n = 15923)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 15907 || n = 15912)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15908 || n = 15913)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15909)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15911)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15915)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 15916)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15919 || n = 15920 || n = 15922)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15921)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 15923 && n < 15928) || n = 15929 || n = 15930 || n = 15932 || n = 15934 || n = 15935 || n = 15937)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15928)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15931)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15933)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15936)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 15938) && (n < 16094))
; 15938 to 16093
.if(n = 15938 || n = 15948 || n = 15952 || n = 16013 || n = 16053)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif((n > 15938 && n < 15948) || (n > 15948 && n < 15952) || (n > 15952 && n < 16008) || (n > 16008 && n < 16013) || (n > 16013 && n < 16053) || (n > 16053 && n < 16094))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16008)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #62
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16098 || n = 16099)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16100)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16100 && n < 16105))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16105)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-295)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16106)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-296)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16107)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-297)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16108)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16109)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16110)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16111)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16112)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16114)
	; 16 bytes
	php
	tya
	pha
	ldy #185
	lda #55
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16116)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #27
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16118)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16121)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #51
	jsr delay_a_25_clocks
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
	lda #51
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
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
	lda #37
	jsr delay_a_25_clocks
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
	lda #59
	jsr delay_a_25_clocks
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
	lda #254
	jsr delay_a_25_clocks
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
	lda #57
	jsr delay_a_25_clocks
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
	lda #58
	jsr delay_a_25_clocks
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
	lda #44
	jsr delay_a_25_clocks
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
	lda #91
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16134)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16135)
	; 16 bytes
	php
	tya
	pha
	ldy #237
	lda #36
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #49
	jsr delay_a_25_clocks
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
	lda #33
	jsr delay_a_25_clocks
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
	lda #88
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16146)
	; 17 bytes
	php
	txa
	pha
	sec
	lda #124
	ldx #232 ;hides 'inx'
	Jne *-1
	sbc #1
	Jpl *-6
	pla
	tax
	plp
.elseif(n = 16147)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16148)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16149)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #60
	jsr delay_a_25_clocks
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
.if(n = 16152 || n = 16155 || n = 16157 || n = 16158 || n = 16162 || n = 16170)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16153)
	; 7 bytes
	php
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16154 || n = 16159)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16156)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16160)
	; 9 bytes
	php
	pha
	lda #63
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16161 || n = 16166)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16163 || n = 16168)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16164 || n = 16169)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16165)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16167)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16171)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16173 || n = 16174 || n = 16179)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16175 || n = 16176 || n = 16178 || n = 16194 || n = 16204 || n = 16208)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16177)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 16179 && n < 16184) || n = 16185 || n = 16186 || n = 16188 || n = 16190 || n = 16191 || n = 16193 || (n > 16194 && n < 16204) || (n > 16204 && n < 16208) || (n > 16208 && n < 16251))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16184)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16187)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16189)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16192)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
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
.if((n > 16250 && n < 16264) || (n > 16264 && n < 16269) || (n > 16269 && n < 16309) || (n > 16309 && n < 16329))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16264)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16269 || n = 16309)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 16329) && (n < 16368))
; 16329 to 16367
.if((n > 16328 && n < 16354))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16354 || n = 16355)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16356)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16356 && n < 16361))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16361)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-296)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16362)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-297)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16363)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-298)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16364)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16365)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16366)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16367)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 16368) && (n < 16387))
; 16368 to 16386
.if(n = 16368)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16369)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16370)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #51
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16371)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16372)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16373)
	; 16 bytes
	php
	tya
	pha
	ldy #221
	lda #42
	jsr delay_a_25_clocks
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
	lda #42
	jsr delay_a_25_clocks
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
	lda #253
	jsr delay_a_25_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16378)
	; 16 bytes
	php
	tya
	pha
	ldy #133
	lda #91
	jsr delay_a_25_clocks
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
	lda #160
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16380)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #27
	jsr delay_a_25_clocks
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
	lda #49
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16383)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #68
	jsr delay_a_25_clocks
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
	lda #66
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
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
	lda #50
	jsr delay_a_25_clocks
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
	lda #(n-5)/256
	jsr delay_a_25_clocks
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
	lda #75
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16392)
	; 16 bytes
	php
	tya
	ldy #252
	pha
	lda #26
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	lda #93
	jsr delay_a_25_clocks
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
	lda #57
	jsr delay_a_25_clocks
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
	lda #74
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16398 || n = 16401 || n = 16406)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 16399)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #33
	jsr delay_a_25_clocks
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
	lda #29
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16402)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #95
	jsr delay_a_25_clocks
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
	lda #32
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16404)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16405)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #58
	jsr delay_a_25_clocks
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
.if(n = 16407 || n = 16412)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16408 || n = 16411 || n = 16413 || n = 16414 || n = 16418 || n = 16426)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16409)
	; 7 bytes
	php
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16410 || n = 16415)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16416)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16417 || n = 16422 || n = 16429 || n = 16430 || n = 16435)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16419 || n = 16424)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16420 || n = 16425)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16421)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16423)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16427)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 16428)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16431 || n = 16432 || n = 16434 || n = 16450 || n = 16460 || n = 16464 || n = 16525)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16433)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 16435 && n < 16440) || n = 16441 || n = 16442 || n = 16444 || n = 16446 || n = 16447 || n = 16449 || (n > 16450 && n < 16460) || (n > 16460 && n < 16464) || (n > 16464 && n < 16520) || (n > 16520 && n < 16525) || (n > 16525 && n < 16563))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16440)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16443)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16445)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16448)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16520)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #64
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
.if((n >= 16563) && (n < 16602))
; 16563 to 16601
.if(n = 16563 || n = 16564 || (n > 16565 && n < 16602))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16565)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 16602) && (n < 16621))
; 16602 to 16620
.if((n > 16601 && n < 16610))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16610 || n = 16611)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16612)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16612 && n < 16617))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16617)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-297)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16618)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-298)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16619)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-299)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16620)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 16621 to 16640
.if(n = 16621)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16622)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16623)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16624)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16625)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16626)
	; 16 bytes
	php
	tya
	ldy #(n-12)/71
	pha
	lda #32
	jsr delay_a_25_clocks
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
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16628)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16629)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #78
	jsr delay_a_25_clocks
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
	lda #81
	jsr delay_a_25_clocks
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
	lda #37
	jsr delay_a_25_clocks
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
	lda #238
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #159
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16635)
	; 16 bytes
	php
	tya
	pha
	ldy #248
	lda #35
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #63
	jsr delay_a_25_clocks
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
	lda #245
	jsr delay_a_25_clocks
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
	lda #165
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
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
	lda #(n-561)/536
	jsr delay_a_25_clocks
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
	lda #190
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
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
	lda #70
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 16647)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16648)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #37
	jsr delay_a_25_clocks
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
	lda #88
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
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
	lda #90
	jsr delay_a_25_clocks
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
	lda #67
	jsr delay_a_25_clocks
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
	lda #50
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #62
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
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
	lda #33
	jsr delay_a_25_clocks
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
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16661)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/106
	lda #74
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16662 || n = 16664 || n = 16667 || n = 16669 || n = 16670 || n = 16674)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16663 || n = 16668)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16665)
	; 7 bytes
	php
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16666 || n = 16671)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16672)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16673 || n = 16678)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16675)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16676)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
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
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16681)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16682)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16683)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 16684)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16685 || n = 16686 || n = 16691)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16687 || n = 16688 || n = 16690 || n = 16706 || n = 16716)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16689)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 16691 && n < 16696) || n = 16697 || n = 16698 || n = 16700 || n = 16702 || n = 16703 || n = 16705 || (n > 16706 && n < 16716) || n = 16717 || n = 16718)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16696)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16699)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16701)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16704)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 16719 to 16875
.if(n = 16719 || (n > 16720 && n < 16776) || (n > 16776 && n < 16781) || (n > 16781 && n < 16821) || (n > 16821 && n < 16866))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16720 || n = 16781 || n = 16821)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16776)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16866 || n = 16867)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16868)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16868 && n < 16873))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16873)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-298)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16874)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-299)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16875)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-300)/255
	jsr delay_256a_16_clocks
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
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16877)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16878)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16879)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16880)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16881)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16882)
	; 16 bytes
	php
	tya
	pha
	ldy #231
	lda #41
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16886)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16889)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #38
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #42
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
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
	lda #86
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
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
	lda #164
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16896)
	; 16 bytes
	php
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
	plp
.elseif(n = 16897)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #55
	jsr delay_a_25_clocks
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
	lda #167
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16899)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #48
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16901)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16902)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16903)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #35
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
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
	lda #70
	jsr delay_a_25_clocks
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
	lda #238
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #50
	jsr delay_a_25_clocks
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
	lda #61
	jsr delay_a_25_clocks
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
	lda #77
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16916)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16917)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16918 || n = 16920 || n = 16923 || n = 16925 || n = 16926 || n = 16930 || n = 16938)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16919 || n = 16924)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16921)
	; 7 bytes
	php
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16922 || n = 16927)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16928)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16929 || n = 16934 || n = 16941 || n = 16942 || n = 16947)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16931 || n = 16936)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16932 || n = 16937)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16933)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16935)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16939)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 16940)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16943 || n = 16944 || n = 16946)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16945)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 16947 && n < 16952) || n = 16953)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16952)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 16954 to 17031
.if(n = 16954 || n = 16956 || n = 16958 || n = 16959 || n = 16961 || (n > 16962 && n < 16972) || (n > 16972 && n < 16976) || (n > 16976 && n < 17032))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16955)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16957)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16960)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16962 || n = 16972 || n = 16976)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
; 17032 to 17109
.if(n = 17032)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif((n > 17032 && n < 17037) || (n > 17037 && n < 17077) || (n > 17077 && n < 17110))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17037 || n = 17077)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 17110) && (n < 17149))
; 17110 to 17148
.if((n > 17109 && n < 17122))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17122 || n = 17123)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17124 || n = 17137)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 17124 && n < 17129))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17129)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-299)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17130)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-300)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17131)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-301)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17132)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17133)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17134)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17135)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17136)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17138)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17139)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #48
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17141)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17142)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17143)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17144)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #93
	jsr delay_a_25_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17146)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #67
	jsr delay_a_25_clocks
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
	lda #76
	jsr delay_a_25_clocks
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
	lda #162
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17150)
	; 16 bytes
	php
	tya
	ldy #209
	pha
	lda #43
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #165
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17153)
	; 16 bytes
	php
	tya
	ldy #(n-12)/281
	pha
	lda #242
	jsr delay_a_25_clocks
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
	lda #83
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17157)
	; 16 bytes
	php
	tya
	pha
	ldy #209
	lda #50
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17160)
	; 16 bytes
	php
	tya
	pha
	ldy #61
	lda #249
	jsr delay_a_25_clocks
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
	lda #203
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17163)
	; 17 bytes
	php
	lda #90
	jsr delay_a_25_clocks
	lda #98
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17164)
	; 16 bytes
	php
	tya
	ldy #0
	pha
	lda #$4A ;hides 'lsr a'
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
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	pla
	Jcs *-12
	plp
.elseif(n = 17166)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 17167)
	; 16 bytes
	php
	tya
	ldy #235
	pha
	lda #34
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
	lda #38
	jsr delay_a_25_clocks
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
	lda #124
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17172)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17173)
	; 16 bytes
	php
	tya
	ldy #(n-12)/131
	pha
	lda #92
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17174 || n = 17176 || n = 17179 || n = 17181 || n = 17182 || n = 17186)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17175 || n = 17180)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17177)
	; 7 bytes
	php
	lda #67
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17185)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17187)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
; 17188 to 17343
.if(n = 17188 || n = 17193)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17189)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17190 || n = 17197 || n = 17198 || n = 17203)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17191)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17192)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17194)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17195)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 17196)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17199 || n = 17200 || n = 17202 || n = 17218 || n = 17228 || n = 17232 || n = 17293 || n = 17333)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17201)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 17203 && n < 17208) || n = 17209 || n = 17210 || n = 17212 || n = 17214 || n = 17215 || n = 17217 || (n > 17218 && n < 17228) || (n > 17228 && n < 17232) || (n > 17232 && n < 17288) || (n > 17288 && n < 17293) || (n > 17293 && n < 17333) || (n > 17333 && n < 17344))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17208)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17211)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17213)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17216)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17288)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #67
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
; 17344 to 17382
.if((n > 17343 && n < 17378))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17378 || n = 17379)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17380)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 17381 || n = 17382)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17385)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-300)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17386)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-301)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17387)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-302)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17388)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17389)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17390)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17391)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17392)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17393)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 17394)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #93
	jsr delay_a_25_clocks
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
	lda #64
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17399)
	; 16 bytes
	php
	tya
	pha
	ldy #220
	lda #47
	jsr delay_a_25_clocks
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
	lda #59
	jsr delay_a_25_clocks
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
	lda #130
	jsr delay_a_25_clocks
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
	lda #35
	jsr delay_a_25_clocks
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
	lda #50
	jsr delay_a_25_clocks
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
	lda #63
	jsr delay_a_25_clocks
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
	lda #92
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #165
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17409)
	; 16 bytes
	php
	tya
	pha
	ldy #235
	lda #42
	jsr delay_a_25_clocks
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
	lda #61
	jsr delay_a_25_clocks
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
	lda #88
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
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
	lda #46
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17416)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #37
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17417)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #41
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 17418)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #95
	jsr delay_a_25_clocks
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
	lda #45
	jsr delay_a_25_clocks
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
.if(n = 17422 || n = 17425 || n = 17429 || n = 17430 || n = 17432 || n = 17435 || n = 17437 || n = 17438 || n = 17442 || n = 17450)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 17423)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17424)
	; 17 bytes
	php
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17426)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #71
	jsr delay_a_25_clocks
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
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17428)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17431 || n = 17436)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17433)
	; 7 bytes
	php
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17434 || n = 17439)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17440)
	; 9 bytes
	php
	pha
	lda #68
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17441 || n = 17446 || n = 17453 || n = 17454 || n = 17459)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17443 || n = 17448)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17444 || n = 17449)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17445)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17447)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17451)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 17452)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17455 || n = 17456 || n = 17458 || n = 17474 || n = 17484 || n = 17488)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17457)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 17459 && n < 17464) || n = 17465 || n = 17466 || n = 17468 || n = 17470 || n = 17471 || n = 17473 || (n > 17474 && n < 17484) || (n > 17484 && n < 17488) || (n > 17488 && n < 17501))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17464)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17467)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17469)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17472)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
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
.if((n > 17500 && n < 17544) || (n > 17544 && n < 17549) || (n > 17549 && n < 17589) || (n > 17589 && n < 17634))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17544)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17549 || n = 17589)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17634 || n = 17635)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17636 || n = 17649)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 17636 && n < 17641))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17641)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-301)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17642)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-302)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17643)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-303)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17644)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17645)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17646)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17647)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17648)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17650)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #204
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 17651)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17652)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17653)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17654)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17655)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17656)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
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
	lda #211
	jsr delay_a_25_clocks
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
	lda #63
	jsr delay_a_25_clocks
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
	lda #38
	jsr delay_a_25_clocks
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
	lda #161
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17661)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #72
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17663)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17664)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17665)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #70
	jsr delay_a_25_clocks
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
	lda #52
	jsr delay_a_25_clocks
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
	lda #68
	jsr delay_a_25_clocks
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
	lda #79
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17670)
	; 16 bytes
	php
	tya
	ldy #(n-12)/81
	pha
	lda #42
	jsr delay_a_25_clocks
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
	lda #63
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #59
	jsr delay_a_25_clocks
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
	lda #75
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #49
	jsr delay_a_25_clocks
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
	lda #34
	jsr delay_a_25_clocks
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
	lda #78
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	lda #62
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17684)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
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
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17697 || n = 17702 || n = 17709 || n = 17710 || n = 17715)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17698 || n = 17706)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17699 || n = 17704)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17700 || n = 17705)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17701)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17703)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17707)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 17708)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17711 || n = 17712 || n = 17714 || n = 17730)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17713)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 17715 && n < 17720) || n = 17721 || n = 17722 || n = 17724 || n = 17726 || n = 17727 || n = 17729 || (n > 17730 && n < 17735))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17720)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17723)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17725)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17728)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 17735 to 17812
.if((n > 17734 && n < 17740) || (n > 17740 && n < 17744) || (n > 17744 && n < 17800) || (n > 17800 && n < 17805) || (n > 17805 && n < 17813))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17740 || n = 17744 || n = 17805)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17800)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #69
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
; 17813 to 17890
.if((n > 17812 && n < 17845) || (n > 17845 && n < 17890))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17845)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17890)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
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
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17892 || n = 17905)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 17892 && n < 17897))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17897)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-302)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17898)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-303)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17899)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-304)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17900)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17901)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17902)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17903)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17904)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17906)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #167
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #57
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
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
	lda #75
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #61
	jsr delay_a_25_clocks
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
	lda #42
	jsr delay_a_25_clocks
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
	lda #159
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17915)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17916)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17917)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #82
	jsr delay_a_25_clocks
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
	lda #107
	jsr delay_a_25_clocks
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
	lda #68
	jsr delay_a_25_clocks
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
	lda #49
	jsr delay_a_25_clocks
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
	lda #166
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17922)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #51
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17924)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17925)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #208
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17929)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #58
	jsr delay_a_25_clocks
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
	lda #250
	jsr delay_a_25_clocks
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
	lda #60
	jsr delay_a_25_clocks
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
	lda #74
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17935)
	; 17 bytes
	php
	tya
	pha
	sec
	lda #26
	ldy #136 ;hides 'dey'
	Jne *-1
	sbc #1
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17936 || n = 17942 || n = 17944 || n = 17947 || n = 17949 || n = 17950 || n = 17954 || n = 17962)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 17937)
	; 16 bytes
	php
	tya
	ldy #239
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17938)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #67
	jsr delay_a_25_clocks
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
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17940)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17941)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #(n-11)/326
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17943 || n = 17948)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17945)
	; 7 bytes
	php
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17946 || n = 17951)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17952)
	; 9 bytes
	php
	pha
	lda #70
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17953 || n = 17958 || n = 17965 || n = 17966)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17955 || n = 17960)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17956 || n = 17961)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17957)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17959)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17963)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 17964)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17967 || n = 17968)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17970 || n = 17986 || n = 17996 || n = 18000 || n = 18061 || n = 18101)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17971)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif((n > 17971 && n < 17976) || n = 17977 || n = 17978 || n = 17980 || n = 17982 || n = 17983 || n = 17985 || (n > 17986 && n < 17996) || (n > 17996 && n < 18000) || (n > 18000 && n < 18056) || (n > 18056 && n < 18061) || (n > 18061 && n < 18101) || (n > 18101 && n < 18126))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17976)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17979)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17981)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17984)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18056)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #70
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18146 || n = 18147)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18148 || n = 18161)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 18148 && n < 18153))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18153)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-303)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18154)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-304)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18155)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-305)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18156)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18157)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18158)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18159)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18160)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18162)
	; 16 bytes
	php
	tya
	ldy #242
	pha
	lda #36
	jsr delay_a_25_clocks
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
	lda #40
	jsr delay_a_25_clocks
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
	lda #63
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #231
	jsr delay_a_25_clocks
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
	lda #87
	jsr delay_a_25_clocks
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
	lda #50
	jsr delay_a_25_clocks
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
	lda #159
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18171)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18172)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	lda #41
	jsr delay_a_25_clocks
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
	lda #88
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18175)
	; 16 bytes
	php
	tya
	pha
	ldy #204
	lda #57
	jsr delay_a_25_clocks
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
	lda #239
	jsr delay_a_25_clocks
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
	lda #66
	jsr delay_a_25_clocks
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
	lda #254
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18182)
	; 16 bytes
	php
	tya
	ldy #230
	pha
	lda #40
	jsr delay_a_25_clocks
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
	lda #44
	jsr delay_a_25_clocks
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
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18185)
	; 17 bytes
	php
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
	plp
.elseif(n = 18186)
	; 16 bytes
	php
	tya
	ldy #233
	pha
	lda #39
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #83
	jsr delay_a_25_clocks
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
	lda #45
	jsr delay_a_25_clocks
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
	lda #51
	jsr delay_a_25_clocks
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
	lda #46
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18196)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18197)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #90
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
.if(n = 18204)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18205 || n = 18206 || n = 18210 || n = 18218)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18207)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18208)
	; 9 bytes
	php
	pha
	lda #71
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18209 || n = 18214 || n = 18221 || n = 18222 || n = 18227)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18211 || n = 18216)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18212 || n = 18217)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18213)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18215)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18219)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 18220)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18223 || n = 18224 || n = 18226 || n = 18242 || n = 18252 || n = 18256)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18225)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 18227 && n < 18232) || n = 18233 || n = 18234 || n = 18236 || n = 18238 || n = 18239 || n = 18241 || (n > 18242 && n < 18252) || (n > 18252 && n < 18256) || (n > 18256 && n < 18282))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18232)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18235)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18237)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18240)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
; 18282 to 18359
.if((n > 18281 && n < 18312) || (n > 18312 && n < 18317) || (n > 18317 && n < 18357) || n = 18358 || n = 18359)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18312)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18317 || n = 18357)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if((n > 18359 && n < 18399))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 18399) && (n < 18418))
; 18399 to 18417
.if((n > 18398 && n < 18402))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18402 || n = 18403)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18404 || n = 18417)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 18404 && n < 18409))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18409)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-304)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18410)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-305)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18411)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-306)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18412)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18413)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18414)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18415)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18416)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
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
	lda #136
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #39
	jsr delay_a_25_clocks
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
	lda #52
	jsr delay_a_25_clocks
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
	lda #45
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18425)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #143
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #46
	jsr delay_a_25_clocks
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
	lda #161
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18429)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #231
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18431)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #74
	jsr delay_a_25_clocks
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
	lda #165
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18433)
	; 16 bytes
	php
	tya
	pha
	ldy #198
	lda #61
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #50
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	lda #236
	jsr delay_a_25_clocks
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
	lda #81
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18440)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #77
	jsr delay_a_25_clocks
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
	lda #90
	jsr delay_a_25_clocks
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
	lda #62
	jsr delay_a_25_clocks
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
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18446 || n = 18454 || n = 18456)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 18447)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/271
	lda #239
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18448)
	; 17 bytes
	php
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18449)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18450)
	; 17 bytes
	php
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18451)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #40
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18452)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18453)
	; 17 bytes
	php
	txa
	pha
	sec
	lda #47
	ldx #202 ;hides 'dex'
	Jmi *-1
	sbc #1
	Jpl *-6
	pla
	tax
	plp
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
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18458 || n = 18463)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18459 || n = 18461 || n = 18462 || n = 18466 || n = 18474)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18460)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18464)
	; 9 bytes
	php
	pha
	lda #72
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18465 || n = 18470)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18467 || n = 18472)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18468 || n = 18473)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18469)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18471)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18475)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 18476)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 18477 to 18515
.if(n = 18477 || n = 18478 || n = 18483)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18479 || n = 18480 || n = 18482 || n = 18498 || n = 18508 || n = 18512)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18481)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 18483 && n < 18488) || n = 18489 || n = 18490 || n = 18492 || n = 18494 || n = 18495 || n = 18497 || (n > 18498 && n < 18508) || (n > 18508 && n < 18512) || (n > 18512 && n < 18516))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18488)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18491)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18493)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18496)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 18516 to 18593
.if((n > 18515 && n < 18568) || (n > 18568 && n < 18573) || (n > 18573 && n < 18594))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18568)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18573)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
; 18594 to 18671
.if((n > 18593 && n < 18613) || (n > 18613 && n < 18658))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18613)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18658 || n = 18659)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18660)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 18660 && n < 18665))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18665)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-305)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18666)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-306)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18667)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-307)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18668)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18669)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18670)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18671)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 18672) && (n < 18711))
.if((n >= 18672) && (n < 18691))
; 18672 to 18690
.if(n = 18672)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18673)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18674)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #59
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18677)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18678)
	; 16 bytes
	php
	tya
	ldy #183
	pha
	lda #63
	jsr delay_a_25_clocks
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
	lda #183
	jsr delay_a_25_clocks
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
	lda #172
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18681)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
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
	lda #159
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18683)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
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
	lda #70
	jsr delay_a_25_clocks
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
	lda #163
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18687)
	; 16 bytes
	php
	tya
	ldy #249
	pha
	lda #36
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #166
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18690)
	; 16 bytes
	php
	tya
	ldy #65
	pha
	lda #244
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18692)
	; 16 bytes
	php
	tya
	pha
	ldy #70
	lda #231
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18694)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_
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
	lda #69
	jsr delay_a_25_clocks
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
	lda #251
	jsr delay_a_25_clocks
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
	jsr delay_256a_16_clocks
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
	lda #225
	jsr delay_a_25_clocks
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
	lda #87
	jsr delay_a_25_clocks
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
	lda #76
	jsr delay_a_25_clocks
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
	lda #69
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #111
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18708)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18709)
	; 16 bytes
	php
	tya
	pha
	ldy #210
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
; 18711 to 18750
.if(n = 18711 || n = 18716)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18712 || n = 18715 || n = 18717 || n = 18718 || n = 18722 || n = 18730)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18713)
	; 7 bytes
	php
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18714 || n = 18719)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18720)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18721 || n = 18726 || n = 18733 || n = 18734 || n = 18739)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18723 || n = 18728)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18724 || n = 18729)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18725)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18727)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18731)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 18732)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18735 || n = 18736 || n = 18738)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18737)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 18739 && n < 18744) || n = 18745 || n = 18746 || n = 18748 || n = 18750)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18744)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18747)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18749)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
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
.if(n = 18751 || n = 18753 || (n > 18754 && n < 18764) || (n > 18764 && n < 18768) || (n > 18768 && n < 18824) || (n > 18824 && n < 18829) || (n > 18829 && n < 18869) || (n > 18869 && n < 18907))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18752)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18754 || n = 18764 || n = 18768 || n = 18829 || n = 18869)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18824)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #73
	jsr delay_256a_16_clocks
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
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18914 || n = 18915)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18916)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 18916 && n < 18921))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18921)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-306)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18922)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-307)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18923)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-308)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18924)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18925)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 18926 to 18945
.if(n = 18926)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18927)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18928)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18929)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18930)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18931)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #64
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #66
	jsr delay_a_25_clocks
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
	lda #65
	jsr delay_a_25_clocks
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
	lda #88
	jsr delay_a_25_clocks
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
	lda #37
	jsr delay_a_25_clocks
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
	lda #186
	jsr delay_a_25_clocks
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
	lda #141
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #87
	jsr delay_a_25_clocks
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
	lda #162
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18942)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #95
	jsr delay_a_25_clocks
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
	lda #44
	jsr delay_a_25_clocks
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
	lda #165
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18945)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #148
	jsr delay_a_25_clocks
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
	lda #222
	jsr delay_a_25_clocks
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
	lda #59
	jsr delay_a_25_clocks
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
	lda #224
	jsr delay_a_25_clocks
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
	lda #136
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18951)
	; 16 bytes
	php
	tya
	ldy #177
	pha
	lda #68
	jsr delay_a_25_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18953)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #229
	jsr delay_a_25_clocks
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
	lda #231
	jsr delay_a_25_clocks
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
	lda #75
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18960)
	; 17 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	nop
	pha
	tya
	ldy #161
	dey
	Jne *-1
	tay
	pla
	Jcs *-12
	plp
.elseif(n = 18961)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #45
	jsr delay_a_25_clocks
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
	lda #62
	jsr delay_a_25_clocks
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
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18964)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 18965 to 18984
.if(n = 18965 || n = 18966 || n = 18968 || n = 18971 || n = 18973 || n = 18974 || n = 18978)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 18967 || n = 18972)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18969)
	; 7 bytes
	php
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18970 || n = 18975)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18976)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18977 || n = 18982)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18979 || n = 18984)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18980)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18981)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18983)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18986)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18987)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 18988)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18989 || n = 18990 || n = 18995)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18991 || n = 18992 || n = 18994 || n = 19010 || n = 19020 || n = 19024)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18993)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 18995 && n < 19000) || n = 19001 || n = 19002 || n = 19004 || n = 19006 || n = 19007 || n = 19009 || (n > 19010 && n < 19020) || (n > 19020 && n < 19024) || (n > 19024 && n < 19063))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19000)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19003)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19005)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19008)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
; 19063 to 19140
.if((n > 19062 && n < 19080) || (n > 19080 && n < 19085) || (n > 19085 && n < 19125) || (n > 19125 && n < 19141))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19080)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19085 || n = 19125)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 19141) && (n < 19180))
; 19141 to 19179
.if((n > 19140 && n < 19170))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19170 || n = 19171)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19172)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 19172 && n < 19177))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19177)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-307)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19178)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-308)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19179)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-309)/255
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 19180) && (n < 19199))
; 19180 to 19198
.if(n = 19180)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19181)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19182)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19183)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19184)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19185)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 19186)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19187)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19188)
	; 16 bytes
	php
	tya
	ldy #204
	pha
	lda #55
	jsr delay_a_25_clocks
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
	lda #58
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19192)
	; 16 bytes
	php
	tya
	pha
	ldy #249
	lda #45
	jsr delay_a_25_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19194)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #99
	jsr delay_a_25_clocks
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
	lda #62
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #67
	jsr delay_a_25_clocks
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
	lda #242
	jsr delay_a_25_clocks
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
	lda #246
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19205)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/106
	lda #74
	jsr delay_a_25_clocks
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
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19207)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #46
	jsr delay_a_25_clocks
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
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19209)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #63
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	lda #224
	jsr delay_a_25_clocks
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
	lda #52
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19214)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 19215)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #72
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #76
	jsr delay_a_25_clocks
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
	lda #231
	jsr delay_a_25_clocks
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
	lda #43
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19220)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19221)
	; 17 bytes
	php
	txa
	pha
	sec
	lda #50
	ldx #202 ;hides 'dex'
	Jmi *-1
	sbc #1
	Jne *-6
	pla
	tax
	plp
.elseif(n = 19222 || n = 19224 || n = 19227 || n = 19229 || n = 19230 || n = 19234 || n = 19242)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19223 || n = 19228)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19225)
	; 7 bytes
	php
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19226 || n = 19231)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19232)
	; 9 bytes
	php
	pha
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19233 || n = 19238 || n = 19245 || n = 19246 || n = 19251)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 19235 || n = 19240)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19236 || n = 19241)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19237)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19239)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19243)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 19244)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19247 || n = 19248 || n = 19250 || n = 19266 || n = 19276 || n = 19280 || n = 19341)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19249)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 19251 && n < 19256) || n = 19257 || n = 19258 || n = 19260 || n = 19262 || n = 19263 || n = 19265 || (n > 19266 && n < 19276) || (n > 19276 && n < 19280) || (n > 19280 && n < 19336) || (n > 19336 && n < 19341) || (n > 19341 && n < 19376))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19256)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19259)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19261)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19264)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19336)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #75
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 19376) && (n < 19688))
.if((n >= 19376) && (n < 19532))
.if((n >= 19376) && (n < 19454))
; 19376 to 19453
.if((n > 19375 && n < 19381) || (n > 19381 && n < 19426))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19381)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19426 || n = 19427)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19428 || n = 19441)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 19428 && n < 19433))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19433)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-308)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19434)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-309)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19435)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-310)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19436)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19437)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19438)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19439)
	; 15 bytes
	php
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19440)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19442)
	; 16 bytes
	php
	tya
	ldy #145
	pha
	lda #95
	jsr delay_a_25_clocks
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
	lda #88
	jsr delay_a_25_clocks
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
	lda #73
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
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
	lda #76
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #45
	jsr delay_a_25_clocks
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
	lda #41
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #83
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #61
	jsr delay_a_25_clocks
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
	lda #236
	jsr delay_a_25_clocks
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
	lda #146
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #49
	jsr delay_a_25_clocks
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
	lda #170
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19462)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19463)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19464)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19465)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #153
	jsr delay_a_25_clocks
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
	lda #98
	jsr delay_a_25_clocks
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
	lda #56
	jsr delay_a_25_clocks
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
	lda #55
	jsr delay_a_25_clocks
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
	ldy #69
	pha
	lda #239
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
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
	lda #242
	jsr delay_a_25_clocks
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
	lda #224
	jsr delay_a_25_clocks
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
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19476)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
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
.elseif(n = 19478 || n = 19480 || n = 19483 || n = 19485 || n = 19486 || n = 19490)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19479 || n = 19484)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19481)
	; 7 bytes
	php
	lda #76
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19489)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 19491)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19492)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.else
; 19493 to 19531
.if(n = 19493)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19494 || n = 19501 || n = 19502 || n = 19507)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 19495)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19496)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19497)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19498)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19499)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 19500)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19503 || n = 19504 || n = 19506 || n = 19522)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19505)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 19507 && n < 19512) || n = 19513 || n = 19514 || n = 19516 || n = 19518 || n = 19519 || n = 19521 || (n > 19522 && n < 19532))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19512)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19515)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19517)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19520)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 19532 to 19687
.if(n = 19532 || n = 19536 || n = 19597 || n = 19637)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif((n > 19532 && n < 19536) || (n > 19536 && n < 19592) || (n > 19592 && n < 19597) || (n > 19597 && n < 19637) || (n > 19637 && n < 19682))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19592)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19682 || n = 19683)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19684)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 19684 && n < 19688))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19689)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-309)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19690)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-310)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19691)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #(n-311)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19692)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19693)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19694)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19695)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19696)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19697)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 19698)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #63
	jsr delay_a_25_clocks
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
	lda #48
	jsr delay_a_25_clocks
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
	lda #53
	jsr delay_a_25_clocks
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
	lda #232
	jsr delay_a_25_clocks
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
	lda #49
	jsr delay_a_25_clocks
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
	ldy #76
	lda #227
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #70
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #161
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19709)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #78
	jsr delay_a_25_clocks
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
	lda #65
	jsr delay_a_25_clocks
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
	lda #195
	jsr delay_a_25_clocks
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
	lda #61
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #69
	jsr delay_a_25_clocks
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
	lda #168
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19716)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19717)
	; 16 bytes
	php
	tya
	pha
	ldy #201
	lda #66
	jsr delay_a_25_clocks
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
	lda #79
	jsr delay_a_25_clocks
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
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19721)
	; 17 bytes
	php
	sec
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
.elseif(n = 19722)
	; 16 bytes
	php
	tya
	ldy #219
	pha
	lda #51
	jsr delay_a_25_clocks
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
	lda #133
	jsr delay_a_25_clocks
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
	lda #86
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19726)
	; 17 bytes
	delay_n (n-2)
	nop
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
	lda #185
	jsr delay_a_25_clocks
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
	lda #54
	jsr delay_a_25_clocks
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
	lda #58
	jsr delay_a_25_clocks
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
	lda #44
	jsr delay_a_25_clocks
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
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19732)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19733)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #152
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19734 || n = 19736 || n = 19739 || n = 19741 || n = 19742 || n = 19746 || n = 19754)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19735 || n = 19740)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19737)
	; 7 bytes
	php
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19738 || n = 19743)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19744)
	; 9 bytes
	php
	pha
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19745 || n = 19750 || n = 19757 || n = 19758 || n = 19763)
	; 9..11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 19747 || n = 19752)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19748 || n = 19753)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19749)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19751)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19755)
	; 11 bytes
	delay_n 18
	delay_n (n-18)
.elseif(n = 19756)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19759 || n = 19760 || n = 19762)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19761)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19764 || n = 19765)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 19766 to 19843
.if(n = 19766 || n = 19767 || n = 19769 || n = 19770 || n = 19772 || n = 19774 || n = 19775 || n = 19777 || (n > 19778 && n < 19788) || (n > 19788 && n < 19792) || (n > 19792 && n < 19844))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19768)
	; 12 bytes
	php
	clc
	lda #<(n-46) ;hides 'asl a'
	Jcc *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19771)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19773)
	; 12 bytes
	php
	nop
	lda #<(n+13) ;hides 'lsr a'
	Jne *-1
	lda #>(n+13)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19776)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19778 || n = 19788 || n = 19792)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
; 19844 to 19921
.if((n > 19843 && n < 19848) || (n > 19848 && n < 19853) || (n > 19853 && n < 19893) || (n > 19893 && n < 19922))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19848)
	; 12 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19853 || n = 19893)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 19922) && (n < 19961))
; 19922 to 19960
.if((n > 19921 && n < 19938))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19938 || n = 19939)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19940 || n = 19953)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 19940 && n < 19945))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19945)
	; 14 bytes
	php
	brk 0
	lda #168
	jsr delay_a_25_clocks
	lda #(n-310)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19946)
	; 14 bytes
	php
	brk 0
	lda #169
	jsr delay_a_25_clocks
	lda #(n-311)/255
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19947)
	; 14 bytes
	php
	brk 0
	lda #170
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19948)
	; 14 bytes
	php
	brk 0
	lda #171
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19949)
	; 14 bytes
	php
	brk 0
	lda #172
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19950)
	; 14 bytes
	php
	brk 0
	lda #173
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19951)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19952)
	; 15 bytes
	php
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19954)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #79
	jsr delay_a_25_clocks
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
	lda #57
	jsr delay_a_25_clocks
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
	lda #238
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19958)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #95
	jsr delay_a_25_clocks
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
	lda #70
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19960)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
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
	lda #86
	jsr delay_a_25_clocks
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
	lda #227
	jsr delay_a_25_clocks
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
	lda #245
	jsr delay_a_25_clocks
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
	lda #47
	jsr delay_a_25_clocks
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
	lda #162
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19966)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #77
	jsr delay_a_25_clocks
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
	lda #99
	jsr delay_a_25_clocks
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
	lda #165
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19969)
	; 16 bytes
	php
	tya
	pha
	ldy #210
	lda #63
	jsr delay_a_25_clocks
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
	lda #249
	jsr delay_a_25_clocks
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
	lda #169
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19973)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19974)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19975)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #124
	jsr delay_a_25_clocks
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
	lda #53
	jsr delay_a_25_clocks
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
	lda #82
	jsr delay_a_25_clocks
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
	lda #95
	jsr delay_a_25_clocks
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
	lda #181
	jsr delay_a_25_clocks
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
	lda #131
	jsr delay_a_25_clocks
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
	lda #102
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19986 || n = 19990 || n = 19992 || n = 19995 || n = 19997 || n = 19998)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 19987)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19988)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19989)
	; 17 bytes
	php
	txa
	pha
	sec
	lda #52
	ldx #202 ;hides 'dex'
	Jmi *-1
	sbc #1
	Jne *-6
	pla
	tax
	plp
.elseif(n = 19991 || n = 19996)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19993)
	; 7 bytes
	php
	lda #78
	jsr delay_256a_16_clocks
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
	jsr delay_256a_16_clocks
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
