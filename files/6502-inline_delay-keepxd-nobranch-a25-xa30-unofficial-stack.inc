; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, Y, S, Z+N, C, and V
; Will preserve X, D, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 12
; - 4 bytes or less when n <= 19
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 286
; - 7 bytes or less when n <= 290
; - 8 bytes or less when n <= 297
; - 9 bytes or less when n <= 301
; -10 bytes or less when n <= 482
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=3, avg=3.7, max=5
; - When 21 <= n <= 30: min=5, avg=5.3, max=6
; - When 31 <= n <= 50: min=5, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=5, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.5, max=9
; - When 301 <= n <= 500: min=9, avg=10.1, max=11
; - When 501 <= n <= 750: min=9, avg=11.0, max=11
; - When 751 <= n <= 1000: min=9, avg=11.0, max=11
; - When 1001 <= n <= 2000: min=9, avg=11.0, max=11
; - When 2001 <= n <= 3000: min=9, avg=11.0, max=11
; - When 3001 <= n <= 5000: min=9, avg=11.0, max=11
; - When 5001 <= n <= 7500: min=9, avg=11.0, max=11
; - When 7501 <= n <= 10000: min=9, avg=11.0, max=11
; - When 10001 <= n <= 20000: min=9, avg=11.0, max=11
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
.elseif(n = 4)
	; 1 bytes
	pla
.elseif(n = 5 || n = 16 || n = 17)
	; 2..4 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 18 || n = 19 || n = 23)
	; 4..5 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 20)
	; 5 bytes
	.repeat 5
	 pla
	.endrepeat
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26)
	; 6 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 26 && n < 157))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.endif
.else
; 157 to 312
.if((n > 156 && n < 283))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif(n = 283 || n = 284 || n = 291 || n = 298 || n = 299)
	; 6..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 285)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 286)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 287 || n = 288)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 289)
	; 7 bytes
	delay_n (n-4)
	pla
.elseif(n = 290)
	; 7 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n > 291 && n < 295))
	; 8 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 295)
	; 8 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 296 || n = 297)
	; 8 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 300)
	; 9 bytes
	txa
	pha
	lax #1
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 301)
	; 9 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n > 301 && n < 310))
	; 10 bytes
	delay_n 27
	delay_n (n-27)
.elseif(n = 310)
	; 10 bytes
	delay_n 28
	delay_n (n-28)
.elseif(n = 311)
	; 10 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 312)
	; 10 bytes
	delay_n 30
	delay_n (n-30)
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
.if((n >= 313) && (n < 332))
; 313 to 331
.if(n = 313)
	; 10 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 314)
	; 10 bytes
	delay_n 32
	delay_n (n-32)
.elseif(n = 315)
	; 10 bytes
	delay_n 33
	delay_n (n-33)
.elseif(n = 316)
	; 10 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 317)
	; 10 bytes
	delay_n 35
	delay_n (n-35)
.elseif(n = 318)
	; 10 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 319)
	; 10 bytes
	delay_n 37
	delay_n (n-37)
.elseif(n = 320)
	; 10 bytes
	delay_n 38
	delay_n (n-38)
.elseif(n = 321)
	; 10 bytes
	delay_n 39
	delay_n (n-39)
.elseif(n = 322)
	; 10 bytes
	delay_n 40
	delay_n (n-40)
.elseif(n = 323)
	; 10 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 324)
	; 10 bytes
	delay_n 42
	delay_n (n-42)
.elseif(n = 325)
	; 10 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 326)
	; 10 bytes
	delay_n 44
	delay_n (n-44)
.elseif(n = 327)
	; 10 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 328)
	; 10 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 329)
	; 10 bytes
	delay_n 47
	delay_n (n-47)
.elseif(n = 330)
	; 10 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 331)
	; 10 bytes
	delay_n 49
	delay_n (n-49)
.endif
.else
; 332 to 351
.if(n = 332)
	; 10 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 333)
	; 10 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 334)
	; 10 bytes
	delay_n 52
	delay_n (n-52)
.elseif(n = 335)
	; 10 bytes
	delay_n 53
	delay_n (n-53)
.elseif(n = 336)
	; 10 bytes
	delay_n 54
	delay_n (n-54)
.elseif(n = 337)
	; 10 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 338)
	; 10 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 339)
	; 10 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 340)
	; 10 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 341)
	; 10 bytes
	delay_n 59
	delay_n (n-59)
