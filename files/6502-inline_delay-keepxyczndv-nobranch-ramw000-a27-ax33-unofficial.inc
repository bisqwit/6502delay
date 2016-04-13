; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A
; Will preserve X, Y, S, and P
;
; WARNING: May write garbage into $00-$FF.
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
; - 4 bytes or less when n <= 17
; - 5 bytes or less when n <= 19
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 32
; - 8 bytes or less when n <= 293
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 306
; -11 bytes or less when n <= 308
; -12 bytes or less when n <= 491
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.1, max=6
; - When 21 <= n <= 30: min=5, avg=5.9, max=7
; - When 31 <= n <= 50: min=7, avg=7.2, max=8
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=10
; - When 301 <= n <= 500: min=10, avg=12.0, max=13
; - When 501 <= n <= 750: min=11, avg=13.0, max=13
; - When 751 <= n <= 1000: min=11, avg=13.0, max=13
; - When 1001 <= n <= 2000: min=11, avg=13.0, max=13
; - When 2001 <= n <= 3000: min=11, avg=13.0, max=13
; - When 3001 <= n <= 5000: min=11, avg=13.0, max=13
; - When 5001 <= n <= 7500: min=11, avg=13.0, max=13
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
; 1 to 312
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	sta @zptemp
.elseif(n = 4 || n = 6 || (n > 15 && n < 19) || n = 31 || n = 32 || n = 34 || n = 292 || n = 293 || n = 299 || n = 300 || n = 307 || n = 308)
	; 2..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 3 bytes
	delay_n (n-3)
	sta @zptemp
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || n = 19 || (n > 20 && n < 24) || n = 33 || n = 35)
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
	; 4 bytes
	php
	inc $F6,X
	plp
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 20)
	; 6 bytes
	php
	pha
	inc $F6,X
	pla
	plp
.elseif(n = 24 || n = 25 || (n > 300 && n < 304))
	; 6..10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 28 || n = 304)
	; 6..10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 27 || n = 29 || n = 30 || n = 305 || n = 306)
	; 6..10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif((n > 35 && n < 292))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif((n > 293 && n < 299))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 309)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #244
	jsr delay_a_27_clocks
	plp
.elseif(n = 310)
	; 11 bytes
	php
	txa
	pha
	lax #1
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 311)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #246
	jsr delay_a_27_clocks
	plp
.elseif(n = 312)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #247
	jsr delay_a_27_clocks
	plp
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
.if((n >= 313) && (n < 332))
; 313 to 331
.if(n = 313)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #248
	jsr delay_a_27_clocks
	plp
.elseif(n = 314)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #249
	jsr delay_a_27_clocks
	plp
.elseif(n = 315)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #250
	jsr delay_a_27_clocks
	plp
.elseif(n = 316)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #251
	jsr delay_a_27_clocks
	plp
.elseif(n = 317)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #252
	jsr delay_a_27_clocks
	plp
.elseif(n = 318)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #253
	jsr delay_a_27_clocks
	plp
.elseif(n = 319)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #254
	jsr delay_a_27_clocks
	plp
