; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber Z+N, and D
; Will preserve A, X, Y, S, C, V, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 5
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 18
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 31
; - 7 bytes or less when n <= 35
; - 8 bytes or less when n <= 44
; - 9 bytes or less when n <= 297
; -10 bytes or less when n <= 310
; -11 bytes or less when n <= 1312
; -12 bytes or less when n <= 2355
; -13 bytes or less when n <= 3154
; -14 bytes or less when n <= 4865
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.6, max=4
; - When 11 <= n <= 20: min=2, avg=3.6, max=5
; - When 21 <= n <= 30: min=4, avg=5.3, max=6
; - When 31 <= n <= 50: min=6, avg=7.7, max=9
; - When 51 <= n <= 75: min=8, avg=8.8, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=10
; - When 301 <= n <= 500: min=8, avg=10.0, max=11
; - When 501 <= n <= 750: min=8, avg=10.1, max=11
; - When 751 <= n <= 1000: min=9, avg=10.1, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.6, max=12
; - When 2001 <= n <= 3000: min=10, avg=11.6, max=13
; - When 3001 <= n <= 5000: min=10, avg=12.5, max=15
; - When 5001 <= n <= 7500: min=11, avg=14.1, max=15
; - When 7501 <= n <= 10000: min=12, avg=14.8, max=15
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
	sta @zptemp
.elseif(n = 4 || n = 6 || n = 16 || n = 17 || n = 28 || n = 32)
	; 2..7 bytes
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
.elseif(n = 9 || n = 11 || (n > 18 && n < 24) || (n > 32 && n < 38))
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
	brk 0
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 31)
	; 4..6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 24 || n = 25 || n = 38)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 39)
	; 4..6 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 27)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 29 || n = 30)
	; 6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.else
; 40 to 78
.if(n = 40)
	; 7 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 41 || n = 43)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 42)
	; 8 bytes
	delay_n (n-3)
	sta @zptemp
.elseif(n = 44)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 45)
	; 9 bytes
	php
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	plp
.elseif(n = 46)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 47 || n = 49 || n = 51 || n = 54 || (n > 55 && n < 60) || (n > 60 && n < 65) || (n > 65 && n < 69) || (n > 69 && n < 79))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
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
.elseif(n = 52)
	; 8 bytes
	.repeat 4
	 brk 0
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
	delay_n (n-13)
	brk 0
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
.elseif(n = 330 || n = 360 || n = 375)
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
.elseif(n = 335 || n = 340 || n = 345 || n = 380)
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
.elseif(n = 365)
	; 10 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 370)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	iny
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
.if((n >= 391) && (n < 430))
; 391 to 429
.if(n = 391 || n = 394 || n = 396 || n = 399 || n = 414 || n = 419 || n = 421 || n = 424 || n = 426 || n = 429)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 392 || n = 407 || n = 417 || n = 422)
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
.elseif(n = 397 || n = 402 || n = 412 || n = 427)
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
	delay_n (n-13)
	brk 0
.elseif(n = 400 || n = 405)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 401)
	; 10 bytes
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jpl *-3
	ldy @zptemp
.elseif(n = 404)
	; 9 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 406)
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
.elseif(n = 410)
	; 10 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	dey
	dey
	Jmi *-3
	ldy @zptemp
.elseif(n = 411)
	; 9 bytes
	sty @zptemp
	ldy #(n-5)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 413 || n = 420)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 415)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	dey
	Jmi *-3
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
.elseif(n = 418)
	; 9 bytes
	sty @zptemp
	ldy #(n-12)/7 ;hides 'nop'
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 423)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 425)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
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
.endif
.else
; 430 to 468
.if(n = 430 || n = 440 || n = 445 || n = 450)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 431 || n = 434 || n = 436 || n = 443 || n = 444 || n = 449 || n = 451 || n = 454 || n = 456 || n = 459 || n = 464)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 432 || n = 462 || n = 467)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
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
.elseif(n = 435 || n = 465)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 5 = 2) && ((n > 436 && n < 458)))
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 438)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 439 || n = 446 || n = 453)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 441 || n = 448)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 455)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 460)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 461)
	; 10 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
	ldy @zptemp
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
.endif
.else
.if((n >= 469) && (n < 547))
.if((n >= 469) && (n < 508))
; 469 to 507
.if(n = 469 || n = 471 || n = 479 || n = 484 || n = 489 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 470 || n = 480 || n = 505)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif(n = 472 || n = 482 || n = 497 || n = 507)
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
.elseif(n = 474 || n = 481)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 475 || n = 485)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 476)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 477 || n = 487 || n = 492 || n = 502)
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
.elseif(n = 483 || n = 490)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 486)
	; 10 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	nop
	iny
	Jpl *-3
	ldy @zptemp
.elseif(n = 488)
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
.elseif(n = 495)
	; 9 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	iny
	Jpl *-2
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
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.endif
.else
; 508 to 546
.if(n = 508)
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
.elseif(n = 510)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 511)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 512 || n = 517 || n = 522 || n = 532 || n = 537 || n = 542)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 513 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 539)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 514)
	; 9 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
	pla
.elseif(n = 515 || n = 520 || n = 535 || n = 540 || n = 545)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif(n = 516 || n = 544)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 518 || n = 525 || n = 546)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 521)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 523 || n = 530)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 527)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
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
.elseif(n = 536)
	; 10 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	nop
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 538)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 548 || n = 549 || n = 554 || n = 556 || n = 559 || n = 564 || n = 569 || n = 574 || n = 584 || n = 589 || n = 591 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 619 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 550)
	; 10 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	nop
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 555)
	; 10 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 563)
	; 10 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop
	dey
	Jpl *-3
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
.elseif(n = 566)
	; 10 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	dey
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 570 || n = 575 || n = 610 || n = 615 || n = 620)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif(n = 571)
	; 10 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 573 || n = 578 || n = 598 || n = 603)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
.elseif(n = 608)
	; 10 bytes
	sty @zptemp
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	ldy @zptemp
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
.if((n >= 626) && (n < 665))
; 626 to 664
.if(n = 626 || n = 629 || n = 631 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 661 || n = 664)
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
.elseif(n = 628)
	; 9 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	ldy @zptemp
.elseif(n = 630 || n = 651)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 633)
	; 9 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
	pla
.elseif(n = 635)
	; 9 bytes
	sty @zptemp
	ldy #(n-5)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 642 || n = 662)
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
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 645)
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
.elseif(n = 649 || n = 663)
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
.elseif(n = 656)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
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
.endif
.else
; 665 to 703
.if(n = 665 || n = 686 || n = 700)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 666 || n = 669 || n = 671 || n = 674 || n = 676 || n = 679 || n = 681 || n = 688 || n = 689 || n = 694 || n = 699)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 2) && ((n > 666 && n < 703)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
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
.elseif(n = 670 || n = 691)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
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
.elseif(n = 675)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
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
.elseif(n = 680)
	; 10 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jpl *-3
	ldy @zptemp
.elseif(n = 683)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 684 || n = 698)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
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
.endif
.else
; 704 to 781
.if(n = 704 || n = 706 || n = 709 || n = 711 || n = 714 || n = 716 || n = 729 || n = 731 || n = 734 || n = 736 || n = 739 || n = 744 || n = 746 || n = 749 || n = 751 || n = 758 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781)
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
	delay_n (n-13)
	brk 0
.elseif((n .mod 5 = 0) && ((n > 709 && n < 731) || (n > 749 && n < 766) || n = 780))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
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
.elseif(n = 719)
	; 9 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	ldy @zptemp
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
.elseif(n = 724)
	; 9 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	tay
	pla
.elseif(n = 726 || n = 740 || n = 761 || n = 768 || n = 775)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
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
.elseif(n = 745)
	; 11 bytes
	php
	delay_n (n-7)
	plp
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
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
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
.elseif(n = 783 || n = 818)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 784 || n = 793 || n = 794 || n = 799 || n = 801 || n = 804 || n = 809 || n = 814 || n = 819 || n = 821 || n = 829 || n = 834 || n = 836 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 784 && n < 801) || (n > 824 && n < 836) || n = 850 || n = 855))
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
	ldy #96
	sta @zptemp
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 789 || n = 796 || n = 831 || n = 845)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 791 || n = 826 || n = 840)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 798 || n = 805 || n = 833)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
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
.elseif(n = 806)
	; 10 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	ldy @zptemp
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
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 815)
	; 10 bytes
	sty @zptemp
	ldy #(n-5)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 848)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 853)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 859)
	; 9 bytes
	sty @zptemp
	ldy #(n-5)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
.endif
.else
; 860 to 937
.if((n .mod 5 = 0) && ((n > 859 && n < 871) || (n > 884 && n < 901) || (n > 919 && n < 936)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif(n = 861 || n = 868 || n = 875 || n = 896 || n = 903)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif((n .mod 5 = 2) && ((n > 861 && n < 938)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 863 || n = 869 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 906 || n = 909 || n = 911 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 933 || n = 934)
	; 10..11 bytes
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
.elseif(n = 866)
	; 9 bytes
	sty @zptemp
	ldy #(n-12)/7 ;hides 'nop'
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 871)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 873 || n = 880 || n = 901 || n = 908 || n = 915)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
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
.elseif(n = 878 || n = 918)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 883)
	; 11 bytes
	sty @zptemp
	ldy #(n-19)/12
	delay_n 7
	dey
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 893 || n = 913)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
.elseif(n = 898)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
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
.elseif(n = 923)
	; 10 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 931)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.endif
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
.elseif(n = 939 || n = 944 || n = 949 || n = 951 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 989 || n = 994 || n = 1003 || n = 1004 || n = 1009 || n = 1011 || n = 1014)
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
.elseif(n = 943)
	; 9 bytes
	sty @zptemp
	ldy #122 ;hides 'nop'
	iny
	Jne *-2
	ldy @zptemp
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
.elseif(n = 950 || n = 971 || n = 978 || n = 985 || n = 999 || n = 1006 || n = 1013)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
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
.elseif(n = 986)
	; 10 bytes
	sty @zptemp
	ldy #218 ;hides 'nop'
	dey
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 988)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jne *-1
	sta (@ptrtemp,x)
	ldx @zptemp
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
	sta (@ptrtemp,x)
	tax
	pla
.elseif(n = 1033 || n = 1038)
	; 10 bytes
	delay_n (n-13)
	brk 0
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
.elseif((n .mod 5 = 0) && (n = 1035 || (n > 1059 && n < 1081)))
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
.elseif(n = 1040)
	; 10 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	iny
	iny
	Jne *-3
	ldy @zptemp
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
.if(n = 1094 || n = 1096 || n = 1099 || n = 1109 || n = 1114 || n = 1119 || n = 1124 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1143 || n = 1144 || n = 1149 || n = 1151 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1178 || n = 1179 || n = 1184 || n = 1186 || n = 1189 || n = 1191 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1221 || n = 1224 || n = 1226 || n = 1229 || n = 1231 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1116) || (n > 1139 && n < 1151) || (n > 1164 && n < 1186) || (n > 1199 && n < 1221) || (n > 1234 && n < 1251)))
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
	; 10 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
	ldy @zptemp
.elseif(n = 1103)
	; 10 bytes
	sty @zptemp
	ldy #(n-5)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
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
.elseif(n = 1121)
	; 10 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1123 || n = 1168)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 1126)
	; 10 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 1130)
	; 10 bytes
	sty @zptemp
	ldy #250 ;hides 'nop'
	dey
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1133 || n = 1163 || n = 1203 || n = 1208 || n = 1233)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 1198)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	dey
	Jne *-4
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
.if((n >= 1251) && (n < 1290))
; 1251 to 1289
.if(n = 1251 || n = 1258 || n = 1265 || n = 1279 || n = 1286)
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
.elseif(n = 1253 || n = 1260 || n = 1281 || n = 1288)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1254 || n = 1259 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1274 || n = 1276 || n = 1283 || n = 1284 || n = 1289)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1255 || n = 1275 || n = 1280 || n = 1285)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif(n = 1256)
	; 10 bytes
	sty @zptemp
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1263)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1268)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
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
.endif
.else
; 1290 to 1328
.if(n = 1290)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif(n = 1291 || n = 1294 || n = 1296 || n = 1304 || n = 1311 || n = 1313 || n = 1318 || n = 1325)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1292)
	; 9 bytes
	pha
	tya
	ldy #0
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 7 = 5) && ((n > 1292 && n < 1329)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif((n .mod 7 = 0) && ((n > 1294 && n < 1324)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
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
.elseif(n = 1301)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $70A0'
	ldy #112
	iny
	Jne *-4
	ldy @zptemp
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
	nop #<$0C ;hides 'nop $70A0'
	ldy #112
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 1310)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $91A0'
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
.elseif(n = 1324)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.endif
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $6DA0'
	ldy #109
	iny
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
.elseif(n = 1331)
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
	nop #<$0C ;hides 'nop $93A0'
	ldy #147
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1334)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $6DA0'
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
.elseif(n = 1336)
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	ldy @zptemp
.elseif(n = 1338)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $6CA0'
	ldy #108
	iny
	Jne *-4
	ldy @zptemp
.elseif((n .mod 9 = 7) && ((n > 1338 && n < 1367)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1340 || n = 1364)
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
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	tay
	pla
.elseif(n = 1343 || n = 1359)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1345)
	; 11 bytes
	sty @zptemp
	ldy #122 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1346 || n = 1360 || n = 1367)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1347)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
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
	nop #<$0C ;hides 'nop $96A0'
	ldy #150
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1361)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $6AA0'
	ldy #106
	iny
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
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1370 || n = 1377 || n = 1384 || n = 1398)
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
	brk 0
	Jmi *-3
	ldx @zptemp
.elseif(n = 1374)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
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
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk 0
	Jmi *-3
	tax
	pla
.elseif(n = 1381 || n = 1388 || n = 1395)
	; 11 bytes
	delay_n (n-2)
	nop
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
	nop $04
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1391)
	; 9 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	ldy @zptemp
.elseif(n = 1394)
	; 11 bytes
	delay_n (n-3)
	nop $04
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
	sty @zptemp
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	sta (@ptrtemp),y
	ldy @zptemp
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
	php
	pha
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
	pla
	plp
.elseif(n = 1403)
	; 11 bytes
	php
	delay_n (n-7)
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
	nop <$0C ;hides 'nop $9BA0'
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
.elseif((n .mod 7 = 2) && (n = 1409 || (n > 1422 && n < 1445)))
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
.elseif(n = 1411 || n = 1420 || n = 1429)
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
	nop #<$0C ;hides 'nop $63A0'
	ldy #99
	iny
	Jne *-4
	ldy @zptemp
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
	nop <$0C ;hides 'nop $9DA0'
	ldy #157
	dey
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
	nop #<$0C ;hides 'nop $62A0'
	ldy #98
	iny
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
.elseif(n = 1436)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1439)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1441)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $9FA0'
	ldy #159
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1445)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $60A0'
	ldy #96
	iny
	Jne *-4
	ldy @zptemp
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
	tya
	nop #<$0C ;hides 'nop $60A0'
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
	nop $04
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
	sta @zptemp
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
	nop <$0C ;hides 'nop $A2A0'
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
	nop <$0C ;hides 'nop $A3A0'
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
	nop <$0C ;hides 'nop $A3A0'
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
	nop #<$0C ;hides 'nop $A4A0'
	ldy #164
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if(n = 1485)
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
.elseif(n = 1486 || n = 1493 || n = 1509 || n = 1514 || n = 1521)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1487 || n = 1511)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
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
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1511)))
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
	nop #<$0C ;hides 'nop $A5A0'
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
.elseif(n = 1492 || n = 1501)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
	; 10 bytes
	sty @zptemp
	ldy #90 ;hides 'nop'
	nop
	iny
	Jne *-3
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
.elseif(n = 1502)
	; 10 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1513)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $A7A0'
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
.elseif(n = 1516)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1517)
	; 9 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1518)
	; 10 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1520)
	; 11 bytes
	stx @zptemp
	ldx $A6
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	ldx @zptemp
.elseif(n = 1522)
	; 9 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
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
.if((n .mod 7 = 5) && (n = 1524 || (n > 1537 && n < 1560)))
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
	pha
	lda $A5
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
	pla
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
	delay_n (n-7)
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
.elseif(n = 1531)
	; 9 bytes
	sty @zptemp
	ldy #(n-5)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1532)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1534 || n = 1539)
	; 11 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1535 || n = 1556)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1536)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
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
	; 10 bytes
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
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
	ldy #116 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1553)
	; 10 bytes
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
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
	; 10 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1562)
	; 10 bytes
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1564 || n = 1591 || n = 1600)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1565)
	; 11 bytes
	sty @zptemp
	ldy #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	ldy @zptemp
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
	sty @zptemp
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1579)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1581)
	; 11 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1583 || n = 1599)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1585)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1586)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1588)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1590)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $B0A0'
	ldy #176
	dey
	Jne *-4
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
	nop <$0C ;hides 'nop $B0A0'
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
.elseif(n = 1604 || n = 1628)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
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
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1608 || n = 1622 || n = 1636)
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
.elseif((n .mod 7 = 2) && ((n > 1611 && n < 1641)))
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1613)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $B2A0'
	ldy #178
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1615)
	; 9 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
	ldy @zptemp
