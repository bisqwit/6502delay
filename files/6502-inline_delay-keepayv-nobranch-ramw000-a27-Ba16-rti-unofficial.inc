; delay_n, delays n cycles with small code
; Will clobber X, Z+N, and C
; Will preserve A, Y, S, D, V, and I (interrupt-disable flag)
;
; WARNING: May write garbage into $00-$FF.
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
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 9
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 35
; - 8 bytes or less when n <= 46
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 300
; -11 bytes or less when n <= 311
; -12 bytes or less when n <= 313
; -13 bytes or less when n <= 324
; -14 bytes or less when n <= 498
; -15 bytes or less when n <= 500
; -16 bytes or less when n <= 511
; -17 bytes or less when n <= 513
; -18 bytes or less when n <= 524
; -19 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.3, max=6
; - When 31 <= n <= 50: min=6, avg=7.5, max=9
; - When 51 <= n <= 75: min=8, avg=9.0, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=9, avg=9.0, max=9
; - When 201 <= n <= 300: min=9, avg=9.0, max=10
; - When 301 <= n <= 500: min=11, avg=13.8, max=15
; - When 501 <= n <= 750: min=9, avg=14.2, max=19
; - When 751 <= n <= 1000: min=9, avg=14.4, max=19
; - When 1001 <= n <= 2000: min=9, avg=14.4, max=19
; - When 2001 <= n <= 3000: min=9, avg=14.4, max=19
; - When 3001 <= n <= 5000: min=9, avg=14.4, max=19
; - When 5001 <= n <= 7500: min=9, avg=14.4, max=19
; - When 7501 <= n <= 10000: min=9, avg=14.4, max=19
; - When 10001 <= n <= 20000: min=9, avg=14.4, max=19
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
	; 2 bytes
	cmp $C5
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 28 || n = 34 || n = 41 || n = 43 || (n > 298 && n < 303))
	; 2..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 6)
	; 2 bytes
	inc $F6,X
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 9 || n = 11 || (n > 19 && n < 24) || n = 33 || n = 35 || n = 37 || n = 46)
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
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 31 || n = 44)
	; 4..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 19 || n = 26 || n = 32 || n = 39 || n = 45)
	; 4..8 bytes
	delay_n (n-13)
	brk
.elseif(n = 24 || n = 25 || n = 38)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27 || n = 40)
	; 5..7 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 29 || n = 30 || n = 312)
	; 6..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 36)
	; 8 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 42)
	; 8 bytes
	delay_n (n-3)
	cmp $C5
.elseif((n > 46 && n < 52) || (n > 52 && n < 299))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 52)
	; 8 bytes
	.repeat 4
	 brk
	.endrepeat
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
.elseif((n > 305 && n < 312))
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
.if((n >= 313) && (n < 332))
; 313 to 331
.if(n = 313)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 314)
	; 13 bytes
	delay_n (n-16)
	delay_n 16
.elseif(n = 315)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif((n > 315 && n < 325))
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 325)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #253
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 326)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #254
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 327)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 328)
	; 14 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 329)
	; 14 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 330)
	; 14 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 331)
	; 14 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.else
; 332 to 351
.if(n = 332)
	; 14 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 333)
	; 14 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 334)
	; 14 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 335)
	; 14 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 336)
	; 14 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 337)
	; 14 bytes
	php
	pha
	lda #10
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 338)
	; 14 bytes
	php
	pha
	lda #11
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 339)
	; 14 bytes
	php
	pha
	lda #12
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 340)
	; 14 bytes
	php
	pha
	lda #13
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 341)
	; 14 bytes
	php
	pha
	lda #14
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 342)
	; 14 bytes
	php
	pha
	lda #15
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 343)
	; 14 bytes
	php
	pha
	lda #16
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 344)
	; 14 bytes
	php
	pha
	lda #17
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 345)
	; 14 bytes
	php
	pha
	lda #18
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 346)
	; 14 bytes
	php
	pha
	lda #19
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 347)
	; 14 bytes
	php
	pha
	lda #20
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 348)
	; 14 bytes
	php
	pha
	lda #21
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 349)
	; 14 bytes
	php
	pha
	lda #22
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 350)
	; 14 bytes
	php
	pha
	lda #23
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 351)
	; 14 bytes
	php
	pha
	lda #24
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 352) && (n < 371))
; 352 to 370
.if(n = 352)
	; 14 bytes
	php
	pha
	lda #25
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 353)
	; 14 bytes
	php
	pha
	lda #26
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 354)
	; 14 bytes
	php
	pha
	lda #27
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 355)
	; 14 bytes
	php
	pha
	lda #28
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 356)
	; 14 bytes
	php
	pha
	lda #29
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 357)
	; 14 bytes
	php
	pha
	lda #30
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 358)
	; 14 bytes
	php
	pha
	lda #31
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 359)
	; 14 bytes
	php
	pha
	lda #32
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 360)
	; 14 bytes
	php
	pha
	lda #33
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 361)
	; 14 bytes
	php
	pha
	lda #34
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 362)
	; 14 bytes
	php
	pha
	lda #35
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 363)
	; 14 bytes
	php
	pha
	lda #36
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 364)
	; 14 bytes
	php
	pha
	lda #37
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 365)
	; 14 bytes
	php
	pha
	lda #38
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 366)
	; 14 bytes
	php
	pha
	lda #39
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 367)
	; 14 bytes
	php
	pha
	lda #40
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 368)
	; 14 bytes
	php
	pha
	lda #41
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 369)
	; 14 bytes
	php
	pha
	lda #42
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 370)
	; 14 bytes
	php
	pha
	lda #43
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.else
; 371 to 390
.if(n = 371)
	; 14 bytes
	php
	pha
	lda #44
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 372)
	; 14 bytes
	php
	pha
	lda #45
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 373)
	; 14 bytes
	php
	pha
	lda #46
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 374)
	; 14 bytes
	php
	pha
	lda #47
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 375)
	; 14 bytes
	php
	pha
	lda #48
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 376)
	; 14 bytes
	php
	pha
	lda #49
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 377)
	; 14 bytes
	php
	pha
	lda #50
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 378)
	; 14 bytes
	php
	pha
	lda #51
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 379)
	; 14 bytes
	php
	pha
	lda #52
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 380)
	; 14 bytes
	php
	pha
	lda #53
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 381)
	; 14 bytes
	php
	pha
	lda #54
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 382)
	; 14 bytes
	php
	pha
	lda #55
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 383)
	; 14 bytes
	php
	pha
	lda #56
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 384)
	; 14 bytes
	php
	pha
	lda #57
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 385)
	; 14 bytes
	php
	pha
	lda #58
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 386)
	; 14 bytes
	php
	pha
	lda #59
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 387)
	; 14 bytes
	php
	pha
	lda #60
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 388)
	; 14 bytes
	php
	pha
	lda #61
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 389)
	; 14 bytes
	php
	pha
	lda #62
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 390)
	; 14 bytes
	php
	pha
	lda #63
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 391) && (n < 430))
.if((n >= 391) && (n < 410))
; 391 to 409
.if(n = 391)
	; 14 bytes
	php
	pha
	lda #64
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 392)
	; 14 bytes
	php
	pha
	lda #65
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 393)
	; 14 bytes
	php
	pha
	lda #66
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 394)
	; 14 bytes
	php
	pha
	lda #67
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 395)
	; 14 bytes
	php
	pha
	lda #68
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 396)
	; 14 bytes
	php
	pha
	lda #69
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 397)
	; 14 bytes
	php
	pha
	lda #70
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 398)
	; 14 bytes
	php
	pha
	lda #71
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 399)
	; 14 bytes
	php
	pha
	lda #72
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 400)
	; 14 bytes
	php
	pha
	lda #73
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 401)
	; 14 bytes
	php
	pha
	lda #74
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 402)
	; 14 bytes
	php
	pha
	lda #75
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 403)
	; 14 bytes
	php
	pha
	lda #76
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 404)
	; 14 bytes
	php
	pha
	lda #77
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 405)
	; 14 bytes
	php
	pha
	lda #78
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 406)
	; 14 bytes
	php
	pha
	lda #79
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 407)
	; 14 bytes
	php
	pha
	lda #80
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 408)
	; 14 bytes
	php
	pha
	lda #81
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 409)
	; 14 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.else
; 410 to 429
.if(n = 410)
	; 14 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 411)
	; 14 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 412)
	; 14 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 413)
	; 14 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 414)
	; 14 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 415)
	; 14 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 416)
	; 14 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 417)
	; 14 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 418)
	; 14 bytes
	php
	pha
	lda #91
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 419)
	; 14 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 420)
	; 14 bytes
	php
	pha
	lda #93
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 421)
	; 14 bytes
	php
	pha
	lda #94
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 422)
	; 14 bytes
	php
	pha
	lda #95
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 423)
	; 14 bytes
	php
	pha
	lda #96
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 424)
	; 14 bytes
	php
	pha
	lda #97
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 425)
	; 14 bytes
	php
	pha
	lda #98
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 426)
	; 14 bytes
	php
	pha
	lda #99
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 427)
	; 14 bytes
	php
	pha
	lda #100
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 428)
	; 14 bytes
	php
	pha
	lda #101
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 429)
	; 14 bytes
	php
	pha
	lda #102
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 430) && (n < 449))
; 430 to 448
.if(n = 430)
	; 14 bytes
	php
	pha
	lda #103
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 431)
	; 14 bytes
	php
	pha
	lda #104
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 432)
	; 14 bytes
	php
	pha
	lda #105
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 433)
	; 14 bytes
	php
	pha
	lda #106
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 434)
	; 14 bytes
	php
	pha
	lda #107
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 435)
	; 14 bytes
	php
	pha
	lda #108
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 436)
	; 14 bytes
	php
	pha
	lda #109
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 437)
	; 14 bytes
	php
	pha
	lda #110
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 438)
	; 14 bytes
	php
	pha
	lda #111
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 439)
	; 14 bytes
	php
	pha
	lda #112
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 440)
	; 14 bytes
	php
	pha
	lda #113
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 441)
	; 14 bytes
	php
	pha
	lda #114
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 442)
	; 14 bytes
	php
	pha
	lda #115
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 443)
	; 14 bytes
	php
	pha
	lda #116
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 444)
	; 14 bytes
	php
	pha
	lda #117
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 445)
	; 14 bytes
	php
	pha
	lda #118
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 446)
	; 14 bytes
	php
	pha
	lda #119
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 447)
	; 14 bytes
	php
	pha
	lda #120
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 448)
	; 14 bytes
	php
	pha
	lda #121
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.else
; 449 to 468
.if(n = 449)
	; 14 bytes
	php
	pha
	lda #122
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 450)
	; 14 bytes
	php
	pha
	lda #123
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 451)
	; 14 bytes
	php
	pha
	lda #124
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 452)
	; 14 bytes
	php
	pha
	lda #125
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 453)
	; 14 bytes
	php
	pha
	lda #126
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 454)
	; 14 bytes
	php
	pha
	lda #127
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 455)
	; 14 bytes
	php
	pha
	lda #128
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 456)
	; 14 bytes
	php
	pha
	lda #129
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 457)
	; 14 bytes
	php
	pha
	lda #130
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 458)
	; 14 bytes
	php
	pha
	lda #131
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 459)
	; 14 bytes
	php
	pha
	lda #132
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 460)
	; 14 bytes
	php
	pha
	lda #133
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 461)
	; 14 bytes
	php
	pha
	lda #134
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 462)
	; 14 bytes
	php
	pha
	lda #135
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 463)
	; 14 bytes
	php
	pha
	lda #136
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 464)
	; 14 bytes
	php
	pha
	lda #137
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 465)
	; 14 bytes
	php
	pha
	lda #138
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 466)
	; 14 bytes
	php
	pha
	lda #139
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 467)
	; 14 bytes
	php
	pha
	lda #140
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 468)
	; 14 bytes
	php
	pha
	lda #141
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 469) && (n < 547))
.if((n >= 469) && (n < 508))
.if((n >= 469) && (n < 488))
; 469 to 487
.if(n = 469)
	; 14 bytes
	php
	pha
	lda #142
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 470)
	; 14 bytes
	php
	pha
	lda #143
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 471)
	; 14 bytes
	php
	pha
	lda #144
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 472)
	; 14 bytes
	php
	pha
	lda #145
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 473)
	; 14 bytes
	php
	pha
	lda #146
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 474)
	; 14 bytes
	php
	pha
	lda #147
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 475)
	; 14 bytes
	php
	pha
	lda #148
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 476)
	; 14 bytes
	php
	pha
	lda #149
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 477)
	; 14 bytes
	php
	pha
	lda #150
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 478)
	; 14 bytes
	php
	pha
	lda #151
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 479)
	; 14 bytes
	php
	pha
	lda #152
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 480)
	; 14 bytes
	php
	pha
	lda #153
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 481)
	; 14 bytes
	php
	pha
	lda #154
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 482)
	; 14 bytes
	php
	pha
	lda #155
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 483)
	; 14 bytes
	php
	pha
	lda #156
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 484)
	; 14 bytes
	php
	pha
	lda #157
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 485)
	; 14 bytes
	php
	pha
	lda #158
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 486)
	; 14 bytes
	php
	pha
	lda #159
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 487)
	; 14 bytes
	php
	pha
	lda #160
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.else
; 488 to 507
.if(n = 488)
	; 14 bytes
	php
	pha
	lda #161
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 489)
	; 14 bytes
	php
	pha
	lda #162
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 490)
	; 14 bytes
	php
	pha
	lda #163
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 491)
	; 14 bytes
	php
	pha
	lda #164
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 492)
	; 14 bytes
	php
	pha
	lda #165
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 493)
	; 14 bytes
	php
	pha
	lda #166
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 494)
	; 14 bytes
	php
	pha
	lda #167
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 495)
	; 14 bytes
	php
	pha
	lda #168
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 496)
	; 14 bytes
	php
	pha
	lda #169
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 497)
	; 14 bytes
	php
	pha
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 498)
	; 14 bytes
	php
	pha
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 499)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 500)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 501 || n = 502)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 503)
	; 16 bytes
	php
	pha
	php
	lda #169
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
	pla
	plp
.elseif(n = 504)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
	pla
	plp
.elseif(n = 505)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
	pla
	plp
.elseif(n = 506)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 507)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 508) && (n < 527))
; 508 to 526
.if(n = 508)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 509)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 510)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 511)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 512)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 513)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 514)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 515)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif((n > 515 && n < 519))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 519)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 520)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 521)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 522)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 523)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 524)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 525)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #n/3-6
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 526)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #(n-16)/3
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.endif
.else
; 527 to 546
.if(n = 527)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 528)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 529)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #(n-16)/3
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 530)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 531)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #n/3-6
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 532)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 533)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 534)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 535)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 536)
	; 9 bytes
	php
	tax
	lda #2
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 537)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 538 || n = 540 || n = 543 || n = 545 || n = 546)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 539)
	; 9 bytes
	php
	pha
	lda #2
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 541)
	; 9 bytes
	php
	tax
	lda #2
	jsr delay_256a_16_clocks_b
	txa
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
.endif
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
.elseif(n = 548 || n = 553 || n = 560 || n = 561)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 549)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 550)
	; 11 bytes
	php
	pha
	inc $F6,X
	lda #2
	jsr delay_256a_16_clocks_b
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
.elseif(n = 552 || n = 554 || n = 557)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 555 || n = 558)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 556)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 559)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 562 || n = 565 || n = 567 || n = 570)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 563)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 564)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 566)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 568 || n = 569 || n = 571 || n = 572)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 572 && n < 626))
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
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if((n > 625 && n < 704))
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
.if((n >= 704) && (n < 743))
; 704 to 742
.if((n > 703 && n < 743))
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
.if((n >= 743) && (n < 762))
; 743 to 761
.if(n = 743 || n = 744)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 745)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 746)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-236)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 747 || n = 748)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 749)
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
.elseif(n = 750)
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
.elseif(n = 751)
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
.elseif(n = 752)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 753)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 754)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 755)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 756)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 757)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 758)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 759)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 760)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 761)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 762 to 781
.if((n > 761 && n < 765))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 765)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 766)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 767)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 768)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 769)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 770)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 771)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 772)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 773)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 774)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 775)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 776)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 777)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 778)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 779)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 780)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 781)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 782) && (n < 860))
.if((n >= 782) && (n < 821))
.if((n >= 782) && (n < 801))
; 782 to 800
.if(n = 782)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 783)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 784)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 785)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 786)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 787)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 788)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 789)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 790)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 791)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 792)
	; 9 bytes
	php
	tax
	lda #3
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 793)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 794 || n = 796 || n = 799)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 795)
	; 9 bytes
	php
	pha
	lda #3
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 797)
	; 9 bytes
	php
	tax
	lda #3
	jsr delay_256a_16_clocks_b
	txa
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
.endif
.else
; 801 to 820
.if(n = 801 || n = 802 || n = 804 || n = 809 || n = 816 || n = 817)
	; 10..13 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 806)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 808 || n = 810 || n = 813)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 811 || n = 814)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 812)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 815)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 818)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 819)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 820)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.else
; 821 to 859
.if(n = 821 || n = 823 || n = 826)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 822)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 824 || n = 825 || n = 827 || n = 828)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 828 && n < 860))
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
; 860 to 937
.if((n > 859 && n < 938))
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
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if((n > 937 && n < 1001))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1001)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-236)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1002)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-237)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1003 || n = 1004)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1005)
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
.elseif(n = 1006)
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
.elseif(n = 1007)
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
.elseif(n = 1008)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1009)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1010)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1011)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1012)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1013)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1014)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1015)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 1016) && (n < 1055))
.if((n >= 1016) && (n < 1035))
; 1016 to 1034
.if(n = 1016)
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
.elseif(n = 1017)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 1017 && n < 1021))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1021)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1022)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1023)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1024)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1025)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1026)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1027)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1028)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #(n-8)/6
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1029)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1030)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1031)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1032)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1033)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1034)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #(n-8)/6
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 1035 to 1054
.if(n = 1035)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1036)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1037)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1038)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1039)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1040)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1041)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1042)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1043)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1044)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1045)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1046)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1047)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1048)
	; 9 bytes
	php
	tax
	lda #4
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 1049)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1050 || n = 1052)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1051)
	; 9 bytes
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1053)
	; 9 bytes
	php
	tax
	lda #4
	jsr delay_256a_16_clocks_b
	txa
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
.endif
.endif
.else
; 1055 to 1093
.if(n = 1055 || n = 1057 || n = 1058 || n = 1060 || n = 1065 || n = 1072 || n = 1073)
	; 10..13 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1062)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 1064 || n = 1066 || n = 1069)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1067 || n = 1070)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1068)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1071)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1074 || n = 1077 || n = 1079 || n = 1082)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1075)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1076)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1078)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 1080 || n = 1081 || n = 1083 || n = 1084)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 1084 && n < 1094))
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
; 1094 to 1250
.if((n > 1093 && n < 1251))
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
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
.if((n >= 1251) && (n < 1290))
.if((n >= 1251) && (n < 1270))
; 1251 to 1269
.if((n > 1250 && n < 1257))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1257)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-237)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1258)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-238)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1259 || n = 1260)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1261)
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
.elseif(n = 1262)
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
.elseif(n = 1263)
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
.elseif(n = 1264)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1265)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1266)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1267)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1268)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1269)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 1270 to 1289
.if(n = 1270)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1271)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1272)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1273)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 1273 && n < 1277))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1277)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1278)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1279)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1280)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1281)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1282)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1283)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1284)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1285)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1286)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1287)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1288)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1289)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 1290) && (n < 1309))
; 1290 to 1308
.if(n = 1290)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1291)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1292)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1293)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1294)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1295)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1296)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1297)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1298)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1299)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1300)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1301)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1302)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1303)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1304)
	; 9 bytes
	php
	tax
	lda #5
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 1305)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1306 || n = 1308)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1307)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 1309 to 1328
.if(n = 1309)
	; 9 bytes
	php
	tax
	lda #5
	jsr delay_256a_16_clocks_b
	txa
	plp
.elseif(n = 1310)
	; 11 bytes
	php
	pha
	lda $A5
	lda #5
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1311 || n = 1313 || n = 1314 || n = 1316 || n = 1321 || n = 1328)
	; 10..13 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1318)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 1320 || n = 1322 || n = 1325)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1323 || n = 1326)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1324)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1327)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.else