.elseif(n = 342)
	; 10 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 343)
	; 10 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 344)
	; 10 bytes
	delay_n 62
	delay_n (n-62)
.elseif(n = 345)
	; 10 bytes
	delay_n 63
	delay_n (n-63)
.elseif(n = 346)
	; 10 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 347)
	; 10 bytes
	delay_n 65
	delay_n (n-65)
.elseif(n = 348)
	; 10 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 349)
	; 10 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 350)
	; 10 bytes
	delay_n 68
	delay_n (n-68)
.elseif(n = 351)
	; 10 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.else
.if((n >= 352) && (n < 371))
; 352 to 370
.if(n = 352)
	; 10 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 353)
	; 10 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 354)
	; 10 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 355)
	; 10 bytes
	delay_n 73
	delay_n (n-73)
.elseif(n = 356)
	; 10 bytes
	delay_n 74
	delay_n (n-74)
.elseif(n = 357)
	; 10 bytes
	delay_n 75
	delay_n (n-75)
.elseif(n = 358)
	; 10 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 359)
	; 10 bytes
	delay_n 77
	delay_n (n-77)
.elseif(n = 360)
	; 10 bytes
	delay_n 78
	delay_n (n-78)
.elseif(n = 361)
	; 10 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 362)
	; 10 bytes
	delay_n 80
	delay_n (n-80)
.elseif(n = 363)
	; 10 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 364)
	; 10 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 365)
	; 10 bytes
	delay_n 83
	delay_n (n-83)
.elseif(n = 366)
	; 10 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 367)
	; 10 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 368)
	; 10 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 369)
	; 10 bytes
	delay_n 87
	delay_n (n-87)
.elseif(n = 370)
	; 10 bytes
	delay_n 88
	delay_n (n-88)
.endif
.else
; 371 to 390
.if(n = 371)
	; 10 bytes
	delay_n 89
	delay_n (n-89)
.elseif(n = 372)
	; 10 bytes
	delay_n 90
	delay_n (n-90)
.elseif(n = 373)
	; 10 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 374)
	; 10 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 375)
	; 10 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 376)
	; 10 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 377)
	; 10 bytes
	delay_n 95
	delay_n (n-95)
.elseif(n = 378)
	; 10 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 379)
	; 10 bytes
	delay_n 97
	delay_n (n-97)
.elseif(n = 380)
	; 10 bytes
	delay_n 98
	delay_n (n-98)
.elseif(n = 381)
	; 10 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 382)
	; 10 bytes
	delay_n 100
	delay_n (n-100)
.elseif(n = 383)
	; 10 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 384)
	; 10 bytes
	delay_n 102
	delay_n (n-102)
.elseif(n = 385)
	; 10 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 386)
	; 10 bytes
	delay_n 104
	delay_n (n-104)
.elseif(n = 387)
	; 10 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 388)
	; 10 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 389)
	; 10 bytes
	delay_n 107
	delay_n (n-107)
.elseif(n = 390)
	; 10 bytes
	delay_n 108
	delay_n (n-108)
.endif
.endif
.endif
.else
.if((n >= 391) && (n < 430))
.if((n >= 391) && (n < 410))
; 391 to 409
.if(n = 391)
	; 10 bytes
	delay_n 109
	delay_n (n-109)
.elseif(n = 392)
	; 10 bytes
	delay_n 110
	delay_n (n-110)
.elseif(n = 393)
	; 10 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 394)
	; 10 bytes
	delay_n 112
	delay_n (n-112)
.elseif(n = 395)
	; 10 bytes
	delay_n 113
	delay_n (n-113)
.elseif(n = 396)
	; 10 bytes
	delay_n 114
	delay_n (n-114)
.elseif(n = 397)
	; 10 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 398)
	; 10 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 399)
	; 10 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 400)
	; 10 bytes
	delay_n 118
	delay_n (n-118)
.elseif(n = 401)
	; 10 bytes
	delay_n 119
	delay_n (n-119)
.elseif(n = 402)
	; 10 bytes
	delay_n 120
	delay_n (n-120)
.elseif(n = 403)
	; 10 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 404)
	; 10 bytes
	delay_n 122
	delay_n (n-122)
.elseif(n = 405)
	; 10 bytes
	delay_n 123
	delay_n (n-123)
.elseif(n = 406)
	; 10 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 407)
	; 10 bytes
	delay_n 125
	delay_n (n-125)