.elseif(n = 1616)
	; 10 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	ldx @zptemp
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
	nop #<$0C ;hides 'nop $B4A0'
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
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	sta (@ptrtemp),y
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
	ldy #(n-5)/7 ;hides 'nop'
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
.elseif((n .mod 7 = 0) && ((n > 1644 && n < 1674)))
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
.elseif(n = 1647 || n = 1661 || n = 1675)
	; 11 bytes
	delay_n (n-2)
	nop
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
	lda #(n-15)/7 ;hides 'nop'
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
	nop <$0C ;hides 'nop $B7A0'
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
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1663 || n = 1672)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1665)
	; 11 bytes
	sty @zptemp
	ldy #90 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1667)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $B8A0'
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
	pha
	tya
	ldy #90 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
	pla
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
	nop $04
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
.elseif(n = 1683)
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
	sta @zptemp
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
	nop #<$0C ;hides 'nop $BCA0'
	ldy #188
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1698)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $BCA0'
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
	nop <$0C ;hides 'nop $BDA0'
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1720 || n = 1727 || n = 1734 || n = 1748)
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
	nop <$0C ;hides 'nop $BEA0'
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
.elseif(n = 1724 || n = 1738 || n = 1745)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1725)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $BFA0'
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
	nop <$0C ;hides 'nop $BFA0'
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
.elseif(n = 1732 || n = 1756)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1733)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $C0A0'
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
.elseif(n = 1739)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1741)
	; 9 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	ldy @zptemp
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
	; 9 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
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
	delay_n (n-7)
	plp
.elseif(n = 1754)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1755)
	; 9 bytes
	sty @zptemp
	ldy #(n-5)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
.endif
.else
; 1758 to 1796
.if(n = 1758 || n = 1767 || n = 1794)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1759 || n = 1766 || n = 1773)
	; 11 bytes
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
	; 11 bytes
	sty @zptemp
	ldy #250 ;hides 'nop'
	dey
	Jne *-2
	sta (@ptrtemp),y
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
	; 11 bytes
	delay_n (n-3)
	nop $04
.elseif((n .mod 7 = 0) && ((n > 1763 && n < 1793)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1765)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1768)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1770)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $3CA0'
	ldy #60
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1772)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1774)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $3CA0'
	ldy #60
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1775 || n = 1791)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1777)
	; 12 bytes
	sty @zptemp
	ldy #252
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 1779)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $3BA0'
	ldy #59
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1780 || n = 1789)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1781)
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
	nop <$0C ;hides 'nop $3BA0'
	ldy #59
	iny
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
	; 10 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	nop
	iny
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1793)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $C6A0'
	ldy #198
	dey
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
	nop <$0C ;hides 'nop $C8A0'
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
	nop #<$0C ;hides 'nop $37A0'
	ldy #55
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1815 || n = 1823)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
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
	sta @zptemp
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
	nop <$0C ;hides 'nop $CAA0'
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
	nop <$0C ;hides 'nop $35A0'
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
.if((n >= 1836) && (n < 1856))
; 1836 to 1855
.if(n = 1836)
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
	nop <$0C ;hides 'nop $35A0'
	ldy #53
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1839 || n = 1848)
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
	; 10 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1842)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1843 || n = 1852)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1844)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
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
	sty @zptemp
	ldy #(n-7)/8
	nop $04
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
	; 10 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1851)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $CDA0'
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
.elseif(n = 1854)
	; 12 bytes
	delay_n (n-2)
	nop
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
.endif
.else
; 1856 to 1875
.if(n = 1856)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $CDA0'
	ldy #205
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1857 || n = 1866 || n = 1875)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1858)
	; 12 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1859)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $32A0'
	ldy #50
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1860 || n = 1872)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1861 || n = 1870)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1862)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1863 || n = 1871)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1864)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $32A0'
	ldy #50
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1865)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $CEA0'
	ldy #206
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1867)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1868)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1869)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $31A0'
	ldy #49
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1873)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $31A0'
	ldy #49
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1874)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $31A0'
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
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1877)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $30A0'
	ldy #48
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1878)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $30A0'
	ldy #48
	iny
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
	nop #<$0C ;hides 'nop $AAA0'
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
	nop <$0C ;hides 'nop $30A0'
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
.elseif(n = 1885)
	; 11 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1887 || n = 1895)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1889 || n = 1894 || n = 1898)
	; 12 bytes
	delay_n (n-2)
	nop
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
	ldy #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1904)
	; 10 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1905)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $2DA0'
	ldy #45
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1907)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
	nop <$0C ;hides 'nop $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1913)
	; 10 bytes
	sty @zptemp
	ldy #(n-5)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1914)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $D4A0'
	ldy #212
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
.if((n >= 1915) && (n < 1934))
; 1915 to 1933
.if(n = 1915 || n = 1924 || n = 1933)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1916)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1917)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1918)
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1919)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1920 || n = 1929)
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1922)
	; 10 bytes
	sty @zptemp
	ldy #(n-14)/9 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
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
.elseif(n = 1927)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/9 ;hides 'nop zp,x'
	iny ;first loop only
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
.elseif(n = 1931)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $D6A0'
	ldy #214
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1932)
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
; 1934 to 1953
.if(n = 1934)
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
.elseif(n = 1935 || n = 1943)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1936)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $D6A0'
	ldy #214
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1937)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1938 || n = 1947)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1939)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1940 || n = 1948)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1941)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $D7A0'
	ldy #215
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1942 || n = 1951)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1944)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1945)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $29A0'
	ldy #41
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1946)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $D7A0'
	ldy #215
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1949)
	; 10 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1950)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $D8A0'
	ldy #216
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1952)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1953)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if(n = 1954)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1955)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $D8A0'
	ldy #216
	dey
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
	pha
	lda $A5
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1958)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $27A0'
	ldy #39
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1959 || n = 1991)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
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
	delay_n (n-7)
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
.elseif(n = 1964 || n = 1988)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1966 || n = 1971 || n = 1984 || n = 1989)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1967)
	; 10 bytes
	sty @zptemp
	ldy #(n-5)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1973)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1976)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $DBA0'
	ldy #219
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1977)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $DBA0'
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
	nop #<$0C ;hides 'nop $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1985)
	; 11 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1986)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $24A0'
	ldy #36
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1990)
	; 11 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
	pla
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 2006 || n = 2024)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1994)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $23A0'
	ldy #35
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1995)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $DDA0'
	ldy #221
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 9 = 7) && ((n > 1995 && n < 2024)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1997)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	nop $04
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1998)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
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
	nop <$0C ;hides 'nop $DDA0'
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
	nop #<$0C ;hides 'nop $DEA0'
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
.elseif(n = 2007 || n = 2015 || n = 2031)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	nop $04
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2008)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $DEA0'
	ldy #222
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2009)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $22A0'
	ldy #34
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2012 || n = 2020)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2013)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $DFA0'
	ldy #223
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2017)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $DFA0'
	ldy #223
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2018)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $DFA0'
	ldy #223
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2021)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $20A0'
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
	nop #<$0C ;hides 'nop $20A0'
	ldy #32
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2027)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $20A0'
	ldy #32
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 2030)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $1FA0'
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
	sta @zptemp
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
	nop $04
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
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2048)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $1DA0'
	ldy #29
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2049)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E3A0'
	ldy #227
	dey
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
	tya
	nop #<$0C ;hides 'nop $1DA0'
	ldy #29
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2054)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $E3A0'
	ldy #227
	dey
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
	stx @zptemp
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2058)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E4A0'
	ldy #228
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2063)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $E4A0'
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
	nop #<$0C ;hides 'nop $E5A0'
	ldy #229
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2067)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $1BA0'
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
	; 10 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	nop
	iny
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
	nop <$0C ;hides 'nop $E6A0'
	ldy #230
	dey
	Jne *-4
	tay
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
	nop <$0C ;hides 'nop $19A2'
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
	nop <$0C ;hides 'nop $19A2'
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
	nop <$0C ;hides 'nop $18A0'
	ldy #24
	iny
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
	nop <$0C ;hides 'nop $18A0'
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
.elseif(n = 2103)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E9A0'
	ldy #233
	dey
	Jne *-4
	ldy @zptemp
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
	tya
	nop <$0C ;hides 'nop $E9A0'
	ldy #233
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
.if((n >= 2110) && (n < 2129))
; 2110 to 2128
.if(n = 2110 || n = 2114 || n = 2115)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2111)
	; 10 bytes
	sty @zptemp
	ldy #(n-5)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2112 || n = 2124)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2113 || n = 2122)
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
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2118 || n = 2127)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2120)
	; 10 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2121)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $EBA0'
	ldy #235
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2123)
	; 12 bytes
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
	nop <$0C ;hides 'nop $EBA0'
	ldy #235
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2128)
	; 12 bytes
	delay_n (n-3)
	nop $04
.endif
.else
; 2129 to 2148
.if(n = 2129)
	; 10 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2130)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $ECA0'
	ldy #236
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2131 || n = 2140)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 2133)
	; 12 bytes
	delay_n (n-2)
	nop
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
	nop <$0C ;hides 'nop $ECA0'
	ldy #236
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2136 || n = 2145)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2137)
	; 12 bytes
	pha
	tya
	ldy #124
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2138)
	; 10 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2139)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $13A0'
	ldy #19
	iny
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
	nop <$0C ;hides 'nop $13A0'
	ldy #19
	iny
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
	nop #<$0C ;hides 'nop $12A0'
	ldy #18
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2148)
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
	brk 0
	dey
	Jmi *-4
	tay
	pla
.elseif(n = 2151 || n = 2164 || n = 2169 || n = 2178 || n = 2186 || n = 2187)
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
	nop <$0C ;hides 'nop $EEA0'
	ldy #238
	dey
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
	nop #<$0C ;hides 'nop $11A0'
	ldy #17
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2157)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $11A0'
	ldy #17
	iny
	Jne *-4
	ldy @zptemp
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
	nop #<$0C ;hides 'nop $11A0'
	ldy #17
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2165)
	; 11 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop $04
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2166)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $10A0'
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
	ldy #216 ;hides 'cld'
	sta @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2171)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $10A0'
	ldy #16
	iny
	Jne *-4
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
.elseif(n = 2174)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $0FA0'
	ldy #15
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2175)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $F1A0'
	ldy #241
	dey
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
	nop <$0C ;hides 'nop $F1A0'
	ldy #241
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2183)
	; 11 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
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
	tya
	ldy #58 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2189)
	; 11 bytes
	pha
	txa
	nop <$0C ;hides 'nop $0EA2'
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
.elseif(n = 2192)
	; 10 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2193)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $F3A0'
	ldy #243
	dey
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
	delay_n (n-3)
	nop $04
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
	nop <$0C ;hides 'nop $F3A0'
	ldy #243
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2200)
	; 12 bytes
	sty @zptemp
	ldy #128
	jsr @nearby_rts
	dey
	Jpl *-4
	ldy @zptemp
.elseif(n = 2201)
	; 10 bytes
	sty @zptemp
	ldy #(n-5)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2202)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 2207 to 2226
.if(n = 2207)
	; 11 bytes
	pha
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
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
.elseif(n = 2209)
	; 11 bytes
	pha
	tya
	ldy #12 ;hides 'nop abs'
	inc @zptemp
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2210)
	; 10 bytes
	sty @zptemp
	ldy #(n-14)/9 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2211)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $0BA0'
	ldy #11
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2212 || n = 2221)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2213)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2214)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2215)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/9 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2216)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2218)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2219)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $F6A0'
	ldy #246
	dey
	Jne *-4
	ldy @zptemp
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
	tya
	nop #<$0C ;hides 'nop $F6A0'
	ldy #246
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2225)
	; 11 bytes
	pha
	txa
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	inx
	Jne *-4
	tax
	pla
.endif
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2251 || n = 2263)
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
	nop #<$0C ;hides 'nop $09A0'
	ldy #9
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2229)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $09A0'
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
.elseif(n = 2231 || n = 2236 || n = 2241 || n = 2250 || n = 2258 || n = 2259)
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
	nop #<$0C ;hides 'nop $09A0'
	ldy #9
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2234)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $09A0'
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
	; 10 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
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
	; 10 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
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
	nop #<$0C ;hides 'nop $07A0'
	ldy #7
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2247)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $07A0'
	ldy #7
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2249)
	; 11 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2252)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 2255)
	; 10 bytes
	sty @zptemp
	ldy #(n-5)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
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
	nop <$0C ;hides 'nop $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2264)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $05A0'
	ldy #5
	iny
	Jne *-4
	ldy @zptemp
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
	nop #<$0C ;hides 'nop $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2270)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $05A0'
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
	nop #<$0C ;hides 'nop $04A0'
	ldy #4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2274)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $04A0'
	ldy #4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2276 || n = 2277 || n = 2281 || n = 2286)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2278)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2279)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2283)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $03A0'
	ldy #3
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2285)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2287 || n = 2299)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2288)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2290)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2291)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $02A0'
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
	nop #<$0C ;hides 'nop $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2297)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2300)
	; 11 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $01A0'
	ldy #1
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2301)
	; 11 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $01A0'
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
	nop #<$0C ;hides 'nop $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2306)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $01A0'
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
	nop #<$0C ;hides 'nop $00A0'
	ldy #0
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2313 || n = 2318)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2314)
	; 11 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $00A0'
	ldy #0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2315)
	; 11 bytes
	pha
	tya
	nop <$0C ;hides 'nop $00A0'
	ldy #0
	iny
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
.elseif(n = 2317)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
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
	nop #<$0C ;hides 'nop $D2A0'
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
	nop <$0C ;hides 'nop $2EA0'
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
.if(n = 2324 || n = 2338 || n = 2343)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2325)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop $04
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2326)
	; 11 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2327)
	; 12 bytes
	sty @zptemp
	ldy #(n-27)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
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
	ldy #90 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
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
	; 11 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2337)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/11 ;hides 'nop zp,x'
	delay_n 4
	dey
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
	ldy #(n-5)/10 ;hides 'nop'
	nop $04
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2346)
	; 12 bytes
	sty @zptemp
	ldy #100 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2347 || n = 2359 || n = 2371)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2348)
	; 11 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2349 || n = 2354 || n = 2356 || n = 2374 || n = 2379)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-21)/11 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2357)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	nop $04
	dey
	Jne *-4
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
.elseif(n = 2360)
	; 12 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $2AA0'
	ldy #42
	nop
	iny
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
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2367)
	; 12 bytes
	sty @zptemp
	ldy #(n-27)/20
	jsr @nearby_rts_15cyc
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
	brk 0
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
	; 11 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
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
.elseif(n = 2384 || n = 2385 || n = 2390 || n = 2397 || n = 2404 || n = 2409 || n = 2411 || n = 2414)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2386)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
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
	pha
	lda $A5
	tya
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
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
	nop $04
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
	sta @zptemp
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
.elseif(n = 2401)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2403)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2406)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	nop $04
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2418)
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 2423 || n = 2426 || n = 2428 || n = 2433 || n = 2438 || n = 2440)
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
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jmi *-4
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
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jmi *-4
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
	brk 0
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
.elseif(n = 2445)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	ldy @zptemp
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
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	sta (@ptrtemp,x)
	ldx @zptemp
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
	brk 0
	dey
	Jne *-4
	tay
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
	nop <$0C ;hides 'nop $DFA0'
	ldy #223
	nop
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
.if((n >= 2461) && (n < 2481))
; 2461 to 2480
.if(n = 2461)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 2462)
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
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2464 || n = 2469 || n = 2474)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2465)
	; 11 bytes
	sty @zptemp
	ldy #122 ;hides 'nop'
	brk 0
	dey
	Jpl *-4
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
.elseif(n = 2467 || n = 2479)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2468)
	; 12 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2470)
	; 11 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	brk 0
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2471)
	; 10 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	brk 0
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2475)
	; 12 bytes
	pha
	tya
	nop <$0C ;hides 'nop $20A0'
	ldy #32
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2476)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	brk 0
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2480)
	; 12 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $1FA0'
	ldy #31
	nop
	iny
	Jne *-5
	ldy @zptemp
.endif
.else
; 2481 to 2500
.if(n = 2481 || n = 2486 || n = 2488 || n = 2494)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2482 || n = 2492)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2483)
	; 12 bytes
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
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop $04
	dey
	Jne *-4
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2490)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	sta @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2491)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2493)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
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
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2499)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2505)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/10 ;hides 'nop'
	nop $04
	dey
	Jne *-4
	ldy @zptemp
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
	ldy #(n-27)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
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
.elseif(n = 2509)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
	brk 0
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
.elseif(n = 2517 || n = 2524 || n = 2529 || n = 2533 || n = 2534)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2519)
	; 12 bytes
	pha
	tya
	nop <$0C ;hides 'nop $E4A0'
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
	ldy #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2525)
	; 12 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $1BA0'
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
	nop <$0C ;hides 'nop $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2531)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2535)
	; 11 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2536)
	; 12 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E6A0'
	ldy #230
	nop
	dey
	Jne *-5
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 2547 || n = 2567)
	; 12 bytes
	sty @zptemp
	ldy #(n-27)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	ldy @zptemp
