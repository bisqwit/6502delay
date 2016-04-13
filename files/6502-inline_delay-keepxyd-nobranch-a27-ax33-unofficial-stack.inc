; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, S, Z+N, C, and V
; Will preserve X, Y, D, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 12
; - 4 bytes or less when n <= 19
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 288
; - 7 bytes or less when n <= 292
; - 8 bytes or less when n <= 299
; - 9 bytes or less when n <= 303
; -10 bytes or less when n <= 484
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=3, avg=3.7, max=5
; - When 21 <= n <= 30: min=5, avg=5.5, max=6
; - When 31 <= n <= 50: min=5, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=5, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.4, max=9
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
; 1 to 312
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
.elseif(n = 5 || n = 16 || n = 17 || n = 285 || n = 286 || n = 293 || n = 300 || n = 301)
	; 2..9 bytes
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
.elseif(n = 8 || n = 291)
	; 2..7 bytes
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
.elseif(n = 18 || n = 19 || n = 23 || n = 27 || n = 299)
	; 4..8 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 20)
	; 5 bytes
	.repeat 5
	 pla
	.endrepeat
.elseif(n = 24 || (n > 293 && n < 297))
	; 6..8 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 28 || n = 297 || n = 298)
	; 6..8 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 28 && n < 285))
	; 5 bytes
	lda #n-29
	jsr delay_a_27_clocks
.elseif(n = 287)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 288)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 289 || n = 290)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 292)
	; 7 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 302)
	; 9 bytes
	pla
	jsr @nearby_rts_14cyc
	delay_n (n-18)
.elseif(n = 303)
	; 9 bytes
	txa
	pha
	lax #1
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n > 303 && n < 313))
	; 10 bytes
	delay_n 29
	delay_n (n-29)
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
.if((n >= 313) && (n < 332))
; 313 to 331
.if(n = 313)
	; 10 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 314)
	; 10 bytes
	delay_n 30
	delay_n (n-30)
.elseif(n = 315)
	; 10 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 316)
	; 10 bytes
	delay_n 32
	delay_n (n-32)
.elseif(n = 317)
	; 10 bytes
	delay_n 33
	delay_n (n-33)
.elseif(n = 318)
	; 10 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 319)
	; 10 bytes
	delay_n 35
	delay_n (n-35)
.elseif(n = 320)
	; 10 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 321)
	; 10 bytes
	delay_n 37
	delay_n (n-37)
.elseif(n = 322)
	; 10 bytes
	delay_n 38
	delay_n (n-38)
.elseif(n = 323)
	; 10 bytes
	delay_n 39
	delay_n (n-39)
.elseif(n = 324)
	; 10 bytes
	delay_n 40
	delay_n (n-40)
.elseif(n = 325)
	; 10 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 326)
	; 10 bytes
	delay_n 42
	delay_n (n-42)
.elseif(n = 327)
	; 10 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 328)
	; 10 bytes
	delay_n 44
	delay_n (n-44)
.elseif(n = 329)
	; 10 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 330)
	; 10 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 331)
	; 10 bytes
	delay_n 47
	delay_n (n-47)
.endif
.else
; 332 to 351
.if(n = 332)
	; 10 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 333)
	; 10 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 334)
	; 10 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 335)
	; 10 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 336)
	; 10 bytes
	delay_n 52
	delay_n (n-52)
.elseif(n = 337)
	; 10 bytes
	delay_n 53
	delay_n (n-53)
.elseif(n = 338)
	; 10 bytes
	delay_n 54
	delay_n (n-54)
.elseif(n = 339)
	; 10 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 340)
	; 10 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 341)
	; 10 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 342)
	; 10 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 343)
	; 10 bytes
	delay_n 59
	delay_n (n-59)
.elseif(n = 344)
	; 10 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 345)
	; 10 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 346)
	; 10 bytes
	delay_n 62
	delay_n (n-62)
.elseif(n = 347)
	; 10 bytes
	delay_n 63
	delay_n (n-63)
.elseif(n = 348)
	; 10 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 349)
	; 10 bytes
	delay_n 65
	delay_n (n-65)
.elseif(n = 350)
	; 10 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 351)
	; 10 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 352) && (n < 371))
; 352 to 370
.if(n = 352)
	; 10 bytes
	delay_n 68
	delay_n (n-68)