.elseif(n = 408)
	; 10 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 409)
	; 10 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 410 to 429
.if(n = 410)
	; 10 bytes
	delay_n 128
	delay_n (n-128)
.elseif(n = 411)
	; 10 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 412)
	; 10 bytes
	delay_n 130
	delay_n (n-130)
.elseif(n = 413)
	; 10 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 414)
	; 10 bytes
	delay_n 132
	delay_n (n-132)
.elseif(n = 415)
	; 10 bytes
	delay_n 133
	delay_n (n-133)
.elseif(n = 416)
	; 10 bytes
	delay_n 134
	delay_n (n-134)
.elseif(n = 417)
	; 10 bytes
	delay_n 135
	delay_n (n-135)
.elseif(n = 418)
	; 10 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 419)
	; 10 bytes
	delay_n 137
	delay_n (n-137)
.elseif(n = 420)
	; 10 bytes
	delay_n 138
	delay_n (n-138)
.elseif(n = 421)
	; 10 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 422)
	; 10 bytes
	delay_n 140
	delay_n (n-140)
.elseif(n = 423)
	; 10 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 424)
	; 10 bytes
	delay_n 142
	delay_n (n-142)
.elseif(n = 425)
	; 10 bytes
	delay_n 143
	delay_n (n-143)
.elseif(n = 426)
	; 10 bytes
	delay_n 144
	delay_n (n-144)
.elseif(n = 427)
	; 10 bytes
	delay_n 145
	delay_n (n-145)
.elseif(n = 428)
	; 10 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 429)
	; 10 bytes
	delay_n 147
	delay_n (n-147)
.endif
.endif
.else
.if((n >= 430) && (n < 449))
; 430 to 448
.if(n = 430)
	; 10 bytes
	delay_n 148
	delay_n (n-148)
.elseif(n = 431)
	; 10 bytes
	delay_n 149
	delay_n (n-149)
.elseif(n = 432)
	; 10 bytes
	delay_n 150
	delay_n (n-150)
.elseif(n = 433)
	; 10 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 434)
	; 10 bytes
	delay_n 152
	delay_n (n-152)
.elseif(n = 435)
	; 10 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 436)
	; 10 bytes
	delay_n 154
	delay_n (n-154)
.elseif(n = 437)
	; 10 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 438)
	; 10 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 439)
	; 10 bytes
	delay_n 157
	delay_n (n-157)
.elseif(n = 440)
	; 10 bytes
	delay_n 158
	delay_n (n-158)
.elseif(n = 441)
	; 10 bytes
	delay_n 159
	delay_n (n-159)
.elseif(n = 442)
	; 10 bytes
	delay_n 160
	delay_n (n-160)
.elseif(n = 443)
	; 10 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 444)
	; 10 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 445)
	; 10 bytes
	delay_n 163
	delay_n (n-163)
.elseif(n = 446)
	; 10 bytes
	delay_n 164
	delay_n (n-164)
.elseif(n = 447)
	; 10 bytes
	delay_n 165
	delay_n (n-165)
.elseif(n = 448)
	; 10 bytes
	delay_n 166
	delay_n (n-166)
.endif
.else
; 449 to 468
.if(n = 449)
	; 10 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 450)
	; 10 bytes
	delay_n 168
	delay_n (n-168)
.elseif(n = 451)
	; 10 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 452)
	; 10 bytes
	delay_n 170
	delay_n (n-170)
.elseif(n = 453)
	; 10 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 454)
	; 10 bytes
	delay_n 172
	delay_n (n-172)
.elseif(n = 455)
	; 10 bytes
	delay_n 173
	delay_n (n-173)
.elseif(n = 456)
	; 10 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 457)
	; 10 bytes
	delay_n 175
	delay_n (n-175)
.elseif(n = 458)
	; 10 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 459)
	; 10 bytes
	delay_n 177
	delay_n (n-177)
.elseif(n = 460)
	; 10 bytes
	delay_n 178
	delay_n (n-178)
.elseif(n = 461)
	; 10 bytes
	delay_n 179
	delay_n (n-179)
.elseif(n = 462)
	; 10 bytes
	delay_n 180
	delay_n (n-180)
.elseif(n = 463)
	; 10 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 464)
	; 10 bytes
	delay_n 182
	delay_n (n-182)
.elseif(n = 465)
	; 10 bytes
	delay_n 183
	delay_n (n-183)