.elseif(n = 2548 || n = 2559)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2549)
	; 12 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	nop
	nop $04
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2550)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2554 || n = 2558 || n = 2565 || n = 2570 || n = 2577)
	; 12 bytes
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
.elseif(n = 2566)
	; 12 bytes
	pha
	tya
	ldy #(n-22)/12 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	dey
	Jne *-5
	tay
	pla
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
	nop #<$0C ;hides 'nop $E9A0'
	ldy #233
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2576)
	; 12 bytes
	sty @zptemp
	ldy #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dey
	Jpl *-5
	ldy @zptemp
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
	ldy #(n-5)/11 ;hides 'nop'
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
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
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
	nop <$0C ;hides 'nop $16A2'
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
	brk 0
	dey
	Jne *-3
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
	; 11 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2591)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2593)
	; 12 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jpl *-5
	tay
	pla
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
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
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
	; 11 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2601)
	; 11 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2602)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	inc @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	ldy @zptemp
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
.if((n >= 2618) && (n < 2637))
; 2618 to 2636
.if(n = 2618 || n = 2620)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2619 || n = 2630)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2621)
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/12 ;hides 'nop'
	nop
	nop $04
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2622)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2623 || n = 2635)
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
	nop <$0C ;hides 'nop $EEA0'
	ldy #238
	nop
	dey
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
	; 11 bytes
	pha
	tya
	ldy #(n-10)/12 ;hides 'nop'
	inc @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2627)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2628)
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
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
; 2637 to 2656
.if(n = 2637 || n = 2642 || n = 2643 || n = 2654)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2640 || n = 2652)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2641)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
	nop #<$0C ;hides 'nop $10A0'
	ldy #16
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2646)
	; 12 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $10A0'
	ldy #16
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2647)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
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
	delay_n 8
	Jne *-5
	tax
	pla
.elseif(n = 2650)
	; 12 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $10A0'
	ldy #16
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2651)
	; 12 bytes
	pha
	tya
	nop <$0C ;hides 'nop $10A0'
	ldy #16
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2653)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2655)
	; 12 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2656)
	; 12 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	ldy @zptemp
.endif
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
.if((n >= 2657) && (n < 2676))
; 2657 to 2675
.if(n = 2657)
	; 12 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	delay_n 6
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2658)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2659 || n = 2671)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2661 || n = 2665 || n = 2666 || n = 2673)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2662)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 6
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2663 || n = 2674)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2664)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2667)
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
	nop <$0C ;hides 'nop $0EA2'
	ldx #14
	nop
	inx
	Jne *-5
	ldx @zptemp
.elseif(n = 2669)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2670)
	; 12 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2672)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.endif
.else
; 2676 to 2695
.if(n = 2676 || n = 2688)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 2678)
	; 11 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2679)
	; 12 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $F3A0'
	ldy #243
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2680 || n = 2691)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2681)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 2683 || n = 2695)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2684)
	; 12 bytes
	pha
	tya
	nop <$0C ;hides 'nop $F3A0'
	ldy #243
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2685)
	; 11 bytes
	sty @zptemp
	ldy #122 ;hides 'nop'
	brk 0
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2686)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 2689)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/11 ;hides 'nop zp,x'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2690)
	; 11 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	brk 0
	iny
	Jne *-4
	tay
	pla
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
.endif
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
.elseif(n = 2697 || n = 2699 || n = 2702 || n = 2704 || n = 2720 || n = 2721 || n = 2726)
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
	nop <$0C ;hides 'nop $0BA0'
	ldy #11
	nop
	iny
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
	; 11 bytes
	pha
	tya
	ldy #(n-21)/11 ;hides 'nop zp,x'
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
	nop <$0C ;hides 'nop $0BA0'
	ldy #11
	nop
	iny
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
.elseif(n = 2708)
	; 13 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2709 || n = 2723)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2710 || n = 2727)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2711)
	; 12 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $F6A0'
	ldy #246
	nop
	dey
	Jne *-5
	ldy @zptemp
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
	tya
	nop #<$0C ;hides 'nop $F6A0'
	ldy #246
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2717)
	; 12 bytes
	pha
	txa
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2722)
	; 12 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $09A0'
	ldy #9
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2725)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2730)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2733)
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2734)
	; 12 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $08A0'
	ldy #8
	nop
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 2738)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2739)
	; 12 bytes
	pha
	tya
	nop <$0C ;hides 'nop $08A0'
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
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2742 || n = 2756)
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
.elseif(n = 2749)
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
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2758 || n = 2763 || n = 2769)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2759)
	; 12 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2761)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2766)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2770)
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	inc @zptemp
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2771)
	; 12 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
	pla
.endif
.else
; 2774 to 2812
.if((n > 2773 && n < 2777) || n = 2781 || n = 2786 || n = 2809)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2777)
	; 11 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
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
.elseif(n = 2788)
	; 12 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $03A0'
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
	nop $04
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2797)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
	nop #<$0C ;hides 'nop $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2802)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2804)
	; 12 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $02A0'
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
	nop <$0C ;hides 'nop $01A0'
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
	ldy #216 ;hides 'cld'
	delay_n 6
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2814 || n = 2822 || n = 2829 || n = 2834 || n = 2841 || n = 2846 || n = 2848)
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
	nop <$0C ;hides 'nop $01A0'
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
	ldy #(n-10)/12 ;hides 'nop'
	inc @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2819)
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
	; 12 bytes
	sty @zptemp
	ldy #0
	delay_n 6
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
.elseif(n = 2825)
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	iny
	Jne *-5
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2836)
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2837)
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop
	nop $04
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2838)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 2850)
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
.elseif(n = 2853 || n = 2858 || n = 2860 || n = 2864 || n = 2865 || n = 2870 || n = 2871 || n = 2889)
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
.elseif(n = 2866)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
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
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
.elseif(n = 2878 || n = 2890)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/12
	delay_n 7
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 2879)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #11
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 2888)
	; 13 bytes
	delay_n (n-13)
	brk 0
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891 || n = 2919)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2893)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2894 || n = 2901 || n = 2903 || n = 2906 || n = 2908 || n = 2913 || n = 2915 || n = 2918 || n = 2920 || n = 2925)
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
.elseif(n = 2897 || n = 2914)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2899 || n = 2911 || n = 2923)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2900)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2902)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2905)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2907 || n = 2927)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2909)
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
.elseif(n = 2910)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2917)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2921)
	; 12 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop $04
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
.elseif(n = 2929)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.endif
.else
; 2930 to 2968
.if(n = 2930 || n = 2937 || n = 2939 || (n > 2941 && n < 2945) || n = 2949 || n = 2951 || n = 2954 || n = 2956)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2931 || n = 2948 || n = 2965)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2933 || n = 2961)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2935 || n = 2947 || n = 2959)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2941)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2945)
	; 12 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2946)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2950)
	; 12 bytes
	pha
	tya
	ldy #(n-22)/12 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	dey
	Jne *-5
	tay
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
.elseif(n = 2957)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2958)
	; 12 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
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
.elseif(n = 2962)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.if(n = 2969 || n = 2993)
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
.elseif(n = 2974 || n = 2998)
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
	brk 0
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
.elseif(n = 2981)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	nop $04
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2982)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2986)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	inc @zptemp
	dey
	Jne *-4
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
	ldy #(n-5)/12 ;hides 'nop'
	nop
	nop $04
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3006)
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
.elseif(n = 3009 || n = 3011 || n = 3014 || n = 3015 || n = 3020 || n = 3026 || n = 3033 || n = 3035 || n = 3038 || n = 3040)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3010)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/12 ;hides 'nop'
	inc @zptemp
	dey
	Jne *-4
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3018)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3019 || n = 3031 || n = 3043)
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
	delay_n 6
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3023)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3025)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
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
.elseif(n = 3029)
	; 11 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 7
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
.elseif(n = 3034)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3037)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #216 ;hides 'cld'
	delay_n 7
	dey
	Jne *-4
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
.elseif(n = 3041)
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
	delay_n (n-13)
	brk 0
.elseif(n = 3046)
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
.elseif(n = 3049 || n = 3085)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 3051 || n = 3056 || n = 3061 || n = 3069 || n = 3074 || n = 3081 || n = 3083)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	brk 0
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
.elseif(n = 3057)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3058 || n = 3070)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/12
	delay_n 7
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 3059 || n = 3073)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3063)
	; 12 bytes
	sty @zptemp
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	brk 0
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3066)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3068)
	; 12 bytes
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	brk 0
	iny
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
.elseif(n = 3079)
	; 11 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3082)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
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
.if(n = 3086 || n = 3088 || n = 3094 || n = 3105 || n = 3110 || n = 3122 || n = 3125)
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
	brk 0
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
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	ldy @zptemp
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
	nop #<$0C ;hides 'nop $C1A0'
	ldy #193
	delay_n 7
	dey
	Jne *-6
	tay
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
.elseif(n = 3103 || n = 3121)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
	nop <$0C ;hides 'nop $11A0'
	ldy #17
	delay_n 4
	iny
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
	brk 0
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
.elseif(n = 3130 || n = 3131 || n = 3137 || n = 3141 || n = 3146 || n = 3149 || n = 3150 || n = 3154 || n = 3163)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3132)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3133)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $DFA0'
	ldy #223
	inc @zptemp
	dey
	Jne *-6
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
.elseif(n = 3135 || n = 3152)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3136)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #12
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 3138)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $0FA0'
	ldy #15
	delay_n 4
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3139 || n = 3157)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 3151)
	; 13 bytes
	stx @zptemp
	nop #<$0C ;hides 'nop $0EA2'
	ldx #14
	delay_n 4
	inx
	Jne *-6
	ldx @zptemp
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
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
	ldy @zptemp
.elseif(n = 3159)
	; 12 bytes
	sty @zptemp
	ldy #90 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3160)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $1FA0'
	ldy #31
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3164)
	; 12 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 3165 to 3203
.if(n = 3165)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $F3A0'
	ldy #243
	delay_n 4
	dey
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
.elseif(n = 3168 || n = 3188 || n = 3200 || n = 3201)
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
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3173)
	; 12 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	delay_n 9
	iny
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
.elseif(n = 3175 || n = 3193)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 3177)
	; 12 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	delay_n 6
	dey
	Jne *-5
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3180 || n = 3198)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3181)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
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
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3206 || n = 3209 || n = 3214 || n = 3215)
	; 13 bytes
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 3213)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3216)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
.elseif(n = 3220)
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
	nop #<$0C ;hides 'nop $09A0'
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
.endif
.else
; 3223 to 3242
.if(n = 3223)
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
.elseif(n = 3225)
	; 11 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 3226)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $37A0'
	ldy #55
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3227 || n = 3241)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3231 || n = 3236)
	; 12 bytes
	delay_n (n-2)
	nop
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3235)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 3237)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3238)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $CAA0'
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
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3242)
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
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3248 || n = 3249 || n = 3251 || n = 3273 || n = 3278)
	; 12..13 bytes
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
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 8
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3247 || n = 3265)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3250)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 8
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3252 || n = 3270)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
.elseif(n = 3257)
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3262)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
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
	; 12 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3266)
	; 12 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	iny ;first loop only
	brk 0
	iny
	Jne *-5
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
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 3277)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3281)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/14 ;hides 'nop'
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
; 3282 to 3320
.if(n = 3282)
	; 12 bytes
	sty @zptemp
	ldy #100 ;hides 'nop zp'
	nop
	brk 0
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3283 || n = 3301 || n = 3319)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 3285)
	; 12 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
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
	brk 0
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
.elseif(n = 3295 || n = 3303 || n = 3304 || n = 3308 || n = 3315 || n = 3320)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3296 || n = 3309)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3297 || n = 3311)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
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
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	inc @zptemp
	iny
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
.elseif(n = 3305)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3314)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3317)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $31A0'
	ldy #49
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3326 || n = 3329 || n = 3341 || n = 3346 || n = 3349 || n = 3354 || n = 3357 || n = 3359)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3322 || n = 3356)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3323)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	nop $04
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3324 || n = 3342)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3325)
	; 11 bytes
	sty @zptemp
	ldy #90 ;hides 'nop'
	brk 0
	iny
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3330)
	; 11 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	brk 0
	iny
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
	nop #<$0C ;hides 'nop $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	ldy @zptemp
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
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3336)
	; 13 bytes
	pha
	txa
	ldx #90 ;hides 'nop'
	brk 0
	inx
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 3337 || n = 3355)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3338)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3339 || n = 3353)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3343)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $EEA0'
	ldy #238
	inc @zptemp
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
	brk 0
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
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3350)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $2FA0'
	ldy #47
	delay_n 7
	iny
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
	brk 0
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
	nop #<$0C ;hides 'nop $D2A0'
	ldy #210
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3366)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $2EA0'
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
	delay_n (n-13)
	brk 0
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
	brk 0
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
	brk 0
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
	brk 0
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 3379)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 3382)
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
	nop #<$0C ;hides 'nop $0FA0'
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
	nop <$0C ;hides 'nop $F1A0'
	ldy #241
	inc @zptemp
	dey
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
.elseif(n = 3397)
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
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
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inc @zptemp
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3402)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3403)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $D4A0'
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
.elseif(n = 3405)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	rol $26
	ror $26
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3406 || n = 3411 || n = 3416 || n = 3425 || n = 3434)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	brk 0
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
	brk 0
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
	ldy #(n-26)/16 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3419)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $D5A0'
	ldy #213
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3421)
	; 13 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	dey
	Jne *-6
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	inc @zptemp
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3430)
	; 12 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 7
	brk 0
	Jne *-5
	ldy @zptemp
.elseif(n = 3433)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 3435)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	brk 0
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
	; 13 bytes
	sty @zptemp
	ldy #100 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3439 || n = 3443 || n = 3444 || n = 3448 || n = 3449 || n = 3453 || n = 3454 || n = 3460 || n = 3469 || n = 3474)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3440)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	inc @zptemp
	dey
	Jne *-5
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
.elseif(n = 3445 || n = 3463)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 3447 || n = 3467)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3450 || n = 3468)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 3455)
	; 13 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 8
	iny
	Jne *-6
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3471)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3473)
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
.elseif(n = 3476)
	; 13 bytes
	delay_n (n-13)
	brk 0
.endif
.else
; 3477 to 3515
.if(n = 3477)
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3478)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $27A0'
	ldy #39
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3479 || n = 3493 || n = 3507)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3480 || n = 3497 || n = 3514)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3481 || n = 3499)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3482)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3483 || n = 3488 || n = 3494 || n = 3501 || n = 3506)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3484 || n = 3498 || n = 3512)
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
	pha
	lda $A5
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3486 || n = 3504)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	delay_n 8
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3491)
	; 12 bytes
	sty @zptemp
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3492 || n = 3509)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3495 || n = 3511)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3496)
	; 12 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
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
.elseif(n = 3505)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
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
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 8
	dey
	Jne *-6
	ldy @zptemp
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
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3518)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3519 || n = 3523 || n = 3524 || n = 3528 || n = 3529 || n = 3533 || n = 3534 || n = 3537 || n = 3542 || n = 3545 || n = 3550 || n = 3551)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3525)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $05A0'
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
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-6
	ldy @zptemp
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
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 26
	dey
	Jmi *-6
	ldy @zptemp
.elseif(n = 3538)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $04A0'
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
	nop <$0C ;hides 'nop $04A0'
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
.elseif(n = 3544)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3547)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 3555 to 3593
.if(n = 3555 || n = 3556 || n = 3569 || n = 3570 || n = 3573 || n = 3574 || n = 3578 || n = 3581 || n = 3586 || n = 3593)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3557)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $DEA0'
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
	brk 0
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
	nop #<$0C ;hides 'nop $02A0'
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
	brk 0
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
.elseif(n = 3579)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
	ldy #100 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3590)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $20A0'
	ldy #32
	delay_n 7
	iny
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3595)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $00A0'
	ldy #0
	inc @zptemp
	iny
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
	delay_n (n-13)
	brk 0
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
	nop <$0C ;hides 'nop $E1A0'
	ldy #225
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3607)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
	nop #<$0C ;hides 'nop $1EA0'
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
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3626)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $1EA0'
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3631)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 3634 || n = 3635 || n = 3638 || n = 3649)
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
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
	brk 0
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
	brk 0
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
	brk 0
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
	brk 0
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
	stx @zptemp
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	delay_n 7
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3654 || n = 3663 || n = 3668)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	delay_n (n-13)
	brk 0
.elseif(n = 3657)
	; 12 bytes
	sty @zptemp
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3658)
	; 13 bytes
	pha
	txa
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3659)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	brk 0
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
	brk 0
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
.elseif(n = 3664)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk 0
	Jne *-3
	tax
	pla
.elseif(n = 3665)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	brk 0
	Jne *-3
	sta (@ptrtemp,x)
	ldx @zptemp
.elseif(n = 3666)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	brk 0
	Jne *-3
	sta (@ptrtemp,x)
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
	nop <$0C ;hides 'nop $1BA0'
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
	delay_n (n-13)
	brk 0