.elseif(n = 353)
	; 10 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 354)
	; 10 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 355)
	; 10 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 356)
	; 10 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 357)
	; 10 bytes
	delay_n 73
	delay_n (n-73)
.elseif(n = 358)
	; 10 bytes
	delay_n 74
	delay_n (n-74)
.elseif(n = 359)
	; 10 bytes
	delay_n 75
	delay_n (n-75)
.elseif(n = 360)
	; 10 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 361)
	; 10 bytes
	delay_n 77
	delay_n (n-77)
.elseif(n = 362)
	; 10 bytes
	delay_n 78
	delay_n (n-78)
.elseif(n = 363)
	; 10 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 364)
	; 10 bytes
	delay_n 80
	delay_n (n-80)
.elseif(n = 365)
	; 10 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 366)
	; 10 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 367)
	; 10 bytes
	delay_n 83
	delay_n (n-83)
.elseif(n = 368)
	; 10 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 369)
	; 10 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 370)
	; 10 bytes
	delay_n 86
	delay_n (n-86)
.endif
.else
; 371 to 390
.if(n = 371)
	; 10 bytes
	delay_n 87
	delay_n (n-87)
.elseif(n = 372)
	; 10 bytes
	delay_n 88
	delay_n (n-88)
.elseif(n = 373)
	; 10 bytes
	delay_n 89
	delay_n (n-89)
.elseif(n = 374)
	; 10 bytes
	delay_n 90
	delay_n (n-90)
.elseif(n = 375)
	; 10 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 376)
	; 10 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 377)
	; 10 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 378)
	; 10 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 379)
	; 10 bytes
	delay_n 95
	delay_n (n-95)
.elseif(n = 380)
	; 10 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 381)
	; 10 bytes
	delay_n 97
	delay_n (n-97)
.elseif(n = 382)
	; 10 bytes
	delay_n 98
	delay_n (n-98)
.elseif(n = 383)
	; 10 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 384)
	; 10 bytes
	delay_n 100
	delay_n (n-100)
.elseif(n = 385)
	; 10 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 386)
	; 10 bytes
	delay_n 102
	delay_n (n-102)
.elseif(n = 387)
	; 10 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 388)
	; 10 bytes
	delay_n 104
	delay_n (n-104)
.elseif(n = 389)
	; 10 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 390)
	; 10 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 391) && (n < 430))
.if((n >= 391) && (n < 410))
; 391 to 409
.if(n = 391)
	; 10 bytes
	delay_n 107
	delay_n (n-107)
.elseif(n = 392)
	; 10 bytes
	delay_n 108
	delay_n (n-108)
.elseif(n = 393)
	; 10 bytes
	delay_n 109
	delay_n (n-109)
.elseif(n = 394)
	; 10 bytes
	delay_n 110
	delay_n (n-110)
.elseif(n = 395)
	; 10 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 396)
	; 10 bytes
	delay_n 112
	delay_n (n-112)
.elseif(n = 397)
	; 10 bytes
	delay_n 113
	delay_n (n-113)
.elseif(n = 398)
	; 10 bytes
	delay_n 114
	delay_n (n-114)
.elseif(n = 399)
	; 10 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 400)
	; 10 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 401)
	; 10 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 402)
	; 10 bytes
	delay_n 118
	delay_n (n-118)
.elseif(n = 403)
	; 10 bytes
	delay_n 119
	delay_n (n-119)
.elseif(n = 404)
	; 10 bytes
	delay_n 120
	delay_n (n-120)
.elseif(n = 405)
	; 10 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 406)
	; 10 bytes
	delay_n 122
	delay_n (n-122)
.elseif(n = 407)
	; 10 bytes
	delay_n 123
	delay_n (n-123)
.elseif(n = 408)
	; 10 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 409)
	; 10 bytes
	delay_n 125
	delay_n (n-125)
.endif
.else
; 410 to 429
.if(n = 410)
	; 10 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 411)
	; 10 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 412)
	; 10 bytes
	delay_n 128
	delay_n (n-128)
.elseif(n = 413)
	; 10 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 414)
	; 10 bytes
	delay_n 130
	delay_n (n-130)
.elseif(n = 415)
	; 10 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 416)
	; 10 bytes
	delay_n 132
	delay_n (n-132)