.elseif(n = 466)
	; 10 bytes
	delay_n 184
	delay_n (n-184)
.elseif(n = 467)
	; 10 bytes
	delay_n 185
	delay_n (n-185)
.elseif(n = 468)
	; 10 bytes
	delay_n 186
	delay_n (n-186)
.endif
.endif
.endif
.endif
.else
; 469 to 625
.if(n = 469)
	; 10 bytes
	delay_n 187
	delay_n (n-187)
.elseif(n = 470)
	; 10 bytes
	delay_n 188
	delay_n (n-188)
.elseif(n = 471)
	; 10 bytes
	delay_n 189
	delay_n (n-189)
.elseif(n = 472)
	; 10 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 473)
	; 10 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 474)
	; 10 bytes
	delay_n 192
	delay_n (n-192)
.elseif(n = 475)
	; 10 bytes
	delay_n 193
	delay_n (n-193)
.elseif(n = 476)
	; 10 bytes
	delay_n 194
	delay_n (n-194)
.elseif(n = 477)
	; 10 bytes
	delay_n 195
	delay_n (n-195)
.elseif(n = 478)
	; 10 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 479)
	; 10 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 480)
	; 10 bytes
	delay_n 198
	delay_n (n-198)
.elseif(n = 481)
	; 10 bytes
	delay_n 199
	delay_n (n-199)
.elseif(n = 482)
	; 10 bytes
	delay_n 200
	delay_n (n-200)