.elseif(n = 3678)
	; 13 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3679 || n = 3697)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3680)
	; 13 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3681 || n = 3683 || n = 3686 || n = 3688 || n = 3689 || n = 3694 || n = 3695 || n = 3699 || n = 3700 || n = 3704)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3682)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3684 || n = 3702)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3685)
	; 12 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
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
	nop <$0C ;hides 'nop $E6A0'
	ldy #230
	delay_n 7
	dey
	Jne *-6
	tay
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
	brk 0
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
	; 13 bytes
	sty @zptemp
	ldy #218 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	ldy @zptemp
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
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3721)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
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
	nop <$0C ;hides 'nop $18A0'
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
.elseif(n = 3725)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 8
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3726)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
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
	delay_n (n-13)
	brk 0
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
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3734 || n = 3737 || n = 3740 || n = 3748)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3738)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3739)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $E9A0'
	ldy #233
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3741)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
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
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3744)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3745)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3746)
	; 13 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts
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
	brk 0
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
.elseif(n = 3753 || n = 3758 || n = 3766)
	; 12..13 bytes
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
	brk 0
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
	brk 0
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
	brk 0
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
.elseif(n = 3764)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3765)
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
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
	sty @zptemp
	nop <$0C ;hides 'nop $ABA0'
	ldy #171
	brk 0
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3771 || n = 3773 || n = 3789)
	; 12..13 bytes
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3775)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 3776)
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3778)
	; 12 bytes
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3779)
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
.elseif(n = 3780)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
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
	nop <$0C ;hides 'nop $ECA0'
	ldy #236
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3783)
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3784)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3785)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inc @zptemp
	iny
	Jne *-6
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
	brk 0
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
; 3790 to 3828
.if(n = 3790 || n = 3794 || n = 3795 || n = 3800 || n = 3801 || n = 3806 || n = 3809 || n = 3814 || n = 3824 || n = 3825)
	; 12..14 bytes
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
.elseif(n = 3792 || n = 3810 || n = 3828)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3805 || n = 3823)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 3811)
	; 12 bytes
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	brk 0
	iny
	Jne *-5
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	brk 0
	iny
	Jne *-5
	tay
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
.elseif(n = 3819)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $EEA0'
	ldy #238
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3821)
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/18 ;hides 'nop zp,x'
	delay_n 11
	dey
	Jne *-6
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
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3830 || n = 3833 || n = 3834 || n = 3839 || n = 3843 || n = 3848 || n = 3851 || n = 3856 || n = 3858 || n = 3861)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3831 || n = 3850)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3832 || n = 3849 || n = 3866)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3835 || n = 3857)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3837 || n = 3854)
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
	nop #<$0C ;hides 'nop $AEA0'
	ldy #174
	brk 0
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
.elseif(n = 3841 || n = 3859)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3845)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3846 || n = 3864)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
.elseif(n = 3860)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $AFA0'
	ldy #175
	brk 0
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
.elseif(n = 3873 || n = 3879 || (n > 3883 && n < 3887) || n = 3889 || n = 3890 || n = 3894 || n = 3896 || n = 3897)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3874)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3875)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3876)
	; 12 bytes
	sty @zptemp
	ldy #194 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3877 || n = 3895)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3878)
	; 13 bytes
	stx @zptemp
	nop <$0C ;hides 'nop $0EA2'
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
	brk 0
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 3881)
	; 12 bytes
	pha
	tya
	ldy #(n-1)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3882 || n = 3900)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	; 12 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
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
	nop <$0C ;hides 'nop $F3A0'
	ldy #243
	delay_n 7
	dey
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3904)
	; 12 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
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
	delay_n (n-13)
	brk 0
.elseif(n = 3909)
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3910)
	; 13 bytes
	sty @zptemp
	ldy #12 ;hides 'nop abs'
	nop $0
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
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3914)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3915 || n = 3919 || n = 3920 || n = 3923 || n = 3924 || n = 3928 || n = 3929 || n = 3933 || n = 3935 || n = 3938 || n = 3941 || n = 3942)
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
	brk 0
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
	; 12 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3927)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3930)
	; 12 bytes
	pha
	tya
	ldy #(n-26)/16 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3937)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3943)
	; 12 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	brk 0
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
.if((n >= 3946) && (n < 3965))
; 3946 to 3964
.if(n = 3946)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $F6A0'
	ldy #246
	delay_n 7
	dey
	Jne *-6
	tay
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
	brk 0
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 3949)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 3951)
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
.elseif(n = 3953 || n = 3958 || n = 3961)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3954)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3955)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
	sty @zptemp
	nop #<$0C ;hides 'nop $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3959)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3960)
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
	nop #<$0C ;hides 'nop $09A0'
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
	nop <$0C ;hides 'nop $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3964)
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
; 3965 to 3984
.if(n = 3965)
	; 11 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	brk 0
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 3966 || n = 3969 || n = 3971 || n = 3974)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3967)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3968)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3970)
	; 11 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	brk 0
	iny
	Jne *-4
	tay
	pla
.elseif(n = 3972)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3973)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3975)
	; 12 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	brk 0
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3976)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3977)
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
.elseif(n = 3978)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3979)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $08A0'
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
	brk 0
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3981)
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
.elseif(n = 3982)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3983)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3984)
	; 13 bytes
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.endif
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
	brk 0
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
	ldy #(n-10)/17 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3989)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3990)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	brk 0
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 3995)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $07A0'
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
	delay_n (n-13)
	brk 0
.elseif(n = 3999 || n = 4001)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4000)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	ldx @zptemp
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
	brk 0
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 4006)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $06A0'
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4010)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	brk 0
	iny
	Jne *-4
	tay
	pla
.elseif(n = 4011)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
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
	brk 0
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4022)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $05A0'
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
.elseif(n = 4025 || (n > 4027 && n < 4031) || n = 4034 || n = 4037 || n = 4038 || n = 4042 || n = 4043 || n = 4046 || n = 4048 || n = 4049 || n = 4056 || n = 4061)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4026 || n = 4044 || n = 4062)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	brk 0
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
.elseif(n = 4039 || n = 4057)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 4045)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4050)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
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
; 4063 to 4101
.if(n = 4063)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4064 || n = 4066 || n = 4069 || n = 4074 || n = 4077 || n = 4079 || n = 4082 || n = 4084 || n = 4089 || n = 4094 || n = 4095 || n = 4099 || n = 4100)
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
.elseif(n = 4075 || n = 4093)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 4080 || n = 4098)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
.elseif(n = 4083)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4085)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4088)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4090)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $01A0'
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
	nop <$0C ;hides 'nop $01A0'
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
.elseif(n = 4096)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk 0
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 4101)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 4105)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4106 || n = 4110 || n = 4113 || n = 4115 || n = 4118 || n = 4123 || n = 4128 || n = 4131 || n = 4136 || n = 4137 || n = 4140)
	; 12..14 bytes
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
.elseif(n = 4111 || n = 4129)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 4114)
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4116 || n = 4134)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4117)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4119)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4120)
	; 13 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
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
	delay_n (n-13)
	brk 0
.elseif(n = 4125)
	; 13 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
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
.elseif(n = 4133)
	; 13 bytes
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
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
	brk 0
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
.elseif(n = 4142)
	; 13 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4145 || n = 4151 || n = 4156 || n = 4157 || n = 4162 || n = 4174 || n = 4175 || n = 4179)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4146)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $BCA0'
	ldy #188
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4147 || n = 4165)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 11
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4152 || n = 4170)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4153 || n = 4171)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 4158)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
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
	; 12 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 4164)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #16
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 4169)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 4176)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4178)
	; 13 bytes
	delay_n (n-13)
	brk 0
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4183)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4188)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
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
	brk 0
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 4199 to 4218
.if(n = 4199 || n = 4216)
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
.elseif(n = 4200)
	; 14 bytes
	pha
	txa
	ldx #246
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4201)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4202)
	; 14 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4203 || n = 4208 || n = 4213 || n = 4215)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4204)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk 0
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
	brk 0
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
.elseif(n = 4217)
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/18 ;hides 'nop'
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
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219 || n = 4237 || n = 4255 || n = 4273 || n = 4291)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4220)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
	iny
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4221 || n = 4226 || n = 4229 || n = 4234 || n = 4238 || n = 4239 || n = 4241 || n = 4246 || n = 4251 || n = 4254 || n = 4259 || n = 4264 || n = 4265 || n = 4269 || n = 4270 || (n > 4274 && n < 4278) || n = 4280 || n = 4281 || n = 4284 || n = 4289 || (n > 4292 && n < 4296))
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
	brk 0
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
.elseif(n = 4233)
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
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4248)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4250 || n = 4286)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4256)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $C1A0'
	ldy #193
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4266)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4271)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4283)
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
.elseif(n = 4285)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4288)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 4298 || n = 4300 || n = 4303 || n = 4310 || n = 4311 || n = 4315 || n = 4316 || n = 4318 || n = 4322 || n = 4326 || n = 4329 || n = 4331 || n = 4334)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk 0
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
	delay_n (n-13)
	brk 0
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
.elseif(n = 4308)
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
	brk 0
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
.elseif(n = 4313)
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
	brk 0
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
	nop <$0C ;hides 'nop $3CA0'
	ldy #60
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4325)
	; 11 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	brk 0
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4328)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #216 ;hides 'cld'
	brk 0
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4330)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	brk 0
	dey
	Jne *-4
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
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.endif
.else
.if((n >= 4336) && (n < 4356))
; 4336 to 4355
.if(n = 4336 || n = 4346 || n = 4349 || n = 4354)
	; 13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4339)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4340 || n = 4343)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4341)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4342)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 4345)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
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
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk 0
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 4350)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4351)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4352)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4353)
	; 13 bytes
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
.endif
.else
; 4356 to 4375
.if(n = 4356)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
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
.elseif(n = 4358)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4359)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4360 || n = 4369 || n = 4374)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4361)
	; 12 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 4362)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4363)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 4365)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4366)
	; 12 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
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
.elseif(n = 4368)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4370)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4371)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4373)
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
.elseif(n = 4375)
	; 12 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
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
; 4376 to 4414
.if(n = 4376)
	; 13 bytes
	delay_n (n-13)
	brk 0
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
.elseif(n = 4379 || n = 4383 || n = 4384 || n = 4388 || n = 4389 || n = 4394 || n = 4398 || n = 4400 || n = 4403 || n = 4406 || n = 4408 || n = 4409 || n = 4414)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4381 || n = 4399)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
	brk 0
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
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
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
.elseif(n = 4393 || n = 4411)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4395)
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
.elseif(n = 4397)
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/18 ;hides 'nop zp,x'
	delay_n 11
	dey
	Jne *-6
	ldy @zptemp
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
	stx @zptemp
	ldx $A6
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
	ldx @zptemp
.elseif(n = 4417 || n = 4435 || n = 4453)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
	php
	pha
	stx @zptemp
	lax #17
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 4422 || n = 4440)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4423)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4425)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk 0
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
	brk 0
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
	delay_n (n-13)
	brk 0
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
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $C9A0'
	ldy #201
	brk 0
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
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4446)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4450)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $CAA0'
	ldy #202
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4451)
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454 || n = 4455 || n = 4460 || n = 4473 || n = 4475 || n = 4478 || n = 4479 || n = 4486)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4458 || n = 4476)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4462)
	; 13 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
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
	brk 0
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
.elseif(n = 4465)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4466 || n = 4484)
	; 13 bytes
	delay_n (n-13)
	brk 0
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
	ldy #194 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4469)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 11
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4470 || n = 4490)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4471 || n = 4489)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
.elseif(n = 4474)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 11
	dey
	Jne *-6
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
	; 12 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 4483)
	; 13 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4488)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $07A0'
	ldy #7
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
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
.if((n >= 4493) && (n < 4512))
; 4493 to 4511
.if(n = 4493)
	; 12 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop
	brk 0
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 4494)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
.elseif(n = 4496)
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
	brk 0
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
	brk 0
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4499)
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
.elseif(n = 4500)
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
.elseif(n = 4503)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4504)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $D6A0'
	ldy #214
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4505)
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/18 ;hides 'nop'
	delay_n 11
	dey
	Jne *-6
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
.elseif(n = 4507)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4508 || n = 4509 || n = 4511)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4510)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 4512 to 4531
.if(n = 4512 || n = 4530)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4513)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4514 || n = 4517 || n = 4523 || n = 4531)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4515)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4516)
	; 12 bytes
	sty @zptemp
	ldy #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4518)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4519)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4520)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	brk 0
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4521)
	; 12 bytes
	pha
	tya
	ldy #(n-1)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4522)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
	brk 0
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 4529)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
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
	brk 0
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
	brk 0
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
	ldx #(n-18)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4541)
	; 12 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4542)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $32A0'
	ldy #50
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4543 || n = 4561)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
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
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4556)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4557)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
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
	ldy #58 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4560)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $31A0'
	ldy #49
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4562)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
	brk 0
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
.elseif(n = 4574 || n = 4576 || n = 4581 || n = 4588 || n = 4590 || n = 4593 || n = 4595 || (n > 4598 && n < 4602) || n = 4606 || n = 4608)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4575)
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
.elseif(n = 4577)
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
.elseif(n = 4578)
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
.elseif(n = 4579 || n = 4597)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4580)
	; 14 bytes
	sty @zptemp
	ldy #n/20-1
	nop
	brk 0
	dey
	Jne *-4
	ldy @zptemp
	brk 0
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
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4584 || n = 4602)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	sty @zptemp
	ldy #(n-13)/18
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4604)
	; 10 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4605)
	; 11 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	brk 0
	iny
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
; 4610 to 4648
.if(n = 4610)
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	brk 0
	iny
	Jne *-4
	tay
	pla
.elseif(n = 4611 || n = 4613 || (n > 4616 && n < 4620) || n = 4622 || n = 4626 || n = 4631 || n = 4634)
	; 11..13 bytes
	delay_n (n-2)
	nop
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
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4616 || n = 4636)
	; 12..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4620)
	; 11 bytes
	pha
	tya
	ldy #0
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4621)
	; 13 bytes
	sty @zptemp
	ldy #0
	brk 0
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4623)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #0
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4624 || n = 4643)
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
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4627 || n = 4647)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4628 || n = 4633)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4629 || n = 4648)
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
	nop #<$0C ;hides 'nop $D2A0'
	ldy #210
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4635)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4637)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4641)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4642)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $C1A0'
	ldy #193
	jsr @nearby_rts_15cyc
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
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	brk 0
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 4646)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	ldx @zptemp
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
	brk 0
	iny
	Jne *-4
	tay
	pla
.elseif(n = 4651)
	; 13 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	brk 0
	iny
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
	brk 0
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 4656)
	; 13 bytes
	pha
	txa
	ldx #24 ;hides 'clc'
	brk 0
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
.elseif(n = 4658)
	; 12 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4660 || n = 4664 || n = 4665)
	; 13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4663)
	; 12 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4669)
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/22 ;hides 'nop zp,x'
	nop
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4670)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $D4A0'
	ldy #212
	brk 0
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4673 || n = 4680)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4674)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/22 ;hides 'nop zp,x'
	nop
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4675)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $D4A0'
	ldy #212
	brk 0
	dey
	Jne *-6
	tay
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
.elseif(n = 4678)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #18
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
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
	ldy #(n-5)/20 ;hides 'nop'
	brk 0
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
.if((n >= 4688) && (n < 4707))
; 4688 to 4706
.if(n = 4688 || n = 4689 || n = 4694 || n = 4702)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4690)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4691)
	; 12 bytes
	sty @zptemp
	ldy #(n-27)/22 ;hides 'nop zp,x'
	iny ;first loop only
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4692)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4693)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4695)
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
	; 12 bytes
	pha
	tya
	ldy #(n-32)/22 ;hides 'nop zp,x'
	iny ;first loop only
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4697)
	; 13 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4698)
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
.elseif(n = 4699)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4700)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 4703)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4704)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4705)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4706)
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
; 4707 to 4726
.if(n = 4707)
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
.elseif(n = 4709 || n = 4714 || n = 4721 || n = 4726)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4710)
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
.elseif(n = 4711)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4712)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4713)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $D6A0'
	ldy #214
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4715)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4716)
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
.elseif(n = 4717)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4718)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $D6A0'
	ldy #214
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4719)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4720)
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
.elseif(n = 4722)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4723)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4724)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4725)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.endif
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
.elseif(n = 4728)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4729 || n = 4734 || n = 4740 || n = 4744 || n = 4745 || n = 4749 || n = 4751 || n = 4754 || n = 4756 || n = 4759 || n = 4764)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4730)
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
.elseif(n = 4731 || n = 4739)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4735)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $29A0'
	ldy #41
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4736)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $D7A0'
	ldy #215
	brk 0
	dey
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
.elseif(n = 4741)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $D7A0'
	ldy #215
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4742)
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
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4763)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $D8A0'
	ldy #216
	brk 0
	dey
	Jne *-6
	tay
	pla
