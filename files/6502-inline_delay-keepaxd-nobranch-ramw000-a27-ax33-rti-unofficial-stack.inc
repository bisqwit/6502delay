; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber Y, S, Z+N, C, and V
; Will preserve A, X, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 291
; - 8 bytes or less when n <= 294
; - 9 bytes or less when n <= 304
; -10 bytes or less when n <= 307
; -11 bytes or less when n <= 317
; -12 bytes or less when n <= 491
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=6, avg=6.8, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=9
; - When 301 <= n <= 500: min=9, avg=11.9, max=13
; - When 501 <= n <= 750: min=11, avg=12.9, max=13
; - When 751 <= n <= 1000: min=11, avg=12.9, max=13
; - When 1001 <= n <= 2000: min=11, avg=12.9, max=13
; - When 2001 <= n <= 3000: min=11, avg=12.9, max=13
; - When 3001 <= n <= 5000: min=11, avg=12.9, max=13
; - When 5001 <= n <= 7500: min=11, avg=12.9, max=13
; - When 7501 <= n <= 10000: min=11, avg=12.9, max=13
; - When 10001 <= n <= 20000: min=11, avg=12.9, max=13
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
	; 1 bytes
	pha
.elseif(n = 4 || n = 8 || n = 17 || n = 28 || n = 292 || n = 293 || n = 308)
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
.elseif((n > 8 && n < 12) || (n > 19 && n < 23) || n = 33)
	; 3..6 bytes
	delay_n (n-7)
	delay_n 7
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
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 39)
	; 3..6 bytes
	delay_n (n-13)
	brk
.elseif(n = 18 || n = 31)
	; 4..6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27 || n = 305)
	; 5..10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 30 || n = 306)
	; 6..10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 34 || n = 35)
	; 7 bytes
	tay
	lda #n-33
	jsr delay_a_27_clocks
	tya
.elseif((n > 35 && n < 39) || (n > 39 && n < 292))
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 294)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n > 294 && n < 299))
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 298 && n < 305) || n = 309 || n = 311)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 307)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 310)
	; 11 bytes
	pha
	txa
	pha
	lax #1
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 312)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
.if((n >= 313) && (n < 332))
; 313 to 331
.if((n > 312 && n < 318))
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 318)
	; 12 bytes
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #253
	jsr delay_a_27_clocks
	pla
.elseif(n = 319)
	; 12 bytes
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #254
	jsr delay_a_27_clocks
	pla