; 1329 to 1406
.if(n = 1329)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1330 || n = 1333 || n = 1335 || n = 1338)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1331)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1332)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1334)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 1336 || n = 1337 || n = 1339 || n = 1340)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 1340 && n < 1407))
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
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if((n > 1406 && n < 1485))
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
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if((n > 1484 && n < 1513))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1513)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1514)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1515 || n = 1516)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1517)
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
.elseif(n = 1518)
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
.elseif(n = 1519)
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
.elseif(n = 1520)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1521)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1522)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1523)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 1524) && (n < 1543))
; 1524 to 1542
.if(n = 1524)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1525)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1526)
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
.elseif(n = 1527)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1528)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1529)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 1529 && n < 1533))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1533)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1534)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1535)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1536)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1537)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1538)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1539)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1540)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #(n-10)/9
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1541)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1542)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 1543 to 1562
.if(n = 1543)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1544)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1545)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1546)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1547)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1548)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1549)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #(n-10)/9
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1550)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1551)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1552)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1553)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1554)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1555)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1556)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1557)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1558)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1559)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1560)
	; 9 bytes
	php
	tax
	lda #6
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 1561)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1562)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1564 || n = 1567 || n = 1569 || n = 1570 || n = 1572 || n = 1577 || n = 1584 || n = 1585)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565)
	; 9 bytes
	php
	tax
	lda #6
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1574)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 1576 || n = 1578 || n = 1581)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1579 || n = 1582)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1580)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1583)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1586 || n = 1589 || n = 1591 || n = 1594)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1587)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1588)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1590)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 1592 || n = 1593 || n = 1595 || n = 1596)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 1596 && n < 1719))
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
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if((n > 1718 && n < 1758))
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
.if((n >= 1758) && (n < 1777))
; 1758 to 1776
.if((n > 1757 && n < 1769))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1769)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-239)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1770)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-240)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1771 || n = 1772)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1773)
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
.elseif(n = 1774)
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
.elseif(n = 1775)
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
.elseif(n = 1776)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 1777 to 1796
.if(n = 1777)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1778)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1779)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1780)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1781)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1782)
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
.elseif(n = 1783)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1784)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1785)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 1785 && n < 1789))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1789)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1790)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1791)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1792)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1793)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1794)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1795)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1796)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
.if((n >= 1797) && (n < 1816))
; 1797 to 1815
.if(n = 1797)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1798)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1799)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1800)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1801)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1802)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1803)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1804)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1805)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1806)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1807)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1808)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1809)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1810)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1811)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1812)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1813)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1814)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1815)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 1816 to 1835
.if(n = 1816)
	; 9 bytes
	php
	tax
	lda #7
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 1817)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1818 || n = 1820 || n = 1823 || n = 1825 || n = 1826 || n = 1828 || n = 1833)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1819)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1821)
	; 9 bytes
	php
	tax
	lda #7
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1830)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 1832 || n = 1834)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1835)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1837)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1838)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1839)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1840 || n = 1841)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1842 || n = 1845 || n = 1847 || n = 1850)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1843)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1844)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1846)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 1848 || n = 1849 || n = 1851 || n = 1852)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 1852 && n < 1876))
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
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
; 1876 to 2031
.if((n > 1875 && n < 2025))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2025)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2026)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2027 || n = 2028)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2029)
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
.elseif(n = 2030)
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
.elseif(n = 2031)
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
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2033)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2034)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2035)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2036)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2037)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2038)
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
.elseif(n = 2039)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2040)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2041)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 2041 && n < 2045))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2045)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2046)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2047)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2048)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2049)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2050)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 2051 to 2070
.if(n = 2051)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2052)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2053)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2054)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2055)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2056)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2057)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2058)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2059)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2060)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2061)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2062)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2063)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2064)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2065)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2066)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2067)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2068)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2069)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2070)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 2071 to 2109
.if(n = 2071)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2072)
	; 9 bytes
	php
	tax
	lda #8
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 2073)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2074 || n = 2076 || n = 2079 || n = 2081 || n = 2082 || n = 2084 || n = 2089 || n = 2096 || n = 2097)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2075)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2077)
	; 9 bytes
	php
	tax
	lda #8
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2086)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 2088 || n = 2090 || n = 2093)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2091 || n = 2094)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2092)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2095)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2098 || n = 2101 || n = 2103 || n = 2106)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2099)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2100)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2102)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 2104 || n = 2105 || n = 2107 || n = 2108)
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
.else
; 2110 to 2187
.if((n > 2109 && n < 2188))
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
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if((n > 2187 && n < 2266))
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
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if((n > 2265 && n < 2281))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2281)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-241)/255
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
	lda #(n-242)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2283 || n = 2284)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2285)
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
.elseif(n = 2286)
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
.elseif(n = 2287)
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
.elseif(n = 2288)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2289)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2290)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2291)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2292)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2293)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2294)
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
.elseif(n = 2295)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2296)
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
.elseif(n = 2297)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 2297 && n < 2301))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2301)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2302)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2303)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2304)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2306)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2307)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2308)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2309)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2310)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2311)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2312)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2313)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2314)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2315)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2316)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2317)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2318)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2319)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2320)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2321)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2322)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2323)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 2324 to 2343
.if(n = 2324)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2325)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2326)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2327)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2328)
	; 9 bytes
	php
	tax
	lda #9
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 2329)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2330 || n = 2332 || n = 2335 || n = 2337 || n = 2338 || n = 2340)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2331)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2333)
	; 9 bytes
	php
	tax
	lda #9
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2342)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.if(n = 2344 || n = 2346 || n = 2349)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2345 || n = 2352 || n = 2353)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347 || n = 2350)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2348)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2351)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2354 || n = 2357 || n = 2359 || n = 2362)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2355)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2356)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2358)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 2360 || n = 2361 || n = 2363 || n = 2364)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 2364 && n < 2501))
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
.if((n >= 2501) && (n < 3751))
.if((n >= 2501) && (n < 3126))
.if((n >= 2501) && (n < 2813))
.if((n >= 2501) && (n < 2657))
.if((n >= 2501) && (n < 2579))
.if((n >= 2501) && (n < 2540))
; 2501 to 2539
.if((n > 2500 && n < 2537))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2537)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-242)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2538)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-243)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2539)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2541)
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
.elseif(n = 2542)
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
.elseif(n = 2543)
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
.elseif(n = 2544)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2545)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2546)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2547)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2548)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2549)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2550)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2551)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2552)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2553)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 2553 && n < 2557))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2557)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2558)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 2559 to 2578
.if(n = 2559)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2560)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2561)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2562)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2563)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2564)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2565)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2566)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2567)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2568)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2569)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2570)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2571)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2572)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2573)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2574)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2575)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2576)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2577)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2578)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2580)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2581)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2582)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2583)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2584)
	; 9 bytes
	php
	tax
	lda #10
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 2585)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2586 || n = 2588 || n = 2591 || n = 2593 || n = 2594 || n = 2596 || n = 2601 || n = 2608 || n = 2609)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2587)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2589)
	; 9 bytes
	php
	tax
	lda #10
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2598)
	; 11 bytes
	php
	pha
	inc $F6,X
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 2600 || n = 2602 || n = 2605)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2603 || n = 2606)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2604)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2607)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2610 || n = 2613 || n = 2615)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2611)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2612)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2614)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 2616 || n = 2617)
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
.else
; 2618 to 2656
.if(n = 2618)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2619 || n = 2620)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 2620 && n < 2657))
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
; 2657 to 2812
.if((n > 2656 && n < 2793))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2793)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 2795 || n = 2796)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2799)
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
.elseif(n = 2800)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2801)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2802)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2803)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2804)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2805)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2806)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2807)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2808)
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
.elseif(n = 2809)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 2809 && n < 2813))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
.if((n >= 2813) && (n < 2832))
; 2813 to 2831
.if(n = 2813)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2814)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2815)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2816)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2817)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2818)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2819)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2820)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2821)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2822)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2823)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2824)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2825)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2826)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2827)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2828)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2829)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2830)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2831)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 2832 to 2851
.if(n = 2832)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2833)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2834)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2835)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2836)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2837)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2838)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2839)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2840)
	; 9 bytes
	php
	tax
	lda #11
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 2841)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2842 || n = 2844 || n = 2847 || n = 2849 || n = 2850)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2843)
	; 9 bytes
	php
	pha
	lda #11
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2845)
	; 9 bytes
	php
	tax
	lda #11
	jsr delay_256a_16_clocks_b
	txa
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
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2857 || n = 2864 || n = 2865)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2853)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2854)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 2856 || n = 2858 || n = 2861)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2859 || n = 2862)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2860)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2863)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2866 || n = 2869 || n = 2871 || n = 2874)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2867)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2868)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2870)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 2872 || n = 2873 || n = 2875 || n = 2876)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 2876 && n < 2891))
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
; 2891 to 2968
.if((n > 2890 && n < 2969))
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
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n > 2968 && n < 3047))
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
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
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
.elseif(n = 3053)
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
.elseif(n = 3054)
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
.elseif(n = 3055)
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
.elseif(n = 3056)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3057)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3058)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3059)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3060)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3061)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3062)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3063)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3064)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3065)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 3066 to 3085
.if((n > 3065 && n < 3069))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3069)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3070)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3071)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3072)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3073)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3074)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3075)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3076)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3077)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3078)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3079)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3080)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3081)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3082)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3083)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3084)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3085)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 3086) && (n < 3106))
; 3086 to 3105
.if(n = 3086)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3087)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3088)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3089)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3090)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3091)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3092)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3093)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3094)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3095)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3096)
	; 9 bytes
	php
	tax
	lda #12
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 3097)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3098 || n = 3100 || n = 3103 || n = 3105)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3099)
	; 9 bytes
	php
	pha
	lda #12
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3101)
	; 9 bytes
	php
	tax
	lda #12
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 3106 || n = 3108 || n = 3113 || n = 3120 || n = 3121)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3110)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 3112 || n = 3114 || n = 3117)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3115 || n = 3118)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3116)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3119)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3122 || n = 3125)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3123)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3124)
	; 13 bytes
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
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 3127 || n = 3130)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3128 || n = 3129 || n = 3131 || n = 3132)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 3132 && n < 3282))
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
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if((n > 3281 && n < 3305))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3307 || n = 3308)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3309)
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
.elseif(n = 3312)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3313)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3314)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3315)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3316)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3317)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3318)
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
.elseif(n = 3319)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3320)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 3321) && (n < 3340))
; 3321 to 3339
.if(n = 3321)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 3321 && n < 3325))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3325)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3326)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3327)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3328)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3329)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3330)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3331)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3332)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3333)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3334)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3335)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3336)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3337)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3338)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3339)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 3340 to 3359
.if(n = 3340)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3341)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3342)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3343)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3344)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3345)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3346)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3347)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3348)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3349)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3350)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3351)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3352)
	; 9 bytes
	php
	tax
	lda #13
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 3353)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3354 || n = 3356 || n = 3359)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3355)
	; 9 bytes
	php
	pha
	lda #13
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3357)
	; 9 bytes
	php
	tax
	lda #13
	jsr delay_256a_16_clocks_b
	txa
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
.elseif(n = 3361 || n = 3362 || n = 3364 || n = 3369 || n = 3376 || n = 3377)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3366)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 3368 || n = 3370 || n = 3373)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3371 || n = 3374)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3372)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3375)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3378 || n = 3381 || n = 3383 || n = 3386)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3379)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3380)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 3382)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 3384 || n = 3385 || n = 3387 || n = 3388)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 3388 && n < 3438))
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
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if((n > 3437 && n < 3516))
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
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3562)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3563 || n = 3564)
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3569)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3570)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3571)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3572)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3573)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 3574 to 3593
.if(n = 3574)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3575)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3576)
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
.elseif(n = 3577)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 3577 && n < 3581))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3581)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3582)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3583)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3584)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3585)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3586)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3587)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3588)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3589)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3590)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3591)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3592)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3593)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3595)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3596)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3597)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3598)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3599)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3600)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3601)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3602)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3603)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3604)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3605)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3606)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3607)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3608)
	; 9 bytes
	php
	tax
	lda #14
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 3609)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3610 || n = 3612)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
.if(n = 3613)
	; 9 bytes
	php
	tax
	lda #14
	jsr delay_256a_16_clocks_b
	txa
	plp
.elseif(n = 3614)
	; 11 bytes
	php
	pha
	lda $A5
	lda #14
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3615 || n = 3617 || n = 3618 || n = 3620 || n = 3625 || n = 3632)
	; 10..13 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3622)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 3624 || n = 3626 || n = 3629)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3627 || n = 3630)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3628)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3631)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.else
; 3633 to 3671
.if(n = 3633)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3634 || n = 3637 || n = 3639 || n = 3642)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3635)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3636)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 3638)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 3640 || n = 3641 || n = 3643 || n = 3644)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 3644 && n < 3672))
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
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/255
	jsr delay_256a_16_clocks_b
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
.elseif(n = 3819 || n = 3820)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3821)
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
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3825)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3826)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3827)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3828)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 3829) && (n < 3868))
.if((n >= 3829) && (n < 3848))
; 3829 to 3847
.if(n = 3829)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3830)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3831)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3832)
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
.elseif(n = 3833)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 3833 && n < 3837))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3837)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3838)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3839)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3840)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3841)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3842)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3843)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3844)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3845)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3846)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3847)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 3848 to 3867
.if(n = 3848)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3849)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3850)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3851)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3852)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3853)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3854)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3855)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3856)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3857)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3858)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3859)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3860)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3861)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3862)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3863)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3864)
	; 9 bytes
	php
	tax
	lda #15
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 3865)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3866)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3867)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3871 || n = 3873 || n = 3874 || n = 3876 || n = 3881 || n = 3888 || n = 3889)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3869)
	; 9 bytes
	php
	tax
	lda #15
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3878)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 3880 || n = 3882 || n = 3885)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3883 || n = 3886)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3884)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3887)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3890 || n = 3893 || n = 3895 || n = 3898)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3891)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3892)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 3894)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 3896 || n = 3897 || n = 3899 || n = 3900)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 3900 && n < 3907))
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
; 3907 to 4062
.if((n > 3906 && n < 4063))
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
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
.if((n >= 4063) && (n < 4082))
; 4063 to 4081
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
.elseif(n = 4080)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4081)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 4082 to 4101
.if(n = 4082)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4083)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4084)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4085)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4086)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4087)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4088)
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
.elseif(n = 4089)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 4089 && n < 4093))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4093)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4094)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4095)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4096)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4097)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4098)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4099)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4100)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4101)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 4102) && (n < 4121))
; 4102 to 4120
.if(n = 4102)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4103)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4104)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4105)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4106)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4107)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4108)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4109)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4110)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4111)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4112)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4113)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4114)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4115)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4116)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4117)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4118)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4119)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4120)
	; 9 bytes
	php
	tax
	lda #16
	jsr delay_256a_11_clocks_b_
	txa
	plp
.endif
.else
; 4121 to 4140
.if(n = 4121)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4122 || n = 4124 || n = 4127 || n = 4129 || n = 4130 || n = 4132 || n = 4137)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4123)
	; 9 bytes
	php
	pha
	lda #16
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4125)
	; 9 bytes
	php
	tax
	lda #16
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4134)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 4136 || n = 4138)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4139)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4140)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4142)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4143)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4144 || n = 4145)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4146 || n = 4149 || n = 4151 || n = 4154)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4147)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4148)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4150)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 4152 || n = 4153 || n = 4155 || n = 4156)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 4156 && n < 4219))
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
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-250)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4331 || n = 4332)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 4335)
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
.if((n >= 4336) && (n < 4356))
; 4336 to 4355
.if(n = 4336)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4337)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4338)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4339)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4340)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4341)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4342)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4343)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4344)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4345)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 4345 && n < 4349))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4349)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4350)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4351)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4352)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4353)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4354)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4355)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 4356 to 4375
.if(n = 4356)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4357)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4358)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4359)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4360)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4361)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4362)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4363)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4364)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4365)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4366)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4367)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4368)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4369)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4370)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4371)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4372)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4373)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4374)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4375)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
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
; 4376 to 4531
.if(n = 4376)
	; 9 bytes
	php
	tax
	lda #17
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 4377)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4378 || n = 4380 || n = 4383 || n = 4385 || n = 4386 || n = 4388 || n = 4393 || n = 4400 || n = 4401)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4379)
	; 9 bytes
	php
	pha
	lda #17
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4381)
	; 9 bytes
	php
	tax
	lda #17
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4390)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 4392 || n = 4394 || n = 4397)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4395 || n = 4398)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4396)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4399)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4402 || n = 4405 || n = 4407 || n = 4410)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4403)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4404)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4406)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 4408 || n = 4409 || n = 4411 || n = 4412)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 4412 && n < 4532))
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
	lda #(n-250)/255
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4593)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4594)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4595)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4596)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4597)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4598)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4599)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4600)
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
.elseif(n = 4601)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 4601 && n < 4605))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4605)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4606)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4607)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4608)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4609)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 4610) && (n < 4649))
.if((n >= 4610) && (n < 4629))
; 4610 to 4628
.if(n = 4610)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4611)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4612)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4613)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4614)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4615)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4616)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4617)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4618)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4619)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4620)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4621)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4622)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4623)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4624)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4625)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4626)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4627)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4628)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 4629 to 4648
.if(n = 4629)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4630)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4631)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4632)
	; 9 bytes
	php
	tax
	lda #18
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 4633)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4634 || n = 4636 || n = 4639 || n = 4641 || n = 4642 || n = 4644)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4635)
	; 9 bytes
	php
	pha
	lda #18
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4637)
	; 9 bytes
	php
	tax
	lda #18
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4646)
	; 11 bytes
	php
	pha
	inc $F6,X
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
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4656 || n = 4657)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4650 || n = 4653)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4651 || n = 4654)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4652)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4655)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4658 || n = 4661 || n = 4663 || n = 4666)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4659)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4660)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4662)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 4664 || n = 4665 || n = 4667 || n = 4668)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 4668 && n < 4688))
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
	; 16 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 4848)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4849)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4850)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4851)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4852)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4853)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4854)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4855)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4856)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4857)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 4857 && n < 4861))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4861)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4862)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 4863 to 4882
.if(n = 4863)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4864)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4865)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4866)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4867)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4868)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4869)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4870)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4871)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4872)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4873)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4874)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4875)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4876)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4877)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4878)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4879)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4880)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4881)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4882)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4884)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4885)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4886)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4887)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4888)
	; 9 bytes
	php
	tax
	lda #19
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 4889)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4890 || n = 4892 || n = 4895 || n = 4897 || n = 4898 || n = 4900 || n = 4905 || n = 4912 || n = 4913)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4891)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4893)
	; 9 bytes
	php
	tax
	lda #19
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4902)
	; 11 bytes
	php
	pha
	inc $F6,X
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 4904 || n = 4906 || n = 4909)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4907 || n = 4910)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4908)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4911)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4914 || n = 4917 || n = 4919)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4915)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4916)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4918)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 4920 || n = 4921)
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
; 4922 to 5000
.if(n = 4922)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4923 || n = 4924)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 4924 && n < 5001))
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
.elseif(n = 5099 || n = 5100)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5101)
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5105)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5106)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5107)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5108)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5109)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5110)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5111)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5112)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5113)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 5113 && n < 5117))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5117)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 5118) && (n < 5137))
; 5118 to 5136
.if(n = 5118)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5119)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5120)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5121)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5122)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5123)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5124)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5125)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5126)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5127)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5128)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5129)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5130)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5131)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5132)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5133)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5134)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5135)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5136)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 5137 to 5156
.if(n = 5137)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5138)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5139)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5140)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5141)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5142)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5143)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5144)
	; 9 bytes
	php
	tax
	lda #20
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 5145)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5146 || n = 5148 || n = 5151 || n = 5153 || n = 5154 || n = 5156)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5147)
	; 9 bytes
	php
	pha
	lda #20
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5149)
	; 9 bytes
	php
	tax
	lda #20
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5158)
	; 11 bytes
	php
	pha
	inc $F6,X
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 5160 || n = 5162 || n = 5165)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5161 || n = 5168 || n = 5169)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5163 || n = 5166)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5164)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5167)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5170 || n = 5173 || n = 5175 || n = 5178)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5171)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5172)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5174)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 5176 || n = 5177 || n = 5179 || n = 5180)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 5180 && n < 5313))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5361)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5362)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5363)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5364)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5365)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5368)
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
.elseif(n = 5369)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5370)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 5371 to 5390
.if(n = 5371 || n = 5372)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5373)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5374)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5375)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5376)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5377)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5378)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5379)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5380)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5381)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5382)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5383)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5384)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5385)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5386)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5387)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5388)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5389)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5390)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5392)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5393)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5394)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5395)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5396)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5397)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5398)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5399)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5400)
	; 9 bytes
	php
	tax
	lda #21
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 5401)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5402 || n = 5404 || n = 5407 || n = 5409)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5403)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5405)
	; 9 bytes
	php
	tax
	lda #21
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 5410 || n = 5412 || n = 5417 || n = 5424 || n = 5425)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5414)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 5416 || n = 5418 || n = 5421)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5419 || n = 5422)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5420)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5423)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5426 || n = 5429)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5427)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5428)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.else
; 5430 to 5468
.if(n = 5430)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 5431 || n = 5434)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5432 || n = 5433 || n = 5435 || n = 5436)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 5436 && n < 5469))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5617)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5618)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5619)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5620)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5621)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5622)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5623)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5624)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5625)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
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
.if((n > 5625 && n < 5629))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5629)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5630)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5631)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5632)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5633)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5634)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5635)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5636)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5637)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5638)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5639)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5640)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5641)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5642)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5643)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5644)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 5645 to 5664
.if(n = 5645)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5646)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5647)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5648)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5649)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5650)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5651)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5652)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5653)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5654)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5655)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5656)
	; 9 bytes
	php
	tax
	lda #22
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 5657)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5658 || n = 5660 || n = 5663)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5659)
	; 9 bytes
	php
	pha
	lda #22
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5661)
	; 9 bytes
	php
	tax
	lda #22
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 5665 || n = 5666 || n = 5668 || n = 5673 || n = 5680 || n = 5681)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5670)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 5672 || n = 5674 || n = 5677)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5675 || n = 5678)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5676)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5679)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5682 || n = 5685 || n = 5687 || n = 5690)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5683)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5684)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5686)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 5688 || n = 5689 || n = 5691 || n = 5692)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 5692 && n < 5704))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5873)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5874)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5875)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5876)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5877)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5880)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5881)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 5881 && n < 5885))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5885)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5886)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5887)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5888)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5889)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5890)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5891)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5892)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5893)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5894)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5895)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5896)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5897)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5898)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 5899) && (n < 5918))
; 5899 to 5917
.if(n = 5899)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5900)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5901)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5902)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5903)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5904)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5905)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5906)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5907)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5908)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5909)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5910)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5911)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5912)
	; 9 bytes
	php
	tax
	lda #23
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 5913)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5914 || n = 5916)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5915)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5917)
	; 9 bytes
	php
	tax
	lda #23
	jsr delay_256a_16_clocks_b
	txa
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
.elseif(n = 5919 || n = 5921 || n = 5922 || n = 5924 || n = 5929 || n = 5936 || n = 5937)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5926)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 5928 || n = 5930 || n = 5933)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5931 || n = 5934)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5932)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5935)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if(n = 5938 || n = 5941 || n = 5943 || n = 5946)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5939)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5940)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5942)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 5944 || n = 5945 || n = 5947 || n = 5948)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 5948 && n < 6094))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6129)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6130)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6131)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6132)
	; 16 bytes
	php
	pha
	brk
	lda #170
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
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6136)
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
.elseif(n = 6137)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 6137 && n < 6141))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6141)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6142)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6143)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6144)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6145)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6146)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6147)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6148)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6149)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6150)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6151)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 6152 to 6171
.if(n = 6152)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6153)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6154)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6155)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6156)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6157)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6158)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6159)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6160)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6161)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6162)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6163)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6164)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6165)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6166)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6167)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6168)
	; 9 bytes
	php
	tax
	lda #24
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 6169)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6170)
	; 10 bytes
	delay_n (n-2)
	nop
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
.if(n = 6172 || n = 6175 || n = 6177 || n = 6178 || n = 6180 || n = 6185 || n = 6192 || n = 6193)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6173)
	; 9 bytes
	php
	tax
	lda #24
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6182)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 6184 || n = 6186 || n = 6189)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6187 || n = 6190)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6188)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6191)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6194 || n = 6197 || n = 6199 || n = 6202)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6195)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6196)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6198)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 6200 || n = 6201 || n = 6203 || n = 6204)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 6204 && n < 6251))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6385)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6386)
	; 16 bytes
	php
	pha
	brk
	lda #168
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
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6388)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6389)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
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
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6392)
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
.elseif(n = 6393)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 6393 && n < 6397))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6397)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6398)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6399)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6400)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6401)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6402)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6403)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6404)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6405)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6406)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6408)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6409)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6410)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6411)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6412)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6413)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6414)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6415)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6416)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6417)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6418)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6419)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6420)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6421)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6422)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6423)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6424)
	; 9 bytes
	php
	tax
	lda #25
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 6425)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 6426 to 6445
.if(n = 6426 || n = 6428 || n = 6431 || n = 6433 || n = 6434 || n = 6436 || n = 6441)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6427)
	; 9 bytes
	php
	pha
	lda #25
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6429)
	; 9 bytes
	php
	tax
	lda #25
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6438)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 6440 || n = 6442 || n = 6445)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6443)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6448 || n = 6449)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6450 || n = 6453 || n = 6455 || n = 6458)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6451)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6452)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6454)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 6456 || n = 6457 || n = 6459 || n = 6460)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 6460 && n < 6485))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 6641) && (n < 6680))
.if((n >= 6641) && (n < 6660))
; 6641 to 6659
.if(n = 6641)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6642)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6643)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6644)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6645)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6646)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6647)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 6649 && n < 6653))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6653)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6654)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6655)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6656)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6657)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6658)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6659)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 6660 to 6679
.if(n = 6660)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6661)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6662)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6663)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6664)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6665)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6666)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6667)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6668)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6669)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6670)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6671)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6672)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6673)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6674)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6675)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6676)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6677)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6678)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6679)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 6680 to 6718
.if(n = 6680)
	; 9 bytes
	php
	tax
	lda #26
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 6681)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6682 || n = 6684 || n = 6687 || n = 6689 || n = 6690 || n = 6692 || n = 6697 || n = 6704 || n = 6705)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6683)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6685)
	; 9 bytes
	php
	tax
	lda #26
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6694)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 6696 || n = 6698 || n = 6701)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6699 || n = 6702)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6700)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6703)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6706 || n = 6709 || n = 6711 || n = 6714)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6707)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6708)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6710)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 6712 || n = 6713 || n = 6715 || n = 6716)
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
.else
; 6719 to 6875
.if((n > 6718 && n < 6876))
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
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
.if((n >= 6876) && (n < 6915))
; 6876 to 6914
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
.elseif(n = 6895)
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6897)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6898)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6899)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6900)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6901)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6902)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6903)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6904)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6905)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 6905 && n < 6909))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6909)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6910)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6911)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6912)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6913)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6914)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 6915) && (n < 6934))
; 6915 to 6933
.if(n = 6915)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6916)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6917)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6918)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6919)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6920)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6921)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6922)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6923)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6924)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6925)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6926)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6927)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6928)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6929)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6930)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6931)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6932)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6933)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 6934 to 6953
.if(n = 6934)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6935)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6936)
	; 9 bytes
	php
	tax
	lda #27
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 6937)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6938 || n = 6940 || n = 6943 || n = 6945 || n = 6946 || n = 6948 || n = 6953)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6939)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6941)
	; 9 bytes
	php
	tax
	lda #27
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6950)
	; 11 bytes
	php
	pha
	inc $F6,X
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
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954 || n = 6957)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6955 || n = 6958)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6956)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6959)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6960 || n = 6961)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6962 || n = 6965 || n = 6967 || n = 6970)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6963)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6964)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6966)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 6968 || n = 6969 || n = 6971 || n = 6972)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 6972 && n < 7032))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7153)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7154)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7155)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7156)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7157)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7158)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7159)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7160)
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
.elseif(n = 7161)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 7161 && n < 7165))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7165)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7166)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7167)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 7168 to 7187
.if(n = 7168)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7169)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7170)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7171)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7172)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7173)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7174)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7175)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7176)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7177)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7178)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7179)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7180)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7181)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7182)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7183)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7184)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7185)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7186)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7187)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
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
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7189)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7190)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7191)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7192)
	; 9 bytes
	php
	tax
	lda #28
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 7193)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7194 || n = 7196 || n = 7199 || n = 7201 || n = 7202 || n = 7204 || n = 7209 || n = 7216 || n = 7217)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 7195)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7197)
	; 9 bytes
	php
	tax
	lda #28
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7206)
	; 11 bytes
	php
	pha
	inc $F6,X
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7207)
	; 11 bytes
	php
	pha
	php
	lda #28
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7208 || n = 7210 || n = 7213)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7211 || n = 7214)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7212)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7215)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7218 || n = 7221 || n = 7223 || n = 7226)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7219)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7220)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7222)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 7224 || n = 7225 || n = 7227 || n = 7228)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 7228 && n < 7344))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7409)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7410)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7411)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7412)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7413)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
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
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 7417 && n < 7421))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7421)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
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
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7423)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7424)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7425)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7426)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7427)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7428)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7429)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7430)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7431)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7432)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7433)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7434)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7435)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7436)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7437)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7438)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7439)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7440)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 7441 to 7460
.if(n = 7441)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7442)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7443)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7444)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7445)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7446)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7447)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7448)
	; 9 bytes
	php
	tax
	lda #29
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 7449)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7450 || n = 7452 || n = 7455 || n = 7457 || n = 7458 || n = 7460)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7451)
	; 9 bytes
	php
	pha
	lda #29
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7453)
	; 9 bytes
	php
	tax
	lda #29
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7462)
	; 11 bytes
	php
	pha
	inc $F6,X
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 7464 || n = 7466 || n = 7469)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7465 || n = 7472 || n = 7473)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 7467 || n = 7470)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7468)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7471)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7474 || n = 7477 || n = 7479 || n = 7482)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7475)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7476)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7478)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 7480 || n = 7481 || n = 7483 || n = 7484)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 7484 && n < 7501))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7665)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7666)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7667)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7668)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7669)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7670)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7671)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7672)
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
.elseif(n = 7673)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7674 || n = 7675)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 7676 to 7695
.if(n = 7676)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7677)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7678)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7679)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7680)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7681)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7682)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7683)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7684)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7685)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7686)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7687)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7688)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7689)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7690)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7691)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7692)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7693)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7694)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7695)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 7696) && (n < 7715))
; 7696 to 7714
.if(n = 7696)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7697)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7698)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7699)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7700)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7701)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7702)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7703)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7704)
	; 9 bytes
	php
	tax
	lda #30
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 7705)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7706 || n = 7708 || n = 7711 || n = 7713 || n = 7714)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7707)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7709)
	; 9 bytes
	php
	tax
	lda #30
	jsr delay_256a_16_clocks_b
	txa
	plp
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
.elseif(n = 7716 || n = 7721 || n = 7728 || n = 7729)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 7717)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7718)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 7720 || n = 7722 || n = 7725)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7723 || n = 7726)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7724)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7727)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7730 || n = 7733)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7731)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7732)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7734)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.endif
.endif
.endif
.else
; 7735 to 7812
.if(n = 7735 || n = 7738)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7736 || n = 7737 || n = 7739 || n = 7740)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 7740 && n < 7813))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7921)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7922)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7923)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7924)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7925)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7926)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7927)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7928)
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
.elseif(n = 7929)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 7930) && (n < 7949))
; 7930 to 7948
.if((n > 7929 && n < 7933))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7933)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7934)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7935)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7936)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7937)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7938)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7939)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7940)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7941)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7942)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7943)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7944)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7945)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7946)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7947)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7948)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 7949 to 7968
.if(n = 7949)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7950)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7951)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7952)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7953)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7954)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7955)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7956)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7957)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7960)
	; 9 bytes
	php
	tax
	lda #31
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 7961)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7962 || n = 7964 || n = 7967)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7963)
	; 9 bytes
	php
	pha
	lda #31
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7965)
	; 9 bytes
	php
	tax
	lda #31
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 7969 || n = 7970 || n = 7972 || n = 7977 || n = 7984 || n = 7985)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7974)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 7976 || n = 7978 || n = 7981)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7979 || n = 7982)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7980)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7983)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7986 || n = 7989 || n = 7991 || n = 7994)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7987)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7988)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7990)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 7992 || n = 7993 || n = 7995 || n = 7996)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 7996 && n < 8126))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8177)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8178)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8179)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8180)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8181)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 8185 && n < 8189))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8189)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8190)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8191)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8192)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8193)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8194)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8195)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8196)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8197)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8198)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8199)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8200)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8201)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8202)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8203)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8205)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8206)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8207)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8208)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8209)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8210)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8211)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8212)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8213)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8214)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
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
	; 9 bytes
	php
	tax
	lda #32
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 8217)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8218 || n = 8220)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8219)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8221)
	; 9 bytes
	php
	tax
	lda #32
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 8223 || n = 8225 || n = 8226 || n = 8228 || n = 8233 || n = 8240 || n = 8241)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8230)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 8232 || n = 8234 || n = 8237)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 8235 || n = 8238)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8236)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8239)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8242)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.endif
.endif
.else
; 8243 to 8281
.if(n = 8243)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8244)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 8245 || n = 8247 || n = 8250)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8246)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 8248 || n = 8249 || n = 8251 || n = 8252)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 8252 && n < 8282))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8433)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8434)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8435)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8436)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8437)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8439)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8440)
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
.elseif(n = 8441)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 8441 && n < 8445))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8445)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8446)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8447)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8448)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8449)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8450)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8451)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8452)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8453)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8454)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8455)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8456)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 8457 to 8476
.if(n = 8457)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8458)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8459)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8460)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8461)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8462)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8463)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8464)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8465)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8466)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8467)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8468)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8469)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8470)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8471)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8472)
	; 9 bytes
	php
	tax
	lda #33
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 8473)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8474 || n = 8476)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8475)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 8477 to 8515
.if(n = 8477)
	; 9 bytes
	php
	tax
	lda #33
	jsr delay_256a_16_clocks_b
	txa
	plp
