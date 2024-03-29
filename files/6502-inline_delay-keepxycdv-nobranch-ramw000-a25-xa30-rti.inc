; delay_n, delays n cycles with small code
; Will clobber A, and Z+N
; Will preserve X, Y, S, C, D, V, and I (interrupt-disable flag)
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
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 9
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 289
; - 8 bytes or less when n <= 291
; - 9 bytes or less when n <= 302
; -10 bytes or less when n <= 304
; -11 bytes or less when n <= 315
; -12 bytes or less when n <= 489
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.3, max=6
; - When 31 <= n <= 50: min=6, avg=6.8, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=9
; - When 301 <= n <= 500: min=9, avg=11.9, max=13
; - When 501 <= n <= 750: min=13, avg=13.0, max=13
; - When 751 <= n <= 1000: min=13, avg=13.0, max=13
; - When 1001 <= n <= 2000: min=13, avg=13.0, max=13
; - When 2001 <= n <= 3000: min=13, avg=13.0, max=13
; - When 3001 <= n <= 5000: min=13, avg=13.0, max=13
; - When 5001 <= n <= 7500: min=13, avg=13.0, max=13
; - When 7501 <= n <= 10000: min=13, avg=13.0, max=13
; - When 10001 <= n <= 20000: min=13, avg=13.0, max=13
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
	lda $A5
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 28 || n = 290 || n = 291 || n = 306)
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
.elseif(n = 9 || n = 11 || (n > 19 && n < 24) || n = 33)
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
.elseif(n = 13)
	; 2 bytes
	brk
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
.elseif(n = 19 || n = 26 || n = 32 || n = 39)
	; 4..6 bytes
	delay_n (n-13)
	brk
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27 || n = 303)
	; 5..10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 29 || n = 30 || n = 304)
	; 6..10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif((n > 33 && n < 39) || (n > 39 && n < 290))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif((n > 291 && n < 297))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif((n > 296 && n < 303) || (n > 306 && n < 310))
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 305)
	; 11 bytes
	delay_n (n-17)
	delay_n 17