.endif
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
.elseif(n = 4768)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4769 || n = 4771 || n = 4774 || n = 4778 || n = 4783 || n = 4789 || n = 4794 || n = 4797 || n = 4802 || n = 4804)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
	brk 0
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
.elseif(n = 4780)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $27A0'
	ldy #39
	brk 0
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4784)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $27A0'
	ldy #39
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4785)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $27A0'
	ldy #39
	brk 0
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
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4798)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4799)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4801)
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
	; 12 bytes
	pha
	tya
	ldy #(n-10)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
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
	sty @zptemp
	ldy #(n-13)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
	nop #<$0C ;hides 'nop $DBA0'
	ldy #219
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 4824 to 4843
.if(n = 4824)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $DBA0'
	ldy #219
	brk 0
	dey
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
	nop #<$0C ;hides 'nop $DBA0'
	ldy #219
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4829 || n = 4834)
	; 13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4835)
	; 12 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4840)
	; 12 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4841)
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
.if((n .mod 5 = 4) && ((n > 4843 && n < 4860)))
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4845)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $24A0'
	ldy #36
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4846)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $24A0'
	ldy #36
	brk 0
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
	nop #<$0C ;hides 'nop $24A0'
	ldy #36
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4851)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $24A0'
	ldy #36
	brk 0
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
.elseif(n = 4853)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
	brk 0
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
	brk 0
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
	nop <$0C ;hides 'nop $DDA0'
	ldy #221
	brk 0
	dey
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
.elseif(n = 4873 || n = 4878 || n = 4880)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
; 4883 to 4921
.if(n = 4883 || n = 4899)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4886)
	; 13 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4887 || n = 4907)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4888 || n = 4911)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4889 || n = 4893 || n = 4894 || n = 4898 || n = 4902 || n = 4909 || n = 4914 || n = 4920)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4890)
	; 13 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4895)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	iny
	Jne *-6
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
.elseif(n = 4900 || n = 4921)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4901)
	; 13 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
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
	; 13 bytes
	sty @zptemp
	ldy #(n-28)/23 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4906)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
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
	nop #<$0C ;hides 'nop $DFA0'
	ldy #223
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4917)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $DFA0'
	ldy #223
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4919)
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
	ldy #(n-10)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4925)
	; 13 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
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
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
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
	nop #<$0C ;hides 'nop $20A0'
	ldy #32
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4935)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #19
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4938)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $20A0'
	ldy #32
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4939)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $20A0'
	ldy #32
	brk 0
	iny
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
	nop <$0C ;hides 'nop $EBA0'
	ldy #235
	jsr @nearby_rts
	dey
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
	nop #<$0C ;hides 'nop $1FA0'
	ldy #31
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4956)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E1A0'
	ldy #225
	brk 0
	dey
	Jne *-6
	ldy @zptemp
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
	ldy #58 ;hides 'nop'
	inc @zptemp
	brk 0
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 4961) && (n < 4981))
; 4961 to 4980
.if(n = 4961)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4962)
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
.elseif(n = 4964 || n = 4969 || n = 4974)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4965)
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4968)
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
.elseif(n = 4970)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4971)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4972)
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
	ldy #216 ;hides 'cld'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4975)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	brk 0
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4976)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4977)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4978)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	jsr @nearby_rts_14cyc
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 4981 to 5000
.if(n = 4981 || n = 4986 || n = 4991 || n = 4994 || n = 4996)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4982)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4983)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4984)
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
.elseif(n = 4987)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4988)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $13A0'
	ldy #19
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4989)
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
.elseif(n = 4990)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4993)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
	sty @zptemp
	nop #<$0C ;hides 'nop $1DA0'
	ldy #29
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5000)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E3A0'
	ldy #227
	brk 0
	dey
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
	tya
	nop #<$0C ;hides 'nop $1DA0'
	ldy #29
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5005)
	; 11 bytes
	sty @zptemp
	ldy #(n-5)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5006)
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
.elseif(n = 5007)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5008 || n = 5013)
	; 13 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5009 || n = 5014)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5010)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5011)
	; 13 bytes
	sty @zptemp
	ldy #250 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 5012)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5015)
	; 13 bytes
	php
	pha
	sec
	lda #250 ;hides 'nop'
	brk 0
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5016)
	; 13 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5017)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5018)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5019)
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 5020 to 5039
.if(n = 5020)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $DAA0'
	ldy #218
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5021)
	; 13 bytes
	stx @zptemp
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	brk 0
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 5022)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E4A0'
	ldy #228
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5023)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5024)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5025)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5026)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5027)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5028)
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
.elseif(n = 5029 || n = 5033 || n = 5034 || n = 5036)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5030)
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
.elseif(n = 5031)
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
.elseif(n = 5032)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5035)
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
.elseif(n = 5037)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5038)
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
.elseif(n = 5039)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
.if((n >= 5040) && (n < 5059))
; 5040 to 5058
.if(n = 5040)
	; 14 bytes
	sty @zptemp
	ldy #n/20-1
	nop
	brk 0
	dey
	Jne *-4
	ldy @zptemp
	brk 0
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
	nop #<$0C ;hides 'nop $DBA0'
	ldy #219
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5043)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $E5A0'
	ldy #229
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5044)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $1BA0'
	ldy #27
	brk 0
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
	nop <$0C ;hides 'nop $10A0'
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
	nop #<$0C ;hides 'nop $E5A0'
	ldy #229
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5049 || n = 5051 || n = 5054 || n = 5056)
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
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 5055)
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
.elseif(n = 5057)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5058)
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
.else
; 5059 to 5078
.if(n = 5059)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5060)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
	; 12 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5066)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E6A0'
	ldy #230
	brk 0
	dey
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
	; 12 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5071)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $E6A0'
	ldy #230
	brk 0
	dey
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
	brk 0
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
	brk 0
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
	brk 0
	Jne *-5
	tax
	pla
.elseif(n = 5086)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 7
	brk 0
	Jne *-5
	sta (@ptrtemp,x)
	ldx @zptemp
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
	nop <$0C ;hides 'nop $19A2'
	ldx #25
	brk 0
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
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-5)/24 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
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
	brk 0
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
	; 13 bytes
	pha
	tya
	ldy #(n-9)/24 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk 0
	dey
	Jne *-6
	tay
	pla
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
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	nop
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
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
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5115)
	; 11 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
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
	nop $EA ;hides 'nop'
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
	stx @zptemp
	ldx $A6
	ldx #0
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx @zptemp
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
	lda $A5
	tya
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
	brk 0
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 5139)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5140)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5141)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
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
.elseif(n = 5143)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5144 || n = 5149)
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
.elseif(n = 5145)
	; 14 bytes
	pha
	tya
	ldy #0
	nop
	brk 0
	dey
	Jne *-4
	tay
	pla
	brk 0
.elseif(n = 5146)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $D6A0'
	ldy #214
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5147)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $2AA0'
	ldy #42
	jsr @nearby_rts_15cyc
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
.elseif(n = 5150)
	; 13 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5151)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $0BA0'
	ldy #11
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
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
	ldy #(n-5)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5154)
	; 13 bytes
	stx @zptemp
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	brk 0
	inx
	Jne *-6
	ldx @zptemp
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
	stx @zptemp
	ldx $A6
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	ldx @zptemp
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
	ldy #(n-10)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5159)
	; 13 bytes
	pha
	txa
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	brk 0
	inx
	Jne *-6
	tax
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
	brk 0
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
	brk 0
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
	nop <$0C ;hides 'nop $D7A0'
	ldy #215
	jsr @nearby_rts_15cyc
	dey
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
.elseif(n = 5168 || n = 5174)
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
	nop #<$0C ;hides 'nop $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5171)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $F6A0'
	ldy #246
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 5173)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5175)
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	brk 0
	iny
	Jne *-5
	ldy @zptemp
.endif
.else
; 5176 to 5195
.if(n = 5176)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $EBA0'
	ldy #235
	brk 0
	dey
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
.elseif(n = 5179 || n = 5188 || n = 5193)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5180)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	brk 0
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5181)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $EBA0'
	ldy #235
	brk 0
	dey
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
.elseif(n = 5183)
	; 14 bytes
	delay_n (n-3)
	nop $04
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
	nop #<$0C ;hides 'nop $1FA0'
	ldy #31
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5186)
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5187)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5189)
	; 13 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5190)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $D8A0'
	ldy #216
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5191)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5192)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #20
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 5194)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5195)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $D8A0'
	ldy #216
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 5196) && (n < 5215))
; 5196 to 5214
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
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop
	brk 0
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5198)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $ECA0'
	ldy #236
	brk 0
	dey
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
.elseif(n = 5200)
	; 14 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5201 || n = 5206)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5202)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	brk 0
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5203)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $ECA0'
	ldy #236
	brk 0
	dey
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5208)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $1EA0'
	ldy #30
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5209)
	; 14 bytes
	pha
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5210)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5213)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5214)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $08A0'
	ldy #8
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
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
	stx @zptemp
	ldx $A6
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	ldx @zptemp
.elseif(n = 5217 || n = 5222 || n = 5228)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5218)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5219)
	; 12 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	brk 0
	iny
	Jne *-5
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	brk 0
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5225)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $13A0'
	ldy #19
	brk 0
	iny
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
	nop <$0C ;hides 'nop $E3A0'
	ldy #227
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5229)
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5230)
	; 14 bytes
	pha
	txa
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	brk 0
	inx
	Jne *-5
	inc @zptemp,x
	tax
	pla
.elseif(n = 5231)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $1DA0'
	ldy #29
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5232)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5233)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5234)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $07A0'
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
	nop <$0C ;hides 'nop $07A0'
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
	sty @zptemp
	ldy #(n-5)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5240)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $07A0'
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
	pha
	tya
	ldy #(n-10)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
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
	nop <$0C ;hides 'nop $EEA0'
	ldy #238
	brk 0
	dey
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
	stx @zptemp
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	ldx @zptemp
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
	txa
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5255)
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5256)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
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
.elseif(n = 5258)
	; 14 bytes
	delay_n (n-3)
	nop $04
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
	; 12 bytes
	pha
	tya
	ldy #(n-10)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5261)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $06A0'
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
	nop #<$0C ;hides 'nop $11A0'
	ldy #17
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5264)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $11A0'
	ldy #17
	brk 0
	iny
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
	ldy #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5267 || n = 5271 || n = 5272)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5268)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $11A0'
	ldy #17
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5269)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $11A0'
	ldy #17
	brk 0
	iny
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
	nop <$0C ;hides 'nop $1BA0'
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
	pha
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5276)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $05A0'
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
	brk 0
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
	nop #<$0C ;hides 'nop $05A0'
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
	nop #<$0C ;hides 'nop $10A0'
	ldy #16
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5286)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $10A0'
	ldy #16
	brk 0
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
	nop #<$0C ;hides 'nop $10A0'
	ldy #16
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5291)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $10A0'
	ldy #16
	brk 0
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
	; 13 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5296)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E6A0'
	ldy #230
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5297)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5298)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $04A0'
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
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5302)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5303)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $04A0'
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
	sty @zptemp
	ldy #(n-5)/25 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5307)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
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
	pha
	tya
	ldy #(n-10)/25 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
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
	brk 0
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
	nop <$0C ;hides 'nop $DDA0'
	ldy #221
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5316)
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
.elseif(n = 5317)
	; 14 bytes
	sty @zptemp
	ldy #177
	jsr @nearby_rts
	brk 0
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
	nop <$0C ;hides 'nop $03A0'
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
	nop <$0C ;hides 'nop $03A0'
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
	nop #<$0C ;hides 'nop $0EA2'
	ldx #14
	brk 0
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 5330)
	; 13 bytes
	stx @zptemp
	nop <$0C ;hides 'nop $0EA2'
	ldx #14
	brk 0
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
	brk 0
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
	nop #<$0C ;hides 'nop $0EA2'
	ldx #14
	brk 0
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5335)
	; 13 bytes
	pha
	txa
	nop <$0C ;hides 'nop $0EA2'
	ldx #14
	brk 0
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
	brk 0
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
	nop <$0C ;hides 'nop $02A0'
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
	nop <$0C ;hides 'nop $18A0'
	ldy #24
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5348)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
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
.elseif(n = 5351)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop $04
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
	nop <$0C ;hides 'nop $F3A0'
	ldy #243
	brk 0
	dey
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
	nop $04
.elseif(n = 5355 || n = 5360 || n = 5364 || n = 5369)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5356)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	sta @zptemp
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5357)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
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
.elseif(n = 5361)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5362)
	; 12 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
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
.elseif(n = 5365)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5366)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5367)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
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
	nop $FF44 ;hides 'nop zp'
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5372)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	nop $04
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5373)
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/22 ;hides 'nop zp,x'
	nop
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5374)
	; 13 bytes
	sty @zptemp
	ldy #12 ;hides 'nop abs'
	nop $0
	brk 0
	iny
	Jne *-6
	ldy @zptemp
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
	nop $FF44 ;hides 'nop zp'
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
	sta @zptemp
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5378)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/22 ;hides 'nop zp,x'
	nop
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5379 || n = 5390)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5381)
	; 13 bytes
	pha
	tya
	ldy #12 ;hides 'nop abs'
	inc @zptemp
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5382)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
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
	nop #<$0C ;hides 'nop $00A0'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5387)
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/23 ;hides 'nop'
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
.if((n >= 5391) && (n < 5410))
; 5391 to 5409
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
	ldy #(n-10)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5393)
	; 14 bytes
	pha
	txa
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5394)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5395)
	; 12 bytes
	sty @zptemp
	ldy #(n-27)/22 ;hides 'nop zp,x'
	iny ;first loop only
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5396)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $0BA0'
	ldy #11
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5397)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5398)
	; 14 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5399 || n = 5403 || n = 5404 || n = 5408 || n = 5409)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5400)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/22 ;hides 'nop zp,x'
	iny ;first loop only
	brk 0
	dey
	Jne *-5
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
.elseif(n = 5402)
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
	brk 0
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 5410 to 5429
.if(n = 5410)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5411)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $E1A0'
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5413)
	; 14 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5414 || n = 5421 || n = 5426)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5415)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5416)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $EBA0'
	ldy #235
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5417)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5418)
	; 13 bytes
	stx @zptemp
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	brk 0
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 5419)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
	tya
	nop #<$0C ;hides 'nop $F6A0'
	ldy #246
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5423)
	; 13 bytes
	pha
	txa
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	brk 0
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5424)
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
.elseif(n = 5425)
	; 14 bytes
	pha
	tya
	ldy #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	delay_n 4
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5427)
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
.elseif(n = 5428)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5429)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $1EA0'
	ldy #30
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
.if((n >= 5430) && (n < 5449))
; 5430 to 5448
.if(n = 5430)
	; 14 bytes
	sty @zptemp
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
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
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
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
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5439)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $09A0'
	ldy #9
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5440)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
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
.elseif(n = 5442 || n = 5443 || n = 5448)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5444)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $09A0'
	ldy #9
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5445)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $09A0'
	ldy #9
	brk 0
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
.elseif(n = 5447)
	; 14 bytes
	sty @zptemp
	ldy #170
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 5449 to 5468
.if(n = 5449)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #21
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 5450)
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
.elseif(n = 5451 || n = 5465)
	; 14 bytes
	delay_n (n-2)
	nop
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
	sty @zptemp
	nop #<$0C ;hides 'nop $1DA0'
	ldy #29
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
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
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5457)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
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
	nop <$0C ;hides 'nop $E3A0'
	ldy #227
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5460)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/25 ;hides 'nop'
	inc @zptemp
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5461)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5462)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
.elseif(n = 5464)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	ldx @zptemp
.elseif(n = 5466)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5467)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $08A0'
	ldy #8
	brk 0
	iny
	Jne *-6
	tay
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
.if(n = 5469)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5470)
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
	delay_n (n-7)
	plp
.elseif(n = 5474)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	brk 0
	dey
	Jne *-5
	ldy @zptemp
	brk 0