.elseif(n = 8478)
	; 11 bytes
	php
	pha
	lda $A5
	lda #33
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8479 || n = 8481 || n = 8482 || n = 8484 || n = 8489 || n = 8496 || n = 8497)
	; 10..13 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8486)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 8488 || n = 8490 || n = 8493)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 8491 || n = 8494)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8492)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8495)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8498 || n = 8501 || n = 8503 || n = 8506)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8499)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8500)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 8502)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 8504 || n = 8505 || n = 8507 || n = 8508)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 8508 && n < 8516))
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
; 8516 to 8593
.if((n > 8515 && n < 8594))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8689)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8690)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 8691 to 8710
.if(n = 8691)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8692)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8693)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 8697 && n < 8701))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8701)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8702)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8703)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8704)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8705)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8706)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8707)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8708)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8709)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8710)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 8711) && (n < 8731))
; 8711 to 8730
.if(n = 8711)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8712)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8713)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8714)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8715)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8716)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8717)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8718)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8719)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8720)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8721)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8724)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8727)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8728)
	; 9 bytes
	php
	tax
	lda #34
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 8729)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8730)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8732 || n = 8735 || n = 8737 || n = 8738 || n = 8740 || n = 8745)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8733)
	; 9 bytes
	php
	tax
	lda #34
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8742)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 8744 || n = 8746 || n = 8749)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 8747 || n = 8750)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8748)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8752 || n = 8753)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 8754 || n = 8757 || n = 8759 || n = 8762)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8755)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8756)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 8758)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 8760 || n = 8761 || n = 8763 || n = 8764)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 8764 && n < 8907))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8945)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 8946) && (n < 8965))
; 8946 to 8964
.if(n = 8946)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8947)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8948)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8949)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8952)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8953)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 8953 && n < 8957))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8957)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8958)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8959)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8960)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8961)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8962)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8963)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8964)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 8965 to 8984
.if(n = 8965)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8966)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8967)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8968)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8969)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8970)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8971)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8972)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8973)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8974)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8975)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8976)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8977)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8978)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8979)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8980)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8981)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8984)
	; 9 bytes
	php
	tax
	lda #35
	jsr delay_256a_11_clocks_b_
	txa
	plp
.endif
.endif
.endif
.else
; 8985 to 9062
.if(n = 8985)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8986 || n = 8988 || n = 8991 || n = 8993 || n = 8994 || n = 8996 || n = 9001 || n = 9008 || n = 9009)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 8987)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8989)
	; 9 bytes
	php
	tax
	lda #35
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8998)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 9000 || n = 9002 || n = 9005)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9003 || n = 9006)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9004)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9007)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9010 || n = 9013 || n = 9015 || n = 9018)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9011)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9012)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9014)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 9016 || n = 9017 || n = 9019 || n = 9020)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 9020 && n < 9063))
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
.if((n >= 9063) && (n < 9219))
; 9063 to 9218
.if((n > 9062 && n < 9193))
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
.elseif(n = 9199)
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9201)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9202)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9203)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9204)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9205)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9206)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9207)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9208)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9209)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 9209 && n < 9213))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9213)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9214)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9215)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9216)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9217)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9218)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
.if((n >= 9219) && (n < 9238))
; 9219 to 9237
.if(n = 9219)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9220)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9221)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9222)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9223)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9224)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9225)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9226)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9227)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9228)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9229)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9230)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9231)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9232)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9233)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9234)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9235)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 9238 to 9257
.if(n = 9238)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9239)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9240)
	; 9 bytes
	php
	tax
	lda #36
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 9241)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9242 || n = 9244 || n = 9247 || n = 9249 || n = 9250 || n = 9252 || n = 9257)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 9243)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9245)
	; 9 bytes
	php
	tax
	lda #36
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9254)
	; 11 bytes
	php
	pha
	inc $F6,X
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
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.endif
.else
; 9258 to 9296
.if(n = 9258 || n = 9261)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9259 || n = 9262)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9260)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9263)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9264 || n = 9265)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 9266 || n = 9269 || n = 9271 || n = 9274)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9267)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9268)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9270)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 9272 || n = 9273 || n = 9275 || n = 9276)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 9276 && n < 9297))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9457)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9458)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9459)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9460)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9461)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9462)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9463)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
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
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 9465 && n < 9469))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9469)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9470)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9471)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9472)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 9473 to 9492
.if(n = 9473)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9474)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9475)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9476)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9477)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9478)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9479)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9480)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9481)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9482)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9483)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9484)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9485)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9486)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9487)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9488)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9491)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
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
; 9493 to 9531
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
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9495)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9496)
	; 9 bytes
	php
	tax
	lda #37
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 9497)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9498 || n = 9500 || n = 9503 || n = 9505 || n = 9506 || n = 9508 || n = 9513 || n = 9520 || n = 9521)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 9499)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9501)
	; 9 bytes
	php
	tax
	lda #37
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9510)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 9512 || n = 9514 || n = 9517)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9515 || n = 9518)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9516)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9519)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9522 || n = 9525 || n = 9527 || n = 9530)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9523)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9524)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9526)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 9528 || n = 9529 || n = 9531)
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
.elseif((n > 9532 && n < 9688))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9713)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9714)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9715)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9716)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9717)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9718)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9719)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9720)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9721)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 9721 && n < 9725))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9725)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9726)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
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
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9728)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9729)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9730)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9731)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9732)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9733)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9734)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9735)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9736)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9737)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9738)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9739)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9740)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9741)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9742)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9743)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9744)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9745)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 9746 to 9765
.if(n = 9746)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9747)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9748)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9749)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9750)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9751)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9752)
	; 9 bytes
	php
	tax
	lda #38
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 9753)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9754 || n = 9756 || n = 9759 || n = 9761 || n = 9762 || n = 9764)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9755)
	; 9 bytes
	php
	pha
	lda #38
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9757)
	; 9 bytes
	php
	tax
	lda #38
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 9766 to 9843
.if(n = 9766)
	; 11 bytes
	php
	pha
	inc $F6,X
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 9768 || n = 9770 || n = 9773)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9769 || n = 9776 || n = 9777)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 9771 || n = 9774)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9772)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9775)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9778 || n = 9781 || n = 9783 || n = 9786)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9779)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9780)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9782)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 9784 || n = 9785 || n = 9787 || n = 9788)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 9788 && n < 9844))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9969)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9970)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9971)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9972)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9973)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9974)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9975)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9976)
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
.elseif(n = 9977)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 9977 && n < 9981))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 9981 to 10000
.if(n = 9981)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9982)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9983)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9984)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9985)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9986)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9987)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9988)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9989)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9990)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9991)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9992)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9993)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9994)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9995)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9996)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9997)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9998)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9999)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10000)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
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
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10002)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10003)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
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
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10007)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10008)
	; 9 bytes
	php
	tax
	lda #39
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 10009)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10010 || n = 10012 || n = 10015 || n = 10017 || n = 10018)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10011)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10013)
	; 9 bytes
	php
	tax
	lda #39
	jsr delay_256a_16_clocks_b
	txa
	plp
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
.if(n = 10020 || n = 10025 || n = 10032 || n = 10033)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 10021)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10022)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 10024 || n = 10026 || n = 10029)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10027 || n = 10030)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10028)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10031)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10034 || n = 10037 || n = 10039)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10035)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10036)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10038)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.endif
.endif
.else
; 10040 to 10078
.if(n = 10040 || n = 10041 || n = 10043 || n = 10044)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10042)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 10044 && n < 10079))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10225)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10226)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10227)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10228)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10229)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10230)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10231)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10232)
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
.elseif(n = 10233)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10234)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
.if((n >= 10235) && (n < 10274))
.if((n >= 10235) && (n < 10254))
; 10235 to 10253
.if(n = 10235 || n = 10236)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10237)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10238)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10239)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10240)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10241)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10242)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10243)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10244)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10245)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10246)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10247)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10248)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10249)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10250)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10251)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10252)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10253)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 10254 to 10273
.if(n = 10254)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10255)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10256)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10257)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10258)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10259)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10260)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10261)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10262)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10263)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10264)
	; 9 bytes
	php
	tax
	lda #40
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 10265)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10266 || n = 10268 || n = 10271 || n = 10273)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10267)
	; 9 bytes
	php
	pha
	lda #40
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10269)
	; 9 bytes
	php
	tax
	lda #40
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 10274 || n = 10276 || n = 10281 || n = 10288 || n = 10289)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10278)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 10280 || n = 10282 || n = 10285)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10283 || n = 10286)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10284)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10287)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10290 || n = 10293 || n = 10295 || n = 10298)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10291)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10292)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10294)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 10296 || n = 10297 || n = 10299 || n = 10300)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 10300 && n < 10313))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10481)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10482)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10483)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10484)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10485)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
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
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10489)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 10489 && n < 10493))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10493)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10494)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10495)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10496)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10497)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10498)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10499)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10500)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10501)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10502)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10503)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10504)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10505)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10506)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10507)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 10508) && (n < 10527))
; 10508 to 10526
.if(n = 10508)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10509)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10510)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10511)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10512)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10513)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10514)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10515)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10516)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10517)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10518)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10519)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10520)
	; 9 bytes
	php
	tax
	lda #41
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 10521)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10522 || n = 10524)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10523)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10525)
	; 9 bytes
	php
	tax
	lda #41
	jsr delay_256a_16_clocks_b
	txa
	plp
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
.if(n = 10527 || n = 10529 || n = 10530 || n = 10532 || n = 10537 || n = 10544 || n = 10545)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10534)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 10536 || n = 10538 || n = 10541)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10539 || n = 10542)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10540)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10543)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10546)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.endif
.endif
.endif
.else
; 10547 to 10625
.if(n = 10547)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10548)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10549 || n = 10551 || n = 10554)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10550)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 10552 || n = 10553 || n = 10555 || n = 10556)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 10556 && n < 10626))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10737)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10738)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10739)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10740)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10741)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10744)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10745)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 10745 && n < 10749))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10749)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10750)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10751)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10752)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10753)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10754)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10755)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10756)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10757)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10758)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10759)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10760)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10761)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 10762 to 10781
.if(n = 10762)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10763)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10764)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10765)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10766)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10767)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10768)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10769)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10770)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10771)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10772)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10773)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10774)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10775)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10776)
	; 9 bytes
	php
	tax
	lda #42
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 10777)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10778 || n = 10780)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10779)
	; 9 bytes
	php
	pha
	lda #42
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10781)
	; 9 bytes
	php
	tax
	lda #42
	jsr delay_256a_16_clocks_b
	txa
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
.elseif(n = 10783 || n = 10785 || n = 10786 || n = 10788 || n = 10793 || n = 10800 || n = 10801)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10790)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 10792 || n = 10794 || n = 10797)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10795 || n = 10798)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10796)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10799)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10802 || n = 10805 || n = 10807 || n = 10810)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10803)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10804)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10806)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 10808 || n = 10809 || n = 10811 || n = 10812)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 10812 && n < 10938))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10993)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10994)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10995)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10997)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11000)
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
.elseif(n = 11001)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 11001 && n < 11005))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11005)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11006)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11007)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11008)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11009)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11010)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11011)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11012)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11013)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11014)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11015)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11017)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11018)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11019)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11020)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11021)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11022)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11023)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11024)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11025)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11026)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11027)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11028)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11031)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11032)
	; 9 bytes
	php
	tax
	lda #43
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 11033)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11034)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 11036 || n = 11039 || n = 11041 || n = 11042 || n = 11044 || n = 11049)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11037)
	; 9 bytes
	php
	tax
	lda #43
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11046)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 11048 || n = 11050 || n = 11053)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11051 || n = 11054)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11052)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 11055 to 11093
.if(n = 11055)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11056 || n = 11057)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 11058 || n = 11061 || n = 11063 || n = 11066)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11059)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11060)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11062)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 11064 || n = 11065 || n = 11067 || n = 11068)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 11068 && n < 11094))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11249)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11250)
	; 16 bytes
	php
	pha
	brk
	lda #168
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
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11252)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11253)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
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
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11257)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 11257 && n < 11261))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11261)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11262)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11263)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11264)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11265)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11266)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11267)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11268)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11269)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 11270 to 11289
.if(n = 11270)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11271)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11272)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11273)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11274)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11275)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11276)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11277)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11278)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11279)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11280)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11283)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
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
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11288)
	; 9 bytes
	php
	tax
	lda #44
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 11289)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 11290 to 11328
.if(n = 11290 || n = 11292 || n = 11295 || n = 11297 || n = 11298 || n = 11300 || n = 11305 || n = 11312 || n = 11313)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 11291)
	; 9 bytes
	php
	pha
	lda #44
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11293)
	; 9 bytes
	php
	tax
	lda #44
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11302)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 11304 || n = 11306 || n = 11309)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11307 || n = 11310)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11308)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11311)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11314 || n = 11317 || n = 11319 || n = 11322)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11315)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11316)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11318)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 11320 || n = 11321 || n = 11323 || n = 11324)
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
.if((n > 11328 && n < 11407))
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
; 11485 to 11523
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
.elseif(n = 11504)
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11505)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11506)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11507)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11508)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11509)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11512)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11513)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 11513 && n < 11517))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11517)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11518)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11519)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11520)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11521)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11522)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11523)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 11524) && (n < 11543))
; 11524 to 11542
.if(n = 11524)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11525)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11526)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11527)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11528)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11529)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11530)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11531)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11532)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11533)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11534)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11535)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11536)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11537)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11538)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11539)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11540)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11541)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11542)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 11543 to 11562
.if(n = 11543)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11544)
	; 9 bytes
	php
	tax
	lda #45
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 11545)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11546 || n = 11548 || n = 11551 || n = 11553 || n = 11554 || n = 11556 || n = 11561)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11547)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11549)
	; 9 bytes
	php
	tax
	lda #45
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11558)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 11560 || n = 11562)
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
; 11563 to 11718
.if(n = 11563 || n = 11566)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11564)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11565)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11567)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11568 || n = 11569)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 11570 || n = 11573 || n = 11575 || n = 11578)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11571)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11572)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11574)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 11576 || n = 11577 || n = 11579 || n = 11580)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 11580 && n < 11719))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11761)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11762)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11763)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11764)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11765)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11768)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11769)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 11769 && n < 11773))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11773)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11774)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11775)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11776)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 11777 to 11796
.if(n = 11777)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11778)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11779)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11780)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11781)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11782)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11783)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11784)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11785)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11786)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11787)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11788)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11789)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11790)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11791)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11792)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11793)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11794)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11795)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11796)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 11797 to 11875
.if(n = 11797)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11798)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11799)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11800)
	; 9 bytes
	php
	tax
	lda #46
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 11801)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11802 || n = 11804 || n = 11807 || n = 11809 || n = 11810 || n = 11812 || n = 11817 || n = 11824 || n = 11825)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 11803)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11805)
	; 9 bytes
	php
	tax
	lda #46
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11814)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 11816 || n = 11818 || n = 11821)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11819 || n = 11822)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11820)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11823)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11826 || n = 11829 || n = 11831 || n = 11834)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11827)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11828)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11830)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 11832 || n = 11833 || n = 11835 || n = 11836)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 11836 && n < 11876))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12017)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12018)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12019)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12020)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12021)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12024)
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
.elseif(n = 12025)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 12025 && n < 12029))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12029)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12030)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12031)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
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
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12033)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12034)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12035)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12036)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12037)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12038)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12039)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12040)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12041)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12042)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12043)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12044)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12045)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12046)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12047)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12048)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12049)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12050)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 12051 to 12070
.if(n = 12051)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12052)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12053)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12054)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12055)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12056)
	; 9 bytes
	php
	tax
	lda #47
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 12057)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12058 || n = 12060 || n = 12063 || n = 12065 || n = 12066 || n = 12068)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12059)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12061)
	; 9 bytes
	php
	tax
	lda #47
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12070)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 12072 || n = 12074 || n = 12077)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12073 || n = 12080 || n = 12081)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 12075 || n = 12078)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12076)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12079)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12082 || n = 12085 || n = 12087 || n = 12090)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12083)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12084)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12086)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 12088 || n = 12089 || n = 12091 || n = 12092)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 12092 && n < 12110))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12273)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12274)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12275)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12276)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12277)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12278)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12279)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12280)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12281)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 12281 && n < 12285))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 12285 to 12304
.if(n = 12285)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12286)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12287)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12288)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12289)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12290)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12291)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12292)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12293)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12294)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12295)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12296)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12297)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12298)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12299)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12300)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12301)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12302)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12303)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12304)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 12305) && (n < 12324))
; 12305 to 12323
.if(n = 12305)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12306)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12307)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12308)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12309)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12310)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12311)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12312)
	; 9 bytes
	php
	tax
	lda #48
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 12313)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12314 || n = 12316 || n = 12319 || n = 12321 || n = 12322)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12315)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12317)
	; 9 bytes
	php
	tax
	lda #48
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 12324 || n = 12329 || n = 12336 || n = 12337)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 12325)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12326)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 12328 || n = 12330 || n = 12333)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12331 || n = 12334)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12332)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12335)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12338 || n = 12341 || n = 12343)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12339)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12340)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12342)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.endif
.endif
.endif
.endif
.else
; 12344 to 12500
.if(n = 12344 || n = 12345 || n = 12347 || n = 12348)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12346)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 12348 && n < 12501))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12529)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12530)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12531)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12532)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12533)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12536)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12537)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12538 || n = 12539)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
.if((n >= 12540) && (n < 12559))
; 12540 to 12558
.if(n = 12540)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12541)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12542)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12543)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12544)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12545)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12546)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12547)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12548)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12549)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12550)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12551)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12552)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12553)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12554)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12555)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12556)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12557)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12558)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 12559 to 12578
.if(n = 12559)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12560)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12561)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12562)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12563)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12564)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12565)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12566)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12567)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12568)
	; 9 bytes
	php
	tax
	lda #49
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 12569)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12570 || n = 12572 || n = 12575 || n = 12577 || n = 12578)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12571)
	; 9 bytes
	php
	pha
	lda #49
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12573)
	; 9 bytes
	php
	tax
	lda #49
	jsr delay_256a_16_clocks_b
	txa
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
.elseif(n = 12580 || n = 12585 || n = 12592 || n = 12593)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 12581)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12582)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 12584 || n = 12586 || n = 12589)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12587 || n = 12590)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12588)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12591)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12594 || n = 12597 || n = 12599 || n = 12602)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12595)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12596)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12598)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 12600 || n = 12601 || n = 12603 || n = 12604)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 12604 && n < 12657))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12785)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12786)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12787)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12788)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12789)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12790)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12791)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 12793 && n < 12797))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12797)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12798)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12799)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12800)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12801)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12802)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12803)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12804)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12805)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12806)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12807)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12808)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12809)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12810)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12811)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12812)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
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
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12814)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12815)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12816)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12817)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12818)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12819)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12820)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12821)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12822)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12823)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12824)
	; 9 bytes
	php
	tax
	lda #50
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 12825)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12826 || n = 12828 || n = 12831)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12827)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12829)
	; 9 bytes
	php
	tax
	lda #50
	jsr delay_256a_16_clocks_b
	txa
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
.elseif(n = 12833 || n = 12834 || n = 12836 || n = 12841 || n = 12848 || n = 12849)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12838)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 12840 || n = 12842 || n = 12845)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12843 || n = 12846)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12844)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12847)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12850)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12851)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 12852 to 12890
.if(n = 12852)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12853 || n = 12855 || n = 12858)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12854)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 12856 || n = 12857 || n = 12859 || n = 12860)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 12860 && n < 12891))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13041)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13042)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13043)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13044)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13045)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13048)
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
.elseif(n = 13049)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 13049 && n < 13053))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13053)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13054)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13055)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13056)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13057)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13058)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13059)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13060)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13061)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13062)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13063)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13064)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13065)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13067)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13068)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13069)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13070)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13071)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13072)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13073)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13074)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13075)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13076)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13077)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13078)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13079)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13080)
	; 9 bytes
	php
	tax
	lda #51
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 13081)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13082 || n = 13084)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13083)
	; 9 bytes
	php
	pha
	lda #51
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13085)
	; 9 bytes
	php
	tax
	lda #51
	jsr delay_256a_16_clocks_b
	txa
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
.elseif(n = 13087 || n = 13089 || n = 13090 || n = 13092 || n = 13097 || n = 13104 || n = 13105)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13094)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 13096 || n = 13098 || n = 13101)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13099 || n = 13102)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13100)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13103)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13106 || n = 13109 || n = 13111 || n = 13114)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13107)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13108)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13110)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 13112 || n = 13113 || n = 13115 || n = 13116)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 13116 && n < 13126))
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
.if((n >= 13126) && (n < 13438))
.if((n >= 13126) && (n < 13282))
; 13126 to 13281
.if((n > 13125 && n < 13282))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13297)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13298)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13299)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13300)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 13301 to 13320
.if(n = 13301)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13302)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13303)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13304)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13305)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 13305 && n < 13309))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13309)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13310)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13311)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13312)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13313)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13314)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13315)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13316)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13317)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13318)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13319)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13320)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 13321) && (n < 13340))
; 13321 to 13339
.if(n = 13321)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13322)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13323)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13324)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13325)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13326)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13327)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13328)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13329)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13330)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13331)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13335)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13336)
	; 9 bytes
	php
	tax
	lda #52
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 13337)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13338)
	; 10 bytes
	delay_n (n-2)
	nop
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
.if(n = 13340 || n = 13343 || n = 13345 || n = 13346 || n = 13348 || n = 13353)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13341)
	; 9 bytes
	php
	tax
	lda #52
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13350)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 13352 || n = 13354 || n = 13357)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13355 || n = 13358)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13356)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13359)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.else
; 13360 to 13437
.if(n = 13360 || n = 13361)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 13362 || n = 13365 || n = 13367 || n = 13370)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13363)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13364)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13366)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 13368 || n = 13369 || n = 13371 || n = 13372)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 13372 && n < 13438))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13553)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13554)
	; 16 bytes
	php
	pha
	brk
	lda #168
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
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13556)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13557)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13558)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13559)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13560)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13561)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 13561 && n < 13565))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13565)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13566)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13567)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13568)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13569)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13570)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13571)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13572)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13573)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 13574 to 13593
.if(n = 13574)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13575)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13576)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13577)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13578)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13579)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13580)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13581)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13582)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13583)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13586)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13587)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13590)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13591)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13592)
	; 9 bytes
	php
	tax
	lda #53
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 13593)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 13594 to 13750
.if(n = 13594 || n = 13596 || n = 13599 || n = 13601 || n = 13602 || n = 13604 || n = 13609 || n = 13616 || n = 13617)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 13595)
	; 9 bytes
	php
	pha
	lda #53
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13597)
	; 9 bytes
	php
	tax
	lda #53
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13606)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 13608 || n = 13610 || n = 13613)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13611 || n = 13614)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13612)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13615)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13618 || n = 13621 || n = 13623 || n = 13626)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13619)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13620)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13622)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 13624 || n = 13625 || n = 13627 || n = 13628)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 13628 && n < 13751))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13810)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13811)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13812)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13813)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13814)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13815)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
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
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 13817 && n < 13821))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13821)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13822)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13823)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13824)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13825)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13826)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13827)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13828)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13830)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13831)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13832)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13833)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13834)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13835)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13836)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13837)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13838)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13839)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13840)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13841)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13842)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13843)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13844)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13845)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13846)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13847)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 13848 to 13867
.if(n = 13848)
	; 9 bytes
	php
	tax
	lda #54
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 13849)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13850 || n = 13852 || n = 13855 || n = 13857 || n = 13858 || n = 13860 || n = 13865)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13851)
	; 9 bytes
	php
	pha
	lda #54
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13853)
	; 9 bytes
	php
	tax
	lda #54
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13862)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 13864 || n = 13866)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13867)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 13868 to 13906
.if(n = 13868)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13869)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13870)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13871)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13872 || n = 13873)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 13874 || n = 13877 || n = 13879 || n = 13882)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13875)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13876)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13878)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 13880 || n = 13881 || n = 13883 || n = 13884)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 13884 && n < 13907))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14065)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14066)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14067)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14068)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14069)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14070)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14071)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14072)
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
.elseif(n = 14073)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 14073 && n < 14077))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14077)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14078)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14079)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14080)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14081)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 14082 to 14101
.if(n = 14082)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14083)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14084)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14085)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14086)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14087)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14088)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14089)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14090)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14091)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14092)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14093)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14094)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14095)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14096)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14099)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14100)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
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
; 14102 to 14140
.if(n = 14102)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14103)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14104)
	; 9 bytes
	php
	tax
	lda #55
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 14105)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14106 || n = 14108 || n = 14111 || n = 14113 || n = 14114 || n = 14116 || n = 14121 || n = 14128 || n = 14129)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 14107)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14109)
	; 9 bytes
	php
	tax
	lda #55
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14118)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 14120 || n = 14122 || n = 14125)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14123 || n = 14126)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14124)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14127)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14130 || n = 14133 || n = 14135 || n = 14138)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14131)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14132)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14134)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 14136 || n = 14137 || n = 14139 || n = 14140)
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
; 14141 to 14218
.if((n > 14140 && n < 14219))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14321)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14322)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14323)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14324)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14325)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14326)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14327)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14328)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14329)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 14329 && n < 14333))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14333)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14334)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14335)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
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
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14337)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14338)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14339)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14340)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14341)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14342)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14343)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14344)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14345)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14346)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14347)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14348)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14349)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14350)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14351)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14352)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14353)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14354)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14355)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 14356 to 14375
.if(n = 14356)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14357)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14358)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14359)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14360)
	; 9 bytes
	php
	tax
	lda #56
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 14361)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14362 || n = 14364 || n = 14367 || n = 14369 || n = 14370 || n = 14372)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14363)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14365)
	; 9 bytes
	php
	tax
	lda #56
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14374)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.if(n = 14376 || n = 14378 || n = 14381)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14377 || n = 14384 || n = 14385)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 14379 || n = 14382)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14380)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14383)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14386 || n = 14389 || n = 14391 || n = 14394)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14387)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14388)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14390)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 14392 || n = 14393 || n = 14395 || n = 14396)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 14396 && n < 14532))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14577)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14578)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14579)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14580)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14581)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14584)
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
.elseif(n = 14585)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 14585 && n < 14589))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14589)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 14590 to 14609
.if(n = 14590)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14591)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14592)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14593)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14594)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14595)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14596)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14597)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14598)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14599)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14600)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14601)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14602)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14603)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14604)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14605)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14606)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14607)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14611)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14612)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14613)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14614)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14615)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14616)
	; 9 bytes
	php
	tax
	lda #57
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 14617)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14618 || n = 14620 || n = 14623 || n = 14625 || n = 14626 || n = 14628)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14619)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14621)
	; 9 bytes
	php
	tax
	lda #57
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14630)
	; 11 bytes
	php
	pha
	inc $F6,X
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 14632 || n = 14634 || n = 14637)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14633 || n = 14640 || n = 14641)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 14635 || n = 14638)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14636)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14639)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14642 || n = 14645 || n = 14647)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14643)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14644)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14646)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 14648)
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
; 14649 to 14687
.if(n = 14649 || n = 14651 || n = 14652)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14650)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 14652 && n < 14688))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14833)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14834)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14835)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14836)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14837)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14838)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14839)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14840)
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
.elseif(n = 14841)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14842 || n = 14843)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
.if((n >= 14844) && (n < 14922))
.if((n >= 14844) && (n < 14883))
.if((n >= 14844) && (n < 14863))
; 14844 to 14862
.if(n = 14844)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14845)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14846)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14847)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14848)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14849)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14850)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14851)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14852)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14853)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14854)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14855)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14856)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14857)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14858)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14859)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14860)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14861)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14862)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 14863 to 14882
.if(n = 14863)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14864)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14865)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14866)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14867)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14868)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14869)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14870)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14871)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14872)
	; 9 bytes
	php
	tax
	lda #58
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 14873)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14874 || n = 14876 || n = 14879 || n = 14881 || n = 14882)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14875)
	; 9 bytes
	php
	pha
	lda #58
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14877)
	; 9 bytes
	php
	tax
	lda #58
	jsr delay_256a_16_clocks_b
	txa
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
.elseif(n = 14884 || n = 14889 || n = 14896 || n = 14897)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 14885)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14886)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 14888 || n = 14890 || n = 14893)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14891 || n = 14894)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14892)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14895)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14898 || n = 14901 || n = 14903 || n = 14906)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14899)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14900)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14902)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 14904 || n = 14905 || n = 14907 || n = 14908)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 14908 && n < 14922))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15089)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15090)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15091)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15092)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15093)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15096)
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
.elseif(n = 15097)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 15098 to 15117
.if((n > 15097 && n < 15101))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15101)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15102)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15103)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15104)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15105)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15106)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15107)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15108)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15109)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15110)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15111)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15112)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15113)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15114)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15115)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15116)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15117)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
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
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15119)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15120)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15123)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15124)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15125)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15126)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15127)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15128)
	; 9 bytes
	php
	tax
	lda #59
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 15129)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15130 || n = 15132 || n = 15135)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15131)
	; 9 bytes
	php
	pha
	lda #59
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15133)
	; 9 bytes
	php
	tax
	lda #59
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 15137 || n = 15138 || n = 15140 || n = 15145 || n = 15152 || n = 15153)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15142)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 15144 || n = 15146 || n = 15149)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15147 || n = 15150)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15148)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15151)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15154)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15155)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15156)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.endif
.endif
.else
; 15157 to 15312
.if(n = 15157 || n = 15159 || n = 15162)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15158)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 15160 || n = 15161 || n = 15163 || n = 15164)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 15164 && n < 15313))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15345)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15346)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15347)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15348)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15349)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
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
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15353)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 15353 && n < 15357))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15357)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15358)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15359)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15360)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15361)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15362)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15363)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15364)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15365)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15366)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15367)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15368)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15369)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15370)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 15371 to 15390
.if(n = 15371)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15372)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15373)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15374)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15375)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15376)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15377)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15378)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15379)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15380)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15381)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15382)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15383)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15384)
	; 9 bytes
	php
	tax
	lda #60
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 15385)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15386 || n = 15388)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15387)
	; 9 bytes
	php
	pha
	lda #60
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15389)
	; 9 bytes
	php
	tax
	lda #60
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 15391 || n = 15393 || n = 15394 || n = 15396 || n = 15401 || n = 15408 || n = 15409)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15398)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 15400 || n = 15402 || n = 15405)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15403 || n = 15406)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15404)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15407)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15410 || n = 15413 || n = 15415 || n = 15418)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15411)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15412)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15414)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 15416 || n = 15417 || n = 15419 || n = 15420)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 15420 && n < 15469))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15601)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15602)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15603)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15604)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15605)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15607)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15608)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15609)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 15609 && n < 15613))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15613)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15614)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15615)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15616)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15617)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15618)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15619)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15620)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15621)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15622)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15623)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15624)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15625)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15627)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15628)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15629)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15630)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15631)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15632)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15633)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15634)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15635)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15636)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15637)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15638)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15639)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15640)
	; 9 bytes
	php
	tax
	lda #61
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 15641)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15642 || n = 15644)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15643)
	; 9 bytes
	php
	pha
	lda #61
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 15645 to 15664
.if(n = 15645)
	; 9 bytes
	php
	tax
	lda #61
	jsr delay_256a_16_clocks_b
	txa
	plp