.elseif(n = 417)
	; 10 bytes
	delay_n 133
	delay_n (n-133)
.elseif(n = 418)
	; 10 bytes
	delay_n 134
	delay_n (n-134)
.elseif(n = 419)
	; 10 bytes
	delay_n 135
	delay_n (n-135)
.elseif(n = 420)
	; 10 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 421)
	; 10 bytes
	delay_n 137
	delay_n (n-137)
.elseif(n = 422)
	; 10 bytes
	delay_n 138
	delay_n (n-138)
.elseif(n = 423)
	; 10 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 424)
	; 10 bytes
	delay_n 140
	delay_n (n-140)
.elseif(n = 425)
	; 10 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 426)
	; 10 bytes
	delay_n 142
	delay_n (n-142)
.elseif(n = 427)
	; 10 bytes
	delay_n 143
	delay_n (n-143)
.elseif(n = 428)
	; 10 bytes
	delay_n 144
	delay_n (n-144)
.elseif(n = 429)
	; 10 bytes
	delay_n 145
	delay_n (n-145)
.endif
.endif
.else
.if((n >= 430) && (n < 449))
; 430 to 448
.if(n = 430)
	; 10 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 431)
	; 10 bytes
	delay_n 147
	delay_n (n-147)
.elseif(n = 432)
	; 10 bytes
	delay_n 148
	delay_n (n-148)
.elseif(n = 433)
	; 10 bytes
	delay_n 149
	delay_n (n-149)
.elseif(n = 434)
	; 10 bytes
	delay_n 150
	delay_n (n-150)
.elseif(n = 435)
	; 10 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 436)
	; 10 bytes
	delay_n 152
	delay_n (n-152)
.elseif(n = 437)
	; 10 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 438)
	; 10 bytes
	delay_n 154
	delay_n (n-154)
.elseif(n = 439)
	; 10 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 440)
	; 10 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 441)
	; 10 bytes
	delay_n 157
	delay_n (n-157)
.elseif(n = 442)
	; 10 bytes
	delay_n 158
	delay_n (n-158)
.elseif(n = 443)
	; 10 bytes
	delay_n 159
	delay_n (n-159)
.elseif(n = 444)
	; 10 bytes
	delay_n 160
	delay_n (n-160)
.elseif(n = 445)
	; 10 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 446)
	; 10 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 447)
	; 10 bytes
	delay_n 163
	delay_n (n-163)
.elseif(n = 448)
	; 10 bytes
	delay_n 164
	delay_n (n-164)
.endif
.else
; 449 to 468
.if(n = 449)
	; 10 bytes
	delay_n 165
	delay_n (n-165)
.elseif(n = 450)
	; 10 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 451)
	; 10 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 452)
	; 10 bytes
	delay_n 168
	delay_n (n-168)
.elseif(n = 453)
	; 10 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 454)
	; 10 bytes
	delay_n 170
	delay_n (n-170)
.elseif(n = 455)
	; 10 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 456)
	; 10 bytes
	delay_n 172
	delay_n (n-172)
.elseif(n = 457)
	; 10 bytes
	delay_n 173
	delay_n (n-173)
.elseif(n = 458)
	; 10 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 459)
	; 10 bytes
	delay_n 175
	delay_n (n-175)
.elseif(n = 460)
	; 10 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 461)
	; 10 bytes
	delay_n 177
	delay_n (n-177)
.elseif(n = 462)
	; 10 bytes
	delay_n 178
	delay_n (n-178)
.elseif(n = 463)
	; 10 bytes
	delay_n 179
	delay_n (n-179)
.elseif(n = 464)
	; 10 bytes
	delay_n 180
	delay_n (n-180)
.elseif(n = 465)
	; 10 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 466)
	; 10 bytes
	delay_n 182
	delay_n (n-182)
.elseif(n = 467)
	; 10 bytes
	delay_n 183
	delay_n (n-183)
.elseif(n = 468)
	; 10 bytes
	delay_n 184
	delay_n (n-184)
.endif
.endif
.endif
.endif
.else
; 469 to 625
.if(n = 469)
	; 10 bytes
	delay_n 185
	delay_n (n-185)
.elseif(n = 470)
	; 10 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 471)
	; 10 bytes
	delay_n 187
	delay_n (n-187)
.elseif(n = 472)
	; 10 bytes
	delay_n 188
	delay_n (n-188)