.elseif(n = 5475 || n = 5476)
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
	stx @zptemp
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5478)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E4A0'
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
	nop <$0C ;hides 'nop $EEA0'
	ldy #238
	jsr @nearby_rts_14cyc
	dey
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5483)
	; 13 bytes
	sty @zptemp
	ldy #90 ;hides 'nop'
	delay_n 26
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5484)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $07A0'
	ldy #7
	brk 0
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
	brk 0
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
	ldy #90 ;hides 'nop'
	delay_n 26
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5489)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $07A0'
	ldy #7
	brk 0
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
.elseif(n = 5492)
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
.elseif(n = 5495 || n = 5500)
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
.elseif(n = 5496)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5497)
	; 14 bytes
	sty @zptemp
	ldy #183
	jsr @nearby_rts
	brk 0
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
.elseif(n = 5501)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $E5A0'
	ldy #229
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5502)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $11A0'
	ldy #17
	jsr @nearby_rts_14cyc
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
	; 12 bytes
	sty @zptemp
	ldy #(n-5)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5506)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $06A0'
	ldy #6
	brk 0
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
.if((n >= 5508) && (n < 5527))
; 5508 to 5526
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5510)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5511)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $06A0'
	ldy #6
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5512)
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
.elseif(n = 5513)
	; 14 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5514)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5515)
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
	ldy #250 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5517)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5518)
	; 14 bytes
	sty @zptemp
	ldy #167
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5519)
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
.elseif(n = 5521)
	; 14 bytes
	pha
	tya
	ldy #(n-9)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5522)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/26 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
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
	ldy #26 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5526)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $E6A0'
	ldy #230
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.endif
.else
; 5527 to 5546
.if(n = 5527)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $05A0'
	ldy #5
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5528)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $05A0'
	ldy #5
	brk 0
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
	ldy #26 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5531 || n = 5535 || n = 5536 || n = 5539)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5532)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5533)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $05A0'
	ldy #5
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5534)
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
.elseif(n = 5537)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5538 || n = 5540 || n = 5541 || n = 5544 || n = 5546)
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
.elseif(n = 5542)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5543)
	; 14 bytes
	sty @zptemp
	ldy #(n-31)/26 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5545)
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
	pha
	tya
	ldy #(n-36)/26 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
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
	nop <$0C ;hides 'nop $04A0'
	ldy #4
	brk 0
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
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
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
	brk 0
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
	brk 0
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
	nop #<$0C ;hides 'nop $03A0'
	ldy #3
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5572)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $03A0'
	ldy #3
	brk 0
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
	nop #<$0C ;hides 'nop $03A0'
	ldy #3
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5577)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $03A0'
	ldy #3
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5579)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $18A0'
	ldy #24
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5580)
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
	brk 0
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
	nop #<$0C ;hides 'nop $02A0'
	ldy #2
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5594)
	; 13 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	dey
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
	nop #<$0C ;hides 'nop $02A0'
	ldy #2
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5599)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	dey
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
	brk 0
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
	tya
	nop <$0C ;hides 'nop $E9A0'
	ldy #233
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5604)
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5608 || n = 5609 || n = 5613 || n = 5614)
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5615)
	; 13 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $01A0'
	ldy #1
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5616)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $01A0'
	ldy #1
	brk 0
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
.elseif(n = 5619)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5620)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $01A0'
	ldy #1
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5621)
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/24 ;hides 'nop'
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5625)
	; 14 bytes
	pha
	tya
	ldy #12 ;hides 'nop abs'
	inc @zptemp
	jsr @nearby_rts_14cyc
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
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5627)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5628)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 6
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5629 || n = 5630 || n = 5634 || n = 5635 || n = 5641)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5631)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5632)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
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
	delay_n 6
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 5637)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5638)
	; 13 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $00A0'
	ldy #0
	brk 0
	iny
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
	; 13 bytes
	sty @zptemp
	ldy #(n-28)/23 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5642)
	; 13 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $00A0'
	ldy #0
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5643)
	; 13 bytes
	pha
	tya
	nop <$0C ;hides 'nop $00A0'
	ldy #0
	brk 0
	iny
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
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5646 || n = 5656 || n = 5659 || n = 5660)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5647)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5648 || n = 5653 || n = 5661)
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
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5651)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $EBA0'
	ldy #235
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5652)
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5658)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5662)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5663)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5664)
	; 14 bytes
	stx @zptemp
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	ldx @zptemp
.endif
.endif
.else
.if((n >= 5665) && (n < 5684))
; 5665 to 5683
.if(n = 5665)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5666 || n = 5667 || n = 5679 || n = 5681)
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
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5669)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5670)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5671)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5672)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5673)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5674)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5675)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5676)
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
.elseif(n = 5677)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
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
	brk 0
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
.endif
.else
; 5684 to 5703
.if(n = 5684)
	; 14 bytes
	delay_n (n-2)
	nop
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
	nop #<$0C ;hides 'nop $09A0'
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5688)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5689)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 9
	jsr @nearby_rts_14cyc
	Jne *-7
	ldx @zptemp
.elseif(n = 5690 || n = 5696 || n = 5697 || n = 5702)
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
.elseif(n = 5691)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $09A0'
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
	nop <$0C ;hides 'nop $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5693)
	; 13 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 5695)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5698)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5699)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $13A0'
	ldy #19
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5700)
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
.elseif(n = 5701)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 5705 || n = 5720)
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
.elseif(n = 5706)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #22
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 5707)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5708 || n = 5713)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5709)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5710)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $08A0'
	ldy #8
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 5712)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5714)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5715)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $08A0'
	ldy #8
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
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
	nop #<$0C ;hides 'nop $12A0'
	ldy #18
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5718)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $EEA0'
	ldy #238
	jsr @nearby_rts_15cyc
	dey
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
	nop #<$0C ;hides 'nop $12A0'
	ldy #18
	jsr @nearby_rts_15cyc
	iny
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
	nop <$0C ;hides 'nop $EEA0'
	ldy #238
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5724)
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
.elseif(n = 5725 || n = 5726 || (n > 5727 && n < 5731))
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
.elseif(n = 5727)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5733)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $07A0'
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5738)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $07A0'
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
	brk 0
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
	nop #<$0C ;hides 'nop $11A0'
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
.elseif(n = 5744 || n = 5749 || n = 5759 || n = 5764 || n = 5775)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5745 || n = 5750 || n = 5751 || n = 5768 || n = 5769 || n = 5774 || n = 5777)
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
	nop #<$0C ;hides 'nop $11A0'
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
.elseif(n = 5752)
	; 14 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
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
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5756)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $06A0'
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
	brk 0
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
	; 13 bytes
	pha
	tya
	ldy #(n-10)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5761)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $06A0'
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
	brk 0
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
	nop #<$0C ;hides 'nop $10A0'
	ldy #16
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5766)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $10A0'
	ldy #16
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5770)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $10A0'
	ldy #16
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5771)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $10A0'
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
	brk 0
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
	brk 0
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5783)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $05A0'
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5786 || n = 5788 || n = 5792 || n = 5793 || n = 5798 || n = 5800)
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5789)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $0FA0'
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
	nop #<$0C ;hides 'nop $0FA0'
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
	nop <$0C ;hides 'nop $F1A0'
	ldy #241
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5796)
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
.elseif(n = 5797)
	; 14 bytes
	sty @zptemp
	ldy #193
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
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
	nop #<$0C ;hides 'nop $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5802)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $04A0'
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
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5806)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $04A0'
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5808)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
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
	brk 0
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5813)
	; 14 bytes
	stx @zptemp
	nop #<$0C ;hides 'nop $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5814)
	; 14 bytes
	stx @zptemp
	nop <$0C ;hides 'nop $0EA2'
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
	nop #<$0C ;hides 'nop $0EA2'
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
	nop <$0C ;hides 'nop $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5820)
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
; 5821 to 5859
.if((n > 5820 && n < 5824) || n = 5828 || n = 5841 || n = 5845 || n = 5846 || n = 5851 || n = 5858)
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
.elseif(n = 5824)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5825)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $03A0'
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
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5829)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $03A0'
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
	nop <$0C ;hides 'nop $03A0'
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5832 || n = 5857)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5833 || n = 5834 || n = 5856 || n = 5859)
	; 14 bytes
	delay_n (n-2)
	nop
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
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5838)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $F3A0'
	ldy #243
	jsr @nearby_rts_15cyc
	dey
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
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5843)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $F3A0'
	ldy #243
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5844)
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
.elseif(n = 5847)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5848)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5850)
	; 13 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5852)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $02A0'
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
	nop <$0C ;hides 'nop $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5855)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	dey
	Jne *-6
	tay
	pla
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
	ldy #(n-10)/25 ;hides 'nop'
	inc @zptemp
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5861)
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/24 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5862)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
.elseif(n = 5865)
	; 13 bytes
	pha
	tya
	ldy #(n-9)/24 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk 0
	dey
	Jne *-6
	tay
	pla
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
	ldy #12 ;hides 'nop abs'
	nop $0
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5869)
	; 14 bytes
	pha
	tya
	ldy #12 ;hides 'nop abs'
	inc @zptemp
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5870)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $01A0'
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
.elseif(n = 5874 || n = 5878)
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
.elseif(n = 5875)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $01A0'
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
	nop <$0C ;hides 'nop $01A0'
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
	brk 0
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
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5883)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	nop $04
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5884)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop $04
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5885)
	; 13 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5886)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5888)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	nop $04
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
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5892)
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
	nop <$0C ;hides 'nop $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	iny
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
	; 13 bytes
	pha
	tya
	ldy #(n-10)/27 ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-6
	tay
	pla
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
	nop <$0C ;hides 'nop $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5900)
	; 13 bytes
	pha
	tya
	ldy #0
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5901 || n = 5903 || n = 5906 || n = 5908 || n = 5913 || n = 5918 || n = 5923 || n = 5924 || n = 5929 || n = 5931 || n = 5936)
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
.elseif(n = 5904)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5905)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5907 || n = 5932)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
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
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
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
	brk 0
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
	txa
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
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
.elseif(n = 5917)
	; 14 bytes
	sty @zptemp
	ldy #197
	jsr @nearby_rts
	brk 0
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
.elseif(n = 5921)
	; 14 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	delay_n 9
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5922)
	; 14 bytes
	pha
	tya
	ldy #197
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
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
.elseif(n = 5926)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 9
	brk 0
	iny
	Jne *-7
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
.elseif(n = 5930)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5934)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $09A0'
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
	nop #<$0C ;hides 'nop $09A0'
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
	nop <$0C ;hides 'nop $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5940)
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
.elseif((n > 5940 && n < 5947) || n = 5948 || n = 5949 || n = 5951 || n = 5953 || n = 5954 || n = 5956 || n = 5960 || n = 5967 || n = 5969 || (n > 5970 && n < 5974) || n = 5975 || n = 5976)
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
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk 0
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 5955)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5957)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5958)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $08A0'
	ldy #8
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
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
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5963)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #23
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 5965 || n = 5970)
	; 14 bytes
	delay_n (n-2)
	nop
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
.endif
.else
; 5977 to 6015
.if(n = 5977)
	; 14 bytes
	sty @zptemp
	ldy #199
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5978 || n = 5993 || n = 5994 || (n > 5997 && n < 6001) || (n > 6001 && n < 6005) || n = 6008 || n = 6015)
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
	nop #<$0C ;hides 'nop $07A0'
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
	brk 0
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
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5986)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $07A0'
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5988)
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
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6010)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
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
.elseif(n = 6017 || n = 6020 || n = 6022 || n = 6024 || n = 6027 || n = 6040 || n = 6041 || n = 6043 || n = 6045 || (n > 6046 && n < 6051))
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
.elseif(n = 6019)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	iny
	Jne *-7
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
.elseif(n = 6025)
	; 14 bytes
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	nop
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
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
	nop #<$0C ;hides 'nop $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6030)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $05A0'
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
	brk 0
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
	nop #<$0C ;hides 'nop $05A0'
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
	nop <$0C ;hides 'nop $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6036)
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
.elseif(n = 6037)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
	ldy #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6054)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $04A0'
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
.elseif(n = 6056 || n = 6061 || n = 6063 || n = 6066 || n = 6068 || n = 6069 || n = 6071 || (n > 6072 && n < 6077) || n = 6081 || n = 6086 || n = 6093)
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
	ldy #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6059)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6060 || n = 6084)
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
.elseif(n = 6062 || n = 6087)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6064 || n = 6090)
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
	brk 0
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
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6077)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $03A0'
	ldy #3
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6078)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $03A0'
	ldy #3
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6080)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6082)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6083)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6085)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6089)
	; 14 bytes
	sty @zptemp
	ldy #(n-5)/26 ;hides 'nop'
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
.elseif(n = 6095)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	brk 0
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
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6098)
	; 14 bytes
	sty @zptemp
	ldy #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	delay_n 7
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6099)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
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
	brk 0
	Jne *-6
	tax
	pla
.elseif(n = 6101)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6102)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $02A0'
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
.elseif(n = 6104)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6105)
	; 14 bytes
	sty @zptemp
	ldy #(n-5)/25 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6106)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $02A0'
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6108)
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
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6110)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/25 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6111)
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
.elseif(n = 6112)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 6113 to 6132
.if(n = 6113 || n = 6128)
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
.elseif(n = 6115)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6116)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 6
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
	; 14 bytes
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
.elseif(n = 6120)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
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
	tya
	ldy #191
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6125)
	; 14 bytes
	sty @zptemp
	nop #<$0C ;hides 'nop $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6126)
	; 14 bytes
	sty @zptemp
	nop <$0C ;hides 'nop $01A0'
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
.elseif(n = 6129)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6130)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $01A0'
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
	nop <$0C ;hides 'nop $01A0'
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
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
; 6133 to 6171
.if(n = 6133 || n = 6160 || n = 6161 || n = 6165 || n = 6166 || n = 6170 || n = 6171)
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
.elseif(n = 6134 || n = 6142 || n = 6147 || n = 6152 || n = 6159 || n = 6164)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6135)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6136 || n = 6163)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6137 || n = 6162)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6138)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EAEA ;hides two 'nop's
	nop
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 6139)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	delay_n 4
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6140)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 8
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6141 || n = 6168)
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
.elseif(n = 6143)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $EAEA ;hides two 'nop's
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 6144)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	delay_n 4
	dey
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
.elseif(n = 6146)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
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
	nop #<$0C ;hides 'nop $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
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
.elseif(n = 6153)
	; 14 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	delay_n 9
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6154)
	; 14 bytes
	pha
	tya
	nop #<$0C ;hides 'nop $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6155)
	; 14 bytes
	pha
	tya
	nop <$0C ;hides 'nop $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	iny
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
.elseif(n = 6157)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6158)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	delay_n 9
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6167)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6169)
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
; 6172 to 6250
.if(n = 6172)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6173 || n = 6175 || n = 6177 || n = 6179 || n = 6180 || n = 6185 || n = 6186 || n = 6191 || n = 6193 || n = 6194 || (n > 6195 && n < 6200) || (n > 6200 && n < 6205) || n = 6206 || n = 6208 || n = 6213 || n = 6218 || n = 6219 || n = 6223 || n = 6224 || (n > 6227 && n < 6232) || n = 6233 || n = 6235 || n = 6236 || n = 6241 || n = 6242 || n = 6246 || n = 6248 || n = 6250)
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
	brk 0
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
.elseif(n = 6184 || n = 6189 || n = 6209 || n = 6214 || n = 6222 || n = 6227 || n = 6234 || n = 6239 || n = 6240 || n = 6245)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6187 || n = 6237)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6195 || n = 6249)
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
.elseif(n = 6205)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6210)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	inc @zptemp
	brk 0
	dey
	Jne *-6
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
	; 13 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 7
	brk 0
	iny
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
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #24
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 6221 || n = 6247)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
.if(n = 6251 || n = 6253 || n = 6254 || n = 6256 || n = 6258 || n = 6261 || n = 6263 || (n > 6264 && n < 6269) || n = 6272 || n = 6280 || n = 6281 || n = 6283 || n = 6285 || n = 6286 || n = 6288)
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
.elseif(n = 6255)
	; 14 bytes
	sty @zptemp
	ldy #250 ;hides 'nop'
	nop $04
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6257 || n = 6282)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6259 || n = 6264 || n = 6284 || n = 6289)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6260)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/25 ;hides 'nop'
	inc @zptemp
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6262 || n = 6287)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
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
.if((n > 6289 && n < 6298) || n = 6313 || (n > 6316 && n < 6323) || n = 6324 || n = 6326)
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
.elseif(n = 6299 || n = 6325)
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
	brk 0
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
	delay_n (n-13)
	brk 0
