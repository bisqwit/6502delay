; delay_n, delays n cycles with small code
; Will clobber A, Z+N, C, and V
; Will preserve X, Y, S, D, and I (interrupt-disable flag)
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
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 17
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 284
; - 7 bytes or less when n <= 295
; - 8 bytes or less when n <= 297
; - 9 bytes or less when n <= 308
; -10 bytes or less when n <= 482
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=2, avg=3.7, max=5
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=5, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=5, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.4, max=9
; - When 301 <= n <= 500: min=9, avg=10.1, max=11
; - When 501 <= n <= 750: min=11, avg=11.0, max=11
; - When 751 <= n <= 1000: min=11, avg=11.0, max=11
; - When 1001 <= n <= 2000: min=11, avg=11.0, max=11
; - When 2001 <= n <= 3000: min=11, avg=11.0, max=11
; - When 3001 <= n <= 5000: min=11, avg=11.0, max=11
; - When 5001 <= n <= 7500: min=11, avg=11.0, max=11
; - When 7501 <= n <= 10000: min=11, avg=11.0, max=11
; - When 10001 <= n <= 20000: min=11, avg=11.0, max=11
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
.elseif((n > 3 && n < 7) || (n > 15 && n < 19) || n = 283 || n = 284 || n = 298 || n = 299)
	; 2..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || (n > 18 && n < 24))
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
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26)
	; 4 bytes
	delay_n (n-13)
	brk
.elseif((n > 26 && n < 283))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif((n > 284 && n < 290))
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n > 289 && n < 296) || (n > 299 && n < 303))
	; 7..9 bytes
	brk
	delay_n (n-13)
.elseif(n = 296 || n = 297)
	; 8 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif((n > 302 && n < 309))
	; 9 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 309)
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
.elseif(n = 483 || n = 484)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif((n > 484 && n < 626))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 1251))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 2501))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 5001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 10001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif

.endif
	rest
.endmacro