.elseif(n = 473)
	; 10 bytes
	delay_n 189
	delay_n (n-189)
.elseif(n = 474)
	; 10 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 475)
	; 10 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 476)
	; 10 bytes
	delay_n 192
	delay_n (n-192)
.elseif(n = 477)
	; 10 bytes
	delay_n 193
	delay_n (n-193)
.elseif(n = 478)
	; 10 bytes
	delay_n 194
	delay_n (n-194)
.elseif(n = 479)
	; 10 bytes
	delay_n 195
	delay_n (n-195)
.elseif(n = 480)
	; 10 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 481)
	; 10 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 482)
	; 10 bytes
	delay_n 198
	delay_n (n-198)
.elseif(n = 483)
	; 10 bytes
	delay_n 199
	delay_n (n-199)
.elseif(n = 484)
	; 10 bytes
	delay_n 200
	delay_n (n-200)
.elseif(n = 485 || n = 486 || n = 562)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n > 486 && n < 560) || n = 561 || (n > 564 && n < 626))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 560)
	; 9 bytes
	txa
	pha
	lax #2
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 563)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 564)
	; 10 bytes
	pla
	delay_n (n-4)
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 817) || n = 818 || (n > 821 && n < 1074) || n = 1075 || (n > 1078 && n < 1251))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 817)
	; 9 bytes
	txa
	pha
	lax #3
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 819 || n = 1076)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 820 || n = 1077)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 821 || n = 1078)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1074)
	; 9 bytes
	txa
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1331) || n = 1332 || (n > 1335 && n < 1588) || n = 1589 || (n > 1592 && n < 1845) || n = 1846 || (n > 1849 && n < 2102) || n = 2103 || (n > 2106 && n < 2359) || n = 2360 || (n > 2363 && n < 2501))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 1331)
	; 9 bytes
	txa
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 1333 || n = 1590 || n = 1847 || n = 2104 || n = 2361)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1334 || n = 1591 || n = 1848 || n = 2105 || n = 2362)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1335 || n = 1592 || n = 1849 || n = 2106 || n = 2363)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1588)
	; 9 bytes
	txa
	pha
	lax #6
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 1845)
	; 9 bytes
	txa
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2102)
	; 9 bytes
	txa
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2359)
	; 9 bytes
	txa
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2616) || n = 2617 || (n > 2620 && n < 2873) || n = 2874 || (n > 2877 && n < 3130) || n = 3131 || (n > 3134 && n < 3387) || n = 3388 || (n > 3391 && n < 3644) || n = 3645 || (n > 3648 && n < 3901) || n = 3902 || (n > 3905 && n < 4158) || n = 4159 || (n > 4162 && n < 4415) || n = 4416 || (n > 4419 && n < 4672) || n = 4673 || (n > 4676 && n < 4929) || n = 4930 || (n > 4933 && n < 5001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2616)
	; 9 bytes
	txa
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2618 || n = 2875 || n = 3132 || n = 3389 || n = 3646 || n = 3903 || n = 4160 || n = 4417 || n = 4674 || n = 4931)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2619 || n = 2876 || n = 3133 || n = 3390 || n = 3647 || n = 3904 || n = 4161 || n = 4418 || n = 4675 || n = 4932)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2620 || n = 2877 || n = 3134 || n = 3391 || n = 3648 || n = 3905 || n = 4162 || n = 4419 || n = 4676 || n = 4933)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2873)
	; 9 bytes
	txa
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3130)
	; 9 bytes
	txa
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3387)
	; 9 bytes
	txa
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3644)
	; 9 bytes
	txa
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3901)
	; 9 bytes
	txa
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4158)
	; 9 bytes
	txa
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4415)
	; 9 bytes
	txa
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4672)
	; 9 bytes
	txa
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4929)
	; 9 bytes
	txa
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5186) || n = 5187 || (n > 5190 && n < 5443) || n = 5444 || (n > 5447 && n < 5700) || n = 5701 || (n > 5704 && n < 5957) || n = 5958 || (n > 5961 && n < 6214) || n = 6215 || (n > 6218 && n < 6471) || n = 6472 || (n > 6475 && n < 6728) || n = 6729 || (n > 6732 && n < 6985) || n = 6986 || (n > 6989 && n < 7242) || n = 7243 || (n > 7246 && n < 7499) || n = 7500 || (n > 7503 && n < 7756) || n = 7757 || (n > 7760 && n < 8013) || n = 8014 || (n > 8017 && n < 8270) || n = 8271 || (n > 8274 && n < 8527) || n = 8528 || (n > 8531 && n < 8784) || n = 8785 || (n > 8788 && n < 9041) || n = 9042 || (n > 9045 && n < 9298) || n = 9299 || (n > 9302 && n < 9555) || n = 9556 || (n > 9559 && n < 9812) || n = 9813 || (n > 9816 && n < 10001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5186)
	; 9 bytes
	txa
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5188 || n = 5445 || n = 5702 || n = 5959 || n = 6216 || n = 6473 || n = 6730 || n = 6987 || n = 7244 || n = 7501 || n = 7758 || n = 8015 || n = 8272 || n = 8529 || n = 8786 || n = 9043 || n = 9300 || n = 9557 || n = 9814)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5189 || n = 5446 || n = 5703 || n = 5960 || n = 6217 || n = 6474 || n = 6731 || n = 6988 || n = 7245 || n = 7502 || n = 7759 || n = 8016 || n = 8273 || n = 8530 || n = 8787 || n = 9044 || n = 9301 || n = 9558 || n = 9815)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5190 || n = 5447 || n = 5704 || n = 5961 || n = 6218 || n = 6475 || n = 6732 || n = 6989 || n = 7246 || n = 7503 || n = 7760 || n = 8017 || n = 8274 || n = 8531 || n = 8788 || n = 9045 || n = 9302 || n = 9559 || n = 9816)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 5443)
	; 9 bytes
	txa
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5700)
	; 9 bytes
	txa
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5957)
	; 9 bytes
	txa
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6214)
	; 9 bytes
	txa
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6471)
	; 9 bytes
	txa
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6728)
	; 9 bytes
	txa
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6985)
	; 9 bytes
	txa
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 7242)
	; 9 bytes
	txa
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 7499)
	; 9 bytes
	txa
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 7756)
	; 9 bytes
	txa
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 8013)
	; 9 bytes
	txa
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 8270)
	; 9 bytes
	txa
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 8527)
	; 9 bytes
	txa
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 8784)
	; 9 bytes
	txa
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 9041)
	; 9 bytes
	txa
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 9298)
	; 9 bytes
	txa
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 9555)
	; 9 bytes
	txa
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 9812)
	; 9 bytes
	txa
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10069) || n = 10070 || (n > 10073 && n < 10326) || n = 10327 || (n > 10330 && n < 10583) || n = 10584 || (n > 10587 && n < 10840) || n = 10841 || (n > 10844 && n < 11097) || n = 11098 || (n > 11101 && n < 11354) || n = 11355 || (n > 11358 && n < 11611) || n = 11612 || (n > 11615 && n < 11868) || n = 11869 || (n > 11872 && n < 12125) || n = 12126 || (n > 12129 && n < 12382) || n = 12383 || (n > 12386 && n < 12639) || n = 12640 || (n > 12643 && n < 12896) || n = 12897 || (n > 12900 && n < 13153) || n = 13154 || (n > 13157 && n < 13410) || n = 13411 || (n > 13414 && n < 13667) || n = 13668 || (n > 13671 && n < 13924) || n = 13925 || (n > 13928 && n < 14181) || n = 14182 || (n > 14185 && n < 14438) || n = 14439 || (n > 14442 && n < 14695) || n = 14696 || (n > 14699 && n < 14952) || n = 14953 || (n > 14956 && n < 15001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 10069)
	; 9 bytes
	txa
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 10071 || n = 10328 || n = 10585 || n = 10842 || n = 11099 || n = 11356 || n = 11613 || n = 11870 || n = 12127 || n = 12384 || n = 12641 || n = 12898 || n = 13155 || n = 13412 || n = 13669 || n = 13926 || n = 14183 || n = 14440 || n = 14697 || n = 14954)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10072 || n = 10329 || n = 10586 || n = 10843 || n = 11100 || n = 11357 || n = 11614 || n = 11871 || n = 12128 || n = 12385 || n = 12642 || n = 12899 || n = 13156 || n = 13413 || n = 13670 || n = 13927 || n = 14184 || n = 14441 || n = 14698 || n = 14955)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10073 || n = 10330 || n = 10587 || n = 10844 || n = 11101 || n = 11358 || n = 11615 || n = 11872 || n = 12129 || n = 12386 || n = 12643 || n = 12900 || n = 13157 || n = 13414 || n = 13671 || n = 13928 || n = 14185 || n = 14442 || n = 14699 || n = 14956)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 10326)
	; 9 bytes
	txa
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 10583)
	; 9 bytes
	txa
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 10840)
	; 9 bytes
	txa
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 11097)
	; 9 bytes
	txa
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 11354)
	; 9 bytes
	txa
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 11611)
	; 9 bytes
	txa
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 11868)
	; 9 bytes
	txa
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 12125)
	; 9 bytes
	txa
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 12382)
	; 9 bytes
	txa
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 12639)
	; 9 bytes
	txa
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 12896)
	; 9 bytes
	txa
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 13153)
	; 9 bytes
	txa
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 13410)
	; 9 bytes
	txa
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 13667)
	; 9 bytes
	txa
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 13924)
	; 9 bytes
	txa
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 14181)
	; 9 bytes
	txa
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 14438)
	; 9 bytes
	txa
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 14695)
	; 9 bytes
	txa
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 14952)
	; 9 bytes
	txa
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15209) || n = 15210 || (n > 15213 && n < 15466) || n = 15467 || (n > 15470 && n < 15723) || n = 15724 || (n > 15727 && n < 15980) || n = 15981 || (n > 15984 && n < 16237) || n = 16238 || (n > 16241 && n < 16494) || n = 16495 || (n > 16498 && n < 16751) || n = 16752 || (n > 16755 && n < 17008) || n = 17009 || (n > 17012 && n < 17265) || n = 17266 || (n > 17269 && n < 17522) || n = 17523 || (n > 17526 && n < 17779) || n = 17780 || (n > 17783 && n < 18036) || n = 18037 || (n > 18040 && n < 18293) || n = 18294 || (n > 18297 && n < 18550) || n = 18551 || (n > 18554 && n < 18807) || n = 18808 || (n > 18811 && n < 19064) || n = 19065 || (n > 19068 && n < 19321) || n = 19322 || (n > 19325 && n < 19578) || n = 19579 || (n > 19582 && n < 19835) || n = 19836 || (n > 19839 && n < 20001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 15209)
	; 9 bytes
	txa
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 15211 || n = 15468 || n = 15725 || n = 15982 || n = 16239 || n = 16496 || n = 16753 || n = 17010 || n = 17267 || n = 17524 || n = 17781 || n = 18038 || n = 18295 || n = 18552 || n = 18809 || n = 19066 || n = 19323 || n = 19580 || n = 19837)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15212 || n = 15469 || n = 15726 || n = 15983 || n = 16240 || n = 16497 || n = 16754 || n = 17011 || n = 17268 || n = 17525 || n = 17782 || n = 18039 || n = 18296 || n = 18553 || n = 18810 || n = 19067 || n = 19324 || n = 19581 || n = 19838)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15213 || n = 15470 || n = 15727 || n = 15984 || n = 16241 || n = 16498 || n = 16755 || n = 17012 || n = 17269 || n = 17526 || n = 17783 || n = 18040 || n = 18297 || n = 18554 || n = 18811 || n = 19068 || n = 19325 || n = 19582 || n = 19839)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 15466)
	; 9 bytes
	txa
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 15723)
	; 9 bytes
	txa
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 15980)
	; 9 bytes
	txa
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 16237)
	; 9 bytes
	txa
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 16494)
	; 9 bytes
	txa
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 16751)
	; 9 bytes
	txa
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 17008)
	; 9 bytes
	txa
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 17265)
	; 9 bytes
	txa
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 17522)
	; 9 bytes
	txa
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 17779)
	; 9 bytes
	txa
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 18036)
	; 9 bytes
	txa
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 18293)
	; 9 bytes
	txa
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 18550)
	; 9 bytes
	txa
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 18807)
	; 9 bytes
	txa
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 19064)
	; 9 bytes
	txa
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 19321)
	; 9 bytes
	txa
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 19578)
	; 9 bytes
	txa
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 19835)
	; 9 bytes
	txa
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.endif
.endif

.endif
	rest
.endmacro