.elseif(n = 6312)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
	ldy #(n-5)/27 ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6327)
	; 14 bytes
	sty @zptemp
	ldy #218 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6328)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/27 ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if(n = 6329 || n = 6335 || n = 6340 || n = 6344 || n = 6345 || n = 6350 || n = 6358 || n = 6360 || n = 6361 || n = 6363 || n = 6365 || n = 6366)
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
.elseif(n = 6330)
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
	brk 0
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6341)
	; 14 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	jsr @nearby_rts
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6342)
	; 14 bytes
	pha
	tya
	ldy #211
	jsr @nearby_rts
	brk 0
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
.elseif(n = 6346)
	; 14 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6347)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6348)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6349)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 6353)
	; 14 bytes
	pha
	tya
	ldy #(n-9)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6354)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/26 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6355)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	inc @zptemp
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6356)
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
.if((n >= 6368) && (n < 6387))
; 6368 to 6386
.if(n = 6368 || n = 6370 || n = 6385 || n = 6386)
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
.elseif(n = 6369 || n = 6373 || n = 6378)
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
	brk 0
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
.elseif(n = 6375)
	; 14 bytes
	sty @zptemp
	ldy #(n-31)/26 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 6379)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6380)
	; 14 bytes
	pha
	tya
	ldy #(n-36)/26 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
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
.elseif(n = 6382)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
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
.endif
.else
; 6387 to 6406
.if(n = 6387)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6388 || n = 6390)
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
.elseif(n = 6389 || n = 6395 || n = 6403 || n = 6405)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6391)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6392)
	; 14 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	iny
	Jne *-7
	tay
	pla
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
	nop $EAEA ;hides two 'nop's
	nop $04
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
	sty @zptemp
	ldy #213
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
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
	nop $EA ;hides 'nop'
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
.elseif(n = 6402)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 9
	iny
	Jne *-5
	sta (@ptrtemp),y
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.endif
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
	brk 0
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
	sty @zptemp
	ldy #194
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6410)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	brk 0
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6413 || (n > 6414 && n < 6424) || n = 6425 || n = 6428 || n = 6430 || n = 6435 || n = 6436 || n = 6440 || n = 6441 || n = 6443)
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
.elseif(n = 6424)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6426 || n = 6431)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6427)
	; 14 bytes
	sty @zptemp
	ldy #214
	jsr @nearby_rts
	brk 0
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
	brk 0
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
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.endif
.else
; 6446 to 6484
.if(n = 6446 || n = 6448 || n = 6449 || n = 6451 || n = 6452 || n = 6454 || n = 6456 || n = 6459 || n = 6461 || n = 6463 || n = 6464 || (n > 6465 && n < 6471) || (n > 6471 && n < 6475) || n = 6478 || n = 6483)
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
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6453)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6455)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6457 || n = 6462 || n = 6479 || n = 6484)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6458)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 6477)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #25
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.endif
.endif
.else
; 6485 to 6562
.if(n = 6485 || n = 6489 || n = 6490 || (n > 6493 && n < 6501) || n = 6502 || n = 6504 || n = 6509 || n = 6511 || n = 6515 || n = 6516 || n = 6518 || n = 6520 || n = 6521 || n = 6523 || (n > 6524 && n < 6533) || n = 6534 || n = 6536 || n = 6537 || n = 6542 || n = 6543 || n = 6545 || n = 6549 || n = 6551 || n = 6554 || n = 6556 || n = 6558 || n = 6560 || n = 6561)
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
.elseif(n = 6505)
	; 14 bytes
	sty @zptemp
	ldy #(n-5)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
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
.elseif(n = 6539)
	; 13 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	delay_n 26
	iny
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
.elseif(n = 6547)
	; 14 bytes
	sty @zptemp
	ldy #218
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6552)
	; 14 bytes
	pha
	tya
	ldy #218
	jsr @nearby_rts
	brk 0
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
.if((n >= 6563) && (n < 6602))
; 6563 to 6601
.if(n = 6563 || n = 6565 || n = 6566 || n = 6569 || n = 6570 || n = 6575 || n = 6576 || n = 6578 || n = 6580 || n = 6583 || n = 6588 || n = 6589 || (n > 6590 && n < 6595) || n = 6596 || n = 6597 || n = 6601)
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
.elseif(n = 6564 || n = 6590)
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
.elseif(n = 6568 || n = 6595)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6571)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	inc @zptemp
	brk 0
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 6573 || n = 6600)
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
	brk 0
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
.elseif(n = 6581 || n = 6586)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6582)
	; 14 bytes
	pha
	tya
	ldy #219
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6584)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6585)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6587)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	inc @zptemp
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6598)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	inc @zptemp
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6599)
	; 14 bytes
	sty @zptemp
	ldy #206
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 6602 to 6640
.if(n = 6602 || n = 6603 || n = 6606 || n = 6608 || n = 6609 || n = 6613 || n = 6614 || (n > 6617 && n < 6622) || n = 6623 || n = 6624 || (n > 6627 && n < 6631) || (n > 6631 && n < 6636) || n = 6638 || n = 6639)
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
.elseif(n = 6611 || n = 6637)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 6615)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6616)
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
.elseif(n = 6617)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6622)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6625)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	inc @zptemp
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6626)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 8
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6627)
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
.elseif(n = 6643 || n = 6648 || n = 6674 || n = 6679)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6644 || n = 6647 || n = 6653 || (n > 6657 && n < 6663) || (n > 6663 && n < 6667) || (n > 6668 && n < 6672))
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
	nop $EAEA ;hides two 'nop's
	delay_n 4
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6651)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	delay_n 6
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
	nop $EAEA ;hides two 'nop's
	delay_n 4
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6656)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	delay_n 6
	dey
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
	brk 0
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
.elseif(n = 6675)
	; 14 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
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
.if(n = 6680)
	; 14 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
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
.elseif((n > 6681 && n < 6695) || n = 6696 || n = 6698 || n = 6699 || n = 6707 || (n > 6711 && n < 6719))
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
	brk 0
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
.elseif(n = 6701)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 7
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6702)
	; 14 bytes
	pha
	tya
	ldy #223
	jsr @nearby_rts
	brk 0
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
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	brk 0
	dey
	Jne *-6
	tay
	pla
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
.if((n > 6718 && n < 6727) || n = 6728 || n = 6729 || n = 6731 || n = 6740 || n = 6743 || (n > 6744 && n < 6755) || n = 6756 || n = 6758 || n = 6761 || n = 6763 || n = 6766 || n = 6768 || n = 6769 || n = 6771 || (n > 6772 && n < 6777) || (n > 6777 && n < 6784) || n = 6785 || n = 6786 || n = 6788 || n = 6790 || (n > 6792 && n < 6796))
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
	brk 0
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
	brk 0
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
.elseif(n = 6734)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #26
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 6737)
	; 14 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	delay_n 27
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
.elseif(n = 6742)
	; 14 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	delay_n 27
	iny
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
.elseif(n = 6755)
	; 13 bytes
	sty @zptemp
	ldy #(n-5)/27 ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6759)
	; 14 bytes
	sty @zptemp
	ldy #211
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6760)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/27 ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6765 || n = 6796)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6770)
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
.elseif(n = 6787)
	; 14 bytes
	sty @zptemp
	ldy #226
	jsr @nearby_rts
	brk 0
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
	brk 0
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 6797 to 6875
.if(n = 6797 || n = 6799 || n = 6800 || n = 6802 || n = 6804 || (n > 6805 && n < 6810) || (n > 6811 && n < 6816) || n = 6818 || n = 6819 || n = 6821 || n = 6824 || n = 6826 || n = 6830 || n = 6831 || n = 6833 || (n > 6834 && n < 6838) || (n > 6838 && n < 6843) || n = 6845 || n = 6846 || n = 6848 || n = 6850 || n = 6851 || n = 6853 || n = 6856 || n = 6857 || n = 6859 || n = 6861 || n = 6862 || n = 6864 || (n > 6865 && n < 6870) || (n > 6871 && n < 6876))
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
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6820)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6822)
	; 14 bytes
	pha
	tya
	ldy #227
	jsr @nearby_rts
	brk 0
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
.elseif(n = 6825)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	brk 0
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 6844)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6847)
	; 14 bytes
	sty @zptemp
	ldy #228
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6849)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	delay_n 9
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6852)
	; 14 bytes
	pha
	tya
	ldy #228
	jsr @nearby_rts
	brk 0
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
	brk 0
	iny
	Jne *-7
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
.if((n >= 6876) && (n < 6915))
; 6876 to 6914
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
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6878)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6879 || n = 6880 || n = 6884 || n = 6885 || n = 6890 || n = 6893 || n = 6895 || (n > 6897 && n < 6903) || n = 6906)
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
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6883 || n = 6888 || n = 6891 || n = 6896 || n = 6905 || n = 6910 || n = 6914)
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
.elseif(n = 6889)
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
.elseif(n = 6894)
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
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
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
	nop $EA ;hides 'nop'
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
	nop $EAEA ;hides two 'nop's
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
	nop $EA ;hides 'nop'
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
.endif
.else
; 6915 to 6953
.if(n = 6915 || n = 6922 || n = 6927 || n = 6953)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6916 || n = 6918 || n = 6921 || n = 6923 || n = 6926 || (n > 6927 && n < 6935) || n = 6936 || n = 6938 || n = 6939 || n = 6941 || (n > 6942 && n < 6949) || n = 6950 || n = 6952)
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
.elseif(n = 6917)
	; 14 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6919)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6920 || n = 6951)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 6925)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6935)
	; 14 bytes
	sty @zptemp
	ldy #58 ;hides 'nop'
	nop
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6937)
	; 14 bytes
	sty @zptemp
	ldy #210
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 6949)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6955 || n = 6957 || (n > 6958 && n < 6967) || n = 6968 || n = 6969 || n = 6971 || n = 6973 || n = 6974 || (n > 6975 && n < 6981) || n = 6985 || n = 6990 || n = 6992 || n = 6994 || n = 6995 || (n > 6998 && n < 7002) || n = 7004 || n = 7006 || n = 7007 || n = 7009 || n = 7011 || n = 7012 || n = 7014 || n = 7016 || n = 7017 || n = 7019 || (n > 7020 && n < 7027) || (n > 7027 && n < 7032))
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
.elseif(n = 6958 || n = 6984 || n = 6989 || n = 6993 || n = 6998 || n = 7015 || n = 7020)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6967)
	; 14 bytes
	sty @zptemp
	ldy #232
	jsr @nearby_rts
	brk 0
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
	brk 0
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
.elseif(n = 6981)
	; 14 bytes
	sty @zptemp
	ldy #218 ;hides 'nop'
	jsr @nearby_rts
	brk 0
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 6986)
	; 14 bytes
	pha
	tya
	ldy #218 ;hides 'nop'
	jsr @nearby_rts
	brk 0
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 6991)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #27
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 6997)
	; 14 bytes
	sty @zptemp
	ldy #233
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7002)
	; 14 bytes
	pha
	tya
	ldy #233
	jsr @nearby_rts
	brk 0
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
	ldy #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
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
	ldy #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7027)
	; 14 bytes
	sty @zptemp
	ldy #234
	jsr @nearby_rts
	brk 0
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
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif((n > 7032 && n < 7036) || (n > 7036 && n < 7041) || n = 7042 || n = 7043 || n = 7045 || n = 7048 || n = 7050 || (n > 7052 && n < 7057) || (n > 7057 && n < 7062) || (n > 7062 && n < 7069) || (n > 7070 && n < 7074) || n = 7078 || n = 7083 || n = 7085 || (n > 7087 && n < 7092) || (n > 7092 && n < 7102) || (n > 7102 && n < 7106) || n = 7109)
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
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7062)
	; 14 bytes
	pha
	tya
	ldy #235
	jsr @nearby_rts
	brk 0
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
.elseif(n = 7070)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	delay_n 7
	dey
	Jne *-7
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
.elseif(n = 7076)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7079)
	; 14 bytes
	sty @zptemp
	ldy #221
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7081)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	brk 0
	dey
	Jne *-7
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
.elseif(n = 7086)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7087)
	; 14 bytes
	sty @zptemp
	ldy #236
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7092)
	; 14 bytes
	pha
	tya
	ldy #236
	jsr @nearby_rts
	brk 0
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
.elseif(n = 7112 || n = 7114 || (n > 7117 && n < 7122) || (n > 7122 && n < 7133) || n = 7146)
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
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7122)
	; 14 bytes
	pha
	tya
	ldy #237
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7133)
	; 13 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7134)
	; 14 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 7138)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 26
	dey
	Jne *-6
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
.elseif(n = 7143)
	; 14 bytes
	sty @zptemp
	ldy #223
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7145)
	; 14 bytes
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7147)
	; 14 bytes
	sty @zptemp
	ldy #238
	jsr @nearby_rts
	brk 0
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
.if(n = 7149 || n = 7151 || (n > 7152 && n < 7159) || (n > 7159 && n < 7163) || n = 7165 || n = 7167 || n = 7171 || n = 7172 || n = 7174 || n = 7176 || n = 7178 || n = 7179 || n = 7181 || n = 7183 || n = 7184 || n = 7186 || n = 7187)
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
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7159)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7163)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $FF44 ;hides 'nop zp'
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
.elseif(n = 7166)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inc @zptemp
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
	brk 0
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
	brk 0
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
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
.if((n >= 7188) && (n < 7266))
; 7188 to 7265
.if((n > 7187 && n < 7197) || n = 7198 || n = 7200 || n = 7203 || n = 7205 || (n > 7207 && n < 7212) || (n > 7212 && n < 7230) || n = 7231 || n = 7233 || n = 7236 || n = 7238 || n = 7240 || n = 7241 || n = 7243 || (n > 7244 && n < 7248) || n = 7249 || n = 7251 || n = 7252 || n = 7254 || (n > 7255 && n < 7260) || n = 7262 || n = 7264 || n = 7265)
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
.elseif(n = 7197)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 7202)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
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
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7212)
	; 14 bytes
	pha
	tya
	ldy #240
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7232 || n = 7237 || n = 7250 || n = 7255 || n = 7263)
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
	brk 0
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
.elseif(n = 7248)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #28
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
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
	brk 0
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
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7332)
	; 14 bytes
	pha
	tya
	ldy #244
	jsr @nearby_rts
	brk 0
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
.if((n > 7343 && n < 7349) || (n > 7349 && n < 7354) || n = 7355 || n = 7358 || n = 7360 || n = 7363 || n = 7364 || n = 7368 || n = 7369 || (n > 7372 && n < 7385) || n = 7386 || n = 7388 || n = 7389 || n = 7393 || n = 7394 || n = 7397 || n = 7398 || (n > 7399 && n < 7404) || (n > 7404 && n < 7414))
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
.elseif(n = 7349)
	; 14 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 27
	dey
	Jne *-7
	ldy @zptemp
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
	brk 0
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
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7365)
	; 14 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	brk 0
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 7370)
	; 14 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	brk 0
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
.elseif(n = 7414)
	; 14 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7415)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7417)
	; 14 bytes
	sty @zptemp
	ldy #247
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7419)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	tay
	pla
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
	ldy #247
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
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
	nop $EA ;hides 'nop'
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
.elseif((n > 7425 && n < 7429) || n = 7433 || n = 7435 || (n > 7437 && n < 7447) || n = 7448 || n = 7450 || n = 7451 || n = 7453 || (n > 7454 && n < 7459))
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
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7430)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
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
	brk 0
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
.elseif(n = 7459)
	; 14 bytes
	pha
	tya
	ldy #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7460)
	; 14 bytes
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.endif
.else
; 7461 to 7500
.if(n = 7461 || n = 7462 || n = 7464 || n = 7466 || n = 7467 || n = 7469 || (n > 7470 && n < 7477) || n = 7479 || n = 7481 || n = 7484 || (n > 7485 && n < 7493) || n = 7494 || n = 7496 || n = 7497 || n = 7499)
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
.elseif(n = 7477)
	; 14 bytes
	sty @zptemp
	ldy #249
	jsr @nearby_rts
	brk 0
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
.elseif(n = 7480 || n = 7485)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7482)
	; 14 bytes
	pha
	tya
	ldy #249
	jsr @nearby_rts
	brk 0
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
.elseif(n = 7493)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	brk 0
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
.if(n = 7501 || n = 7502 || n = 7504 || n = 7506 || n = 7508 || n = 7513 || n = 7515 || (n > 7516 && n < 7527) || (n > 7527 && n < 7531) || (n > 7532 && n < 7536) || n = 7538 || n = 7539 || n = 7541 || n = 7543 || n = 7544 || n = 7546 || (n > 7547 && n < 7559) || (n > 7559 && n < 7564) || n = 7566 || n = 7568 || n = 7574 || n = 7575 || n = 7577)
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
.elseif(n = 7505)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #29
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 7507 || n = 7511 || n = 7512 || n = 7516 || n = 7542 || n = 7547 || n = 7573 || n = 7578)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7509 || n = 7540 || n = 7571)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
	brk 0
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
.elseif(n = 7565)
	; 14 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7567)
	; 14 bytes
	sty @zptemp
	ldy #252
	jsr @nearby_rts
	brk 0
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
.elseif(n = 7570)
	; 14 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7572)
	; 14 bytes
	pha
	tya
	ldy #252
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 7579 to 7656
.if((n > 7578 && n < 7591) || (n > 7591 && n < 7595) || n = 7598 || n = 7599 || n = 7601 || n = 7603 || n = 7605 || n = 7606 || n = 7608 || (n > 7609 && n < 7623) || (n > 7623 && n < 7627) || n = 7629 || n = 7631 || n = 7634 || n = 7636 || n = 7637 || n = 7639 || (n > 7640 && n < 7646) || n = 7647 || n = 7649 || n = 7650 || n = 7652 || n = 7654 || n = 7656)
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
.elseif(n = 7595)
	; 13 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 26
	iny
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
	brk 0
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
	brk 0
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
; 7657 to 7734
.if(n = 7657)
	; 14 bytes
	sty @zptemp
	ldy #255
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7658 || n = 7659 || n = 7665 || n = 7667 || n = 7670 || (n > 7671 && n < 7676) || n = 7677 || n = 7679 || n = 7680 || n = 7682 || (n > 7683 && n < 7687) || (n > 7687 && n < 7692) || n = 7693 || n = 7694 || n = 7698 || n = 7699 || (n > 7702 && n < 7719) || (n > 7719 && n < 7724) || n = 7725 || n = 7730)
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
	brk 0
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
.elseif(n = 7664 || n = 7695 || n = 7726)
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
.elseif(n = 7669 || n = 7700 || n = 7731)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7671 || n = 7678 || n = 7683 || n = 7697 || n = 7702 || n = 7728 || n = 7733)
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
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7692)
	; 14 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7696)
	; 14 bytes
	sty @zptemp
	ldy #233
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
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
.else
; 7735 to 7812
.if((n > 7734 && n < 7751) || (n > 7751 && n < 7756) || n = 7758 || n = 7760 || n = 7761 || n = 7763 || n = 7765 || n = 7766 || n = 7768 || (n > 7769 && n < 7783) || (n > 7783 && n < 7788) || n = 7789 || n = 7791 || n = 7792 || n = 7794 || (n > 7795 && n < 7800) || (n > 7800 && n < 7813))
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
.elseif(n = 7759 || n = 7764 || n = 7769 || n = 7790 || n = 7795)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7762)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #30
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 7783)
	; 14 bytes
	sty @zptemp
	ldy #243
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7793)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
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
.if(n = 7813 || n = 7814 || (n > 7815 && n < 7819) || n = 7822 || n = 7823 || n = 7827 || n = 7829 || n = 7831 || n = 7832 || (n > 7833 && n < 7847) || n = 7848 || n = 7849 || n = 7851 || n = 7853 || n = 7854 || n = 7856 || (n > 7857 && n < 7861) || (n > 7861 && n < 7866) || (n > 7866 && n < 7879) || n = 7880 || n = 7882 || n = 7885 || n = 7887 || n = 7889 || n = 7890)
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
.elseif(n = 7825)
	; 14 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 27
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7828)
	; 14 bytes
	sty @zptemp
	ldy #237
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7830)
	; 14 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 27
	iny
	Jne *-7
	tay
	pla
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
	brk 0
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
	brk 0
	dex
	Jne *-5
	ldx @zptemp