.elseif(n = 320)
	; 12 bytes
	pha
	lda #0
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 321)
	; 12 bytes
	pha
	lda #1
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 322)
	; 12 bytes
	pha
	lda #2
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 323)
	; 12 bytes
	pha
	lda #3
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 324)
	; 12 bytes
	pha
	lda #4
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 325)
	; 12 bytes
	pha
	lda #5
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 326)
	; 12 bytes
	pha
	lda #6
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 327)
	; 12 bytes
	pha
	lda #7
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 328)
	; 12 bytes
	pha
	lda #8
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 329)
	; 12 bytes
	pha
	lda #9
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 330)
	; 12 bytes
	pha
	lda #10
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 331)
	; 12 bytes
	pha
	lda #11
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.endif
.else
; 332 to 351
.if(n = 332)
	; 12 bytes
	pha
	lda #12
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 333)
	; 12 bytes
	pha
	lda #13
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 334)
	; 12 bytes
	pha
	lda #14
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 335)
	; 12 bytes
	pha
	lda #15
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 336)
	; 12 bytes
	pha
	lda #16
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 337)
	; 12 bytes
	pha
	lda #17
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 338)
	; 12 bytes
	pha
	lda #18
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 339)
	; 12 bytes
	pha
	lda #19
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 340)
	; 12 bytes
	pha
	lda #20
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 341)
	; 12 bytes
	pha
	lda #21
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 342)
	; 12 bytes
	pha
	lda #22
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 343)
	; 12 bytes
	pha
	lda #23
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 344)
	; 12 bytes
	pha
	lda #24
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 345)
	; 12 bytes
	pha
	lda #25
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 346)
	; 12 bytes
	pha
	lda #26
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 347)
	; 12 bytes
	pha
	lda #27
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 348)
	; 12 bytes
	pha
	lda #28
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 349)
	; 12 bytes
	pha
	lda #29
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 350)
	; 12 bytes
	pha
	lda #30
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 351)
	; 12 bytes
	pha
	lda #31
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.endif
.endif
.else
.if((n >= 352) && (n < 371))
; 352 to 370
.if(n = 352)
	; 12 bytes
	pha
	lda #32
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 353)
	; 12 bytes
	pha
	lda #33
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 354)
	; 12 bytes
	pha
	lda #34
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 355)
	; 12 bytes
	pha
	lda #35
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 356)
	; 12 bytes
	pha
	lda #36
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 357)
	; 12 bytes
	pha
	lda #37
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 358)
	; 12 bytes
	pha
	lda #38
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 359)
	; 12 bytes
	pha
	lda #39
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 360)
	; 12 bytes
	pha
	lda #40
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 361)
	; 12 bytes
	pha
	lda #41
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 362)
	; 12 bytes
	pha
	lda #42
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 363)
	; 12 bytes
	pha
	lda #43
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 364)
	; 12 bytes
	pha
	lda #44
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 365)
	; 12 bytes
	pha
	lda #45
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 366)
	; 12 bytes
	pha
	lda #46
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 367)
	; 12 bytes
	pha
	lda #47
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 368)
	; 12 bytes
	pha
	lda #48
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 369)
	; 12 bytes
	pha
	lda #49
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 370)
	; 12 bytes
	pha
	lda #50
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.endif
.else
; 371 to 390
.if(n = 371)
	; 12 bytes
	pha
	lda #51
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 372)
	; 12 bytes
	pha
	lda #52
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 373)
	; 12 bytes
	pha
	lda #53
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 374)
	; 12 bytes
	pha
	lda #54
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 375)
	; 12 bytes
	pha
	lda #55
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 376)
	; 12 bytes
	pha
	lda #56
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 377)
	; 12 bytes
	pha
	lda #57
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 378)
	; 12 bytes
	pha
	lda #58
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 379)
	; 12 bytes
	pha
	lda #59
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 380)
	; 12 bytes
	pha
	lda #60
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 381)
	; 12 bytes
	pha
	lda #61
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 382)
	; 12 bytes
	pha
	lda #62
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 383)
	; 12 bytes
	pha
	lda #63
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 384)
	; 12 bytes
	pha
	lda #64
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 385)
	; 12 bytes
	pha
	lda #65
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 386)
	; 12 bytes
	pha
	lda #66
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 387)
	; 12 bytes
	pha
	lda #67
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 388)
	; 12 bytes
	pha
	lda #68
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 389)
	; 12 bytes
	pha
	lda #69
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 390)
	; 12 bytes
	pha
	lda #70
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 391) && (n < 430))
.if((n >= 391) && (n < 410))
; 391 to 409
.if(n = 391)
	; 12 bytes
	pha
	lda #71
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 392)
	; 12 bytes
	pha
	lda #72
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 393)
	; 12 bytes
	pha
	lda #73
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 394)
	; 12 bytes
	pha
	lda #74
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 395)
	; 12 bytes
	pha
	lda #75
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 396)
	; 12 bytes
	pha
	lda #76
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 397)
	; 12 bytes
	pha
	lda #77
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 398)
	; 12 bytes
	pha
	lda #78
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 399)
	; 12 bytes
	pha
	lda #79
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 400)
	; 12 bytes
	pha
	lda #80
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 401)
	; 12 bytes
	pha
	lda #81
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 402)
	; 12 bytes
	pha
	lda #82
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 403)
	; 12 bytes
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 404)
	; 12 bytes
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 405)
	; 12 bytes
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 406)
	; 12 bytes
	pha
	lda #86
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 407)
	; 12 bytes
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 408)
	; 12 bytes
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 409)
	; 12 bytes
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.endif
.else
; 410 to 429
.if(n = 410)
	; 12 bytes
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 411)
	; 12 bytes
	pha
	lda #91
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 412)
	; 12 bytes
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 413)
	; 12 bytes
	pha
	lda #93
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 414)
	; 12 bytes
	pha
	lda #94
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 415)
	; 12 bytes
	pha
	lda #95
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 416)
	; 12 bytes
	pha
	lda #96
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 417)
	; 12 bytes
	pha
	lda #97
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 418)
	; 12 bytes
	pha
	lda #98
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 419)
	; 12 bytes
	pha
	lda #99
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 420)
	; 12 bytes
	pha
	lda #100
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 421)
	; 12 bytes
	pha
	lda #101
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 422)
	; 12 bytes
	pha
	lda #102
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 423)
	; 12 bytes
	pha
	lda #103
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 424)
	; 12 bytes
	pha
	lda #104
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 425)
	; 12 bytes
	pha
	lda #105
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 426)
	; 12 bytes
	pha
	lda #106
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 427)
	; 12 bytes
	pha
	lda #107
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 428)
	; 12 bytes
	pha
	lda #108
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 429)
	; 12 bytes
	pha
	lda #109
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.endif
.endif
.else
.if((n >= 430) && (n < 449))
; 430 to 448
.if(n = 430)
	; 12 bytes
	pha
	lda #110
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 431)
	; 12 bytes
	pha
	lda #111
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 432)
	; 12 bytes
	pha
	lda #112
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 433)
	; 12 bytes
	pha
	lda #113
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 434)
	; 12 bytes
	pha
	lda #114
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 435)
	; 12 bytes
	pha
	lda #115
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 436)
	; 12 bytes
	pha
	lda #116
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 437)
	; 12 bytes
	pha
	lda #117
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 438)
	; 12 bytes
	pha
	lda #118
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 439)
	; 12 bytes
	pha
	lda #119
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 440)
	; 12 bytes
	pha
	lda #120
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 441)
	; 12 bytes
	pha
	lda #121
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 442)
	; 12 bytes
	pha
	lda #122
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 443)
	; 12 bytes
	pha
	lda #123
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 444)
	; 12 bytes
	pha
	lda #124
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 445)
	; 12 bytes
	pha
	lda #125
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 446)
	; 12 bytes
	pha
	lda #126
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 447)
	; 12 bytes
	pha
	lda #127
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 448)
	; 12 bytes
	pha
	lda #128
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.endif
.else
; 449 to 468
.if(n = 449)
	; 12 bytes
	pha
	lda #129
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 450)
	; 12 bytes
	pha
	lda #130
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 451)
	; 12 bytes
	pha
	lda #131
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 452)
	; 12 bytes
	pha
	lda #132
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 453)
	; 12 bytes
	pha
	lda #133
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 454)
	; 12 bytes
	pha
	lda #134
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 455)
	; 12 bytes
	pha
	lda #135
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 456)
	; 12 bytes
	pha
	lda #136
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 457)
	; 12 bytes
	pha
	lda #137
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 458)
	; 12 bytes
	pha
	lda #138
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 459)
	; 12 bytes
	pha
	lda #139
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 460)
	; 12 bytes
	pha
	lda #140
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 461)
	; 12 bytes
	pha
	lda #141
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 462)
	; 12 bytes
	pha
	lda #142
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 463)
	; 12 bytes
	pha
	lda #143
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 464)
	; 12 bytes
	pha
	lda #144
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 465)
	; 12 bytes
	pha
	lda #145
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 466)
	; 12 bytes
	pha
	lda #146
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 467)
	; 12 bytes
	pha
	lda #147
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 468)
	; 12 bytes
	pha
	lda #148
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
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
	pha
	lda #149
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 470)
	; 12 bytes
	pha
	lda #150
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 471)
	; 12 bytes
	pha
	lda #151
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 472)
	; 12 bytes
	pha
	lda #152
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 473)
	; 12 bytes
	pha
	lda #153
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 474)
	; 12 bytes
	pha
	lda #154
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 475)
	; 12 bytes
	pha
	lda #155
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 476)
	; 12 bytes
	pha
	lda #156
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 477)
	; 12 bytes
	pha
	lda #157
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 478)
	; 12 bytes
	pha
	lda #158
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 479)
	; 12 bytes
	pha
	lda #159
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 480)
	; 12 bytes
	pha
	lda #160
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 481)
	; 12 bytes
	pha
	lda #161
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 482)
	; 12 bytes
	pha
	lda #162
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 483)
	; 12 bytes
	pha
	lda #163
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 484)
	; 12 bytes
	pha
	lda #164
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 485)
	; 12 bytes
	pha
	lda #165
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 486)
	; 12 bytes
	pha
	lda #166
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 487)
	; 12 bytes
	pha
	lda #167
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.endif
.else
; 488 to 507
.if(n = 488)
	; 12 bytes
	pha
	lda #168
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 489)
	; 12 bytes
	pha
	lda #169
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 490)
	; 12 bytes
	pha
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 491)
	; 12 bytes
	pha
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 492)
	; 13 bytes
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif(n = 493)
	; 13 bytes
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #255
	jsr delay_a_27_clocks
	pla