.elseif(n = 15646)
	; 11 bytes
	php
	pha
	lda $A5
	lda #61
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15647 || n = 15649 || n = 15650 || n = 15652 || n = 15657 || n = 15664)
	; 10..13 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15654)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 15656 || n = 15658 || n = 15661)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15659 || n = 15662)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15660)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15663)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.else
; 15665 to 15703
.if(n = 15665)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 15666 || n = 15669 || n = 15671 || n = 15674)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15667)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15668)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15670)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 15672 || n = 15673 || n = 15675 || n = 15676)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 15676 && n < 15704))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15857)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15858)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15859)
	; 16 bytes
	php
	pha
	brk
	lda #169
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
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15861)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15864)
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
.elseif(n = 15865)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 15865 && n < 15869))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15869)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15870)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15871)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15872)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15873)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15874)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15875)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15876)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15877)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15878)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 15879 to 15898
.if(n = 15879)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15880)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15881)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15882)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15883)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15884)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15885)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15886)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15887)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15888)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15889)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15890)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15891)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15892)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15893)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15894)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15895)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15896)
	; 9 bytes
	php
	tax
	lda #62
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 15897)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15898)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 15900 || n = 15903 || n = 15905 || n = 15906 || n = 15908 || n = 15913 || n = 15920 || n = 15921)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 15901)
	; 9 bytes
	php
	tax
	lda #62
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15910)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 15912 || n = 15914 || n = 15917)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15915 || n = 15918)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15916)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15919)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15922 || n = 15925 || n = 15927 || n = 15930)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15923)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15924)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15926)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 15928 || n = 15929 || n = 15931 || n = 15932)
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
.if((n > 15937 && n < 16094))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16114)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16115)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16116)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16117)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16118)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16119)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16120)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16121)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 16121 && n < 16125))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16125)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16126)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16127)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16128)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16129)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16130)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16131)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16132)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 16133) && (n < 16152))
; 16133 to 16151
.if(n = 16133)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16134)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16135)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16136)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16137)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16138)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16139)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16140)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16141)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16142)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16143)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16144)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16145)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16146)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16147)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16148)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16149)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16150)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16151)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 16152 to 16171
.if(n = 16152)
	; 9 bytes
	php
	tax
	lda #63
	jsr delay_256a_11_clocks_b_
	txa
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
.elseif(n = 16154 || n = 16156 || n = 16159 || n = 16161 || n = 16162 || n = 16164 || n = 16169)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16155)
	; 9 bytes
	php
	pha
	lda #63
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16157)
	; 9 bytes
	php
	tax
	lda #63
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16166)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 16168 || n = 16170)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16171)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.else
; 16172 to 16250
.if(n = 16172)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16173)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16174)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16175)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16176 || n = 16177)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 16178 || n = 16181 || n = 16183 || n = 16186)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16179)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16180)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16182)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 16184 || n = 16185 || n = 16187 || n = 16188)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 16188 && n < 16251))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16369)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16370)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16371)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16372)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16373)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16374)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16375)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16376)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16377)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 16377 && n < 16381))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16381)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16382)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16383)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16384)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16385)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16386)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 16387 to 16406
.if(n = 16387)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16388)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16389)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16390)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16391)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16392)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16393)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16394)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16395)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16396)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16397)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16398)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16399)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16400)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16401)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16402)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16403)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16404)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 16407 to 16562
.if(n = 16407)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16408)
	; 9 bytes
	php
	tax
	lda #64
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 16409)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16410 || n = 16412 || n = 16415 || n = 16417 || n = 16418 || n = 16420 || n = 16425 || n = 16432 || n = 16433)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 16411)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16413)
	; 9 bytes
	php
	tax
	lda #64
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16422)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 16424 || n = 16426 || n = 16429)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16427 || n = 16430)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16428)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16431)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16434 || n = 16437 || n = 16439 || n = 16442)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16435)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16436)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16438)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 16440 || n = 16441 || n = 16443 || n = 16444)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 16444 && n < 16563))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16625)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16626)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16627)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16628)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16629)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16630)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16631)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16632)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16633)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 16633 && n < 16637))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16637)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16638)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16639)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16640)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
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
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16642)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16643)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16644)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16645)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16646)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16647)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16648)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16649)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16650)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16651)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16652)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16653)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16654)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16655)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16656)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16657)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16658)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16659)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 16660 to 16679
.if(n = 16660)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16661)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16662)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16663)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16664)
	; 9 bytes
	php
	tax
	lda #65
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 16665)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16666 || n = 16668 || n = 16671 || n = 16673 || n = 16674 || n = 16676)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16667)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16669)
	; 9 bytes
	php
	tax
	lda #65
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16678)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.if(n = 16680 || n = 16682 || n = 16685)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16681 || n = 16688 || n = 16689)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 16683 || n = 16686)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16684)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16687)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16690 || n = 16693 || n = 16695 || n = 16698)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16691)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16692)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16694)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 16696 || n = 16697 || n = 16699 || n = 16700)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 16700 && n < 16719))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16881)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16882)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16883)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16884)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16885)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16886)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16887)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16888)
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
.elseif(n = 16889)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 16889 && n < 16893))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16893)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16894)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 16895 to 16914
.if(n = 16895)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16896)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16897)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16898)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16899)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16900)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16901)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16902)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16903)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16904)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16905)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16906)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16907)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16908)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16909)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16910)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16911)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16912)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16913)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16914)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 16915 to 16953
.if(n = 16915)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16916)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16917)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16918)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16919)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16920)
	; 9 bytes
	php
	tax
	lda #66
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 16921)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16922 || n = 16924 || n = 16927 || n = 16929 || n = 16930 || n = 16932 || n = 16937 || n = 16944 || n = 16945)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 16923)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16925)
	; 9 bytes
	php
	tax
	lda #66
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16934)
	; 11 bytes
	php
	pha
	inc $F6,X
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 16936 || n = 16938 || n = 16941)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16939 || n = 16942)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16940)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16943)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16946 || n = 16949 || n = 16951)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16947)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16948)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16950)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 16952 || n = 16953)
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
; 16954 to 17031
.if(n = 16954)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16955 || n = 16956)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 16956 && n < 17032))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17137)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17138)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17139)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17140)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17141)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17144)
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
.elseif(n = 17145)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 17145 && n < 17149))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
.if((n >= 17149) && (n < 17168))
; 17149 to 17167
.if(n = 17149)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17150)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17151)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17152)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17153)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17154)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17155)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17156)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17157)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17158)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17159)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17160)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17161)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17162)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17163)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17164)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17165)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17166)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17167)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 17168 to 17187
.if(n = 17168)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17169)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17175)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17176)
	; 9 bytes
	php
	tax
	lda #67
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 17177)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17178 || n = 17180 || n = 17183 || n = 17185 || n = 17186)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17179)
	; 9 bytes
	php
	pha
	lda #67
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17181)
	; 9 bytes
	php
	tax
	lda #67
	jsr delay_256a_16_clocks_b
	txa
	plp
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
.if(n = 17188 || n = 17193 || n = 17200 || n = 17201)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 17189)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17190)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 17192 || n = 17194 || n = 17197)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17195 || n = 17198)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17196)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17199)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17202 || n = 17205 || n = 17207 || n = 17210)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17203)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17204)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17206)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 17208 || n = 17209 || n = 17211 || n = 17212)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 17212 && n < 17344))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17393)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17394)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17395)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17396)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17397)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17400)
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
.elseif(n = 17401)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 17402 to 17421
.if((n > 17401 && n < 17405))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17405)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17406)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17407)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17408)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17409)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17410)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17411)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17412)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17413)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17414)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17415)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17416)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17417)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17418)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17419)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17420)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17421)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17423)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17424)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17425)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17426)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17427)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17428)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
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
.elseif(n = 17432)
	; 9 bytes
	php
	tax
	lda #68
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 17433)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17434 || n = 17436 || n = 17439)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17435)
	; 9 bytes
	php
	pha
	lda #68
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17437)
	; 9 bytes
	php
	tax
	lda #68
	jsr delay_256a_16_clocks_b
	txa
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
.if(n = 17441 || n = 17442 || n = 17444 || n = 17449 || n = 17456 || n = 17457)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17446)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 17448 || n = 17450 || n = 17453)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17451 || n = 17454)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17452)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17455)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17458)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17459)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17460)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.else
; 17461 to 17500
.if(n = 17461 || n = 17463 || n = 17466)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17462)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 17464 || n = 17465 || n = 17467 || n = 17468)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 17468 && n < 17501))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17649)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17650)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17651)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17652)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17653)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17656)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
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
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 17657 && n < 17661))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17661)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17662)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17663)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17664)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17665)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17666)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17667)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17668)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17669)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17670)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17671)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17672)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17673)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17674)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17675)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 17676 to 17695
.if(n = 17676)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17677)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17678)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17679)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17680)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17681)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17682)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17683)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17684)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17685)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17686)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17687)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17688)
	; 9 bytes
	php
	tax
	lda #69
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 17689)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17690 || n = 17692 || n = 17695)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17691)
	; 9 bytes
	php
	pha
	lda #69
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17693)
	; 9 bytes
	php
	tax
	lda #69
	jsr delay_256a_16_clocks_b
	txa
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
.elseif(n = 17697 || n = 17698 || n = 17700 || n = 17705 || n = 17712 || n = 17713)
	; 10..13 bytes
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17702)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 17704 || n = 17706 || n = 17709)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17707 || n = 17710)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17708)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17711)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17714 || n = 17717 || n = 17719 || n = 17722)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17715)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17716)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17718)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 17720 || n = 17721 || n = 17723 || n = 17724)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 17724 && n < 17735))
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
; 17735 to 17812
.if((n > 17734 && n < 17813))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17905)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17906)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17907)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17908)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17909)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17912)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17913)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 17913 && n < 17917))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17917)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17918)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17919)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17920)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17921)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17922)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17923)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17924)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17925)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17926)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17927)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17928)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17929)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 17930) && (n < 17949))
; 17930 to 17948
.if(n = 17930)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17931)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17932)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17933)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17934)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17935)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17936)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17937)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17938)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17939)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17940)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17941)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
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
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17944)
	; 9 bytes
	php
	tax
	lda #70
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 17945)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17946 || n = 17948)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17947)
	; 9 bytes
	php
	pha
	lda #70
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 17949 to 17968
.if(n = 17949)
	; 9 bytes
	php
	tax
	lda #70
	jsr delay_256a_16_clocks_b
	txa
	plp
.elseif(n = 17950)
	; 11 bytes
	php
	pha
	lda $A5
	lda #70
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17951 || n = 17953 || n = 17954 || n = 17956 || n = 17961 || n = 17968)
	; 10..13 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17958)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 17960 || n = 17962 || n = 17965)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17963 || n = 17966)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17964)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17967)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.endif
.else
; 17969 to 18125
.if(n = 17969)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 17970 || n = 17973 || n = 17975 || n = 17978)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17971)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17972)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17974)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 17976 || n = 17977 || n = 17979 || n = 17980)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 17980 && n < 18126))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18161)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18162)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18163)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18164)
	; 16 bytes
	php
	pha
	brk
	lda #170
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
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18166)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18167)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18168)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18169)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 18169 && n < 18173))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18173)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18174)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18175)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18176)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18177)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18178)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18179)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18180)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18181)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18182)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18183)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 18184 to 18203
.if(n = 18184)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18185)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18186)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18187)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18188)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18189)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18190)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18191)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18194)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18195)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18196)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18197)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18198)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18199)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18200)
	; 9 bytes
	php
	tax
	lda #71
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 18201)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18202)
	; 10 bytes
	delay_n (n-2)
	nop
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
.if(n = 18204 || n = 18207 || n = 18209 || n = 18210 || n = 18212 || n = 18217 || n = 18224 || n = 18225)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 18205)
	; 9 bytes
	php
	tax
	lda #71
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18214)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 18216 || n = 18218 || n = 18221)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18219 || n = 18222)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18220)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18223)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18226 || n = 18229 || n = 18231 || n = 18234)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18227)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18228)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18230)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 18232 || n = 18233 || n = 18235 || n = 18236)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 18236 && n < 18282))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18417)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 18418 to 18437
.if(n = 18418)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18419)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18420)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18421)
	; 16 bytes
	php
	pha
	brk
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18422)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18423)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18424)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18425)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 18425 && n < 18429))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18429)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18430)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18431)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18432)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18433)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18434)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18435)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18436)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18437)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
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
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18439)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18440)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18441)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18442)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18443)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18444)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18445)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18446)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18447)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18448)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18449)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18450)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18451)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18452)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
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
	; 9 bytes
	php
	tax
	lda #72
	jsr delay_256a_11_clocks_b_
	txa
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
.elseif(n = 18458 || n = 18460 || n = 18463 || n = 18465 || n = 18466 || n = 18468 || n = 18473)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18459)
	; 9 bytes
	php
	pha
	lda #72
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18461)
	; 9 bytes
	php
	tax
	lda #72
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18470)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 18472 || n = 18474)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18475)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18476)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 18477 to 18515
.if(n = 18477)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18478)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18479)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18480 || n = 18481)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 18482 || n = 18485 || n = 18487 || n = 18490)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18483)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18484)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18486)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 18488 || n = 18489 || n = 18491 || n = 18492)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 18492 && n < 18516))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18673)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18674)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18675)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18676)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18677)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18680)
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
.elseif(n = 18681)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 18681 && n < 18685))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18685)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18686)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18687)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18688)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18689)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18690)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 18691 to 18710
.if(n = 18691)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18692)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18693)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18694)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18695)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18696)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18697)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18698)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18699)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18700)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18701)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18702)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18703)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18704)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18705)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18706)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18707)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18708)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18709)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18710)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 18711 to 18750
.if(n = 18711)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18712)
	; 9 bytes
	php
	tax
	lda #73
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 18713)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18714 || n = 18716 || n = 18719 || n = 18721 || n = 18722 || n = 18724 || n = 18729 || n = 18736 || n = 18737)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 18715)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18717)
	; 9 bytes
	php
	tax
	lda #73
	jsr delay_256a_16_clocks_b
	txa
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18726)
	; 11 bytes
	php
	pha
	inc $F6,X
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
.elseif(n = 18728 || n = 18730 || n = 18733)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18731 || n = 18734)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18732)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18735)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18738 || n = 18741 || n = 18743 || n = 18746)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18739)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18740)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18742)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 18744 || n = 18745 || n = 18747 || n = 18748)
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
.else
.if((n >= 18751) && (n < 19376))
.if((n >= 18751) && (n < 19063))
.if((n >= 18751) && (n < 18907))
; 18751 to 18906
.if((n > 18750 && n < 18907))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18929)
	; 16 bytes
	php
	pha
	brk
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18930)
	; 16 bytes
	php
	pha
	brk
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18931)
	; 16 bytes
	php
	pha
	brk
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18932)
	; 16 bytes
	php
	pha
	brk
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18933)
	; 16 bytes
	php
	pha
	brk
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18936)
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
.elseif(n = 18937)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 18937 && n < 18941))
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18941)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #166
	jsr delay_a_27_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18942)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #167
	jsr delay_a_27_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18943)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #168
	jsr delay_a_27_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18944)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #169
	jsr delay_a_27_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18945)
	; 18 bytes
	php
	pha
	delay_n 26
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks_b
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
	delay_n 26
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18947)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #169
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18948)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #170
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18949)
	; 19 bytes
	php
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18950)
	; 19 bytes
	php
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18951)
	; 19 bytes
	php
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18952)
	; 19 bytes
	php
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18953)
	; 19 bytes
	php
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18954)
	; 19 bytes
	php
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18955)
	; 19 bytes
	php
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18956)
	; 19 bytes
	php
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18957)
	; 19 bytes
	php
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18958)
	; 19 bytes
	php
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18959)
	; 19 bytes
	php
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18960)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18961)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18962)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18963)
	; 19 bytes
	php
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18964)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 18965 to 18984
.if(n = 18965)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18966)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
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
	; 9 bytes
	php
	tax
	lda #74
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 18969)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18970 || n = 18972 || n = 18975 || n = 18977 || n = 18978 || n = 18980)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18971)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18973)
	; 9 bytes
	php
	tax
	lda #74
	jsr delay_256a_16_clocks_b
	txa
	plp
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
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18982)
	; 11 bytes
	php
	pha
	inc $F6,X
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
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985 || n = 18992 || n = 18993)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 18986 || n = 18989)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18987 || n = 18990)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18988)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18991)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18994 || n = 18997 || n = 18999 || n = 19002)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18995)
	; 13 bytes
	php
	pha
	delay_n 19
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18996)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18998)
	; 14 bytes
	php
	tax
	lda #<(n-53)
	jsr delay_a_27_clocks
	lda #>(n-53)
	jsr delay_256a_11_clocks_b_
	txa
	plp