.elseif(n = 320)
	; 12 bytes
	php
	lda #0
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 321)
	; 12 bytes
	php
	lda #1
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 322)
	; 12 bytes
	php
	lda #2
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 323)
	; 12 bytes
	php
	lda #3
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 324)
	; 12 bytes
	php
	lda #4
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 325)
	; 12 bytes
	php
	lda #5
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 326)
	; 12 bytes
	php
	lda #6
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 327)
	; 12 bytes
	php
	lda #7
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 328)
	; 12 bytes
	php
	lda #8
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 329)
	; 12 bytes
	php
	lda #9
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 330)
	; 12 bytes
	php
	lda #10
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 331)
	; 12 bytes
	php
	lda #11
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.endif
.else
; 332 to 351
.if(n = 332)
	; 12 bytes
	php
	lda #12
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 333)
	; 12 bytes
	php
	lda #13
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 334)
	; 12 bytes
	php
	lda #14
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 335)
	; 12 bytes
	php
	lda #15
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 336)
	; 12 bytes
	php
	lda #16
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 337)
	; 12 bytes
	php
	lda #17
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 338)
	; 12 bytes
	php
	lda #18
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 339)
	; 12 bytes
	php
	lda #19
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 340)
	; 12 bytes
	php
	lda #20
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 341)
	; 12 bytes
	php
	lda #21
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 342)
	; 12 bytes
	php
	lda #22
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 343)
	; 12 bytes
	php
	lda #23
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 344)
	; 12 bytes
	php
	lda #24
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 345)
	; 12 bytes
	php
	lda #25
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 346)
	; 12 bytes
	php
	lda #26
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 347)
	; 12 bytes
	php
	lda #27
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 348)
	; 12 bytes
	php
	lda #28
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 349)
	; 12 bytes
	php
	lda #29
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 350)
	; 12 bytes
	php
	lda #30
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 351)
	; 12 bytes
	php
	lda #31
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.endif
.endif
.else
.if((n >= 352) && (n < 371))
; 352 to 370
.if(n = 352)
	; 12 bytes
	php
	lda #32
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 353)
	; 12 bytes
	php
	lda #33
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 354)
	; 12 bytes
	php
	lda #34
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 355)
	; 12 bytes
	php
	lda #35
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 356)
	; 12 bytes
	php
	lda #36
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 357)
	; 12 bytes
	php
	lda #37
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 358)
	; 12 bytes
	php
	lda #38
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 359)
	; 12 bytes
	php
	lda #39
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 360)
	; 12 bytes
	php
	lda #40
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 361)
	; 12 bytes
	php
	lda #41
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 362)
	; 12 bytes
	php
	lda #42
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 363)
	; 12 bytes
	php
	lda #43
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 364)
	; 12 bytes
	php
	lda #44
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 365)
	; 12 bytes
	php
	lda #45
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 366)
	; 12 bytes
	php
	lda #46
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 367)
	; 12 bytes
	php
	lda #47
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 368)
	; 12 bytes
	php
	lda #48
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 369)
	; 12 bytes
	php
	lda #49
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 370)
	; 12 bytes
	php
	lda #50
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.endif
.else
; 371 to 390
.if(n = 371)
	; 12 bytes
	php
	lda #51
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 372)
	; 12 bytes
	php
	lda #52
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 373)
	; 12 bytes
	php
	lda #53
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 374)
	; 12 bytes
	php
	lda #54
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 375)
	; 12 bytes
	php
	lda #55
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 376)
	; 12 bytes
	php
	lda #56
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 377)
	; 12 bytes
	php
	lda #57
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 378)
	; 12 bytes
	php
	lda #58
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 379)
	; 12 bytes
	php
	lda #59
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 380)
	; 12 bytes
	php
	lda #60
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 381)
	; 12 bytes
	php
	lda #61
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 382)
	; 12 bytes
	php
	lda #62
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 383)
	; 12 bytes
	php
	lda #63
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 384)
	; 12 bytes
	php
	lda #64
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 385)
	; 12 bytes
	php
	lda #65
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 386)
	; 12 bytes
	php
	lda #66
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 387)
	; 12 bytes
	php
	lda #67
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 388)
	; 12 bytes
	php
	lda #68
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 389)
	; 12 bytes
	php
	lda #69
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 390)
	; 12 bytes
	php
	lda #70
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 391) && (n < 430))
.if((n >= 391) && (n < 410))
; 391 to 409
.if(n = 391)
	; 12 bytes
	php
	lda #71
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 392)
	; 12 bytes
	php
	lda #72
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 393)
	; 12 bytes
	php
	lda #73
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 394)
	; 12 bytes
	php
	lda #74
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 395)
	; 12 bytes
	php
	lda #75
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 396)
	; 12 bytes
	php
	lda #76
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 397)
	; 12 bytes
	php
	lda #77
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 398)
	; 12 bytes
	php
	lda #78
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 399)
	; 12 bytes
	php
	lda #79
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 400)
	; 12 bytes
	php
	lda #80
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 401)
	; 12 bytes
	php
	lda #81
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 402)
	; 12 bytes
	php
	lda #82
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 403)
	; 12 bytes
	php
	lda #83
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 404)
	; 12 bytes
	php
	lda #84
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 405)
	; 12 bytes
	php
	lda #85
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 406)
	; 12 bytes
	php
	lda #86
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 407)
	; 12 bytes
	php
	lda #87
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 408)
	; 12 bytes
	php
	lda #88
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 409)
	; 12 bytes
	php
	lda #89
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.endif
.else
; 410 to 429
.if(n = 410)
	; 12 bytes
	php
	lda #90
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 411)
	; 12 bytes
	php
	lda #91
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 412)
	; 12 bytes
	php
	lda #92
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 413)
	; 12 bytes
	php
	lda #93
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 414)
	; 12 bytes
	php
	lda #94
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 415)
	; 12 bytes
	php
	lda #95
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 416)
	; 12 bytes
	php
	lda #96
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 417)
	; 12 bytes
	php
	lda #97
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 418)
	; 12 bytes
	php
	lda #98
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 419)
	; 12 bytes
	php
	lda #99
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 420)
	; 12 bytes
	php
	lda #100
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 421)
	; 12 bytes
	php
	lda #101
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 422)
	; 12 bytes
	php
	lda #102
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 423)
	; 12 bytes
	php
	lda #103
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 424)
	; 12 bytes
	php
	lda #104
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 425)
	; 12 bytes
	php
	lda #105
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 426)
	; 12 bytes
	php
	lda #106
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 427)
	; 12 bytes
	php
	lda #107
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 428)
	; 12 bytes
	php
	lda #108
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 429)
	; 12 bytes
	php
	lda #109
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.endif
.endif
.else
.if((n >= 430) && (n < 449))
; 430 to 448
.if(n = 430)
	; 12 bytes
	php
	lda #110
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 431)
	; 12 bytes
	php
	lda #111
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 432)
	; 12 bytes
	php
	lda #112
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 433)
	; 12 bytes
	php
	lda #113
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 434)
	; 12 bytes
	php
	lda #114
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 435)
	; 12 bytes
	php
	lda #115
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 436)
	; 12 bytes
	php
	lda #116
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 437)
	; 12 bytes
	php
	lda #117
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 438)
	; 12 bytes
	php
	lda #118
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 439)
	; 12 bytes
	php
	lda #119
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 440)
	; 12 bytes
	php
	lda #120
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 441)
	; 12 bytes
	php
	lda #121
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 442)
	; 12 bytes
	php
	lda #122
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 443)
	; 12 bytes
	php
	lda #123
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 444)
	; 12 bytes
	php
	lda #124
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 445)
	; 12 bytes
	php
	lda #125
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 446)
	; 12 bytes
	php
	lda #126
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 447)
	; 12 bytes
	php
	lda #127
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 448)
	; 12 bytes
	php
	lda #128
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.endif
.else
; 449 to 468
.if(n = 449)
	; 12 bytes
	php
	lda #129
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 450)
	; 12 bytes
	php
	lda #130
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 451)
	; 12 bytes
	php
	lda #131
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 452)
	; 12 bytes
	php
	lda #132
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 453)
	; 12 bytes
	php
	lda #133
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 454)
	; 12 bytes
	php
	lda #134
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 455)
	; 12 bytes
	php
	lda #135
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 456)
	; 12 bytes
	php
	lda #136
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 457)
	; 12 bytes
	php
	lda #137
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 458)
	; 12 bytes
	php
	lda #138
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 459)
	; 12 bytes
	php
	lda #139
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 460)
	; 12 bytes
	php
	lda #140
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 461)
	; 12 bytes
	php
	lda #141
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 462)
	; 12 bytes
	php
	lda #142
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 463)
	; 12 bytes
	php
	lda #143
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 464)
	; 12 bytes
	php
	lda #144
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 465)
	; 12 bytes
	php
	lda #145
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 466)
	; 12 bytes
	php
	lda #146
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 467)
	; 12 bytes
	php
	lda #147
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 468)
	; 12 bytes
	php
	lda #148
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
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
	; 12 bytes
	php
	lda #149
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 470)
	; 12 bytes
	php
	lda #150
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 471)
	; 12 bytes
	php
	lda #151
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 472)
	; 12 bytes
	php
	lda #152
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 473)
	; 12 bytes
	php
	lda #153
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 474)
	; 12 bytes
	php
	lda #154
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 475)
	; 12 bytes
	php
	lda #155
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 476)
	; 12 bytes
	php
	lda #156
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 477)
	; 12 bytes
	php
	lda #157
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 478)
	; 12 bytes
	php
	lda #158
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 479)
	; 12 bytes
	php
	lda #159
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 480)
	; 12 bytes
	php
	lda #160
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 481)
	; 12 bytes
	php
	lda #161
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 482)
	; 12 bytes
	php
	lda #162
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 483)
	; 12 bytes
	php
	lda #163
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 484)
	; 12 bytes
	php
	lda #164
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 485)
	; 12 bytes
	php
	lda #165
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 486)
	; 12 bytes
	php
	lda #166
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 487)
	; 12 bytes
	php
	lda #167
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.endif
.else
; 488 to 507
.if(n = 488)
	; 12 bytes
	php
	lda #168
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 489)
	; 12 bytes
	php
	lda #169
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 490)
	; 12 bytes
	php
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 491)
	; 12 bytes
	php
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 492)
	; 13 bytes
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif(n = 493)
	; 13 bytes
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	plp
.elseif((n > 493 && n < 508))
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
.endif
.endif
.else
; 508 to 546
.if((n > 507 && n < 547))
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
.endif
.endif
.else
; 547 to 625
.if((n > 546 && n < 562) || n = 563 || n = 565 || n = 566 || n = 568 || (n > 569 && n < 626))
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
.elseif(n = 562)
	; 11 bytes
	php
	stx @zptemp
	lax #2
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 564 || n = 569)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 567)
	; 11 bytes
	php
	txa
	pha
	lax #2
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 819) || n = 820 || n = 822 || n = 823 || n = 825 || (n > 826 && n < 1076) || n = 1077 || n = 1079 || n = 1080 || n = 1082 || (n > 1083 && n < 1251))
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
.elseif(n = 819)
	; 11 bytes
	php
	stx @zptemp
	lax #3
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 821 || n = 826 || n = 1078 || n = 1083)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 824)
	; 11 bytes
	php
	txa
	pha
	lax #3
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 1076)
	; 11 bytes
	php
	stx @zptemp
	lax #4
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 1081)
	; 11 bytes
	php
	txa
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1333) || n = 1334 || n = 1336 || n = 1337 || n = 1339 || (n > 1340 && n < 1590) || n = 1591 || n = 1593 || n = 1594 || n = 1596 || (n > 1597 && n < 1847) || n = 1848 || n = 1850 || n = 1851 || n = 1853 || (n > 1854 && n < 2104) || n = 2105 || n = 2107 || n = 2108 || n = 2110 || (n > 2111 && n < 2361) || n = 2362 || n = 2364 || n = 2365 || n = 2367 || (n > 2368 && n < 2501))
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
.elseif(n = 1333)
	; 11 bytes
	php
	stx @zptemp
	lax #5
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 1335 || n = 1340 || n = 1592 || n = 1597 || n = 1849 || n = 1854 || n = 2106 || n = 2111 || n = 2363 || n = 2368)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1338)
	; 11 bytes
	php
	txa
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 1590)
	; 11 bytes
	php
	stx @zptemp
	lax #6
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 1595)
	; 11 bytes
	php
	txa
	pha
	lax #6
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 1847)
	; 11 bytes
	php
	stx @zptemp
	lax #7
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 1852)
	; 11 bytes
	php
	txa
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 2104)
	; 11 bytes
	php
	stx @zptemp
	lax #8
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 2109)
	; 11 bytes
	php
	txa
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 2361)
	; 11 bytes
	php
	stx @zptemp
	lax #9
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2618) || n = 2619 || n = 2621 || n = 2622 || n = 2624 || (n > 2625 && n < 2875) || n = 2876 || n = 2878 || n = 2879 || n = 2881 || (n > 2882 && n < 3132) || n = 3133 || n = 3135 || n = 3136 || n = 3138 || (n > 3139 && n < 3389) || n = 3390 || n = 3392 || n = 3393 || n = 3395 || (n > 3396 && n < 3646) || n = 3647 || n = 3649 || n = 3650 || n = 3652 || (n > 3653 && n < 3903) || n = 3904 || n = 3906 || n = 3907 || n = 3909 || (n > 3910 && n < 4160) || n = 4161 || n = 4163 || n = 4164 || n = 4166 || (n > 4167 && n < 4417) || n = 4418 || n = 4420 || n = 4421 || n = 4423 || (n > 4424 && n < 4674) || n = 4675 || n = 4677 || n = 4678 || n = 4680 || (n > 4681 && n < 4931) || n = 4932 || n = 4934 || n = 4935 || n = 4937 || (n > 4938 && n < 5001))
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
.elseif(n = 2618)
	; 11 bytes
	php
	stx @zptemp
	lax #10
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 2620 || n = 2625 || n = 2877 || n = 2882 || n = 3134 || n = 3139 || n = 3391 || n = 3396 || n = 3648 || n = 3653 || n = 3905 || n = 3910 || n = 4162 || n = 4167 || n = 4419 || n = 4424 || n = 4676 || n = 4681 || n = 4933 || n = 4938)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2623)
	; 11 bytes
	php
	txa
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 2875)
	; 11 bytes
	php
	stx @zptemp
	lax #11
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 2880)
	; 11 bytes
	php
	txa
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3132)
	; 11 bytes
	php
	stx @zptemp
	lax #12
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 3389)
	; 11 bytes
	php
	stx @zptemp
	lax #13
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 3646)
	; 11 bytes
	php
	stx @zptemp
	lax #14
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 3903)
	; 11 bytes
	php
	stx @zptemp
	lax #15
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 4160)
	; 11 bytes
	php
	stx @zptemp
	lax #16
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 4165)
	; 11 bytes
	php
	txa
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4417)
	; 11 bytes
	php
	stx @zptemp
	lax #17
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 4674)
	; 11 bytes
	php
	stx @zptemp
	lax #18
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 4679)
	; 11 bytes
	php
	txa
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4931)
	; 11 bytes
	php
	stx @zptemp
	lax #19
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
; 5001 to 7500
.if((n > 5000 && n < 5188) || n = 5189 || n = 5191 || n = 5192 || n = 5194 || (n > 5195 && n < 5445) || n = 5446 || n = 5448 || n = 5449 || n = 5451 || (n > 5452 && n < 5702) || n = 5703 || n = 5705 || n = 5706 || n = 5708 || (n > 5709 && n < 5959) || n = 5960 || n = 5962 || n = 5963 || n = 5965 || (n > 5966 && n < 6216) || n = 6217 || n = 6219 || n = 6220 || n = 6222 || (n > 6223 && n < 6473) || n = 6474 || n = 6476 || n = 6477 || n = 6479 || (n > 6480 && n < 6730) || n = 6731 || n = 6733 || n = 6734 || n = 6736 || (n > 6737 && n < 6987) || n = 6988 || n = 6990 || n = 6991 || n = 6993 || (n > 6994 && n < 7244) || n = 7245 || n = 7247 || n = 7248 || n = 7250 || (n > 7251 && n < 7501))
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
.elseif(n = 5188)
	; 11 bytes
	php
	stx @zptemp
	lax #20
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
.elseif(n = 5190 || n = 5195 || n = 5447 || n = 5452 || n = 5704 || n = 5709 || n = 5961 || n = 5966 || n = 6218 || n = 6223 || n = 6475 || n = 6480 || n = 6732 || n = 6737 || n = 6989 || n = 6994 || n = 7246 || n = 7251)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5702)
	; 11 bytes
	php
	stx @zptemp
	lax #22
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 5959)
	; 11 bytes
	php
	stx @zptemp
	lax #23
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 6216)
	; 11 bytes
	php
	stx @zptemp
	lax #24
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 6473)
	; 11 bytes
	php
	stx @zptemp
	lax #25
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 6730)
	; 11 bytes
	php
	stx @zptemp
	lax #26
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 6987)
	; 11 bytes
	php
	stx @zptemp
	lax #27
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	plp
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
.elseif(n = 7502 || n = 7504 || n = 7505 || n = 7507 || (n > 7508 && n < 7758) || n = 7759 || n = 7761 || n = 7762 || n = 7764 || (n > 7765 && n < 8015) || n = 8016 || n = 8018 || n = 8019 || n = 8021 || (n > 8022 && n < 8272) || n = 8273 || n = 8275 || n = 8276 || n = 8278 || (n > 8279 && n < 8529) || n = 8530 || n = 8532 || n = 8533 || n = 8535 || (n > 8536 && n < 8786) || n = 8787 || n = 8789 || n = 8790 || n = 8792 || (n > 8793 && n < 9043) || n = 9044 || n = 9046 || n = 9047 || n = 9049 || (n > 9050 && n < 9300) || n = 9301 || n = 9303 || n = 9304 || n = 9306 || (n > 9307 && n < 9557) || n = 9558 || n = 9560 || n = 9561 || n = 9563 || (n > 9564 && n < 9814) || n = 9815 || n = 9817 || n = 9818 || n = 9820 || (n > 9821 && n < 10001))
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
.if((n > 10000 && n < 10071) || n = 10072 || n = 10074 || n = 10075 || n = 10077 || (n > 10078 && n < 10328) || n = 10329 || n = 10331 || n = 10332 || n = 10334 || (n > 10335 && n < 10585) || n = 10586 || n = 10588 || n = 10589 || n = 10591 || (n > 10592 && n < 10842) || n = 10843 || n = 10845 || n = 10846 || n = 10848 || (n > 10849 && n < 11099) || n = 11100 || n = 11102 || n = 11103 || n = 11105 || (n > 11106 && n < 11356) || n = 11357 || n = 11359 || n = 11360 || n = 11362 || (n > 11363 && n < 11613) || n = 11614 || n = 11616 || n = 11617 || n = 11619 || (n > 11620 && n < 11870) || n = 11871 || n = 11873 || n = 11874 || n = 11876 || (n > 11877 && n < 12127) || n = 12128 || n = 12130 || n = 12131 || n = 12133 || (n > 12134 && n < 12384) || n = 12385 || n = 12387 || n = 12388 || n = 12390 || (n > 12391 && n < 12501))
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
.if((n > 12500 && n < 12641) || n = 12642 || n = 12644 || n = 12645 || n = 12647 || (n > 12648 && n < 12898) || n = 12899 || n = 12901 || n = 12902 || n = 12904 || (n > 12905 && n < 13155) || n = 13156 || n = 13158 || n = 13159 || n = 13161 || (n > 13162 && n < 13412) || n = 13413 || n = 13415 || n = 13416 || n = 13418 || (n > 13419 && n < 13669) || n = 13670 || n = 13672 || n = 13673 || n = 13675 || (n > 13676 && n < 13926) || n = 13927 || n = 13929 || n = 13930 || n = 13932 || (n > 13933 && n < 14183) || n = 14184 || n = 14186 || n = 14187 || n = 14189 || (n > 14190 && n < 14440) || n = 14441 || n = 14443 || n = 14444 || n = 14446 || (n > 14447 && n < 14697) || n = 14698 || n = 14700 || n = 14701 || n = 14703 || (n > 14704 && n < 14954) || n = 14955 || n = 14957 || n = 14958 || n = 14960 || (n > 14961 && n < 15001))
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
.if((n > 15000 && n < 15211) || n = 15212 || n = 15214 || n = 15215 || n = 15217 || (n > 15218 && n < 15468) || n = 15469 || n = 15471 || n = 15472 || n = 15474 || (n > 15475 && n < 15725) || n = 15726 || n = 15728 || n = 15729 || n = 15731 || (n > 15732 && n < 15982) || n = 15983 || n = 15985 || n = 15986 || n = 15988 || (n > 15989 && n < 16239) || n = 16240 || n = 16242 || n = 16243 || n = 16245 || (n > 16246 && n < 16496) || n = 16497 || n = 16499 || n = 16500 || n = 16502 || (n > 16503 && n < 16753) || n = 16754 || n = 16756 || n = 16757 || n = 16759 || (n > 16760 && n < 17010) || n = 17011 || n = 17013 || n = 17014 || n = 17016 || (n > 17017 && n < 17267) || n = 17268 || n = 17270 || n = 17271 || n = 17273 || (n > 17274 && n < 17501))
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
.if((n > 17500 && n < 17524) || n = 17525 || n = 17527 || n = 17528 || n = 17530 || (n > 17531 && n < 17781) || n = 17782 || n = 17784 || n = 17785 || n = 17787 || (n > 17788 && n < 18038) || n = 18039 || n = 18041 || n = 18042 || n = 18044 || (n > 18045 && n < 18295) || n = 18296 || n = 18298 || n = 18299 || n = 18301 || (n > 18302 && n < 18552) || n = 18553 || n = 18555 || n = 18556 || n = 18558 || (n > 18559 && n < 18809) || n = 18810 || n = 18812 || n = 18813 || n = 18815 || (n > 18816 && n < 19066) || n = 19067 || n = 19069 || n = 19070 || n = 19072 || (n > 19073 && n < 19323) || n = 19324 || n = 19326 || n = 19327 || n = 19329 || (n > 19330 && n < 19580) || n = 19581 || n = 19583 || n = 19584 || n = 19586 || (n > 19587 && n < 19837) || n = 19838 || n = 19840 || n = 19841 || n = 19843 || (n > 19844 && n < 20001))
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