.elseif((n > 493 && n < 508))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.endif
.else
; 508 to 546
.if((n > 507 && n < 547))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.endif
.else
; 547 to 625
.if((n > 546 && n < 559) || n = 560 || (n > 570 && n < 626))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 559)
	; 11 bytes
	tay
	stx @zptemp
	lax #2
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 561 || n = 566 || n = 569)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 562)
	; 11 bytes
	pha
	stx @zptemp
	lax #2
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 563)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #2
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 564)
	; 11 bytes
	pha
	txa
	tay
	lax #2
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 565 || n = 570)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 567)
	; 11 bytes
	pha
	txa
	pha
	lax #2
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 568)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #2
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 816) || n = 817 || (n > 827 && n < 1073) || n = 1074 || (n > 1084 && n < 1251))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 816)
	; 11 bytes
	tay
	stx @zptemp
	lax #3
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 818 || n = 823 || n = 826 || n = 1075 || n = 1080 || n = 1083)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 819)
	; 11 bytes
	pha
	stx @zptemp
	lax #3
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 820)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #3
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 821)
	; 11 bytes
	pha
	txa
	tay
	lax #3
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 822 || n = 827 || n = 1079 || n = 1084)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 824)
	; 11 bytes
	pha
	txa
	pha
	lax #3
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 825)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #3
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 1073)
	; 11 bytes
	tay
	stx @zptemp
	lax #4
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 1076)
	; 11 bytes
	pha
	stx @zptemp
	lax #4
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 1077)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #4
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 1078)
	; 11 bytes
	pha
	txa
	tay
	lax #4
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 1081)
	; 11 bytes
	pha
	txa
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 1082)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
; 1251 to 1875
.if((n > 1250 && n < 1330) || n = 1331 || (n > 1341 && n < 1587) || n = 1588 || (n > 1598 && n < 1844) || n = 1845 || (n > 1855 && n < 1876))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 1330)
	; 11 bytes
	tay
	stx @zptemp
	lax #5
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 1332 || n = 1337 || n = 1340 || n = 1589 || n = 1594 || n = 1597 || n = 1846 || n = 1851 || n = 1854)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1333)
	; 11 bytes
	pha
	stx @zptemp
	lax #5
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 1334)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #5
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 1335)
	; 11 bytes
	pha
	txa
	tay
	lax #5
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 1336 || n = 1341 || n = 1593 || n = 1598 || n = 1850 || n = 1855)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1338)
	; 11 bytes
	pha
	txa
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 1339)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 1587)
	; 11 bytes
	tay
	stx @zptemp
	lax #6
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 1590)
	; 11 bytes
	pha
	stx @zptemp
	lax #6
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 1591)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #6
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 1592)
	; 11 bytes
	pha
	txa
	tay
	lax #6
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 1595)
	; 11 bytes
	pha
	txa
	pha
	lax #6
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 1596)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #6
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 1844)
	; 11 bytes
	tay
	stx @zptemp
	lax #7
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 1847)
	; 11 bytes
	pha
	stx @zptemp
	lax #7
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 1848)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #7
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 1849)
	; 11 bytes
	pha
	txa
	tay
	lax #7
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 1852)
	; 11 bytes
	pha
	txa
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 1853)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 1876 to 2500
.if((n > 1875 && n < 2101) || n = 2102 || (n > 2112 && n < 2358) || n = 2359 || (n > 2369 && n < 2501))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2101)
	; 11 bytes
	tay
	stx @zptemp
	lax #8
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 2103 || n = 2108 || n = 2111 || n = 2360 || n = 2365 || n = 2368)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2104)
	; 11 bytes
	pha
	stx @zptemp
	lax #8
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 2105)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #8
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 2106)
	; 11 bytes
	pha
	txa
	tay
	lax #8
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 2107 || n = 2112 || n = 2364 || n = 2369)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2109)
	; 11 bytes
	pha
	txa
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2110)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 2358)
	; 11 bytes
	tay
	stx @zptemp
	lax #9
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 2361)
	; 11 bytes
	pha
	stx @zptemp
	lax #9
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 2362)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #9
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 2363)
	; 11 bytes
	pha
	txa
	tay
	lax #9
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 2366)
	; 11 bytes
	pha
	txa
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2367)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.endif
.else
.if((n >= 2501) && (n < 3751))
.if((n >= 2501) && (n < 3126))
; 2501 to 3125
.if((n > 2500 && n < 2615) || n = 2616 || (n > 2626 && n < 2872) || n = 2873 || (n > 2883 && n < 3126))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2615)
	; 11 bytes
	tay
	stx @zptemp
	lax #10
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 2617 || n = 2622 || n = 2625 || n = 2874 || n = 2879 || n = 2882)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2618)
	; 11 bytes
	pha
	stx @zptemp
	lax #10
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 2619)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #10
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 2620)
	; 11 bytes
	pha
	txa
	tay
	lax #10
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 2621 || n = 2626 || n = 2878 || n = 2883)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2623)
	; 11 bytes
	pha
	txa
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2624)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 2872)
	; 11 bytes
	tay
	stx @zptemp
	lax #11
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 2875)
	; 11 bytes
	pha
	stx @zptemp
	lax #11
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 2876)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #11
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 2877)
	; 11 bytes
	pha
	txa
	tay
	lax #11
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 2880)
	; 11 bytes
	pha
	txa
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2881)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 3126 to 3750
.if((n > 3125 && n < 3129) || n = 3130 || (n > 3140 && n < 3386) || n = 3387 || (n > 3397 && n < 3643) || n = 3644 || (n > 3654 && n < 3751))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3129)
	; 11 bytes
	tay
	stx @zptemp
	lax #12
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 3131 || n = 3136 || n = 3139 || n = 3388 || n = 3393 || n = 3396 || n = 3645 || n = 3650 || n = 3653)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3132)
	; 11 bytes
	pha
	stx @zptemp
	lax #12
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 3133)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #12
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 3134)
	; 11 bytes
	pha
	txa
	tay
	lax #12
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 3135 || n = 3140 || n = 3392 || n = 3397 || n = 3649 || n = 3654)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3137)
	; 11 bytes
	pha
	txa
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3138)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 3386)
	; 11 bytes
	tay
	stx @zptemp
	lax #13
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 3389)
	; 11 bytes
	pha
	stx @zptemp
	lax #13
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 3390)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #13
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 3391)
	; 11 bytes
	pha
	txa
	tay
	lax #13
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 3394)
	; 11 bytes
	pha
	txa
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3395)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 3643)
	; 11 bytes
	tay
	stx @zptemp
	lax #14
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 3646)
	; 11 bytes
	pha
	stx @zptemp
	lax #14
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 3647)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #14
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 3648)
	; 11 bytes
	pha
	txa
	tay
	lax #14
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 3651)
	; 11 bytes
	pha
	txa
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3652)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
; 3751 to 4375
.if((n > 3750 && n < 3900) || n = 3901 || (n > 3911 && n < 4157) || n = 4158 || (n > 4168 && n < 4376))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3900)
	; 11 bytes
	tay
	stx @zptemp
	lax #15
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 3902 || n = 3907 || n = 3910 || n = 4159 || n = 4164 || n = 4167)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3903)
	; 11 bytes
	pha
	stx @zptemp
	lax #15
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 3904)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #15
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 3905)
	; 11 bytes
	pha
	txa
	tay
	lax #15
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 3906 || n = 3911 || n = 4163 || n = 4168)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3908)
	; 11 bytes
	pha
	txa
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3909)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 4157)
	; 11 bytes
	tay
	stx @zptemp
	lax #16
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 4160)
	; 11 bytes
	pha
	stx @zptemp
	lax #16
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 4161)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #16
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 4162)
	; 11 bytes
	pha
	txa
	tay
	lax #16
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 4165)
	; 11 bytes
	pha
	txa
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 4166)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 4376 to 5000
.if((n > 4375 && n < 4414) || n = 4415 || (n > 4425 && n < 4671) || n = 4672 || (n > 4682 && n < 4928) || n = 4929 || (n > 4939 && n < 5001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 4414)
	; 11 bytes
	tay
	stx @zptemp
	lax #17
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 4416 || n = 4421 || n = 4424 || n = 4673 || n = 4678 || n = 4681 || n = 4930 || n = 4935 || n = 4938)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4417)
	; 11 bytes
	pha
	stx @zptemp
	lax #17
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 4418)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #17
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 4419)
	; 11 bytes
	pha
	txa
	tay
	lax #17
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 4420 || n = 4425 || n = 4677 || n = 4682 || n = 4934 || n = 4939)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4422)
	; 11 bytes
	pha
	txa
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 4423)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 4671)
	; 11 bytes
	tay
	stx @zptemp
	lax #18
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 4674)
	; 11 bytes
	pha
	stx @zptemp
	lax #18
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 4675)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #18
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 4676)
	; 11 bytes
	pha
	txa
	tay
	lax #18
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 4679)
	; 11 bytes
	pha
	txa
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 4680)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 4928)
	; 11 bytes
	tay
	stx @zptemp
	lax #19
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 4931)
	; 11 bytes
	pha
	stx @zptemp
	lax #19
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 4932)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #19
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 4933)
	; 11 bytes
	pha
	txa
	tay
	lax #19
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 4936)
	; 11 bytes
	pha
	txa
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 4937)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
.if((n >= 5001) && (n < 5626))
; 5001 to 5625
.if((n > 5000 && n < 5185) || n = 5186 || (n > 5196 && n < 5442) || n = 5443 || (n > 5453 && n < 5626))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5185)
	; 11 bytes
	tay
	stx @zptemp
	lax #20
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 5187 || n = 5192 || n = 5195 || n = 5444 || n = 5449 || n = 5452)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5188)
	; 11 bytes
	pha
	stx @zptemp
	lax #20
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 5189)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #20
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 5190)
	; 11 bytes
	pha
	txa
	tay
	lax #20
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 5191 || n = 5196 || n = 5448 || n = 5453)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5193)
	; 11 bytes
	pha
	txa
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5194)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 5442)
	; 11 bytes
	tay
	stx @zptemp
	lax #21
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 5445)
	; 11 bytes
	pha
	stx @zptemp
	lax #21
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 5446)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #21
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 5447)
	; 11 bytes
	pha
	txa
	tay
	lax #21
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 5450)
	; 11 bytes
	pha
	txa
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5451)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 5626 to 6250
.if((n > 5625 && n < 5699) || n = 5700 || (n > 5710 && n < 5956) || n = 5957 || (n > 5967 && n < 6213) || n = 6214 || (n > 6224 && n < 6251))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5699)
	; 11 bytes
	tay
	stx @zptemp
	lax #22
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 5701 || n = 5706 || n = 5709 || n = 5958 || n = 5963 || n = 5966 || n = 6215 || n = 6220 || n = 6223)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5702)
	; 11 bytes
	pha
	stx @zptemp
	lax #22
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 5703)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #22
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 5704)
	; 11 bytes
	pha
	txa
	tay
	lax #22
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 5705 || n = 5710 || n = 5962 || n = 5967 || n = 6219 || n = 6224)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5707)
	; 11 bytes
	pha
	txa
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5708)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 5956)
	; 11 bytes
	tay
	stx @zptemp
	lax #23
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 5959)
	; 11 bytes
	pha
	stx @zptemp
	lax #23
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 5960)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #23
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 5961)
	; 11 bytes
	pha
	txa
	tay
	lax #23
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 5964)
	; 11 bytes
	pha
	txa
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5965)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 6213)
	; 11 bytes
	tay
	stx @zptemp
	lax #24
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 6216)
	; 11 bytes
	pha
	stx @zptemp
	lax #24
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 6217)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #24
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 6218)
	; 11 bytes
	pha
	txa
	tay
	lax #24
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 6221)
	; 11 bytes
	pha
	txa
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6222)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.else
.if((n >= 6251) && (n < 6876))
; 6251 to 6875
.if((n > 6250 && n < 6470) || n = 6471 || (n > 6481 && n < 6727) || n = 6728 || (n > 6738 && n < 6876))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6470)
	; 11 bytes
	tay
	stx @zptemp
	lax #25
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 6472 || n = 6477 || n = 6480 || n = 6729 || n = 6734 || n = 6737)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6473)
	; 11 bytes
	pha
	stx @zptemp
	lax #25
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 6474)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #25
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 6475)
	; 11 bytes
	pha
	txa
	tay
	lax #25
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 6476 || n = 6481 || n = 6733 || n = 6738)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6478)
	; 11 bytes
	pha
	txa
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6479)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 6727)
	; 11 bytes
	tay
	stx @zptemp
	lax #26
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 6730)
	; 11 bytes
	pha
	stx @zptemp
	lax #26
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 6731)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #26
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 6732)
	; 11 bytes
	pha
	txa
	tay
	lax #26
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 6735)
	; 11 bytes
	pha
	txa
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6736)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 6876 to 7500
.if((n > 6875 && n < 6984) || n = 6985 || (n > 6995 && n < 7241) || n = 7242 || (n > 7252 && n < 7498) || n = 7499)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6984)
	; 11 bytes
	tay
	stx @zptemp
	lax #27
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 6986 || n = 6991 || n = 6994 || n = 7243 || n = 7248 || n = 7251 || n = 7500)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6987)
	; 11 bytes
	pha
	stx @zptemp
	lax #27
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 6988)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #27
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 6989)
	; 11 bytes
	pha
	txa
	tay
	lax #27
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 6990 || n = 6995 || n = 7247 || n = 7252)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6992)
	; 11 bytes
	pha
	txa
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6993)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 7241)
	; 11 bytes
	tay
	stx @zptemp
	lax #28
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 7244)
	; 11 bytes
	pha
	stx @zptemp
	lax #28
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 7245)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #28
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 7246)
	; 11 bytes
	pha
	txa
	tay
	lax #28
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 7249)
	; 11 bytes
	pha
	txa
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 7250)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 7498)
	; 11 bytes
	tay
	stx @zptemp
	lax #29
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.endif
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
; 7501 to 8125
.if(n = 7501)
	; 11 bytes
	pha
	stx @zptemp
	lax #29
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 7502)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #29
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 7503)
	; 11 bytes
	pha
	txa
	tay
	lax #29
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 7504 || n = 7509 || n = 7761 || n = 7766 || n = 8018 || n = 8023)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7505 || n = 7508 || n = 7757 || n = 7762 || n = 7765 || n = 8014 || n = 8019 || n = 8022)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7506)
	; 11 bytes
	pha
	txa
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 7507)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif((n > 7509 && n < 7755) || n = 7756 || (n > 7766 && n < 8012) || n = 8013 || (n > 8023 && n < 8126))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 7755)
	; 11 bytes
	tay
	stx @zptemp
	lax #30
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 7758)
	; 11 bytes
	pha
	stx @zptemp
	lax #30
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 7759)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #30
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 7760)
	; 11 bytes
	pha
	txa
	tay
	lax #30
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 7763)
	; 11 bytes
	pha
	txa
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 7764)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 8012)
	; 11 bytes
	tay
	stx @zptemp
	lax #31
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 8015)
	; 11 bytes
	pha
	stx @zptemp
	lax #31
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 8016)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #31
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 8017)
	; 11 bytes
	pha
	txa
	tay
	lax #31
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 8020)
	; 11 bytes
	pha
	txa
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8021)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 8126 to 8750
.if((n > 8125 && n < 8269) || n = 8270 || (n > 8280 && n < 8526) || n = 8527 || (n > 8537 && n < 8751))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8269)
	; 11 bytes
	tay
	stx @zptemp
	lax #32
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 8271 || n = 8276 || n = 8279 || n = 8528 || n = 8533 || n = 8536)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8272)
	; 11 bytes
	pha
	stx @zptemp
	lax #32
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 8273)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #32
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 8274)
	; 11 bytes
	pha
	txa
	tay
	lax #32
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 8275 || n = 8280 || n = 8532 || n = 8537)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8277)
	; 11 bytes
	pha
	txa
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8278)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 8526)
	; 11 bytes
	tay
	stx @zptemp
	lax #33
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 8529)
	; 11 bytes
	pha
	stx @zptemp
	lax #33
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 8530)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #33
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 8531)
	; 11 bytes
	pha
	txa
	tay
	lax #33
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 8534)
	; 11 bytes
	pha
	txa
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8535)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.else
.if((n >= 8751) && (n < 9376))
; 8751 to 9375
.if((n > 8750 && n < 8783) || n = 8784 || (n > 8794 && n < 9040) || n = 9041 || (n > 9051 && n < 9297) || n = 9298 || (n > 9308 && n < 9376))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8783)
	; 11 bytes
	tay
	stx @zptemp
	lax #34
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 8785 || n = 8790 || n = 8793 || n = 9042 || n = 9047 || n = 9050 || n = 9299 || n = 9304 || n = 9307)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8786)
	; 11 bytes
	pha
	stx @zptemp
	lax #34
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 8787)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #34
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 8788)
	; 11 bytes
	pha
	txa
	tay
	lax #34
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 8789 || n = 8794 || n = 9046 || n = 9051 || n = 9303 || n = 9308)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8791)
	; 11 bytes
	pha
	txa
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8792)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 9040)
	; 11 bytes
	tay
	stx @zptemp
	lax #35
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 9043)
	; 11 bytes
	pha
	stx @zptemp
	lax #35
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 9044)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #35
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 9045)
	; 11 bytes
	pha
	txa
	tay
	lax #35
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 9048)
	; 11 bytes
	pha
	txa
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 9049)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 9297)
	; 11 bytes
	tay
	stx @zptemp
	lax #36
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 9300)
	; 11 bytes
	pha
	stx @zptemp
	lax #36
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 9301)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #36
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 9302)
	; 11 bytes
	pha
	txa
	tay
	lax #36
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 9305)
	; 11 bytes
	pha
	txa
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 9306)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 9376 to 10000
.if((n > 9375 && n < 9554) || n = 9555 || (n > 9565 && n < 9811) || n = 9812 || (n > 9822 && n < 10001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 9554)
	; 11 bytes
	tay
	stx @zptemp
	lax #37
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 9556 || n = 9561 || n = 9564 || n = 9813 || n = 9818 || n = 9821)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 9557)
	; 11 bytes
	pha
	stx @zptemp
	lax #37
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 9558)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #37
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 9559)
	; 11 bytes
	pha
	txa
	tay
	lax #37
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 9560 || n = 9565 || n = 9817 || n = 9822)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 9562)
	; 11 bytes
	pha
	txa
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 9563)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 9811)
	; 11 bytes
	tay
	stx @zptemp
	lax #38
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 9814)
	; 11 bytes
	pha
	stx @zptemp
	lax #38
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 9815)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #38
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 9816)
	; 11 bytes
	pha
	txa
	tay
	lax #38
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 9819)
	; 11 bytes
	pha
	txa
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 9820)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
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
; 10001 to 10625
.if((n > 10000 && n < 10068) || n = 10069 || (n > 10079 && n < 10325) || n = 10326 || (n > 10336 && n < 10582) || n = 10583 || (n > 10593 && n < 10626))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10068)
	; 11 bytes
	tay
	stx @zptemp
	lax #39
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 10070 || n = 10075 || n = 10078 || n = 10327 || n = 10332 || n = 10335 || n = 10584 || n = 10589 || n = 10592)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10071)
	; 11 bytes
	pha
	stx @zptemp
	lax #39
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 10072)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #39
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 10073)
	; 11 bytes
	pha
	txa
	tay
	lax #39
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 10074 || n = 10079 || n = 10331 || n = 10336 || n = 10588 || n = 10593)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 10076)
	; 11 bytes
	pha
	txa
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10077)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 10325)
	; 11 bytes
	tay
	stx @zptemp
	lax #40
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 10328)
	; 11 bytes
	pha
	stx @zptemp
	lax #40
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 10329)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #40
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 10330)
	; 11 bytes
	pha
	txa
	tay
	lax #40
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 10333)
	; 11 bytes
	pha
	txa
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10334)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 10582)
	; 11 bytes
	tay
	stx @zptemp
	lax #41
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 10585)
	; 11 bytes
	pha
	stx @zptemp
	lax #41
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 10586)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #41
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 10587)
	; 11 bytes
	pha
	txa
	tay
	lax #41
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 10590)
	; 11 bytes
	pha
	txa
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10591)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 10626 to 11250
.if((n > 10625 && n < 10839) || n = 10840 || (n > 10850 && n < 11096) || n = 11097 || (n > 11107 && n < 11251))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10839)
	; 11 bytes
	tay
	stx @zptemp
	lax #42
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 10841 || n = 10846 || n = 10849 || n = 11098 || n = 11103 || n = 11106)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10842)
	; 11 bytes
	pha
	stx @zptemp
	lax #42
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 10843)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #42
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 10844)
	; 11 bytes
	pha
	txa
	tay
	lax #42
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 10845 || n = 10850 || n = 11102 || n = 11107)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 10847)
	; 11 bytes
	pha
	txa
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10848)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 11096)
	; 11 bytes
	tay
	stx @zptemp
	lax #43
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 11099)
	; 11 bytes
	pha
	stx @zptemp
	lax #43
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 11100)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #43
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 11101)
	; 11 bytes
	pha
	txa
	tay
	lax #43
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 11104)
	; 11 bytes
	pha
	txa
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 11105)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.else
.if((n >= 11251) && (n < 11876))
; 11251 to 11875
.if((n > 11250 && n < 11353) || n = 11354 || (n > 11364 && n < 11610) || n = 11611 || (n > 11621 && n < 11867) || n = 11868)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 11353)
	; 11 bytes
	tay
	stx @zptemp
	lax #44
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 11355 || n = 11360 || n = 11363 || n = 11612 || n = 11617 || n = 11620 || n = 11869 || n = 11874)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11356)
	; 11 bytes
	pha
	stx @zptemp
	lax #44
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 11357)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #44
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 11358)
	; 11 bytes
	pha
	txa
	tay
	lax #44
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 11359 || n = 11364 || n = 11616 || n = 11621 || n = 11873)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 11361)
	; 11 bytes
	pha
	txa
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 11362)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 11610)
	; 11 bytes
	tay
	stx @zptemp
	lax #45
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 11613)
	; 11 bytes
	pha
	stx @zptemp
	lax #45
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 11614)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #45
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 11615)
	; 11 bytes
	pha
	txa
	tay
	lax #45
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 11618)
	; 11 bytes
	pha
	txa
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 11619)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 11867)
	; 11 bytes
	tay
	stx @zptemp
	lax #46
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 11870)
	; 11 bytes
	pha
	stx @zptemp
	lax #46
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 11871)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #46
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 11872)
	; 11 bytes
	pha
	txa
	tay
	lax #46
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 11875)
	; 11 bytes
	pha
	txa
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.else
; 11876 to 12500
.if(n = 11876)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 11877 || n = 12126 || n = 12131 || n = 12134 || n = 12383 || n = 12388 || n = 12391)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11878 || n = 12130 || n = 12135 || n = 12387 || n = 12392)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif((n > 11878 && n < 12124) || n = 12125 || (n > 12135 && n < 12381) || n = 12382 || (n > 12392 && n < 12501))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12124)
	; 11 bytes
	tay
	stx @zptemp
	lax #47
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 12127)
	; 11 bytes
	pha
	stx @zptemp
	lax #47
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 12128)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #47
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 12129)
	; 11 bytes
	pha
	txa
	tay
	lax #47
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 12132)
	; 11 bytes
	pha
	txa
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12133)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 12381)
	; 11 bytes
	tay
	stx @zptemp
	lax #48
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 12384)
	; 11 bytes
	pha
	stx @zptemp
	lax #48
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 12385)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #48
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 12386)
	; 11 bytes
	pha
	txa
	tay
	lax #48
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 12389)
	; 11 bytes
	pha
	txa
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12390)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.endif
.else
.if((n >= 12501) && (n < 13751))
.if((n >= 12501) && (n < 13126))
; 12501 to 13125
.if((n > 12500 && n < 12638) || n = 12639 || (n > 12649 && n < 12895) || n = 12896 || (n > 12906 && n < 13126))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12638)
	; 11 bytes
	tay
	stx @zptemp
	lax #49
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 12640 || n = 12645 || n = 12648 || n = 12897 || n = 12902 || n = 12905)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12641)
	; 11 bytes
	pha
	stx @zptemp
	lax #49
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 12642)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #49
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 12643)
	; 11 bytes
	pha
	txa
	tay
	lax #49
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 12644 || n = 12649 || n = 12901 || n = 12906)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 12646)
	; 11 bytes
	pha
	txa
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12647)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 12895)
	; 11 bytes
	tay
	stx @zptemp
	lax #50
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 12898)
	; 11 bytes
	pha
	stx @zptemp
	lax #50
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 12899)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #50
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 12900)
	; 11 bytes
	pha
	txa
	tay
	lax #50
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 12903)
	; 11 bytes
	pha
	txa
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12904)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 13126 to 13750
.if((n > 13125 && n < 13152) || n = 13153 || (n > 13163 && n < 13409) || n = 13410 || (n > 13420 && n < 13666) || n = 13667 || (n > 13677 && n < 13751))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13152)
	; 11 bytes
	tay
	stx @zptemp
	lax #51
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 13154 || n = 13159 || n = 13162 || n = 13411 || n = 13416 || n = 13419 || n = 13668 || n = 13673 || n = 13676)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13155)
	; 11 bytes
	pha
	stx @zptemp
	lax #51
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 13156)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #51
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 13157)
	; 11 bytes
	pha
	txa
	tay
	lax #51
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 13158 || n = 13163 || n = 13415 || n = 13420 || n = 13672 || n = 13677)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 13160)
	; 11 bytes
	pha
	txa
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13161)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 13409)
	; 11 bytes
	tay
	stx @zptemp
	lax #52
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 13412)
	; 11 bytes
	pha
	stx @zptemp
	lax #52
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 13413)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #52
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 13414)
	; 11 bytes
	pha
	txa
	tay
	lax #52
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 13417)
	; 11 bytes
	pha
	txa
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13418)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 13666)
	; 11 bytes
	tay
	stx @zptemp
	lax #53
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 13669)
	; 11 bytes
	pha
	stx @zptemp
	lax #53
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 13670)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #53
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 13671)
	; 11 bytes
	pha
	txa
	tay
	lax #53
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 13674)
	; 11 bytes
	pha
	txa
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13675)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.else
.if((n >= 13751) && (n < 14376))
; 13751 to 14375
.if((n > 13750 && n < 13923) || n = 13924 || (n > 13934 && n < 14180) || n = 14181 || (n > 14191 && n < 14376))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13923)
	; 11 bytes
	tay
	stx @zptemp
	lax #54
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 13925 || n = 13930 || n = 13933 || n = 14182 || n = 14187 || n = 14190)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13926)
	; 11 bytes
	pha
	stx @zptemp
	lax #54
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 13927)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #54
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 13928)
	; 11 bytes
	pha
	txa
	tay
	lax #54
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 13929 || n = 13934 || n = 14186 || n = 14191)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 13931)
	; 11 bytes
	pha
	txa
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13932)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 14180)
	; 11 bytes
	tay
	stx @zptemp
	lax #55
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 14183)
	; 11 bytes
	pha
	stx @zptemp
	lax #55
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 14184)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #55
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 14185)
	; 11 bytes
	pha
	txa
	tay
	lax #55
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 14188)
	; 11 bytes
	pha
	txa
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 14189)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 14376 to 15000
.if((n > 14375 && n < 14437) || n = 14438 || (n > 14448 && n < 14694) || n = 14695 || (n > 14705 && n < 14951) || n = 14952 || (n > 14962 && n < 15001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 14437)
	; 11 bytes
	tay
	stx @zptemp
	lax #56
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 14439 || n = 14444 || n = 14447 || n = 14696 || n = 14701 || n = 14704 || n = 14953 || n = 14958 || n = 14961)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14440)
	; 11 bytes
	pha
	stx @zptemp
	lax #56
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 14441)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #56
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 14442)
	; 11 bytes
	pha
	txa
	tay
	lax #56
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 14443 || n = 14448 || n = 14700 || n = 14705 || n = 14957 || n = 14962)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 14445)
	; 11 bytes
	pha
	txa
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 14446)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 14694)
	; 11 bytes
	tay
	stx @zptemp
	lax #57
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 14697)
	; 11 bytes
	pha
	stx @zptemp
	lax #57
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 14698)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #57
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 14699)
	; 11 bytes
	pha
	txa
	tay
	lax #57
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 14702)
	; 11 bytes
	pha
	txa
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 14703)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 14951)
	; 11 bytes
	tay
	stx @zptemp
	lax #58
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 14954)
	; 11 bytes
	pha
	stx @zptemp
	lax #58
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 14955)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #58
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 14956)
	; 11 bytes
	pha
	txa
	tay
	lax #58
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 14959)
	; 11 bytes
	pha
	txa
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 14960)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
.if((n >= 15001) && (n < 15626))
; 15001 to 15625
.if((n > 15000 && n < 15208) || n = 15209 || (n > 15219 && n < 15465) || n = 15466 || (n > 15476 && n < 15626))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15208)
	; 11 bytes
	tay
	stx @zptemp
	lax #59
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 15210 || n = 15215 || n = 15218 || n = 15467 || n = 15472 || n = 15475)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15211)
	; 11 bytes
	pha
	stx @zptemp
	lax #59
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 15212)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #59
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 15213)
	; 11 bytes
	pha
	txa
	tay
	lax #59
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 15214 || n = 15219 || n = 15471 || n = 15476)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 15216)
	; 11 bytes
	pha
	txa
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15217)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 15465)
	; 11 bytes
	tay
	stx @zptemp
	lax #60
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 15468)
	; 11 bytes
	pha
	stx @zptemp
	lax #60
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 15469)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #60
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 15470)
	; 11 bytes
	pha
	txa
	tay
	lax #60
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 15473)
	; 11 bytes
	pha
	txa
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15474)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 15626 to 16250
.if((n > 15625 && n < 15722) || n = 15723 || (n > 15733 && n < 15979) || n = 15980 || (n > 15990 && n < 16236) || n = 16237 || (n > 16247 && n < 16251))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15722)
	; 11 bytes
	tay
	stx @zptemp
	lax #61
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 15724 || n = 15729 || n = 15732 || n = 15981 || n = 15986 || n = 15989 || n = 16238 || n = 16243 || n = 16246)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15725)
	; 11 bytes
	pha
	stx @zptemp
	lax #61
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 15726)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #61
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 15727)
	; 11 bytes
	pha
	txa
	tay
	lax #61
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 15728 || n = 15733 || n = 15985 || n = 15990 || n = 16242 || n = 16247)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 15730)
	; 11 bytes
	pha
	txa
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15731)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 15979)
	; 11 bytes
	tay
	stx @zptemp
	lax #62
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 15982)
	; 11 bytes
	pha
	stx @zptemp
	lax #62
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 15983)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #62
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 15984)
	; 11 bytes
	pha
	txa
	tay
	lax #62
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 15987)
	; 11 bytes
	pha
	txa
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15988)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 16236)
	; 11 bytes
	tay
	stx @zptemp
	lax #63
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 16239)
	; 11 bytes
	pha
	stx @zptemp
	lax #63
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 16240)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #63
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 16241)
	; 11 bytes
	pha
	txa
	tay
	lax #63
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 16244)
	; 11 bytes
	pha
	txa
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 16245)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.else
.if((n >= 16251) && (n < 16876))
; 16251 to 16875
.if((n > 16250 && n < 16493) || n = 16494 || (n > 16504 && n < 16750) || n = 16751 || (n > 16761 && n < 16876))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 16493)
	; 11 bytes
	tay
	stx @zptemp
	lax #64
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 16495 || n = 16500 || n = 16503 || n = 16752 || n = 16757 || n = 16760)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16496)
	; 11 bytes
	pha
	stx @zptemp
	lax #64
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 16497)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #64
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 16498)
	; 11 bytes
	pha
	txa
	tay
	lax #64
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 16499 || n = 16504 || n = 16756 || n = 16761)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 16501)
	; 11 bytes
	pha
	txa
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 16502)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 16750)
	; 11 bytes
	tay
	stx @zptemp
	lax #65
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 16753)
	; 11 bytes
	pha
	stx @zptemp
	lax #65
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 16754)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #65
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 16755)
	; 11 bytes
	pha
	txa
	tay
	lax #65
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 16758)
	; 11 bytes
	pha
	txa
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 16759)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 16876 to 17500
.if((n > 16875 && n < 17007) || n = 17008 || (n > 17018 && n < 17264) || n = 17265 || (n > 17275 && n < 17501))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17007)
	; 11 bytes
	tay
	stx @zptemp
	lax #66
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 17009 || n = 17014 || n = 17017 || n = 17266 || n = 17271 || n = 17274)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17010)
	; 11 bytes
	pha
	stx @zptemp
	lax #66
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 17011)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #66
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 17012)
	; 11 bytes
	pha
	txa
	tay
	lax #66
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 17013 || n = 17018 || n = 17270 || n = 17275)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 17015)
	; 11 bytes
	pha
	txa
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17016)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 17264)
	; 11 bytes
	tay
	stx @zptemp
	lax #67
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 17267)
	; 11 bytes
	pha
	stx @zptemp
	lax #67
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 17268)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #67
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 17269)
	; 11 bytes
	pha
	txa
	tay
	lax #67
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 17272)
	; 11 bytes
	pha
	txa
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17273)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.endif
.else
.if((n >= 17501) && (n < 18751))
.if((n >= 17501) && (n < 18126))
; 17501 to 18125
.if((n > 17500 && n < 17521) || n = 17522 || (n > 17532 && n < 17778) || n = 17779 || (n > 17789 && n < 18035) || n = 18036 || (n > 18046 && n < 18126))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17521)
	; 11 bytes
	tay
	stx @zptemp
	lax #68
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 17523 || n = 17528 || n = 17531 || n = 17780 || n = 17785 || n = 17788 || n = 18037 || n = 18042 || n = 18045)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17524)
	; 11 bytes
	pha
	stx @zptemp
	lax #68
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 17525)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #68
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 17526)
	; 11 bytes
	pha
	txa
	tay
	lax #68
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 17527 || n = 17532 || n = 17784 || n = 17789 || n = 18041 || n = 18046)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 17529)
	; 11 bytes
	pha
	txa
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17530)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 17778)
	; 11 bytes
	tay
	stx @zptemp
	lax #69
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 17781)
	; 11 bytes
	pha
	stx @zptemp
	lax #69
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 17782)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #69
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 17783)
	; 11 bytes
	pha
	txa
	tay
	lax #69
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 17786)
	; 11 bytes
	pha
	txa
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17787)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 18035)
	; 11 bytes
	tay
	stx @zptemp
	lax #70
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 18038)
	; 11 bytes
	pha
	stx @zptemp
	lax #70
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 18039)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #70
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 18040)
	; 11 bytes
	pha
	txa
	tay
	lax #70
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 18043)
	; 11 bytes
	pha
	txa
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18044)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 18126 to 18750
.if((n > 18125 && n < 18292) || n = 18293 || (n > 18303 && n < 18549) || n = 18550 || (n > 18560 && n < 18751))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18292)
	; 11 bytes
	tay
	stx @zptemp
	lax #71
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 18294 || n = 18299 || n = 18302 || n = 18551 || n = 18556 || n = 18559)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18295)
	; 11 bytes
	pha
	stx @zptemp
	lax #71
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 18296)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #71
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 18297)
	; 11 bytes
	pha
	txa
	tay
	lax #71
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 18298 || n = 18303 || n = 18555 || n = 18560)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 18300)
	; 11 bytes
	pha
	txa
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18301)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 18549)
	; 11 bytes
	tay
	stx @zptemp
	lax #72
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 18552)
	; 11 bytes
	pha
	stx @zptemp
	lax #72
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 18553)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #72
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 18554)
	; 11 bytes
	pha
	txa
	tay
	lax #72
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 18557)
	; 11 bytes
	pha
	txa
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18558)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.else
.if((n >= 18751) && (n < 19376))
; 18751 to 19375
.if((n > 18750 && n < 18806) || n = 18807 || (n > 18817 && n < 19063) || n = 19064 || (n > 19074 && n < 19320) || n = 19321 || (n > 19331 && n < 19376))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18806)
	; 11 bytes
	tay
	stx @zptemp
	lax #73
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 18808 || n = 18813 || n = 18816 || n = 19065 || n = 19070 || n = 19073 || n = 19322 || n = 19327 || n = 19330)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18809)
	; 11 bytes
	pha
	stx @zptemp
	lax #73
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 18810)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #73
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 18811)
	; 11 bytes
	pha
	txa
	tay
	lax #73
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 18812 || n = 18817 || n = 19069 || n = 19074 || n = 19326 || n = 19331)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 18814)
	; 11 bytes
	pha
	txa
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18815)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 19063)
	; 11 bytes
	tay
	stx @zptemp
	lax #74
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 19066)
	; 11 bytes
	pha
	stx @zptemp
	lax #74
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 19067)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #74
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 19068)
	; 11 bytes
	pha
	txa
	tay
	lax #74
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 19071)
	; 11 bytes
	pha
	txa
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 19072)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 19320)
	; 11 bytes
	tay
	stx @zptemp
	lax #75
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 19323)
	; 11 bytes
	pha
	stx @zptemp
	lax #75
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 19324)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #75
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 19325)
	; 11 bytes
	pha
	txa
	tay
	lax #75
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 19328)
	; 11 bytes
	pha
	txa
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 19329)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.else
; 19376 to 20000
.if((n > 19375 && n < 19577) || n = 19578 || (n > 19588 && n < 19834) || n = 19835 || (n > 19845 && n < 20001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 19577)
	; 11 bytes
	tay
	stx @zptemp
	lax #76
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 19579 || n = 19584 || n = 19587 || n = 19836 || n = 19841 || n = 19844)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 19580)
	; 11 bytes
	pha
	stx @zptemp
	lax #76
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 19581)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #76
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 19582)
	; 11 bytes
	pha
	txa
	tay
	lax #76
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 19583 || n = 19588 || n = 19840 || n = 19845)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 19585)
	; 11 bytes
	pha
	txa
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 19586)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.elseif(n = 19834)
	; 11 bytes
	tay
	stx @zptemp
	lax #77
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 19837)
	; 11 bytes
	pha
	stx @zptemp
	lax #77
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	pla
.elseif(n = 19838)
	; 12 bytes
	tay
	stx @zptemp
	pla
	lax #77
	jsr delay_256a_x_33_clocks
	ldx @zptemp
	tya
.elseif(n = 19839)
	; 11 bytes
	pha
	txa
	tay
	lax #77
	jsr delay_256a_x_33_clocks
	tya
	tax
	pla
.elseif(n = 19842)
	; 11 bytes
	pha
	txa
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 19843)
	; 12 bytes
	tay
	pla
	txa
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	tax
	tya
.endif
.endif
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