.elseif(n = 19000 || n = 19001 || n = 19003 || n = 19004)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 19004 && n < 19063))
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
	; 16 bytes
	php
	pha
	brk
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19185)
	; 16 bytes
	php
	pha
	brk
	lda #16„ÊW¶÷ì{wÂö…È5îš¦´İbp.ÀšÏéÎ1§î¬oõàDé$ÓDÏ¼Ku—®û×”OZaæêVó3ô4Œa;æB$u°×l-	¥ q
#ÉŒX-ª»ZÍºZ¿úJØÀ6Q˜à8måû^'ºÏñf=-ª¯î€P@[šŸ20áŒCÈš¿ªâV§ğl¶‡'—ÒãàìX¸œ¿iÖxûŠŸ°°Kk× ôacUG:`R‰>¶:˜GJ7Ì½ËõÄŠŠÜÔª2¯ğmµÂ JÛ2¾ı@»9ü«9È®et€+ÚAZ…çKï
Q4–ÔdØb® ([şH-¶6Ã“ªå€Ğ¿º/Õc—£:‡%²º’ûêÕ‹jDçV:! J]"RæÆ_tÓœŒ²Ã‚n0?ó±vŠÍ›+<{İP&m"DÌ!Çèl²”æ,…ÁÅ65\V´>ô‚wñ¸¤ÉE†ßéÈ½ĞòV¹( Z „TP_KH¯Œ“*é”WÆlé€=*ÛòÚ2=?ÖŸXÍY'®ozœÂ”ÀÖvIS¨g¾h©êéåBãBü¤»ø#^^C÷€¥mÑŸŞxš=ºú¾H'Ü=úá¦R_ÎÎÜngÎRî–9ÕÅ¯v}$2}^¬¡y»ÂMSp†“»bàpG_õ~Ôœv×•Ñ¼@×âÍ}tiÁØ÷ê‚üı$o×ÀD	Sh·¼-iØQü¦”¹=“k)šØaê,ìê.™ÔŠÕä4ÖM¶bjEƒd÷®p“ª%¶ÛâÖeéø¢cæ4Ùj^¡c	ğSí¬ö_[”ÖÖ&0:¼D#Š¸	’t‹óòï´Œ†’?ÂÊÍ†rÙC¢Ğ·‘ø~×Æ…º­(#;İ¡Äİ…ré€b›ÌB‰Èù`&ñ¹¶4¯¶¸ÒƒW”ÏŠòÎi¯zC@9šTâ!!©3|07ùêu"Ê÷\ÉPHÛ*‚…ã$¢ÉWÁëcN›•†ÕåÃ”Í½³×š³'nÎ›§ªıû¢wĞ1…FI%R2)EYö¡¤ÖGh‰Ø0*˜X‰ï¤ñĞÒ2Ë¾Ï9-pP‹nÅp}™ŸÅŞ	ğh>zñ« FD°FwS»-Šù@&ùşµ=ü<İÌ;"Qèt1/o³GåM­¥w?©ÆbÉSÛaÀf«Ûöbİ£æĞøsˆ§‚A‚@ß¯jchÒIÉ”	ƒ¾ :¤qFş{–˜µµvë*RÂ›5Áıœ“«„4Êğ˜·XÆ¸e)Væ¹†X«­}­­XÚl^×1®)¤ÑO¼ÀÓŞ^6r6'Mœ×ÊL§ioğ‚cïÔY…IÊ>´ÛY}k÷qã@P90Ia»í¿ÆXcBjjê{tØìÓŞÃæİ6òïm æÕİ¿,?kÔV)4ğ^Ésa]{-úí o¯m¼ÃÿqÓ}GtËZ	#–¹ˆ®n9‡ÙgBÀ¸o{·pÀ .„QsC˜$6|ùdl)OÙß};œ¹¡" ğb8·FŞsª;|ë#3Ò¤òâ‘£ü34ÿ±ˆì½ÎĞË1f7²é¦qHË®’‚4	Äa{?aä‚H2zØ0u²2yàviÍyïõ}XÎa‰P¡ülH.Îìíøñ»b‡^.©3!K)$"ïr§‚&»R¤[å+;áá†"J¥\ßaãñ¨QŸ¤»l,P“Ê·ãç7Ğİ	89Â˜­MÄÇ€sèoš%E©è<PaĞ¶¥g…Z† …Ì­	±Yó ><ZâY"‚
FoûìW¤±ä¼¬›U4[»_mõ¢$!V8µ¿5ÄÚ¸GI‡Ä”ô|•¿ğÇº@ÿ‘=SÃòÎxÁıjÈ•ËHä
{…‰ã,$¬ª3‰ü7¦(R¹÷¨èZBXàfVß
_?	=OYBÙFË3iñ1S*ş‘JÒ.¿~=O5ÅÍ˜Ó_ïÒƒ]¸Õi…
'‡bnó]˜¶ß>ÙêûĞ2÷§@íŠÛ£)ïw`3Y¸ÿZÏ„ƒ"kÍÚ±r´Ùíêµ³m.SíØ~Ã.DE"šx«Å¢òTj–Wr ¤¨^î¤ÎùÁH-¦‚•6d²nĞ™—ÄtQ AdÂ(2÷5 ^ßİ£›«'FĞº¿QÆÆ]ƒà›®		fèÙç±`’ıMúWÅ+qİú%òöÂ…KåQúãÃ~˜]ô†Qò#¢Æb•‰¤ÈT²+Ó—¥Mğ²R’áÙV.râRŒà ImÃ^•ÈOŒĞğDñÍà2‚Åq;Úóë Á ÷‹Ğ¾"Pïê†P³«ËæZ^“i~•/â¬‡°¥¢®hL]_
¢uÿúF¿KÓ¬"©Ğpkà~‰AäÂ> "¸@-n‹FD¡ZóÔC4ˆÌ‘Äíu”#ì[û—õi²pá%UîñáÉ©²¢p,>
«à	§’(óP5¢Íì
)7‚^û~Î! õıè*&‚! 	ü½°3;Ğ7ez´Û§W(\İZ<PÈGĞgÊ?.N 'QŒØÅ2(æ£•=¥OœôÌ‰—jaÄßRŠŸÛÎûÎ¢2AıŞ9ãQüEã˜…ÛËÜaÏª¢œ? NŞ¯1\—p±ã<˜çÇAü ğ>…²ã¥–ØÀs«¦-l<G¶@Û³ş"y0ÕƒÁŞ÷¤ˆœü&+´ıºêĞÔÂ°1¥×Ë–ŞÂ7âdŒvvÚ$G¨×\É¢Î9p{xŞğÇ¿ışèz WÅü¢¢ŞZõ¾¸•¨E²˜¦ù•]Í1€°y=Šñ•÷:Éİˆù®Şµv”ó1&Ğ[XdÍÉÒZ+	ØéŞ§#CWË}”;’œ±,£ĞW[}ğ9â¼HVèaåX²ÂwÂò3õØ¼g®&–ìÀZ`İ3½Qg¯4*Q¹
bó	`¹„'
É^q(2¯<0ı‰G_ƒVpÉšôê¨Õ­ÌŠÈËÛë™9^‹ŞšÑö_·Ú®—Ï¤5$pè”[Éåe`Âvü=¡)štíŸ¯X…á†2¯SÜ€½r/–Õ>IÖLŞôíTı“mä»¾0åO	(ö<=Iâ;cwT2zSıŞ©N¸  óSÓ‡fßò+ä~s¸p zi2Ù½ªXá_a+oúm`©ÀµYıY’½ RXôNlË¡¢´ÍõeÍK…¿¦Û~1gNËgÎxÍ«4T¸…›±··
¾{™WÁ¿r$ğE\QÑ/äïd,¨)ƒ²HZ‰¬¸‰Is³m¹¶‰¬´<Ô ¤Ö\%f9à©§{ÍÇx·"°ûØ<å8è|ÿ¿âŞÚ/ìÙaP‹-OøWdä£²¹¹ş¯´¥;q[Jùƒ_ælİH‹Es&“¿|3}VqºØ'—½yB$G¹byÔè^í6ä
Sjw4µBÃ2íÉ€ÒMIšr\ÑeVúåµ¬¦	jÂMXWğ‹{óéµBëºR,OKŠ*ÎçµFÒ¹Yå^YéëO¡l—†THPki¤ì–¾!
³pË¿Ï%¸z’^„SÜ^€•zÙ]¯{YÌùdÚÍJàÅú-XCÛÌ$lÇ6Ì}5@lôàÎùÃ4?u¯«,‚¹(`c#§2ÔÙ³ìÇ3TµJ»%¼¨‰œõŠŸî*x·D íÏó‘&d_´PÀ=ùn_AbåÅ&Ù<	éâï¥›orkĞ®x3a£ ¬…Š   ¬}A›ûI¨Al™LhÍo…+UJaSäY‘,W¤²[{Ú^2İïá? ‹_iÆ(W]ŠÛt}1&0¯y•_À¤İzv?ìAÆY¶[ÃŒôœÍM7Hıã›Éó(\¬ö>j,l)7>}Î-ÔÄ:^8r,B±I”¤±»rûQ\WLp(:¥3+ˆ|ßOuÃ‘ñéªn˜{uHĞóNvl‰0¥ÃàË+*ãtå¾)d<2<xÑ{‰“0wÀõyX¹Æ>ºYÒ!ß1+åC’}.I@é¹ã¤ï ş)©$²lÄYtõÏœvÛER„î ”kr;+¢¿I¨? zŠ¦•´B ğÇ±•eg½æ[UğMÍ×òrşı S²1šrÍŠ¤Y’n[J3N¸Éi¨4Ó½ÑÊ/M)2'¿ã1a>Ì[“šß+çøú,IÎ±¿9–O*Ofx¬,Š	_zğ˜VÍ5 ¸@$ø¨	åû
‡¿¬/êgvÒü6jŸUÒe
2ög«Ÿ;ä×&Â¾şµÂ~ip¯ªRC_Ağ%)6¹–‹ ,„² =y…KÜšÚõAVÉ˜XaËo‚<‚?ï¤3`ôavŞùÏ:AXå1¹ÈÇbµ&2ÜÔk«=lÂ­Ã÷İÃï×Pçw„ù·pÂ4ŸészFcÊ#^IôEºÉğDKcíóécCz/ú—`ËCüvü/Úî)ÓÓl¦=·ëÄes$ğĞLâƒÉºs¾Ÿ–°l8İ¶â.–ù‰I”iVâëzcï-äq#tëX¢öºğñ'e1“yOØŸAXI}À½°lu³!aPÙ"Xv$é•ÙAu“™+«îÇg¸†ãvÒ“Œ™~fœÕ:© \ñ¹<|êº(Âäÿš0å:¥ß±>Âÿ¼¨õaë0Tæ)¯¡ã;A¤,gŞß½8R­ìv2ÇÀK[eéÆÑç=+_k[*ñ$ïÉÒnH³ø€Ûß}Ñ"&€ƒ9ÀÎóæŸVjÄ[K^Hcò¶ZF[Æ‘s%WœMK¢8üÖ­&“‡÷IµĞ§Ã§¥–·Á¸œæqøñÚÆ'‹C9‹À<Õqe›‚Ì/Úñ‡€Û>$Å‰mÿ7Ş<f±¥\G‹ÃÓÇ³	w?´òP¦ôH7çON›—NÖ^=£LOQY2 ^¿â›7·dJlŞ*jÃ~ÅuNróÌA¢§Ä Ë;œıFÇÊœ¥
wÇáVíL^ 4Ü¯iØQé?; 7,è
}½£(Hñ×Ó×sØâ*/ğ4ü÷ËŠ.€İ“ù©hQ€>4Ÿı
i| KBÏ.‘asi±´Dªl8ïb9œ‡;Óxs¾ŸqiÄBÒÀşÙ¶¾Æ+QÎ*kG9(>­°î+·¥CI ˜a=Rv—´†×ë!C­œE³í9T÷¸·LcAX›'{˜Oöb}YjŠ9’‡egg˜1J."ÚNlnóÈ‘ö6ÊSQÁÍÄ"t°H¨E‹’ŠLÆá«¼…ï_ôSqË•yÀi8ŠïùdzÏ§\áNºy•5­ÑåŒ‚Öp(İ#.›W¬æ–Ş¶è} :x‹Ñk/£ÉS;D©Éõ0’“ğ2´ùßJDûU|ßš÷¡”Å’<¼:ë3§7Gb´I×ÉÂSwóÈ$4M…‰‰(_¹E¿¢£çsç?|Õ+¯K¹_m¬ï%¶êkŸÃ®7!oÂĞ.m ¦ÊgÜ	«y,÷T%[q¶¾°oøü,±D‡Oğ"ŠiÑHÏ–ŞñElÚ èX6İ°ˆs[š0‘#Ñ’&Ø‰jÁñ4Ú¼e®Jd{^ØĞL@×h¸D412ÅĞØ¨_ÊAŸ£Näx¯k=lÆğ`ª $¦íº…×@•iğ™û¥ooA"vÀtŒ:IÛF»{4&|şüjx±uÛtxf^/È›à_^FõÃù|0_ŸX¾†¯?KœŞ“'ŸGxÂÆuæL®^ö1|û÷:–†ÀÙ± ¿UpÕ|áƒZ6×4<è« 1x±¶¡‰Â÷R Gš$<ÉºT÷fàÛ‡=ï»¿ÀïÙ¶¾ñ™ƒ&K¥JÒxŠ»…™¥ÄÀôouh™Ätœıo
åüä6æËjCÈ~*˜~\.Âê÷ûbòï÷ë‚¡C”şs¡’Q¯wb-ÿMzv uù‘û0N¯³{G_ Í'_"ÌÍ^İ-¥fÍ :ŠhŸE¤V©8ˆãkObÁ{ÄšgÙrğ?ÒÅ;lïÎŸöú†öş<ÆÍÃ]	Û«àË¿b:ä­.TõØì]¢‹!Äÿvt4œ-İ/ëÓsÇ%8b	-¹ıghÂÒí@#ÌS5EVè¯\_”îò¯6×ˆgÍ{… GŸ¨9)~·tg7ºÆ%nuiÿœ!´W½ï°fh˜ü¬D‚3c?“˜Ö¸C¯$Ï062€ÑúÔ}É=X–Ì²F»šwæ%¸"-ã!Xh¸Eïí.•ûXì­xˆÔ!dßÇ³úİ ¹÷@UßşêÚ‡·Ñ/(Q(ú“*¬eŞ‹_')wJ ŸöŸñÑ!gÙŠ”d1ü%p#b­¹}VÓI@:³“”71szcª–§¾Eluôr?<,ëê*çöZ@›1cGõÑÀD¡ëoÖ¼=Ìy›>©Ì-&]2æAâ9RjÔT	5ÅBÁkZ¾®ºŒ®ïÄè¯òÕ,î8È„â£4Ó±ïÒ!AtÀ}U® °„ç5^gîåW?1‰ìË	)>ÌRfV.ı× Cıùr~òªĞj¨œëú©ºÌDç§³ÈmuéLãÖdü{s«;Ò>Ù“q‡âíú‰_˜T`‘V-ñe}"ä6İéô†ğ§½†Ê:Vüş\a/°™…İÏç²ª:)k#ñÎ‡˜F8=ôÕAğ†”ë~OtÏ;¾Ç/¾/÷¬R²ø¹T»Öù™È×h}qÈôŞo%IğÄê4Ìo:ô.y#µÌ¿¿%»8ëŠVĞõ[n0‡§NVXëf(”•lY¥}ş¿|ÔÈÿH¹Œâ÷•öòUõÈ†¸Š”=ùÀd„ø ª}Î8xAª”Ãz&„–c\½$LA·1ëAe›¶ØX®;k|&mCÓ[U—µ•³M·"É­Ñ:àY] qU>ÔKEIŒÇv"N‹ÃÎg;æ„fŸ©É³öh( =¦
T[ ·Û.›æÌâªR
ït&ä_vfÀ‰4F‰à7×óóËÊ5Ä>°8ßè†d‡)ÓÄh±¨§\‰w`G}«|ã+uU]q«B3k:¦~‰I4Js ¡ıo¦f¹%Ö0@5Q÷Ì4él)©¦jÜ¬•ÁßÛ'N®Æ¢ßn:–d‘íÂ–adöÓïO–{muYØ·ï»!õúVhÆK|MÅ®®©xà‰aÛŠ\D'†ãÌu_v¦ˆEFIRSTk"Õ)Vt¹–C…Ü‹ñ¨PàñO¹[*|Eu»ìQÛ=Ãí ïŸ{ÍSYÁi´gN;àıÄ‡ÓQ;äî­O?ƒQ!ÈÈ´³m3
\m`íDúC»ù‰è©9 ò$½´WmÛ>e(ÇJ¡=ø¨öí°é:ÉÇ¹øÄlJ{wYÙ(Ê:(Eıú‚Y Ìtú—Õ*¯ŸšÓÉïÉ=†˜êùL}F¹í›+p‘¥°wt¸ñ“ŠğéYãÛ
1U¢s8ÖöXÄZûà¢Ä¸iøİŞ5IH4=Ò%‘Ú24z²¥j‰„¯İå0C‘Hf<Å¹ä)‘Æ»©Uáˆi)¢Å÷ø¤âğF*ìK±yÉ|kå
Ú­/Ú‚fœÚ#õæAåÈYS§ùÿ¨)+ÒÚ‰VEq{²D=UºûœÈ8ü•Cxm·@Æ/^‡>¿ô¤YtcêKşvŸ‘¦
ªÁn½KZ…ªîxÓç¿*[°7úµ¼¬„À~z¹_j”/q9ÓÅñ}¤Î„î¡îÜMşÖWà¤F‚®á*X”£óBOd¦¤ğqÆ¶ =@1'í™[zÁ^mÆ$YĞÚ±ŸXÉãÃÉ‘ˆŸ–óø·ÁÊ}DØxÏ*BC}‰8—Æ›¤6…óRbsmÏy`ãVO\ŞÛe,š}‡&@ãÖ›.‰²×QÑ)a9µòğîlÙÖd¼£ö&OP\·ÿ’œâÒ·Ò.¼'»Nn	9Y¢8örSåfıÂÛ‡®ôv•¾3o–~Ï1yƒ¿à=£îçöÿ†¬­¸×j®*CÆm”Ö±cbˆX“Ñnğ7±H²D]XÍì®œ2ÁRKR´Û„â´Ùı‘5à–M'Ğ.…ä&O áVihs’qüÇÛ‚ùTŞ=Á¥iÔ`V§m€<9ÍºƒÜ;ÌQ3ª/8Ø¡ùe~…õ¹û†,ú)ÇÃÂ(ß³Éßú.á$‘íq§	óı»„P“µ†à¼?>A¿W³RŞMå’eù0LE‹ÓÊjqîä¯Ø]Ë?ÒíŠ»x¬•Ÿ2ÆGE#/w§8ğÎƒ‡ôŸZ9_Yc¬² Õ“ à#T)¼»2”Ê‘ª¦ëlÙã›92XÙaÆùG:îjş>úŞô§±Êõ¾û>ş_ ¾Äö WJpØæ›û5"_ÏÚ£Îà]w!ÂmÚNÛA“:³ª¿ÜÆ(è †Ë£¥e1Ò²Ïx’èeÁWDsœ4=“é1à{¼ù‹œf”ªX'ñ¾R¶8ƒÓÑ„r(j:Ä‹^¢Dë5D©á‚C$·ÎuŸäal¿³:'[r°†â8ÉD¯hO®ö¼‰°>œ4w•'¾êÄÏCíÙÈ™°'¾ó9=ô¦áÍŞ`%±k¸Ç¶¨ûÛs¾€qÖøŸÒtOíÇˆL±+yí˜ûvê£Y:İi°2ê;!60ø;Ÿeƒh¿ÉEŞUXx;Õ>[¾ôLŸ^¼˜“™äÌ=\wÂØÙf‰ÔˆÕ\Äÿ5¶™–}¨L£I6š³ËUá¥×«X¢:æ¤ÈÙ‹CmÓÜnÜt 3Ñ³Sp.ğ“xH	öÉjMŠ­ĞFÈátâ]Ùó¯±=]ÄÙæùF?F!œcFimS$>Ö•+&%’!7[¼—„+cÏÉ)¥dÃæã~…«ßğ~Rå?!–	úÚÈ>òkO_F&h¯øŠÍ´¦±]tO\+%|0Ş]8cÏÚô‚/ã¬È£ïÌ^_âtèW=¿W 6Y“+²«b»‚iãV~@:CªœŞŞÕÇ"H2V«JÅ¸=¯9ƒ-0’'uy!ã=‡¯L?Ì0ÌóUÊıe1f  uô2…7X€Ê¾‘tœ7R8ê»d%Ø ¶™ë;ˆàMDXñUä.JÛŠ„3êë((2s¨m¼Eü |Í"–O™ÃİçI2Š°"<'şÄ,ğog¶.­HÓØ7J¯,¦#¬,Ÿü.[=ù.+­[ÊÖÇ 37Üß‡H9åÏr¨SP»×Ğ©œS¸Gt¯­.Š‚²Y¨¡ç×§UÒîœçê'ŞÈ¬_"äN‘KÜ¼PÄÂT,Är_+C²­İT‘ñ½¯ŠÈ¡úßŸ†±‚Göú z6	XÓ›MœX¸³" ×F –,ëßÆb“æ†ï
9ş™kŞYt.§Ïü{wkJujˆ×ësL¼nYã@€²v’üÁâzÈÜœ*ºK,ŠeÙá“üeáÊ¿
‘éBÁ®Ûšjê[WtqlÇ.¸%å>fmŞáå<˜±ÿ¸DX.AëAèŒs®Ïûu¸‡K*“«‘-ÛO%Í1ü~Vé“I×[m2W`"%ÿnT9W ™!ÈŸÑª~ŞÅÑ˜ ˆ«iPBncÔè;Ş¤æ÷yĞÿÂxbqÆ¨"®I×†Ñ7È[Î¡J¨ÛëICÈ>™JèÖ«ı·¦Ì¾ÄÈmj•¢
D»MÄøšÕé1…M#ÎÃ¨Sò“ºÓÓz„—ŞAæ.±&Á?êèZ—°×«ØJ®½%š÷‡bE3ÄD( 4’¦BqÆEÈ–P¸µˆi‡bIíÄõeu+R[eM¢·=F N½‚ûúÊ¯Ç©Nn%ÚÊHæ\¼”@Nw»ú$ZªÛ[ûøè<@ã£1Tàá€Ú†Ü·ŸßßşjšËÎ™¬:a'	IºK`)VÈzÆÓÏ7[Y×Ò‰.*ú© Ø—aÎ¸º¶§DÎš7ÿâ)/×BU©Ü¡·òÆŒÑXú„Øfu6öZÖ‰ïŸÔ.C±¤fQh<ceDÇ¯Çš"SšyùxÉ€ëÛÛÌÕî/K"­‰Eâ»??
jå¿ŒW?Æ‘óÍ¿Õ˜CÜ)RV2¼è3íx¥/ƒ]‡İræi®şåx*¨Ø™ÓƒÊšhx!–{½¼ÙOóõä¾-l¬QMÙ#Rî¢§	²Æò¥¨£Ú¾,àŸÁÒjy¯'r8|M»0³®!fC¸ı²{)ÏSƒ#;Ûº£ÌEÙë‡¤!¸¤Eï“JŠËôÍÛÜÜy|Ó²ÅÍ(â‰“n<PS³ôÌƒá½”¥°İ¨­^!ŸÜ.\KRuH¹—Y¤ª.Li£á‹[Ñğ6ãÁw½ËÙ£UM ğŠ.¥öíR—$}k¡6TïàºfyÕXš¶hÿ)‡Ã€ôcûa°±üX_ÒvÅÆu*#ÜBêï(®o¶ƒvÖ!]š’xÇ½HG	…“äOšŸ„±Lã³m¢vış]õpV4Adë·"ñëş]äfáÈ „YÖï£şáÖR*S–Vú™—§ü^`„ŠÇ¤@)ÅO.ë®®l¸©hù§ñTğğ;áië˜:„Ê¬¢ËóÈ/ó~*ï½`¥?]šóõtÿËå¹=Ñ.&%î<æ]G“ñ›¬C;M,w¤øË½84²ÓŒbér¿ŞŒF›6G0VGÜÃŞñú½†ö³ŞZÀ ç´¥G±z§ÈD >Îœ²‰ñØ5ÏÁ4½=™ +EÜê‡­gXKOı‡t'•¹1”ãÙ‘}£óñè54ëŸ¼Ìí×ÀllÑ–ÌdU’gğ00ïÅñ†}”N~/å‰P§BêğO½÷r©iZ ©µÔfù“`ªNß)á\2!cÜa.•h„!QEa–êaÙ¢§^>Ç¸zï®HİD1.­u,Yàø•mPÈÀ·¤LÓš '†¨SÂj±Â•ğW2ÖJ‰
	ÚÃµ^5úrÀ)× Óã2²_X6Y\	‘&ıX%–·J8…CİdDOSë¤„^³y2cñÿ,ÉòŒO™§Q]=kİà~ T…w(ÃdØšn<ıÍE#âgÜWÈNaÇÓ·À°ØğöÑŒ}#¦›âûºw62MHT¦åkÅİÉèQ {àãwÑ_jÄ$°!%.íÖQ½¨Ÿ“á)*Ü)uùÿ.¢Jó½4Ê‡Lúî¨œ¡]¨æ$'Ò<*gÎ’ÒÙ¯3WŠÊ&n²û#û>XŒE9¥„/ØLô©Ñé2ôGU>¾É.á\izšvUÙÖ¹šÙi]§k“/uiÉ6GÜS\Ï>ğgˆ9ü×Ÿ'—Ğ¿o„v0
Î¨ºÅo×§è±áì^¯<ÎqÔˆçåÊÀ«–$ ²Ço‚ÁD³n²oò<mS¡Ñş¸>qµğäàîÕª7™OL
¡ş±mm—+ NÇTmŒ
üŞˆ± ÕÃ1«´áÇíŒÈ´?úW†»EŸØÔÄŞ·}»$‹J$_WŸ
*qŠã¾#=¤-•{x_qÚE¬Àãáh|"ÙÔé¯mfw>Ëu…ÇÁ™]ãd1¦ÀPxË%ƒüj…ŞBä,f4óÉ@jäpF:Kn\æßyë½’q"J‰¢ÛF^µˆi7Q¡ûÓiìeWŒ˜Øi2¾”=ºÓùJ„È5æIµ AIõ¹İWû¢SÈ>~@(>d‡—ÙUIÎënïèSû:ÊQ/¾AY-ıM›ˆxŸäZ¨š„…òUòflT-<¡#Ï×ÂsØUáo*l,ÒS<B±ˆT›ñIèÄI®ìÕƒæİdIëù÷ßúP¥èqĞß"R]Ñ>†6î}Ø+qÄş}D†IiPB°æ2óÈ(>z3PÄ>Åk’°/*Õß	k>…cÿÌ›CïNˆÄ¬hh"]KB X¸ë›~ºÛŒNš×ËHè$B*bÖşft¬‹ˆ;Â°¯Ş‹/a`Õ¥‰Ò*ÉîD×İ¼Dšò^ı/eÛøÊ*f®
Ô–•®°¼€Í4zP%é=ÎûŸ3¦í6˜İ#È¹¨gœÑ…<ÖIÅG#eœg…÷‘İ°;‹”à¼-TĞäıTô²Õ¤f…òr†¸"ærô´~ğâøpá6úÑa±Oñ£•Å‘‹õ~í#ğzp½••ç§“æ·oDªPMÏ|¥o£GWoVIaÆÿR‚†´KÖÔ= #"`óšá,Ï[g[ŸOÙ’\[Ÿ%JîÚ kÖSOÌi0sÎ‘_m!“q8®–	69èp:ööâQ´“¤ÄY,2şIlãªèõ•	SÜFÈ6°cç¿‰1éaè‹z[—qŞì/°£Ós„íI««œ—ĞN°M;üB¶@ùq<š{GÆõôôŸ"²1Å?·?ÜFL1Òû…û?ÖËó;:>.ºƒk4Ñ­P¦‹…l—@L’2ºPz^t½ú=:ËÜ*èûÃt"ºÕP%d­€ê(	^vŸ•°xss=ˆÁ)æ]<¤S”l¨Û˜¿w#ˆpÉí¬‰˜Ş!“¼¼AJ¬YmÚ›¯¦/lº£@á-Ñ]‘b9	àiE‡û]û2ß¤jYğé—³„MĞ­*J”C0µ­Ñ‚,ê9Ø+©&á¿ŠõM	xÂû„fÌ³ÏcŒ0p²¦,l6õ½G7%UjìOX[æÎ	‘ôù)CåiM—8ÍÉhLp¡ƒîÖ1 ešÛõ8TÍ¤õ:j,ÑXdP›g¶ï `	Ğ³ÎK§/óêÆìÍJ;Œ¢-³3¬ ¦ê®êSÑ¢‰8øØ(qĞ­½j+«ÙˆaÀ#{¦ÀtåˆzÃæaò.Ó×½ Ôëò¬[›Ä€5ùÎ[%q¶œ>N$§²“E…ÖÒöTp¯DI¨|éÅPfüV¼æ¼´7›7YÁW²¤a•êš¾wPmÏ§‡fzX¹x¦"yÓG Í²<|f“˜óx"EÃµÖ½—¹NoDÇæí@1@ñ–&h Ez-¥¤ñÊ¥š²˜şâ½ĞÌ?’e}ì`lW½nĞ³¸óëgK ÷r¶lãÇŞ z¯C“iö?:ÙÂÍˆ–ßYbùø§‡8ŒˆàAHÙË¨ßÖ/6CE”í’[]v!rË¶ÊŠpkËl¾ï´ı!Rá%ªGôîé+ó¤¥,C‰¨ŠUi™cŒ7¶İ}8ıu	t ·¥™5ñœKI½*cşåH´BZCNñéc’·ÅgqÚ#B|~\~½9¡äÛkîØ·Òv}8şÌF9Ø³^f”ÉÚ0B«£u´€>Ùe91l–ËØ4>iø™f©êoğ¤Ë†Ğ“Ï%VW@¡ÕÑ\ÊiçQ	l¢ °0‚c.«,i‰ëŞÚ©Xº5ú<.59^ó˜§Ç5İ»ÈÌÇÖmÃ5$.´”ø<Ë‘óƒR¦œ"LÏ)=HbÈ´©¢qGHpG-&ûá¯ñH
ˆ½x4„,6T›˜°%ë úœ
uÕŠ>ó]ö)åyÍİšÈõ¼¯×íD l’–C;Aÿ¥k“â¿{ÚÜĞnsñ3Øˆ€X¡¢Ê	×Ü	!«c¹æD³şMÀzvx¡ıùÂORYÍlF‘°Ïı|¨'RSĞ†‚
æé(­Şë4Z¾pZ?‰››<XU„]\ÊöÈñ0RP0ôé¡`…]†¹/Ëé¯‹22ÇÊé@CŸnZ4p83
òÿA·â¸æb^¯ßõ’ÃŸkqìûyü—i¡²j>R–Íì;ô\òÃqÉ•ô¼P²‚$·ÀÎÿßğü5óiÌX²r»„¼–P'ÓŒ7áş$Ô¦¢tÇ,€†™—ú§çcÆ –”åchàj¯®]e•¡¬÷ªáĞº,ë>"¢³ì@€¥d=k‡ÚZùı@(€B‹Àb“{’VNøŸÁkì.ŠÄ2ÂPÅ¶e-á’•í6¥óÑb…ã[Eìh¬È²–WœsÜ'{OöØ'<€z'æöğ¾óbûğ—Gj”oëÊ

®ìS¬Ç2CZm¸¡p=y.ÓU²Ó†<ãFz÷ö¼yÄS ŠÅk8.B?XÉ°sjˆœêi‡)3øO¡P…Éu¢vd ı2¿ş^±·{öµ&´^Ûz®VÚïò&şF;BÀùYø‘åœ§.1«ÆØ}´ÌÍAº*3û¿ÈÊpèe{ÏûùÑGë³wù€ÂÜ!–÷!îÛ+NÇı™“I¿~UÚBQŒaŒëe®pI¡4ñ-ÍØL{¶Y\MŠ²Tïˆå({Ğ:Ki QØs]ŒÙ1ŞJæ´^§)Üˆì5ò–Å¢£ªÖ5rk×œéıB³‰ÄïÊòÕ™èŸÑ"iDS¡{çN )ì?ŞÆ)hr÷ˆg$](ox YIû)hûKí‘ş£«ü¥…§¯Ø’O#£Ö	zĞE¨œ™~%×ys`ÁŠf™0[ #Ú…"¿5›€©ë^@*\OdÎö2ƒæ/À×ÖÚ‘ÜĞ#Œv·~Ó~ -¬HşcX-ş°ü§'a¶|İN¯nuâºÃ¥â>ÿätzİZKAZJ…®%¸?ä…C«»á•±`D­fN—®5Ÿ8ÍãD£]N~wz}	ÒºùäÌ¤0Édr Å„ø÷\ã Ù€ˆŠ7w ¢qˆà˜i	Ú^¾>LlŞvO€è?äş(¾Àl
ÌSŠ	~7YWSß‰ûa© &$˜‰C¼vü´úÕª-!ã\„úĞ~"7 U!ÃYßNÚJŠ­ÚÄ$¤
ú¥wò²ª*>ì‚âïë"ªqóª(xp˜0¥†F¤„ÆV;3Y6ôĞıCÉ­.å¨nì‘YíÄ
}×®nP-Ø$êM<ótLà×ú—Ç:B@¶…­]ó:üÀ#@'‚!Ò‘¯Äâc¯Y†Öì<dëbAöş‡!|5äLL1­Âoô[;¬Äæ•°Ùx¢øå‘Î…7¨ôAëöÌz4ŒÕJîÚki=,;¸ëo¸¼Æ DD—kFç;°dk™J7HŸ¢~‹ı‰Ò–j¥@ì›ÂÆğŒÅ5îê£=6ÿ3³kwè&ß¼Áí
2ğXjô‘Ø•ßªèÆr	û$Üs?£’Ç7¦y–ÂÃñÏÖ‘[C¯İ'lLq›Amù³şÈLÊ‰ô©IíA¦š¢-Y~¤¸İò qAÒñ­2ñôám‚ğ™hõïy6³é^zn!¬©S–/”ˆ«ñu(ãK%AÔO™û ı”Q¤”ËñøóÆ·#ÎÑµ¹°kç.”ˆí»*ºÍËMñŒşŒNP€YÚçŠåhhã “X@Í–²ë::¬ñ'â¤yxÌyJ2hâêSc—3ŸšÚ>=½=¿ZuAú4ö¶
:7AŸÊGÙùñ!ˆÿD§7¦wı}TÿÌœ§)"sOê¹s²™³í3Tôâ…Ù}÷óÍõ½½M¸útYc³šEİ˜tæ$µ$²Ë[¨‰[åÊLHÚjvi¦„2™N>;éGúª‹p´÷˜nøÊ²önu%÷Ë´“ìx%±ê³ã®
vÇkğºø vP_„Lo7U„òÏ¯™H$5V&†ZÚ1iÁşçœ	˜×º²0ÄÍÉˆYÁø¢&ŠÖã#ÃG-ÂM)|ƒÀ¶sz8™Vìúpkm°$‘¶AÀx+Î¸Ì‹øÁ^ÔÜêåÄ>ÓS‹«|„ Y‹q?¸¢iâÅ>5TŞßøjç½o¹Í0ÑZ×gÒ<éƒú ÓíiğÌµ5uÓ7«­¯¯óÉµÖÃ¤Éì™á÷>4ÉL<Á™…§\Ğ(»w‘«]µ¼òtOÆ—;¥æËöcş‹#ï®R±Ô˜:kBJOÑŒaÒ-æc¾µ÷ßm"¡a­¿hÿÚ#Æ¥y7é‹°‰(÷=Ì¾· Î3¨Ïº—PÙn‘¨æ ±¸öt6ÊŠGñ´ó@O’C–™–^Wâ nªÜÈ‡ÍƒğŞ×›…¿Ê“NºìE½¸ÈThrœV[Ó&@l 7âœ¢cA{ßŞ¥rªÄ®¬•¦Ì…¢ÏªœyÃ•qÖ­/¦SCV×aAš ÊS|©¬ã¼XÌ²¤ğu=—ãrà8[)I>:¡YwñÆï3‘gwêÓé*,ä7¼£¤Ÿ"§Ş-¬x$Z&ÓWóáy¤€NëŞİÕYUªg%.-0|€Qæ /V/#öú,r’+I±ĞŸ­]= Œ”mµR¶ñªı¡/ckM–Æè–N¾b¿t;v<X„PÔƒ”çoùî,ÛRáí"»ª¸hğë†‚ôl;±}ßÑ²ß|¦sqpÿ*É¹ŸDFp²§B3X¾9s„4.]65ßzÙ)ŸLã`«{á Êî¶ëÜİ ZÆ·~p\bKú¾?
ÍÛ/îx,I¾ıc<Çı
àìˆ}¡İÀ2WÅvâÕíÈ¿œÙ¯„_.¹ş'ë¼ùF‡ÚĞ¨³ösšÌ;¯}5¬JÔÆ«<huÍ3¨H+óªOÉÏá¦ãê(xó¤òM†sEmOÏ½."¹³%R-#Iğs‘VY!¯_*MäÃJñ &²õo\«xâ›Q}Ì€ÁrÅÜKA„HŒi¼Yú
‚_T»H¾u’Æt.¬À^äaW}êÑµú{	KÚó-Ár[¥éZ²ûñ*‡Ü~şÂ•Ó¦c¥p/¯mõş÷ß¬†ÌÌ&:ƒZz9r¨Ç°~'÷×Ë²ıòÔÌ	ú]niYD#›è;ªí`–ø1²'ù1LÜà©ô´.c‘}øó>Çèæ°¡ÈA”Vr·Öİ…ÙE^Ç,uÁkXu,ıMÜ+îFı‰hZ)[™Œ$‹d€¶s&®¶åˆø,éAÎ‘àJùÖ¬À/ı¢¸sğ(eàÜ—`&'éc"{%ÑüµÍÅZ=Ø2ÙPŠ9ˆ!œI¶jÁ›‚0´fD‡°AeVOkXp–õ5Â_ùp|Õyïö¥Ù‹}·œ²ñ¡Èqø¤½e¥º-àÒ¤MÌÇ{g^ä†ŸÎØ#1ºÇ™…Å;šÛVy»±éãz–Aù‡ îæ#j–š—TæZ­à‘íØÌ\ÕK=»t›‹E/¤}*ÿ9>9IÃÜ€û0”ºmš»’IËödãXWU?TC*ÆeU¾=ò%~šçiN©ûÇ¹CHÜ¹ùhq¤Ë™Qgoh‚ëÃÕ$ÓkgèŸ1]xT0§8ôæ[©äz5|Ø«úœÒŠ‰+ÈiT¨ÎâÓ‚D‡ò9‰·0)Ÿ°ú2›¹Y0ƒöµZıù´RKí)96ºÀó}× .&ÓKÜiÄC}xº$çrÁ|sB°vŞõ¬g ŒõÑHöªĞĞÇÃ‹°¨Ô=4š;ÖúÁjQo\T•™Gê•ù²ô>ˆs{¹r'‰…Ë6úØ|d0¬HñHíŞxìµÀĞ¥âÑ˜öŒÆŞ§ÛÙZW2Qí¨n÷%/´ºÿˆáÆaåáF'±ÄJ¤|™m‘{iğìa^33`‰–dÈ¦ºÜ}W	ƒ}¤Bë‚?Ø 8rüuO1
m¸ô\Äó®Ké0,ùÆ›ñ.Õ@÷˜Ô¾Ù¢²¨Ïo½X—…àfÌê?"‰ùgß?w&Ï¥«c9|W½nÂ¥k²ª€î"Sá7èJ‰ nÁqîğue—î•öà†sê
2j0r6.€ÄÃW5¶0ù©xàŠ!RT.É“)Ş ¨W¬¼¸ƒŠqÁ¤ü(çÔAGBºq_ØÑæ]å¦ªIĞiüàÅ[Î¢DQ@-Jü~ø´xªá y¹hX¼¹„ñ´lë¬NèX1Áã$â­B">ç*vìz)ĞÚ¡@jÛÉ—Š‹5HtüäQ Ôı840ënØú1fwÙ=,×UßhÆ¤E™Z˜gO€/şÄş…J¤¢F±¼ÂøXì6³éW*":ÂD„6 :ÚNo =Ò¦<£7x·¾.4Jøê¿ŠXŸ<•xà*âb®­şKÃ,}¿!s¿Ä·«Ã×ˆÿ`ş—;¢l˜µDJ!‘=$Hé¨²ò_ójj×õĞT¡Ö¦ŸXkÑiQzSwÄß¹şrı kp~AôlÖ ,çtL›ÈµŒîŠ?å*âQ}—áéÛ±£‡—pB*Ö:®bx=ÆêaÉY_ Cöé{À,F{±­fÏ£ñZ¸V Ëñ–ß,Í¸"ÿ>A›³¢'n•ÿÅ5ñµĞZg1]IB¦óü|4Ğœù!ŞÉ¼›Í‘A¶OˆÇ¬"¡Q¸¶dç¥\õÅ|º¯N}Ñz„—ùŸ-&âFWÔ¹˜%¿A%ä’Hô´,wÊûÔO‚ƒ(.r™ „»¯!ŸGØw"Î<#š†Ädàwôeµ+„Î•QXJüV“ªÉMi¢_%úI55‡^ûÇ^@†ÿBwQüùPmW»Ã áO¥Ê¦Á×>ıwõ{ëU+¹K;äê	
½^‘Ü~_gó{•Â›1ÃVM¢5çƒÈ5!Şõ3“ä©ˆu¢ne3šÀÿĞ—„¥ø	ù™˜‰ÆÏ»	†
<¬&‡}ä`W
â|(xRÛü)ï²ñ(aÓXq#Üİƒ¹V«[¥L­±nÊøÖ¿Ù†ÏMfğñŒÙ€ä–+, ÷ãJ1ŞÅ‚íÈ÷6²Œ„]É.ÛMö6ŠhÚ­)àÑRo×Slú£§#†$WÏ×¦ÒV½DCü¯’ÕŸÖVµìSwå"t­M4j·0û<£šÃ*„üœ½—ß½‹¼ªl“Ì›!õKñ>—óÁğã›’à]mùÃYLş 
Ö¬ ùËTÕ™K.4ÍFx<ÊFx¯å–ŞÌ.Å*x&)}»æEµu³8ÈMr7˜ 5qX-<®–òöÂ·H­¾?0ªüaŠ ®ÂĞ	@ÎáŞøZ}îÎ‰ÕÚÜNû7ñ{F ÿ•=¼°È®ıÈ„öø÷tØÁßQÒÓV¨òÉ‹Îš§{`¯›:©âß`übR<¾2Ï”Rïû=/_;Ê¹ş§2+é‚@9¾4¡ ÂpMŞ½ìÄCåïQiúäL‚VL¼=¶K®Iæ}=àYštJdŠ*‰(ÕR%§Îyo7Lbx+ï­E}+|Š4£nj¢H£28Ÿ°<íÜiqZÏöøß˜T’«ñ² i’öZ¦#¡à‘°İ2mÕTª[~‚ t¡Â	Ê‡UâÎÌºãù–N0Ïy=¨EJv—“şë$“¿ğyø¥uîXÅağ¤pèMÍñÉëšéfÿĞ)Sn+Ğ	¦ odµ)¿sMCPÑIoY8-‘z$ÛÕiRšÁÁ.üZyÇDÂÜıâ!ï1òŒE=•WN™/j¸â'xN'7óé£RŒÿ÷íM6DîõAÑŠKˆP‰±cŸ{yeqŸ>`÷“œ¼>BDXõÖ'í:¯Š1wú™))áUÓ:k _§¾öŸ`òb¡ûeô¸Ù?ƒÚ³èÎ3gÜû(²ßŞÔ¨n8Š{ ™!é•™¯¡zûpyygúÃQïİñD¥¤o{¼+”—ERçfÙxÁ—´ö…Ìßî%Î+-ÒÇÿ×³¹6ÉS÷ºM×)_Šk×ËŠ[¹—× Øs
«
ƒ&Ì1à·R¥ÑÏDk¯Î= &£¸Ğp£r!Æµr~`z¤"½JK2c(íIpÛ05gÌ.Ÿ*±^¹g‚çòâ?zÔÇ¡Ï¾kÇõÁĞ€ªX¨uÊg;ëâW†©®–\Ö¸€Ñyú\kú8#Húj.™zT¨(ËI°€Íy;ßüÑ¤&^Êl#í2ÈÎò~8:£=•swxbk>•Å>äé
ı³7…‹
	÷p3@3ıüã^vG—½7“z#Æ<´½äÄ:i­\ÙF'~Jfô
šÒ‚l˜gşvIî\£—öQi™âß´)pû‰·Z3/wÑÈÊ=q=äxĞl´Øc}x‰o˜÷RßÓ, š¹ù”‡p?Y#¼.hÜ¿Äs„ºtÍ€ñŒ®Ú°à} J×MªD¦‘'VO£ymèV”~KbøKÉ–0ó€÷vÉvšI×€ptu)l6Æv`ÉÖ»pàê›ZTYzÅ†Êc¦;ø,°¨ô5l!á(‘rÓet¿ƒ‘V3iiûhÿç4ZïÖgjc…ğ;ªqõ×+Æ
ŞZmxÒÁk€˜Í¦:K?‘5 Œì4›e»û³ >«úhÖ3)­ŠˆøËÉiïï+ş¸‰ÉìİŸ»`¾yı˜£ªD¥ƒtO 1«"°7òâ#áªÃíõ¤’lnÿ‚É]n DÚBÄ·#×ıDflÒ9§êM±Ú )ÜÄJª©ØßêÇzp;:B{¾¡âNíä8­SR"Q€ÛÚl95^ùœSªØåüÍ¬sap»i±ˆ÷:ğÒÎ	\BÉ4N;Ò·š?äl,öæ>¬p¼ÉXp‰üè[
’ğI” Æ¨Y êÃY ĞËuöÿş0+oĞ@ÔÉ œÿ=©n»Üõ°v9ÎSâYm>5o¤†Ñe}4ºßö5 ey~Exğÿ%†÷IäÜ¤Ä5ˆı`Ì	E"*Ú±5©²+•z&ã8k¸TŒù³ŸVôš¸!Ìÿ…™›¯šíã®TÖt¼Òi÷e'âŒƒhŒ™è¶”æ s§çÎg !«MoĞÌ²ŠSã§½_ø¡pf)Ÿ5ºgt}·¤êÏ4 ¤~{œ2øÁUú!CP¬+kwºä-Ğ¾Úi"s¦•8'3f›Ğ÷MV§Â\ı4¬@˜°
y.×ò—€a?{©ûÕb4†”pšÖ°¼˜´)C«?}?v/˜‚ğ~t …‰ÍÑ`Ä$‰ı|6Y;*´nçÄ‹ïq_7Sä­€§v	,wŞ: €í?Ìg¢õŞ9AşÁw£àïQÿ"Q§~MÃJdÀ^'ßòÃD¸‰GoòR±?«TáJ
îzAûµO!ÚŸbgg/z‡wİ…´ßÇ²%e§Æí1ÎCÇÉ½ lÂn?äE\Ç¨ÇÔãØ¥'Ä°&Áìß¹³W£^«ªD˜µTRĞ¹ly3Ç.}ÛWBŒ	®ªÅ·ş/Ê¢D‹lp?«:-ÌñÒu“‘Ô»÷áàØU<H\x»»[
>[ÎÙ½àU
"ïOr¾9Ç@¾qhèñˆªy—ˆ{ \¹~¡Í¬/Ûî-*H[C4ş#pšnK ña–khFd‰?/Mqóÿë¤ÀÑWåø†ßwmAZFÃ™òcÖã9ƒÖC+ì¹äœÒm†¦³ÔYÛ'¤.¤©wT
œcÌõl§Ÿ¡»êÚ4J©œ=µó	ïËÈq½‚gÌQÌ(CÎ #ç›¾œˆZØà¬·!+PØ*]Bê­~¹ØRvŠ0b›Ã¸ÀŸGFl;ŞÆû
¯}î:ZtY^6€´g ÌC^®]Âzq’è nX(®
Áœæ¦}ù\™¬ÙH‹sµ¾±¦× Ïu`Q»ƒQÊû/Dy%×²`È oDÿÔª•”	vÔØ¯|äFÆíÉz-ya'ukİbœM£L
cm“ñ“ôNRÅ=túKÖ^œ•ïŸ–hšœŒ¸¿ØhQi¸=kOùb^êOY¤«+C´IB®Y­tI@xœ°Jêú¾5,4«à¼¿:|íq¡3PÚğ²B ¯#êCàÑì„Eyû]ş#é\Æ“Òq†S¬²¿ÓR`§Y”ğ×ÁÉÉ†¥ÆQbÿ¦ãfÂ]4†NZ3Ôs×¶ƒ/÷Î¨:¤ºS¦Œî#vnÒ¨ü-‹îÔ#™ÍšÂÈÖWÃa_9ñî½´à°¡j· Û-v.’€+vPÎG§Z7@Õ“çD°W†Ÿ„|ğôAÈ}"ÜÍV“ô}dt0¯x(,¤¦èaÌzuÒ®;u˜½Qï•˜ÌŠÔ¢ofâ©hÜêE~¸İa¦L¥ùIÚšØ9ëÒÌéê4Tÿ;uíü¬ÓÂSÄHÈ%¿ï©Œç™3†¼lß¬i6÷Øp@ÌÜ”ñıÍÆ
Eë‡hL(ñ- äİ”ÙQ½¦¾©ÿ(dhÇKëÇÛüTœíÎi·ò“Êmåo ¢7‘
9w»É“Ì©#<>*R€
'«xô'Ş^—ô‡½ğˆPÆÇ»áb.M/u¾˜)Q=Ë—oB1w@¶¾ú{­¬Ğ³gadO¾d”"Xaiy›¬¤O~:õİ²Ëí}Isêı’“N%¨İÜ^Ët¡"¹»\ÇnéY'—’äu)—¾˜ê8e†A4m}ß5Çi®X¹6@Ş3|.ÅÏşËkñ‹„1İX<õ¨ÁÉÜ©6"R6ŒQõÖ'nCîŠ\ÁŒ»Ã»†G}ãÎ|ºdß=ÚÕ¡%_âwÙù“=”’ôâ× GrrD"d”Ãåtè¡8&súãhì¦„Ê–H-ìyæ="4<ºÿˆ¨®Å.TBoÖÕm¡‰-¶ÆñïıHbÊ$ö9›Ãry$ş_¼âsÜ_i¦AFğ©Btl?vTá°­Ö³¥/~ÀJŠäUTåZLÂy]Êù5ë·¥fª§Ü°3cøéß©…×Ûñ_¶‡ÜtÆæğİŠzÊì¡¾N@QÈvw,*İ¯w§µj{M–ZÃüöÓiìç7 ĞÀtÌˆ°ãÂX%pÄxÛ ‘Àİv§äı}v¨‘GE/·›©:	0™‘ÛÎ8Ø 4Êg#§ -åRfÒ%44µ‚åŠ>VŞîİ%J`éJ–°Óg$Ÿ©‘§V¢C¨ÓôbÃ½€Æ¦Ú›ŠiäF)œì…í´`îr¾ÉA{İ[)¾ÎÆ÷%ŒÊÄYŒt9{l|mÒTî:cŞ›¿’7åŸúñ)îª.ïĞ xÌnDÂ@OP,İ­¹ôoÑ9$Ah·’FÎT`è\¦ôÖÑê—Ö”zHcÉµ…«~íå me­rè"ÀéKÆ	YáÚÊ½Ö%c[©ÅŞ¾ŞÆ¢™äX;èm¶ÜÊª+>‡0
I9.~È ´U÷ó!íÄ ¤"‚s\’nRÅ=/ÃúmíÊkœ¡Ïd’qNå‚Ö‘ßÀ‹¡ZôxËhÊ±ªvì»ø³+j…q4VÖEf
œkj¯F2ÒR0¬ø~½¦¬0·hÌf÷Üîvüúd…F‹—µ /ğ“Z!ï_RÀ—%(kµí–CÊ?ãí:\±?gâQû›9{äÄë'|Á–}òÙò˜¬j¯RR˜úĞ‰µ¾a®*¤+ÇTïôªŠ±áñ´“;°fƒ¾¾së Í¼æˆaì¦&50‘#ü—JÁ®~p¥sP›µk™MµÔ¦Êw;_àÌõÃ	n#Ë9©­¹§ò8İÁsèâbV7ÕÔ2Ìéµfõå¦(­¾ÓH²EMàEÒ¿C¹ü ŞÒ…Ôóoê‡=”GYFf·”ƒ]]›^0ıDc‘½bœø­ç¥!¨c×D/eB[NAÇæİ3óš[™:¸7îòäÀËf±=‚÷Ûpe‰DÎ‚É	×M9I“|gô£Zjëô¬è|‚·ÆÉ‰ô¯‚ãF+°mVëÜ¡s¥ü,­ïóF]L½ (HĞû[H¥Èn7Ì[UÉbgü§é¾èÉ.ÛòÀøãáî:Ş~Pg®×ò{ç	5wş´„yÊKO‡Rˆ¢cÿB´W:¨ÎEë¼éö‘D$w¾Î™¹*6-:‚Âi¹PU‹i2
ÅêY.Hö
‘÷zƒSlÉÔüy)Q­JäËü÷>ÒÒFäFc½–h²lË å2"[4®ï`¹˜Câ<ó5ÂOyØ4{A“èê$uòm¥\, ş‘X5VfêŞ ]EÄA,û) †Ù†É¼àIÙ!¶‰ó´ õÛñ""4©-‡DŠR°›! ¿siDëA·"±Pl¹Sr^¨5%®0Ehcd ·éĞî3ãì6ÌLî]Ê	–ä¬Å§ìû&«çÿ¿\¼ê—ÃÆf¦@°ÕÔƒ¯=Hqùû+òçOAZFY–Mƒa/Š±Ş¹¼/âA[¿ieíj¿ß±B_ãNÄ ª )¬©d &'ÛÀÿÛGbébkk”jÿ}¢œòÅøĞf!_}v_º%ş7™‘“™¶aä?Ê„ĞN9›¢0~³Uµlåx%ÕëĞàò×£È–CÒîÿŞ*¸Ûl'ÁÏSl6:_L:îÊ3²,ò#ÆAÚÎ7§÷x%-”?•Ş:…¹WÜƒš]5‹W“—‰iÛ£ú«öLg£²ù¯&‘:<Y-l@*’;{w’††!Òö*§«eoƒzÇÑâ´§ûhà÷‘Qé›`Xšf”Ü‘ßš<[ƒÜÜÔR¤íî»ÚƒB?­÷¥kDg„¥y!¢(‡æG‹]}Ş>#?a¢©’5¹ôjŒƒüPÒ 5±F(´£÷_	Ó3‡nØ¸q›ê¡æ\V ±xÎó»÷60êÕé\Â–¼ù¹ğù i£¹<2ëu_gàKWï!Ú nšıÈi_,\,µöGi}]"1qø|$f—6äC.“›GçêTEMoö¿åÌe³Õİ#.|¹IàRG
ìÉ‘³s!–¶ôŸĞm—ß¿Rx m²«¶¤å?4=B5íZÆ™áiÇÇ)Ò5CÖ_œÊÀ€;#v†¼ag”?fwƒÅ„¯` dñš~63öLvNê·í¦öz¿ëM òQuœÒƒ5ˆìã8ú±Wíq†L¼olBÆ/µßL™ÚÉáë|˜µ´Çñ˜ËFâ1²Ó6˜×¾]#"üjLÅ tµáQÉTiÎ+§>qôÁ5ÔÇ;úO¢Cšu”Ì>o†gô…é?ÿÚ2/n†9]8WX	I×ıÖcšÊĞ&@EDÿ	x²úéll2Øc–ÒšiçxÂqš¬£t#Bo×³ğ9ãŞù’Ñ¦7¸ÑPè$Ùƒg.Y)>¾¥¥Şé­'Õ}pSº÷¸èéUÕ¼`.hÙ.w#­°è†ã\æÒÂ…¨.¯µ3’ì²	Ò®3Ë”0)»»ŞÇìpäbğQ`Õ§Eœµ–õˆŸ<…U4Åle½¥ÂPä87]’ÎëX&¼k‡?ı6I®B\‹­ob%ÖnÉ…F5xiä}$Bù€Â¹@şfl fñõâ®>9îqAæ«4+­`%‹şğâ“ÂJèu ­aj´(]Ç;Ç6¶şJàÉiäuûtGŸ¤Ô]5ÿ8›+ ¯£ØyŒîa`¿Ñâfï(kN7Örs+0P?lÿOÆ«¥óË Ù”Åª±“æ0.ŠÂ¡‹|‡dÓP9¯÷„±àw¨¤—$ø—9ßèÌ³ÿ¥¡Œ~%XM	·•'á·Ê§ôÌÌ•Kmv]¡"~¤	f×ı‹ƒòÒö;–‡´×5RÒv‰ÏoÔó ÷‡†Ñ2Ş°ãhû°LÅ:”ïáix¨ÖŞ}¤ì¬ö_	Ô½[ Rú¥IE£ĞTog`ÜËóä\9ßÜµw>¿KÕ“Õœ=F*#èL–ÕÀçx#1»ËE#¦»«y‰>‡'ogĞ)ÎÄxĞë·Ü¤|¥µQCÃ¬zp¹Ö9ôÒ¾Ä<qNÑ)X Ö˜oÃnÄDùDÉÊŸxÎ­rf­–J ¤Ÿ“,|eÑs|¦KÈˆÓ`ŞÕÏ“³²©v!t‡a«êŞJA¨l|!x©Ï]ljV¢Šà¸Ày~¼¥:v™®…ö/ógõ—ëo‡¼z›İ¶Öu€…5k¾œÿHµz3”¾µŸuğ|ØÂm+[ÚoßTX!	¥«5Œ¹fÖ|îŸè¾ÇO?—‘-JöÇg‹\(Å¦	=Æìme¢Ë÷½ˆìÚÌ¯iÀŞ¤b£RNÒ;ÖuÍ³&¨²”}è³—ßìãdÙs¾†µiÿI5"Q+“‚<’
áaˆ÷1ÇòÑĞ-46Ğ’)w?‡?ÍB!¬ıŠÀÕ3–baOê·vEÚeCæ¦È•Ô=]ŞLy:Ôz©_–d®h¿çÓ+‚7?İ~­EF2¼fÒ•˜SMgP„qÍáÛ<T%X/‹b\ªö÷ñ}NÁ[¶Y€°#gšé­Z×]Ã§ùvÍNXZtZ;™Hï¬çCYnk²Í 0ôñ†Çlo„$æÉ&/-¯ê8À,§<_˜| ³šŠ¸fÀœ„(µŞt$iÎcWfâ:ØEöĞº»ØqtL+«`‘9:«ÔHeÄóğF¹îkWX­Ë¦rÖeh¸8ßÌU/`WÇ7ã=Íi*Ò˜bUy¼AÉÍ k;°}
Ù 
Ù$ô±>ôïOí&@ÀEÅèÄâI°…|nZÁÍlUxÔ ÍÕê£g	œÀiÙ4Få«ıÄ5¯ÿ<"º“ÇTÄşA¬1ÌÔ`ÑªÄXöEØ=H!İ§SZ´¸İvÊÄââşn‚vnØH»:R‰•ox/0ècèDÅÜòÛëyœiDÂ ã±sC ™´TsşÁœ[‹,49ˆÁ¨Ğ–iÑñÖG>á÷_ØÕØSnğ9©a·üRè‰TVhN˜‡3cùW!"qİåñwšayëÑÄ*M“($òšß½Ïx+ÔBKäµÇCåÈqÿÿğ³\ëhØ¾ìì»ÚwRt—O•Ü7€/Ò#Mm+›ñ;÷µÙ6²yèö¸0Ôvoè¶NS'«T.wÇŒ)âo•\ğğ´‡ùx€øKkÏAHXMüI$Öì(ÕN‡7`Àá*$	Ä‹|ÔhÇrüjáWj6cpˆlÜ›¿†ì´£VF"b<aş("B„URÅ%GQ;Y¬ƒ)µ[¤¢OäÖ ?Ãv+İÍÇ'ŒPİ÷2¨½<îZ‰N"óm„~À¨¹¦¦ª@,/í@¡‰ƒ!eõÙDóH?šGØF‡ÀU9AóMÙ3Óõó“FiÛ±Ÿ@$¿Û¾°BÎŸCL»qC$À”DÁ0kà@EHmÏmVã’Òè{ ÑgYâ†÷_‚T´?b$}{âíë|öğÙÊ”gB2THxõ<®¯‘#jtÁñ{k‹æ7¤©ÛËÑYŞıÙ)Œ´úSßİjQ	5Tî¿ëÖ±Øÿ’«·Ôdèñ¶ªùqd‰¢}É·„9-tH_‘q4 O:5ZÒhzUÆæŸ›íq¬àOìOŞ+§{¢š-úD–ÄMIå( ó§Á†}LLñ‹?ÆÏGŞ„íâ|¯îZ
ò¶Ë ›Ñve©Aµ™GH¤İåÈ£,yÌÚ€f«<%àè@ıÕ‡òÃï&Kš8q­FN2AÑîs›8¼€UµV¼dûàâV“µÓÉƒÂ} «Ëı†I¶]ÊË…»´ŠQjù÷	ØN=5:sÊöà¥£â‰L ›,}Æ÷›ĞÒrÔóÔÉùeà.Ë'Q™í+ÚÙ4tŸ2_æÄh­²O—Eå–Eÿy‹ä!%|@CG•èEãDœ‰ÀYÄbu¾ûë¤†öïBU%¦	Úl.ÿY¿wewG-cÑ9¾B­Vëf´²Ä‚	ÒñLª…ßû¾É‘•ò€•*†+r~×úxºÚàŒî½ÉaÇQ³î´9ZÂÃGj÷¥Ñ(XÆ‰':h3]´è2§Ç=—NèÄ:!eø%Ôşº·?ùY3"ŒÒAße\Ğæ'Î{2Ë#WÛwÎş+eLg^ÜÑ¦çJÖ8YÑ’ÓQ|X Á¾G§—Jd®î\IÈ/ÜB	Ò8²c‰ÊL]ìšä¹ëŒ¥(Y»×Èm¡r½[¾—ÌõwoM'$.7ìèÌN5ÿÄæñyÂ÷ÕKíLWôaV\®P¹ñgŠÃT"‘§8}}çiqà™ùWyz'ó£C‘5Ìï‡çë.ä“€é«Inÿhêú`í³u«» …ÁÄ²p© Ãş¼\²j°??Ñê1‘¬;c¼ñ‘ø´:¬f>T@i¢îüW{JXı‹ã³‚üyÆİÔ¢°Ó}ƒ%®¨·0R.b´Ô"<íÄò=äNŒ	Ø¼puòôLÅ‹ÔiQd
Éù?¼U×ŞHÚ3Ä)ªlÂKÍŒ£aÇ;Ù4fÜ@:J2-ÃSQÔ1è²xLëT’ŞcSLà ,0Ù)Z¨a›åÄ“Œı¾_ğçR†zc}ÈfjÁÄİ®Âù2oúˆ¶¤ouôºcôÑÃk;6)ÆØ ZJB¬+òäÇ½ÀEBÔ·T–f„–İáÑ1ê|ÚÓÍı¼B¥¯ö²£m€Å´ƒøoÃä‹EÎëª¯¡€Z9÷osüÜ1áê}Û%i8÷G¨¦àN‘™ä^kò@>Xæ®h,ãmËuóê*|¸Ñuøz÷¼by—”Ä•)¶¢dé‰ç;Ò¢uµK™-İÌúRñ1„œòp	¾ø¨åÚc…(US1rñõŒÕ«g€¥§†«±à¡Cn…©Ö”hú·¬¼¶‚­XB
Râ½ÉôCE^Fè£j÷Æ¨áëÛF7,T”Û:¡” R8€jEv·ÎÍÓí`Y6G4:<İy¿Œ)¯ãrš€}³d]üŸÊ-í(	²Uö€½öq×
ªZÜ•¬ÄÈß6J à:ù_€•å¨ğvºÊï)ü¨£Ä*ö.SÃ[$`Ğ'­ `Ij£û:›·bÄÕÜ0’¹37ŒWğ`¯8)¼ãü>ş»®…Å‚/­ˆiqhtÍNi¸è``|ÑáˆäZíÃ#	Ş³‡;Î_Ö‘TÇlÉ®…Àˆå¶YZqáfmÄp*øeÙ{‘S[?r5Z˜ù}ò½I!Öz+üf†Í¦qÌt±+fFvzfëP{÷$Cjà#€ÔXùR£“M1@k~~¥ÍË»&•c›ßZKË²Ô®»Ò_8èVtrEÅÂÊ÷ÙäC‚ğDít”‚ÌYâ·›Jæï€*}ì˜)Å·`I©S›O‡±¿ğ­rš2dóÛáŸeãux‚öæß#Ölqgø¡.BĞ‘´¾'qõ„2ı?©?¼ö‹ÍAå"·Œ@“ù¥QÑ33‘L<ÇHb²=›¾B &fRÖL©ÇØÕç¨_o/ÒËê¾ä(]ÊŠBşÚxö¥»ˆµªú6Ÿa«!¤J»Š@’›»åcõ+\šôòD|Ø¦çP@‘èË®ô*ÔÛ)­=‘Õ¬ò\„Åïˆ^ ò«Ó\J<ËX¥ß‚×c+=gw³µS°x1HøGA-:}Ÿîfßı…-€¾' :î,ÈÔÎ#tg[Û‚(¢3•O °C\‰ò	Óø(¥Ï>0äM•BX;¾³ª€c­s#—rŞ+p§ó8}õ-Ÿ‡4×m ¼¡¶é4êCÁlƒM‘\ğ½¶ôò;{
ŸÄ‚Sc2\uÑ…Osw[ Sq›¬Kq}_ ŒY‰Tíi²TQZÈ	Ä»—CÅıIĞ¬©£#ùÇî¨ì)áÇë‹V$Å¢÷Ú¬FµJ’;Åíc%6xaèuñå%¾<tQQÙ¬pco2jÎ.„©;4rÄÃ-ekîÿ|ƒáúH’‘F^^è¯°Ü×K6¿Áü¼‡'²ÀÂì„ÓŠ±ıøìX
ÑÕ Ì;¥Ûc•táp·
Eâ¬ó ùJùI‰Vˆ BªÌ[$™šÌ hZ¨M†©Z¸pë–c¹Ù:ù'[$oA+"àŸ·	0ßŸMx›2Xûèp'á©íJ%Ûóeû§EßE€4\«?»ş¯cĞû…–è–”ô×¨Zgr,Ëm_Yªé+" H¾ê\`ò\åöw“p`°n7X&Y×w-Y€²b3&Ç…>â-z;§ÔÁ`Q"	ÏÄ¤^ûe’fÔÑã‰²U ¨RìÀ^vşÓŒP¨xáiÓ‹‰b'ÏÑ"iÉ±ƒõÛ„9ïGß!‚yÌe¶Ò$ê­‚¢x‚Ò4r=Åù^ˆÇ©ÃR,mÇ/&cÉ>SöïAuÁ^‰AòX ŒXeÅ/hNšÜ‡Ë©¾Îp˜ğ…ˆ}¥¦qİ‹‰!ïj#œÅ¡j¾àFß7’ğ:çTö¤Ãò¹Îüê3®x’<6e›º•™hú_6šì5pãğz›,œùÕ7{‹~tÛ@xJ³dvH½wŞº/h|y"ÈZ oŠ?ÿß~<#İëû8áìñ62â6ÎŒÓÇ¶ıìæ‹)jÆÁÀ­®¾ã:ß…¦õÚ;EîĞ©6Â—•„‡ãÌ	ã}k;èÂ.6qiÆg!*KC³İ&ÇaÓV´×š€\ÇVJ•‚çïj¬eH„^zY	³'¢†µã<ØóòBÔ´%cj-·÷Q¨mÈâëáì­®´™y™Mö¬¡ç\Şš›Ä­ıVÀÍInß-—‘¥\EF¢sX
CR©3ÄbzÆQôÚ±zıbd™L‡ñí¯>}Ø±åÇU_ü°ÑùeÏ}£2ïïÆÒ§”Õkñ2{|^#ZİÃ‡Å~üt/F^e–XÔ`ìlˆ]ÅD1öé/AÎ›
ÄÇs˜Ge)z*Ü\•M8;Ÿ‹qp}°„L×jŞ—G3¬£$š¼WAáKÕâJØµ˜¸ÒhÖ[úªt¨çáEª4q0ÿ#š$ä¾óé²êó“L8¤-É¼;’½•{|pgBarOõ%M3C,Ø@éĞ“	ò@ĞŸ_^çŞG*ÑFeÌìÇKÌq»÷*€œû½é?»”áĞ¿ÓÜ6ÆÙİtYx˜Zô²3{VäÉÉÈÍ¾]©jÚ£—Ë™orL!Ø:düı¨ @”g ‚H©ƒÈ­÷5»«`d(yŠÕùaff$ïmŞBêÒIÿD)æÚßîn—°ï•2Ã² °<¦îÿ@¤|µĞ'Ü»†³æ×xøğ­‘Öáåûl^ş±:<kuE.c©Eœ7w›>‡'Í,!Õ_&­qÜÂ>oûö’äféqvj@Y¬Èö«	wŒ´•"õÄÚÇÛ[ö"ë§#&·?êíı‚ü™ÿ/«£ºŸ«’Ä%xJÓÿÊyRU®ß¥¬Å9¬Ôl¨H]¿]_ÓæE¢şx^htÉwd7'¶Ğ†uˆìšó’aŒmÈÅı’¡©Ä´ÔU2’K‘^Í5³¥’=OøıÛC‹ÉUól#ÖO…Æ:;¸<f¦“1}6£·S[˜=ª*´>TYlì£Ì¯\oié¿°T5a0©Éºa?àË4Ë?(¢¯Vv¬ëÇÎ6ş³Á8®‡p`¯,”¸©íjt4;pZÆEïóˆunşĞY”t+ä“§`ùŞ]„d:§Ş)•Ç×ª\ĞõŞSÈWÛ¨úy®1ÜbZÕŸiôˆË¦ŸI¶Xáloƒ¬İ5;¨¿¬ “¹†ƒTz¹Œ÷è«%ÂÂ§ŞØ&¯&²  a0õôá2C6ßÉ´m­ovÙs—UVôó† <@Á%Şã®º•KeLWşeª
·â‡”BÛÆ7ÅÅä”\Ş+Vj.,h'Ä7G4šç¨3Š…á.½´ÂİŞŠkûÔôPxg¨Ø4.•ì€§ƒ­IäM<ÆsiÿÀ¼À3ü®)P8eÇä`{—ÁØ…Õ™ËC×Ã&%1ï2ÖB#Áİ³P½*­Û•iÈÂÖ&Eí!_ZÛ©¥®è	¡Ağ.²³¤yÊIl¾ó¯Íí[«ó:Eİ+Ršû¿ßÃdNfD¾û’.)Ç»ˆK;Ü{Möœ
n_ˆİ„¼±^ôUÉ°ká_P{ı¶A…ñÌ[_PlO“YnœiÎZ6»Xè®Õ1`vpÎ·Æ~BÙgq4Î¢÷µàcjk¿ªN©Ôğî+N*C×ƒ¶õœˆôÖ6‚ä+"m­ûä>#Áî°3wÆLpéáOn”Óé°Õ:®ë h'*Â ÿŞI{ù‡Ò¶CJgº0+]şlg½ Uè®Ëó>8³tºWœïlöJ#¶Â'È²‹@°'KG.2†´åtİßøEWË:ó²İ‚m•\[KW©MQıŞ’RÓ´É=M–ĞÚÓ¡Ü±2é›Bİ¼òåÚ_÷À«„TÙJëEn½ìçTê’CB#r÷4±›ª˜u	5 ë^ÌázË› ¥÷pŠÂ“]ÙÃÚ£~<y´5ò².]Dèş-eJñ·;aO«ô˜j‰MØVw=edÿ™
 Ëk~EÁè¬\LÎ×gbNGR,Ç@SİŸtâ~ÓBÏõ+×øıšßú®k‚;H4Õ"»iq0ê
­Fı¹#Ó’¶Ã¨ÄœH£mÛˆ5ßH»»úäØZ}ï¸0·Ş1Ÿò;¢ô¦$ıeœî½øJÚ¤ÃÖ[ó“_òßƒt½—‹ÏÜ™Œ›TÀÖ¦ÔQa[LQˆÄ}_5ğ^*àÆ6Á‹j
Õs¸F¯ß‡`Èk¹£~Woı³—ÁÍY²bËÂÎFh½Jk—Ëúıqcı}bbÄÃ´ØÑ6!“µùõúÕè"90$€¡à2İ}²¶Æ²S-ÂÜß[„êÌµJ—cªf¶˜2Ştô	ø¹A3‹^¶>îàI„Ş÷˜?d*âX…Û(Ğ¥óİ%Q hg¦h=„qÚ1å;=$µÛ°5ÈW‹4—Ñ‰û¨í5Z–RÆ$½şg.{!—ÿ+)Ş…H!Bï+Q=¶ çšƒ62IÜò5	¾&Å	)½=€åÚ²ÚŸi«vÚÒËÅqvÊôôÍÓel›ÀIg!¼ñå¿!e­×6ØM¢6ô{m¾,}ù$¯4ÖŒ:ƒ/•8M,Û^#fPqL-,çKŞ ç|çK±Š¬\/+£^²RÅĞJE8XœÿÁL3Íšœ,ò“6¥B½H#¥˜}şÉt²ÌRIsG—e“áŠY?ÄûF ?ôF¦Z·S?;ÅA’ß+ÕÕ¯Bg,Nß€¨4”t¾ÍK*ëú¸ñû†‰röüO£İsğ åè	ã=a¾rÑÌ=`tŞ¢ÿ~)ÄLD§÷)RÉ¬ÁÜÚ‘ïx÷ü£Òõ´ü˜€¢²7JCfH,¡<º™G¶Î{Ìq£§D¸!ÈwFgÜïEËP*àœ¿}­õFCáK l$n5ì)˜,ÖÑºƒE¦uÇ—¿ÆÔ“RB ;fdhÂölÅz¦ÁR$î-…MhNn1ƒÆİ½­Š¸IñÊÂ:)±ñ³0¹c›˜CÅa¸±—!ñ[D¼ıı1¯Nı¨eÓ 'ƒÁŸ›°i¼T‰X'Yl;=‰‚í¬¹vcì®
Éª*OR©áõí›ôîv³Wpk-Aª!íã²MgßµÂwšïxÀ@Å—eÌÕÙ‘šÿÅR#i‘Ì`h5~¬µ<ŒSÕ›ìûCò¢¶L—™ÇpêÄjpß²Ò®¾ª!éÉø:+&KŒË)ÊóL“Ux¦ËÆ,îõê	Ä <‚³£†'ö€ÅJ[¼·E@Tgp”»8T.ş;M(Ø©|Â5†xíJ"|Éh<mi¹GE*ËEXcáDÿ¯{ĞÛØ½3Ë±÷_o?„ÕÍ£,ãØ§a‰²3%úØÀ^ÕKü±&Ük¿5f	(Ï#¸æˆâİ€ETÓÒXBÔt$9ïıj~#,ÿÒ¿ü¯·‹jZË<0 zİšË~a•zCíâ¾iâVŞlŞ½i{S=6'èåâUAr¢ùä&/}¦ÉqxÌ}ÿĞÀÂa†"è9ßàp»;·Æ€ğ¡edS´#F¯M+»ƒÃ7+Cä} ’€L1×ûÿ¸Yd2^¬©€S¯˜ÿL•¦eTikñT56ä+ìæÃE‚ôß†P¹‰ü™Å<ğÙ±Ä¯ÀæÂeştc„°KN^î\”“ˆÌ™›ÛAáÁ±ĞT‹ƒ7ÔWÒºî©C;Ÿ#`KÁwŸFÚÙÚåÂJ¡óT¾I6úçv«>‘@ ¯Er/0Ş+°AT¥æ¶‰,Î'\ïJg)––é/›€ßÁ®è±ÏQ] ƒxSO¹Gw©BÎ¬¨EoïnŒ4¦]uÓ¢/:>D‰Ÿ§C¸wl4	†ın¾ê=­šÌ'|l?½;¦c¶pé´«‘¦ÚUô_Á$½U$Aâ¿³¸’B'|Ø¥L>Éä´Zp½êzLgÔ³İáj”†(èúEnôúh«‡L©Ùû>E¯ö¸[®CAgœ›æ°•W)Ãå&“x/ıÎ}ì¡^oVl¥}‰ÃQ’lß‹~²©€U„Ñæó:|õàÖĞ_öaZƒ·×ö8r?ºµ²ĞrhâY;½¦%uÑç9ñqv³`öxcF=ÂÆµ¨®%Z´pvÄcÌÙB¬»ûgª8¡¸äHä$š×­ì%5=*­ş6?ñà‡´ílDjS†u±œËt
ª–ÕJ#tõ]„»¶¸÷Ş'QÍçœ(L.¼/€î1qE†d­²7áYÍÚÉª¿§RÜûaİ9‚©¬ş¦Ô{û–<j¯û<òëMšgÖ)^¨A,^Ónú;£îä;Û‚Ç¬´™	Vaíjñô5Çf¨jñ §;6¤eĞä±÷’“Ô 8è½ÔÌZø©F05+#Ê‹ßÜë, ûl¼öã%?Kæ‡°G<(äMZ+nZkÓzï…ü¿YÕ›œ}t0ó†×ôØæ×ÕF”Ü0ZƒŠ¤Œ×,\ı×!TàµDÛ©ßçw¿oD™ªEp8f¹HUšš6FR9Ğ(]·Ã“\øØF·ypş¯Õª7‚–Zínk‡ş‚I5±œÇ^î@QlïxX¼şÌxâšëv,éím>½ÛØ~‚ †ëØÇ5§VÚ-X ¶Â¤³\FxL=¡ç÷‰õçtCÚş$6ó&'0á¬¬ıMõ»|Øã¼Íîšå¦øfí5OÒ·2¡LM>øaĞúÀN¿İ•u‘qÔñx{ïÌ0ÏD	ÒÈşçW#ÛtW—²°¯ qŞ×Š‡,cEÖ‹nè@5°´Ÿ6±‹p$s?{åO5-\*’å—`vÓlVËèóD1‘œøKü(ÉÍ)é‘¢]##Øş‡PéÕUAdY¢‹!)Cvs–út“/¢Éà2ªpe¦³mA˜’ai¡4‚~"ã\-œò‘ïœ‚úAïWÉÛ1‘jí®6u‘ÃÕéõ›¯ÆñÌåŞ×®ÂêcÅĞ« µ³ëûeï]œÜsû`©°=¬M¥™IÅ¾	¿µ9qMèĞ«™¾.^ ÌD¹ö3V¿WYP¾¼“Œî
ÏÈxÇï·(`÷ÕŒş¹¼
¶Ç™†L×ÄFñ`(£SÛ‚û‘¿¯ËÕÓh†È¡*2 lšµ?8`œ|ÖTŠ\S–‚^NšSZ†·4š+ilH8ú¾£eƒÜà¡òy}÷*Œ­‘ª·Xn ÅtµêüŠßXaÏ™{1„\`Ê¨÷ßIsfU€’§p0š—D—–o´“s[£‚-'ÍûšÍ›ÒÄ=Ê—…â.†l’#XAˆoü=a¸HøD ík€¾Ş'+l C®IÄ¾«¡ô=êéØI“„:Šrİ\6`U`iàÒô~5Ù‡½¾/}BBn³:âÊògç†Ca;	mÜm›^–!
%b¨¡B@TT¶‹Ì 
àÌxËd*Õ7S(c ˆùØïB4qeøwk7Û±™ÊXM[7APØJÅµNY4|:øt†
MÏº| ¦E¢¬ÔÊ£´êYË´ŞÕ£µfÎõÌ¥8yÜ¿¸Håô,;ÚöS;"-¬²¨Ò¶A_ğ\Ñ@§…v}b†Uqûö¸Y‡µ¹lXIˆ Şù„µtŸÎïÜò_@©6§J/íûk¼\—uHl…ÛÇ{QÖ˜ûÃÆ–oÆ€w'ìÎªÚ/çC‰ğTdè{ÖT´|tÀ¼,Í­p6Ş®‘\`+®ïcåÓÅvàØGœu•ØqšÜ¤²ëñŸ[‰ÈäÙ(JJ©ÊĞ;Ãv©„Sm÷Mçrc€‡òzÿÛkÏk#î?í¥3œçPÌJó¢¦öV“”\yêÜßßRY?2½çK ¶2üŞ×…p”:”\fg4ÀUq8Ên>„N“)”»;ü	bì¼YCïÇáñÄØiÙ­$kBŒ²iËŠœH"×¡ 
ähmêå£L@›©q~ö‚&b»÷Q½œ™ÚuWÛğ|¯Îq‘rl4«8¤s·¦ WôRğTâOU¯ænTtQ¼rÙ÷¿LJl–nŠ<™]ˆÃj³ñãF¢ÙVùNtç—·KÚx'üœ"ñïk^„ğè(ÃaÃ™™Ê×“šŒ“2;?aû>y@¦3†'%¸fÊ	¤¸)òĞ%
ÎBš6Ä0X;$ÚY˜7¯,‘bhV{ÒH–„ñ
=˜a|R;LÌowÒ„9¬é¡êîŠíÛq`´šºß|ŞíŒÜ"÷5cRÒBkÔ‘sØ‘¤á»øO.õ“ ñznXTY€ ~?³ÚëøjB[’ UÂ‡éj¦â.ÃLD}—ŸoøñXR0‰ñhÀkeyµÆ9|yå"*.ÕwèA§BÃæXï 9ğÙR«ü‹ş¨`#'R5îÓ·Ê!i…„m:¤(şİöv›9©}’ÆílÎãšBÆ1²NRğ›ÿ÷c¤…/<è€ïKÙDWBëmvZ°ÄŞ¥p×¾Äà…9cë2ğÒ!z·®–¬ë»¸¶
ç¶`¤F)jÂè)ö·óbw—lDŸgS“ˆ:ûdŒCjşo™*%<Í—‰KêÈÿ=5A kšE	7N(ÜˆuÇûîjÁ{Òº˜¢ÖM
”¸Çã
öi÷^êä”_â4`.HËª_aÂE¡sƒ›‹<«nökjë´_Æ¡š|A"Ÿ˜ÎYG,üØt®wÁïBŒ¸¡åÖ‰@“íÎŸµ^„ÉJª‹T 5çê0FòaÀ©’¾	c JW$Åëb¶ÑiEÂÄËeÙÃyÚèˆø;\öÙ[êœCÏn&¶dE•ëZßi¯Æ'ôUQwĞ—¸e~cÙ”á!õièe%¿8 Ë¾øÄ
¹ĞDŸîn.+‚R›gIågõnoàÉÖ4¾éw7u5Ù2Š!Çk!Ğ_½ïï®	Üë;C¬G‹B6ª[TfAÏ¼Ãp‚Yo¡ôÿ=¸È2ç¸Uyä{OóvsPIöü{¼O¨ãÀEiy¤Oã½'E£ÁJ dŸÜ·îDuç‡,T?Ô¼„Fğ™ÚH“é¿Ï‘é¹™Ûõt´‚âÉoµ8lÍ“OìŞ1+ÿñÂ¤üşñ´*é,sø~â[Œˆ40 ˆ1¬èI…ÓóS…I¦èâZÆ©›áåÁ°¬ß @0k¡rËö}ò<y¬Ù±$z74/WÆ&	¹x¼¼ªÆóı¨ƒ!äŸÇíÀJ(,`-ñƒä¢×Mìkî<''øé©¥ë½ƒ!Ëá¹¿G–<¾ñŠÚbv·F\%»Ş2F
wî«2ÌÁÒ*êâ ’Ô‘p$‚*#İ.)Ô`Ç+íH2Ôîa¾mO-ˆşûÈõ“ìÇâ#~}Œá
7nÙÛEªDX]½i-¥[ ©şµDd9÷¾¡i‚ÑøÒÌìñˆlœ1P2£V pĞ­º’jï,â<LGÇl !şÎ–ë”pÕP€dcÒåØ®ÉuİÙs~Â¯~çBLİAàEIòcÕÁÁ¯ó—¯Ò³—¨gh/®š;5~IZ¥nĞ…+È‹cQ–‘–6}¿Vê*ô"
4†ç)î/…C0‚LÍ»—Êw÷­qçòuÈ˜§OP‹r)lÅO:âÉqK„F·dçˆ“L/+#$¿¿{ú>ò¯ÀÌmb8X«jE½®‘hAuN'cU§Ë¢¨*]€—4´nµˆı`yé<(ü‡X­1¨NĞV5Ô8™6Œû”‘`Ú)İÖ*h×îAÂõÑ6YEqÕH7‹ƒ_H&	gÅæLíŠ›–*˜É’xEMÉÜàË52£À Ww[´IºOÌ‰¶C-Vs«$g»6ôQ˜#,^N¼ÀUƒœÒ†íÈBšfà•ì7‰€#‹Ğ¨wû<lIu†ŸÀ¢?)ì*A¨^†@lŒrèóZjwy0CïgK‹©½ .³}gÏ2é³ø	¦OÍ¼TŞ™o¡JÀ%µ
ä‘°4s¤ÄˆÓÂ–’9>¾¯ß2‹U…¼~-êª¤6-,ÉcÂ®D13ÀFÉ"ŠKÊ<úºÛ2N>È<F¢q}ÍñLØc½ÄÔ=§ePã/L|ŞË^I#R5;Ÿ¶~ÅÊWc:¦ĞŞµè;²ÕÀ}Õà‰R¢ËœÜz9ø,ÚI…÷ş7P0¡¤jG´‘ !Ñ½ëX¡<ä]ÛËO Â¤Àè:Ùó5˜5aÇı8„BÔ ùuŠğ3$E·­­ ¤l¶×YRÁpól|b 8P´ûÇ…JšVßJ©íÄbãWi«x“ß°!ô“(ÖcIÁËmş	æ‚Y_µåU—z›’\ZL½ !lôÁË£…—D…)§ 1Pv&Ã:F)}3ïlr¾Q@ÖvO+ßÁW`B³+tq†Ü)xçR;
'g¼ChÌm×¶v*ÅÄr›ØgbUşjÇ½ÂŸ"–¾Œ?ÿ·´şASB vNè,óUö”“Jõ)FİòBÒ–·+J?U"!Ùá’{8$Z*Í’½ª^M\¡ö¸¼ÕÓ¥Câ)&4ı!ïÂz`&‘ª›¤Ê˜+¾Ş¢×|Ë/ŸGæŒ¶x®#•†‘ÔWw%ºo¢ï‡Êş²× _€ölÎ1Ú—±‘Ê?‹Hæ~£&Æb\ÍÈU#«“¡©˜Šíc_jP‰ºö0øÉ9VjjÉñ…Ÿá%÷=ÿ“X…–’‹õ‘c@Wa èfD¥º‹İºClìŒ¯Kp`˜™ò1¥÷ÄT
òQµÓ4Hä`#pI]ğc¾b².óIÅœäE„/ÌãXdRqU¢xÿ¿†L×-BóÚÿ€ÀŸ>óÚ³ÀŸÄ`Âk®DíÎÁ^7£ô€$Õs–[ò¶Ş°’²
»Nİ‰NcF÷0Ü;óO;¢¬‘–sÛqP Ù¥Æáfgá6ş|½Şö(ü§5¿LÕ¹áÿs‹W	„àoñ
èKD¬u¡l:–9ùÈ-ıªdì¼Ø"å©ÊjÁÄ›Ì.OJ`
Ü(İVûŸ»üRB•KÊNø¯Áb"Xİø³Z2£J^.
äºï7ïˆO 0eQÀŠ£OX?Ì{¢æ&8~[NâKCoø¼ÇÛıÅôäÚ+?:D™‡ç¸šÌè&b¨;ÌreA»ŠC‰œŞåô8bÍÇ–ówìY-<†{¹™Æ…ÔŒ-ûgİ
ğÚ>Le½Êˆ¡7¾²W¢í÷&Pt¯uÔ©-\–,ßø–?)ĞúR¿jpÿjomË½ÈÄNŠÃ‘Ë$!¾6ëº†ky–®òÜ‚–.‹LJnl\…ëÂiªS›ıŞn‘…0Ÿ¨Ë½•sˆO¤}è*!ã'8‡ĞÚm6OåP¼_MÃ•.@@ÑØzo1ñÉßrvŸ3beÄ¾£¿Ğ›Ü•à íÉƒG_÷ŞÀ¸ÈºÒKÙÜõÔe…GS¨©÷zßKcšÂ³‹³èéÆõ¿f&¡j¬@½T${*hÆ„10‡få÷®ê4\÷±LSè+¾ÙZ,E7ZAœ°_Ş”i¶KP–ş‹D¦¿új§ğÊÚèƒ~ºÕr–£Yò‡5¨
n¤=6§D£´òiüÕ$OÀ‘wñ˜ÿë®e‚gvQêvƒjì"~ÏßsÖ PÒú”\-)%¼!‹ÉŠ,şˆ;;‡Y Œö¿%	Kùº¬24æÅ#èIı_'îÍ8r‡5Ñ-/—³¶æ¯òïdõÁvúınÔ%é•¥dFedrbk£¸SaÇœx‰«'f~ƒÓê]8Z±c[øJ×3§ØÈx×t5Fû°íRÄb‘?‡s&fBAgSà0­ãhÔîÒ˜	é ë1^rÆ$ûÆø4Wùs¨+éäù^4K a(µ÷‡Õ_†‡»ÿ$ĞÇß¨¨ÿ¸GÑ`KIKş†Æè^Œâ·¹™Î<6pTQ˜hIwDLr|!ïáü(ãAI¨:¬aánĞ	,P»0vÉ³:ëÿ©¸¤eïƒû9çR+^‰0=Aw{Kk½ó3‘(õìıí)j¢òû¹i~sÕ™Ã­|„ÀzDh#ûõ ½øÅ´08s‹ˆûXëĞÅX¥d)çÈ»ºÛÀ©Oğ¨'ı<L2Ÿã´’;yã^¢Œ:ã¶ê@Ñ5P“nK{/)å¬lò×ÀHl­ãBµÆ7mq²e˜GVĞ¦él’mù-Ñ¨út´Â“ Ğ8ˆ½Í**±E2pDõÎ¨ï4_ÚVYóÎKôU7ŒM!•¿'Ëy1rÄ8Î5kÃ¹ÿ÷W_ÌªÏİH­«Ã{‰rD‹Õ<©£ß:¾Å®ËÎVG*È·2»Td§ÛWsàµÕZØïİ4?ü8úO“¼
¹½¾C˜ÉÀú_ùÉ—v¨=¿Ş(~O¼ë¼Ş„TŞêQ3füôË|TuI 4 İ =î—ãS-BGµ>÷b%Æ~« veZ+Á»ÃSŞ`¯±-L‘Ú•[Ğ‹åN¶™ı(†«h9ñ¹+¼f'IVQnÁÁ¨–ïz.:T¦¡å'gûÓyŠ°Dal5mø?6í÷·Àááku’¿O¢t<¨ÙÏ ›_5<pP×éD’Rˆõ³Ä÷°È?Êu%CÖ5sõ²2p¦¶ÕAÆG,a0R"'V„ eö=;r•ùR}rÀUAHßó™ğPCòÚşJ(£™ÉûR»ÑÀø	CéüàÀGòĞnwËõ”Ÿ,GmJÿ› H]é‚0p~kğA%•R÷D=\šú„<l†€ÂÔ~*ÿrw“œêA“™~»”»Y .ÊFÄR…5W¯)>ßh¸tƒ•²é7ß:¶£remq†‹]
M|è‹"$9`.ç)Jù€şRf*ª8ÀòhÙÓò!ùü
ìõMÊ¼êçİôSâŞ¤¼ í¦9J'|Ù‰›”Å™¶Iñ”¿h±êø™®d”Îg,é"‡ŠE¡ÚæÔYVJŸ¡v•ztûåCôîaÒ½¢ÉpÃ ç^›ÄVxÚÌÃô÷‘O6i²çïB)Jè‡êRÀ ›1ñ{“Ë51xãy[(ºyã¯ Ò§r·+áv¼=0ªèWë2ŠóæX¬ÀàÏf[B÷)*JXŒš»TQX}™ÛKê
ğÕ‚%k°\™Ïxsç.Ã~íúlU‹n„J“ZqÇL
â«\.{D¿TQ¦r¿u[ŸÆ&:[º'ƒ	Í8G‚“VH^ÊHõ¿S·±áÊB+¼¢Ö
Šè˜7ğˆµZ1iˆÔ€X9–Wz¡óQ5°Ím	‹õ¡Î·ø1Z„Ÿ·‹ìYÇªèÏÁù
r‘L{³¨¬ŒoİRjîMfæò*ºHÌ^x[ê*ƒ­ ºß[ãÏñN¦$Šc;¢¦½ü…Gs¿qvÊëßi+–-«^¬ö ˜‘ç—XC¨¨Y\"ë…ÖF¶ŞrÚl”×ÃØù¦$Q¤páÆ¬!ÒÕ$ÄåÈaÁ²EÃcTÉ7·R[ÿú VX&,#NŒøÌ>tt‘/ Z%6‹(-ƒ>i†•Ş½“Im7¥0Ù¿)¨¾d{Ù%¬ğ½P¢©jU÷m¿%Oñµ\8ª1•BX±ÚZ>ÁÀù½&"Êö»g·5ÅOTNÀ9L–ÎPIÀp*õ®Œ ı&À”¾èÏ½·ù_ğ5zE'‹Z1L tå14½â–«Èó“¡†‡CYµGGu*&$õ…TÒK}™ÈJvAÿî+àóª·›«g°SZ³ï/0,áÍ‚y—4³Ûq	ÊÄ{9SÂn&YĞ(A{ì•odˆ’¹"eIfĞÚOş‡…‰Õ)ÊÎƒ':±m¯Û‚Uÿë%÷73®8›q	Í\Â×$O++wW'b¨^‹‚q@uşĞŠå-8²Ra­“ïøür<ÌË\4›—qí/k'Sşc¦ğ”}w§Á× 7IM£*g	0ÅDæ"'`vIëbÄÒÉe~ún²z$ÄzôØ	“~”C­ƒ'uwğˆlò•Q¶ÈZ¯H HîärT©Ù×ÑßlzÚX¥ğëaŠ7{ñ9¾Â'ê–¯^ æ:UİtºŒcÁ"ò`•µYGLÓÔ
’¯çxf«ÙüV i•ŞÜšVt‰0¹aìQı¹
Ë}§”ènp¾‰‘ÌƒQää–8í °£~——FáO³;”`u8M9j™|ú|êÆÅé±¿‘²§â×Ö_:±ÉC©\A›Ìğ´x^¬…ÏdFÈay(ú©°Ì	Iÿ§U}Ô_³²íÉ­ˆÏØg.&KËnøJ‚åjwª¯ã(ƒ¼‹‡¸úÛüß&„ítÚû]R»^`X	éÈºvş¡S	£©?àßçT#Ò*ü'ã…Ò»e9}3vë¿Õ×iß±‚ÔËBÒª[¢Ó8i³?ü!÷Ê^gèÀ’SàMtr‘(|ÔŞl<øg‘azT»øÀ\‘Àošİä4w˜.–T1Û¬{–¼İk3ğ›Tmğ£Í»ã<!(¦¼ÁezçqZÆ«”<yN`}/6·>_V2X·DªÁàë°æùûæ”Ú*QÇ“*äğ&¼Úı ˜ówXÆÖƒ‘c
±²WÃ@OÙô-±*B9&½Öy7•7¸pŒ`¨?ËĞ’Ÿd®É€0‰3Yl¬’]÷?ÈQØÁè0º2ãmÉªö’„ñBCXfhê)(ìÄ –…IrxÂ].7V¶€Fª£>(œuÊ¨$E²GH”û (¬š]ÎÊïî‹Á™XŸF¬ud}>]Ã?Éi¬±´sÅø!ïì¢²m½uÅWß§ıYŒ®B;ŸI4}ì*Øap‚d‹ø!ø—×TH‡´`p/WqôªüğXúÓmÆhqi0SÒ±Ó¾‘+êÑM9ä ƒl¥âó*¼PBdÕ„¸T7†«€¯\È¶o8ÎRùC¶ÅàA¢}q=ğj ¹Î¯ª·R .è’;ÛJÈrÚš¾˜ÕÌµS$ò,œK£ÕÉdwÚ‹ À—<ğåfæˆû¨&  è‰)v„otä¯½,İî×G6%<ˆ•ŸÖÔê¬LÔW…ˆôÎä~aë+ªèà¼\ôWéµ`ŒÌD‡Pÿ³ ó611,nI¼Ì»´àÉ4hJ>´S,k£˜×‹ñ‘úÈQôYø¸Ä8ƒ~×Ï3éB.Ğ?·0™“M•Ÿ)x5wÑÛ˜ŸÌşéOwÊlÑ^T”¥W	“EüXÛ”-¹{²iÖäëF
·˜Š^xğ3ÙİåÛc–tU.`p¦¾u~ƒŸ›yø^­ m½- oñ^¢Œ	ûx‘¯Îçœqìæ‘Ã´MğÙ?“2	‰aÊ±Åó¥	©µ´o–O–PÕO™ißÙ§”GE³ª"¹•¥±@Ï•ìh&r½oÁN€Ş…K©;Ñ¥°gh´ëºX¸ÏÅõÈ÷æUÕW1œ->sáp†!y¯©MÉŒØTàp[RŞïh ÌlKÙNĞ