.elseif(n = 483 || n = 484 || n = 556 || n = 559)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n > 484 && n < 554) || n = 555 || (n > 561 && n < 626))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 554)
	; 9 bytes
	txa
	tay
	lax #2
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 557)
	; 9 bytes
	txa
	pha
	lax #2
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 558 || n = 561)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 560)
	; 10 bytes
	pha
	delay_n (n-3)
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 811) || n = 812 || (n > 818 && n < 1068) || n = 1069 || (n > 1075 && n < 1251))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 811)
	; 9 bytes
	txa
	tay
	lax #3
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 813 || n = 816 || n = 1070 || n = 1073)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 814)
	; 9 bytes
	txa
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 815 || n = 818 || n = 1072 || n = 1075)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 817 || n = 1074)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1068)
	; 9 bytes
	txa
	tay
	lax #4
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 1071)
	; 9 bytes
	txa
	pha
	lax #4
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1325) || n = 1326 || (n > 1332 && n < 1582) || n = 1583 || (n > 1589 && n < 1839) || n = 1840 || (n > 1846 && n < 2096) || n = 2097 || (n > 2103 && n < 2353) || n = 2354 || (n > 2360 && n < 2501))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1325)
	; 9 bytes
	txa
	tay
	lax #5
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 1327 || n = 1330 || n = 1584 || n = 1587 || n = 1841 || n = 1844 || n = 2098 || n = 2101 || n = 2355 || n = 2358)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1328)
	; 9 bytes
	txa
	pha
	lax #5
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1329 || n = 1332 || n = 1586 || n = 1589 || n = 1843 || n = 1846 || n = 2100 || n = 2103 || n = 2357 || n = 2360)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1331 || n = 1588 || n = 1845 || n = 2102 || n = 2359)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1582)
	; 9 bytes
	txa
	tay
	lax #6
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 1585)
	; 9 bytes
	txa
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1839)
	; 9 bytes
	txa
	tay
	lax #7
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 1842)
	; 9 bytes
	txa
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2096)
	; 9 bytes
	txa
	tay
	lax #8
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 2099)
	; 9 bytes
	txa
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2353)
	; 9 bytes
	txa
	tay
	lax #9
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 2356)
	; 9 bytes
	txa
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2610) || n = 2611 || (n > 2617 && n < 2867) || n = 2868 || (n > 2874 && n < 3124) || n = 3125 || (n > 3131 && n < 3381) || n = 3382 || (n > 3388 && n < 3638) || n = 3639 || (n > 3645 && n < 3895) || n = 3896 || (n > 3902 && n < 4152) || n = 4153 || (n > 4159 && n < 4409) || n = 4410 || (n > 4416 && n < 4666) || n = 4667 || (n > 4673 && n < 4923) || n = 4924 || (n > 4930 && n < 5001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2610)
	; 9 bytes
	txa
	tay
	lax #10
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 2612 || n = 2615 || n = 2869 || n = 2872 || n = 3126 || n = 3129 || n = 3383 || n = 3386 || n = 3640 || n = 3643 || n = 3897 || n = 3900 || n = 4154 || n = 4157 || n = 4411 || n = 4414 || n = 4668 || n = 4671 || n = 4925 || n = 4928)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2613)
	; 9 bytes
	txa
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2614 || n = 2617 || n = 2871 || n = 2874 || n = 3128 || n = 3131 || n = 3385 || n = 3388 || n = 3642 || n = 3645 || n = 3899 || n = 3902 || n = 4156 || n = 4159 || n = 4413 || n = 4416 || n = 4670 || n = 4673 || n = 4927 || n = 4930)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2616 || n = 2873 || n = 3130 || n = 3387 || n = 3644 || n = 3901 || n = 4158 || n = 4415 || n = 4672 || n = 4929)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2867)
	; 9 bytes
	txa
	tay
	lax #11
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 2870)
	; 9 bytes
	txa
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3124)
	; 9 bytes
	txa
	tay
	lax #12
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 3127)
	; 9 bytes
	txa
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3381)
	; 9 bytes
	txa
	tay
	lax #13
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 3384)
	; 9 bytes
	txa
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3638)
	; 9 bytes
	txa
	tay
	lax #14
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 3641)
	; 9 bytes
	txa
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3895)
	; 9 bytes
	txa
	tay
	lax #15
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 3898)
	; 9 bytes
	txa
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 4152)
	; 9 bytes
	txa
	tay
	lax #16
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 4155)
	; 9 bytes
	txa
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 4409)
	; 9 bytes
	txa
	tay
	lax #17
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 4412)
	; 9 bytes
	txa
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 4666)
	; 9 bytes
	txa
	tay
	lax #18
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 4669)
	; 9 bytes
	txa
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 4923)
	; 9 bytes
	txa
	tay
	lax #19
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 4926)
	; 9 bytes
	txa
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
; 5001 to 7500
.if((n > 5000 && n < 5180) || n = 5181 || (n > 5187 && n < 5437) || n = 5438 || (n > 5444 && n < 5694) || n = 5695 || (n > 5701 && n < 5951) || n = 5952 || (n > 5958 && n < 6208) || n = 6209 || (n > 6215 && n < 6465) || n = 6466 || (n > 6472 && n < 6722) || n = 6723 || (n > 6729 && n < 6979) || n = 6980 || (n > 6986 && n < 7236) || n = 7237 || (n > 7243 && n < 7493) || n = 7494)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 5180)
	; 9 bytes
	txa
	tay
	lax #20
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 5182 || n = 5185 || n = 5439 || n = 5442 || n = 5696 || n = 5699 || n = 5953 || n = 5956 || n = 6210 || n = 6213 || n = 6467 || n = 6470 || n = 6724 || n = 6727 || n = 6981 || n = 6984 || n = 7238 || n = 7241 || n = 7495 || n = 7498)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5183)
	; 9 bytes
	txa
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 5184 || n = 5187 || n = 5441 || n = 5444 || n = 5698 || n = 5701 || n = 5955 || n = 5958 || n = 6212 || n = 6215 || n = 6469 || n = 6472 || n = 6726 || n = 6729 || n = 6983 || n = 6986 || n = 7240 || n = 7243 || n = 7497 || n = 7500)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 5186 || n = 5443 || n = 5700 || n = 5957 || n = 6214 || n = 6471 || n = 6728 || n = 6985 || n = 7242 || n = 7499)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5437)
	; 9 bytes
	txa
	tay
	lax #21
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 5440)
	; 9 bytes
	txa
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 5694)
	; 9 bytes
	txa
	tay
	lax #22
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 5697)
	; 9 bytes
	txa
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 5951)
	; 9 bytes
	txa
	tay
	lax #23
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 5954)
	; 9 bytes
	txa
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 6208)
	; 9 bytes
	txa
	tay
	lax #24
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 6211)
	; 9 bytes
	txa
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 6465)
	; 9 bytes
	txa
	tay
	lax #25
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 6468)
	; 9 bytes
	txa
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 6722)
	; 9 bytes
	txa
	tay
	lax #26
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 6725)
	; 9 bytes
	txa
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 6979)
	; 9 bytes
	txa
	tay
	lax #27
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 6982)
	; 9 bytes
	txa
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 7236)
	; 9 bytes
	txa
	tay
	lax #28
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 7239)
	; 9 bytes
	txa
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 7493)
	; 9 bytes
	txa
	tay
	lax #29
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 7496)
	; 9 bytes
	txa
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7750) || n = 7751 || (n > 7757 && n < 8007) || n = 8008 || (n > 8014 && n < 8264) || n = 8265 || (n > 8271 && n < 8521) || n = 8522 || (n > 8528 && n < 8778) || n = 8779 || (n > 8785 && n < 9035) || n = 9036 || (n > 9042 && n < 9292) || n = 9293 || (n > 9299 && n < 9549) || n = 9550 || (n > 9556 && n < 9806) || n = 9807 || (n > 9813 && n < 10001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 7750)
	; 9 bytes
	txa
	tay
	lax #30
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 7752 || n = 7755 || n = 8009 || n = 8012 || n = 8266 || n = 8269 || n = 8523 || n = 8526 || n = 8780 || n = 8783 || n = 9037 || n = 9040 || n = 9294 || n = 9297 || n = 9551 || n = 9554 || n = 9808 || n = 9811)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7753)
	; 9 bytes
	txa
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 7754 || n = 7757 || n = 8011 || n = 8014 || n = 8268 || n = 8271 || n = 8525 || n = 8528 || n = 8782 || n = 8785 || n = 9039 || n = 9042 || n = 9296 || n = 9299 || n = 9553 || n = 9556 || n = 9810 || n = 9813)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 7756 || n = 8013 || n = 8270 || n = 8527 || n = 8784 || n = 9041 || n = 9298 || n = 9555 || n = 9812)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8007)
	; 9 bytes
	txa
	tay
	lax #31
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 8010)
	; 9 bytes
	txa
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 8264)
	; 9 bytes
	txa
	tay
	lax #32
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 8267)
	; 9 bytes
	txa
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 8521)
	; 9 bytes
	txa
	tay
	lax #33
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 8524)
	; 9 bytes
	txa
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 8778)
	; 9 bytes
	txa
	tay
	lax #34
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 8781)
	; 9 bytes
	txa
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 9035)
	; 9 bytes
	txa
	tay
	lax #35
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 9038)
	; 9 bytes
	txa
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 9292)
	; 9 bytes
	txa
	tay
	lax #36
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 9295)
	; 9 bytes
	txa
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 9549)
	; 9 bytes
	txa
	tay
	lax #37
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 9552)
	; 9 bytes
	txa
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 9806)
	; 9 bytes
	txa
	tay
	lax #38
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 9809)
	; 9 bytes
	txa
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
; 10001 to 12500
.if((n > 10000 && n < 10063) || n = 10064 || (n > 10070 && n < 10320) || n = 10321 || (n > 10327 && n < 10577) || n = 10578 || (n > 10584 && n < 10834) || n = 10835 || (n > 10841 && n < 11091) || n = 11092 || (n > 11098 && n < 11348) || n = 11349 || (n > 11355 && n < 11605) || n = 11606 || (n > 11612 && n < 11862) || n = 11863 || (n > 11869 && n < 12119) || n = 12120 || (n > 12126 && n < 12376) || n = 12377 || (n > 12383 && n < 12501))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 10063)
	; 9 bytes
	txa
	tay
	lax #39
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 10065 || n = 10068 || n = 10322 || n = 10325 || n = 10579 || n = 10582 || n = 10836 || n = 10839 || n = 11093 || n = 11096 || n = 11350 || n = 11353 || n = 11607 || n = 11610 || n = 11864 || n = 11867 || n = 12121 || n = 12124 || n = 12378 || n = 12381)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10066)
	; 9 bytes
	txa
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 10067 || n = 10070 || n = 10324 || n = 10327 || n = 10581 || n = 10584 || n = 10838 || n = 10841 || n = 11095 || n = 11098 || n = 11352 || n = 11355 || n = 11609 || n = 11612 || n = 11866 || n = 11869 || n = 12123 || n = 12126 || n = 12380 || n = 12383)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 10069 || n = 10326 || n = 10583 || n = 10840 || n = 11097 || n = 11354 || n = 11611 || n = 11868 || n = 12125 || n = 12382)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10320)
	; 9 bytes
	txa
	tay
	lax #40
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 10323)
	; 9 bytes
	txa
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 10577)
	; 9 bytes
	txa
	tay
	lax #41
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 10580)
	; 9 bytes
	txa
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 10834)
	; 9 bytes
	txa
	tay
	lax #42
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 10837)
	; 9 bytes
	txa
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 11091)
	; 9 bytes
	txa
	tay
	lax #43
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 11094)
	; 9 bytes
	txa
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 11348)
	; 9 bytes
	txa
	tay
	lax #44
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 11351)
	; 9 bytes
	txa
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 11605)
	; 9 bytes
	txa
	tay
	lax #45
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 11608)
	; 9 bytes
	txa
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 11862)
	; 9 bytes
	txa
	tay
	lax #46
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 11865)
	; 9 bytes
	txa
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 12119)
	; 9 bytes
	txa
	tay
	lax #47
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 12122)
	; 9 bytes
	txa
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 12376)
	; 9 bytes
	txa
	tay
	lax #48
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 12379)
	; 9 bytes
	txa
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.else
; 12501 to 15000
.if((n > 12500 && n < 12633) || n = 12634 || (n > 12640 && n < 12890) || n = 12891 || (n > 12897 && n < 13147) || n = 13148 || (n > 13154 && n < 13404) || n = 13405 || (n > 13411 && n < 13661) || n = 13662 || (n > 13668 && n < 13918) || n = 13919 || (n > 13925 && n < 14175) || n = 14176 || (n > 14182 && n < 14432) || n = 14433 || (n > 14439 && n < 14689) || n = 14690 || (n > 14696 && n < 14946) || n = 14947 || (n > 14953 && n < 15001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 12633)
	; 9 bytes
	txa
	tay
	lax #49
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 12635 || n = 12638 || n = 12892 || n = 12895 || n = 13149 || n = 13152 || n = 13406 || n = 13409 || n = 13663 || n = 13666 || n = 13920 || n = 13923 || n = 14177 || n = 14180 || n = 14434 || n = 14437 || n = 14691 || n = 14694 || n = 14948 || n = 14951)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12636)
	; 9 bytes
	txa
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 12637 || n = 12640 || n = 12894 || n = 12897 || n = 13151 || n = 13154 || n = 13408 || n = 13411 || n = 13665 || n = 13668 || n = 13922 || n = 13925 || n = 14179 || n = 14182 || n = 14436 || n = 14439 || n = 14693 || n = 14696 || n = 14950 || n = 14953)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 12639 || n = 12896 || n = 13153 || n = 13410 || n = 13667 || n = 13924 || n = 14181 || n = 14438 || n = 14695 || n = 14952)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12890)
	; 9 bytes
	txa
	tay
	lax #50
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 12893)
	; 9 bytes
	txa
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 13147)
	; 9 bytes
	txa
	tay
	lax #51
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 13150)
	; 9 bytes
	txa
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 13404)
	; 9 bytes
	txa
	tay
	lax #52
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 13407)
	; 9 bytes
	txa
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 13661)
	; 9 bytes
	txa
	tay
	lax #53
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 13664)
	; 9 bytes
	txa
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 13918)
	; 9 bytes
	txa
	tay
	lax #54
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 13921)
	; 9 bytes
	txa
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 14175)
	; 9 bytes
	txa
	tay
	lax #55
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 14178)
	; 9 bytes
	txa
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 14432)
	; 9 bytes
	txa
	tay
	lax #56
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 14435)
	; 9 bytes
	txa
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 14689)
	; 9 bytes
	txa
	tay
	lax #57
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 14692)
	; 9 bytes
	txa
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 14946)
	; 9 bytes
	txa
	tay
	lax #58
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 14949)
	; 9 bytes
	txa
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
; 15001 to 17500
.if((n > 15000 && n < 15203) || n = 15204 || (n > 15210 && n < 15460) || n = 15461 || (n > 15467 && n < 15717) || n = 15718 || (n > 15724 && n < 15974) || n = 15975 || (n > 15981 && n < 16231) || n = 16232 || (n > 16238 && n < 16488) || n = 16489 || (n > 16495 && n < 16745) || n = 16746 || (n > 16752 && n < 17002) || n = 17003 || (n > 17009 && n < 17259) || n = 17260 || (n > 17266 && n < 17501))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 15203)
	; 9 bytes
	txa
	tay
	lax #59
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 15205 || n = 15208 || n = 15462 || n = 15465 || n = 15719 || n = 15722 || n = 15976 || n = 15979 || n = 16233 || n = 16236 || n = 16490 || n = 16493 || n = 16747 || n = 16750 || n = 17004 || n = 17007 || n = 17261 || n = 17264)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15206)
	; 9 bytes
	txa
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 15207 || n = 15210 || n = 15464 || n = 15467 || n = 15721 || n = 15724 || n = 15978 || n = 15981 || n = 16235 || n = 16238 || n = 16492 || n = 16495 || n = 16749 || n = 16752 || n = 17006 || n = 17009 || n = 17263 || n = 17266)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 15209 || n = 15466 || n = 15723 || n = 15980 || n = 16237 || n = 16494 || n = 16751 || n = 17008 || n = 17265)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15460)
	; 9 bytes
	txa
	tay
	lax #60
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 15463)
	; 9 bytes
	txa
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 15717)
	; 9 bytes
	txa
	tay
	lax #61
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 15720)
	; 9 bytes
	txa
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 15974)
	; 9 bytes
	txa
	tay
	lax #62
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 15977)
	; 9 bytes
	txa
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 16231)
	; 9 bytes
	txa
	tay
	lax #63
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 16234)
	; 9 bytes
	txa
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 16488)
	; 9 bytes
	txa
	tay
	lax #64
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 16491)
	; 9 bytes
	txa
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 16745)
	; 9 bytes
	txa
	tay
	lax #65
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 16748)
	; 9 bytes
	txa
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 17002)
	; 9 bytes
	txa
	tay
	lax #66
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 17005)
	; 9 bytes
	txa
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 17259)
	; 9 bytes
	txa
	tay
	lax #67
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 17262)
	; 9 bytes
	txa
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.else
; 17501 to 20000
.if((n > 17500 && n < 17516) || n = 17517 || (n > 17523 && n < 17773) || n = 17774 || (n > 17780 && n < 18030) || n = 18031 || (n > 18037 && n < 18287) || n = 18288 || (n > 18294 && n < 18544) || n = 18545 || (n > 18551 && n < 18801) || n = 18802 || (n > 18808 && n < 19058) || n = 19059 || (n > 19065 && n < 19315) || n = 19316 || (n > 19322 && n < 19572) || n = 19573 || (n > 19579 && n < 19829) || n = 19830 || (n > 19836 && n < 20001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 17516)
	; 9 bytes
	txa
	tay
	lax #68
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 17518 || n = 17521 || n = 17775 || n = 17778 || n = 18032 || n = 18035 || n = 18289 || n = 18292 || n = 18546 || n = 18549 || n = 18803 || n = 18806 || n = 19060 || n = 19063 || n = 19317 || n = 19320 || n = 19574 || n = 19577 || n = 19831 || n = 19834)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17519)
	; 9 bytes
	txa
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 17520 || n = 17523 || n = 17777 || n = 17780 || n = 18034 || n = 18037 || n = 18291 || n = 18294 || n = 18548 || n = 18551 || n = 18805 || n = 18808 || n = 19062 || n = 19065 || n = 19319 || n = 19322 || n = 19576 || n = 19579 || n = 19833 || n = 19836)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 17522 || n = 17779 || n = 18036 || n = 18293 || n = 18550 || n = 18807 || n = 19064 || n = 19321 || n = 19578 || n = 19835)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 17773)
	; 9 bytes
	txa
	tay
	lax #69
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 17776)
	; 9 bytes
	txa
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 18030)
	; 9 bytes
	txa
	tay
	lax #70
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 18033)
	; 9 bytes
	txa
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 18287)
	; 9 bytes
	txa
	tay
	lax #71
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 18290)
	; 9 bytes
	txa
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 18544)
	; 9 bytes
	txa
	tay
	lax #72
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 18547)
	; 9 bytes
	txa
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 18801)
	; 9 bytes
	txa
	tay
	lax #73
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 18804)
	; 9 bytes
	txa
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 19058)
	; 9 bytes
	txa
	tay
	lax #74
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 19061)
	; 9 bytes
	txa
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 19315)
	; 9 bytes
	txa
	tay
	lax #75
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 19318)
	; 9 bytes
	txa
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 19572)
	; 9 bytes
	txa
	tay
	lax #76
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 19575)
	; 9 bytes
	txa
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 19829)
	; 9 bytes
	txa
	tay
	lax #77
	jsr delay_256x_a_30_clocks
	tya
	tax
.elseif(n = 19832)
	; 9 bytes
	txa
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