.elseif((n > 309 && n < 313))
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
.if((n > 312 && n < 316))
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 316)
	; 12 bytes
	php
	lda #0
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 317)
	; 12 bytes
	php
	lda #1
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 318)
	; 12 bytes
	php
	lda #2
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 319)
	; 12 bytes
	php
	lda #3
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 320)
	; 12 bytes
	php
	lda #4
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 321)
	; 12 bytes
	php
	lda #5
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 322)
	; 12 bytes
	php
	lda #6
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 323)
	; 12 bytes
	php
	lda #7
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 324)
	; 12 bytes
	php
	lda #8
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 325)
	; 12 bytes
	php
	lda #9
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 326)
	; 12 bytes
	php
	lda #10
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 327)
	; 12 bytes
	php
	lda #11
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 328)
	; 12 bytes
	php
	lda #12
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 329)
	; 12 bytes
	php
	lda #13
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 330)
	; 12 bytes
	php
	lda #14
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 331)
	; 12 bytes
	php
	lda #15
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.endif
.else
; 332 to 351
.if(n = 332)
	; 12 bytes
	php
	lda #16
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 333)
	; 12 bytes
	php
	lda #17
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 334)
	; 12 bytes
	php
	lda #18
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 335)
	; 12 bytes
	php
	lda #19
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 336)
	; 12 bytes
	php
	lda #20
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 337)
	; 12 bytes
	php
	lda #21
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 338)
	; 12 bytes
	php
	lda #22
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 339)
	; 12 bytes
	php
	lda #23
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 340)
	; 12 bytes
	php
	lda #24
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 341)
	; 12 bytes
	php
	lda #25
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 342)
	; 12 bytes
	php
	lda #26
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 343)
	; 12 bytes
	php
	lda #27
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 344)
	; 12 bytes
	php
	lda #28
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 345)
	; 12 bytes
	php
	lda #29
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 346)
	; 12 bytes
	php
	lda #30
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 347)
	; 12 bytes
	php
	lda #31
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 348)
	; 12 bytes
	php
	lda #32
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 349)
	; 12 bytes
	php
	lda #33
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 350)
	; 12 bytes
	php
	lda #34
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 351)
	; 12 bytes
	php
	lda #35
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.endif
.endif
.else
.if((n >= 352) && (n < 371))
; 352 to 370
.if(n = 352)
	; 12 bytes
	php
	lda #36
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 353)
	; 12 bytes
	php
	lda #37
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 354)
	; 12 bytes
	php
	lda #38
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 355)
	; 12 bytes
	php
	lda #39
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 356)
	; 12 bytes
	php
	lda #40
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 357)
	; 12 bytes
	php
	lda #41
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 358)
	; 12 bytes
	php
	lda #42
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 359)
	; 12 bytes
	php
	lda #43
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 360)
	; 12 bytes
	php
	lda #44
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 361)
	; 12 bytes
	php
	lda #45
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 362)
	; 12 bytes
	php
	lda #46
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 363)
	; 12 bytes
	php
	lda #47
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 364)
	; 12 bytes
	php
	lda #48
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 365)
	; 12 bytes
	php
	lda #49
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 366)
	; 12 bytes
	php
	lda #50
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 367)
	; 12 bytes
	php
	lda #51
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 368)
	; 12 bytes
	php
	lda #52
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 369)
	; 12 bytes
	php
	lda #53
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 370)
	; 12 bytes
	php
	lda #54
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.endif
.else
; 371 to 390
.if(n = 371)
	; 12 bytes
	php
	lda #55
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 372)
	; 12 bytes
	php
	lda #56
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 373)
	; 12 bytes
	php
	lda #57
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 374)
	; 12 bytes
	php
	lda #58
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 375)
	; 12 bytes
	php
	lda #59
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 376)
	; 12 bytes
	php
	lda #60
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 377)
	; 12 bytes
	php
	lda #61
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 378)
	; 12 bytes
	php
	lda #62
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 379)
	; 12 bytes
	php
	lda #63
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 380)
	; 12 bytes
	php
	lda #64
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 381)
	; 12 bytes
	php
	lda #65
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 382)
	; 12 bytes
	php
	lda #66
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 383)
	; 12 bytes
	php
	lda #67
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 384)
	; 12 bytes
	php
	lda #68
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 385)
	; 12 bytes
	php
	lda #69
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 386)
	; 12 bytes
	php
	lda #70
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 387)
	; 12 bytes
	php
	lda #71
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 388)
	; 12 bytes
	php
	lda #72
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 389)
	; 12 bytes
	php
	lda #73
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 390)
	; 12 bytes
	php
	lda #74
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
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
	lda #75
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 392)
	; 12 bytes
	php
	lda #76
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 393)
	; 12 bytes
	php
	lda #77
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 394)
	; 12 bytes
	php
	lda #78
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 395)
	; 12 bytes
	php
	lda #79
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 396)
	; 12 bytes
	php
	lda #80
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 397)
	; 12 bytes
	php
	lda #81
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 398)
	; 12 bytes
	php
	lda #82
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 399)
	; 12 bytes
	php
	lda #83
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 400)
	; 12 bytes
	php
	lda #84
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 401)
	; 12 bytes
	php
	lda #85
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 402)
	; 12 bytes
	php
	lda #86
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 403)
	; 12 bytes
	php
	lda #87
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 404)
	; 12 bytes
	php
	lda #88
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 405)
	; 12 bytes
	php
	lda #89
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 406)
	; 12 bytes
	php
	lda #90
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 407)
	; 12 bytes
	php
	lda #91
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 408)
	; 12 bytes
	php
	lda #92
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 409)
	; 12 bytes
	php
	lda #93
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.endif
.else
; 410 to 429
.if(n = 410)
	; 12 bytes
	php
	lda #94
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 411)
	; 12 bytes
	php
	lda #95
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 412)
	; 12 bytes
	php
	lda #96
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 413)
	; 12 bytes
	php
	lda #97
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 414)
	; 12 bytes
	php
	lda #98
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 415)
	; 12 bytes
	php
	lda #99
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 416)
	; 12 bytes
	php
	lda #100
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 417)
	; 12 bytes
	php
	lda #101
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 418)
	; 12 bytes
	php
	lda #102
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 419)
	; 12 bytes
	php
	lda #103
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 420)
	; 12 bytes
	php
	lda #104
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 421)
	; 12 bytes
	php
	lda #105
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 422)
	; 12 bytes
	php
	lda #106
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 423)
	; 12 bytes
	php
	lda #107
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 424)
	; 12 bytes
	php
	lda #108
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 425)
	; 12 bytes
	php
	lda #109
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 426)
	; 12 bytes
	php
	lda #110
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 427)
	; 12 bytes
	php
	lda #111
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 428)
	; 12 bytes
	php
	lda #112
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 429)
	; 12 bytes
	php
	lda #113
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.endif
.endif
.else
.if((n >= 430) && (n < 449))
; 430 to 448
.if(n = 430)
	; 12 bytes
	php
	lda #114
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 431)
	; 12 bytes
	php
	lda #115
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 432)
	; 12 bytes
	php
	lda #116
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 433)
	; 12 bytes
	php
	lda #117
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 434)
	; 12 bytes
	php
	lda #118
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 435)
	; 12 bytes
	php
	lda #119
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 436)
	; 12 bytes
	php
	lda #120
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 437)
	; 12 bytes
	php
	lda #121
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 438)
	; 12 bytes
	php
	lda #122
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 439)
	; 12 bytes
	php
	lda #123
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 440)
	; 12 bytes
	php
	lda #124
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 441)
	; 12 bytes
	php
	lda #125
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 442)
	; 12 bytes
	php
	lda #126
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 443)
	; 12 bytes
	php
	lda #127
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 444)
	; 12 bytes
	php
	lda #128
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 445)
	; 12 bytes
	php
	lda #129
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 446)
	; 12 bytes
	php
	lda #130
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 447)
	; 12 bytes
	php
	lda #131
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 448)
	; 12 bytes
	php
	lda #132
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.endif
.else
; 449 to 468
.if(n = 449)
	; 12 bytes
	php
	lda #133
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 450)
	; 12 bytes
	php
	lda #134
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 451)
	; 12 bytes
	php
	lda #135
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 452)
	; 12 bytes
	php
	lda #136
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 453)
	; 12 bytes
	php
	lda #137
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 454)
	; 12 bytes
	php
	lda #138
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 455)
	; 12 bytes
	php
	lda #139
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 456)
	; 12 bytes
	php
	lda #140
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 457)
	; 12 bytes
	php
	lda #141
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 458)
	; 12 bytes
	php
	lda #142
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 459)
	; 12 bytes
	php
	lda #143
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 460)
	; 12 bytes
	php
	lda #144
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 461)
	; 12 bytes
	php
	lda #145
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 462)
	; 12 bytes
	php
	lda #146
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 463)
	; 12 bytes
	php
	lda #147
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 464)
	; 12 bytes
	php
	lda #148
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 465)
	; 12 bytes
	php
	lda #149
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 466)
	; 12 bytes
	php
	lda #150
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 467)
	; 12 bytes
	php
	lda #151
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 468)
	; 12 bytes
	php
	lda #152
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.endif
.endif
.endif
.endif
.else
; 469 to 625
.if(n = 469)
	; 12 bytes
	php
	lda #153
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 470)
	; 12 bytes
	php
	lda #154
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 471)
	; 12 bytes
	php
	lda #155
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 472)
	; 12 bytes
	php
	lda #156
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 473)
	; 12 bytes
	php
	lda #157
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 474)
	; 12 bytes
	php
	lda #158
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 475)
	; 12 bytes
	php
	lda #159
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 476)
	; 12 bytes
	php
	lda #160
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 477)
	; 12 bytes
	php
	lda #161
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 478)
	; 12 bytes
	php
	lda #162
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 479)
	; 12 bytes
	php
	lda #163
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 480)
	; 12 bytes
	php
	lda #164
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 481)
	; 12 bytes
	php
	lda #165
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 482)
	; 12 bytes
	php
	lda #166
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 483)
	; 12 bytes
	php
	lda #167
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 484)
	; 12 bytes
	php
	lda #168
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 485)
	; 12 bytes
	php
	lda #169
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 486)
	; 12 bytes
	php
	lda #170
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 487)
	; 12 bytes
	php
	lda #171
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 488)
	; 12 bytes
	php
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 489)
	; 12 bytes
	php
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 490)
	; 13 bytes
	php
	nop
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif(n = 491)
	; 13 bytes
	php
	nop
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	plp
.elseif((n > 491 && n < 626))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 1251))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 2501))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 5001))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 10001))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif

.endif
	rest
.endmacro