.elseif(n = 7906)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk 0
	iny
	Jne *-5
	tay
	pla
.elseif(n = 7907)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	brk 0
	dey
	Jne *-5
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 7909)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk 0
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
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 7924 || n = 7929)
	; 14 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 7926)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	iny
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
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7932)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 7935)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #0 ;hides 'brk imm'
	nop
	brk 0
	dex
	Jne *-5
	ldx @zptemp
.elseif(n = 7937)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7938)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
	iny
	Jne *-5
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 7940)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
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
	brk 0
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
.elseif(n = 7946)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7947 || n = 7949 || (n > 7950 && n < 7960) || (n > 7961 && n < 7965) || n = 7967 || n = 7968)
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
.if((n >= 7969) && (n < 8047))
; 7969 to 8046
.if((n > 7968 && n < 7975) || (n > 7975 && n < 7980) || (n > 7980 && n < 7993) || (n > 7993 && n < 7998) || (n > 7998 && n < 8005) || n = 8006 || n = 8008 || n = 8009 || n = 8011 || (n > 8012 && n < 8019) || n = 8020 || n = 8022 || n = 8023 || n = 8025 || (n > 8026 && n < 8031) || (n > 8031 && n < 8039) || (n > 8039 && n < 8044) || n = 8045 || n = 8046)
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
.elseif(n = 8005)
	; 14 bytes
	sty @zptemp
	ldy #250 ;hides 'nop'
	jsr @nearby_rts
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8007)
	; 14 bytes
	sty @zptemp
	ldy #250
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8010)
	; 14 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	jsr @nearby_rts
	brk 0
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 8019)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #31
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 8021 || n = 8026)
	; 14 bytes
	delay_n (n-2)
	nop
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
.endif
.else
; 8047 to 8125
.if((n > 8046 && n < 8055) || (n > 8055 && n < 8059) || (n > 8060 && n < 8064) || (n > 8064 && n < 8071) || (n > 8071 && n < 8076) || (n > 8076 && n < 8090) || n = 8091 || n = 8093 || n = 8094 || n = 8096 || (n > 8097 && n < 8103) || (n > 8103 && n < 8108) || (n > 8108 && n < 8125))
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
.elseif(n = 8055)
	; 14 bytes
	sty @zptemp
	ldy #26 ;hides 'nop'
	nop
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 8059)
	; 14 bytes
	sty @zptemp
	ldy #244
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
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
.endif
.else
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
.if((n >= 8126) && (n < 8204))
; 8126 to 8203
.if((n > 8125 && n < 8130) || (n > 8130 && n < 8135) || (n > 8135 && n < 8140) || (n > 8140 && n < 8156) || n = 8157 || n = 8159 || n = 8160 || n = 8162 || (n > 8163 && n < 8167) || (n > 8167 && n < 8172) || (n > 8172 && n < 8183) || (n > 8183 && n < 8187) || n = 8197 || n = 8198 || (n > 8200 && n < 8204))
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
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8187)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
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
.elseif(n = 8189)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 26
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
	nop $EA ;hides 'nop'
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
.elseif(n = 8194)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 26
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
.elseif((n > 8204 && n < 8224) || (n > 8224 && n < 8229) || (n > 8230 && n < 8235) || (n > 8235 && n < 8254) || n = 8256 || n = 8258 || n = 8261 || n = 8263 || n = 8264 || (n > 8265 && n < 8270) || (n > 8270 && n < 8276) || n = 8277 || n = 8279 || n = 8280)
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
.elseif(n = 8230)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 8254)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 8255)
	; 13 bytes
	sty @zptemp
	ldy #250 ;hides 'nop'
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8257)
	; 14 bytes
	sty @zptemp
	ldy #250
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8259)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
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
.elseif(n = 8265)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 8276)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #32
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 8278)
	; 14 bytes
	delay_n (n-2)
	nop
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
.endif
.endif
.else
; 8282 to 8437
.if(n = 8282 || (n > 8283 && n < 8290) || (n > 8290 && n < 8295) || (n > 8295 && n < 8300) || (n > 8300 && n < 8305) || (n > 8305 && n < 8323) || (n > 8323 && n < 8328) || (n > 8328 && n < 8356) || (n > 8356 && n < 8361) || (n > 8361 && n < 8389) || (n > 8389 && n < 8394) || (n > 8394 && n < 8411) || n = 8412 || n = 8414 || n = 8415 || n = 8417 || (n > 8418 && n < 8422) || (n > 8422 && n < 8427) || (n > 8427 && n < 8433) || n = 8434 || n = 8436)
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
.elseif(n = 8283 || n = 8413 || n = 8418 || n = 8435)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8300)
	; 14 bytes
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
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
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 8416)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	brk 0
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
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 8437)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 27
	dey
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
; 8438 to 8593
.if(n = 8438)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 8439 || n = 8441 || n = 8447 || (n > 8451 && n < 8455) || (n > 8455 && n < 8460) || (n > 8460 && n < 8505) || (n > 8505 && n < 8510) || (n > 8510 && n < 8515) || (n > 8515 && n < 8533) || n = 8536 || n = 8537 || (n > 8540 && n < 8545) || (n > 8545 && n < 8550) || (n > 8550 && n < 8580) || (n > 8580 && n < 8585) || (n > 8585 && n < 8594))
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
.elseif(n = 8440 || n = 8445 || n = 8446 || n = 8450 || n = 8451 || n = 8535 || n = 8540)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8442)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8443)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8444)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8448)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	brk 0
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
	brk 0
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
.elseif(n = 8505)
	; 14 bytes
	sty @zptemp
	ldy #250 ;hides 'nop'
	delay_n 27
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8510)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 8533)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #33
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 8534)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	ldy @zptemp
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
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8545)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 8580)
	; 14 bytes
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
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
.endif
.else
; 8594 to 8750
.if((n > 8593 && n < 8666) || (n > 8666 && n < 8671) || (n > 8671 && n < 8685) || (n > 8685 && n < 8689) || (n > 8690 && n < 8694) || (n > 8694 && n < 8699) || (n > 8700 && n < 8704) || (n > 8705 && n < 8751))
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
.elseif(n = 8666)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 8671)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	sta @zptemp
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8685)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8689)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop $04
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 8690)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8694)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	nop $04
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8699)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $FF44 ;hides 'nop zp'
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8700)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8704)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $FF44 ;hides 'nop zp'
	brk 0
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
	sta @zptemp
	brk 0
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 8751) && (n < 9376))
.if((n >= 8751) && (n < 9063))
; 8751 to 9062
.if((n > 8750 && n < 8755) || (n > 8755 && n < 8760) || (n > 8760 && n < 8790) || n = 8791 || n = 8793 || n = 8794 || n = 8796 || (n > 8797 && n < 8921) || (n > 8921 && n < 8926) || (n > 8926 && n < 8937) || (n > 8937 && n < 8942) || n = 8943 || n = 8944 || (n > 8945 && n < 8950) || (n > 8950 && n < 8954) || n = 8957 || n = 8958 || (n > 8961 && n < 9047) || n = 9048 || n = 9050 || n = 9051 || n = 9053 || (n > 9054 && n < 9063))
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
.elseif(n = 8755)
	; 14 bytes
	sty @zptemp
	ldy #250 ;hides 'nop'
	nop
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 8790)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #34
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 8792 || n = 8797 || n = 9049 || n = 9054)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8921)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	brk 0
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
	brk 0
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
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	delay_n 4
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 8950)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	delay_n 4
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8954)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EAEA ;hides two 'nop's
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 8955)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	nop
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8956)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 6
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8959)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $EAEA ;hides two 'nop's
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8960)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop $EA ;hides 'nop'
	nop
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8961)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 6
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9047)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #35
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.endif
.else
; 9063 to 9375
.if((n > 9062 && n < 9181) || (n > 9181 && n < 9206) || (n > 9206 && n < 9214) || n = 9215 || n = 9216 || (n > 9217 && n < 9304) || n = 9305 || n = 9307 || n = 9308 || n = 9310 || (n > 9311 && n < 9376))
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
.elseif(n = 9181)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	inc @zptemp
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9206)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	inc @zptemp
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9214)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	brk 0
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
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9304)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #36
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 9306 || n = 9311)
	; 14 bytes
	delay_n (n-2)
	nop
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
.endif
.endif
.else
; 9376 to 10000
.if((n > 9375 && n < 9561) || n = 9562 || n = 9564 || n = 9565 || n = 9567 || (n > 9568 && n < 9686) || (n > 9686 && n < 9691) || (n > 9691 && n < 9713) || (n > 9713 && n < 9718) || (n > 9719 && n < 9724) || (n > 9724 && n < 9729) || (n > 9729 && n < 9818) || n = 9819 || n = 9821 || n = 9822 || n = 9824 || (n > 9825 && n < 10001))
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
.elseif(n = 9561)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #37
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 9563 || n = 9568 || n = 9820 || n = 9825)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 9686)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	brk 0
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
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9713)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	delay_n 7
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 9718)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9719)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9724)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 9
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9729)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9818)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #38
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
.if((n >= 10001) && (n < 11251))
; 10001 to 11250
.if((n > 10000 && n < 10075) || n = 10076 || n = 10078 || n = 10079 || n = 10081 || (n > 10082 && n < 10332) || n = 10333 || n = 10335 || n = 10336 || n = 10338 || (n > 10339 && n < 10589) || n = 10590 || n = 10592 || n = 10593 || n = 10595 || (n > 10596 && n < 10846) || n = 10847 || n = 10849 || n = 10850 || n = 10852 || (n > 10853 && n < 11103) || n = 11104 || n = 11106 || n = 11107 || n = 11109 || (n > 11110 && n < 11216) || (n > 11216 && n < 11221) || (n > 11221 && n < 11249) || n = 11250)
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
.elseif(n = 10075)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #39
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 10077 || n = 10082 || n = 10334 || n = 10339 || n = 10591 || n = 10596 || n = 10848 || n = 10853 || n = 11105 || n = 11110)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 10332)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #40
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 10589)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #41
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 10846)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #42
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 11103)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #43
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.endif
.else
; 11251 to 12500
.if((n > 11250 && n < 11254) || (n > 11254 && n < 11260) || (n > 11260 && n < 11265) || (n > 11265 && n < 11360) || n = 11361 || n = 11363 || n = 11364 || n = 11366 || (n > 11367 && n < 11617) || n = 11618 || n = 11620 || n = 11621 || n = 11623 || (n > 11624 && n < 11874) || n = 11875 || n = 11877 || n = 11878 || n = 11880 || (n > 11881 && n < 12131) || n = 12132 || n = 12134 || n = 12135 || n = 12137 || (n > 12138 && n < 12388) || n = 12389 || n = 12391 || n = 12392 || n = 12394 || (n > 12395 && n < 12501))
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
.elseif(n = 11254)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	brk 0
	iny
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
.elseif(n = 11360)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #44
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 11362 || n = 11367 || n = 11619 || n = 11624 || n = 11876 || n = 11881 || n = 12133 || n = 12138 || n = 12390 || n = 12395)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 11617)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #45
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 11874)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #46
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 12131)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #47
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 12388)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #48
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.endif
.endif
.else
; 12501 to 15000
.if((n > 12500 && n < 12645) || n = 12646 || n = 12648 || n = 12649 || n = 12651 || (n > 12652 && n < 12902) || n = 12903 || n = 12905 || n = 12906 || n = 12908 || (n > 12909 && n < 13159) || n = 13160 || n = 13162 || n = 13163 || n = 13165 || (n > 13166 && n < 13416) || n = 13417 || n = 13419 || n = 13420 || n = 13422 || (n > 13423 && n < 13673) || n = 13674 || n = 13676 || n = 13677 || n = 13679 || (n > 13680 && n < 13930) || n = 13931 || n = 13933 || n = 13934 || n = 13936 || (n > 13937 && n < 14187) || n = 14188 || n = 14190 || n = 14191 || n = 14193 || (n > 14194 && n < 14444) || n = 14445 || n = 14447 || n = 14448 || n = 14450 || (n > 14451 && n < 14701) || n = 14702 || n = 14704 || n = 14705 || n = 14707 || (n > 14708 && n < 14958) || n = 14959 || n = 14961 || n = 14962 || n = 14964 || (n > 14965 && n < 15001))
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
.elseif(n = 12645)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #49
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 12647 || n = 12652 || n = 12904 || n = 12909 || n = 13161 || n = 13166 || n = 13418 || n = 13423 || n = 13675 || n = 13680 || n = 13932 || n = 13937 || n = 14189 || n = 14194 || n = 14446 || n = 14451 || n = 14703 || n = 14708 || n = 14960 || n = 14965)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 12902)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #50
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 13159)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #51
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 13416)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #52
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 13673)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #53
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 13930)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #54
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 14187)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #55
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 14444)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #56
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 14701)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #57
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 14958)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #58
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.endif
.else
.if((n >= 15001) && (n < 17501))
; 15001 to 17500
.if((n > 15000 && n < 15215) || n = 15216 || n = 15218 || n = 15219 || n = 15221 || (n > 15222 && n < 15472) || n = 15473 || n = 15475 || n = 15476 || n = 15478 || (n > 15479 && n < 15729) || n = 15730 || n = 15732 || n = 15733 || n = 15735 || (n > 15736 && n < 15986) || n = 15987 || n = 15989 || n = 15990 || n = 15992 || (n > 15993 && n < 16243) || n = 16244 || n = 16246 || n = 16247 || n = 16249 || (n > 16250 && n < 16500) || n = 16501 || n = 16503 || n = 16504 || n = 16506 || (n > 16507 && n < 16757) || n = 16758 || n = 16760 || n = 16761 || n = 16763 || (n > 16764 && n < 17014) || n = 17015 || n = 17017 || n = 17018 || n = 17020 || (n > 17021 && n < 17271) || n = 17272 || n = 17274 || n = 17275 || n = 17277 || (n > 17278 && n < 17501))
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
.elseif(n = 15215)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #59
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 15217 || n = 15222 || n = 15474 || n = 15479 || n = 15731 || n = 15736 || n = 15988 || n = 15993 || n = 16245 || n = 16250 || n = 16502 || n = 16507 || n = 16759 || n = 16764 || n = 17016 || n = 17021 || n = 17273 || n = 17278)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 15472)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #60
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 15729)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #61
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 15986)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #62
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 16243)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #63
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 16500)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #64
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 16757)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #65
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 17014)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #66
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 17271)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #67
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.endif
.else
; 17501 to 20000
.if((n > 17500 && n < 17528) || n = 17529 || n = 17531 || n = 17532 || n = 17534 || (n > 17535 && n < 17785) || n = 17786 || n = 17788 || n = 17789 || n = 17791 || (n > 17792 && n < 18042) || n = 18043 || n = 18045 || n = 18046 || n = 18048 || (n > 18049 && n < 18299) || n = 18300 || n = 18302 || n = 18303 || n = 18305 || (n > 18306 && n < 18556) || n = 18557 || n = 18559 || n = 18560 || n = 18562 || (n > 18563 && n < 18813) || n = 18814 || n = 18816 || n = 18817 || n = 18819 || (n > 18820 && n < 19070) || n = 19071 || n = 19073 || n = 19074 || n = 19076 || (n > 19077 && n < 19327) || n = 19328 || n = 19330 || n = 19331 || n = 19333 || (n > 19334 && n < 19584) || n = 19585 || n = 19587 || n = 19588 || n = 19590 || (n > 19591 && n < 19841) || n = 19842 || n = 19844 || n = 19845 || n = 19847 || (n > 19848 && n < 20001))
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
.elseif(n = 17528)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #68
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 17530 || n = 17535 || n = 17787 || n = 17792 || n = 18044 || n = 18049 || n = 18301 || n = 18306 || n = 18558 || n = 18563 || n = 18815 || n = 18820 || n = 19072 || n = 19077 || n = 19329 || n = 19334 || n = 19586 || n = 19591 || n = 19843 || n = 19848)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 17785)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #69
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 18042)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #70
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 18299)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #71
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 18556)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #72
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 18813)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #73
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 19070)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #74
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 19327)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #75
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 19584)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #76
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 19841)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #77
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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

.endif
	rest
.endmacro
