; delay_n, delays n cycles with small code
; Will clobber Z+N, and V
; Will preserve A, X, Y, S, C, D, and I (interrupt-disable flag)
;
; WARNING: May write garbage into $700-$7FF.
;
; This version of delay_n does not change any bytes in the $100 page.
;
; Generated code is:
; -209 bytes or less when n <= 626
; -417 bytes or less when n <= 1250
; -626 bytes or less when n <= 1876
; -834 bytes or less when n <= 2500
; -1042 bytes or less when n <= 3124
; -1251 bytes or less when n <= 3752
; -1459 bytes or less when n <= 4376
; -1667 bytes or less when n <= 5000
; -1876 bytes or less when n <= 5626
; -2084 bytes or less when n <= 6250
; -2292 bytes or less when n <= 6874
; -2501 bytes or less when n <= 7502
; -2709 bytes or less when n <= 8126
; -2917 bytes or less when n <= 8750
; -3126 bytes or less when n <= 9376
; -3334 bytes or less when n <= 10000
; -3542 bytes or less when n <= 10624
; -3751 bytes or less when n <= 11252
; -3959 bytes or less when n <= 11876
; -4167 bytes or less when n <= 12500
; -4376 bytes or less when n <= 13126
; -4584 bytes or less when n <= 13750
; -4792 bytes or less when n <= 14374
; -5001 bytes or less when n <= 15002
; -5209 bytes or less when n <= 15626
; -5417 bytes or less when n <= 16250
; -5626 bytes or less when n <= 16876
; -5834 bytes or less when n <= 17500
; -6042 bytes or less when n <= 18124
; -6251 bytes or less when n <= 18752
; -6459 bytes or less when n <= 19376
; -6667 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.9, max=4
; - When 11 <= n <= 20: min=4, avg=6.1, max=8
; - When 21 <= n <= 30: min=8, avg=9.4, max=11
; - When 31 <= n <= 50: min=11, avg=14.4, max=18
; - When 51 <= n <= 75: min=18, avg=21.7, max=26
; - When 76 <= n <= 100: min=26, avg=30.0, max=34
; - When 101 <= n <= 200: min=34, avg=50.8, max=67
; - When 201 <= n <= 300: min=68, avg=84.2, max=101
; - When 301 <= n <= 500: min=101, avg=134.2, max=167
; - When 501 <= n <= 750: min=168, avg=209.2, max=251
; - When 751 <= n <= 1000: min=251, avg=292.5, max=334
; - When 1001 <= n <= 2000: min=334, avg=500.8, max=667
; - When 2001 <= n <= 3000: min=668, avg=834.2, max=1001
; - When 3001 <= n <= 5000: min=1001, avg=1334.2, max=1667
; - When 5001 <= n <= 7500: min=1668, avg=2084.2, max=2501
; - When 7501 <= n <= 10000: min=2501, avg=2917.5, max=3334
; - When 10001 <= n <= 20000: min=3334, avg=5000.8, max=6667
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
	; 2 bytes
	bit $24
.elseif((n > 3 && n < 7) || n = 9 || n = 11 || n = 14 || (n > 15 && n < 19) || n = 21 || n = 23 || n = 26 || (n > 27 && n < 31) || n = 33 || n = 35 || n = 38 || (n > 39 && n < 43) || n = 45 || n = 47 || n = 50 || n = 52 || n = 53 || n = 56 || n = 59 || n = 62 || n = 65 || n = 68 || n = 71 || n = 74 || n = 77)
	; 2..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 7)
	; 3 bytes
	inc $0700,X
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 10)
	; 4 bytes
	rol $26
	ror $26
.elseif(n = 12)
	; 4 bytes
	rol $36,X
	ror $36,X
.elseif(n = 13 || n = 15 || n = 25 || n = 27 || n = 37 || n = 39 || n = 49 || n = 51 || n = 57 || n = 63 || n = 69 || n = 75)
	; 6..26 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19 || n = 22)
	; 7..8 bytes
	delay_n (n-12)
	delay_n 12
.elseif(n = 20)
	; 8 bytes
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
.elseif((n .mod 12 = 0) && ((n > 23 && n < 73)))
	; 8..24 bytes
	.repeat n/12
	 rol $36,X
	 ror $36,X
	.endrepeat
.elseif(n = 31 || n = 43 || n = 55 || n = 61 || n = 67)
	; 11..23 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 32)
	; 12 bytes
	delay_n (n-20)
	delay_n 20
.elseif(n = 34 || n = 46 || n = 58 || n = 64 || n = 70)
	; 12..24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 44)
	; 16 bytes
	delay_n 20
	delay_n (n-20)
.elseif(n = 54)
	; 18 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 66)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 73)
	; 25 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 76)
	; 26 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 78)
	; 26 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 79 to 156
.if(n = 79 || n = 91 || n = 103 || n = 115)
	; 27..39 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 79 && n < 156)))
	; 27..53 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 80 && n < 154)))
	; 28..52 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 82 || n = 94 || n = 106 || n = 118)
	; 28..40 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 84 || n = 96)
	; 28..32 bytes
	.repeat n/12
	 rol $36,X
	 ror $36,X
	.endrepeat
.elseif(n = 85 || n = 139)
	; 29..47 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 88 || n = 142)
	; 30..48 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 90 || n = 144)
	; 30..48 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 97 || n = 151)
	; 33..51 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 100 || n = 154)
	; 34..52 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 102 || n = 156)
	; 34..52 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 108)
	; 36 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 109 || n = 112 || n = 114 || n = 121 || n = 124 || n = 126 || n = 133 || n = 136 || n = 138 || n = 145 || n = 148 || n = 150)
	; 37..50 bytes
	delay_n (n-54)
	delay_n 54
.elseif(n = 120)
	; 40 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 127)
	; 43 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 130)
	; 44 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 132)
	; 44 bytes
	delay_n 24
	delay_n (n-24)
.endif
.endif
.else
.if((n >= 157) && (n < 235))
.if((n >= 157) && (n < 196))
; 157 to 195
.if(n = 157 || n = 160)
	; 53..54 bytes
	delay_n (n-54)
	delay_n 54
.elseif((n .mod 3 = 2) && ((n > 157 && n < 195)))
	; 53..65 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 158 && n < 196)))
	; 54..66 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 162)
	; 54 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 163)
	; 55 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 166)
	; 56 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 168)
	; 56 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 169)
	; 57 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 172)
	; 58 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 174)
	; 58 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 175)
	; 59 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 178)
	; 60 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 180)
	; 60 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 181)
	; 61 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 184)
	; 62 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 186)
	; 62 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 187)
	; 63 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 190)
	; 64 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 192)
	; 64 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 193)
	; 65 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 196 to 234
.if(n = 196)
	; 66 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 196 && n < 234)))
	; 67..79 bytes
	delay_n (n-2)
	nop
.elseif(n = 198)
	; 66 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 199)
	; 67 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 200 && n < 232)))
	; 68..78 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 202)
	; 68 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 204)
	; 68 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 205)
	; 69 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 208)
	; 70 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 210)
	; 70 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 211)
	; 71 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 214)
	; 72 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 216)
	; 72 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 217)
	; 73 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 220)
	; 74 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 222)
	; 74 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 223)
	; 75 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 226)
	; 76 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 228)
	; 76 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 229)
	; 77 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 232)
	; 78 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 234)
	; 78 bytes
	delay_n 72
	delay_n (n-72)
.endif
.endif
.else
.if((n >= 235) && (n < 274))
; 235 to 273
.if(n = 235)
	; 79 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 235 && n < 273)))
	; 79..91 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 236 && n < 274)))
	; 80..92 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 238)
	; 80 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 240)
	; 80 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 241)
	; 81 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 244)
	; 82 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 246)
	; 82 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 247)
	; 83 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 250)
	; 84 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 252)
	; 84 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 253)
	; 85 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 256)
	; 86 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 258)
	; 86 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 259)
	; 87 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 262)
	; 88 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 264)
	; 88 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 265)
	; 89 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 268)
	; 90 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 270)
	; 90 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 271)
	; 91 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 274 to 312
.if(n = 274)
	; 92 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 274 && n < 312)))
	; 93..105 bytes
	delay_n (n-2)
	nop
.elseif(n = 276)
	; 92 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 277)
	; 93 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 278 && n < 310)))
	; 94..104 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 280)
	; 94 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 282)
	; 94 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 283)
	; 95 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 286)
	; 96 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 288)
	; 96 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 289)
	; 97 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 292)
	; 98 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 294)
	; 98 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 295)
	; 99 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 298)
	; 100 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 300)
	; 100 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 301)
	; 101 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 304)
	; 102 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 306)
	; 102 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 307)
	; 103 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 310)
	; 104 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 312)
	; 104 bytes
	delay_n 96
	delay_n (n-96)
.endif
.endif
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
; 313 to 351
.if(n = 313)
	; 105 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 313 && n < 351)))
	; 105..117 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 314 && n < 352)))
	; 106..118 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 316)
	; 106 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 318)
	; 106 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 319)
	; 107 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 322)
	; 108 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 324)
	; 108 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 325)
	; 109 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 328)
	; 110 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 330)
	; 110 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 331)
	; 111 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 334)
	; 112 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 336)
	; 112 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 337)
	; 113 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 340)
	; 114 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 342)
	; 114 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 343)
	; 115 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 346)
	; 116 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 348)
	; 116 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 349)
	; 117 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 352 to 390
.if(n = 352)
	; 118 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 352 && n < 390)))
	; 119..131 bytes
	delay_n (n-2)
	nop
.elseif(n = 354)
	; 118 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 355)
	; 119 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 356 && n < 388)))
	; 120..130 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 358)
	; 120 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 360)
	; 120 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 361)
	; 121 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 364)
	; 122 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 366)
	; 122 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 367)
	; 123 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 370)
	; 124 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 372)
	; 124 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 373)
	; 125 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 376)
	; 126 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 378)
	; 126 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 379)
	; 127 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 382)
	; 128 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 384)
	; 128 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 385)
	; 129 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 388)
	; 130 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 390)
	; 130 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 391) && (n < 430))
; 391 to 429
.if(n = 391)
	; 131 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 391 && n < 429)))
	; 131..143 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 392 && n < 430)))
	; 132..144 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 394)
	; 132 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 396)
	; 132 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 397)
	; 133 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 400)
	; 134 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 402)
	; 134 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 403)
	; 135 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 406)
	; 136 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 408)
	; 136 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 409)
	; 137 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 412)
	; 138 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 414)
	; 138 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 415)
	; 139 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 418)
	; 140 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 420)
	; 140 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 421)
	; 141 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 424)
	; 142 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 426)
	; 142 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 427)
	; 143 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 430 to 468
.if(n = 430)
	; 144 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 430 && n < 468)))
	; 145..157 bytes
	delay_n (n-2)
	nop
.elseif(n = 432)
	; 144 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 433)
	; 145 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 434 && n < 466)))
	; 146..156 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 436)
	; 146 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 438)
	; 146 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 439)
	; 147 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 442)
	; 148 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 444)
	; 148 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 445)
	; 149 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 448)
	; 150 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 450)
	; 150 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 451)
	; 151 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 454)
	; 152 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 456)
	; 152 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 457)
	; 153 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 460)
	; 154 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 462)
	; 154 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 463)
	; 155 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 466)
	; 156 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 468)
	; 156 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.endif
.else
.if((n >= 469) && (n < 547))
.if((n >= 469) && (n < 508))
; 469 to 507
.if(n = 469)
	; 157 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 469 && n < 507)))
	; 157..169 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 470 && n < 508)))
	; 158..170 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 472)
	; 158 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 474)
	; 158 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 475)
	; 159 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 478)
	; 160 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 480)
	; 160 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 481)
	; 161 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 484)
	; 162 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 486)
	; 162 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 487)
	; 163 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 490)
	; 164 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 492)
	; 164 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 493)
	; 165 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 496)
	; 166 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 498)
	; 166 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 499)
	; 167 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 502)
	; 168 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 504)
	; 168 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 505)
	; 169 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 508 to 546
.if(n = 508)
	; 170 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 508 && n < 546)))
	; 171..183 bytes
	delay_n (n-2)
	nop
.elseif(n = 510)
	; 170 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 511)
	; 171 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 512 && n < 544)))
	; 172..182 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 514)
	; 172 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 516)
	; 172 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 517)
	; 173 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 520)
	; 174 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 522)
	; 174 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 523)
	; 175 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 526)
	; 176 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 528)
	; 176 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 529)
	; 177 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 532)
	; 178 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 534)
	; 178 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 535)
	; 179 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 538)
	; 180 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 540)
	; 180 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 541)
	; 181 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 544)
	; 182 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 546)
	; 182 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.else
.if((n >= 547) && (n < 586))
; 547 to 585
.if(n = 547)
	; 183 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 547 && n < 585)))
	; 183..195 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 548 && n < 586)))
	; 184..196 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 550)
	; 184 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 552)
	; 184 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 553)
	; 185 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 556)
	; 186 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 558)
	; 186 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 559)
	; 187 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 562)
	; 188 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 564)
	; 188 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 565)
	; 189 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 568)
	; 190 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 570)
	; 190 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 571)
	; 191 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 574)
	; 192 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 576)
	; 192 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 577)
	; 193 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 580)
	; 194 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 582)
	; 194 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 583)
	; 195 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 586 to 625
.if(n = 586)
	; 196 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 586 && n < 624)))
	; 197..209 bytes
	delay_n (n-2)
	nop
.elseif(n = 588)
	; 196 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 589)
	; 197 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 590 && n < 622)))
	; 198..208 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 592)
	; 198 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 594)
	; 198 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 595)
	; 199 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 598)
	; 200 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 600)
	; 200 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 601)
	; 201 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 604)
	; 202 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 606)
	; 202 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 607)
	; 203 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 610)
	; 204 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 612)
	; 204 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 613)
	; 205 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 616)
	; 206 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 618)
	; 206 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 619)
	; 207 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 622)
	; 208 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 624)
	; 208 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 625)
	; 209 bytes
	delay_n 31
	delay_n (n-31)
.endif
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
.if((n .mod 3 = 2) && ((n > 625 && n < 663)))
	; 209..221 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 626 && n < 664)))
	; 210..222 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 628)
	; 210 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 630)
	; 210 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 631)
	; 211 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 634)
	; 212 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 636)
	; 212 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 637)
	; 213 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 640)
	; 214 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 642)
	; 214 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 643)
	; 215 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 646)
	; 216 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 648)
	; 216 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 649)
	; 217 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 652)
	; 218 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 654)
	; 218 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 655)
	; 219 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 658)
	; 220 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 660)
	; 220 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 661)
	; 221 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 664)
	; 222 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 665 to 703
.if((n .mod 3 = 2) && ((n > 664 && n < 702)))
	; 223..235 bytes
	delay_n (n-2)
	nop
.elseif(n = 666)
	; 222 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 667)
	; 223 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 668 && n < 700)))
	; 224..234 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 670)
	; 224 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 672)
	; 224 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 673)
	; 225 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 676)
	; 226 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 678)
	; 226 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 679)
	; 227 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 682)
	; 228 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 684)
	; 228 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 685)
	; 229 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 688)
	; 230 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 690)
	; 230 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 691)
	; 231 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 694)
	; 232 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 696)
	; 232 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 697)
	; 233 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 700)
	; 234 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 702)
	; 234 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 703)
	; 235 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 704) && (n < 743))
; 704 to 742
.if((n .mod 3 = 2) && ((n > 703 && n < 741)))
	; 235..247 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 704 && n < 742)))
	; 236..248 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 706)
	; 236 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 708)
	; 236 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 709)
	; 237 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 712)
	; 238 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 714)
	; 238 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 715)
	; 239 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 718)
	; 240 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 720)
	; 240 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 721)
	; 241 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 724)
	; 242 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 726)
	; 242 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 727)
	; 243 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 730)
	; 244 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 732)
	; 244 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 733)
	; 245 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 736)
	; 246 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 738)
	; 246 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 739)
	; 247 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 742)
	; 248 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 743 to 781
.if((n .mod 3 = 2) && ((n > 742 && n < 780)))
	; 249..261 bytes
	delay_n (n-2)
	nop
.elseif(n = 744)
	; 248 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 745)
	; 249 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 746 && n < 778)))
	; 250..260 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 748)
	; 250 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 750)
	; 250 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 751)
	; 251 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 754)
	; 252 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 756)
	; 252 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 757)
	; 253 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 760)
	; 254 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 762)
	; 254 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 763)
	; 255 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 766)
	; 256 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 768)
	; 256 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 769)
	; 257 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 772)
	; 258 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 774)
	; 258 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 775)
	; 259 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 778)
	; 260 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 780)
	; 260 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 781)
	; 261 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.else
.if((n >= 782) && (n < 860))
.if((n >= 782) && (n < 821))
; 782 to 820
.if((n .mod 3 = 2) && ((n > 781 && n < 819)))
	; 261..273 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 782 && n < 820)))
	; 262..274 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 784)
	; 262 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 786)
	; 262 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 787)
	; 263 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 790)
	; 264 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 792)
	; 264 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 793)
	; 265 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 796)
	; 266 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 798)
	; 266 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 799)
	; 267 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 802)
	; 268 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 804)
	; 268 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 805)
	; 269 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 808)
	; 270 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 810)
	; 270 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 811)
	; 271 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 814)
	; 272 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 816)
	; 272 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 817)
	; 273 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 820)
	; 274 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 821 to 859
.if((n .mod 3 = 2) && ((n > 820 && n < 858)))
	; 275..287 bytes
	delay_n (n-2)
	nop
.elseif(n = 822)
	; 274 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 823)
	; 275 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 824 && n < 856)))
	; 276..286 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 826)
	; 276 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 828)
	; 276 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 829)
	; 277 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 832)
	; 278 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 834)
	; 278 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 835)
	; 279 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 838)
	; 280 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 840)
	; 280 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 841)
	; 281 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 844)
	; 282 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 846)
	; 282 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 847)
	; 283 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 850)
	; 284 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 852)
	; 284 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 853)
	; 285 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 856)
	; 286 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 858)
	; 286 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 859)
	; 287 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 860) && (n < 899))
; 860 to 898
.if((n .mod 3 = 2) && ((n > 859 && n < 897)))
	; 287..299 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 860 && n < 898)))
	; 288..300 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 862)
	; 288 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 864)
	; 288 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 865)
	; 289 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 868)
	; 290 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 870)
	; 290 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 871)
	; 291 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 874)
	; 292 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 876)
	; 292 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 877)
	; 293 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 880)
	; 294 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 882)
	; 294 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 883)
	; 295 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 886)
	; 296 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 888)
	; 296 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 889)
	; 297 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 892)
	; 298 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 894)
	; 298 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 895)
	; 299 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 898)
	; 300 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 899 to 937
.if((n .mod 3 = 2) && ((n > 898 && n < 936)))
	; 301..313 bytes
	delay_n (n-2)
	nop
.elseif(n = 900)
	; 300 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 901)
	; 301 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 902 && n < 934)))
	; 302..312 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 904)
	; 302 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 906)
	; 302 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 907)
	; 303 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 910)
	; 304 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 912)
	; 304 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 913)
	; 305 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 916)
	; 306 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 918)
	; 306 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 919)
	; 307 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 922)
	; 308 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 924)
	; 308 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 925)
	; 309 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 928)
	; 310 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 930)
	; 310 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 931)
	; 311 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 934)
	; 312 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 936)
	; 312 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 937)
	; 313 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
.if((n >= 938) && (n < 977))
; 938 to 976
.if((n .mod 3 = 2) && ((n > 937 && n < 975)))
	; 313..325 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 938 && n < 976)))
	; 314..326 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 940)
	; 314 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 942)
	; 314 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 943)
	; 315 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 946)
	; 316 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 948)
	; 316 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 949)
	; 317 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 952)
	; 318 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 954)
	; 318 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 955)
	; 319 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 958)
	; 320 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 960)
	; 320 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 961)
	; 321 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 964)
	; 322 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 966)
	; 322 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 967)
	; 323 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 970)
	; 324 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 972)
	; 324 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 973)
	; 325 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 976)
	; 326 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 977 to 1015
.if((n .mod 3 = 2) && ((n > 976 && n < 1014)))
	; 327..339 bytes
	delay_n (n-2)
	nop
.elseif(n = 978)
	; 326 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 979)
	; 327 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 980 && n < 1012)))
	; 328..338 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 982)
	; 328 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 984)
	; 328 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 985)
	; 329 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 988)
	; 330 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 990)
	; 330 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 991)
	; 331 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 994)
	; 332 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 996)
	; 332 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 997)
	; 333 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1000)
	; 334 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1002)
	; 334 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1003)
	; 335 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1006)
	; 336 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1008)
	; 336 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1009)
	; 337 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1012)
	; 338 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1014)
	; 338 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1015)
	; 339 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.else
.if((n >= 1016) && (n < 1055))
; 1016 to 1054
.if((n .mod 3 = 2) && ((n > 1015 && n < 1053)))
	; 339..351 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1016 && n < 1054)))
	; 340..352 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1018)
	; 340 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1020)
	; 340 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1021)
	; 341 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1024)
	; 342 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1026)
	; 342 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1027)
	; 343 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1030)
	; 344 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1032)
	; 344 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1033)
	; 345 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1036)
	; 346 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1038)
	; 346 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1039)
	; 347 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1042)
	; 348 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1044)
	; 348 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1045)
	; 349 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1048)
	; 350 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1050)
	; 350 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1051)
	; 351 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1054)
	; 352 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 1055 to 1093
.if((n .mod 3 = 2) && ((n > 1054 && n < 1092)))
	; 353..365 bytes
	delay_n (n-2)
	nop
.elseif(n = 1056)
	; 352 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1057)
	; 353 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 1058 && n < 1090)))
	; 354..364 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1060)
	; 354 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1062)
	; 354 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1063)
	; 355 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1066)
	; 356 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1068)
	; 356 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1069)
	; 357 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1072)
	; 358 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1074)
	; 358 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1075)
	; 359 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1078)
	; 360 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1080)
	; 360 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1081)
	; 361 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1084)
	; 362 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1086)
	; 362 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1087)
	; 363 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1090)
	; 364 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1092)
	; 364 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1093)
	; 365 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 1094) && (n < 1172))
.if((n >= 1094) && (n < 1133))
; 1094 to 1132
.if((n .mod 3 = 2) && ((n > 1093 && n < 1131)))
	; 365..377 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1094 && n < 1132)))
	; 366..378 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1096)
	; 366 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1098)
	; 366 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1099)
	; 367 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1102)
	; 368 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1104)
	; 368 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1105)
	; 369 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1108)
	; 370 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1110)
	; 370 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1111)
	; 371 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1114)
	; 372 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1116)
	; 372 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1117)
	; 373 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1120)
	; 374 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1122)
	; 374 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1123)
	; 375 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1126)
	; 376 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1128)
	; 376 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1129)
	; 377 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1132)
	; 378 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 1133 to 1171
.if((n .mod 3 = 2) && ((n > 1132 && n < 1170)))
	; 379..391 bytes
	delay_n (n-2)
	nop
.elseif(n = 1134)
	; 378 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1135)
	; 379 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 1136 && n < 1168)))
	; 380..390 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1138)
	; 380 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1140)
	; 380 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1141)
	; 381 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1144)
	; 382 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1146)
	; 382 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1147)
	; 383 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1150)
	; 384 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1152)
	; 384 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1153)
	; 385 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1156)
	; 386 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1158)
	; 386 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1159)
	; 387 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1162)
	; 388 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1164)
	; 388 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1165)
	; 389 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1168)
	; 390 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1170)
	; 390 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1171)
	; 391 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 1172) && (n < 1211))
; 1172 to 1210
.if((n .mod 3 = 2) && ((n > 1171 && n < 1209)))
	; 391..403 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1172 && n < 1210)))
	; 392..404 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1174)
	; 392 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1176)
	; 392 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1177)
	; 393 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1180)
	; 394 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1182)
	; 394 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1183)
	; 395 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1186)
	; 396 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1188)
	; 396 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1189)
	; 397 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1192)
	; 398 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1194)
	; 398 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1195)
	; 399 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1198)
	; 400 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1200)
	; 400 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1201)
	; 401 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1204)
	; 402 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1206)
	; 402 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1207)
	; 403 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1210)
	; 404 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 1211 to 1250
.if((n .mod 3 = 2) && ((n > 1210 && n < 1251)))
	; 405..417 bytes
	delay_n (n-2)
	nop
.elseif(n = 1212)
	; 404 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1213)
	; 405 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 1214 && n < 1246)))
	; 406..416 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1216)
	; 406 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1218)
	; 406 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1219)
	; 407 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1222)
	; 408 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1224)
	; 408 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1225)
	; 409 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1228)
	; 410 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1230)
	; 410 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1231)
	; 411 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1234)
	; 412 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1236)
	; 412 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1237)
	; 413 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1240)
	; 414 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1242)
	; 414 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1243)
	; 415 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1246)
	; 416 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1248)
	; 416 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1249)
	; 417 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
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
.if((n .mod 6 = 3) && ((n > 1250 && n < 1288)))
	; 418..430 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1252)
	; 418 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 1252 && n < 1290)))
	; 419..431 bytes
	delay_n (n-2)
	nop
.elseif(n = 1254)
	; 418 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1255)
	; 419 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1258)
	; 420 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1260)
	; 420 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1261)
	; 421 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1264)
	; 422 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1266)
	; 422 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1267)
	; 423 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1270)
	; 424 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1272)
	; 424 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1273)
	; 425 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1276)
	; 426 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1278)
	; 426 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1279)
	; 427 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1282)
	; 428 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1284)
	; 428 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1285)
	; 429 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1288)
	; 430 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 1290 to 1328
.if(n = 1290)
	; 430 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1291)
	; 431 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 1291 && n < 1329)))
	; 431..443 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1292 && n < 1324)))
	; 432..442 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1294)
	; 432 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1296)
	; 432 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1297)
	; 433 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1300)
	; 434 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1302)
	; 434 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1303)
	; 435 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1306)
	; 436 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1308)
	; 436 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1309)
	; 437 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1312)
	; 438 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1314)
	; 438 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1315)
	; 439 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1318)
	; 440 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1320)
	; 440 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1321)
	; 441 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1324)
	; 442 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1326)
	; 442 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1327)
	; 443 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if((n .mod 6 = 3) && ((n > 1328 && n < 1366)))
	; 444..456 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1330)
	; 444 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 1330 && n < 1368)))
	; 445..457 bytes
	delay_n (n-2)
	nop
.elseif(n = 1332)
	; 444 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1333)
	; 445 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1336)
	; 446 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1338)
	; 446 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1339)
	; 447 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1342)
	; 448 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1344)
	; 448 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1345)
	; 449 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1348)
	; 450 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1350)
	; 450 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1351)
	; 451 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1354)
	; 452 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1356)
	; 452 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1357)
	; 453 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1360)
	; 454 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1362)
	; 454 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1363)
	; 455 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1366)
	; 456 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 1368 to 1406
.if(n = 1368)
	; 456 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1369)
	; 457 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 1369 && n < 1407)))
	; 457..469 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1370 && n < 1402)))
	; 458..468 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1372)
	; 458 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1374)
	; 458 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1375)
	; 459 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1378)
	; 460 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1380)
	; 460 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1381)
	; 461 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1384)
	; 462 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1386)
	; 462 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1387)
	; 463 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1390)
	; 464 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1392)
	; 464 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1393)
	; 465 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1396)
	; 466 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1398)
	; 466 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1399)
	; 467 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1402)
	; 468 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1404)
	; 468 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1405)
	; 469 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if((n .mod 6 = 3) && ((n > 1406 && n < 1444)))
	; 470..482 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1408)
	; 470 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 1408 && n < 1446)))
	; 471..483 bytes
	delay_n (n-2)
	nop
.elseif(n = 1410)
	; 470 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1411)
	; 471 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1414)
	; 472 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1416)
	; 472 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1417)
	; 473 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1420)
	; 474 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1422)
	; 474 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1423)
	; 475 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1426)
	; 476 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1428)
	; 476 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1429)
	; 477 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1432)
	; 478 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1434)
	; 478 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1435)
	; 479 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1438)
	; 480 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1440)
	; 480 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1441)
	; 481 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1444)
	; 482 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 1446 to 1484
.if(n = 1446)
	; 482 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1447)
	; 483 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 1447 && n < 1485)))
	; 483..495 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1448 && n < 1480)))
	; 484..494 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1450)
	; 484 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1452)
	; 484 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1453)
	; 485 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1456)
	; 486 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1458)
	; 486 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1459)
	; 487 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1462)
	; 488 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1464)
	; 488 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1465)
	; 489 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1468)
	; 490 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1470)
	; 490 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1471)
	; 491 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1474)
	; 492 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1476)
	; 492 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1477)
	; 493 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1480)
	; 494 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1482)
	; 494 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1483)
	; 495 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if((n .mod 6 = 3) && ((n > 1484 && n < 1522)))
	; 496..508 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1486)
	; 496 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 1486 && n < 1524)))
	; 497..509 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488)
	; 496 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1489)
	; 497 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1492)
	; 498 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1494)
	; 498 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1495)
	; 499 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1498)
	; 500 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1500)
	; 500 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1501)
	; 501 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1504)
	; 502 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1506)
	; 502 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1507)
	; 503 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1510)
	; 504 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1512)
	; 504 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1513)
	; 505 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1516)
	; 506 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1518)
	; 506 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1519)
	; 507 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1522)
	; 508 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 508 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1525)
	; 509 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 1525 && n < 1563)))
	; 509..521 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1526 && n < 1558)))
	; 510..520 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1528)
	; 510 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1530)
	; 510 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1531)
	; 511 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1534)
	; 512 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1536)
	; 512 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1537)
	; 513 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1540)
	; 514 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1542)
	; 514 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1543)
	; 515 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1546)
	; 516 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1548)
	; 516 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1549)
	; 517 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1552)
	; 518 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1554)
	; 518 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1555)
	; 519 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1558)
	; 520 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1560)
	; 520 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1561)
	; 521 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if((n .mod 6 = 3) && ((n > 1562 && n < 1600)))
	; 522..534 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1564)
	; 522 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 1564 && n < 1602)))
	; 523..535 bytes
	delay_n (n-2)
	nop
.elseif(n = 1566)
	; 522 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1567)
	; 523 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1570)
	; 524 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1572)
	; 524 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1573)
	; 525 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1576)
	; 526 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1578)
	; 526 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1579)
	; 527 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1582)
	; 528 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1584)
	; 528 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1585)
	; 529 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1588)
	; 530 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1590)
	; 530 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1591)
	; 531 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1594)
	; 532 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1596)
	; 532 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1597)
	; 533 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1600)
	; 534 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 534 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1603)
	; 535 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 1603 && n < 1641)))
	; 535..547 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1604 && n < 1636)))
	; 536..546 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1606)
	; 536 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1608)
	; 536 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1609)
	; 537 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1612)
	; 538 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1614)
	; 538 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1615)
	; 539 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1618)
	; 540 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1620)
	; 540 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1621)
	; 541 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1624)
	; 542 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1626)
	; 542 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1627)
	; 543 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1630)
	; 544 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1632)
	; 544 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1633)
	; 545 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1636)
	; 546 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1638)
	; 546 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1639)
	; 547 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if((n .mod 6 = 3) && ((n > 1640 && n < 1678)))
	; 548..560 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1642)
	; 548 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 1642 && n < 1680)))
	; 549..561 bytes
	delay_n (n-2)
	nop
.elseif(n = 1644)
	; 548 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1645)
	; 549 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1648)
	; 550 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1650)
	; 550 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1651)
	; 551 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1654)
	; 552 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1656)
	; 552 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1657)
	; 553 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1660)
	; 554 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1662)
	; 554 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1663)
	; 555 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1666)
	; 556 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1668)
	; 556 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1669)
	; 557 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1672)
	; 558 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1674)
	; 558 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1675)
	; 559 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1678)
	; 560 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 1680 to 1718
.if(n = 1680)
	; 560 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1681)
	; 561 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 1681 && n < 1719)))
	; 561..573 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1682 && n < 1714)))
	; 562..572 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1684)
	; 562 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1686)
	; 562 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1687)
	; 563 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1690)
	; 564 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1692)
	; 564 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1693)
	; 565 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1696)
	; 566 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1698)
	; 566 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1699)
	; 567 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1702)
	; 568 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1704)
	; 568 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1705)
	; 569 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1708)
	; 570 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1710)
	; 570 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1711)
	; 571 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1714)
	; 572 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1716)
	; 572 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1717)
	; 573 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if((n .mod 6 = 3) && ((n > 1718 && n < 1756)))
	; 574..586 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1720)
	; 574 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 1720 && n < 1758)))
	; 575..587 bytes
	delay_n (n-2)
	nop
.elseif(n = 1722)
	; 574 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1723)
	; 575 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1726)
	; 576 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1728)
	; 576 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1729)
	; 577 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1732)
	; 578 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1734)
	; 578 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1735)
	; 579 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1738)
	; 580 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1740)
	; 580 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1741)
	; 581 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1744)
	; 582 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1746)
	; 582 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1747)
	; 583 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1750)
	; 584 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1752)
	; 584 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1753)
	; 585 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1756)
	; 586 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 1758 to 1796
.if(n = 1758)
	; 586 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1759)
	; 587 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 1759 && n < 1797)))
	; 587..599 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1760 && n < 1792)))
	; 588..598 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1762)
	; 588 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1764)
	; 588 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1765)
	; 589 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1768)
	; 590 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1770)
	; 590 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1771)
	; 591 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1774)
	; 592 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1776)
	; 592 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1777)
	; 593 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1780)
	; 594 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1782)
	; 594 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1783)
	; 595 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1786)
	; 596 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1788)
	; 596 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1789)
	; 597 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1792)
	; 598 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1794)
	; 598 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1795)
	; 599 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if((n .mod 6 = 3) && ((n > 1796 && n < 1834)))
	; 600..612 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1798)
	; 600 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 1798 && n < 1836)))
	; 601..613 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800)
	; 600 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1801)
	; 601 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1804)
	; 602 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1806)
	; 602 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1807)
	; 603 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1810)
	; 604 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1812)
	; 604 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1813)
	; 605 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1816)
	; 606 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1818)
	; 606 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1819)
	; 607 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1822)
	; 608 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1824)
	; 608 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1825)
	; 609 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1828)
	; 610 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1830)
	; 610 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1831)
	; 611 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1834)
	; 612 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 612 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1837)
	; 613 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 1837 && n < 1875)))
	; 613..625 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1838 && n < 1876)))
	; 614..626 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1840)
	; 614 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1842)
	; 614 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1843)
	; 615 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1846)
	; 616 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1848)
	; 616 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1849)
	; 617 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1852)
	; 618 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1854)
	; 618 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1855)
	; 619 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1858)
	; 620 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1860)
	; 620 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1861)
	; 621 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1864)
	; 622 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1866)
	; 622 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1867)
	; 623 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1870)
	; 624 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1872)
	; 624 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1873)
	; 625 bytes
	delay_n 91
	delay_n (n-91)
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
	; 626 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 1876 && n < 1914)))
	; 627..639 bytes
	delay_n (n-2)
	nop
.elseif(n = 1878)
	; 626 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1879)
	; 627 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 1880 && n < 1912)))
	; 628..638 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1882)
	; 628 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1884)
	; 628 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1885)
	; 629 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1888)
	; 630 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1890)
	; 630 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1891)
	; 631 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1894)
	; 632 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1896)
	; 632 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1897)
	; 633 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 1900)
	; 634 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1902)
	; 634 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1903)
	; 635 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1906)
	; 636 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1908)
	; 636 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1909)
	; 637 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1912)
	; 638 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1914)
	; 638 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 1915 to 1953
.if(n = 1915)
	; 639 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 1915 && n < 1953)))
	; 639..651 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1916 && n < 1954)))
	; 640..652 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1918)
	; 640 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1920)
	; 640 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1921)
	; 641 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1924)
	; 642 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1926)
	; 642 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1927)
	; 643 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1930)
	; 644 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1932)
	; 644 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1933)
	; 645 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1936)
	; 646 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1938)
	; 646 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1939)
	; 647 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1942)
	; 648 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1944)
	; 648 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1945)
	; 649 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1948)
	; 650 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1950)
	; 650 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1951)
	; 651 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if(n = 1954)
	; 652 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 1954 && n < 1992)))
	; 653..665 bytes
	delay_n (n-2)
	nop
.elseif(n = 1956)
	; 652 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1957)
	; 653 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 1958 && n < 1990)))
	; 654..664 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1960)
	; 654 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1962)
	; 654 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1963)
	; 655 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1966)
	; 656 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1968)
	; 656 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1969)
	; 657 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1972)
	; 658 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1974)
	; 658 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1975)
	; 659 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1978)
	; 660 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1980)
	; 660 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1981)
	; 661 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1984)
	; 662 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1986)
	; 662 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1987)
	; 663 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 1990)
	; 664 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1992)
	; 664 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 1993 to 2031
.if(n = 1993)
	; 665 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 1993 && n < 2031)))
	; 665..677 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 1994 && n < 2032)))
	; 666..678 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1996)
	; 666 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1998)
	; 666 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1999)
	; 667 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2002)
	; 668 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2004)
	; 668 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2005)
	; 669 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2008)
	; 670 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2010)
	; 670 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2011)
	; 671 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2014)
	; 672 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2016)
	; 672 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2017)
	; 673 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2020)
	; 674 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2022)
	; 674 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2023)
	; 675 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2026)
	; 676 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2028)
	; 676 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2029)
	; 677 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032)
	; 678 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 2032 && n < 2070)))
	; 679..691 bytes
	delay_n (n-2)
	nop
.elseif(n = 2034)
	; 678 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2035)
	; 679 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 2036 && n < 2068)))
	; 680..690 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2038)
	; 680 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2040)
	; 680 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2041)
	; 681 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2044)
	; 682 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2046)
	; 682 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2047)
	; 683 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2050)
	; 684 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2052)
	; 684 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2053)
	; 685 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2056)
	; 686 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2058)
	; 686 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2059)
	; 687 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2062)
	; 688 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2064)
	; 688 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2065)
	; 689 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2068)
	; 690 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2070)
	; 690 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 2071 to 2109
.if(n = 2071)
	; 691 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 2071 && n < 2109)))
	; 691..703 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2072 && n < 2110)))
	; 692..704 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2074)
	; 692 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2076)
	; 692 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2077)
	; 693 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2080)
	; 694 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2082)
	; 694 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2083)
	; 695 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2086)
	; 696 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2088)
	; 696 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2089)
	; 697 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2092)
	; 698 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2094)
	; 698 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2095)
	; 699 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2098)
	; 700 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2100)
	; 700 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2101)
	; 701 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2104)
	; 702 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2106)
	; 702 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2107)
	; 703 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110)
	; 704 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 2110 && n < 2148)))
	; 705..717 bytes
	delay_n (n-2)
	nop
.elseif(n = 2112)
	; 704 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2113)
	; 705 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 2114 && n < 2146)))
	; 706..716 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2116)
	; 706 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2118)
	; 706 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2119)
	; 707 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2122)
	; 708 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2124)
	; 708 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2125)
	; 709 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2128)
	; 710 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2130)
	; 710 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2131)
	; 711 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2134)
	; 712 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2136)
	; 712 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2137)
	; 713 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2140)
	; 714 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2142)
	; 714 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2143)
	; 715 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2146)
	; 716 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2148)
	; 716 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 2149 to 2187
.if(n = 2149)
	; 717 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 2149 && n < 2187)))
	; 717..729 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2150 && n < 2188)))
	; 718..730 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2152)
	; 718 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2154)
	; 718 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2155)
	; 719 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2158)
	; 720 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2160)
	; 720 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2161)
	; 721 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2164)
	; 722 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2166)
	; 722 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2167)
	; 723 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2170)
	; 724 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2172)
	; 724 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2173)
	; 725 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2176)
	; 726 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2178)
	; 726 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2179)
	; 727 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2182)
	; 728 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2184)
	; 728 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2185)
	; 729 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188)
	; 730 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 2188 && n < 2226)))
	; 731..743 bytes
	delay_n (n-2)
	nop
.elseif(n = 2190)
	; 730 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2191)
	; 731 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 2192 && n < 2224)))
	; 732..742 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2194)
	; 732 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2196)
	; 732 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2197)
	; 733 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2200)
	; 734 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2202)
	; 734 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2203)
	; 735 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2206)
	; 736 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2208)
	; 736 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2209)
	; 737 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2212)
	; 738 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2214)
	; 738 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2215)
	; 739 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2218)
	; 740 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2220)
	; 740 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2221)
	; 741 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2224)
	; 742 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2226)
	; 742 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 2227 to 2265
.if(n = 2227)
	; 743 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 2227 && n < 2265)))
	; 743..755 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2228 && n < 2266)))
	; 744..756 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2230)
	; 744 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2232)
	; 744 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2233)
	; 745 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2236)
	; 746 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2238)
	; 746 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2239)
	; 747 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2242)
	; 748 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2244)
	; 748 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2245)
	; 749 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2248)
	; 750 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2250)
	; 750 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2251)
	; 751 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2254)
	; 752 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2256)
	; 752 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2257)
	; 753 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2260)
	; 754 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2262)
	; 754 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2263)
	; 755 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266)
	; 756 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 2266 && n < 2304)))
	; 757..769 bytes
	delay_n (n-2)
	nop
.elseif(n = 2268)
	; 756 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2269)
	; 757 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 2270 && n < 2302)))
	; 758..768 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2272)
	; 758 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2274)
	; 758 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2275)
	; 759 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2278)
	; 760 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2280)
	; 760 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2281)
	; 761 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2284)
	; 762 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2286)
	; 762 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2287)
	; 763 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2290)
	; 764 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2292)
	; 764 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2293)
	; 765 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2296)
	; 766 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2298)
	; 766 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2299)
	; 767 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2302)
	; 768 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2304)
	; 768 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 2305 to 2343
.if(n = 2305)
	; 769 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 2305 && n < 2343)))
	; 769..781 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2306 && n < 2344)))
	; 770..782 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2308)
	; 770 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2310)
	; 770 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2311)
	; 771 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2314)
	; 772 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2316)
	; 772 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2317)
	; 773 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2320)
	; 774 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2322)
	; 774 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2323)
	; 775 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2326)
	; 776 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2328)
	; 776 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2329)
	; 777 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2332)
	; 778 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2334)
	; 778 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2335)
	; 779 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2338)
	; 780 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2340)
	; 780 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2341)
	; 781 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344)
	; 782 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 2344 && n < 2382)))
	; 783..795 bytes
	delay_n (n-2)
	nop
.elseif(n = 2346)
	; 782 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2347)
	; 783 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 2348 && n < 2380)))
	; 784..794 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2350)
	; 784 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2352)
	; 784 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2353)
	; 785 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2356)
	; 786 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2358)
	; 786 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2359)
	; 787 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2362)
	; 788 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2364)
	; 788 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2365)
	; 789 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2368)
	; 790 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2370)
	; 790 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2371)
	; 791 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2374)
	; 792 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2376)
	; 792 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2377)
	; 793 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2380)
	; 794 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2382)
	; 794 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 2383 to 2421
.if(n = 2383)
	; 795 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 2383 && n < 2421)))
	; 795..807 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2384 && n < 2422)))
	; 796..808 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2386)
	; 796 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2388)
	; 796 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2389)
	; 797 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2392)
	; 798 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2394)
	; 798 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2395)
	; 799 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2398)
	; 800 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2400)
	; 800 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2401)
	; 801 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2404)
	; 802 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2406)
	; 802 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2407)
	; 803 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2410)
	; 804 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2412)
	; 804 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2413)
	; 805 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2416)
	; 806 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2418)
	; 806 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2419)
	; 807 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422)
	; 808 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 2422 && n < 2460)))
	; 809..821 bytes
	delay_n (n-2)
	nop
.elseif(n = 2424)
	; 808 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2425)
	; 809 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 2426 && n < 2458)))
	; 810..820 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2428)
	; 810 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2430)
	; 810 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2431)
	; 811 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2434)
	; 812 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2436)
	; 812 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2437)
	; 813 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2440)
	; 814 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2442)
	; 814 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2443)
	; 815 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2446)
	; 816 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2448)
	; 816 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2449)
	; 817 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2452)
	; 818 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2454)
	; 818 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2455)
	; 819 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2458)
	; 820 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2460)
	; 820 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 2461 to 2500
.if(n = 2461)
	; 821 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 2461 && n < 2499)))
	; 821..833 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2462 && n < 2500)))
	; 822..834 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2464)
	; 822 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2466)
	; 822 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2467)
	; 823 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2470)
	; 824 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2472)
	; 824 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2473)
	; 825 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2476)
	; 826 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2478)
	; 826 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2479)
	; 827 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2482)
	; 828 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2484)
	; 828 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2485)
	; 829 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2488)
	; 830 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2490)
	; 830 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2491)
	; 831 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2494)
	; 832 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2496)
	; 832 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2497)
	; 833 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2500)
	; 834 bytes
	delay_n 70
	delay_n (n-70)
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
.if((n .mod 3 = 2) && ((n > 2500 && n < 2538)))
	; 835..847 bytes
	delay_n (n-2)
	nop
.elseif(n = 2502)
	; 834 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2503)
	; 835 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 2504 && n < 2536)))
	; 836..846 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2506)
	; 836 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2508)
	; 836 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2509)
	; 837 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2512)
	; 838 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2514)
	; 838 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2515)
	; 839 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2518)
	; 840 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2520)
	; 840 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2521)
	; 841 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2524)
	; 842 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2526)
	; 842 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2527)
	; 843 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2530)
	; 844 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2532)
	; 844 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2533)
	; 845 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2536)
	; 846 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2538)
	; 846 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2539)
	; 847 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 2540 to 2578
.if((n .mod 3 = 2) && ((n > 2539 && n < 2577)))
	; 847..859 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2540 && n < 2578)))
	; 848..860 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2542)
	; 848 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2544)
	; 848 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2545)
	; 849 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2548)
	; 850 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2550)
	; 850 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2551)
	; 851 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2554)
	; 852 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2556)
	; 852 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2557)
	; 853 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2560)
	; 854 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2562)
	; 854 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2563)
	; 855 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2566)
	; 856 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2568)
	; 856 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2569)
	; 857 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2572)
	; 858 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2574)
	; 858 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2575)
	; 859 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2578)
	; 860 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if((n .mod 3 = 2) && ((n > 2578 && n < 2616)))
	; 861..873 bytes
	delay_n (n-2)
	nop
.elseif(n = 2580)
	; 860 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2581)
	; 861 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 2582 && n < 2614)))
	; 862..872 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2584)
	; 862 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2586)
	; 862 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2587)
	; 863 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2590)
	; 864 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2592)
	; 864 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2593)
	; 865 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2596)
	; 866 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2598)
	; 866 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2599)
	; 867 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2602)
	; 868 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2604)
	; 868 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2605)
	; 869 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2608)
	; 870 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2610)
	; 870 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2611)
	; 871 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2614)
	; 872 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2616)
	; 872 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2617)
	; 873 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 2618 to 2656
.if((n .mod 3 = 2) && ((n > 2617 && n < 2655)))
	; 873..885 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2618 && n < 2656)))
	; 874..886 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2620)
	; 874 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2622)
	; 874 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2623)
	; 875 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2626)
	; 876 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2628)
	; 876 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2629)
	; 877 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2632)
	; 878 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2634)
	; 878 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2635)
	; 879 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2638)
	; 880 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2640)
	; 880 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2641)
	; 881 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2644)
	; 882 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2646)
	; 882 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2647)
	; 883 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2650)
	; 884 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2652)
	; 884 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2653)
	; 885 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2656)
	; 886 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if((n .mod 3 = 2) && ((n > 2656 && n < 2694)))
	; 887..899 bytes
	delay_n (n-2)
	nop
.elseif(n = 2658)
	; 886 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2659)
	; 887 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 2660 && n < 2692)))
	; 888..898 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2662)
	; 888 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2664)
	; 888 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2665)
	; 889 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2668)
	; 890 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2670)
	; 890 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2671)
	; 891 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2674)
	; 892 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2676)
	; 892 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2677)
	; 893 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2680)
	; 894 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2682)
	; 894 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2683)
	; 895 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2686)
	; 896 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2688)
	; 896 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2689)
	; 897 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2692)
	; 898 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2694)
	; 898 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2695)
	; 899 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 2696 to 2734
.if((n .mod 3 = 2) && ((n > 2695 && n < 2733)))
	; 899..911 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2696 && n < 2734)))
	; 900..912 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2698)
	; 900 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2700)
	; 900 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2701)
	; 901 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2704)
	; 902 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2706)
	; 902 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2707)
	; 903 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2710)
	; 904 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2712)
	; 904 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2713)
	; 905 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2716)
	; 906 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2718)
	; 906 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2719)
	; 907 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2722)
	; 908 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2724)
	; 908 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2725)
	; 909 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2728)
	; 910 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2730)
	; 910 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2731)
	; 911 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2734)
	; 912 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if((n .mod 3 = 2) && ((n > 2734 && n < 2772)))
	; 913..925 bytes
	delay_n (n-2)
	nop
.elseif(n = 2736)
	; 912 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2737)
	; 913 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 2738 && n < 2770)))
	; 914..924 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2740)
	; 914 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2742)
	; 914 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2743)
	; 915 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2746)
	; 916 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2748)
	; 916 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2749)
	; 917 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2752)
	; 918 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2754)
	; 918 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2755)
	; 919 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2758)
	; 920 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2760)
	; 920 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2761)
	; 921 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2764)
	; 922 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2766)
	; 922 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2767)
	; 923 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2770)
	; 924 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2772)
	; 924 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2773)
	; 925 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 2774 to 2812
.if((n .mod 3 = 2) && ((n > 2773 && n < 2811)))
	; 925..937 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2774 && n < 2812)))
	; 926..938 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2776)
	; 926 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2778)
	; 926 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2779)
	; 927 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2782)
	; 928 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2784)
	; 928 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2785)
	; 929 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2788)
	; 930 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2790)
	; 930 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2791)
	; 931 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2794)
	; 932 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2796)
	; 932 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2797)
	; 933 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2800)
	; 934 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2802)
	; 934 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2803)
	; 935 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2806)
	; 936 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2808)
	; 936 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2809)
	; 937 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2812)
	; 938 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if((n .mod 3 = 2) && ((n > 2812 && n < 2850)))
	; 939..951 bytes
	delay_n (n-2)
	nop
.elseif(n = 2814)
	; 938 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2815)
	; 939 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 2816 && n < 2848)))
	; 940..950 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2818)
	; 940 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2820)
	; 940 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2821)
	; 941 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2824)
	; 942 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2826)
	; 942 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2827)
	; 943 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2830)
	; 944 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2832)
	; 944 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2833)
	; 945 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2836)
	; 946 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2838)
	; 946 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2839)
	; 947 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2842)
	; 948 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2844)
	; 948 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2845)
	; 949 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2848)
	; 950 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2850)
	; 950 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2851)
	; 951 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 2852 to 2890
.if((n .mod 3 = 2) && ((n > 2851 && n < 2889)))
	; 951..963 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2852 && n < 2890)))
	; 952..964 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2854)
	; 952 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2856)
	; 952 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2857)
	; 953 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2860)
	; 954 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2862)
	; 954 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2863)
	; 955 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2866)
	; 956 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2868)
	; 956 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2869)
	; 957 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2872)
	; 958 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2874)
	; 958 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2875)
	; 959 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2878)
	; 960 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2880)
	; 960 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2881)
	; 961 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2884)
	; 962 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2886)
	; 962 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2887)
	; 963 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2890)
	; 964 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if((n .mod 3 = 2) && ((n > 2890 && n < 2928)))
	; 965..977 bytes
	delay_n (n-2)
	nop
.elseif(n = 2892)
	; 964 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2893)
	; 965 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 2894 && n < 2926)))
	; 966..976 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2896)
	; 966 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2898)
	; 966 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2899)
	; 967 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2902)
	; 968 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2904)
	; 968 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2905)
	; 969 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2908)
	; 970 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2910)
	; 970 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2911)
	; 971 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2914)
	; 972 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 2916)
	; 972 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2917)
	; 973 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2920)
	; 974 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2922)
	; 974 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2923)
	; 975 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2926)
	; 976 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2928)
	; 976 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2929)
	; 977 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 2930 to 2968
.if((n .mod 3 = 2) && ((n > 2929 && n < 2967)))
	; 977..989 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 2930 && n < 2968)))
	; 978..990 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2932)
	; 978 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2934)
	; 978 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2935)
	; 979 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2938)
	; 980 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2940)
	; 980 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2941)
	; 981 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2944)
	; 982 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2946)
	; 982 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2947)
	; 983 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2950)
	; 984 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2952)
	; 984 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2953)
	; 985 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2956)
	; 986 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2958)
	; 986 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2959)
	; 987 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 2962)
	; 988 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2964)
	; 988 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2965)
	; 989 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2968)
	; 990 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if((n .mod 3 = 2) && ((n > 2968 && n < 3006)))
	; 991..1003 bytes
	delay_n (n-2)
	nop
.elseif(n = 2970)
	; 990 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2971)
	; 991 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 2972 && n < 3004)))
	; 992..1002 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 2974)
	; 992 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2976)
	; 992 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2977)
	; 993 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 2980)
	; 994 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2982)
	; 994 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2983)
	; 995 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2986)
	; 996 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2988)
	; 996 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2989)
	; 997 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2992)
	; 998 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2994)
	; 998 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2995)
	; 999 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2998)
	; 1000 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3000)
	; 1000 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3001)
	; 1001 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3004)
	; 1002 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3006)
	; 1002 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3007)
	; 1003 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 3008 to 3046
.if((n .mod 3 = 2) && ((n > 3007 && n < 3045)))
	; 1003..1015 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3008 && n < 3046)))
	; 1004..1016 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3010)
	; 1004 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3012)
	; 1004 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3013)
	; 1005 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3016)
	; 1006 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3018)
	; 1006 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3019)
	; 1007 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3022)
	; 1008 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3024)
	; 1008 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3025)
	; 1009 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3028)
	; 1010 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3030)
	; 1010 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3031)
	; 1011 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3034)
	; 1012 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3036)
	; 1012 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3037)
	; 1013 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3040)
	; 1014 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3042)
	; 1014 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3043)
	; 1015 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3046)
	; 1016 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if((n .mod 3 = 2) && ((n > 3046 && n < 3084)))
	; 1017..1029 bytes
	delay_n (n-2)
	nop
.elseif(n = 3048)
	; 1016 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3049)
	; 1017 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 3050 && n < 3082)))
	; 1018..1028 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3052)
	; 1018 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3054)
	; 1018 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3055)
	; 1019 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3058)
	; 1020 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3060)
	; 1020 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3061)
	; 1021 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3064)
	; 1022 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3066)
	; 1022 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3067)
	; 1023 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3070)
	; 1024 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3072)
	; 1024 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3073)
	; 1025 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3076)
	; 1026 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3078)
	; 1026 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3079)
	; 1027 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3082)
	; 1028 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3084)
	; 1028 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3085)
	; 1029 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 3086 to 3125
.if((n .mod 3 = 2) && ((n > 3085 && n < 3126)))
	; 1029..1043 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3086 && n < 3124)))
	; 1030..1042 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3088)
	; 1030 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3090)
	; 1030 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3091)
	; 1031 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3094)
	; 1032 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3096)
	; 1032 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3097)
	; 1033 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3100)
	; 1034 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3102)
	; 1034 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3103)
	; 1035 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3106)
	; 1036 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3108)
	; 1036 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3109)
	; 1037 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3112)
	; 1038 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3114)
	; 1038 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3115)
	; 1039 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3118)
	; 1040 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3120)
	; 1040 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3121)
	; 1041 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3124)
	; 1042 bytes
	delay_n 46
	delay_n (n-46)
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
	; 1042 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3127)
	; 1043 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 3127 && n < 3165)))
	; 1043..1055 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3128 && n < 3160)))
	; 1044..1054 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3130)
	; 1044 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3132)
	; 1044 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3133)
	; 1045 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3136)
	; 1046 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3138)
	; 1046 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3139)
	; 1047 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3142)
	; 1048 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3144)
	; 1048 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3145)
	; 1049 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3148)
	; 1050 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3150)
	; 1050 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3151)
	; 1051 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3154)
	; 1052 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3156)
	; 1052 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3157)
	; 1053 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3160)
	; 1054 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3162)
	; 1054 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3163)
	; 1055 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 3165 to 3203
.if((n .mod 6 = 3) && ((n > 3164 && n < 3202)))
	; 1056..1068 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3166)
	; 1056 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 3166 && n < 3204)))
	; 1057..1069 bytes
	delay_n (n-2)
	nop
.elseif(n = 3168)
	; 1056 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3169)
	; 1057 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3172)
	; 1058 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3174)
	; 1058 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3175)
	; 1059 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3178)
	; 1060 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3180)
	; 1060 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3181)
	; 1061 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3184)
	; 1062 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3186)
	; 1062 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3187)
	; 1063 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3190)
	; 1064 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3192)
	; 1064 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3193)
	; 1065 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3196)
	; 1066 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3198)
	; 1066 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3199)
	; 1067 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3202)
	; 1068 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if(n = 3204)
	; 1068 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3205)
	; 1069 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 3205 && n < 3243)))
	; 1069..1081 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3206 && n < 3238)))
	; 1070..1080 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3208)
	; 1070 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3210)
	; 1070 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3211)
	; 1071 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3214)
	; 1072 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3216)
	; 1072 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3217)
	; 1073 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3220)
	; 1074 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3222)
	; 1074 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3223)
	; 1075 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3226)
	; 1076 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3228)
	; 1076 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3229)
	; 1077 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3232)
	; 1078 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3234)
	; 1078 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3235)
	; 1079 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3238)
	; 1080 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3240)
	; 1080 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3241)
	; 1081 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 3243 to 3281
.if((n .mod 6 = 3) && ((n > 3242 && n < 3280)))
	; 1082..1094 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3244)
	; 1082 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 3244 && n < 3282)))
	; 1083..1095 bytes
	delay_n (n-2)
	nop
.elseif(n = 3246)
	; 1082 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3247)
	; 1083 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3250)
	; 1084 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3252)
	; 1084 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3253)
	; 1085 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3256)
	; 1086 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3258)
	; 1086 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3259)
	; 1087 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3262)
	; 1088 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3264)
	; 1088 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3265)
	; 1089 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3268)
	; 1090 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3270)
	; 1090 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3271)
	; 1091 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3274)
	; 1092 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3276)
	; 1092 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3277)
	; 1093 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3280)
	; 1094 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282)
	; 1094 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3283)
	; 1095 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 3283 && n < 3321)))
	; 1095..1107 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3284 && n < 3316)))
	; 1096..1106 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3286)
	; 1096 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3288)
	; 1096 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3289)
	; 1097 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3292)
	; 1098 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3294)
	; 1098 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3295)
	; 1099 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3298)
	; 1100 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3300)
	; 1100 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3301)
	; 1101 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3304)
	; 1102 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3306)
	; 1102 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3307)
	; 1103 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3310)
	; 1104 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3312)
	; 1104 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3313)
	; 1105 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3316)
	; 1106 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3318)
	; 1106 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3319)
	; 1107 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 3321 to 3359
.if((n .mod 6 = 3) && ((n > 3320 && n < 3358)))
	; 1108..1120 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3322)
	; 1108 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 3322 && n < 3360)))
	; 1109..1121 bytes
	delay_n (n-2)
	nop
.elseif(n = 3324)
	; 1108 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3325)
	; 1109 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3328)
	; 1110 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3330)
	; 1110 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3331)
	; 1111 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3334)
	; 1112 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3336)
	; 1112 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3337)
	; 1113 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3340)
	; 1114 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3342)
	; 1114 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3343)
	; 1115 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3346)
	; 1116 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3348)
	; 1116 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3349)
	; 1117 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3352)
	; 1118 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3354)
	; 1118 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3355)
	; 1119 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3358)
	; 1120 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360)
	; 1120 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3361)
	; 1121 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 3361 && n < 3399)))
	; 1121..1133 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3362 && n < 3394)))
	; 1122..1132 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3364)
	; 1122 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3366)
	; 1122 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3367)
	; 1123 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3370)
	; 1124 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3372)
	; 1124 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3373)
	; 1125 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3376)
	; 1126 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3378)
	; 1126 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3379)
	; 1127 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3382)
	; 1128 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3384)
	; 1128 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3385)
	; 1129 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3388)
	; 1130 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3390)
	; 1130 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3391)
	; 1131 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3394)
	; 1132 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3396)
	; 1132 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3397)
	; 1133 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 3399 to 3437
.if((n .mod 6 = 3) && ((n > 3398 && n < 3436)))
	; 1134..1146 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3400)
	; 1134 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 3400 && n < 3438)))
	; 1135..1147 bytes
	delay_n (n-2)
	nop
.elseif(n = 3402)
	; 1134 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3403)
	; 1135 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3406)
	; 1136 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3408)
	; 1136 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3409)
	; 1137 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3412)
	; 1138 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3414)
	; 1138 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3415)
	; 1139 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3418)
	; 1140 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3420)
	; 1140 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3421)
	; 1141 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3424)
	; 1142 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3426)
	; 1142 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3427)
	; 1143 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3430)
	; 1144 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3432)
	; 1144 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3433)
	; 1145 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3436)
	; 1146 bytes
	delay_n 34
	delay_n (n-34)
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
	; 1146 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3439)
	; 1147 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 3439 && n < 3477)))
	; 1147..1159 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3440 && n < 3472)))
	; 1148..1158 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3442)
	; 1148 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3444)
	; 1148 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3445)
	; 1149 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3448)
	; 1150 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3450)
	; 1150 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3451)
	; 1151 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3454)
	; 1152 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3456)
	; 1152 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3457)
	; 1153 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3460)
	; 1154 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3462)
	; 1154 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3463)
	; 1155 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3466)
	; 1156 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3468)
	; 1156 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3469)
	; 1157 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3472)
	; 1158 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3474)
	; 1158 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3475)
	; 1159 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 3477 to 3515
.if((n .mod 6 = 3) && ((n > 3476 && n < 3514)))
	; 1160..1172 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3478)
	; 1160 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 3478 && n < 3516)))
	; 1161..1173 bytes
	delay_n (n-2)
	nop
.elseif(n = 3480)
	; 1160 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3481)
	; 1161 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3484)
	; 1162 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3486)
	; 1162 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3487)
	; 1163 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3490)
	; 1164 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3492)
	; 1164 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3493)
	; 1165 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3496)
	; 1166 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3498)
	; 1166 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3499)
	; 1167 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3502)
	; 1168 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3504)
	; 1168 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3505)
	; 1169 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3508)
	; 1170 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3510)
	; 1170 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3511)
	; 1171 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3514)
	; 1172 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516)
	; 1172 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3517)
	; 1173 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 3517 && n < 3555)))
	; 1173..1185 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3518 && n < 3550)))
	; 1174..1184 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3520)
	; 1174 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3522)
	; 1174 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3523)
	; 1175 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3526)
	; 1176 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3528)
	; 1176 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3529)
	; 1177 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3532)
	; 1178 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3534)
	; 1178 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3535)
	; 1179 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3538)
	; 1180 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3540)
	; 1180 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3541)
	; 1181 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3544)
	; 1182 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3546)
	; 1182 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3547)
	; 1183 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3550)
	; 1184 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3552)
	; 1184 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3553)
	; 1185 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 3555 to 3593
.if((n .mod 6 = 3) && ((n > 3554 && n < 3592)))
	; 1186..1198 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3556)
	; 1186 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 3556 && n < 3594)))
	; 1187..1199 bytes
	delay_n (n-2)
	nop
.elseif(n = 3558)
	; 1186 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3559)
	; 1187 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3562)
	; 1188 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3564)
	; 1188 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3565)
	; 1189 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3568)
	; 1190 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3570)
	; 1190 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3571)
	; 1191 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3574)
	; 1192 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3576)
	; 1192 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3577)
	; 1193 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3580)
	; 1194 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3582)
	; 1194 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3583)
	; 1195 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3586)
	; 1196 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3588)
	; 1196 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3589)
	; 1197 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3592)
	; 1198 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594)
	; 1198 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3595)
	; 1199 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 3595 && n < 3633)))
	; 1199..1211 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3596 && n < 3628)))
	; 1200..1210 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3598)
	; 1200 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3600)
	; 1200 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3601)
	; 1201 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3604)
	; 1202 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3606)
	; 1202 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3607)
	; 1203 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3610)
	; 1204 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3612)
	; 1204 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3613)
	; 1205 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3616)
	; 1206 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3618)
	; 1206 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3619)
	; 1207 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3622)
	; 1208 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3624)
	; 1208 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3625)
	; 1209 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3628)
	; 1210 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3630)
	; 1210 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3631)
	; 1211 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 3633 to 3671
.if((n .mod 6 = 3) && ((n > 3632 && n < 3670)))
	; 1212..1224 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3634)
	; 1212 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 3634 && n < 3672)))
	; 1213..1225 bytes
	delay_n (n-2)
	nop
.elseif(n = 3636)
	; 1212 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3637)
	; 1213 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3640)
	; 1214 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3642)
	; 1214 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3643)
	; 1215 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3646)
	; 1216 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3648)
	; 1216 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3649)
	; 1217 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3652)
	; 1218 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3654)
	; 1218 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3655)
	; 1219 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3658)
	; 1220 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3660)
	; 1220 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3661)
	; 1221 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3664)
	; 1222 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3666)
	; 1222 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3667)
	; 1223 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3670)
	; 1224 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672)
	; 1224 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3673)
	; 1225 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 3673 && n < 3711)))
	; 1225..1237 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3674 && n < 3706)))
	; 1226..1236 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3676)
	; 1226 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3678)
	; 1226 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3679)
	; 1227 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3682)
	; 1228 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3684)
	; 1228 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3685)
	; 1229 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3688)
	; 1230 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3690)
	; 1230 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3691)
	; 1231 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3694)
	; 1232 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3696)
	; 1232 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3697)
	; 1233 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3700)
	; 1234 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3702)
	; 1234 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3703)
	; 1235 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3706)
	; 1236 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3708)
	; 1236 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3709)
	; 1237 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 3711 to 3750
.if((n .mod 6 = 3) && ((n > 3710 && n < 3748)))
	; 1238..1250 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3712)
	; 1238 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 3712 && n < 3750)))
	; 1239..1251 bytes
	delay_n (n-2)
	nop
.elseif(n = 3714)
	; 1238 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3715)
	; 1239 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3718)
	; 1240 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3720)
	; 1240 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3721)
	; 1241 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3724)
	; 1242 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3726)
	; 1242 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3727)
	; 1243 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3730)
	; 1244 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3732)
	; 1244 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3733)
	; 1245 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3736)
	; 1246 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3738)
	; 1246 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3739)
	; 1247 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3742)
	; 1248 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3744)
	; 1248 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3745)
	; 1249 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3748)
	; 1250 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3750)
	; 1250 bytes
	delay_n 24
	delay_n (n-24)
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
.if(n = 3751)
	; 1251 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 3751 && n < 3789)))
	; 1251..1263 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3752 && n < 3790)))
	; 1252..1264 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3754)
	; 1252 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3756)
	; 1252 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3757)
	; 1253 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3760)
	; 1254 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3762)
	; 1254 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3763)
	; 1255 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3766)
	; 1256 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3768)
	; 1256 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3769)
	; 1257 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3772)
	; 1258 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3774)
	; 1258 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3775)
	; 1259 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3778)
	; 1260 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3780)
	; 1260 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3781)
	; 1261 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3784)
	; 1262 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3786)
	; 1262 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3787)
	; 1263 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 3790 to 3828
.if(n = 3790)
	; 1264 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 3790 && n < 3828)))
	; 1265..1277 bytes
	delay_n (n-2)
	nop
.elseif(n = 3792)
	; 1264 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3793)
	; 1265 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 3794 && n < 3826)))
	; 1266..1276 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3796)
	; 1266 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3798)
	; 1266 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3799)
	; 1267 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3802)
	; 1268 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3804)
	; 1268 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3805)
	; 1269 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3808)
	; 1270 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3810)
	; 1270 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3811)
	; 1271 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3814)
	; 1272 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3816)
	; 1272 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3817)
	; 1273 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3820)
	; 1274 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3822)
	; 1274 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3823)
	; 1275 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3826)
	; 1276 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3828)
	; 1276 bytes
	delay_n 48
	delay_n (n-48)
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829)
	; 1277 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 3829 && n < 3867)))
	; 1277..1289 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3830 && n < 3868)))
	; 1278..1290 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3832)
	; 1278 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3834)
	; 1278 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3835)
	; 1279 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3838)
	; 1280 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3840)
	; 1280 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3841)
	; 1281 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3844)
	; 1282 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3846)
	; 1282 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3847)
	; 1283 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3850)
	; 1284 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3852)
	; 1284 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3853)
	; 1285 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3856)
	; 1286 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3858)
	; 1286 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3859)
	; 1287 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3862)
	; 1288 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3864)
	; 1288 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3865)
	; 1289 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 3868 to 3906
.if(n = 3868)
	; 1290 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 3868 && n < 3906)))
	; 1291..1303 bytes
	delay_n (n-2)
	nop
.elseif(n = 3870)
	; 1290 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3871)
	; 1291 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 3872 && n < 3904)))
	; 1292..1302 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3874)
	; 1292 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3876)
	; 1292 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3877)
	; 1293 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3880)
	; 1294 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3882)
	; 1294 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3883)
	; 1295 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3886)
	; 1296 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3888)
	; 1296 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3889)
	; 1297 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3892)
	; 1298 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3894)
	; 1298 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3895)
	; 1299 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 3898)
	; 1300 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3900)
	; 1300 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3901)
	; 1301 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3904)
	; 1302 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3906)
	; 1302 bytes
	delay_n 72
	delay_n (n-72)
.endif
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
; 3907 to 3945
.if(n = 3907)
	; 1303 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 3907 && n < 3945)))
	; 1303..1315 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3908 && n < 3946)))
	; 1304..1316 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3910)
	; 1304 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3912)
	; 1304 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3913)
	; 1305 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3916)
	; 1306 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3918)
	; 1306 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3919)
	; 1307 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3922)
	; 1308 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3924)
	; 1308 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3925)
	; 1309 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3928)
	; 1310 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3930)
	; 1310 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3931)
	; 1311 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 3934)
	; 1312 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3936)
	; 1312 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3937)
	; 1313 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3940)
	; 1314 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3942)
	; 1314 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3943)
	; 1315 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 3946 to 3984
.if(n = 3946)
	; 1316 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 3946 && n < 3984)))
	; 1317..1329 bytes
	delay_n (n-2)
	nop
.elseif(n = 3948)
	; 1316 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3949)
	; 1317 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 3950 && n < 3982)))
	; 1318..1328 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3952)
	; 1318 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3954)
	; 1318 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3955)
	; 1319 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3958)
	; 1320 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3960)
	; 1320 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3961)
	; 1321 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3964)
	; 1322 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3966)
	; 1322 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3967)
	; 1323 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3970)
	; 1324 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3972)
	; 1324 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3973)
	; 1325 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3976)
	; 1326 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3978)
	; 1326 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3979)
	; 1327 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3982)
	; 1328 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3984)
	; 1328 bytes
	delay_n 96
	delay_n (n-96)
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985)
	; 1329 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 3985 && n < 4023)))
	; 1329..1341 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3986 && n < 4024)))
	; 1330..1342 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 3988)
	; 1330 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3990)
	; 1330 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3991)
	; 1331 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3994)
	; 1332 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3996)
	; 1332 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3997)
	; 1333 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4000)
	; 1334 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4002)
	; 1334 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4003)
	; 1335 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4006)
	; 1336 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4008)
	; 1336 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4009)
	; 1337 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4012)
	; 1338 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4014)
	; 1338 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4015)
	; 1339 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4018)
	; 1340 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4020)
	; 1340 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4021)
	; 1341 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 4024 to 4062
.if(n = 4024)
	; 1342 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 4024 && n < 4062)))
	; 1343..1355 bytes
	delay_n (n-2)
	nop
.elseif(n = 4026)
	; 1342 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4027)
	; 1343 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 4028 && n < 4060)))
	; 1344..1354 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4030)
	; 1344 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4032)
	; 1344 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4033)
	; 1345 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4036)
	; 1346 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4038)
	; 1346 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4039)
	; 1347 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4042)
	; 1348 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4044)
	; 1348 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4045)
	; 1349 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4048)
	; 1350 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4050)
	; 1350 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4051)
	; 1351 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4054)
	; 1352 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4056)
	; 1352 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4057)
	; 1353 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4060)
	; 1354 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4062)
	; 1354 bytes
	delay_n 12
	delay_n (n-12)
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
	; 1355 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 4063 && n < 4101)))
	; 1355..1367 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4064 && n < 4102)))
	; 1356..1368 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4066)
	; 1356 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4068)
	; 1356 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4069)
	; 1357 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4072)
	; 1358 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4074)
	; 1358 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4075)
	; 1359 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4078)
	; 1360 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4080)
	; 1360 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4081)
	; 1361 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4084)
	; 1362 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4086)
	; 1362 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4087)
	; 1363 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4090)
	; 1364 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4092)
	; 1364 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4093)
	; 1365 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4096)
	; 1366 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4098)
	; 1366 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4099)
	; 1367 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 4102 to 4140
.if(n = 4102)
	; 1368 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 4102 && n < 4140)))
	; 1369..1381 bytes
	delay_n (n-2)
	nop
.elseif(n = 4104)
	; 1368 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4105)
	; 1369 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 4106 && n < 4138)))
	; 1370..1380 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4108)
	; 1370 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4110)
	; 1370 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4111)
	; 1371 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4114)
	; 1372 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4116)
	; 1372 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4117)
	; 1373 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4120)
	; 1374 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4122)
	; 1374 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4123)
	; 1375 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4126)
	; 1376 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4128)
	; 1376 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4129)
	; 1377 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4132)
	; 1378 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4134)
	; 1378 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4135)
	; 1379 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4138)
	; 1380 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4140)
	; 1380 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141)
	; 1381 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 4141 && n < 4179)))
	; 1381..1393 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4142 && n < 4180)))
	; 1382..1394 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4144)
	; 1382 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4146)
	; 1382 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4147)
	; 1383 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4150)
	; 1384 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4152)
	; 1384 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4153)
	; 1385 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4156)
	; 1386 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4158)
	; 1386 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4159)
	; 1387 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4162)
	; 1388 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4164)
	; 1388 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4165)
	; 1389 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4168)
	; 1390 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4170)
	; 1390 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4171)
	; 1391 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4174)
	; 1392 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4176)
	; 1392 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4177)
	; 1393 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 4180 to 4218
.if(n = 4180)
	; 1394 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 4180 && n < 4218)))
	; 1395..1407 bytes
	delay_n (n-2)
	nop
.elseif(n = 4182)
	; 1394 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4183)
	; 1395 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 4184 && n < 4216)))
	; 1396..1406 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4186)
	; 1396 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4188)
	; 1396 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4189)
	; 1397 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4192)
	; 1398 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4194)
	; 1398 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4195)
	; 1399 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4198)
	; 1400 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4200)
	; 1400 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4201)
	; 1401 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4204)
	; 1402 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4206)
	; 1402 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4207)
	; 1403 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4210)
	; 1404 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4212)
	; 1404 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4213)
	; 1405 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4216)
	; 1406 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4218)
	; 1406 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
.if((n >= 4219) && (n < 4258))
; 4219 to 4257
.if(n = 4219)
	; 1407 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 4219 && n < 4257)))
	; 1407..1419 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4220 && n < 4258)))
	; 1408..1420 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4222)
	; 1408 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4224)
	; 1408 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4225)
	; 1409 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4228)
	; 1410 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4230)
	; 1410 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4231)
	; 1411 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4234)
	; 1412 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4236)
	; 1412 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4237)
	; 1413 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4240)
	; 1414 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4242)
	; 1414 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4243)
	; 1415 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4246)
	; 1416 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4248)
	; 1416 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4249)
	; 1417 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4252)
	; 1418 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4254)
	; 1418 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4255)
	; 1419 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 4258 to 4296
.if(n = 4258)
	; 1420 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 4258 && n < 4296)))
	; 1421..1433 bytes
	delay_n (n-2)
	nop
.elseif(n = 4260)
	; 1420 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4261)
	; 1421 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 4262 && n < 4294)))
	; 1422..1432 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4264)
	; 1422 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4266)
	; 1422 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4267)
	; 1423 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4270)
	; 1424 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4272)
	; 1424 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4273)
	; 1425 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4276)
	; 1426 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4278)
	; 1426 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4279)
	; 1427 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4282)
	; 1428 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4284)
	; 1428 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4285)
	; 1429 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4288)
	; 1430 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4290)
	; 1430 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4291)
	; 1431 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4294)
	; 1432 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4296)
	; 1432 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if(n = 4297)
	; 1433 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 4297 && n < 4335)))
	; 1433..1445 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4298 && n < 4336)))
	; 1434..1446 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4300)
	; 1434 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4302)
	; 1434 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4303)
	; 1435 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4306)
	; 1436 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4308)
	; 1436 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4309)
	; 1437 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4312)
	; 1438 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4314)
	; 1438 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4315)
	; 1439 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4318)
	; 1440 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4320)
	; 1440 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4321)
	; 1441 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4324)
	; 1442 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4326)
	; 1442 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4327)
	; 1443 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4330)
	; 1444 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4332)
	; 1444 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4333)
	; 1445 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 4336 to 4375
.if(n = 4336)
	; 1446 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 4336 && n < 4374)))
	; 1447..1459 bytes
	delay_n (n-2)
	nop
.elseif(n = 4338)
	; 1446 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4339)
	; 1447 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 4340 && n < 4372)))
	; 1448..1458 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4342)
	; 1448 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4344)
	; 1448 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4345)
	; 1449 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4348)
	; 1450 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4350)
	; 1450 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4351)
	; 1451 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4354)
	; 1452 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4356)
	; 1452 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4357)
	; 1453 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4360)
	; 1454 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4362)
	; 1454 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4363)
	; 1455 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4366)
	; 1456 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4368)
	; 1456 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4369)
	; 1457 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4372)
	; 1458 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4374)
	; 1458 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4375)
	; 1459 bytes
	delay_n 55
	delay_n (n-55)
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
.if((n .mod 3 = 2) && ((n > 4375 && n < 4413)))
	; 1459..1471 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4376 && n < 4414)))
	; 1460..1472 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4378)
	; 1460 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4380)
	; 1460 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4381)
	; 1461 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4384)
	; 1462 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4386)
	; 1462 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4387)
	; 1463 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4390)
	; 1464 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4392)
	; 1464 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4393)
	; 1465 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4396)
	; 1466 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4398)
	; 1466 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4399)
	; 1467 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4402)
	; 1468 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4404)
	; 1468 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4405)
	; 1469 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4408)
	; 1470 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4410)
	; 1470 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4411)
	; 1471 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4414)
	; 1472 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 4415 to 4453
.if((n .mod 3 = 2) && ((n > 4414 && n < 4452)))
	; 1473..1485 bytes
	delay_n (n-2)
	nop
.elseif(n = 4416)
	; 1472 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4417)
	; 1473 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 4418 && n < 4450)))
	; 1474..1484 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4420)
	; 1474 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4422)
	; 1474 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4423)
	; 1475 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4426)
	; 1476 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4428)
	; 1476 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4429)
	; 1477 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4432)
	; 1478 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4434)
	; 1478 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4435)
	; 1479 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4438)
	; 1480 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4440)
	; 1480 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4441)
	; 1481 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4444)
	; 1482 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4446)
	; 1482 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4447)
	; 1483 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4450)
	; 1484 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4452)
	; 1484 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4453)
	; 1485 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if((n .mod 3 = 2) && ((n > 4453 && n < 4491)))
	; 1485..1497 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4454 && n < 4492)))
	; 1486..1498 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4456)
	; 1486 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4458)
	; 1486 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4459)
	; 1487 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4462)
	; 1488 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4464)
	; 1488 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4465)
	; 1489 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4468)
	; 1490 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4470)
	; 1490 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4471)
	; 1491 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4474)
	; 1492 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4476)
	; 1492 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4477)
	; 1493 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4480)
	; 1494 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4482)
	; 1494 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4483)
	; 1495 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4486)
	; 1496 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4488)
	; 1496 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4489)
	; 1497 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4492)
	; 1498 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 4493 to 4531
.if((n .mod 3 = 2) && ((n > 4492 && n < 4530)))
	; 1499..1511 bytes
	delay_n (n-2)
	nop
.elseif(n = 4494)
	; 1498 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4495)
	; 1499 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 4496 && n < 4528)))
	; 1500..1510 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4498)
	; 1500 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4500)
	; 1500 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4501)
	; 1501 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4504)
	; 1502 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4506)
	; 1502 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4507)
	; 1503 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4510)
	; 1504 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4512)
	; 1504 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4513)
	; 1505 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4516)
	; 1506 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4518)
	; 1506 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4519)
	; 1507 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4522)
	; 1508 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4524)
	; 1508 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4525)
	; 1509 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4528)
	; 1510 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4530)
	; 1510 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4531)
	; 1511 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if((n .mod 3 = 2) && ((n > 4531 && n < 4569)))
	; 1511..1523 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4532 && n < 4570)))
	; 1512..1524 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4534)
	; 1512 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4536)
	; 1512 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4537)
	; 1513 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4540)
	; 1514 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4542)
	; 1514 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4543)
	; 1515 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4546)
	; 1516 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4548)
	; 1516 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4549)
	; 1517 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4552)
	; 1518 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4554)
	; 1518 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4555)
	; 1519 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4558)
	; 1520 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4560)
	; 1520 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4561)
	; 1521 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4564)
	; 1522 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4566)
	; 1522 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4567)
	; 1523 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4570)
	; 1524 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 4571 to 4609
.if((n .mod 3 = 2) && ((n > 4570 && n < 4608)))
	; 1525..1537 bytes
	delay_n (n-2)
	nop
.elseif(n = 4572)
	; 1524 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4573)
	; 1525 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 4574 && n < 4606)))
	; 1526..1536 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4576)
	; 1526 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4578)
	; 1526 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4579)
	; 1527 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4582)
	; 1528 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4584)
	; 1528 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4585)
	; 1529 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4588)
	; 1530 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4590)
	; 1530 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4591)
	; 1531 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4594)
	; 1532 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4596)
	; 1532 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4597)
	; 1533 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4600)
	; 1534 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4602)
	; 1534 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4603)
	; 1535 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4606)
	; 1536 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4608)
	; 1536 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4609)
	; 1537 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if((n .mod 3 = 2) && ((n > 4609 && n < 4647)))
	; 1537..1549 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4610 && n < 4648)))
	; 1538..1550 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4612)
	; 1538 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4614)
	; 1538 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4615)
	; 1539 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4618)
	; 1540 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4620)
	; 1540 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4621)
	; 1541 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4624)
	; 1542 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4626)
	; 1542 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4627)
	; 1543 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4630)
	; 1544 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4632)
	; 1544 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4633)
	; 1545 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4636)
	; 1546 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4638)
	; 1546 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4639)
	; 1547 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4642)
	; 1548 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4644)
	; 1548 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4645)
	; 1549 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4648)
	; 1550 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 4649 to 4687
.if((n .mod 3 = 2) && ((n > 4648 && n < 4686)))
	; 1551..1563 bytes
	delay_n (n-2)
	nop
.elseif(n = 4650)
	; 1550 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4651)
	; 1551 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 4652 && n < 4684)))
	; 1552..1562 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4654)
	; 1552 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4656)
	; 1552 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4657)
	; 1553 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4660)
	; 1554 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4662)
	; 1554 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4663)
	; 1555 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4666)
	; 1556 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4668)
	; 1556 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4669)
	; 1557 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4672)
	; 1558 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4674)
	; 1558 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4675)
	; 1559 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4678)
	; 1560 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4680)
	; 1560 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4681)
	; 1561 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4684)
	; 1562 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4686)
	; 1562 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4687)
	; 1563 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if((n .mod 3 = 2) && ((n > 4687 && n < 4725)))
	; 1563..1575 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4688 && n < 4726)))
	; 1564..1576 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4690)
	; 1564 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4692)
	; 1564 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4693)
	; 1565 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4696)
	; 1566 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4698)
	; 1566 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4699)
	; 1567 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4702)
	; 1568 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4704)
	; 1568 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4705)
	; 1569 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4708)
	; 1570 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4710)
	; 1570 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4711)
	; 1571 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4714)
	; 1572 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4716)
	; 1572 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4717)
	; 1573 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4720)
	; 1574 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4722)
	; 1574 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4723)
	; 1575 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4726)
	; 1576 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 4727 to 4765
.if((n .mod 3 = 2) && ((n > 4726 && n < 4764)))
	; 1577..1589 bytes
	delay_n (n-2)
	nop
.elseif(n = 4728)
	; 1576 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4729)
	; 1577 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 4730 && n < 4762)))
	; 1578..1588 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4732)
	; 1578 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4734)
	; 1578 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4735)
	; 1579 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4738)
	; 1580 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4740)
	; 1580 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4741)
	; 1581 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4744)
	; 1582 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4746)
	; 1582 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4747)
	; 1583 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4750)
	; 1584 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4752)
	; 1584 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4753)
	; 1585 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4756)
	; 1586 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4758)
	; 1586 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4759)
	; 1587 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4762)
	; 1588 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4764)
	; 1588 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4765)
	; 1589 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
; 4766 to 4804
.if((n .mod 3 = 2) && ((n > 4765 && n < 4803)))
	; 1589..1601 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4766 && n < 4804)))
	; 1590..1602 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4768)
	; 1590 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4770)
	; 1590 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4771)
	; 1591 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4774)
	; 1592 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4776)
	; 1592 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4777)
	; 1593 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4780)
	; 1594 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4782)
	; 1594 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4783)
	; 1595 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4786)
	; 1596 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4788)
	; 1596 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4789)
	; 1597 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4792)
	; 1598 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4794)
	; 1598 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4795)
	; 1599 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4798)
	; 1600 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4800)
	; 1600 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4801)
	; 1601 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4804)
	; 1602 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 4805 to 4843
.if((n .mod 3 = 2) && ((n > 4804 && n < 4842)))
	; 1603..1615 bytes
	delay_n (n-2)
	nop
.elseif(n = 4806)
	; 1602 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4807)
	; 1603 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 4808 && n < 4840)))
	; 1604..1614 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4810)
	; 1604 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4812)
	; 1604 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4813)
	; 1605 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4816)
	; 1606 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4818)
	; 1606 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4819)
	; 1607 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4822)
	; 1608 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4824)
	; 1608 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4825)
	; 1609 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4828)
	; 1610 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4830)
	; 1610 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4831)
	; 1611 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4834)
	; 1612 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4836)
	; 1612 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4837)
	; 1613 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4840)
	; 1614 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4842)
	; 1614 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4843)
	; 1615 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if((n .mod 3 = 2) && ((n > 4843 && n < 4881)))
	; 1615..1627 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4844 && n < 4882)))
	; 1616..1628 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4846)
	; 1616 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4848)
	; 1616 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4849)
	; 1617 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4852)
	; 1618 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4854)
	; 1618 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4855)
	; 1619 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4858)
	; 1620 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4860)
	; 1620 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4861)
	; 1621 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4864)
	; 1622 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4866)
	; 1622 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4867)
	; 1623 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4870)
	; 1624 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4872)
	; 1624 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4873)
	; 1625 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4876)
	; 1626 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4878)
	; 1626 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4879)
	; 1627 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4882)
	; 1628 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 4883 to 4921
.if((n .mod 3 = 2) && ((n > 4882 && n < 4920)))
	; 1629..1641 bytes
	delay_n (n-2)
	nop
.elseif(n = 4884)
	; 1628 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4885)
	; 1629 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 4886 && n < 4918)))
	; 1630..1640 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4888)
	; 1630 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4890)
	; 1630 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4891)
	; 1631 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4894)
	; 1632 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4896)
	; 1632 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4897)
	; 1633 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4900)
	; 1634 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4902)
	; 1634 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4903)
	; 1635 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4906)
	; 1636 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4908)
	; 1636 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4909)
	; 1637 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4912)
	; 1638 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4914)
	; 1638 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4915)
	; 1639 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4918)
	; 1640 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4920)
	; 1640 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4921)
	; 1641 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
; 4922 to 4960
.if((n .mod 3 = 2) && ((n > 4921 && n < 4959)))
	; 1641..1653 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 4922 && n < 4960)))
	; 1642..1654 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4924)
	; 1642 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4926)
	; 1642 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4927)
	; 1643 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4930)
	; 1644 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4932)
	; 1644 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4933)
	; 1645 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4936)
	; 1646 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4938)
	; 1646 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4939)
	; 1647 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4942)
	; 1648 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4944)
	; 1648 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4945)
	; 1649 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4948)
	; 1650 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4950)
	; 1650 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4951)
	; 1651 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4954)
	; 1652 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4956)
	; 1652 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4957)
	; 1653 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 4960)
	; 1654 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 4961 to 5000
.if((n .mod 3 = 2) && ((n > 4960 && n < 5001)))
	; 1655..1667 bytes
	delay_n (n-2)
	nop
.elseif(n = 4962)
	; 1654 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4963)
	; 1655 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 4964 && n < 4996)))
	; 1656..1666 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 4966)
	; 1656 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4968)
	; 1656 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4969)
	; 1657 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4972)
	; 1658 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4974)
	; 1658 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4975)
	; 1659 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 4978)
	; 1660 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4980)
	; 1660 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4981)
	; 1661 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4984)
	; 1662 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4986)
	; 1662 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4987)
	; 1663 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4990)
	; 1664 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4992)
	; 1664 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4993)
	; 1665 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4996)
	; 1666 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4998)
	; 1666 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4999)
	; 1667 bytes
	delay_n 31
	delay_n (n-31)
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
; 5001 to 5039
.if((n .mod 6 = 3) && ((n > 5000 && n < 5038)))
	; 1668..1680 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5002)
	; 1668 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 5002 && n < 5040)))
	; 1669..1681 bytes
	delay_n (n-2)
	nop
.elseif(n = 5004)
	; 1668 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5005)
	; 1669 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5008)
	; 1670 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5010)
	; 1670 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5011)
	; 1671 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5014)
	; 1672 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5016)
	; 1672 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5017)
	; 1673 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5020)
	; 1674 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5022)
	; 1674 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5023)
	; 1675 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5026)
	; 1676 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5028)
	; 1676 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5029)
	; 1677 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5032)
	; 1678 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5034)
	; 1678 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5035)
	; 1679 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5038)
	; 1680 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 5040 to 5078
.if(n = 5040)
	; 1680 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5041)
	; 1681 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 5041 && n < 5079)))
	; 1681..1693 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5042 && n < 5074)))
	; 1682..1692 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5044)
	; 1682 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5046)
	; 1682 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5047)
	; 1683 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5050)
	; 1684 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5052)
	; 1684 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5053)
	; 1685 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5056)
	; 1686 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5058)
	; 1686 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5059)
	; 1687 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5062)
	; 1688 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5064)
	; 1688 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5065)
	; 1689 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5068)
	; 1690 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5070)
	; 1690 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5071)
	; 1691 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5074)
	; 1692 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5076)
	; 1692 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5077)
	; 1693 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if((n .mod 6 = 3) && ((n > 5078 && n < 5116)))
	; 1694..1706 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5080)
	; 1694 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 5080 && n < 5118)))
	; 1695..1707 bytes
	delay_n (n-2)
	nop
.elseif(n = 5082)
	; 1694 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5083)
	; 1695 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5086)
	; 1696 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5088)
	; 1696 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5089)
	; 1697 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5092)
	; 1698 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5094)
	; 1698 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5095)
	; 1699 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5098)
	; 1700 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5100)
	; 1700 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5101)
	; 1701 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5104)
	; 1702 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5106)
	; 1702 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5107)
	; 1703 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5110)
	; 1704 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5112)
	; 1704 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5113)
	; 1705 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5116)
	; 1706 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 5118 to 5156
.if(n = 5118)
	; 1706 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5119)
	; 1707 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 5119 && n < 5157)))
	; 1707..1719 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5120 && n < 5152)))
	; 1708..1718 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5122)
	; 1708 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5124)
	; 1708 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5125)
	; 1709 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5128)
	; 1710 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5130)
	; 1710 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5131)
	; 1711 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5134)
	; 1712 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5136)
	; 1712 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5137)
	; 1713 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5140)
	; 1714 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5142)
	; 1714 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5143)
	; 1715 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5146)
	; 1716 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5148)
	; 1716 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5149)
	; 1717 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5152)
	; 1718 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5154)
	; 1718 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5155)
	; 1719 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
; 5157 to 5195
.if((n .mod 6 = 3) && ((n > 5156 && n < 5194)))
	; 1720..1732 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5158)
	; 1720 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 5158 && n < 5196)))
	; 1721..1733 bytes
	delay_n (n-2)
	nop
.elseif(n = 5160)
	; 1720 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5161)
	; 1721 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5164)
	; 1722 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5166)
	; 1722 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5167)
	; 1723 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5170)
	; 1724 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5172)
	; 1724 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5173)
	; 1725 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5176)
	; 1726 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5178)
	; 1726 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5179)
	; 1727 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5182)
	; 1728 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5184)
	; 1728 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5185)
	; 1729 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5188)
	; 1730 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5190)
	; 1730 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5191)
	; 1731 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5194)
	; 1732 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 5196 to 5234
.if(n = 5196)
	; 1732 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5197)
	; 1733 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 5197 && n < 5235)))
	; 1733..1745 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5198 && n < 5230)))
	; 1734..1744 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5200)
	; 1734 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5202)
	; 1734 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5203)
	; 1735 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5206)
	; 1736 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5208)
	; 1736 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5209)
	; 1737 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5212)
	; 1738 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5214)
	; 1738 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5215)
	; 1739 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5218)
	; 1740 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5220)
	; 1740 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5221)
	; 1741 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5224)
	; 1742 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5226)
	; 1742 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5227)
	; 1743 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5230)
	; 1744 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5232)
	; 1744 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5233)
	; 1745 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
; 5235 to 5273
.if((n .mod 6 = 3) && ((n > 5234 && n < 5272)))
	; 1746..1758 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5236)
	; 1746 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 5236 && n < 5274)))
	; 1747..1759 bytes
	delay_n (n-2)
	nop
.elseif(n = 5238)
	; 1746 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5239)
	; 1747 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5242)
	; 1748 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5244)
	; 1748 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5245)
	; 1749 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5248)
	; 1750 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5250)
	; 1750 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5251)
	; 1751 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5254)
	; 1752 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5256)
	; 1752 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5257)
	; 1753 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5260)
	; 1754 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5262)
	; 1754 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5263)
	; 1755 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5266)
	; 1756 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5268)
	; 1756 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5269)
	; 1757 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5272)
	; 1758 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 5274 to 5312
.if(n = 5274)
	; 1758 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5275)
	; 1759 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 5275 && n < 5313)))
	; 1759..1771 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5276 && n < 5308)))
	; 1760..1770 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5278)
	; 1760 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5280)
	; 1760 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5281)
	; 1761 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5284)
	; 1762 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5286)
	; 1762 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5287)
	; 1763 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5290)
	; 1764 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5292)
	; 1764 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5293)
	; 1765 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5296)
	; 1766 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5298)
	; 1766 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5299)
	; 1767 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5302)
	; 1768 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5304)
	; 1768 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5305)
	; 1769 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5308)
	; 1770 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5310)
	; 1770 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5311)
	; 1771 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
.if((n .mod 6 = 3) && ((n > 5312 && n < 5350)))
	; 1772..1784 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5314)
	; 1772 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 5314 && n < 5352)))
	; 1773..1785 bytes
	delay_n (n-2)
	nop
.elseif(n = 5316)
	; 1772 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5317)
	; 1773 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5320)
	; 1774 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5322)
	; 1774 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5323)
	; 1775 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5326)
	; 1776 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5328)
	; 1776 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5329)
	; 1777 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5332)
	; 1778 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5334)
	; 1778 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5335)
	; 1779 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5338)
	; 1780 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5340)
	; 1780 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5341)
	; 1781 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5344)
	; 1782 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5346)
	; 1782 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5347)
	; 1783 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5350)
	; 1784 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 5352 to 5390
.if(n = 5352)
	; 1784 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5353)
	; 1785 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 5353 && n < 5391)))
	; 1785..1797 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5354 && n < 5386)))
	; 1786..1796 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5356)
	; 1786 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5358)
	; 1786 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5359)
	; 1787 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5362)
	; 1788 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5364)
	; 1788 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5365)
	; 1789 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5368)
	; 1790 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5370)
	; 1790 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5371)
	; 1791 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5374)
	; 1792 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5376)
	; 1792 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5377)
	; 1793 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5380)
	; 1794 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5382)
	; 1794 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5383)
	; 1795 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5386)
	; 1796 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5388)
	; 1796 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5389)
	; 1797 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
; 5391 to 5429
.if((n .mod 6 = 3) && ((n > 5390 && n < 5428)))
	; 1798..1810 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5392)
	; 1798 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 5392 && n < 5430)))
	; 1799..1811 bytes
	delay_n (n-2)
	nop
.elseif(n = 5394)
	; 1798 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5395)
	; 1799 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5398)
	; 1800 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5400)
	; 1800 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5401)
	; 1801 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5404)
	; 1802 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5406)
	; 1802 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5407)
	; 1803 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5410)
	; 1804 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5412)
	; 1804 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5413)
	; 1805 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5416)
	; 1806 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5418)
	; 1806 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5419)
	; 1807 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5422)
	; 1808 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5424)
	; 1808 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5425)
	; 1809 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5428)
	; 1810 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 5430 to 5468
.if(n = 5430)
	; 1810 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5431)
	; 1811 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 5431 && n < 5469)))
	; 1811..1823 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5432 && n < 5464)))
	; 1812..1822 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5434)
	; 1812 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5436)
	; 1812 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5437)
	; 1813 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5440)
	; 1814 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5442)
	; 1814 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5443)
	; 1815 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5446)
	; 1816 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5448)
	; 1816 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5449)
	; 1817 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5452)
	; 1818 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5454)
	; 1818 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5455)
	; 1819 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5458)
	; 1820 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5460)
	; 1820 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5461)
	; 1821 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5464)
	; 1822 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5466)
	; 1822 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5467)
	; 1823 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
.if((n >= 5469) && (n < 5508))
; 5469 to 5507
.if((n .mod 6 = 3) && ((n > 5468 && n < 5506)))
	; 1824..1836 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5470)
	; 1824 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 5470 && n < 5508)))
	; 1825..1837 bytes
	delay_n (n-2)
	nop
.elseif(n = 5472)
	; 1824 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5473)
	; 1825 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5476)
	; 1826 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5478)
	; 1826 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5479)
	; 1827 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5482)
	; 1828 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5484)
	; 1828 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5485)
	; 1829 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5488)
	; 1830 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5490)
	; 1830 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5491)
	; 1831 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5494)
	; 1832 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5496)
	; 1832 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5497)
	; 1833 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5500)
	; 1834 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5502)
	; 1834 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5503)
	; 1835 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5506)
	; 1836 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 5508 to 5546
.if(n = 5508)
	; 1836 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5509)
	; 1837 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 5509 && n < 5547)))
	; 1837..1849 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5510 && n < 5542)))
	; 1838..1848 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5512)
	; 1838 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5514)
	; 1838 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5515)
	; 1839 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5518)
	; 1840 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5520)
	; 1840 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5521)
	; 1841 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5524)
	; 1842 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5526)
	; 1842 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5527)
	; 1843 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5530)
	; 1844 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5532)
	; 1844 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5533)
	; 1845 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5536)
	; 1846 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5538)
	; 1846 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5539)
	; 1847 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5542)
	; 1848 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5544)
	; 1848 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5545)
	; 1849 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 5547) && (n < 5586))
; 5547 to 5585
.if((n .mod 6 = 3) && ((n > 5546 && n < 5584)))
	; 1850..1862 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5548)
	; 1850 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 5548 && n < 5586)))
	; 1851..1863 bytes
	delay_n (n-2)
	nop
.elseif(n = 5550)
	; 1850 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5551)
	; 1851 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5554)
	; 1852 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5556)
	; 1852 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5557)
	; 1853 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5560)
	; 1854 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5562)
	; 1854 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5563)
	; 1855 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5566)
	; 1856 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5568)
	; 1856 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5569)
	; 1857 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5572)
	; 1858 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5574)
	; 1858 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5575)
	; 1859 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5578)
	; 1860 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5580)
	; 1860 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5581)
	; 1861 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5584)
	; 1862 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 5586 to 5625
.if(n = 5586)
	; 1862 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5587)
	; 1863 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 5587 && n < 5625)))
	; 1863..1875 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5588 && n < 5626)))
	; 1864..1876 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5590)
	; 1864 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5592)
	; 1864 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5593)
	; 1865 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5596)
	; 1866 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5598)
	; 1866 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5599)
	; 1867 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5602)
	; 1868 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5604)
	; 1868 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5605)
	; 1869 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5608)
	; 1870 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5610)
	; 1870 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5611)
	; 1871 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5614)
	; 1872 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5616)
	; 1872 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5617)
	; 1873 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5620)
	; 1874 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5622)
	; 1874 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5623)
	; 1875 bytes
	inc $0700,X
	delay_n (n-7)
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
; 5626 to 5664
.if(n = 5626)
	; 1876 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 5626 && n < 5664)))
	; 1877..1889 bytes
	delay_n (n-2)
	nop
.elseif(n = 5628)
	; 1876 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5629)
	; 1877 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 5630 && n < 5662)))
	; 1878..1888 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5632)
	; 1878 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5634)
	; 1878 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5635)
	; 1879 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5638)
	; 1880 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5640)
	; 1880 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5641)
	; 1881 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5644)
	; 1882 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5646)
	; 1882 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5647)
	; 1883 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5650)
	; 1884 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5652)
	; 1884 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5653)
	; 1885 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5656)
	; 1886 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5658)
	; 1886 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5659)
	; 1887 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5662)
	; 1888 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5664)
	; 1888 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 5665 to 5703
.if(n = 5665)
	; 1889 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 5665 && n < 5703)))
	; 1889..1901 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5666 && n < 5704)))
	; 1890..1902 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5668)
	; 1890 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5670)
	; 1890 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5671)
	; 1891 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5674)
	; 1892 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5676)
	; 1892 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5677)
	; 1893 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5680)
	; 1894 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5682)
	; 1894 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5683)
	; 1895 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5686)
	; 1896 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5688)
	; 1896 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5689)
	; 1897 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5692)
	; 1898 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5694)
	; 1898 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5695)
	; 1899 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5698)
	; 1900 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5700)
	; 1900 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5701)
	; 1901 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.else
.if((n >= 5704) && (n < 5743))
; 5704 to 5742
.if(n = 5704)
	; 1902 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 5704 && n < 5742)))
	; 1903..1915 bytes
	delay_n (n-2)
	nop
.elseif(n = 5706)
	; 1902 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5707)
	; 1903 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 5708 && n < 5740)))
	; 1904..1914 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5710)
	; 1904 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5712)
	; 1904 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5713)
	; 1905 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5716)
	; 1906 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5718)
	; 1906 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5719)
	; 1907 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5722)
	; 1908 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5724)
	; 1908 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5725)
	; 1909 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5728)
	; 1910 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5730)
	; 1910 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5731)
	; 1911 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5734)
	; 1912 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5736)
	; 1912 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5737)
	; 1913 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5740)
	; 1914 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5742)
	; 1914 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 5743 to 5781
.if(n = 5743)
	; 1915 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 5743 && n < 5781)))
	; 1915..1927 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5744 && n < 5782)))
	; 1916..1928 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5746)
	; 1916 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5748)
	; 1916 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5749)
	; 1917 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5752)
	; 1918 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5754)
	; 1918 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5755)
	; 1919 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5758)
	; 1920 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5760)
	; 1920 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5761)
	; 1921 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5764)
	; 1922 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5766)
	; 1922 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5767)
	; 1923 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5770)
	; 1924 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5772)
	; 1924 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5773)
	; 1925 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5776)
	; 1926 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5778)
	; 1926 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5779)
	; 1927 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
.if((n >= 5782) && (n < 5821))
; 5782 to 5820
.if(n = 5782)
	; 1928 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 5782 && n < 5820)))
	; 1929..1941 bytes
	delay_n (n-2)
	nop
.elseif(n = 5784)
	; 1928 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5785)
	; 1929 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 5786 && n < 5818)))
	; 1930..1940 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5788)
	; 1930 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5790)
	; 1930 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5791)
	; 1931 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5794)
	; 1932 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5796)
	; 1932 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5797)
	; 1933 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5800)
	; 1934 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5802)
	; 1934 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5803)
	; 1935 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5806)
	; 1936 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5808)
	; 1936 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5809)
	; 1937 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5812)
	; 1938 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5814)
	; 1938 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5815)
	; 1939 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5818)
	; 1940 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5820)
	; 1940 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 5821 to 5859
.if(n = 5821)
	; 1941 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 5821 && n < 5859)))
	; 1941..1953 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5822 && n < 5860)))
	; 1942..1954 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5824)
	; 1942 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5826)
	; 1942 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5827)
	; 1943 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5830)
	; 1944 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5832)
	; 1944 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5833)
	; 1945 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5836)
	; 1946 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5838)
	; 1946 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5839)
	; 1947 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5842)
	; 1948 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5844)
	; 1948 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5845)
	; 1949 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5848)
	; 1950 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5850)
	; 1950 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5851)
	; 1951 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5854)
	; 1952 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5856)
	; 1952 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5857)
	; 1953 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860)
	; 1954 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 5860 && n < 5898)))
	; 1955..1967 bytes
	delay_n (n-2)
	nop
.elseif(n = 5862)
	; 1954 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5863)
	; 1955 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 5864 && n < 5896)))
	; 1956..1966 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5866)
	; 1956 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5868)
	; 1956 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5869)
	; 1957 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5872)
	; 1958 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5874)
	; 1958 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5875)
	; 1959 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5878)
	; 1960 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5880)
	; 1960 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5881)
	; 1961 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5884)
	; 1962 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5886)
	; 1962 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5887)
	; 1963 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5890)
	; 1964 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5892)
	; 1964 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5893)
	; 1965 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5896)
	; 1966 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5898)
	; 1966 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 5899 to 5937
.if(n = 5899)
	; 1967 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 5899 && n < 5937)))
	; 1967..1979 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5900 && n < 5938)))
	; 1968..1980 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5902)
	; 1968 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5904)
	; 1968 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5905)
	; 1969 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5908)
	; 1970 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5910)
	; 1970 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5911)
	; 1971 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5914)
	; 1972 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5916)
	; 1972 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5917)
	; 1973 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5920)
	; 1974 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5922)
	; 1974 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5923)
	; 1975 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5926)
	; 1976 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5928)
	; 1976 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5929)
	; 1977 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5932)
	; 1978 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5934)
	; 1978 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5935)
	; 1979 bytes
	delay_n 103
	delay_n (n-103)
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
	; 1980 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 5938 && n < 5976)))
	; 1981..1993 bytes
	delay_n (n-2)
	nop
.elseif(n = 5940)
	; 1980 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5941)
	; 1981 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 5942 && n < 5974)))
	; 1982..1992 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5944)
	; 1982 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5946)
	; 1982 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5947)
	; 1983 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 5950)
	; 1984 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5952)
	; 1984 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5953)
	; 1985 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5956)
	; 1986 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5958)
	; 1986 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5959)
	; 1987 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5962)
	; 1988 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5964)
	; 1988 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5965)
	; 1989 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5968)
	; 1990 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5970)
	; 1990 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5971)
	; 1991 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 5974)
	; 1992 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5976)
	; 1992 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 5977 to 6015
.if(n = 5977)
	; 1993 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 5977 && n < 6015)))
	; 1993..2005 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 5978 && n < 6016)))
	; 1994..2006 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5980)
	; 1994 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5982)
	; 1994 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5983)
	; 1995 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 5986)
	; 1996 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5988)
	; 1996 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5989)
	; 1997 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5992)
	; 1998 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5994)
	; 1998 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5995)
	; 1999 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5998)
	; 2000 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6000)
	; 2000 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6001)
	; 2001 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6004)
	; 2002 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6006)
	; 2002 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6007)
	; 2003 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6010)
	; 2004 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6012)
	; 2004 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6013)
	; 2005 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
.if((n >= 6016) && (n < 6055))
; 6016 to 6054
.if(n = 6016)
	; 2006 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 6016 && n < 6054)))
	; 2007..2019 bytes
	delay_n (n-2)
	nop
.elseif(n = 6018)
	; 2006 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6019)
	; 2007 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 6020 && n < 6052)))
	; 2008..2018 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6022)
	; 2008 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6024)
	; 2008 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6025)
	; 2009 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6028)
	; 2010 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6030)
	; 2010 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6031)
	; 2011 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6034)
	; 2012 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6036)
	; 2012 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6037)
	; 2013 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6040)
	; 2014 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6042)
	; 2014 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6043)
	; 2015 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6046)
	; 2016 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6048)
	; 2016 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6049)
	; 2017 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6052)
	; 2018 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6054)
	; 2018 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 6055 to 6093
.if(n = 6055)
	; 2019 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 6055 && n < 6093)))
	; 2019..2031 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6056 && n < 6094)))
	; 2020..2032 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6058)
	; 2020 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6060)
	; 2020 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6061)
	; 2021 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6064)
	; 2022 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6066)
	; 2022 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6067)
	; 2023 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6070)
	; 2024 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6072)
	; 2024 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6073)
	; 2025 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6076)
	; 2026 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6078)
	; 2026 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6079)
	; 2027 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6082)
	; 2028 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6084)
	; 2028 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6085)
	; 2029 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6088)
	; 2030 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6090)
	; 2030 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6091)
	; 2031 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if(n = 6094)
	; 2032 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 6094 && n < 6132)))
	; 2033..2045 bytes
	delay_n (n-2)
	nop
.elseif(n = 6096)
	; 2032 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6097)
	; 2033 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 6098 && n < 6130)))
	; 2034..2044 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6100)
	; 2034 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6102)
	; 2034 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6103)
	; 2035 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6106)
	; 2036 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6108)
	; 2036 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6109)
	; 2037 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6112)
	; 2038 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6114)
	; 2038 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6115)
	; 2039 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6118)
	; 2040 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6120)
	; 2040 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6121)
	; 2041 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6124)
	; 2042 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6126)
	; 2042 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6127)
	; 2043 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6130)
	; 2044 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6132)
	; 2044 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 6133 to 6171
.if(n = 6133)
	; 2045 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 6133 && n < 6171)))
	; 2045..2057 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6134 && n < 6172)))
	; 2046..2058 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6136)
	; 2046 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6138)
	; 2046 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6139)
	; 2047 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6142)
	; 2048 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6144)
	; 2048 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6145)
	; 2049 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6148)
	; 2050 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6150)
	; 2050 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6151)
	; 2051 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6154)
	; 2052 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6156)
	; 2052 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6157)
	; 2053 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6160)
	; 2054 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6162)
	; 2054 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6163)
	; 2055 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6166)
	; 2056 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6168)
	; 2056 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6169)
	; 2057 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 6172) && (n < 6211))
; 6172 to 6210
.if(n = 6172)
	; 2058 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 6172 && n < 6210)))
	; 2059..2071 bytes
	delay_n (n-2)
	nop
.elseif(n = 6174)
	; 2058 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6175)
	; 2059 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 6176 && n < 6208)))
	; 2060..2070 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6178)
	; 2060 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6180)
	; 2060 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6181)
	; 2061 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6184)
	; 2062 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6186)
	; 2062 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6187)
	; 2063 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6190)
	; 2064 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6192)
	; 2064 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6193)
	; 2065 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6196)
	; 2066 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6198)
	; 2066 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6199)
	; 2067 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6202)
	; 2068 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6204)
	; 2068 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6205)
	; 2069 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6208)
	; 2070 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6210)
	; 2070 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 6211 to 6250
.if(n = 6211)
	; 2071 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 6211 && n < 6249)))
	; 2071..2083 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6212 && n < 6250)))
	; 2072..2084 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6214)
	; 2072 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6216)
	; 2072 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6217)
	; 2073 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6220)
	; 2074 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6222)
	; 2074 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6223)
	; 2075 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6226)
	; 2076 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6228)
	; 2076 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6229)
	; 2077 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6232)
	; 2078 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6234)
	; 2078 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6235)
	; 2079 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6238)
	; 2080 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6240)
	; 2080 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6241)
	; 2081 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6244)
	; 2082 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6246)
	; 2082 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6247)
	; 2083 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6250)
	; 2084 bytes
	delay_n 94
	delay_n (n-94)
.endif
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
.if((n .mod 3 = 2) && ((n > 6250 && n < 6288)))
	; 2085..2097 bytes
	delay_n (n-2)
	nop
.elseif(n = 6252)
	; 2084 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6253)
	; 2085 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 6254 && n < 6286)))
	; 2086..2096 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6256)
	; 2086 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6258)
	; 2086 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6259)
	; 2087 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6262)
	; 2088 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6264)
	; 2088 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6265)
	; 2089 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6268)
	; 2090 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6270)
	; 2090 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6271)
	; 2091 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6274)
	; 2092 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6276)
	; 2092 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6277)
	; 2093 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6280)
	; 2094 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6282)
	; 2094 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6283)
	; 2095 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6286)
	; 2096 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6288)
	; 2096 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6289)
	; 2097 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 6290 to 6328
.if((n .mod 3 = 2) && ((n > 6289 && n < 6327)))
	; 2097..2109 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6290 && n < 6328)))
	; 2098..2110 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6292)
	; 2098 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6294)
	; 2098 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6295)
	; 2099 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6298)
	; 2100 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6300)
	; 2100 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6301)
	; 2101 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6304)
	; 2102 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6306)
	; 2102 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6307)
	; 2103 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6310)
	; 2104 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6312)
	; 2104 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6313)
	; 2105 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6316)
	; 2106 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6318)
	; 2106 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6319)
	; 2107 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6322)
	; 2108 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6324)
	; 2108 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6325)
	; 2109 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6328)
	; 2110 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if((n .mod 3 = 2) && ((n > 6328 && n < 6366)))
	; 2111..2123 bytes
	delay_n (n-2)
	nop
.elseif(n = 6330)
	; 2110 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6331)
	; 2111 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 6332 && n < 6364)))
	; 2112..2122 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6334)
	; 2112 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6336)
	; 2112 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6337)
	; 2113 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6340)
	; 2114 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6342)
	; 2114 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6343)
	; 2115 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6346)
	; 2116 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6348)
	; 2116 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6349)
	; 2117 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6352)
	; 2118 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6354)
	; 2118 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6355)
	; 2119 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6358)
	; 2120 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6360)
	; 2120 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6361)
	; 2121 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6364)
	; 2122 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6366)
	; 2122 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6367)
	; 2123 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 6368 to 6406
.if((n .mod 3 = 2) && ((n > 6367 && n < 6405)))
	; 2123..2135 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6368 && n < 6406)))
	; 2124..2136 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6370)
	; 2124 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6372)
	; 2124 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6373)
	; 2125 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6376)
	; 2126 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6378)
	; 2126 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6379)
	; 2127 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6382)
	; 2128 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6384)
	; 2128 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6385)
	; 2129 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6388)
	; 2130 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6390)
	; 2130 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6391)
	; 2131 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6394)
	; 2132 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6396)
	; 2132 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6397)
	; 2133 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6400)
	; 2134 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6402)
	; 2134 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6403)
	; 2135 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6406)
	; 2136 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
; 6407 to 6445
.if((n .mod 3 = 2) && ((n > 6406 && n < 6444)))
	; 2137..2149 bytes
	delay_n (n-2)
	nop
.elseif(n = 6408)
	; 2136 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6409)
	; 2137 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 6410 && n < 6442)))
	; 2138..2148 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6412)
	; 2138 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6414)
	; 2138 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6415)
	; 2139 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6418)
	; 2140 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6420)
	; 2140 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6421)
	; 2141 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6424)
	; 2142 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6426)
	; 2142 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6427)
	; 2143 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6430)
	; 2144 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6432)
	; 2144 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6433)
	; 2145 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6436)
	; 2146 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6438)
	; 2146 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6439)
	; 2147 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6442)
	; 2148 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6444)
	; 2148 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6445)
	; 2149 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 6446 to 6484
.if((n .mod 3 = 2) && ((n > 6445 && n < 6483)))
	; 2149..2161 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6446 && n < 6484)))
	; 2150..2162 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6448)
	; 2150 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6450)
	; 2150 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6451)
	; 2151 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6454)
	; 2152 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6456)
	; 2152 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6457)
	; 2153 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6460)
	; 2154 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6462)
	; 2154 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6463)
	; 2155 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6466)
	; 2156 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6468)
	; 2156 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6469)
	; 2157 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6472)
	; 2158 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6474)
	; 2158 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6475)
	; 2159 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6478)
	; 2160 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6480)
	; 2160 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6481)
	; 2161 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6484)
	; 2162 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 6485) && (n < 6524))
; 6485 to 6523
.if((n .mod 3 = 2) && ((n > 6484 && n < 6522)))
	; 2163..2175 bytes
	delay_n (n-2)
	nop
.elseif(n = 6486)
	; 2162 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6487)
	; 2163 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 6488 && n < 6520)))
	; 2164..2174 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6490)
	; 2164 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6492)
	; 2164 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6493)
	; 2165 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6496)
	; 2166 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6498)
	; 2166 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6499)
	; 2167 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6502)
	; 2168 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6504)
	; 2168 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6505)
	; 2169 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6508)
	; 2170 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6510)
	; 2170 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6511)
	; 2171 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6514)
	; 2172 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6516)
	; 2172 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6517)
	; 2173 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6520)
	; 2174 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6522)
	; 2174 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6523)
	; 2175 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 6524 to 6562
.if((n .mod 3 = 2) && ((n > 6523 && n < 6561)))
	; 2175..2187 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6524 && n < 6562)))
	; 2176..2188 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6526)
	; 2176 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6528)
	; 2176 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6529)
	; 2177 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6532)
	; 2178 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6534)
	; 2178 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6535)
	; 2179 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6538)
	; 2180 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6540)
	; 2180 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6541)
	; 2181 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6544)
	; 2182 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6546)
	; 2182 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6547)
	; 2183 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6550)
	; 2184 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6552)
	; 2184 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6553)
	; 2185 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6556)
	; 2186 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6558)
	; 2186 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6559)
	; 2187 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6562)
	; 2188 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
.if((n >= 6563) && (n < 6602))
; 6563 to 6601
.if((n .mod 3 = 2) && ((n > 6562 && n < 6600)))
	; 2189..2201 bytes
	delay_n (n-2)
	nop
.elseif(n = 6564)
	; 2188 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6565)
	; 2189 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 6566 && n < 6598)))
	; 2190..2200 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6568)
	; 2190 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6570)
	; 2190 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6571)
	; 2191 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6574)
	; 2192 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6576)
	; 2192 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6577)
	; 2193 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6580)
	; 2194 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6582)
	; 2194 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6583)
	; 2195 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6586)
	; 2196 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6588)
	; 2196 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6589)
	; 2197 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6592)
	; 2198 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6594)
	; 2198 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6595)
	; 2199 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6598)
	; 2200 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6600)
	; 2200 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6601)
	; 2201 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 6602 to 6640
.if((n .mod 3 = 2) && ((n > 6601 && n < 6639)))
	; 2201..2213 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6602 && n < 6640)))
	; 2202..2214 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6604)
	; 2202 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6606)
	; 2202 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6607)
	; 2203 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6610)
	; 2204 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6612)
	; 2204 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6613)
	; 2205 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6616)
	; 2206 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6618)
	; 2206 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6619)
	; 2207 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6622)
	; 2208 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6624)
	; 2208 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6625)
	; 2209 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6628)
	; 2210 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6630)
	; 2210 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6631)
	; 2211 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6634)
	; 2212 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6636)
	; 2212 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6637)
	; 2213 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6640)
	; 2214 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 6641) && (n < 6680))
; 6641 to 6679
.if((n .mod 3 = 2) && ((n > 6640 && n < 6678)))
	; 2215..2227 bytes
	delay_n (n-2)
	nop
.elseif(n = 6642)
	; 2214 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6643)
	; 2215 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 6644 && n < 6676)))
	; 2216..2226 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6646)
	; 2216 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6648)
	; 2216 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6649)
	; 2217 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6652)
	; 2218 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6654)
	; 2218 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6655)
	; 2219 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6658)
	; 2220 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6660)
	; 2220 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6661)
	; 2221 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6664)
	; 2222 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6666)
	; 2222 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6667)
	; 2223 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6670)
	; 2224 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6672)
	; 2224 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6673)
	; 2225 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6676)
	; 2226 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6678)
	; 2226 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6679)
	; 2227 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 6680 to 6718
.if((n .mod 3 = 2) && ((n > 6679 && n < 6717)))
	; 2227..2239 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6680 && n < 6718)))
	; 2228..2240 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6682)
	; 2228 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6684)
	; 2228 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6685)
	; 2229 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6688)
	; 2230 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6690)
	; 2230 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6691)
	; 2231 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6694)
	; 2232 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6696)
	; 2232 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6697)
	; 2233 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6700)
	; 2234 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6702)
	; 2234 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6703)
	; 2235 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6706)
	; 2236 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6708)
	; 2236 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6709)
	; 2237 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6712)
	; 2238 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6714)
	; 2238 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6715)
	; 2239 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6718)
	; 2240 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.else
.if((n >= 6719) && (n < 6797))
.if((n >= 6719) && (n < 6758))
; 6719 to 6757
.if((n .mod 3 = 2) && ((n > 6718 && n < 6756)))
	; 2241..2253 bytes
	delay_n (n-2)
	nop
.elseif(n = 6720)
	; 2240 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6721)
	; 2241 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 6722 && n < 6754)))
	; 2242..2252 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6724)
	; 2242 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6726)
	; 2242 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6727)
	; 2243 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6730)
	; 2244 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6732)
	; 2244 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6733)
	; 2245 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6736)
	; 2246 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6738)
	; 2246 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6739)
	; 2247 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6742)
	; 2248 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6744)
	; 2248 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6745)
	; 2249 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6748)
	; 2250 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6750)
	; 2250 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6751)
	; 2251 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6754)
	; 2252 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6756)
	; 2252 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6757)
	; 2253 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 6758 to 6796
.if((n .mod 3 = 2) && ((n > 6757 && n < 6795)))
	; 2253..2265 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6758 && n < 6796)))
	; 2254..2266 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6760)
	; 2254 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6762)
	; 2254 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6763)
	; 2255 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6766)
	; 2256 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6768)
	; 2256 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6769)
	; 2257 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6772)
	; 2258 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6774)
	; 2258 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6775)
	; 2259 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6778)
	; 2260 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6780)
	; 2260 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6781)
	; 2261 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6784)
	; 2262 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6786)
	; 2262 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6787)
	; 2263 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6790)
	; 2264 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6792)
	; 2264 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6793)
	; 2265 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6796)
	; 2266 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 6797) && (n < 6836))
; 6797 to 6835
.if((n .mod 3 = 2) && ((n > 6796 && n < 6834)))
	; 2267..2279 bytes
	delay_n (n-2)
	nop
.elseif(n = 6798)
	; 2266 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6799)
	; 2267 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 6800 && n < 6832)))
	; 2268..2278 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6802)
	; 2268 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6804)
	; 2268 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6805)
	; 2269 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6808)
	; 2270 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6810)
	; 2270 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6811)
	; 2271 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6814)
	; 2272 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6816)
	; 2272 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6817)
	; 2273 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6820)
	; 2274 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6822)
	; 2274 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6823)
	; 2275 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6826)
	; 2276 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6828)
	; 2276 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6829)
	; 2277 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6832)
	; 2278 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6834)
	; 2278 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6835)
	; 2279 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 6836 to 6875
.if((n .mod 3 = 2) && ((n > 6835 && n < 6876)))
	; 2279..2293 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6836 && n < 6874)))
	; 2280..2292 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6838)
	; 2280 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6840)
	; 2280 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6841)
	; 2281 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6844)
	; 2282 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6846)
	; 2282 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6847)
	; 2283 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6850)
	; 2284 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6852)
	; 2284 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6853)
	; 2285 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6856)
	; 2286 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6858)
	; 2286 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6859)
	; 2287 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6862)
	; 2288 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6864)
	; 2288 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6865)
	; 2289 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6868)
	; 2290 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6870)
	; 2290 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6871)
	; 2291 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6874)
	; 2292 bytes
	delay_n 70
	delay_n (n-70)
.endif
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
	; 2292 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6877)
	; 2293 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 6877 && n < 6915)))
	; 2293..2305 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6878 && n < 6910)))
	; 2294..2304 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6880)
	; 2294 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6882)
	; 2294 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6883)
	; 2295 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6886)
	; 2296 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6888)
	; 2296 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6889)
	; 2297 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6892)
	; 2298 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6894)
	; 2298 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6895)
	; 2299 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6898)
	; 2300 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6900)
	; 2300 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6901)
	; 2301 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 6904)
	; 2302 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6906)
	; 2302 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6907)
	; 2303 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6910)
	; 2304 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6912)
	; 2304 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6913)
	; 2305 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 6915 to 6953
.if((n .mod 6 = 3) && ((n > 6914 && n < 6952)))
	; 2306..2318 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6916)
	; 2306 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 6916 && n < 6954)))
	; 2307..2319 bytes
	delay_n (n-2)
	nop
.elseif(n = 6918)
	; 2306 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6919)
	; 2307 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6922)
	; 2308 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6924)
	; 2308 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6925)
	; 2309 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6928)
	; 2310 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6930)
	; 2310 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6931)
	; 2311 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6934)
	; 2312 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6936)
	; 2312 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6937)
	; 2313 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6940)
	; 2314 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6942)
	; 2314 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6943)
	; 2315 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 6946)
	; 2316 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6948)
	; 2316 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6949)
	; 2317 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6952)
	; 2318 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 6954) && (n < 6993))
; 6954 to 6992
.if(n = 6954)
	; 2318 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6955)
	; 2319 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 6955 && n < 6993)))
	; 2319..2331 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 6956 && n < 6988)))
	; 2320..2330 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6958)
	; 2320 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6960)
	; 2320 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6961)
	; 2321 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6964)
	; 2322 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6966)
	; 2322 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6967)
	; 2323 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6970)
	; 2324 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6972)
	; 2324 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6973)
	; 2325 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 6976)
	; 2326 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6978)
	; 2326 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6979)
	; 2327 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6982)
	; 2328 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6984)
	; 2328 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6985)
	; 2329 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6988)
	; 2330 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6990)
	; 2330 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6991)
	; 2331 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 6993 to 7031
.if((n .mod 6 = 3) && ((n > 6992 && n < 7030)))
	; 2332..2344 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 6994)
	; 2332 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 6994 && n < 7032)))
	; 2333..2345 bytes
	delay_n (n-2)
	nop
.elseif(n = 6996)
	; 2332 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6997)
	; 2333 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7000)
	; 2334 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7002)
	; 2334 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7003)
	; 2335 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7006)
	; 2336 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7008)
	; 2336 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7009)
	; 2337 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7012)
	; 2338 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7014)
	; 2338 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7015)
	; 2339 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7018)
	; 2340 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7020)
	; 2340 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7021)
	; 2341 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7024)
	; 2342 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7026)
	; 2342 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7027)
	; 2343 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7030)
	; 2344 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
.if((n >= 7032) && (n < 7071))
; 7032 to 7070
.if(n = 7032)
	; 2344 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7033)
	; 2345 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 7033 && n < 7071)))
	; 2345..2357 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7034 && n < 7066)))
	; 2346..2356 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7036)
	; 2346 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7038)
	; 2346 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7039)
	; 2347 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7042)
	; 2348 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7044)
	; 2348 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7045)
	; 2349 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7048)
	; 2350 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7050)
	; 2350 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7051)
	; 2351 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7054)
	; 2352 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7056)
	; 2352 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7057)
	; 2353 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7060)
	; 2354 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7062)
	; 2354 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7063)
	; 2355 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7066)
	; 2356 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7068)
	; 2356 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7069)
	; 2357 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 7071 to 7109
.if((n .mod 6 = 3) && ((n > 7070 && n < 7108)))
	; 2358..2370 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7072)
	; 2358 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 7072 && n < 7110)))
	; 2359..2371 bytes
	delay_n (n-2)
	nop
.elseif(n = 7074)
	; 2358 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7075)
	; 2359 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7078)
	; 2360 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7080)
	; 2360 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7081)
	; 2361 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7084)
	; 2362 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7086)
	; 2362 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7087)
	; 2363 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7090)
	; 2364 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7092)
	; 2364 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7093)
	; 2365 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7096)
	; 2366 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7098)
	; 2366 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7099)
	; 2367 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7102)
	; 2368 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7104)
	; 2368 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7105)
	; 2369 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7108)
	; 2370 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if(n = 7110)
	; 2370 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7111)
	; 2371 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 7111 && n < 7149)))
	; 2371..2383 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7112 && n < 7144)))
	; 2372..2382 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7114)
	; 2372 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7116)
	; 2372 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7117)
	; 2373 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7120)
	; 2374 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7122)
	; 2374 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7123)
	; 2375 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7126)
	; 2376 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7128)
	; 2376 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7129)
	; 2377 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7132)
	; 2378 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7134)
	; 2378 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7135)
	; 2379 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7138)
	; 2380 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7140)
	; 2380 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7141)
	; 2381 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7144)
	; 2382 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7146)
	; 2382 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7147)
	; 2383 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 7149 to 7187
.if((n .mod 6 = 3) && ((n > 7148 && n < 7186)))
	; 2384..2396 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7150)
	; 2384 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 7150 && n < 7188)))
	; 2385..2397 bytes
	delay_n (n-2)
	nop
.elseif(n = 7152)
	; 2384 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7153)
	; 2385 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7156)
	; 2386 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7158)
	; 2386 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7159)
	; 2387 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7162)
	; 2388 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7164)
	; 2388 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7165)
	; 2389 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7168)
	; 2390 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7170)
	; 2390 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7171)
	; 2391 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7174)
	; 2392 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7176)
	; 2392 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7177)
	; 2393 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7180)
	; 2394 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7182)
	; 2394 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7183)
	; 2395 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7186)
	; 2396 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
.if((n >= 7188) && (n < 7266))
.if((n >= 7188) && (n < 7227))
; 7188 to 7226
.if(n = 7188)
	; 2396 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7189)
	; 2397 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 7189 && n < 7227)))
	; 2397..2409 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7190 && n < 7222)))
	; 2398..2408 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7192)
	; 2398 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7194)
	; 2398 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7195)
	; 2399 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7198)
	; 2400 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7200)
	; 2400 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7201)
	; 2401 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7204)
	; 2402 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7206)
	; 2402 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7207)
	; 2403 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7210)
	; 2404 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7212)
	; 2404 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7213)
	; 2405 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7216)
	; 2406 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7218)
	; 2406 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7219)
	; 2407 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7222)
	; 2408 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7224)
	; 2408 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7225)
	; 2409 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 7227 to 7265
.if((n .mod 6 = 3) && ((n > 7226 && n < 7264)))
	; 2410..2422 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7228)
	; 2410 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 7228 && n < 7266)))
	; 2411..2423 bytes
	delay_n (n-2)
	nop
.elseif(n = 7230)
	; 2410 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7231)
	; 2411 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7234)
	; 2412 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7236)
	; 2412 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7237)
	; 2413 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7240)
	; 2414 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7242)
	; 2414 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7243)
	; 2415 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7246)
	; 2416 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7248)
	; 2416 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7249)
	; 2417 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7252)
	; 2418 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7254)
	; 2418 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7255)
	; 2419 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7258)
	; 2420 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7260)
	; 2420 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7261)
	; 2421 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7264)
	; 2422 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 7266) && (n < 7305))
; 7266 to 7304
.if(n = 7266)
	; 2422 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7267)
	; 2423 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 7267 && n < 7305)))
	; 2423..2435 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7268 && n < 7300)))
	; 2424..2434 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7270)
	; 2424 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7272)
	; 2424 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7273)
	; 2425 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7276)
	; 2426 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7278)
	; 2426 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7279)
	; 2427 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7282)
	; 2428 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7284)
	; 2428 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7285)
	; 2429 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7288)
	; 2430 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7290)
	; 2430 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7291)
	; 2431 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7294)
	; 2432 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7296)
	; 2432 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7297)
	; 2433 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7300)
	; 2434 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7302)
	; 2434 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7303)
	; 2435 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 7305 to 7343
.if((n .mod 6 = 3) && ((n > 7304 && n < 7342)))
	; 2436..2448 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7306)
	; 2436 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 7306 && n < 7344)))
	; 2437..2449 bytes
	delay_n (n-2)
	nop
.elseif(n = 7308)
	; 2436 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7309)
	; 2437 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7312)
	; 2438 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7314)
	; 2438 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7315)
	; 2439 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7318)
	; 2440 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7320)
	; 2440 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7321)
	; 2441 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7324)
	; 2442 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7326)
	; 2442 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7327)
	; 2443 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7330)
	; 2444 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7332)
	; 2444 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7333)
	; 2445 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7336)
	; 2446 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7338)
	; 2446 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7339)
	; 2447 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7342)
	; 2448 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 7344) && (n < 7422))
.if((n >= 7344) && (n < 7383))
; 7344 to 7382
.if(n = 7344)
	; 2448 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7345)
	; 2449 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 7345 && n < 7383)))
	; 2449..2461 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7346 && n < 7378)))
	; 2450..2460 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7348)
	; 2450 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7350)
	; 2450 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7351)
	; 2451 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7354)
	; 2452 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7356)
	; 2452 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7357)
	; 2453 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7360)
	; 2454 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7362)
	; 2454 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7363)
	; 2455 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7366)
	; 2456 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7368)
	; 2456 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7369)
	; 2457 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7372)
	; 2458 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7374)
	; 2458 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7375)
	; 2459 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7378)
	; 2460 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7380)
	; 2460 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7381)
	; 2461 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 7383 to 7421
.if((n .mod 6 = 3) && ((n > 7382 && n < 7420)))
	; 2462..2474 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7384)
	; 2462 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 7384 && n < 7422)))
	; 2463..2475 bytes
	delay_n (n-2)
	nop
.elseif(n = 7386)
	; 2462 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7387)
	; 2463 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7390)
	; 2464 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7392)
	; 2464 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7393)
	; 2465 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7396)
	; 2466 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7398)
	; 2466 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7399)
	; 2467 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7402)
	; 2468 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7404)
	; 2468 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7405)
	; 2469 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7408)
	; 2470 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7410)
	; 2470 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7411)
	; 2471 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7414)
	; 2472 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7416)
	; 2472 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7417)
	; 2473 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7420)
	; 2474 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 7422) && (n < 7461))
; 7422 to 7460
.if(n = 7422)
	; 2474 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7423)
	; 2475 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 7423 && n < 7461)))
	; 2475..2487 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7424 && n < 7456)))
	; 2476..2486 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7426)
	; 2476 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7428)
	; 2476 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7429)
	; 2477 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7432)
	; 2478 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7434)
	; 2478 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7435)
	; 2479 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7438)
	; 2480 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7440)
	; 2480 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7441)
	; 2481 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7444)
	; 2482 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7446)
	; 2482 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7447)
	; 2483 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7450)
	; 2484 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7452)
	; 2484 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7453)
	; 2485 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7456)
	; 2486 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7458)
	; 2486 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7459)
	; 2487 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 7461 to 7500
.if((n .mod 6 = 3) && ((n > 7460 && n < 7498)))
	; 2488..2500 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7462)
	; 2488 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 7462 && n < 7500)))
	; 2489..2501 bytes
	delay_n (n-2)
	nop
.elseif(n = 7464)
	; 2488 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7465)
	; 2489 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7468)
	; 2490 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7470)
	; 2490 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7471)
	; 2491 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7474)
	; 2492 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7476)
	; 2492 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7477)
	; 2493 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7480)
	; 2494 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7482)
	; 2494 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7483)
	; 2495 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7486)
	; 2496 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7488)
	; 2496 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7489)
	; 2497 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7492)
	; 2498 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7494)
	; 2498 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7495)
	; 2499 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7498)
	; 2500 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7500)
	; 2500 bytes
	delay_n 48
	delay_n (n-48)
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
.if((n >= 7501) && (n < 7540))
; 7501 to 7539
.if(n = 7501)
	; 2501 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 7501 && n < 7539)))
	; 2501..2513 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7502 && n < 7540)))
	; 2502..2514 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7504)
	; 2502 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7506)
	; 2502 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7507)
	; 2503 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7510)
	; 2504 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7512)
	; 2504 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7513)
	; 2505 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7516)
	; 2506 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7518)
	; 2506 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7519)
	; 2507 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7522)
	; 2508 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7524)
	; 2508 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7525)
	; 2509 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7528)
	; 2510 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7530)
	; 2510 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7531)
	; 2511 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7534)
	; 2512 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7536)
	; 2512 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7537)
	; 2513 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 7540 to 7578
.if(n = 7540)
	; 2514 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 7540 && n < 7578)))
	; 2515..2527 bytes
	delay_n (n-2)
	nop
.elseif(n = 7542)
	; 2514 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7543)
	; 2515 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 7544 && n < 7576)))
	; 2516..2526 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7546)
	; 2516 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7548)
	; 2516 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7549)
	; 2517 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7552)
	; 2518 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7554)
	; 2518 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7555)
	; 2519 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7558)
	; 2520 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7560)
	; 2520 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7561)
	; 2521 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7564)
	; 2522 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7566)
	; 2522 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7567)
	; 2523 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7570)
	; 2524 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7572)
	; 2524 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7573)
	; 2525 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7576)
	; 2526 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7578)
	; 2526 bytes
	delay_n 72
	delay_n (n-72)
.endif
.endif
.else
.if((n >= 7579) && (n < 7618))
; 7579 to 7617
.if(n = 7579)
	; 2527 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 7579 && n < 7617)))
	; 2527..2539 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7580 && n < 7618)))
	; 2528..2540 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7582)
	; 2528 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7584)
	; 2528 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7585)
	; 2529 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7588)
	; 2530 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7590)
	; 2530 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7591)
	; 2531 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7594)
	; 2532 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7596)
	; 2532 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7597)
	; 2533 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7600)
	; 2534 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7602)
	; 2534 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7603)
	; 2535 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7606)
	; 2536 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7608)
	; 2536 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7609)
	; 2537 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7612)
	; 2538 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7614)
	; 2538 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7615)
	; 2539 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 7618 to 7656
.if(n = 7618)
	; 2540 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 7618 && n < 7656)))
	; 2541..2553 bytes
	delay_n (n-2)
	nop
.elseif(n = 7620)
	; 2540 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7621)
	; 2541 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 7622 && n < 7654)))
	; 2542..2552 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7624)
	; 2542 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7626)
	; 2542 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7627)
	; 2543 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7630)
	; 2544 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7632)
	; 2544 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7633)
	; 2545 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7636)
	; 2546 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7638)
	; 2546 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7639)
	; 2547 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7642)
	; 2548 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7644)
	; 2548 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7645)
	; 2549 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7648)
	; 2550 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7650)
	; 2550 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7651)
	; 2551 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7654)
	; 2552 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7656)
	; 2552 bytes
	delay_n 96
	delay_n (n-96)
.endif
.endif
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
; 7657 to 7695
.if(n = 7657)
	; 2553 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 7657 && n < 7695)))
	; 2553..2565 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7658 && n < 7696)))
	; 2554..2566 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7660)
	; 2554 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7662)
	; 2554 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7663)
	; 2555 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7666)
	; 2556 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7668)
	; 2556 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7669)
	; 2557 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7672)
	; 2558 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7674)
	; 2558 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7675)
	; 2559 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7678)
	; 2560 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7680)
	; 2560 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7681)
	; 2561 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7684)
	; 2562 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7686)
	; 2562 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7687)
	; 2563 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7690)
	; 2564 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7692)
	; 2564 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7693)
	; 2565 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 7696 to 7734
.if(n = 7696)
	; 2566 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 7696 && n < 7734)))
	; 2567..2579 bytes
	delay_n (n-2)
	nop
.elseif(n = 7698)
	; 2566 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7699)
	; 2567 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 7700 && n < 7732)))
	; 2568..2578 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7702)
	; 2568 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7704)
	; 2568 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7705)
	; 2569 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7708)
	; 2570 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7710)
	; 2570 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7711)
	; 2571 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7714)
	; 2572 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7716)
	; 2572 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7717)
	; 2573 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7720)
	; 2574 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7722)
	; 2574 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7723)
	; 2575 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7726)
	; 2576 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7728)
	; 2576 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7729)
	; 2577 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7732)
	; 2578 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7734)
	; 2578 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 7735) && (n < 7774))
; 7735 to 7773
.if(n = 7735)
	; 2579 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 7735 && n < 7773)))
	; 2579..2591 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7736 && n < 7774)))
	; 2580..2592 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7738)
	; 2580 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7740)
	; 2580 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7741)
	; 2581 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7744)
	; 2582 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7746)
	; 2582 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7747)
	; 2583 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7750)
	; 2584 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7752)
	; 2584 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7753)
	; 2585 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7756)
	; 2586 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7758)
	; 2586 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7759)
	; 2587 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7762)
	; 2588 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7764)
	; 2588 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7765)
	; 2589 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7768)
	; 2590 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7770)
	; 2590 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7771)
	; 2591 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 7774 to 7812
.if(n = 7774)
	; 2592 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 7774 && n < 7812)))
	; 2593..2605 bytes
	delay_n (n-2)
	nop
.elseif(n = 7776)
	; 2592 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7777)
	; 2593 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 7778 && n < 7810)))
	; 2594..2604 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7780)
	; 2594 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7782)
	; 2594 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7783)
	; 2595 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7786)
	; 2596 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7788)
	; 2596 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7789)
	; 2597 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7792)
	; 2598 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7794)
	; 2598 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7795)
	; 2599 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7798)
	; 2600 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7800)
	; 2600 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7801)
	; 2601 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7804)
	; 2602 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7806)
	; 2602 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7807)
	; 2603 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7810)
	; 2604 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7812)
	; 2604 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
.if((n >= 7813) && (n < 7852))
; 7813 to 7851
.if(n = 7813)
	; 2605 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 7813 && n < 7851)))
	; 2605..2617 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7814 && n < 7852)))
	; 2606..2618 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7816)
	; 2606 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7818)
	; 2606 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7819)
	; 2607 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7822)
	; 2608 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7824)
	; 2608 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7825)
	; 2609 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7828)
	; 2610 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7830)
	; 2610 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7831)
	; 2611 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7834)
	; 2612 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7836)
	; 2612 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7837)
	; 2613 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7840)
	; 2614 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7842)
	; 2614 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7843)
	; 2615 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7846)
	; 2616 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7848)
	; 2616 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7849)
	; 2617 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 7852 to 7890
.if(n = 7852)
	; 2618 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 7852 && n < 7890)))
	; 2619..2631 bytes
	delay_n (n-2)
	nop
.elseif(n = 7854)
	; 2618 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7855)
	; 2619 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 7856 && n < 7888)))
	; 2620..2630 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7858)
	; 2620 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7860)
	; 2620 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7861)
	; 2621 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7864)
	; 2622 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7866)
	; 2622 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7867)
	; 2623 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7870)
	; 2624 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7872)
	; 2624 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7873)
	; 2625 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7876)
	; 2626 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7878)
	; 2626 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7879)
	; 2627 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7882)
	; 2628 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7884)
	; 2628 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7885)
	; 2629 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7888)
	; 2630 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7890)
	; 2630 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if(n = 7891)
	; 2631 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 7891 && n < 7929)))
	; 2631..2643 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7892 && n < 7930)))
	; 2632..2644 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7894)
	; 2632 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7896)
	; 2632 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7897)
	; 2633 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7900)
	; 2634 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7902)
	; 2634 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7903)
	; 2635 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7906)
	; 2636 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7908)
	; 2636 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7909)
	; 2637 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7912)
	; 2638 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7914)
	; 2638 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7915)
	; 2639 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7918)
	; 2640 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7920)
	; 2640 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7921)
	; 2641 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7924)
	; 2642 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7926)
	; 2642 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7927)
	; 2643 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 7930 to 7968
.if(n = 7930)
	; 2644 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 7930 && n < 7968)))
	; 2645..2657 bytes
	delay_n (n-2)
	nop
.elseif(n = 7932)
	; 2644 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7933)
	; 2645 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 7934 && n < 7966)))
	; 2646..2656 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7936)
	; 2646 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7938)
	; 2646 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7939)
	; 2647 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7942)
	; 2648 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7944)
	; 2648 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7945)
	; 2649 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 7948)
	; 2650 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7950)
	; 2650 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7951)
	; 2651 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7954)
	; 2652 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7956)
	; 2652 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7957)
	; 2653 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7960)
	; 2654 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7962)
	; 2654 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7963)
	; 2655 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7966)
	; 2656 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7968)
	; 2656 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.endif
.else
.if((n >= 7969) && (n < 8047))
.if((n >= 7969) && (n < 8008))
; 7969 to 8007
.if(n = 7969)
	; 2657 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 7969 && n < 8007)))
	; 2657..2669 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 7970 && n < 8008)))
	; 2658..2670 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7972)
	; 2658 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7974)
	; 2658 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7975)
	; 2659 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7978)
	; 2660 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7980)
	; 2660 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7981)
	; 2661 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 7984)
	; 2662 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7986)
	; 2662 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7987)
	; 2663 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7990)
	; 2664 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7992)
	; 2664 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7993)
	; 2665 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7996)
	; 2666 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7998)
	; 2666 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7999)
	; 2667 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8002)
	; 2668 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8004)
	; 2668 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8005)
	; 2669 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 8008 to 8046
.if(n = 8008)
	; 2670 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 8008 && n < 8046)))
	; 2671..2683 bytes
	delay_n (n-2)
	nop
.elseif(n = 8010)
	; 2670 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8011)
	; 2671 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 8012 && n < 8044)))
	; 2672..2682 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8014)
	; 2672 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8016)
	; 2672 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8017)
	; 2673 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8020)
	; 2674 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8022)
	; 2674 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8023)
	; 2675 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8026)
	; 2676 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8028)
	; 2676 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8029)
	; 2677 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8032)
	; 2678 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8034)
	; 2678 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8035)
	; 2679 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8038)
	; 2680 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8040)
	; 2680 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8041)
	; 2681 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8044)
	; 2682 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8046)
	; 2682 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.endif
.else
.if((n >= 8047) && (n < 8086))
; 8047 to 8085
.if(n = 8047)
	; 2683 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 8047 && n < 8085)))
	; 2683..2695 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8048 && n < 8086)))
	; 2684..2696 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8050)
	; 2684 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8052)
	; 2684 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8053)
	; 2685 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8056)
	; 2686 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8058)
	; 2686 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8059)
	; 2687 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8062)
	; 2688 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8064)
	; 2688 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8065)
	; 2689 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8068)
	; 2690 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8070)
	; 2690 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8071)
	; 2691 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8074)
	; 2692 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8076)
	; 2692 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8077)
	; 2693 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8080)
	; 2694 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8082)
	; 2694 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8083)
	; 2695 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 8086 to 8125
.if(n = 8086)
	; 2696 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 8086 && n < 8124)))
	; 2697..2709 bytes
	delay_n (n-2)
	nop
.elseif(n = 8088)
	; 2696 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8089)
	; 2697 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 8090 && n < 8122)))
	; 2698..2708 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8092)
	; 2698 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8094)
	; 2698 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8095)
	; 2699 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8098)
	; 2700 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8100)
	; 2700 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8101)
	; 2701 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8104)
	; 2702 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8106)
	; 2702 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8107)
	; 2703 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8110)
	; 2704 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8112)
	; 2704 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8113)
	; 2705 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8116)
	; 2706 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8118)
	; 2706 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8119)
	; 2707 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8122)
	; 2708 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8124)
	; 2708 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8125)
	; 2709 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
.if((n >= 8126) && (n < 8204))
.if((n >= 8126) && (n < 8165))
; 8126 to 8164
.if((n .mod 3 = 2) && ((n > 8125 && n < 8163)))
	; 2709..2721 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8126 && n < 8164)))
	; 2710..2722 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8128)
	; 2710 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8130)
	; 2710 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8131)
	; 2711 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8134)
	; 2712 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8136)
	; 2712 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8137)
	; 2713 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8140)
	; 2714 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8142)
	; 2714 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8143)
	; 2715 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8146)
	; 2716 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8148)
	; 2716 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8149)
	; 2717 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8152)
	; 2718 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8154)
	; 2718 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8155)
	; 2719 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8158)
	; 2720 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8160)
	; 2720 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8161)
	; 2721 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8164)
	; 2722 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 8165 to 8203
.if((n .mod 3 = 2) && ((n > 8164 && n < 8202)))
	; 2723..2735 bytes
	delay_n (n-2)
	nop
.elseif(n = 8166)
	; 2722 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8167)
	; 2723 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 8168 && n < 8200)))
	; 2724..2734 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8170)
	; 2724 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8172)
	; 2724 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8173)
	; 2725 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8176)
	; 2726 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8178)
	; 2726 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8179)
	; 2727 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8182)
	; 2728 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8184)
	; 2728 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8185)
	; 2729 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8188)
	; 2730 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8190)
	; 2730 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8191)
	; 2731 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8194)
	; 2732 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8196)
	; 2732 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8197)
	; 2733 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8200)
	; 2734 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8202)
	; 2734 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8203)
	; 2735 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
; 8204 to 8242
.if((n .mod 3 = 2) && ((n > 8203 && n < 8241)))
	; 2735..2747 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8204 && n < 8242)))
	; 2736..2748 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8206)
	; 2736 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8208)
	; 2736 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8209)
	; 2737 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8212)
	; 2738 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8214)
	; 2738 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8215)
	; 2739 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8218)
	; 2740 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8220)
	; 2740 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8221)
	; 2741 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8224)
	; 2742 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8226)
	; 2742 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8227)
	; 2743 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8230)
	; 2744 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8232)
	; 2744 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8233)
	; 2745 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8236)
	; 2746 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8238)
	; 2746 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8239)
	; 2747 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8242)
	; 2748 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 8243 to 8281
.if((n .mod 3 = 2) && ((n > 8242 && n < 8280)))
	; 2749..2761 bytes
	delay_n (n-2)
	nop
.elseif(n = 8244)
	; 2748 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8245)
	; 2749 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 8246 && n < 8278)))
	; 2750..2760 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8248)
	; 2750 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8250)
	; 2750 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8251)
	; 2751 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8254)
	; 2752 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8256)
	; 2752 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8257)
	; 2753 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8260)
	; 2754 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8262)
	; 2754 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8263)
	; 2755 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8266)
	; 2756 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8268)
	; 2756 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8269)
	; 2757 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8272)
	; 2758 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8274)
	; 2758 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8275)
	; 2759 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8278)
	; 2760 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8280)
	; 2760 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8281)
	; 2761 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.endif
.else
.if((n >= 8282) && (n < 8360))
.if((n >= 8282) && (n < 8321))
; 8282 to 8320
.if((n .mod 3 = 2) && ((n > 8281 && n < 8319)))
	; 2761..2773 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8282 && n < 8320)))
	; 2762..2774 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8284)
	; 2762 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8286)
	; 2762 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8287)
	; 2763 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8290)
	; 2764 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8292)
	; 2764 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8293)
	; 2765 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8296)
	; 2766 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8298)
	; 2766 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8299)
	; 2767 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8302)
	; 2768 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8304)
	; 2768 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8305)
	; 2769 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8308)
	; 2770 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8310)
	; 2770 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8311)
	; 2771 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8314)
	; 2772 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8316)
	; 2772 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8317)
	; 2773 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8320)
	; 2774 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 8321 to 8359
.if((n .mod 3 = 2) && ((n > 8320 && n < 8358)))
	; 2775..2787 bytes
	delay_n (n-2)
	nop
.elseif(n = 8322)
	; 2774 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8323)
	; 2775 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 8324 && n < 8356)))
	; 2776..2786 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8326)
	; 2776 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8328)
	; 2776 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8329)
	; 2777 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8332)
	; 2778 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8334)
	; 2778 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8335)
	; 2779 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8338)
	; 2780 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8340)
	; 2780 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8341)
	; 2781 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8344)
	; 2782 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8346)
	; 2782 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8347)
	; 2783 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8350)
	; 2784 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8352)
	; 2784 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8353)
	; 2785 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8356)
	; 2786 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8358)
	; 2786 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8359)
	; 2787 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.else
.if((n >= 8360) && (n < 8399))
; 8360 to 8398
.if((n .mod 3 = 2) && ((n > 8359 && n < 8397)))
	; 2787..2799 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8360 && n < 8398)))
	; 2788..2800 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8362)
	; 2788 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8364)
	; 2788 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8365)
	; 2789 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8368)
	; 2790 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8370)
	; 2790 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8371)
	; 2791 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8374)
	; 2792 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8376)
	; 2792 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8377)
	; 2793 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8380)
	; 2794 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8382)
	; 2794 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8383)
	; 2795 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8386)
	; 2796 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8388)
	; 2796 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8389)
	; 2797 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8392)
	; 2798 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8394)
	; 2798 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8395)
	; 2799 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8398)
	; 2800 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 8399 to 8437
.if((n .mod 3 = 2) && ((n > 8398 && n < 8436)))
	; 2801..2813 bytes
	delay_n (n-2)
	nop
.elseif(n = 8400)
	; 2800 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8401)
	; 2801 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 8402 && n < 8434)))
	; 2802..2812 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8404)
	; 2802 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8406)
	; 2802 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8407)
	; 2803 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8410)
	; 2804 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8412)
	; 2804 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8413)
	; 2805 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8416)
	; 2806 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8418)
	; 2806 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8419)
	; 2807 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8422)
	; 2808 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8424)
	; 2808 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8425)
	; 2809 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8428)
	; 2810 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8430)
	; 2810 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8431)
	; 2811 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8434)
	; 2812 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8436)
	; 2812 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8437)
	; 2813 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
; 8438 to 8476
.if((n .mod 3 = 2) && ((n > 8437 && n < 8475)))
	; 2813..2825 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8438 && n < 8476)))
	; 2814..2826 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8440)
	; 2814 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8442)
	; 2814 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8443)
	; 2815 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8446)
	; 2816 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8448)
	; 2816 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8449)
	; 2817 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8452)
	; 2818 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8454)
	; 2818 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8455)
	; 2819 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8458)
	; 2820 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8460)
	; 2820 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8461)
	; 2821 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8464)
	; 2822 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8466)
	; 2822 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8467)
	; 2823 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8470)
	; 2824 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8472)
	; 2824 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8473)
	; 2825 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8476)
	; 2826 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 8477 to 8515
.if((n .mod 3 = 2) && ((n > 8476 && n < 8514)))
	; 2827..2839 bytes
	delay_n (n-2)
	nop
.elseif(n = 8478)
	; 2826 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8479)
	; 2827 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 8480 && n < 8512)))
	; 2828..2838 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8482)
	; 2828 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8484)
	; 2828 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8485)
	; 2829 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8488)
	; 2830 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8490)
	; 2830 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8491)
	; 2831 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8494)
	; 2832 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8496)
	; 2832 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8497)
	; 2833 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8500)
	; 2834 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8502)
	; 2834 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8503)
	; 2835 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8506)
	; 2836 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8508)
	; 2836 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8509)
	; 2837 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8512)
	; 2838 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8514)
	; 2838 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8515)
	; 2839 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 8516) && (n < 8555))
; 8516 to 8554
.if((n .mod 3 = 2) && ((n > 8515 && n < 8553)))
	; 2839..2851 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8516 && n < 8554)))
	; 2840..2852 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8518)
	; 2840 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8520)
	; 2840 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8521)
	; 2841 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8524)
	; 2842 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8526)
	; 2842 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8527)
	; 2843 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8530)
	; 2844 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8532)
	; 2844 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8533)
	; 2845 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8536)
	; 2846 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8538)
	; 2846 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8539)
	; 2847 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8542)
	; 2848 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8544)
	; 2848 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8545)
	; 2849 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8548)
	; 2850 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8550)
	; 2850 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8551)
	; 2851 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8554)
	; 2852 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 8555 to 8593
.if((n .mod 3 = 2) && ((n > 8554 && n < 8592)))
	; 2853..2865 bytes
	delay_n (n-2)
	nop
.elseif(n = 8556)
	; 2852 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8557)
	; 2853 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 8558 && n < 8590)))
	; 2854..2864 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8560)
	; 2854 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8562)
	; 2854 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8563)
	; 2855 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8566)
	; 2856 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8568)
	; 2856 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8569)
	; 2857 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8572)
	; 2858 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8574)
	; 2858 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8575)
	; 2859 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8578)
	; 2860 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8580)
	; 2860 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8581)
	; 2861 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8584)
	; 2862 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8586)
	; 2862 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8587)
	; 2863 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8590)
	; 2864 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8592)
	; 2864 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8593)
	; 2865 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
.if((n >= 8594) && (n < 8633))
; 8594 to 8632
.if((n .mod 3 = 2) && ((n > 8593 && n < 8631)))
	; 2865..2877 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8594 && n < 8632)))
	; 2866..2878 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8596)
	; 2866 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8598)
	; 2866 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8599)
	; 2867 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8602)
	; 2868 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8604)
	; 2868 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8605)
	; 2869 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8608)
	; 2870 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8610)
	; 2870 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8611)
	; 2871 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8614)
	; 2872 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8616)
	; 2872 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8617)
	; 2873 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8620)
	; 2874 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8622)
	; 2874 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8623)
	; 2875 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8626)
	; 2876 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8628)
	; 2876 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8629)
	; 2877 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8632)
	; 2878 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 8633 to 8671
.if((n .mod 3 = 2) && ((n > 8632 && n < 8670)))
	; 2879..2891 bytes
	delay_n (n-2)
	nop
.elseif(n = 8634)
	; 2878 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8635)
	; 2879 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 8636 && n < 8668)))
	; 2880..2890 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8638)
	; 2880 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8640)
	; 2880 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8641)
	; 2881 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8644)
	; 2882 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8646)
	; 2882 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8647)
	; 2883 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8650)
	; 2884 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8652)
	; 2884 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8653)
	; 2885 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8656)
	; 2886 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8658)
	; 2886 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8659)
	; 2887 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8662)
	; 2888 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8664)
	; 2888 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8665)
	; 2889 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8668)
	; 2890 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8670)
	; 2890 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8671)
	; 2891 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.else
.if((n >= 8672) && (n < 8711))
; 8672 to 8710
.if((n .mod 3 = 2) && ((n > 8671 && n < 8709)))
	; 2891..2903 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8672 && n < 8710)))
	; 2892..2904 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8674)
	; 2892 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8676)
	; 2892 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8677)
	; 2893 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8680)
	; 2894 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8682)
	; 2894 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8683)
	; 2895 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8686)
	; 2896 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8688)
	; 2896 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8689)
	; 2897 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8692)
	; 2898 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8694)
	; 2898 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8695)
	; 2899 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8698)
	; 2900 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8700)
	; 2900 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8701)
	; 2901 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8704)
	; 2902 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8706)
	; 2902 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8707)
	; 2903 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8710)
	; 2904 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 8711 to 8750
.if((n .mod 3 = 2) && ((n > 8710 && n < 8751)))
	; 2905..2917 bytes
	delay_n (n-2)
	nop
.elseif(n = 8712)
	; 2904 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8713)
	; 2905 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 8714 && n < 8746)))
	; 2906..2916 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8716)
	; 2906 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8718)
	; 2906 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8719)
	; 2907 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8722)
	; 2908 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8724)
	; 2908 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8725)
	; 2909 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8728)
	; 2910 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8730)
	; 2910 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8731)
	; 2911 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8734)
	; 2912 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8736)
	; 2912 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8737)
	; 2913 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8740)
	; 2914 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8742)
	; 2914 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8743)
	; 2915 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8746)
	; 2916 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8748)
	; 2916 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8749)
	; 2917 bytes
	delay_n 55
	delay_n (n-55)
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
.if((n >= 8751) && (n < 8829))
.if((n >= 8751) && (n < 8790))
; 8751 to 8789
.if((n .mod 6 = 3) && ((n > 8750 && n < 8788)))
	; 2918..2930 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8752)
	; 2918 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 8752 && n < 8790)))
	; 2919..2931 bytes
	delay_n (n-2)
	nop
.elseif(n = 8754)
	; 2918 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8755)
	; 2919 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8758)
	; 2920 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8760)
	; 2920 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8761)
	; 2921 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8764)
	; 2922 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8766)
	; 2922 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8767)
	; 2923 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8770)
	; 2924 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8772)
	; 2924 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8773)
	; 2925 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8776)
	; 2926 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8778)
	; 2926 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8779)
	; 2927 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8782)
	; 2928 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8784)
	; 2928 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8785)
	; 2929 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8788)
	; 2930 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 8790 to 8828
.if(n = 8790)
	; 2930 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8791)
	; 2931 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 8791 && n < 8829)))
	; 2931..2943 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8792 && n < 8824)))
	; 2932..2942 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8794)
	; 2932 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8796)
	; 2932 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8797)
	; 2933 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8800)
	; 2934 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8802)
	; 2934 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8803)
	; 2935 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8806)
	; 2936 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8808)
	; 2936 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8809)
	; 2937 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8812)
	; 2938 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8814)
	; 2938 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8815)
	; 2939 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8818)
	; 2940 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8820)
	; 2940 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8821)
	; 2941 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8824)
	; 2942 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8826)
	; 2942 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8827)
	; 2943 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 8829) && (n < 8868))
; 8829 to 8867
.if((n .mod 6 = 3) && ((n > 8828 && n < 8866)))
	; 2944..2956 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8830)
	; 2944 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 8830 && n < 8868)))
	; 2945..2957 bytes
	delay_n (n-2)
	nop
.elseif(n = 8832)
	; 2944 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8833)
	; 2945 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8836)
	; 2946 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8838)
	; 2946 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8839)
	; 2947 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8842)
	; 2948 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8844)
	; 2948 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8845)
	; 2949 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8848)
	; 2950 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8850)
	; 2950 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8851)
	; 2951 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8854)
	; 2952 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8856)
	; 2952 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8857)
	; 2953 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8860)
	; 2954 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8862)
	; 2954 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8863)
	; 2955 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8866)
	; 2956 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 8868 to 8906
.if(n = 8868)
	; 2956 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8869)
	; 2957 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 8869 && n < 8907)))
	; 2957..2969 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8870 && n < 8902)))
	; 2958..2968 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8872)
	; 2958 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8874)
	; 2958 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8875)
	; 2959 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8878)
	; 2960 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8880)
	; 2960 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8881)
	; 2961 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8884)
	; 2962 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8886)
	; 2962 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8887)
	; 2963 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8890)
	; 2964 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8892)
	; 2964 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8893)
	; 2965 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8896)
	; 2966 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8898)
	; 2966 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8899)
	; 2967 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8902)
	; 2968 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8904)
	; 2968 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8905)
	; 2969 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.else
.if((n >= 8907) && (n < 8985))
.if((n >= 8907) && (n < 8946))
; 8907 to 8945
.if((n .mod 6 = 3) && ((n > 8906 && n < 8944)))
	; 2970..2982 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8908)
	; 2970 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 8908 && n < 8946)))
	; 2971..2983 bytes
	delay_n (n-2)
	nop
.elseif(n = 8910)
	; 2970 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8911)
	; 2971 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8914)
	; 2972 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8916)
	; 2972 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8917)
	; 2973 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8920)
	; 2974 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8922)
	; 2974 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8923)
	; 2975 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8926)
	; 2976 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8928)
	; 2976 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8929)
	; 2977 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8932)
	; 2978 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8934)
	; 2978 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8935)
	; 2979 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8938)
	; 2980 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8940)
	; 2980 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8941)
	; 2981 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8944)
	; 2982 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 8946 to 8984
.if(n = 8946)
	; 2982 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8947)
	; 2983 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 8947 && n < 8985)))
	; 2983..2995 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 8948 && n < 8980)))
	; 2984..2994 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8950)
	; 2984 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8952)
	; 2984 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8953)
	; 2985 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 8956)
	; 2986 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8958)
	; 2986 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8959)
	; 2987 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8962)
	; 2988 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8964)
	; 2988 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8965)
	; 2989 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8968)
	; 2990 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8970)
	; 2990 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8971)
	; 2991 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 8974)
	; 2992 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8976)
	; 2992 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8977)
	; 2993 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8980)
	; 2994 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8982)
	; 2994 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8983)
	; 2995 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
.if((n >= 8985) && (n < 9024))
; 8985 to 9023
.if((n .mod 6 = 3) && ((n > 8984 && n < 9022)))
	; 2996..3008 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 8986)
	; 2996 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 8986 && n < 9024)))
	; 2997..3009 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988)
	; 2996 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8989)
	; 2997 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8992)
	; 2998 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8994)
	; 2998 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8995)
	; 2999 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8998)
	; 3000 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9000)
	; 3000 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9001)
	; 3001 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9004)
	; 3002 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9006)
	; 3002 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9007)
	; 3003 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9010)
	; 3004 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9012)
	; 3004 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9013)
	; 3005 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9016)
	; 3006 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9018)
	; 3006 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9019)
	; 3007 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9022)
	; 3008 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 9024 to 9062
.if(n = 9024)
	; 3008 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9025)
	; 3009 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 9025 && n < 9063)))
	; 3009..3021 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9026 && n < 9058)))
	; 3010..3020 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9028)
	; 3010 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9030)
	; 3010 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9031)
	; 3011 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9034)
	; 3012 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9036)
	; 3012 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9037)
	; 3013 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9040)
	; 3014 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9042)
	; 3014 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9043)
	; 3015 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9046)
	; 3016 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9048)
	; 3016 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9049)
	; 3017 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9052)
	; 3018 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9054)
	; 3018 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9055)
	; 3019 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9058)
	; 3020 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9060)
	; 3020 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9061)
	; 3021 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
.if((n >= 9063) && (n < 9102))
; 9063 to 9101
.if((n .mod 6 = 3) && ((n > 9062 && n < 9100)))
	; 3022..3034 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9064)
	; 3022 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 9064 && n < 9102)))
	; 3023..3035 bytes
	delay_n (n-2)
	nop
.elseif(n = 9066)
	; 3022 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9067)
	; 3023 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9070)
	; 3024 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9072)
	; 3024 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9073)
	; 3025 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9076)
	; 3026 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9078)
	; 3026 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9079)
	; 3027 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9082)
	; 3028 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9084)
	; 3028 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9085)
	; 3029 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9088)
	; 3030 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9090)
	; 3030 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9091)
	; 3031 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9094)
	; 3032 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9096)
	; 3032 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9097)
	; 3033 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9100)
	; 3034 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 9102 to 9140
.if(n = 9102)
	; 3034 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9103)
	; 3035 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 9103 && n < 9141)))
	; 3035..3047 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9104 && n < 9136)))
	; 3036..3046 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9106)
	; 3036 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9108)
	; 3036 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9109)
	; 3037 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9112)
	; 3038 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9114)
	; 3038 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9115)
	; 3039 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9118)
	; 3040 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9120)
	; 3040 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9121)
	; 3041 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9124)
	; 3042 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9126)
	; 3042 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9127)
	; 3043 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9130)
	; 3044 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9132)
	; 3044 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9133)
	; 3045 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9136)
	; 3046 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9138)
	; 3046 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9139)
	; 3047 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if((n .mod 6 = 3) && ((n > 9140 && n < 9178)))
	; 3048..3060 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9142)
	; 3048 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 9142 && n < 9180)))
	; 3049..3061 bytes
	delay_n (n-2)
	nop
.elseif(n = 9144)
	; 3048 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9145)
	; 3049 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9148)
	; 3050 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9150)
	; 3050 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9151)
	; 3051 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9154)
	; 3052 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9156)
	; 3052 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9157)
	; 3053 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9160)
	; 3054 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9162)
	; 3054 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9163)
	; 3055 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9166)
	; 3056 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9168)
	; 3056 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9169)
	; 3057 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9172)
	; 3058 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9174)
	; 3058 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9175)
	; 3059 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9178)
	; 3060 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 9180 to 9218
.if(n = 9180)
	; 3060 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9181)
	; 3061 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 9181 && n < 9219)))
	; 3061..3073 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9182 && n < 9214)))
	; 3062..3072 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9184)
	; 3062 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9186)
	; 3062 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9187)
	; 3063 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9190)
	; 3064 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9192)
	; 3064 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9193)
	; 3065 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9196)
	; 3066 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9198)
	; 3066 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9199)
	; 3067 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9202)
	; 3068 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9204)
	; 3068 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9205)
	; 3069 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9208)
	; 3070 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9210)
	; 3070 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9211)
	; 3071 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9214)
	; 3072 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9216)
	; 3072 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9217)
	; 3073 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
; 9219 to 9257
.if((n .mod 6 = 3) && ((n > 9218 && n < 9256)))
	; 3074..3086 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9220)
	; 3074 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 9220 && n < 9258)))
	; 3075..3087 bytes
	delay_n (n-2)
	nop
.elseif(n = 9222)
	; 3074 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9223)
	; 3075 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9226)
	; 3076 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9228)
	; 3076 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9229)
	; 3077 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9232)
	; 3078 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9234)
	; 3078 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9235)
	; 3079 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9238)
	; 3080 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9240)
	; 3080 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9241)
	; 3081 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9244)
	; 3082 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9246)
	; 3082 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9247)
	; 3083 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9250)
	; 3084 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9252)
	; 3084 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9253)
	; 3085 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9256)
	; 3086 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 9258 to 9296
.if(n = 9258)
	; 3086 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9259)
	; 3087 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 9259 && n < 9297)))
	; 3087..3099 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9260 && n < 9292)))
	; 3088..3098 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9262)
	; 3088 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9264)
	; 3088 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9265)
	; 3089 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9268)
	; 3090 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9270)
	; 3090 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9271)
	; 3091 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9274)
	; 3092 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9276)
	; 3092 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9277)
	; 3093 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9280)
	; 3094 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9282)
	; 3094 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9283)
	; 3095 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9286)
	; 3096 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9288)
	; 3096 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9289)
	; 3097 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9292)
	; 3098 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9294)
	; 3098 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9295)
	; 3099 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.else
.if((n >= 9297) && (n < 9336))
; 9297 to 9335
.if((n .mod 6 = 3) && ((n > 9296 && n < 9334)))
	; 3100..3112 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9298)
	; 3100 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 9298 && n < 9336)))
	; 3101..3113 bytes
	delay_n (n-2)
	nop
.elseif(n = 9300)
	; 3100 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9301)
	; 3101 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9304)
	; 3102 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9306)
	; 3102 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9307)
	; 3103 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9310)
	; 3104 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9312)
	; 3104 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9313)
	; 3105 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9316)
	; 3106 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9318)
	; 3106 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9319)
	; 3107 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9322)
	; 3108 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9324)
	; 3108 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9325)
	; 3109 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9328)
	; 3110 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9330)
	; 3110 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9331)
	; 3111 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9334)
	; 3112 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 9336 to 9375
.if(n = 9336)
	; 3112 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9337)
	; 3113 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 9337 && n < 9375)))
	; 3113..3125 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9338 && n < 9376)))
	; 3114..3126 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9340)
	; 3114 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9342)
	; 3114 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9343)
	; 3115 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9346)
	; 3116 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9348)
	; 3116 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9349)
	; 3117 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9352)
	; 3118 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9354)
	; 3118 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9355)
	; 3119 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9358)
	; 3120 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9360)
	; 3120 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9361)
	; 3121 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9364)
	; 3122 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9366)
	; 3122 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9367)
	; 3123 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9370)
	; 3124 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9372)
	; 3124 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9373)
	; 3125 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 9376) && (n < 9688))
.if((n >= 9376) && (n < 9532))
.if((n >= 9376) && (n < 9454))
.if((n >= 9376) && (n < 9415))
; 9376 to 9414
.if(n = 9376)
	; 3126 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 9376 && n < 9414)))
	; 3127..3139 bytes
	delay_n (n-2)
	nop
.elseif(n = 9378)
	; 3126 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9379)
	; 3127 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 9380 && n < 9412)))
	; 3128..3138 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9382)
	; 3128 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9384)
	; 3128 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9385)
	; 3129 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9388)
	; 3130 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9390)
	; 3130 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9391)
	; 3131 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9394)
	; 3132 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9396)
	; 3132 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9397)
	; 3133 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9400)
	; 3134 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9402)
	; 3134 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9403)
	; 3135 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9406)
	; 3136 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9408)
	; 3136 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9409)
	; 3137 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9412)
	; 3138 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9414)
	; 3138 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 9415 to 9453
.if(n = 9415)
	; 3139 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 9415 && n < 9453)))
	; 3139..3151 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9416 && n < 9454)))
	; 3140..3152 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9418)
	; 3140 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9420)
	; 3140 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9421)
	; 3141 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9424)
	; 3142 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9426)
	; 3142 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9427)
	; 3143 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9430)
	; 3144 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9432)
	; 3144 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9433)
	; 3145 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9436)
	; 3146 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9438)
	; 3146 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9439)
	; 3147 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9442)
	; 3148 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9444)
	; 3148 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9445)
	; 3149 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9448)
	; 3150 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9450)
	; 3150 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9451)
	; 3151 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 9454) && (n < 9493))
; 9454 to 9492
.if(n = 9454)
	; 3152 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 9454 && n < 9492)))
	; 3153..3165 bytes
	delay_n (n-2)
	nop
.elseif(n = 9456)
	; 3152 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9457)
	; 3153 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 9458 && n < 9490)))
	; 3154..3164 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9460)
	; 3154 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9462)
	; 3154 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9463)
	; 3155 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9466)
	; 3156 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9468)
	; 3156 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9469)
	; 3157 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9472)
	; 3158 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9474)
	; 3158 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9475)
	; 3159 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9478)
	; 3160 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9480)
	; 3160 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9481)
	; 3161 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9484)
	; 3162 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9486)
	; 3162 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9487)
	; 3163 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9490)
	; 3164 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9492)
	; 3164 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 9493 to 9531
.if(n = 9493)
	; 3165 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 9493 && n < 9531)))
	; 3165..3177 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9494 && n < 9532)))
	; 3166..3178 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9496)
	; 3166 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9498)
	; 3166 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9499)
	; 3167 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9502)
	; 3168 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9504)
	; 3168 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9505)
	; 3169 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9508)
	; 3170 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9510)
	; 3170 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9511)
	; 3171 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9514)
	; 3172 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9516)
	; 3172 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9517)
	; 3173 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9520)
	; 3174 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9522)
	; 3174 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9523)
	; 3175 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9526)
	; 3176 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9528)
	; 3176 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9529)
	; 3177 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.else
.if((n >= 9532) && (n < 9610))
.if((n >= 9532) && (n < 9571))
; 9532 to 9570
.if(n = 9532)
	; 3178 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 9532 && n < 9570)))
	; 3179..3191 bytes
	delay_n (n-2)
	nop
.elseif(n = 9534)
	; 3178 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9535)
	; 3179 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 9536 && n < 9568)))
	; 3180..3190 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9538)
	; 3180 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9540)
	; 3180 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9541)
	; 3181 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9544)
	; 3182 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9546)
	; 3182 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9547)
	; 3183 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9550)
	; 3184 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9552)
	; 3184 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9553)
	; 3185 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9556)
	; 3186 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9558)
	; 3186 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9559)
	; 3187 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9562)
	; 3188 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9564)
	; 3188 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9565)
	; 3189 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9568)
	; 3190 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9570)
	; 3190 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 9571 to 9609
.if(n = 9571)
	; 3191 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 9571 && n < 9609)))
	; 3191..3203 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9572 && n < 9610)))
	; 3192..3204 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9574)
	; 3192 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9576)
	; 3192 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9577)
	; 3193 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9580)
	; 3194 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9582)
	; 3194 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9583)
	; 3195 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9586)
	; 3196 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9588)
	; 3196 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9589)
	; 3197 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9592)
	; 3198 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9594)
	; 3198 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9595)
	; 3199 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9598)
	; 3200 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9600)
	; 3200 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9601)
	; 3201 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9604)
	; 3202 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9606)
	; 3202 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9607)
	; 3203 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 9610) && (n < 9649))
; 9610 to 9648
.if(n = 9610)
	; 3204 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 9610 && n < 9648)))
	; 3205..3217 bytes
	delay_n (n-2)
	nop
.elseif(n = 9612)
	; 3204 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9613)
	; 3205 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 9614 && n < 9646)))
	; 3206..3216 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9616)
	; 3206 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9618)
	; 3206 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9619)
	; 3207 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9622)
	; 3208 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9624)
	; 3208 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9625)
	; 3209 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9628)
	; 3210 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9630)
	; 3210 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9631)
	; 3211 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9634)
	; 3212 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9636)
	; 3212 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9637)
	; 3213 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9640)
	; 3214 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9642)
	; 3214 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9643)
	; 3215 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9646)
	; 3216 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9648)
	; 3216 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 9649 to 9687
.if(n = 9649)
	; 3217 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 9649 && n < 9687)))
	; 3217..3229 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9650 && n < 9688)))
	; 3218..3230 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9652)
	; 3218 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9654)
	; 3218 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9655)
	; 3219 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9658)
	; 3220 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9660)
	; 3220 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9661)
	; 3221 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9664)
	; 3222 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9666)
	; 3222 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9667)
	; 3223 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9670)
	; 3224 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9672)
	; 3224 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9673)
	; 3225 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9676)
	; 3226 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9678)
	; 3226 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9679)
	; 3227 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9682)
	; 3228 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9684)
	; 3228 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9685)
	; 3229 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.endif
.endif
.else
.if((n >= 9688) && (n < 9844))
.if((n >= 9688) && (n < 9766))
.if((n >= 9688) && (n < 9727))
; 9688 to 9726
.if(n = 9688)
	; 3230 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 9688 && n < 9726)))
	; 3231..3243 bytes
	delay_n (n-2)
	nop
.elseif(n = 9690)
	; 3230 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9691)
	; 3231 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 9692 && n < 9724)))
	; 3232..3242 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9694)
	; 3232 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9696)
	; 3232 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9697)
	; 3233 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9700)
	; 3234 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9702)
	; 3234 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9703)
	; 3235 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9706)
	; 3236 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9708)
	; 3236 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9709)
	; 3237 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9712)
	; 3238 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9714)
	; 3238 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9715)
	; 3239 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9718)
	; 3240 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9720)
	; 3240 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9721)
	; 3241 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9724)
	; 3242 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9726)
	; 3242 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 9727 to 9765
.if(n = 9727)
	; 3243 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 9727 && n < 9765)))
	; 3243..3255 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9728 && n < 9766)))
	; 3244..3256 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9730)
	; 3244 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9732)
	; 3244 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9733)
	; 3245 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9736)
	; 3246 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9738)
	; 3246 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9739)
	; 3247 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9742)
	; 3248 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9744)
	; 3248 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9745)
	; 3249 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9748)
	; 3250 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9750)
	; 3250 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9751)
	; 3251 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9754)
	; 3252 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9756)
	; 3252 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9757)
	; 3253 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9760)
	; 3254 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9762)
	; 3254 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9763)
	; 3255 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.else
.if((n >= 9766) && (n < 9805))
; 9766 to 9804
.if(n = 9766)
	; 3256 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 9766 && n < 9804)))
	; 3257..3269 bytes
	delay_n (n-2)
	nop
.elseif(n = 9768)
	; 3256 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9769)
	; 3257 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 9770 && n < 9802)))
	; 3258..3268 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9772)
	; 3258 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9774)
	; 3258 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9775)
	; 3259 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9778)
	; 3260 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9780)
	; 3260 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9781)
	; 3261 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9784)
	; 3262 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9786)
	; 3262 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9787)
	; 3263 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9790)
	; 3264 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9792)
	; 3264 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9793)
	; 3265 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9796)
	; 3266 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9798)
	; 3266 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9799)
	; 3267 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9802)
	; 3268 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9804)
	; 3268 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 9805 to 9843
.if(n = 9805)
	; 3269 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 9805 && n < 9843)))
	; 3269..3281 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9806 && n < 9844)))
	; 3270..3282 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9808)
	; 3270 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9810)
	; 3270 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9811)
	; 3271 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9814)
	; 3272 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9816)
	; 3272 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9817)
	; 3273 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9820)
	; 3274 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9822)
	; 3274 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9823)
	; 3275 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9826)
	; 3276 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9828)
	; 3276 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9829)
	; 3277 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9832)
	; 3278 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9834)
	; 3278 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9835)
	; 3279 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9838)
	; 3280 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9840)
	; 3280 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9841)
	; 3281 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
.if((n >= 9844) && (n < 9883))
; 9844 to 9882
.if(n = 9844)
	; 3282 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 9844 && n < 9882)))
	; 3283..3295 bytes
	delay_n (n-2)
	nop
.elseif(n = 9846)
	; 3282 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9847)
	; 3283 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 9848 && n < 9880)))
	; 3284..3294 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9850)
	; 3284 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9852)
	; 3284 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9853)
	; 3285 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9856)
	; 3286 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9858)
	; 3286 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9859)
	; 3287 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9862)
	; 3288 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9864)
	; 3288 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9865)
	; 3289 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9868)
	; 3290 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9870)
	; 3290 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9871)
	; 3291 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9874)
	; 3292 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9876)
	; 3292 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9877)
	; 3293 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9880)
	; 3294 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9882)
	; 3294 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 9883 to 9921
.if(n = 9883)
	; 3295 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 9883 && n < 9921)))
	; 3295..3307 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9884 && n < 9922)))
	; 3296..3308 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9886)
	; 3296 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9888)
	; 3296 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9889)
	; 3297 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9892)
	; 3298 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9894)
	; 3298 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9895)
	; 3299 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9898)
	; 3300 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9900)
	; 3300 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9901)
	; 3301 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9904)
	; 3302 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9906)
	; 3302 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9907)
	; 3303 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9910)
	; 3304 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9912)
	; 3304 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9913)
	; 3305 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9916)
	; 3306 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9918)
	; 3306 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9919)
	; 3307 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 9922) && (n < 9961))
; 9922 to 9960
.if(n = 9922)
	; 3308 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 9922 && n < 9960)))
	; 3309..3321 bytes
	delay_n (n-2)
	nop
.elseif(n = 9924)
	; 3308 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9925)
	; 3309 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 9926 && n < 9958)))
	; 3310..3320 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9928)
	; 3310 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9930)
	; 3310 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9931)
	; 3311 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9934)
	; 3312 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9936)
	; 3312 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9937)
	; 3313 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9940)
	; 3314 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9942)
	; 3314 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9943)
	; 3315 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 9946)
	; 3316 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9948)
	; 3316 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9949)
	; 3317 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9952)
	; 3318 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9954)
	; 3318 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9955)
	; 3319 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9958)
	; 3320 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9960)
	; 3320 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 9961 to 10000
.if(n = 9961)
	; 3321 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 9961 && n < 9999)))
	; 3321..3333 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 9962 && n < 10000)))
	; 3322..3334 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 9964)
	; 3322 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9966)
	; 3322 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9967)
	; 3323 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9970)
	; 3324 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9972)
	; 3324 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9973)
	; 3325 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9976)
	; 3326 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9978)
	; 3326 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9979)
	; 3327 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 9982)
	; 3328 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9984)
	; 3328 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9985)
	; 3329 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9988)
	; 3330 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9990)
	; 3330 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9991)
	; 3331 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9994)
	; 3332 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9996)
	; 3332 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9997)
	; 3333 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10000)
	; 3334 bytes
	delay_n 10
	delay_n (n-10)
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
; 10001 to 10039
.if((n .mod 3 = 2) && ((n > 10000 && n < 10038)))
	; 3335..3347 bytes
	delay_n (n-2)
	nop
.elseif(n = 10002)
	; 3334 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10003)
	; 3335 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 10004 && n < 10036)))
	; 3336..3346 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10006)
	; 3336 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10008)
	; 3336 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10009)
	; 3337 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10012)
	; 3338 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10014)
	; 3338 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10015)
	; 3339 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10018)
	; 3340 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10020)
	; 3340 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10021)
	; 3341 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10024)
	; 3342 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10026)
	; 3342 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10027)
	; 3343 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10030)
	; 3344 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10032)
	; 3344 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10033)
	; 3345 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10036)
	; 3346 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10038)
	; 3346 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10039)
	; 3347 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 10040 to 10078
.if((n .mod 3 = 2) && ((n > 10039 && n < 10077)))
	; 3347..3359 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10040 && n < 10078)))
	; 3348..3360 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10042)
	; 3348 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10044)
	; 3348 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10045)
	; 3349 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10048)
	; 3350 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10050)
	; 3350 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10051)
	; 3351 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10054)
	; 3352 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10056)
	; 3352 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10057)
	; 3353 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10060)
	; 3354 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10062)
	; 3354 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10063)
	; 3355 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10066)
	; 3356 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10068)
	; 3356 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10069)
	; 3357 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10072)
	; 3358 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10074)
	; 3358 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10075)
	; 3359 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10078)
	; 3360 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 10079) && (n < 10118))
; 10079 to 10117
.if((n .mod 3 = 2) && ((n > 10078 && n < 10116)))
	; 3361..3373 bytes
	delay_n (n-2)
	nop
.elseif(n = 10080)
	; 3360 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10081)
	; 3361 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 10082 && n < 10114)))
	; 3362..3372 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10084)
	; 3362 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10086)
	; 3362 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10087)
	; 3363 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10090)
	; 3364 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10092)
	; 3364 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10093)
	; 3365 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10096)
	; 3366 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10098)
	; 3366 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10099)
	; 3367 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10102)
	; 3368 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10104)
	; 3368 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10105)
	; 3369 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10108)
	; 3370 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10110)
	; 3370 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10111)
	; 3371 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10114)
	; 3372 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10116)
	; 3372 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10117)
	; 3373 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 10118 to 10156
.if((n .mod 3 = 2) && ((n > 10117 && n < 10155)))
	; 3373..3385 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10118 && n < 10156)))
	; 3374..3386 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10120)
	; 3374 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10122)
	; 3374 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10123)
	; 3375 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10126)
	; 3376 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10128)
	; 3376 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10129)
	; 3377 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10132)
	; 3378 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10134)
	; 3378 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10135)
	; 3379 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10138)
	; 3380 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10140)
	; 3380 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10141)
	; 3381 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10144)
	; 3382 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10146)
	; 3382 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10147)
	; 3383 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10150)
	; 3384 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10152)
	; 3384 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10153)
	; 3385 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10156)
	; 3386 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.else
.if((n >= 10157) && (n < 10235))
.if((n >= 10157) && (n < 10196))
; 10157 to 10195
.if((n .mod 3 = 2) && ((n > 10156 && n < 10194)))
	; 3387..3399 bytes
	delay_n (n-2)
	nop
.elseif(n = 10158)
	; 3386 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10159)
	; 3387 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 10160 && n < 10192)))
	; 3388..3398 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10162)
	; 3388 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10164)
	; 3388 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10165)
	; 3389 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10168)
	; 3390 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10170)
	; 3390 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10171)
	; 3391 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10174)
	; 3392 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10176)
	; 3392 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10177)
	; 3393 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10180)
	; 3394 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10182)
	; 3394 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10183)
	; 3395 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10186)
	; 3396 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10188)
	; 3396 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10189)
	; 3397 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10192)
	; 3398 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10194)
	; 3398 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10195)
	; 3399 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 10196 to 10234
.if((n .mod 3 = 2) && ((n > 10195 && n < 10233)))
	; 3399..3411 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10196 && n < 10234)))
	; 3400..3412 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10198)
	; 3400 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10200)
	; 3400 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10201)
	; 3401 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10204)
	; 3402 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10206)
	; 3402 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10207)
	; 3403 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10210)
	; 3404 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10212)
	; 3404 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10213)
	; 3405 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10216)
	; 3406 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10218)
	; 3406 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10219)
	; 3407 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10222)
	; 3408 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10224)
	; 3408 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10225)
	; 3409 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10228)
	; 3410 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10230)
	; 3410 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10231)
	; 3411 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10234)
	; 3412 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 10235) && (n < 10274))
; 10235 to 10273
.if((n .mod 3 = 2) && ((n > 10234 && n < 10272)))
	; 3413..3425 bytes
	delay_n (n-2)
	nop
.elseif(n = 10236)
	; 3412 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10237)
	; 3413 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 10238 && n < 10270)))
	; 3414..3424 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10240)
	; 3414 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10242)
	; 3414 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10243)
	; 3415 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10246)
	; 3416 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10248)
	; 3416 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10249)
	; 3417 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10252)
	; 3418 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10254)
	; 3418 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10255)
	; 3419 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10258)
	; 3420 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10260)
	; 3420 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10261)
	; 3421 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10264)
	; 3422 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10266)
	; 3422 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10267)
	; 3423 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10270)
	; 3424 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10272)
	; 3424 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10273)
	; 3425 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 10274 to 10312
.if((n .mod 3 = 2) && ((n > 10273 && n < 10311)))
	; 3425..3437 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10274 && n < 10312)))
	; 3426..3438 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10276)
	; 3426 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10278)
	; 3426 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10279)
	; 3427 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10282)
	; 3428 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10284)
	; 3428 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10285)
	; 3429 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10288)
	; 3430 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10290)
	; 3430 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10291)
	; 3431 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10294)
	; 3432 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10296)
	; 3432 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10297)
	; 3433 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10300)
	; 3434 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10302)
	; 3434 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10303)
	; 3435 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10306)
	; 3436 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10308)
	; 3436 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10309)
	; 3437 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10312)
	; 3438 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
.if((n >= 10313) && (n < 10391))
.if((n >= 10313) && (n < 10352))
; 10313 to 10351
.if((n .mod 3 = 2) && ((n > 10312 && n < 10350)))
	; 3439..3451 bytes
	delay_n (n-2)
	nop
.elseif(n = 10314)
	; 3438 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10315)
	; 3439 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 10316 && n < 10348)))
	; 3440..3450 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10318)
	; 3440 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10320)
	; 3440 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10321)
	; 3441 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10324)
	; 3442 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10326)
	; 3442 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10327)
	; 3443 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10330)
	; 3444 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10332)
	; 3444 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10333)
	; 3445 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10336)
	; 3446 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10338)
	; 3446 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10339)
	; 3447 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10342)
	; 3448 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10344)
	; 3448 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10345)
	; 3449 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10348)
	; 3450 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10350)
	; 3450 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10351)
	; 3451 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 10352 to 10390
.if((n .mod 3 = 2) && ((n > 10351 && n < 10389)))
	; 3451..3463 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10352 && n < 10390)))
	; 3452..3464 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10354)
	; 3452 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10356)
	; 3452 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10357)
	; 3453 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10360)
	; 3454 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10362)
	; 3454 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10363)
	; 3455 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10366)
	; 3456 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10368)
	; 3456 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10369)
	; 3457 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10372)
	; 3458 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10374)
	; 3458 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10375)
	; 3459 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10378)
	; 3460 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10380)
	; 3460 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10381)
	; 3461 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10384)
	; 3462 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10386)
	; 3462 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10387)
	; 3463 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10390)
	; 3464 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 10391) && (n < 10430))
; 10391 to 10429
.if((n .mod 3 = 2) && ((n > 10390 && n < 10428)))
	; 3465..3477 bytes
	delay_n (n-2)
	nop
.elseif(n = 10392)
	; 3464 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10393)
	; 3465 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 10394 && n < 10426)))
	; 3466..3476 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10396)
	; 3466 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10398)
	; 3466 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10399)
	; 3467 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10402)
	; 3468 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10404)
	; 3468 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10405)
	; 3469 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10408)
	; 3470 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10410)
	; 3470 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10411)
	; 3471 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10414)
	; 3472 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10416)
	; 3472 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10417)
	; 3473 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10420)
	; 3474 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10422)
	; 3474 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10423)
	; 3475 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10426)
	; 3476 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10428)
	; 3476 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10429)
	; 3477 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 10430 to 10468
.if((n .mod 3 = 2) && ((n > 10429 && n < 10467)))
	; 3477..3489 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10430 && n < 10468)))
	; 3478..3490 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10432)
	; 3478 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10434)
	; 3478 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10435)
	; 3479 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10438)
	; 3480 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10440)
	; 3480 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10441)
	; 3481 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10444)
	; 3482 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10446)
	; 3482 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10447)
	; 3483 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10450)
	; 3484 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10452)
	; 3484 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10453)
	; 3485 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10456)
	; 3486 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10458)
	; 3486 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10459)
	; 3487 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10462)
	; 3488 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10464)
	; 3488 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10465)
	; 3489 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10468)
	; 3490 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.else
.if((n >= 10469) && (n < 10547))
.if((n >= 10469) && (n < 10508))
; 10469 to 10507
.if((n .mod 3 = 2) && ((n > 10468 && n < 10506)))
	; 3491..3503 bytes
	delay_n (n-2)
	nop
.elseif(n = 10470)
	; 3490 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10471)
	; 3491 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 10472 && n < 10504)))
	; 3492..3502 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10474)
	; 3492 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10476)
	; 3492 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10477)
	; 3493 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10480)
	; 3494 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10482)
	; 3494 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10483)
	; 3495 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10486)
	; 3496 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10488)
	; 3496 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10489)
	; 3497 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10492)
	; 3498 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10494)
	; 3498 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10495)
	; 3499 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10498)
	; 3500 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10500)
	; 3500 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10501)
	; 3501 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10504)
	; 3502 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10506)
	; 3502 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10507)
	; 3503 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 10508 to 10546
.if((n .mod 3 = 2) && ((n > 10507 && n < 10545)))
	; 3503..3515 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10508 && n < 10546)))
	; 3504..3516 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10510)
	; 3504 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10512)
	; 3504 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10513)
	; 3505 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10516)
	; 3506 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10518)
	; 3506 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10519)
	; 3507 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10522)
	; 3508 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10524)
	; 3508 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10525)
	; 3509 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10528)
	; 3510 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10530)
	; 3510 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10531)
	; 3511 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10534)
	; 3512 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10536)
	; 3512 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10537)
	; 3513 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10540)
	; 3514 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10542)
	; 3514 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10543)
	; 3515 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10546)
	; 3516 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 10547) && (n < 10586))
; 10547 to 10585
.if((n .mod 3 = 2) && ((n > 10546 && n < 10584)))
	; 3517..3529 bytes
	delay_n (n-2)
	nop
.elseif(n = 10548)
	; 3516 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10549)
	; 3517 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 10550 && n < 10582)))
	; 3518..3528 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10552)
	; 3518 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10554)
	; 3518 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10555)
	; 3519 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10558)
	; 3520 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10560)
	; 3520 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10561)
	; 3521 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10564)
	; 3522 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10566)
	; 3522 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10567)
	; 3523 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10570)
	; 3524 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10572)
	; 3524 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10573)
	; 3525 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10576)
	; 3526 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10578)
	; 3526 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10579)
	; 3527 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10582)
	; 3528 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10584)
	; 3528 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10585)
	; 3529 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 10586 to 10625
.if((n .mod 3 = 2) && ((n > 10585 && n < 10626)))
	; 3529..3543 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10586 && n < 10624)))
	; 3530..3542 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10588)
	; 3530 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10590)
	; 3530 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10591)
	; 3531 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10594)
	; 3532 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10596)
	; 3532 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10597)
	; 3533 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10600)
	; 3534 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10602)
	; 3534 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10603)
	; 3535 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10606)
	; 3536 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10608)
	; 3536 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10609)
	; 3537 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10612)
	; 3538 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10614)
	; 3538 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10615)
	; 3539 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10618)
	; 3540 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10620)
	; 3540 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10621)
	; 3541 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10624)
	; 3542 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 10626) && (n < 10938))
.if((n >= 10626) && (n < 10782))
.if((n >= 10626) && (n < 10704))
.if((n >= 10626) && (n < 10665))
; 10626 to 10664
.if(n = 10626)
	; 3542 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10627)
	; 3543 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 10627 && n < 10665)))
	; 3543..3555 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10628 && n < 10660)))
	; 3544..3554 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10630)
	; 3544 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10632)
	; 3544 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10633)
	; 3545 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10636)
	; 3546 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10638)
	; 3546 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10639)
	; 3547 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10642)
	; 3548 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10644)
	; 3548 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10645)
	; 3549 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10648)
	; 3550 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10650)
	; 3550 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10651)
	; 3551 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10654)
	; 3552 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10656)
	; 3552 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10657)
	; 3553 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10660)
	; 3554 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10662)
	; 3554 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10663)
	; 3555 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 10665 to 10703
.if((n .mod 6 = 3) && ((n > 10664 && n < 10702)))
	; 3556..3568 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10666)
	; 3556 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 10666 && n < 10704)))
	; 3557..3569 bytes
	delay_n (n-2)
	nop
.elseif(n = 10668)
	; 3556 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10669)
	; 3557 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10672)
	; 3558 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10674)
	; 3558 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10675)
	; 3559 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10678)
	; 3560 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10680)
	; 3560 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10681)
	; 3561 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10684)
	; 3562 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10686)
	; 3562 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10687)
	; 3563 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10690)
	; 3564 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10692)
	; 3564 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10693)
	; 3565 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10696)
	; 3566 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10698)
	; 3566 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10699)
	; 3567 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10702)
	; 3568 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 10704) && (n < 10743))
; 10704 to 10742
.if(n = 10704)
	; 3568 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10705)
	; 3569 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 10705 && n < 10743)))
	; 3569..3581 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10706 && n < 10738)))
	; 3570..3580 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10708)
	; 3570 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10710)
	; 3570 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10711)
	; 3571 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10714)
	; 3572 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10716)
	; 3572 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10717)
	; 3573 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10720)
	; 3574 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10722)
	; 3574 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10723)
	; 3575 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10726)
	; 3576 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10728)
	; 3576 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10729)
	; 3577 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10732)
	; 3578 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10734)
	; 3578 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10735)
	; 3579 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10738)
	; 3580 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10740)
	; 3580 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10741)
	; 3581 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 10743 to 10781
.if((n .mod 6 = 3) && ((n > 10742 && n < 10780)))
	; 3582..3594 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10744)
	; 3582 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 10744 && n < 10782)))
	; 3583..3595 bytes
	delay_n (n-2)
	nop
.elseif(n = 10746)
	; 3582 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10747)
	; 3583 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10750)
	; 3584 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10752)
	; 3584 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10753)
	; 3585 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10756)
	; 3586 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10758)
	; 3586 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10759)
	; 3587 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10762)
	; 3588 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10764)
	; 3588 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10765)
	; 3589 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10768)
	; 3590 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10770)
	; 3590 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10771)
	; 3591 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10774)
	; 3592 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10776)
	; 3592 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10777)
	; 3593 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10780)
	; 3594 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.else
.if((n >= 10782) && (n < 10860))
.if((n >= 10782) && (n < 10821))
; 10782 to 10820
.if(n = 10782)
	; 3594 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10783)
	; 3595 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 10783 && n < 10821)))
	; 3595..3607 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10784 && n < 10816)))
	; 3596..3606 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10786)
	; 3596 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10788)
	; 3596 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10789)
	; 3597 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10792)
	; 3598 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10794)
	; 3598 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10795)
	; 3599 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10798)
	; 3600 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10800)
	; 3600 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10801)
	; 3601 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10804)
	; 3602 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10806)
	; 3602 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10807)
	; 3603 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10810)
	; 3604 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10812)
	; 3604 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10813)
	; 3605 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10816)
	; 3606 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10818)
	; 3606 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10819)
	; 3607 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 10821 to 10859
.if((n .mod 6 = 3) && ((n > 10820 && n < 10858)))
	; 3608..3620 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10822)
	; 3608 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 10822 && n < 10860)))
	; 3609..3621 bytes
	delay_n (n-2)
	nop
.elseif(n = 10824)
	; 3608 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10825)
	; 3609 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10828)
	; 3610 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10830)
	; 3610 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10831)
	; 3611 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10834)
	; 3612 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10836)
	; 3612 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10837)
	; 3613 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10840)
	; 3614 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10842)
	; 3614 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10843)
	; 3615 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10846)
	; 3616 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10848)
	; 3616 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10849)
	; 3617 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10852)
	; 3618 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10854)
	; 3618 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10855)
	; 3619 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10858)
	; 3620 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 10860) && (n < 10899))
; 10860 to 10898
.if(n = 10860)
	; 3620 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10861)
	; 3621 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 10861 && n < 10899)))
	; 3621..3633 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10862 && n < 10894)))
	; 3622..3632 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10864)
	; 3622 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10866)
	; 3622 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10867)
	; 3623 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10870)
	; 3624 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10872)
	; 3624 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10873)
	; 3625 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10876)
	; 3626 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10878)
	; 3626 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10879)
	; 3627 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10882)
	; 3628 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10884)
	; 3628 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10885)
	; 3629 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10888)
	; 3630 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10890)
	; 3630 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10891)
	; 3631 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10894)
	; 3632 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10896)
	; 3632 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10897)
	; 3633 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 10899 to 10937
.if((n .mod 6 = 3) && ((n > 10898 && n < 10936)))
	; 3634..3646 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10900)
	; 3634 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 10900 && n < 10938)))
	; 3635..3647 bytes
	delay_n (n-2)
	nop
.elseif(n = 10902)
	; 3634 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10903)
	; 3635 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10906)
	; 3636 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10908)
	; 3636 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10909)
	; 3637 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10912)
	; 3638 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10914)
	; 3638 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10915)
	; 3639 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10918)
	; 3640 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10920)
	; 3640 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10921)
	; 3641 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10924)
	; 3642 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10926)
	; 3642 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10927)
	; 3643 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10930)
	; 3644 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10932)
	; 3644 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10933)
	; 3645 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10936)
	; 3646 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if(n = 10938)
	; 3646 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10939)
	; 3647 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 10939 && n < 10977)))
	; 3647..3659 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 10940 && n < 10972)))
	; 3648..3658 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10942)
	; 3648 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10944)
	; 3648 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10945)
	; 3649 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10948)
	; 3650 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10950)
	; 3650 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10951)
	; 3651 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 10954)
	; 3652 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10956)
	; 3652 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10957)
	; 3653 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10960)
	; 3654 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10962)
	; 3654 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10963)
	; 3655 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10966)
	; 3656 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10968)
	; 3656 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10969)
	; 3657 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 10972)
	; 3658 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10974)
	; 3658 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10975)
	; 3659 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 10977 to 11015
.if((n .mod 6 = 3) && ((n > 10976 && n < 11014)))
	; 3660..3672 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 10978)
	; 3660 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 10978 && n < 11016)))
	; 3661..3673 bytes
	delay_n (n-2)
	nop
.elseif(n = 10980)
	; 3660 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10981)
	; 3661 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10984)
	; 3662 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10986)
	; 3662 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10987)
	; 3663 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10990)
	; 3664 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10992)
	; 3664 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10993)
	; 3665 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10996)
	; 3666 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10998)
	; 3666 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10999)
	; 3667 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11002)
	; 3668 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11004)
	; 3668 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11005)
	; 3669 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11008)
	; 3670 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11010)
	; 3670 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11011)
	; 3671 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11014)
	; 3672 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
; 11016 to 11054
.if(n = 11016)
	; 3672 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11017)
	; 3673 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 11017 && n < 11055)))
	; 3673..3685 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11018 && n < 11050)))
	; 3674..3684 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11020)
	; 3674 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11022)
	; 3674 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11023)
	; 3675 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11026)
	; 3676 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11028)
	; 3676 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11029)
	; 3677 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11032)
	; 3678 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11034)
	; 3678 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11035)
	; 3679 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11038)
	; 3680 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11040)
	; 3680 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11041)
	; 3681 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11044)
	; 3682 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11046)
	; 3682 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11047)
	; 3683 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11050)
	; 3684 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11052)
	; 3684 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11053)
	; 3685 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 11055 to 11093
.if((n .mod 6 = 3) && ((n > 11054 && n < 11092)))
	; 3686..3698 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11056)
	; 3686 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 11056 && n < 11094)))
	; 3687..3699 bytes
	delay_n (n-2)
	nop
.elseif(n = 11058)
	; 3686 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11059)
	; 3687 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11062)
	; 3688 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11064)
	; 3688 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11065)
	; 3689 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11068)
	; 3690 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11070)
	; 3690 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11071)
	; 3691 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11074)
	; 3692 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11076)
	; 3692 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11077)
	; 3693 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11080)
	; 3694 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11082)
	; 3694 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11083)
	; 3695 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11086)
	; 3696 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11088)
	; 3696 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11089)
	; 3697 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11092)
	; 3698 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.else
.if((n >= 11094) && (n < 11172))
.if((n >= 11094) && (n < 11133))
; 11094 to 11132
.if(n = 11094)
	; 3698 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11095)
	; 3699 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 11095 && n < 11133)))
	; 3699..3711 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11096 && n < 11128)))
	; 3700..3710 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11098)
	; 3700 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11100)
	; 3700 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11101)
	; 3701 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11104)
	; 3702 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11106)
	; 3702 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11107)
	; 3703 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11110)
	; 3704 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11112)
	; 3704 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11113)
	; 3705 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11116)
	; 3706 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11118)
	; 3706 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11119)
	; 3707 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11122)
	; 3708 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11124)
	; 3708 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11125)
	; 3709 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11128)
	; 3710 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11130)
	; 3710 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11131)
	; 3711 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 11133 to 11171
.if((n .mod 6 = 3) && ((n > 11132 && n < 11170)))
	; 3712..3724 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11134)
	; 3712 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 11134 && n < 11172)))
	; 3713..3725 bytes
	delay_n (n-2)
	nop
.elseif(n = 11136)
	; 3712 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11137)
	; 3713 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11140)
	; 3714 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11142)
	; 3714 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11143)
	; 3715 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11146)
	; 3716 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11148)
	; 3716 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11149)
	; 3717 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11152)
	; 3718 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11154)
	; 3718 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11155)
	; 3719 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11158)
	; 3720 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11160)
	; 3720 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11161)
	; 3721 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11164)
	; 3722 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11166)
	; 3722 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11167)
	; 3723 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11170)
	; 3724 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 11172) && (n < 11211))
; 11172 to 11210
.if(n = 11172)
	; 3724 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11173)
	; 3725 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 11173 && n < 11211)))
	; 3725..3737 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11174 && n < 11206)))
	; 3726..3736 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11176)
	; 3726 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11178)
	; 3726 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11179)
	; 3727 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11182)
	; 3728 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11184)
	; 3728 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11185)
	; 3729 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11188)
	; 3730 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11190)
	; 3730 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11191)
	; 3731 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11194)
	; 3732 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11196)
	; 3732 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11197)
	; 3733 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11200)
	; 3734 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11202)
	; 3734 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11203)
	; 3735 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11206)
	; 3736 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11208)
	; 3736 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11209)
	; 3737 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 11211 to 11250
.if((n .mod 6 = 3) && ((n > 11210 && n < 11248)))
	; 3738..3750 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11212)
	; 3738 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 11212 && n < 11250)))
	; 3739..3751 bytes
	delay_n (n-2)
	nop
.elseif(n = 11214)
	; 3738 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11215)
	; 3739 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11218)
	; 3740 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11220)
	; 3740 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11221)
	; 3741 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11224)
	; 3742 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11226)
	; 3742 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11227)
	; 3743 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11230)
	; 3744 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11232)
	; 3744 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11233)
	; 3745 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11236)
	; 3746 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11238)
	; 3746 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11239)
	; 3747 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11242)
	; 3748 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11244)
	; 3748 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11245)
	; 3749 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11248)
	; 3750 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11250)
	; 3750 bytes
	delay_n 72
	delay_n (n-72)
.endif
.endif
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
; 11251 to 11289
.if(n = 11251)
	; 3751 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 11251 && n < 11289)))
	; 3751..3763 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11252 && n < 11290)))
	; 3752..3764 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11254)
	; 3752 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11256)
	; 3752 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11257)
	; 3753 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11260)
	; 3754 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11262)
	; 3754 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11263)
	; 3755 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11266)
	; 3756 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11268)
	; 3756 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11269)
	; 3757 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11272)
	; 3758 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11274)
	; 3758 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11275)
	; 3759 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11278)
	; 3760 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11280)
	; 3760 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11281)
	; 3761 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11284)
	; 3762 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11286)
	; 3762 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11287)
	; 3763 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 11290 to 11328
.if(n = 11290)
	; 3764 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 11290 && n < 11328)))
	; 3765..3777 bytes
	delay_n (n-2)
	nop
.elseif(n = 11292)
	; 3764 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11293)
	; 3765 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 11294 && n < 11326)))
	; 3766..3776 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11296)
	; 3766 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11298)
	; 3766 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11299)
	; 3767 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11302)
	; 3768 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11304)
	; 3768 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11305)
	; 3769 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11308)
	; 3770 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11310)
	; 3770 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11311)
	; 3771 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11314)
	; 3772 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11316)
	; 3772 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11317)
	; 3773 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11320)
	; 3774 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11322)
	; 3774 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11323)
	; 3775 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11326)
	; 3776 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11328)
	; 3776 bytes
	delay_n 96
	delay_n (n-96)
.endif
.endif
.else
.if((n >= 11329) && (n < 11368))
; 11329 to 11367
.if(n = 11329)
	; 3777 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 11329 && n < 11367)))
	; 3777..3789 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11330 && n < 11368)))
	; 3778..3790 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11332)
	; 3778 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11334)
	; 3778 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11335)
	; 3779 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11338)
	; 3780 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11340)
	; 3780 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11341)
	; 3781 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11344)
	; 3782 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11346)
	; 3782 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11347)
	; 3783 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11350)
	; 3784 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11352)
	; 3784 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11353)
	; 3785 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11356)
	; 3786 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11358)
	; 3786 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11359)
	; 3787 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11362)
	; 3788 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11364)
	; 3788 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11365)
	; 3789 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 11368 to 11406
.if(n = 11368)
	; 3790 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 11368 && n < 11406)))
	; 3791..3803 bytes
	delay_n (n-2)
	nop
.elseif(n = 11370)
	; 3790 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11371)
	; 3791 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 11372 && n < 11404)))
	; 3792..3802 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11374)
	; 3792 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11376)
	; 3792 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11377)
	; 3793 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11380)
	; 3794 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11382)
	; 3794 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11383)
	; 3795 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11386)
	; 3796 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11388)
	; 3796 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11389)
	; 3797 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11392)
	; 3798 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11394)
	; 3798 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11395)
	; 3799 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11398)
	; 3800 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11400)
	; 3800 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11401)
	; 3801 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11404)
	; 3802 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11406)
	; 3802 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
.if((n >= 11407) && (n < 11446))
; 11407 to 11445
.if(n = 11407)
	; 3803 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 11407 && n < 11445)))
	; 3803..3815 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11408 && n < 11446)))
	; 3804..3816 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11410)
	; 3804 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11412)
	; 3804 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11413)
	; 3805 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11416)
	; 3806 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11418)
	; 3806 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11419)
	; 3807 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11422)
	; 3808 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11424)
	; 3808 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11425)
	; 3809 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11428)
	; 3810 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11430)
	; 3810 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11431)
	; 3811 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11434)
	; 3812 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11436)
	; 3812 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11437)
	; 3813 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11440)
	; 3814 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11442)
	; 3814 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11443)
	; 3815 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 11446 to 11484
.if(n = 11446)
	; 3816 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 11446 && n < 11484)))
	; 3817..3829 bytes
	delay_n (n-2)
	nop
.elseif(n = 11448)
	; 3816 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11449)
	; 3817 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 11450 && n < 11482)))
	; 3818..3828 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11452)
	; 3818 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11454)
	; 3818 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11455)
	; 3819 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11458)
	; 3820 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11460)
	; 3820 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11461)
	; 3821 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11464)
	; 3822 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11466)
	; 3822 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11467)
	; 3823 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11470)
	; 3824 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11472)
	; 3824 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11473)
	; 3825 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11476)
	; 3826 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11478)
	; 3826 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11479)
	; 3827 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11482)
	; 3828 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11484)
	; 3828 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.else
.if((n >= 11485) && (n < 11524))
; 11485 to 11523
.if(n = 11485)
	; 3829 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 11485 && n < 11523)))
	; 3829..3841 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11486 && n < 11524)))
	; 3830..3842 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11488)
	; 3830 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11490)
	; 3830 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11491)
	; 3831 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11494)
	; 3832 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11496)
	; 3832 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11497)
	; 3833 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11500)
	; 3834 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11502)
	; 3834 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11503)
	; 3835 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11506)
	; 3836 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11508)
	; 3836 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11509)
	; 3837 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11512)
	; 3838 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11514)
	; 3838 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11515)
	; 3839 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11518)
	; 3840 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11520)
	; 3840 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11521)
	; 3841 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 11524 to 11562
.if(n = 11524)
	; 3842 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 11524 && n < 11562)))
	; 3843..3855 bytes
	delay_n (n-2)
	nop
.elseif(n = 11526)
	; 3842 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11527)
	; 3843 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 11528 && n < 11560)))
	; 3844..3854 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11530)
	; 3844 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11532)
	; 3844 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11533)
	; 3845 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11536)
	; 3846 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11538)
	; 3846 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11539)
	; 3847 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11542)
	; 3848 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11544)
	; 3848 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11545)
	; 3849 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11548)
	; 3850 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11550)
	; 3850 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11551)
	; 3851 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11554)
	; 3852 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11556)
	; 3852 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11557)
	; 3853 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11560)
	; 3854 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11562)
	; 3854 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
.if((n >= 11563) && (n < 11641))
.if((n >= 11563) && (n < 11602))
; 11563 to 11601
.if(n = 11563)
	; 3855 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 11563 && n < 11601)))
	; 3855..3867 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11564 && n < 11602)))
	; 3856..3868 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11566)
	; 3856 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11568)
	; 3856 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11569)
	; 3857 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11572)
	; 3858 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11574)
	; 3858 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11575)
	; 3859 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11578)
	; 3860 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11580)
	; 3860 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11581)
	; 3861 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11584)
	; 3862 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11586)
	; 3862 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11587)
	; 3863 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11590)
	; 3864 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11592)
	; 3864 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11593)
	; 3865 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11596)
	; 3866 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11598)
	; 3866 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11599)
	; 3867 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 11602 to 11640
.if(n = 11602)
	; 3868 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 11602 && n < 11640)))
	; 3869..3881 bytes
	delay_n (n-2)
	nop
.elseif(n = 11604)
	; 3868 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11605)
	; 3869 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 11606 && n < 11638)))
	; 3870..3880 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11608)
	; 3870 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11610)
	; 3870 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11611)
	; 3871 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11614)
	; 3872 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11616)
	; 3872 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11617)
	; 3873 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11620)
	; 3874 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11622)
	; 3874 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11623)
	; 3875 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11626)
	; 3876 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11628)
	; 3876 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11629)
	; 3877 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11632)
	; 3878 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11634)
	; 3878 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11635)
	; 3879 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11638)
	; 3880 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11640)
	; 3880 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.else
.if((n >= 11641) && (n < 11680))
; 11641 to 11679
.if(n = 11641)
	; 3881 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 11641 && n < 11679)))
	; 3881..3893 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11642 && n < 11680)))
	; 3882..3894 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11644)
	; 3882 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11646)
	; 3882 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11647)
	; 3883 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11650)
	; 3884 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11652)
	; 3884 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11653)
	; 3885 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11656)
	; 3886 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11658)
	; 3886 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11659)
	; 3887 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11662)
	; 3888 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11664)
	; 3888 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11665)
	; 3889 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11668)
	; 3890 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11670)
	; 3890 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11671)
	; 3891 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11674)
	; 3892 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11676)
	; 3892 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11677)
	; 3893 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 11680 to 11718
.if(n = 11680)
	; 3894 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 11680 && n < 11718)))
	; 3895..3907 bytes
	delay_n (n-2)
	nop
.elseif(n = 11682)
	; 3894 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11683)
	; 3895 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 11684 && n < 11716)))
	; 3896..3906 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11686)
	; 3896 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11688)
	; 3896 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11689)
	; 3897 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11692)
	; 3898 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11694)
	; 3898 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11695)
	; 3899 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11698)
	; 3900 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11700)
	; 3900 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11701)
	; 3901 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11704)
	; 3902 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11706)
	; 3902 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11707)
	; 3903 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11710)
	; 3904 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11712)
	; 3904 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11713)
	; 3905 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11716)
	; 3906 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11718)
	; 3906 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.endif
.endif
.else
.if((n >= 11719) && (n < 11797))
.if((n >= 11719) && (n < 11758))
; 11719 to 11757
.if(n = 11719)
	; 3907 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 11719 && n < 11757)))
	; 3907..3919 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11720 && n < 11758)))
	; 3908..3920 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11722)
	; 3908 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11724)
	; 3908 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11725)
	; 3909 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11728)
	; 3910 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11730)
	; 3910 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11731)
	; 3911 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11734)
	; 3912 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11736)
	; 3912 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11737)
	; 3913 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11740)
	; 3914 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11742)
	; 3914 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11743)
	; 3915 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11746)
	; 3916 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11748)
	; 3916 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11749)
	; 3917 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11752)
	; 3918 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11754)
	; 3918 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11755)
	; 3919 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 11758 to 11796
.if(n = 11758)
	; 3920 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 11758 && n < 11796)))
	; 3921..3933 bytes
	delay_n (n-2)
	nop
.elseif(n = 11760)
	; 3920 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11761)
	; 3921 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 11762 && n < 11794)))
	; 3922..3932 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11764)
	; 3922 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11766)
	; 3922 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11767)
	; 3923 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11770)
	; 3924 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11772)
	; 3924 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11773)
	; 3925 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11776)
	; 3926 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11778)
	; 3926 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11779)
	; 3927 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11782)
	; 3928 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11784)
	; 3928 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11785)
	; 3929 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11788)
	; 3930 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11790)
	; 3930 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11791)
	; 3931 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11794)
	; 3932 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11796)
	; 3932 bytes
	delay_n 24
	delay_n (n-24)
.endif
.endif
.else
.if((n >= 11797) && (n < 11836))
; 11797 to 11835
.if(n = 11797)
	; 3933 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 11797 && n < 11835)))
	; 3933..3945 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11798 && n < 11836)))
	; 3934..3946 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11800)
	; 3934 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11802)
	; 3934 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11803)
	; 3935 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11806)
	; 3936 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11808)
	; 3936 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11809)
	; 3937 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11812)
	; 3938 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11814)
	; 3938 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11815)
	; 3939 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11818)
	; 3940 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11820)
	; 3940 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11821)
	; 3941 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11824)
	; 3942 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11826)
	; 3942 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11827)
	; 3943 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11830)
	; 3944 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11832)
	; 3944 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11833)
	; 3945 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 11836 to 11875
.if(n = 11836)
	; 3946 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 11836 && n < 11874)))
	; 3947..3959 bytes
	delay_n (n-2)
	nop
.elseif(n = 11838)
	; 3946 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11839)
	; 3947 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 11840 && n < 11872)))
	; 3948..3958 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11842)
	; 3948 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11844)
	; 3948 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11845)
	; 3949 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11848)
	; 3950 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11850)
	; 3950 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11851)
	; 3951 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11854)
	; 3952 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11856)
	; 3952 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11857)
	; 3953 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11860)
	; 3954 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11862)
	; 3954 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11863)
	; 3955 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11866)
	; 3956 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11868)
	; 3956 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11869)
	; 3957 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11872)
	; 3958 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11874)
	; 3958 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11875)
	; 3959 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11876) && (n < 12188))
.if((n >= 11876) && (n < 12032))
.if((n >= 11876) && (n < 11954))
.if((n >= 11876) && (n < 11915))
; 11876 to 11914
.if((n .mod 3 = 2) && ((n > 11875 && n < 11913)))
	; 3959..3971 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11876 && n < 11914)))
	; 3960..3972 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11878)
	; 3960 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11880)
	; 3960 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11881)
	; 3961 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11884)
	; 3962 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11886)
	; 3962 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11887)
	; 3963 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11890)
	; 3964 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11892)
	; 3964 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11893)
	; 3965 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11896)
	; 3966 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11898)
	; 3966 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11899)
	; 3967 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11902)
	; 3968 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11904)
	; 3968 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11905)
	; 3969 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11908)
	; 3970 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11910)
	; 3970 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11911)
	; 3971 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11914)
	; 3972 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 11915 to 11953
.if((n .mod 3 = 2) && ((n > 11914 && n < 11952)))
	; 3973..3985 bytes
	delay_n (n-2)
	nop
.elseif(n = 11916)
	; 3972 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11917)
	; 3973 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 11918 && n < 11950)))
	; 3974..3984 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11920)
	; 3974 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11922)
	; 3974 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11923)
	; 3975 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11926)
	; 3976 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11928)
	; 3976 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11929)
	; 3977 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11932)
	; 3978 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11934)
	; 3978 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11935)
	; 3979 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11938)
	; 3980 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11940)
	; 3980 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11941)
	; 3981 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 11944)
	; 3982 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11946)
	; 3982 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11947)
	; 3983 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11950)
	; 3984 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11952)
	; 3984 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11953)
	; 3985 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
.if((n >= 11954) && (n < 11993))
; 11954 to 11992
.if((n .mod 3 = 2) && ((n > 11953 && n < 11991)))
	; 3985..3997 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 11954 && n < 11992)))
	; 3986..3998 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11956)
	; 3986 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11958)
	; 3986 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11959)
	; 3987 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11962)
	; 3988 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11964)
	; 3988 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11965)
	; 3989 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11968)
	; 3990 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11970)
	; 3990 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11971)
	; 3991 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11974)
	; 3992 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11976)
	; 3992 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11977)
	; 3993 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 11980)
	; 3994 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11982)
	; 3994 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11983)
	; 3995 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11986)
	; 3996 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11988)
	; 3996 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11989)
	; 3997 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11992)
	; 3998 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 11993 to 12031
.if((n .mod 3 = 2) && ((n > 11992 && n < 12030)))
	; 3999..4011 bytes
	delay_n (n-2)
	nop
.elseif(n = 11994)
	; 3998 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11995)
	; 3999 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 11996 && n < 12028)))
	; 4000..4010 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 11998)
	; 4000 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12000)
	; 4000 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12001)
	; 4001 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12004)
	; 4002 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12006)
	; 4002 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12007)
	; 4003 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12010)
	; 4004 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12012)
	; 4004 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12013)
	; 4005 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12016)
	; 4006 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12018)
	; 4006 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12019)
	; 4007 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12022)
	; 4008 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12024)
	; 4008 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12025)
	; 4009 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12028)
	; 4010 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12030)
	; 4010 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12031)
	; 4011 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
; 12032 to 12070
.if((n .mod 3 = 2) && ((n > 12031 && n < 12069)))
	; 4011..4023 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12032 && n < 12070)))
	; 4012..4024 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12034)
	; 4012 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12036)
	; 4012 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12037)
	; 4013 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12040)
	; 4014 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12042)
	; 4014 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12043)
	; 4015 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12046)
	; 4016 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12048)
	; 4016 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12049)
	; 4017 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12052)
	; 4018 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12054)
	; 4018 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12055)
	; 4019 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12058)
	; 4020 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12060)
	; 4020 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12061)
	; 4021 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12064)
	; 4022 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12066)
	; 4022 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12067)
	; 4023 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12070)
	; 4024 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 12071 to 12109
.if((n .mod 3 = 2) && ((n > 12070 && n < 12108)))
	; 4025..4037 bytes
	delay_n (n-2)
	nop
.elseif(n = 12072)
	; 4024 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12073)
	; 4025 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 12074 && n < 12106)))
	; 4026..4036 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12076)
	; 4026 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12078)
	; 4026 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12079)
	; 4027 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12082)
	; 4028 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12084)
	; 4028 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12085)
	; 4029 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12088)
	; 4030 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12090)
	; 4030 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12091)
	; 4031 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12094)
	; 4032 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12096)
	; 4032 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12097)
	; 4033 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12100)
	; 4034 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12102)
	; 4034 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12103)
	; 4035 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12106)
	; 4036 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12108)
	; 4036 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12109)
	; 4037 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 12110) && (n < 12149))
; 12110 to 12148
.if((n .mod 3 = 2) && ((n > 12109 && n < 12147)))
	; 4037..4049 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12110 && n < 12148)))
	; 4038..4050 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12112)
	; 4038 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12114)
	; 4038 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12115)
	; 4039 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12118)
	; 4040 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12120)
	; 4040 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12121)
	; 4041 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12124)
	; 4042 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12126)
	; 4042 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12127)
	; 4043 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12130)
	; 4044 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12132)
	; 4044 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12133)
	; 4045 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12136)
	; 4046 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12138)
	; 4046 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12139)
	; 4047 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12142)
	; 4048 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12144)
	; 4048 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12145)
	; 4049 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12148)
	; 4050 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 12149 to 12187
.if((n .mod 3 = 2) && ((n > 12148 && n < 12186)))
	; 4051..4063 bytes
	delay_n (n-2)
	nop
.elseif(n = 12150)
	; 4050 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12151)
	; 4051 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 12152 && n < 12184)))
	; 4052..4062 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12154)
	; 4052 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12156)
	; 4052 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12157)
	; 4053 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12160)
	; 4054 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12162)
	; 4054 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12163)
	; 4055 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12166)
	; 4056 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12168)
	; 4056 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12169)
	; 4057 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12172)
	; 4058 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12174)
	; 4058 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12175)
	; 4059 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12178)
	; 4060 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12180)
	; 4060 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12181)
	; 4061 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12184)
	; 4062 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12186)
	; 4062 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12187)
	; 4063 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
.if((n >= 12188) && (n < 12227))
; 12188 to 12226
.if((n .mod 3 = 2) && ((n > 12187 && n < 12225)))
	; 4063..4075 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12188 && n < 12226)))
	; 4064..4076 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12190)
	; 4064 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12192)
	; 4064 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12193)
	; 4065 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12196)
	; 4066 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12198)
	; 4066 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12199)
	; 4067 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12202)
	; 4068 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12204)
	; 4068 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12205)
	; 4069 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12208)
	; 4070 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12210)
	; 4070 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12211)
	; 4071 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12214)
	; 4072 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12216)
	; 4072 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12217)
	; 4073 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12220)
	; 4074 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12222)
	; 4074 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12223)
	; 4075 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12226)
	; 4076 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 12227 to 12265
.if((n .mod 3 = 2) && ((n > 12226 && n < 12264)))
	; 4077..4089 bytes
	delay_n (n-2)
	nop
.elseif(n = 12228)
	; 4076 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12229)
	; 4077 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 12230 && n < 12262)))
	; 4078..4088 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12232)
	; 4078 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12234)
	; 4078 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12235)
	; 4079 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12238)
	; 4080 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12240)
	; 4080 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12241)
	; 4081 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12244)
	; 4082 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12246)
	; 4082 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12247)
	; 4083 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12250)
	; 4084 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12252)
	; 4084 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12253)
	; 4085 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12256)
	; 4086 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12258)
	; 4086 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12259)
	; 4087 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12262)
	; 4088 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12264)
	; 4088 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12265)
	; 4089 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.else
.if((n >= 12266) && (n < 12305))
; 12266 to 12304
.if((n .mod 3 = 2) && ((n > 12265 && n < 12303)))
	; 4089..4101 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12266 && n < 12304)))
	; 4090..4102 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12268)
	; 4090 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12270)
	; 4090 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12271)
	; 4091 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12274)
	; 4092 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12276)
	; 4092 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12277)
	; 4093 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12280)
	; 4094 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12282)
	; 4094 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12283)
	; 4095 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12286)
	; 4096 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12288)
	; 4096 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12289)
	; 4097 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12292)
	; 4098 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12294)
	; 4098 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12295)
	; 4099 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12298)
	; 4100 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12300)
	; 4100 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12301)
	; 4101 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12304)
	; 4102 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 12305 to 12343
.if((n .mod 3 = 2) && ((n > 12304 && n < 12342)))
	; 4103..4115 bytes
	delay_n (n-2)
	nop
.elseif(n = 12306)
	; 4102 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12307)
	; 4103 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 12308 && n < 12340)))
	; 4104..4114 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12310)
	; 4104 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12312)
	; 4104 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12313)
	; 4105 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12316)
	; 4106 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12318)
	; 4106 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12319)
	; 4107 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12322)
	; 4108 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12324)
	; 4108 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12325)
	; 4109 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12328)
	; 4110 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12330)
	; 4110 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12331)
	; 4111 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12334)
	; 4112 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12336)
	; 4112 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12337)
	; 4113 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12340)
	; 4114 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12342)
	; 4114 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12343)
	; 4115 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.endif
.else
.if((n >= 12344) && (n < 12422))
.if((n >= 12344) && (n < 12383))
; 12344 to 12382
.if((n .mod 3 = 2) && ((n > 12343 && n < 12381)))
	; 4115..4127 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12344 && n < 12382)))
	; 4116..4128 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12346)
	; 4116 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12348)
	; 4116 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12349)
	; 4117 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12352)
	; 4118 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12354)
	; 4118 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12355)
	; 4119 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12358)
	; 4120 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12360)
	; 4120 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12361)
	; 4121 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12364)
	; 4122 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12366)
	; 4122 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12367)
	; 4123 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12370)
	; 4124 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12372)
	; 4124 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12373)
	; 4125 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12376)
	; 4126 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12378)
	; 4126 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12379)
	; 4127 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12382)
	; 4128 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 12383 to 12421
.if((n .mod 3 = 2) && ((n > 12382 && n < 12420)))
	; 4129..4141 bytes
	delay_n (n-2)
	nop
.elseif(n = 12384)
	; 4128 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12385)
	; 4129 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 12386 && n < 12418)))
	; 4130..4140 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12388)
	; 4130 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12390)
	; 4130 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12391)
	; 4131 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12394)
	; 4132 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12396)
	; 4132 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12397)
	; 4133 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12400)
	; 4134 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12402)
	; 4134 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12403)
	; 4135 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12406)
	; 4136 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12408)
	; 4136 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12409)
	; 4137 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12412)
	; 4138 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12414)
	; 4138 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12415)
	; 4139 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12418)
	; 4140 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12420)
	; 4140 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12421)
	; 4141 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 12422) && (n < 12461))
; 12422 to 12460
.if((n .mod 3 = 2) && ((n > 12421 && n < 12459)))
	; 4141..4153 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12422 && n < 12460)))
	; 4142..4154 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12424)
	; 4142 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12426)
	; 4142 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12427)
	; 4143 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12430)
	; 4144 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12432)
	; 4144 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12433)
	; 4145 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12436)
	; 4146 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12438)
	; 4146 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12439)
	; 4147 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12442)
	; 4148 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12444)
	; 4148 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12445)
	; 4149 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12448)
	; 4150 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12450)
	; 4150 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12451)
	; 4151 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12454)
	; 4152 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12456)
	; 4152 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12457)
	; 4153 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12460)
	; 4154 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 12461 to 12500
.if((n .mod 3 = 2) && ((n > 12460 && n < 12501)))
	; 4155..4167 bytes
	delay_n (n-2)
	nop
.elseif(n = 12462)
	; 4154 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12463)
	; 4155 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 12464 && n < 12496)))
	; 4156..4166 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12466)
	; 4156 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12468)
	; 4156 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12469)
	; 4157 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12472)
	; 4158 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12474)
	; 4158 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12475)
	; 4159 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12478)
	; 4160 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12480)
	; 4160 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12481)
	; 4161 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12484)
	; 4162 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12486)
	; 4162 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12487)
	; 4163 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12490)
	; 4164 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12492)
	; 4164 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12493)
	; 4165 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12496)
	; 4166 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12498)
	; 4166 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12499)
	; 4167 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
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
.if((n .mod 6 = 3) && ((n > 12500 && n < 12538)))
	; 4168..4180 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12502)
	; 4168 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 12502 && n < 12540)))
	; 4169..4181 bytes
	delay_n (n-2)
	nop
.elseif(n = 12504)
	; 4168 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12505)
	; 4169 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12508)
	; 4170 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12510)
	; 4170 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12511)
	; 4171 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12514)
	; 4172 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12516)
	; 4172 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12517)
	; 4173 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12520)
	; 4174 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12522)
	; 4174 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12523)
	; 4175 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12526)
	; 4176 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12528)
	; 4176 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12529)
	; 4177 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12532)
	; 4178 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12534)
	; 4178 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12535)
	; 4179 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12538)
	; 4180 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 12540 to 12578
.if(n = 12540)
	; 4180 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12541)
	; 4181 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 12541 && n < 12579)))
	; 4181..4193 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12542 && n < 12574)))
	; 4182..4192 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12544)
	; 4182 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12546)
	; 4182 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12547)
	; 4183 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12550)
	; 4184 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12552)
	; 4184 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12553)
	; 4185 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12556)
	; 4186 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12558)
	; 4186 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12559)
	; 4187 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12562)
	; 4188 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12564)
	; 4188 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12565)
	; 4189 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12568)
	; 4190 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12570)
	; 4190 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12571)
	; 4191 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12574)
	; 4192 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12576)
	; 4192 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12577)
	; 4193 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 12579) && (n < 12618))
; 12579 to 12617
.if((n .mod 6 = 3) && ((n > 12578 && n < 12616)))
	; 4194..4206 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12580)
	; 4194 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 12580 && n < 12618)))
	; 4195..4207 bytes
	delay_n (n-2)
	nop
.elseif(n = 12582)
	; 4194 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12583)
	; 4195 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12586)
	; 4196 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12588)
	; 4196 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12589)
	; 4197 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12592)
	; 4198 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12594)
	; 4198 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12595)
	; 4199 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12598)
	; 4200 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12600)
	; 4200 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12601)
	; 4201 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12604)
	; 4202 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12606)
	; 4202 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12607)
	; 4203 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12610)
	; 4204 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12612)
	; 4204 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12613)
	; 4205 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12616)
	; 4206 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 12618 to 12656
.if(n = 12618)
	; 4206 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12619)
	; 4207 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 12619 && n < 12657)))
	; 4207..4219 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12620 && n < 12652)))
	; 4208..4218 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12622)
	; 4208 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12624)
	; 4208 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12625)
	; 4209 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12628)
	; 4210 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12630)
	; 4210 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12631)
	; 4211 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12634)
	; 4212 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12636)
	; 4212 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12637)
	; 4213 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12640)
	; 4214 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12642)
	; 4214 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12643)
	; 4215 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12646)
	; 4216 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12648)
	; 4216 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12649)
	; 4217 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12652)
	; 4218 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12654)
	; 4218 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12655)
	; 4219 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
.if((n >= 12657) && (n < 12696))
; 12657 to 12695
.if((n .mod 6 = 3) && ((n > 12656 && n < 12694)))
	; 4220..4232 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12658)
	; 4220 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 12658 && n < 12696)))
	; 4221..4233 bytes
	delay_n (n-2)
	nop
.elseif(n = 12660)
	; 4220 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12661)
	; 4221 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12664)
	; 4222 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12666)
	; 4222 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12667)
	; 4223 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12670)
	; 4224 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12672)
	; 4224 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12673)
	; 4225 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12676)
	; 4226 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12678)
	; 4226 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12679)
	; 4227 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12682)
	; 4228 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12684)
	; 4228 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12685)
	; 4229 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12688)
	; 4230 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12690)
	; 4230 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12691)
	; 4231 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12694)
	; 4232 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 12696 to 12734
.if(n = 12696)
	; 4232 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12697)
	; 4233 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 12697 && n < 12735)))
	; 4233..4245 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12698 && n < 12730)))
	; 4234..4244 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12700)
	; 4234 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12702)
	; 4234 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12703)
	; 4235 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12706)
	; 4236 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12708)
	; 4236 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12709)
	; 4237 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12712)
	; 4238 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12714)
	; 4238 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12715)
	; 4239 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12718)
	; 4240 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12720)
	; 4240 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12721)
	; 4241 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12724)
	; 4242 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12726)
	; 4242 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12727)
	; 4243 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12730)
	; 4244 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12732)
	; 4244 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12733)
	; 4245 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.else
.if((n >= 12735) && (n < 12774))
; 12735 to 12773
.if((n .mod 6 = 3) && ((n > 12734 && n < 12772)))
	; 4246..4258 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12736)
	; 4246 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 12736 && n < 12774)))
	; 4247..4259 bytes
	delay_n (n-2)
	nop
.elseif(n = 12738)
	; 4246 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12739)
	; 4247 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12742)
	; 4248 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12744)
	; 4248 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12745)
	; 4249 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12748)
	; 4250 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12750)
	; 4250 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12751)
	; 4251 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12754)
	; 4252 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12756)
	; 4252 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12757)
	; 4253 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12760)
	; 4254 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12762)
	; 4254 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12763)
	; 4255 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12766)
	; 4256 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12768)
	; 4256 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12769)
	; 4257 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12772)
	; 4258 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 12774 to 12812
.if(n = 12774)
	; 4258 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12775)
	; 4259 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 12775 && n < 12813)))
	; 4259..4271 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12776 && n < 12808)))
	; 4260..4270 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12778)
	; 4260 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12780)
	; 4260 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12781)
	; 4261 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12784)
	; 4262 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12786)
	; 4262 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12787)
	; 4263 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12790)
	; 4264 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12792)
	; 4264 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12793)
	; 4265 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12796)
	; 4266 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12798)
	; 4266 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12799)
	; 4267 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12802)
	; 4268 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12804)
	; 4268 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12805)
	; 4269 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12808)
	; 4270 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12810)
	; 4270 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12811)
	; 4271 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.endif
.else
.if((n >= 12813) && (n < 12969))
.if((n >= 12813) && (n < 12891))
.if((n >= 12813) && (n < 12852))
; 12813 to 12851
.if((n .mod 6 = 3) && ((n > 12812 && n < 12850)))
	; 4272..4284 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12814)
	; 4272 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 12814 && n < 12852)))
	; 4273..4285 bytes
	delay_n (n-2)
	nop
.elseif(n = 12816)
	; 4272 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12817)
	; 4273 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12820)
	; 4274 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12822)
	; 4274 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12823)
	; 4275 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12826)
	; 4276 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12828)
	; 4276 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12829)
	; 4277 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12832)
	; 4278 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12834)
	; 4278 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12835)
	; 4279 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12838)
	; 4280 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12840)
	; 4280 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12841)
	; 4281 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12844)
	; 4282 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12846)
	; 4282 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12847)
	; 4283 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12850)
	; 4284 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 12852 to 12890
.if(n = 12852)
	; 4284 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12853)
	; 4285 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 12853 && n < 12891)))
	; 4285..4297 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12854 && n < 12886)))
	; 4286..4296 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12856)
	; 4286 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12858)
	; 4286 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12859)
	; 4287 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12862)
	; 4288 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12864)
	; 4288 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12865)
	; 4289 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12868)
	; 4290 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12870)
	; 4290 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12871)
	; 4291 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12874)
	; 4292 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12876)
	; 4292 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12877)
	; 4293 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12880)
	; 4294 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12882)
	; 4294 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12883)
	; 4295 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12886)
	; 4296 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12888)
	; 4296 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12889)
	; 4297 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 12891) && (n < 12930))
; 12891 to 12929
.if((n .mod 6 = 3) && ((n > 12890 && n < 12928)))
	; 4298..4310 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12892)
	; 4298 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 12892 && n < 12930)))
	; 4299..4311 bytes
	delay_n (n-2)
	nop
.elseif(n = 12894)
	; 4298 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12895)
	; 4299 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12898)
	; 4300 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12900)
	; 4300 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12901)
	; 4301 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12904)
	; 4302 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12906)
	; 4302 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12907)
	; 4303 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12910)
	; 4304 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12912)
	; 4304 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12913)
	; 4305 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 12916)
	; 4306 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12918)
	; 4306 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12919)
	; 4307 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12922)
	; 4308 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12924)
	; 4308 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12925)
	; 4309 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12928)
	; 4310 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 12930 to 12968
.if(n = 12930)
	; 4310 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12931)
	; 4311 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 12931 && n < 12969)))
	; 4311..4323 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 12932 && n < 12964)))
	; 4312..4322 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12934)
	; 4312 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12936)
	; 4312 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12937)
	; 4313 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12940)
	; 4314 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12942)
	; 4314 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12943)
	; 4315 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12946)
	; 4316 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12948)
	; 4316 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12949)
	; 4317 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 12952)
	; 4318 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12954)
	; 4318 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12955)
	; 4319 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12958)
	; 4320 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12960)
	; 4320 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12961)
	; 4321 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12964)
	; 4322 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12966)
	; 4322 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12967)
	; 4323 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.endif
.else
.if((n >= 12969) && (n < 13047))
.if((n >= 12969) && (n < 13008))
; 12969 to 13007
.if((n .mod 6 = 3) && ((n > 12968 && n < 13006)))
	; 4324..4336 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 12970)
	; 4324 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 12970 && n < 13008)))
	; 4325..4337 bytes
	delay_n (n-2)
	nop
.elseif(n = 12972)
	; 4324 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12973)
	; 4325 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12976)
	; 4326 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12978)
	; 4326 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12979)
	; 4327 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12982)
	; 4328 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12984)
	; 4328 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12985)
	; 4329 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12988)
	; 4330 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12990)
	; 4330 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12991)
	; 4331 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12994)
	; 4332 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12996)
	; 4332 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12997)
	; 4333 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13000)
	; 4334 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13002)
	; 4334 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13003)
	; 4335 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13006)
	; 4336 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 13008 to 13046
.if(n = 13008)
	; 4336 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13009)
	; 4337 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 13009 && n < 13047)))
	; 4337..4349 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13010 && n < 13042)))
	; 4338..4348 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13012)
	; 4338 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13014)
	; 4338 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13015)
	; 4339 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13018)
	; 4340 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13020)
	; 4340 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13021)
	; 4341 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13024)
	; 4342 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13026)
	; 4342 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13027)
	; 4343 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13030)
	; 4344 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13032)
	; 4344 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13033)
	; 4345 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13036)
	; 4346 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13038)
	; 4346 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13039)
	; 4347 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13042)
	; 4348 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13044)
	; 4348 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13045)
	; 4349 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.else
.if((n >= 13047) && (n < 13086))
; 13047 to 13085
.if((n .mod 6 = 3) && ((n > 13046 && n < 13084)))
	; 4350..4362 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13048)
	; 4350 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 13048 && n < 13086)))
	; 4351..4363 bytes
	delay_n (n-2)
	nop
.elseif(n = 13050)
	; 4350 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13051)
	; 4351 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13054)
	; 4352 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13056)
	; 4352 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13057)
	; 4353 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13060)
	; 4354 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13062)
	; 4354 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13063)
	; 4355 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13066)
	; 4356 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13068)
	; 4356 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13069)
	; 4357 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13072)
	; 4358 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13074)
	; 4358 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13075)
	; 4359 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13078)
	; 4360 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13080)
	; 4360 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13081)
	; 4361 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13084)
	; 4362 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 13086 to 13125
.if(n = 13086)
	; 4362 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13087)
	; 4363 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 13087 && n < 13125)))
	; 4363..4375 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13088 && n < 13126)))
	; 4364..4376 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13090)
	; 4364 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13092)
	; 4364 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13093)
	; 4365 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13096)
	; 4366 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13098)
	; 4366 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13099)
	; 4367 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13102)
	; 4368 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13104)
	; 4368 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13105)
	; 4369 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13108)
	; 4370 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13110)
	; 4370 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13111)
	; 4371 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13114)
	; 4372 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13116)
	; 4372 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13117)
	; 4373 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13120)
	; 4374 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13122)
	; 4374 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13123)
	; 4375 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 13126) && (n < 13438))
.if((n >= 13126) && (n < 13282))
.if((n >= 13126) && (n < 13204))
.if((n >= 13126) && (n < 13165))
; 13126 to 13164
.if(n = 13126)
	; 4376 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 13126 && n < 13164)))
	; 4377..4389 bytes
	delay_n (n-2)
	nop
.elseif(n = 13128)
	; 4376 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13129)
	; 4377 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 13130 && n < 13162)))
	; 4378..4388 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13132)
	; 4378 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13134)
	; 4378 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13135)
	; 4379 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13138)
	; 4380 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13140)
	; 4380 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13141)
	; 4381 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13144)
	; 4382 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13146)
	; 4382 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13147)
	; 4383 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13150)
	; 4384 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13152)
	; 4384 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13153)
	; 4385 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13156)
	; 4386 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13158)
	; 4386 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13159)
	; 4387 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13162)
	; 4388 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13164)
	; 4388 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 13165 to 13203
.if(n = 13165)
	; 4389 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 13165 && n < 13203)))
	; 4389..4401 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13166 && n < 13204)))
	; 4390..4402 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13168)
	; 4390 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13170)
	; 4390 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13171)
	; 4391 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13174)
	; 4392 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13176)
	; 4392 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13177)
	; 4393 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13180)
	; 4394 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13182)
	; 4394 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13183)
	; 4395 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13186)
	; 4396 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13188)
	; 4396 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13189)
	; 4397 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13192)
	; 4398 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13194)
	; 4398 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13195)
	; 4399 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13198)
	; 4400 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13200)
	; 4400 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13201)
	; 4401 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 13204) && (n < 13243))
; 13204 to 13242
.if(n = 13204)
	; 4402 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 13204 && n < 13242)))
	; 4403..4415 bytes
	delay_n (n-2)
	nop
.elseif(n = 13206)
	; 4402 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13207)
	; 4403 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 13208 && n < 13240)))
	; 4404..4414 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13210)
	; 4404 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13212)
	; 4404 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13213)
	; 4405 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13216)
	; 4406 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13218)
	; 4406 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13219)
	; 4407 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13222)
	; 4408 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13224)
	; 4408 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13225)
	; 4409 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13228)
	; 4410 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13230)
	; 4410 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13231)
	; 4411 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13234)
	; 4412 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13236)
	; 4412 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13237)
	; 4413 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13240)
	; 4414 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13242)
	; 4414 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 13243 to 13281
.if(n = 13243)
	; 4415 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 13243 && n < 13281)))
	; 4415..4427 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13244 && n < 13282)))
	; 4416..4428 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13246)
	; 4416 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13248)
	; 4416 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13249)
	; 4417 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13252)
	; 4418 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13254)
	; 4418 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13255)
	; 4419 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13258)
	; 4420 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13260)
	; 4420 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13261)
	; 4421 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13264)
	; 4422 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13266)
	; 4422 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13267)
	; 4423 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13270)
	; 4424 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13272)
	; 4424 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13273)
	; 4425 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13276)
	; 4426 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13278)
	; 4426 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13279)
	; 4427 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.else
.if((n >= 13282) && (n < 13360))
.if((n >= 13282) && (n < 13321))
; 13282 to 13320
.if(n = 13282)
	; 4428 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 13282 && n < 13320)))
	; 4429..4441 bytes
	delay_n (n-2)
	nop
.elseif(n = 13284)
	; 4428 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13285)
	; 4429 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 13286 && n < 13318)))
	; 4430..4440 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13288)
	; 4430 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13290)
	; 4430 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13291)
	; 4431 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13294)
	; 4432 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13296)
	; 4432 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13297)
	; 4433 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13300)
	; 4434 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13302)
	; 4434 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13303)
	; 4435 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13306)
	; 4436 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13308)
	; 4436 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13309)
	; 4437 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13312)
	; 4438 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13314)
	; 4438 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13315)
	; 4439 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13318)
	; 4440 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13320)
	; 4440 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 13321 to 13359
.if(n = 13321)
	; 4441 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 13321 && n < 13359)))
	; 4441..4453 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13322 && n < 13360)))
	; 4442..4454 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13324)
	; 4442 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13326)
	; 4442 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13327)
	; 4443 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13330)
	; 4444 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13332)
	; 4444 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13333)
	; 4445 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13336)
	; 4446 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13338)
	; 4446 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13339)
	; 4447 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13342)
	; 4448 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13344)
	; 4448 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13345)
	; 4449 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13348)
	; 4450 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13350)
	; 4450 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13351)
	; 4451 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13354)
	; 4452 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13356)
	; 4452 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13357)
	; 4453 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
.if((n >= 13360) && (n < 13399))
; 13360 to 13398
.if(n = 13360)
	; 4454 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 13360 && n < 13398)))
	; 4455..4467 bytes
	delay_n (n-2)
	nop
.elseif(n = 13362)
	; 4454 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13363)
	; 4455 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 13364 && n < 13396)))
	; 4456..4466 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13366)
	; 4456 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13368)
	; 4456 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13369)
	; 4457 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13372)
	; 4458 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13374)
	; 4458 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13375)
	; 4459 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13378)
	; 4460 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13380)
	; 4460 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13381)
	; 4461 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13384)
	; 4462 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13386)
	; 4462 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13387)
	; 4463 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13390)
	; 4464 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13392)
	; 4464 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13393)
	; 4465 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13396)
	; 4466 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13398)
	; 4466 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 13399 to 13437
.if(n = 13399)
	; 4467 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 13399 && n < 13437)))
	; 4467..4479 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13400 && n < 13438)))
	; 4468..4480 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13402)
	; 4468 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13404)
	; 4468 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13405)
	; 4469 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13408)
	; 4470 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13410)
	; 4470 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13411)
	; 4471 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13414)
	; 4472 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13416)
	; 4472 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13417)
	; 4473 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13420)
	; 4474 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13422)
	; 4474 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13423)
	; 4475 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13426)
	; 4476 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13428)
	; 4476 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13429)
	; 4477 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13432)
	; 4478 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13434)
	; 4478 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13435)
	; 4479 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
.if((n >= 13438) && (n < 13477))
; 13438 to 13476
.if(n = 13438)
	; 4480 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 13438 && n < 13476)))
	; 4481..4493 bytes
	delay_n (n-2)
	nop
.elseif(n = 13440)
	; 4480 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13441)
	; 4481 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 13442 && n < 13474)))
	; 4482..4492 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13444)
	; 4482 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13446)
	; 4482 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13447)
	; 4483 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13450)
	; 4484 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13452)
	; 4484 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13453)
	; 4485 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13456)
	; 4486 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13458)
	; 4486 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13459)
	; 4487 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13462)
	; 4488 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13464)
	; 4488 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13465)
	; 4489 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13468)
	; 4490 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13470)
	; 4490 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13471)
	; 4491 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13474)
	; 4492 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13476)
	; 4492 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 13477 to 13515
.if(n = 13477)
	; 4493 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 13477 && n < 13515)))
	; 4493..4505 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13478 && n < 13516)))
	; 4494..4506 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13480)
	; 4494 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13482)
	; 4494 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13483)
	; 4495 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13486)
	; 4496 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13488)
	; 4496 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13489)
	; 4497 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13492)
	; 4498 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13494)
	; 4498 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13495)
	; 4499 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13498)
	; 4500 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13500)
	; 4500 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13501)
	; 4501 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13504)
	; 4502 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13506)
	; 4502 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13507)
	; 4503 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13510)
	; 4504 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13512)
	; 4504 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13513)
	; 4505 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 13516) && (n < 13555))
; 13516 to 13554
.if(n = 13516)
	; 4506 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 13516 && n < 13554)))
	; 4507..4519 bytes
	delay_n (n-2)
	nop
.elseif(n = 13518)
	; 4506 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13519)
	; 4507 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 13520 && n < 13552)))
	; 4508..4518 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13522)
	; 4508 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13524)
	; 4508 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13525)
	; 4509 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13528)
	; 4510 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13530)
	; 4510 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13531)
	; 4511 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13534)
	; 4512 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13536)
	; 4512 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13537)
	; 4513 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13540)
	; 4514 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13542)
	; 4514 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13543)
	; 4515 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13546)
	; 4516 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13548)
	; 4516 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13549)
	; 4517 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13552)
	; 4518 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13554)
	; 4518 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 13555 to 13593
.if(n = 13555)
	; 4519 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 13555 && n < 13593)))
	; 4519..4531 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13556 && n < 13594)))
	; 4520..4532 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13558)
	; 4520 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13560)
	; 4520 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13561)
	; 4521 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13564)
	; 4522 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13566)
	; 4522 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13567)
	; 4523 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13570)
	; 4524 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13572)
	; 4524 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13573)
	; 4525 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13576)
	; 4526 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13578)
	; 4526 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13579)
	; 4527 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13582)
	; 4528 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13584)
	; 4528 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13585)
	; 4529 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13588)
	; 4530 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13590)
	; 4530 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13591)
	; 4531 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 13594) && (n < 13672))
.if((n >= 13594) && (n < 13633))
; 13594 to 13632
.if(n = 13594)
	; 4532 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 13594 && n < 13632)))
	; 4533..4545 bytes
	delay_n (n-2)
	nop
.elseif(n = 13596)
	; 4532 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13597)
	; 4533 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 13598 && n < 13630)))
	; 4534..4544 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13600)
	; 4534 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13602)
	; 4534 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13603)
	; 4535 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13606)
	; 4536 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13608)
	; 4536 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13609)
	; 4537 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13612)
	; 4538 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13614)
	; 4538 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13615)
	; 4539 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13618)
	; 4540 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13620)
	; 4540 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13621)
	; 4541 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13624)
	; 4542 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13626)
	; 4542 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13627)
	; 4543 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13630)
	; 4544 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13632)
	; 4544 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 13633 to 13671
.if(n = 13633)
	; 4545 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 13633 && n < 13671)))
	; 4545..4557 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13634 && n < 13672)))
	; 4546..4558 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13636)
	; 4546 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13638)
	; 4546 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13639)
	; 4547 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13642)
	; 4548 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13644)
	; 4548 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13645)
	; 4549 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13648)
	; 4550 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13650)
	; 4550 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13651)
	; 4551 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13654)
	; 4552 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13656)
	; 4552 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13657)
	; 4553 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13660)
	; 4554 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13662)
	; 4554 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13663)
	; 4555 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13666)
	; 4556 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13668)
	; 4556 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13669)
	; 4557 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.else
.if((n >= 13672) && (n < 13711))
; 13672 to 13710
.if(n = 13672)
	; 4558 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 13672 && n < 13710)))
	; 4559..4571 bytes
	delay_n (n-2)
	nop
.elseif(n = 13674)
	; 4558 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13675)
	; 4559 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 13676 && n < 13708)))
	; 4560..4570 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13678)
	; 4560 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13680)
	; 4560 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13681)
	; 4561 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13684)
	; 4562 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13686)
	; 4562 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13687)
	; 4563 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13690)
	; 4564 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13692)
	; 4564 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13693)
	; 4565 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13696)
	; 4566 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13698)
	; 4566 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13699)
	; 4567 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13702)
	; 4568 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13704)
	; 4568 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13705)
	; 4569 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13708)
	; 4570 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13710)
	; 4570 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 13711 to 13750
.if(n = 13711)
	; 4571 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 13711 && n < 13749)))
	; 4571..4583 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13712 && n < 13750)))
	; 4572..4584 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13714)
	; 4572 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13716)
	; 4572 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13717)
	; 4573 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13720)
	; 4574 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13722)
	; 4574 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13723)
	; 4575 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13726)
	; 4576 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13728)
	; 4576 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13729)
	; 4577 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13732)
	; 4578 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13734)
	; 4578 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13735)
	; 4579 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13738)
	; 4580 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13740)
	; 4580 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13741)
	; 4581 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13744)
	; 4582 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13746)
	; 4582 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13747)
	; 4583 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13750)
	; 4584 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
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
.if((n .mod 3 = 2) && ((n > 13750 && n < 13788)))
	; 4585..4597 bytes
	delay_n (n-2)
	nop
.elseif(n = 13752)
	; 4584 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13753)
	; 4585 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 13754 && n < 13786)))
	; 4586..4596 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13756)
	; 4586 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13758)
	; 4586 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13759)
	; 4587 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13762)
	; 4588 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13764)
	; 4588 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13765)
	; 4589 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13768)
	; 4590 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13770)
	; 4590 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13771)
	; 4591 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13774)
	; 4592 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13776)
	; 4592 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13777)
	; 4593 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13780)
	; 4594 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13782)
	; 4594 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13783)
	; 4595 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13786)
	; 4596 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13788)
	; 4596 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13789)
	; 4597 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 13790 to 13828
.if((n .mod 3 = 2) && ((n > 13789 && n < 13827)))
	; 4597..4609 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13790 && n < 13828)))
	; 4598..4610 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13792)
	; 4598 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13794)
	; 4598 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13795)
	; 4599 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13798)
	; 4600 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13800)
	; 4600 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13801)
	; 4601 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13804)
	; 4602 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13806)
	; 4602 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13807)
	; 4603 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13810)
	; 4604 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13812)
	; 4604 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13813)
	; 4605 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13816)
	; 4606 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13818)
	; 4606 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13819)
	; 4607 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13822)
	; 4608 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13824)
	; 4608 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13825)
	; 4609 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13828)
	; 4610 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 13829) && (n < 13868))
; 13829 to 13867
.if((n .mod 3 = 2) && ((n > 13828 && n < 13866)))
	; 4611..4623 bytes
	delay_n (n-2)
	nop
.elseif(n = 13830)
	; 4610 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13831)
	; 4611 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 13832 && n < 13864)))
	; 4612..4622 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13834)
	; 4612 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13836)
	; 4612 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13837)
	; 4613 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13840)
	; 4614 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13842)
	; 4614 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13843)
	; 4615 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13846)
	; 4616 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13848)
	; 4616 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13849)
	; 4617 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13852)
	; 4618 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13854)
	; 4618 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13855)
	; 4619 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13858)
	; 4620 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13860)
	; 4620 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13861)
	; 4621 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13864)
	; 4622 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13866)
	; 4622 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13867)
	; 4623 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 13868 to 13906
.if((n .mod 3 = 2) && ((n > 13867 && n < 13905)))
	; 4623..4635 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13868 && n < 13906)))
	; 4624..4636 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13870)
	; 4624 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13872)
	; 4624 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13873)
	; 4625 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13876)
	; 4626 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13878)
	; 4626 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13879)
	; 4627 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13882)
	; 4628 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13884)
	; 4628 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13885)
	; 4629 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13888)
	; 4630 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13890)
	; 4630 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13891)
	; 4631 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13894)
	; 4632 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13896)
	; 4632 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13897)
	; 4633 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13900)
	; 4634 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13902)
	; 4634 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13903)
	; 4635 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13906)
	; 4636 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.else
.if((n >= 13907) && (n < 13985))
.if((n >= 13907) && (n < 13946))
; 13907 to 13945
.if((n .mod 3 = 2) && ((n > 13906 && n < 13944)))
	; 4637..4649 bytes
	delay_n (n-2)
	nop
.elseif(n = 13908)
	; 4636 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13909)
	; 4637 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 13910 && n < 13942)))
	; 4638..4648 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13912)
	; 4638 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13914)
	; 4638 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13915)
	; 4639 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13918)
	; 4640 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13920)
	; 4640 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13921)
	; 4641 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13924)
	; 4642 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13926)
	; 4642 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13927)
	; 4643 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13930)
	; 4644 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13932)
	; 4644 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13933)
	; 4645 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13936)
	; 4646 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13938)
	; 4646 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13939)
	; 4647 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13942)
	; 4648 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13944)
	; 4648 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13945)
	; 4649 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 13946 to 13984
.if((n .mod 3 = 2) && ((n > 13945 && n < 13983)))
	; 4649..4661 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 13946 && n < 13984)))
	; 4650..4662 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13948)
	; 4650 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13950)
	; 4650 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13951)
	; 4651 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13954)
	; 4652 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13956)
	; 4652 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13957)
	; 4653 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13960)
	; 4654 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13962)
	; 4654 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13963)
	; 4655 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13966)
	; 4656 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13968)
	; 4656 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13969)
	; 4657 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13972)
	; 4658 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13974)
	; 4658 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13975)
	; 4659 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 13978)
	; 4660 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13980)
	; 4660 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13981)
	; 4661 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13984)
	; 4662 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 13985) && (n < 14024))
; 13985 to 14023
.if((n .mod 3 = 2) && ((n > 13984 && n < 14022)))
	; 4663..4675 bytes
	delay_n (n-2)
	nop
.elseif(n = 13986)
	; 4662 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13987)
	; 4663 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 13988 && n < 14020)))
	; 4664..4674 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 13990)
	; 4664 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13992)
	; 4664 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13993)
	; 4665 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 13996)
	; 4666 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13998)
	; 4666 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13999)
	; 4667 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14002)
	; 4668 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14004)
	; 4668 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14005)
	; 4669 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14008)
	; 4670 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14010)
	; 4670 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14011)
	; 4671 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14014)
	; 4672 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14016)
	; 4672 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14017)
	; 4673 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14020)
	; 4674 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14022)
	; 4674 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14023)
	; 4675 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 14024 to 14062
.if((n .mod 3 = 2) && ((n > 14023 && n < 14061)))
	; 4675..4687 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14024 && n < 14062)))
	; 4676..4688 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14026)
	; 4676 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14028)
	; 4676 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14029)
	; 4677 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14032)
	; 4678 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14034)
	; 4678 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14035)
	; 4679 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14038)
	; 4680 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14040)
	; 4680 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14041)
	; 4681 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14044)
	; 4682 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14046)
	; 4682 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14047)
	; 4683 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14050)
	; 4684 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14052)
	; 4684 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14053)
	; 4685 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14056)
	; 4686 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14058)
	; 4686 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14059)
	; 4687 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14062)
	; 4688 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.endif
.else
.if((n >= 14063) && (n < 14219))
.if((n >= 14063) && (n < 14141))
.if((n >= 14063) && (n < 14102))
; 14063 to 14101
.if((n .mod 3 = 2) && ((n > 14062 && n < 14100)))
	; 4689..4701 bytes
	delay_n (n-2)
	nop
.elseif(n = 14064)
	; 4688 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14065)
	; 4689 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 14066 && n < 14098)))
	; 4690..4700 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14068)
	; 4690 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14070)
	; 4690 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14071)
	; 4691 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14074)
	; 4692 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14076)
	; 4692 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14077)
	; 4693 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14080)
	; 4694 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14082)
	; 4694 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14083)
	; 4695 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14086)
	; 4696 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14088)
	; 4696 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14089)
	; 4697 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14092)
	; 4698 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14094)
	; 4698 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14095)
	; 4699 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14098)
	; 4700 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14100)
	; 4700 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14101)
	; 4701 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 14102 to 14140
.if((n .mod 3 = 2) && ((n > 14101 && n < 14139)))
	; 4701..4713 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14102 && n < 14140)))
	; 4702..4714 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14104)
	; 4702 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14106)
	; 4702 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14107)
	; 4703 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14110)
	; 4704 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14112)
	; 4704 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14113)
	; 4705 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14116)
	; 4706 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14118)
	; 4706 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14119)
	; 4707 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14122)
	; 4708 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14124)
	; 4708 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14125)
	; 4709 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14128)
	; 4710 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14130)
	; 4710 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14131)
	; 4711 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14134)
	; 4712 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14136)
	; 4712 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14137)
	; 4713 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14140)
	; 4714 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 14141) && (n < 14180))
; 14141 to 14179
.if((n .mod 3 = 2) && ((n > 14140 && n < 14178)))
	; 4715..4727 bytes
	delay_n (n-2)
	nop
.elseif(n = 14142)
	; 4714 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14143)
	; 4715 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 14144 && n < 14176)))
	; 4716..4726 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14146)
	; 4716 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14148)
	; 4716 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14149)
	; 4717 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14152)
	; 4718 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14154)
	; 4718 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14155)
	; 4719 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14158)
	; 4720 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14160)
	; 4720 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14161)
	; 4721 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14164)
	; 4722 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14166)
	; 4722 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14167)
	; 4723 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14170)
	; 4724 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14172)
	; 4724 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14173)
	; 4725 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14176)
	; 4726 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14178)
	; 4726 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14179)
	; 4727 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 14180 to 14218
.if((n .mod 3 = 2) && ((n > 14179 && n < 14217)))
	; 4727..4739 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14180 && n < 14218)))
	; 4728..4740 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14182)
	; 4728 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14184)
	; 4728 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14185)
	; 4729 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14188)
	; 4730 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14190)
	; 4730 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14191)
	; 4731 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14194)
	; 4732 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14196)
	; 4732 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14197)
	; 4733 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14200)
	; 4734 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14202)
	; 4734 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14203)
	; 4735 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14206)
	; 4736 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14208)
	; 4736 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14209)
	; 4737 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14212)
	; 4738 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14214)
	; 4738 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14215)
	; 4739 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14218)
	; 4740 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
.if((n >= 14219) && (n < 14258))
; 14219 to 14257
.if((n .mod 3 = 2) && ((n > 14218 && n < 14256)))
	; 4741..4753 bytes
	delay_n (n-2)
	nop
.elseif(n = 14220)
	; 4740 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14221)
	; 4741 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 14222 && n < 14254)))
	; 4742..4752 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14224)
	; 4742 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14226)
	; 4742 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14227)
	; 4743 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14230)
	; 4744 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14232)
	; 4744 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14233)
	; 4745 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14236)
	; 4746 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14238)
	; 4746 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14239)
	; 4747 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14242)
	; 4748 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14244)
	; 4748 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14245)
	; 4749 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14248)
	; 4750 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14250)
	; 4750 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14251)
	; 4751 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14254)
	; 4752 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14256)
	; 4752 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14257)
	; 4753 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 14258 to 14296
.if((n .mod 3 = 2) && ((n > 14257 && n < 14295)))
	; 4753..4765 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14258 && n < 14296)))
	; 4754..4766 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14260)
	; 4754 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14262)
	; 4754 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14263)
	; 4755 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14266)
	; 4756 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14268)
	; 4756 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14269)
	; 4757 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14272)
	; 4758 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14274)
	; 4758 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14275)
	; 4759 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14278)
	; 4760 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14280)
	; 4760 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14281)
	; 4761 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14284)
	; 4762 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14286)
	; 4762 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14287)
	; 4763 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14290)
	; 4764 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14292)
	; 4764 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14293)
	; 4765 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14296)
	; 4766 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 14297) && (n < 14336))
; 14297 to 14335
.if((n .mod 3 = 2) && ((n > 14296 && n < 14334)))
	; 4767..4779 bytes
	delay_n (n-2)
	nop
.elseif(n = 14298)
	; 4766 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14299)
	; 4767 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 14300 && n < 14332)))
	; 4768..4778 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14302)
	; 4768 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14304)
	; 4768 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14305)
	; 4769 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14308)
	; 4770 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14310)
	; 4770 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14311)
	; 4771 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14314)
	; 4772 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14316)
	; 4772 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14317)
	; 4773 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14320)
	; 4774 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14322)
	; 4774 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14323)
	; 4775 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14326)
	; 4776 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14328)
	; 4776 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14329)
	; 4777 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14332)
	; 4778 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14334)
	; 4778 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14335)
	; 4779 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 14336 to 14375
.if((n .mod 3 = 2) && ((n > 14335 && n < 14376)))
	; 4779..4793 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14336 && n < 14374)))
	; 4780..4792 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14338)
	; 4780 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14340)
	; 4780 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14341)
	; 4781 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14344)
	; 4782 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14346)
	; 4782 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14347)
	; 4783 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14350)
	; 4784 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14352)
	; 4784 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14353)
	; 4785 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14356)
	; 4786 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14358)
	; 4786 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14359)
	; 4787 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14362)
	; 4788 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14364)
	; 4788 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14365)
	; 4789 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14368)
	; 4790 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14370)
	; 4790 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14371)
	; 4791 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14374)
	; 4792 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 14376) && (n < 14688))
.if((n >= 14376) && (n < 14532))
.if((n >= 14376) && (n < 14454))
.if((n >= 14376) && (n < 14415))
; 14376 to 14414
.if(n = 14376)
	; 4792 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14377)
	; 4793 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 14377 && n < 14415)))
	; 4793..4805 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14378 && n < 14410)))
	; 4794..4804 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14380)
	; 4794 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14382)
	; 4794 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14383)
	; 4795 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14386)
	; 4796 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14388)
	; 4796 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14389)
	; 4797 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14392)
	; 4798 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14394)
	; 4798 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14395)
	; 4799 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14398)
	; 4800 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14400)
	; 4800 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14401)
	; 4801 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14404)
	; 4802 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14406)
	; 4802 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14407)
	; 4803 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14410)
	; 4804 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14412)
	; 4804 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14413)
	; 4805 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 14415 to 14453
.if((n .mod 6 = 3) && ((n > 14414 && n < 14452)))
	; 4806..4818 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14416)
	; 4806 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 14416 && n < 14454)))
	; 4807..4819 bytes
	delay_n (n-2)
	nop
.elseif(n = 14418)
	; 4806 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14419)
	; 4807 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14422)
	; 4808 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14424)
	; 4808 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14425)
	; 4809 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14428)
	; 4810 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14430)
	; 4810 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14431)
	; 4811 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14434)
	; 4812 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14436)
	; 4812 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14437)
	; 4813 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14440)
	; 4814 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14442)
	; 4814 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14443)
	; 4815 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14446)
	; 4816 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14448)
	; 4816 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14449)
	; 4817 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14452)
	; 4818 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 14454) && (n < 14493))
; 14454 to 14492
.if(n = 14454)
	; 4818 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14455)
	; 4819 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 14455 && n < 14493)))
	; 4819..4831 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14456 && n < 14488)))
	; 4820..4830 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14458)
	; 4820 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14460)
	; 4820 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14461)
	; 4821 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14464)
	; 4822 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14466)
	; 4822 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14467)
	; 4823 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14470)
	; 4824 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14472)
	; 4824 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14473)
	; 4825 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14476)
	; 4826 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14478)
	; 4826 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14479)
	; 4827 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14482)
	; 4828 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14484)
	; 4828 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14485)
	; 4829 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14488)
	; 4830 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14490)
	; 4830 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14491)
	; 4831 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 14493 to 14531
.if((n .mod 6 = 3) && ((n > 14492 && n < 14530)))
	; 4832..4844 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14494)
	; 4832 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 14494 && n < 14532)))
	; 4833..4845 bytes
	delay_n (n-2)
	nop
.elseif(n = 14496)
	; 4832 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14497)
	; 4833 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14500)
	; 4834 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14502)
	; 4834 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14503)
	; 4835 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14506)
	; 4836 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14508)
	; 4836 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14509)
	; 4837 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14512)
	; 4838 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14514)
	; 4838 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14515)
	; 4839 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14518)
	; 4840 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14520)
	; 4840 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14521)
	; 4841 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14524)
	; 4842 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14526)
	; 4842 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14527)
	; 4843 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14530)
	; 4844 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
; 14532 to 14570
.if(n = 14532)
	; 4844 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14533)
	; 4845 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 14533 && n < 14571)))
	; 4845..4857 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14534 && n < 14566)))
	; 4846..4856 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14536)
	; 4846 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14538)
	; 4846 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14539)
	; 4847 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14542)
	; 4848 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14544)
	; 4848 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14545)
	; 4849 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14548)
	; 4850 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14550)
	; 4850 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14551)
	; 4851 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14554)
	; 4852 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14556)
	; 4852 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14557)
	; 4853 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14560)
	; 4854 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14562)
	; 4854 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14563)
	; 4855 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14566)
	; 4856 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14568)
	; 4856 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14569)
	; 4857 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 14571 to 14609
.if((n .mod 6 = 3) && ((n > 14570 && n < 14608)))
	; 4858..4870 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14572)
	; 4858 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 14572 && n < 14610)))
	; 4859..4871 bytes
	delay_n (n-2)
	nop
.elseif(n = 14574)
	; 4858 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14575)
	; 4859 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14578)
	; 4860 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14580)
	; 4860 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14581)
	; 4861 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14584)
	; 4862 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14586)
	; 4862 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14587)
	; 4863 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14590)
	; 4864 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14592)
	; 4864 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14593)
	; 4865 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14596)
	; 4866 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14598)
	; 4866 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14599)
	; 4867 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14602)
	; 4868 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14604)
	; 4868 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14605)
	; 4869 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14608)
	; 4870 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 14610) && (n < 14649))
; 14610 to 14648
.if(n = 14610)
	; 4870 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14611)
	; 4871 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 14611 && n < 14649)))
	; 4871..4883 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14612 && n < 14644)))
	; 4872..4882 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14614)
	; 4872 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14616)
	; 4872 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14617)
	; 4873 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14620)
	; 4874 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14622)
	; 4874 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14623)
	; 4875 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14626)
	; 4876 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14628)
	; 4876 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14629)
	; 4877 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14632)
	; 4878 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14634)
	; 4878 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14635)
	; 4879 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14638)
	; 4880 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14640)
	; 4880 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14641)
	; 4881 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14644)
	; 4882 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14646)
	; 4882 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14647)
	; 4883 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 14649 to 14687
.if((n .mod 6 = 3) && ((n > 14648 && n < 14686)))
	; 4884..4896 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14650)
	; 4884 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 14650 && n < 14688)))
	; 4885..4897 bytes
	delay_n (n-2)
	nop
.elseif(n = 14652)
	; 4884 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14653)
	; 4885 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14656)
	; 4886 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14658)
	; 4886 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14659)
	; 4887 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14662)
	; 4888 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14664)
	; 4888 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14665)
	; 4889 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14668)
	; 4890 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14670)
	; 4890 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14671)
	; 4891 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14674)
	; 4892 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14676)
	; 4892 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14677)
	; 4893 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14680)
	; 4894 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14682)
	; 4894 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14683)
	; 4895 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14686)
	; 4896 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
.if((n >= 14688) && (n < 14766))
.if((n >= 14688) && (n < 14727))
; 14688 to 14726
.if(n = 14688)
	; 4896 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14689)
	; 4897 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 14689 && n < 14727)))
	; 4897..4909 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14690 && n < 14722)))
	; 4898..4908 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14692)
	; 4898 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14694)
	; 4898 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14695)
	; 4899 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14698)
	; 4900 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14700)
	; 4900 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14701)
	; 4901 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14704)
	; 4902 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14706)
	; 4902 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14707)
	; 4903 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14710)
	; 4904 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14712)
	; 4904 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14713)
	; 4905 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14716)
	; 4906 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14718)
	; 4906 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14719)
	; 4907 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14722)
	; 4908 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14724)
	; 4908 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14725)
	; 4909 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 14727 to 14765
.if((n .mod 6 = 3) && ((n > 14726 && n < 14764)))
	; 4910..4922 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14728)
	; 4910 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 14728 && n < 14766)))
	; 4911..4923 bytes
	delay_n (n-2)
	nop
.elseif(n = 14730)
	; 4910 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14731)
	; 4911 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14734)
	; 4912 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14736)
	; 4912 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14737)
	; 4913 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14740)
	; 4914 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14742)
	; 4914 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14743)
	; 4915 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14746)
	; 4916 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14748)
	; 4916 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14749)
	; 4917 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14752)
	; 4918 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14754)
	; 4918 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14755)
	; 4919 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14758)
	; 4920 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14760)
	; 4920 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14761)
	; 4921 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14764)
	; 4922 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 14766) && (n < 14805))
; 14766 to 14804
.if(n = 14766)
	; 4922 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14767)
	; 4923 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 14767 && n < 14805)))
	; 4923..4935 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14768 && n < 14800)))
	; 4924..4934 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14770)
	; 4924 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14772)
	; 4924 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14773)
	; 4925 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14776)
	; 4926 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14778)
	; 4926 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14779)
	; 4927 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14782)
	; 4928 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14784)
	; 4928 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14785)
	; 4929 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14788)
	; 4930 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14790)
	; 4930 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14791)
	; 4931 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14794)
	; 4932 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14796)
	; 4932 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14797)
	; 4933 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14800)
	; 4934 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14802)
	; 4934 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14803)
	; 4935 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 14805 to 14843
.if((n .mod 6 = 3) && ((n > 14804 && n < 14842)))
	; 4936..4948 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14806)
	; 4936 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 14806 && n < 14844)))
	; 4937..4949 bytes
	delay_n (n-2)
	nop
.elseif(n = 14808)
	; 4936 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14809)
	; 4937 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14812)
	; 4938 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14814)
	; 4938 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14815)
	; 4939 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14818)
	; 4940 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14820)
	; 4940 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14821)
	; 4941 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14824)
	; 4942 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14826)
	; 4942 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14827)
	; 4943 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14830)
	; 4944 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14832)
	; 4944 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14833)
	; 4945 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14836)
	; 4946 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14838)
	; 4946 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14839)
	; 4947 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14842)
	; 4948 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.else
.if((n >= 14844) && (n < 14922))
.if((n >= 14844) && (n < 14883))
; 14844 to 14882
.if(n = 14844)
	; 4948 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14845)
	; 4949 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 14845 && n < 14883)))
	; 4949..4961 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14846 && n < 14878)))
	; 4950..4960 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14848)
	; 4950 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14850)
	; 4950 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14851)
	; 4951 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14854)
	; 4952 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14856)
	; 4952 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14857)
	; 4953 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14860)
	; 4954 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14862)
	; 4954 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14863)
	; 4955 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14866)
	; 4956 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14868)
	; 4956 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14869)
	; 4957 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14872)
	; 4958 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14874)
	; 4958 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14875)
	; 4959 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14878)
	; 4960 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14880)
	; 4960 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14881)
	; 4961 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 14883 to 14921
.if((n .mod 6 = 3) && ((n > 14882 && n < 14920)))
	; 4962..4974 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14884)
	; 4962 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 14884 && n < 14922)))
	; 4963..4975 bytes
	delay_n (n-2)
	nop
.elseif(n = 14886)
	; 4962 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14887)
	; 4963 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14890)
	; 4964 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14892)
	; 4964 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14893)
	; 4965 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14896)
	; 4966 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14898)
	; 4966 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14899)
	; 4967 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14902)
	; 4968 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14904)
	; 4968 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14905)
	; 4969 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14908)
	; 4970 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14910)
	; 4970 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14911)
	; 4971 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14914)
	; 4972 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14916)
	; 4972 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14917)
	; 4973 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14920)
	; 4974 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 14922) && (n < 14961))
; 14922 to 14960
.if(n = 14922)
	; 4974 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14923)
	; 4975 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 14923 && n < 14961)))
	; 4975..4987 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 14924 && n < 14956)))
	; 4976..4986 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14926)
	; 4976 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14928)
	; 4976 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14929)
	; 4977 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14932)
	; 4978 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14934)
	; 4978 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14935)
	; 4979 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14938)
	; 4980 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14940)
	; 4980 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14941)
	; 4981 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14944)
	; 4982 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14946)
	; 4982 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14947)
	; 4983 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 14950)
	; 4984 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14952)
	; 4984 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14953)
	; 4985 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14956)
	; 4986 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14958)
	; 4986 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14959)
	; 4987 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 14961 to 15000
.if((n .mod 6 = 3) && ((n > 14960 && n < 14998)))
	; 4988..5000 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 14962)
	; 4988 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 14962 && n < 15000)))
	; 4989..5001 bytes
	delay_n (n-2)
	nop
.elseif(n = 14964)
	; 4988 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14965)
	; 4989 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 14968)
	; 4990 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14970)
	; 4990 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14971)
	; 4991 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14974)
	; 4992 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14976)
	; 4992 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14977)
	; 4993 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14980)
	; 4994 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14982)
	; 4994 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14983)
	; 4995 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14986)
	; 4996 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14988)
	; 4996 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14989)
	; 4997 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14992)
	; 4998 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14994)
	; 4998 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14995)
	; 4999 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14998)
	; 5000 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15000)
	; 5000 bytes
	delay_n 96
	delay_n (n-96)
.endif
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
.if((n >= 15001) && (n < 15040))
; 15001 to 15039
.if(n = 15001)
	; 5001 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 15001 && n < 15039)))
	; 5001..5013 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15002 && n < 15040)))
	; 5002..5014 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15004)
	; 5002 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15006)
	; 5002 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15007)
	; 5003 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15010)
	; 5004 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15012)
	; 5004 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15013)
	; 5005 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15016)
	; 5006 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15018)
	; 5006 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15019)
	; 5007 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15022)
	; 5008 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15024)
	; 5008 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15025)
	; 5009 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15028)
	; 5010 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15030)
	; 5010 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15031)
	; 5011 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15034)
	; 5012 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15036)
	; 5012 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15037)
	; 5013 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 15040 to 15078
.if(n = 15040)
	; 5014 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 15040 && n < 15078)))
	; 5015..5027 bytes
	delay_n (n-2)
	nop
.elseif(n = 15042)
	; 5014 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15043)
	; 5015 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 15044 && n < 15076)))
	; 5016..5026 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15046)
	; 5016 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15048)
	; 5016 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15049)
	; 5017 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15052)
	; 5018 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15054)
	; 5018 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15055)
	; 5019 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15058)
	; 5020 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15060)
	; 5020 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15061)
	; 5021 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15064)
	; 5022 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15066)
	; 5022 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15067)
	; 5023 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15070)
	; 5024 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15072)
	; 5024 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15073)
	; 5025 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15076)
	; 5026 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15078)
	; 5026 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 15079) && (n < 15118))
; 15079 to 15117
.if(n = 15079)
	; 5027 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 15079 && n < 15117)))
	; 5027..5039 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15080 && n < 15118)))
	; 5028..5040 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15082)
	; 5028 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15084)
	; 5028 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15085)
	; 5029 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15088)
	; 5030 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15090)
	; 5030 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15091)
	; 5031 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15094)
	; 5032 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15096)
	; 5032 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15097)
	; 5033 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15100)
	; 5034 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15102)
	; 5034 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15103)
	; 5035 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15106)
	; 5036 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15108)
	; 5036 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15109)
	; 5037 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15112)
	; 5038 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15114)
	; 5038 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15115)
	; 5039 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 15118 to 15156
.if(n = 15118)
	; 5040 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 15118 && n < 15156)))
	; 5041..5053 bytes
	delay_n (n-2)
	nop
.elseif(n = 15120)
	; 5040 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15121)
	; 5041 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 15122 && n < 15154)))
	; 5042..5052 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15124)
	; 5042 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15126)
	; 5042 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15127)
	; 5043 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15130)
	; 5044 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15132)
	; 5044 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15133)
	; 5045 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15136)
	; 5046 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15138)
	; 5046 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15139)
	; 5047 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15142)
	; 5048 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15144)
	; 5048 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15145)
	; 5049 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15148)
	; 5050 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15150)
	; 5050 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15151)
	; 5051 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15154)
	; 5052 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15156)
	; 5052 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.endif
.else
.if((n >= 15157) && (n < 15235))
.if((n >= 15157) && (n < 15196))
; 15157 to 15195
.if(n = 15157)
	; 5053 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 15157 && n < 15195)))
	; 5053..5065 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15158 && n < 15196)))
	; 5054..5066 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15160)
	; 5054 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15162)
	; 5054 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15163)
	; 5055 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15166)
	; 5056 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15168)
	; 5056 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15169)
	; 5057 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15172)
	; 5058 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15174)
	; 5058 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15175)
	; 5059 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15178)
	; 5060 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15180)
	; 5060 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15181)
	; 5061 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15184)
	; 5062 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15186)
	; 5062 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15187)
	; 5063 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15190)
	; 5064 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15192)
	; 5064 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15193)
	; 5065 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 15196 to 15234
.if(n = 15196)
	; 5066 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 15196 && n < 15234)))
	; 5067..5079 bytes
	delay_n (n-2)
	nop
.elseif(n = 15198)
	; 5066 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15199)
	; 5067 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 15200 && n < 15232)))
	; 5068..5078 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15202)
	; 5068 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15204)
	; 5068 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15205)
	; 5069 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15208)
	; 5070 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15210)
	; 5070 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15211)
	; 5071 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15214)
	; 5072 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15216)
	; 5072 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15217)
	; 5073 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15220)
	; 5074 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15222)
	; 5074 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15223)
	; 5075 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15226)
	; 5076 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15228)
	; 5076 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15229)
	; 5077 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15232)
	; 5078 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15234)
	; 5078 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.else
.if((n >= 15235) && (n < 15274))
; 15235 to 15273
.if(n = 15235)
	; 5079 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 15235 && n < 15273)))
	; 5079..5091 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15236 && n < 15274)))
	; 5080..5092 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15238)
	; 5080 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15240)
	; 5080 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15241)
	; 5081 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15244)
	; 5082 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15246)
	; 5082 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15247)
	; 5083 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15250)
	; 5084 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15252)
	; 5084 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15253)
	; 5085 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15256)
	; 5086 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15258)
	; 5086 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15259)
	; 5087 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15262)
	; 5088 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15264)
	; 5088 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15265)
	; 5089 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15268)
	; 5090 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15270)
	; 5090 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15271)
	; 5091 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 15274 to 15312
.if(n = 15274)
	; 5092 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 15274 && n < 15312)))
	; 5093..5105 bytes
	delay_n (n-2)
	nop
.elseif(n = 15276)
	; 5092 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15277)
	; 5093 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 15278 && n < 15310)))
	; 5094..5104 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15280)
	; 5094 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15282)
	; 5094 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15283)
	; 5095 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15286)
	; 5096 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15288)
	; 5096 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15289)
	; 5097 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15292)
	; 5098 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15294)
	; 5098 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15295)
	; 5099 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15298)
	; 5100 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15300)
	; 5100 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15301)
	; 5101 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15304)
	; 5102 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15306)
	; 5102 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15307)
	; 5103 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15310)
	; 5104 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15312)
	; 5104 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.endif
.endif
.else
.if((n >= 15313) && (n < 15469))
.if((n >= 15313) && (n < 15391))
.if((n >= 15313) && (n < 15352))
; 15313 to 15351
.if(n = 15313)
	; 5105 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 15313 && n < 15351)))
	; 5105..5117 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15314 && n < 15352)))
	; 5106..5118 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15316)
	; 5106 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15318)
	; 5106 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15319)
	; 5107 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15322)
	; 5108 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15324)
	; 5108 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15325)
	; 5109 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15328)
	; 5110 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15330)
	; 5110 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15331)
	; 5111 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15334)
	; 5112 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15336)
	; 5112 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15337)
	; 5113 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15340)
	; 5114 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15342)
	; 5114 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15343)
	; 5115 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15346)
	; 5116 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15348)
	; 5116 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15349)
	; 5117 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 15352 to 15390
.if(n = 15352)
	; 5118 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 15352 && n < 15390)))
	; 5119..5131 bytes
	delay_n (n-2)
	nop
.elseif(n = 15354)
	; 5118 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15355)
	; 5119 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 15356 && n < 15388)))
	; 5120..5130 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15358)
	; 5120 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15360)
	; 5120 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15361)
	; 5121 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15364)
	; 5122 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15366)
	; 5122 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15367)
	; 5123 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15370)
	; 5124 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15372)
	; 5124 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15373)
	; 5125 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15376)
	; 5126 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15378)
	; 5126 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15379)
	; 5127 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15382)
	; 5128 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15384)
	; 5128 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15385)
	; 5129 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15388)
	; 5130 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15390)
	; 5130 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.endif
.else
.if((n >= 15391) && (n < 15430))
; 15391 to 15429
.if(n = 15391)
	; 5131 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 15391 && n < 15429)))
	; 5131..5143 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15392 && n < 15430)))
	; 5132..5144 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15394)
	; 5132 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15396)
	; 5132 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15397)
	; 5133 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15400)
	; 5134 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15402)
	; 5134 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15403)
	; 5135 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15406)
	; 5136 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15408)
	; 5136 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15409)
	; 5137 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15412)
	; 5138 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15414)
	; 5138 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15415)
	; 5139 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15418)
	; 5140 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15420)
	; 5140 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15421)
	; 5141 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15424)
	; 5142 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15426)
	; 5142 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15427)
	; 5143 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 15430 to 15468
.if(n = 15430)
	; 5144 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 15430 && n < 15468)))
	; 5145..5157 bytes
	delay_n (n-2)
	nop
.elseif(n = 15432)
	; 5144 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15433)
	; 5145 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 15434 && n < 15466)))
	; 5146..5156 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15436)
	; 5146 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15438)
	; 5146 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15439)
	; 5147 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15442)
	; 5148 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15444)
	; 5148 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15445)
	; 5149 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15448)
	; 5150 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15450)
	; 5150 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15451)
	; 5151 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15454)
	; 5152 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15456)
	; 5152 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15457)
	; 5153 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15460)
	; 5154 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15462)
	; 5154 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15463)
	; 5155 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15466)
	; 5156 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15468)
	; 5156 bytes
	delay_n 24
	delay_n (n-24)
.endif
.endif
.endif
.else
.if((n >= 15469) && (n < 15547))
.if((n >= 15469) && (n < 15508))
; 15469 to 15507
.if(n = 15469)
	; 5157 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 15469 && n < 15507)))
	; 5157..5169 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15470 && n < 15508)))
	; 5158..5170 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15472)
	; 5158 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15474)
	; 5158 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15475)
	; 5159 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15478)
	; 5160 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15480)
	; 5160 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15481)
	; 5161 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15484)
	; 5162 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15486)
	; 5162 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15487)
	; 5163 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15490)
	; 5164 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15492)
	; 5164 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15493)
	; 5165 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15496)
	; 5166 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15498)
	; 5166 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15499)
	; 5167 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15502)
	; 5168 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15504)
	; 5168 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15505)
	; 5169 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 15508 to 15546
.if(n = 15508)
	; 5170 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 15508 && n < 15546)))
	; 5171..5183 bytes
	delay_n (n-2)
	nop
.elseif(n = 15510)
	; 5170 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15511)
	; 5171 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 15512 && n < 15544)))
	; 5172..5182 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15514)
	; 5172 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15516)
	; 5172 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15517)
	; 5173 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15520)
	; 5174 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15522)
	; 5174 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15523)
	; 5175 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15526)
	; 5176 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15528)
	; 5176 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15529)
	; 5177 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15532)
	; 5178 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15534)
	; 5178 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15535)
	; 5179 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15538)
	; 5180 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15540)
	; 5180 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15541)
	; 5181 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15544)
	; 5182 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15546)
	; 5182 bytes
	delay_n 48
	delay_n (n-48)
.endif
.endif
.else
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if(n = 15547)
	; 5183 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 15547 && n < 15585)))
	; 5183..5195 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15548 && n < 15586)))
	; 5184..5196 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15550)
	; 5184 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15552)
	; 5184 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15553)
	; 5185 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15556)
	; 5186 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15558)
	; 5186 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15559)
	; 5187 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15562)
	; 5188 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15564)
	; 5188 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15565)
	; 5189 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15568)
	; 5190 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15570)
	; 5190 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15571)
	; 5191 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15574)
	; 5192 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15576)
	; 5192 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15577)
	; 5193 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15580)
	; 5194 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15582)
	; 5194 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15583)
	; 5195 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 15586 to 15625
.if(n = 15586)
	; 5196 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 15586 && n < 15624)))
	; 5197..5209 bytes
	delay_n (n-2)
	nop
.elseif(n = 15588)
	; 5196 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15589)
	; 5197 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 15590 && n < 15622)))
	; 5198..5208 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15592)
	; 5198 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15594)
	; 5198 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15595)
	; 5199 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15598)
	; 5200 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15600)
	; 5200 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15601)
	; 5201 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15604)
	; 5202 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15606)
	; 5202 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15607)
	; 5203 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15610)
	; 5204 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15612)
	; 5204 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15613)
	; 5205 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15616)
	; 5206 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15618)
	; 5206 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15619)
	; 5207 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15622)
	; 5208 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15624)
	; 5208 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15625)
	; 5209 bytes
	delay_n 19
	delay_n (n-19)
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
.if((n .mod 3 = 2) && ((n > 15625 && n < 15663)))
	; 5209..5221 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15626 && n < 15664)))
	; 5210..5222 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15628)
	; 5210 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15630)
	; 5210 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15631)
	; 5211 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15634)
	; 5212 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15636)
	; 5212 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15637)
	; 5213 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15640)
	; 5214 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15642)
	; 5214 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15643)
	; 5215 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15646)
	; 5216 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15648)
	; 5216 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15649)
	; 5217 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15652)
	; 5218 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15654)
	; 5218 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15655)
	; 5219 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15658)
	; 5220 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15660)
	; 5220 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15661)
	; 5221 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15664)
	; 5222 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 15665 to 15703
.if((n .mod 3 = 2) && ((n > 15664 && n < 15702)))
	; 5223..5235 bytes
	delay_n (n-2)
	nop
.elseif(n = 15666)
	; 5222 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15667)
	; 5223 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 15668 && n < 15700)))
	; 5224..5234 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15670)
	; 5224 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15672)
	; 5224 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15673)
	; 5225 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15676)
	; 5226 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15678)
	; 5226 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15679)
	; 5227 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15682)
	; 5228 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15684)
	; 5228 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15685)
	; 5229 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15688)
	; 5230 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15690)
	; 5230 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15691)
	; 5231 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15694)
	; 5232 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15696)
	; 5232 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15697)
	; 5233 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15700)
	; 5234 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15702)
	; 5234 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15703)
	; 5235 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.else
.if((n >= 15704) && (n < 15743))
; 15704 to 15742
.if((n .mod 3 = 2) && ((n > 15703 && n < 15741)))
	; 5235..5247 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15704 && n < 15742)))
	; 5236..5248 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15706)
	; 5236 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15708)
	; 5236 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15709)
	; 5237 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15712)
	; 5238 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15714)
	; 5238 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15715)
	; 5239 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15718)
	; 5240 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15720)
	; 5240 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15721)
	; 5241 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15724)
	; 5242 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15726)
	; 5242 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15727)
	; 5243 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15730)
	; 5244 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15732)
	; 5244 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15733)
	; 5245 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15736)
	; 5246 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15738)
	; 5246 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15739)
	; 5247 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15742)
	; 5248 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 15743 to 15781
.if((n .mod 3 = 2) && ((n > 15742 && n < 15780)))
	; 5249..5261 bytes
	delay_n (n-2)
	nop
.elseif(n = 15744)
	; 5248 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15745)
	; 5249 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 15746 && n < 15778)))
	; 5250..5260 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15748)
	; 5250 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15750)
	; 5250 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15751)
	; 5251 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15754)
	; 5252 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15756)
	; 5252 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15757)
	; 5253 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15760)
	; 5254 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15762)
	; 5254 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15763)
	; 5255 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15766)
	; 5256 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15768)
	; 5256 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15769)
	; 5257 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15772)
	; 5258 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15774)
	; 5258 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15775)
	; 5259 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15778)
	; 5260 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15780)
	; 5260 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15781)
	; 5261 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 15782) && (n < 15860))
.if((n >= 15782) && (n < 15821))
; 15782 to 15820
.if((n .mod 3 = 2) && ((n > 15781 && n < 15819)))
	; 5261..5273 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15782 && n < 15820)))
	; 5262..5274 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15784)
	; 5262 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15786)
	; 5262 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15787)
	; 5263 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15790)
	; 5264 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15792)
	; 5264 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15793)
	; 5265 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15796)
	; 5266 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15798)
	; 5266 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15799)
	; 5267 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15802)
	; 5268 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15804)
	; 5268 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15805)
	; 5269 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15808)
	; 5270 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15810)
	; 5270 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15811)
	; 5271 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15814)
	; 5272 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15816)
	; 5272 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15817)
	; 5273 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15820)
	; 5274 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 15821 to 15859
.if((n .mod 3 = 2) && ((n > 15820 && n < 15858)))
	; 5275..5287 bytes
	delay_n (n-2)
	nop
.elseif(n = 15822)
	; 5274 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15823)
	; 5275 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 15824 && n < 15856)))
	; 5276..5286 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15826)
	; 5276 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15828)
	; 5276 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15829)
	; 5277 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15832)
	; 5278 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15834)
	; 5278 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15835)
	; 5279 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15838)
	; 5280 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15840)
	; 5280 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15841)
	; 5281 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15844)
	; 5282 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15846)
	; 5282 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15847)
	; 5283 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15850)
	; 5284 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15852)
	; 5284 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15853)
	; 5285 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15856)
	; 5286 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15858)
	; 5286 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15859)
	; 5287 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 15860) && (n < 15899))
; 15860 to 15898
.if((n .mod 3 = 2) && ((n > 15859 && n < 15897)))
	; 5287..5299 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15860 && n < 15898)))
	; 5288..5300 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15862)
	; 5288 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15864)
	; 5288 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15865)
	; 5289 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15868)
	; 5290 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15870)
	; 5290 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15871)
	; 5291 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15874)
	; 5292 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15876)
	; 5292 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15877)
	; 5293 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15880)
	; 5294 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15882)
	; 5294 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15883)
	; 5295 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15886)
	; 5296 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15888)
	; 5296 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15889)
	; 5297 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15892)
	; 5298 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15894)
	; 5298 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15895)
	; 5299 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15898)
	; 5300 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 15899 to 15937
.if((n .mod 3 = 2) && ((n > 15898 && n < 15936)))
	; 5301..5313 bytes
	delay_n (n-2)
	nop
.elseif(n = 15900)
	; 5300 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15901)
	; 5301 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 15902 && n < 15934)))
	; 5302..5312 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15904)
	; 5302 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15906)
	; 5302 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15907)
	; 5303 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15910)
	; 5304 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15912)
	; 5304 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15913)
	; 5305 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15916)
	; 5306 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15918)
	; 5306 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15919)
	; 5307 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15922)
	; 5308 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15924)
	; 5308 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15925)
	; 5309 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15928)
	; 5310 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15930)
	; 5310 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15931)
	; 5311 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15934)
	; 5312 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15936)
	; 5312 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15937)
	; 5313 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.endif
.endif
.else
.if((n >= 15938) && (n < 16094))
.if((n >= 15938) && (n < 16016))
.if((n >= 15938) && (n < 15977))
; 15938 to 15976
.if((n .mod 3 = 2) && ((n > 15937 && n < 15975)))
	; 5313..5325 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 15938 && n < 15976)))
	; 5314..5326 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15940)
	; 5314 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15942)
	; 5314 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15943)
	; 5315 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15946)
	; 5316 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15948)
	; 5316 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15949)
	; 5317 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15952)
	; 5318 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15954)
	; 5318 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15955)
	; 5319 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15958)
	; 5320 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15960)
	; 5320 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15961)
	; 5321 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15964)
	; 5322 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15966)
	; 5322 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15967)
	; 5323 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15970)
	; 5324 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15972)
	; 5324 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15973)
	; 5325 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 15976)
	; 5326 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 15977 to 16015
.if((n .mod 3 = 2) && ((n > 15976 && n < 16014)))
	; 5327..5339 bytes
	delay_n (n-2)
	nop
.elseif(n = 15978)
	; 5326 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15979)
	; 5327 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 15980 && n < 16012)))
	; 5328..5338 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 15982)
	; 5328 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15984)
	; 5328 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15985)
	; 5329 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15988)
	; 5330 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15990)
	; 5330 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15991)
	; 5331 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 15994)
	; 5332 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15996)
	; 5332 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15997)
	; 5333 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16000)
	; 5334 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16002)
	; 5334 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16003)
	; 5335 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16006)
	; 5336 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16008)
	; 5336 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16009)
	; 5337 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16012)
	; 5338 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16014)
	; 5338 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16015)
	; 5339 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.else
.if((n >= 16016) && (n < 16055))
; 16016 to 16054
.if((n .mod 3 = 2) && ((n > 16015 && n < 16053)))
	; 5339..5351 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16016 && n < 16054)))
	; 5340..5352 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16018)
	; 5340 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16020)
	; 5340 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16021)
	; 5341 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16024)
	; 5342 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16026)
	; 5342 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16027)
	; 5343 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16030)
	; 5344 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16032)
	; 5344 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16033)
	; 5345 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16036)
	; 5346 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16038)
	; 5346 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16039)
	; 5347 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16042)
	; 5348 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16044)
	; 5348 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16045)
	; 5349 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16048)
	; 5350 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16050)
	; 5350 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16051)
	; 5351 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16054)
	; 5352 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 16055 to 16093
.if((n .mod 3 = 2) && ((n > 16054 && n < 16092)))
	; 5353..5365 bytes
	delay_n (n-2)
	nop
.elseif(n = 16056)
	; 5352 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16057)
	; 5353 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 16058 && n < 16090)))
	; 5354..5364 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16060)
	; 5354 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16062)
	; 5354 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16063)
	; 5355 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16066)
	; 5356 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16068)
	; 5356 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16069)
	; 5357 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16072)
	; 5358 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16074)
	; 5358 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16075)
	; 5359 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16078)
	; 5360 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16080)
	; 5360 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16081)
	; 5361 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16084)
	; 5362 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16086)
	; 5362 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16087)
	; 5363 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16090)
	; 5364 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16092)
	; 5364 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16093)
	; 5365 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.else
.if((n >= 16094) && (n < 16172))
.if((n >= 16094) && (n < 16133))
; 16094 to 16132
.if((n .mod 3 = 2) && ((n > 16093 && n < 16131)))
	; 5365..5377 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16094 && n < 16132)))
	; 5366..5378 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16096)
	; 5366 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16098)
	; 5366 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16099)
	; 5367 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16102)
	; 5368 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16104)
	; 5368 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16105)
	; 5369 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16108)
	; 5370 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16110)
	; 5370 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16111)
	; 5371 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16114)
	; 5372 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16116)
	; 5372 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16117)
	; 5373 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16120)
	; 5374 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16122)
	; 5374 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16123)
	; 5375 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16126)
	; 5376 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16128)
	; 5376 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16129)
	; 5377 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16132)
	; 5378 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 16133 to 16171
.if((n .mod 3 = 2) && ((n > 16132 && n < 16170)))
	; 5379..5391 bytes
	delay_n (n-2)
	nop
.elseif(n = 16134)
	; 5378 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16135)
	; 5379 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 16136 && n < 16168)))
	; 5380..5390 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16138)
	; 5380 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16140)
	; 5380 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16141)
	; 5381 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16144)
	; 5382 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16146)
	; 5382 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16147)
	; 5383 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16150)
	; 5384 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16152)
	; 5384 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16153)
	; 5385 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16156)
	; 5386 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16158)
	; 5386 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16159)
	; 5387 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16162)
	; 5388 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16164)
	; 5388 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16165)
	; 5389 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16168)
	; 5390 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16170)
	; 5390 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16171)
	; 5391 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 16172) && (n < 16211))
; 16172 to 16210
.if((n .mod 3 = 2) && ((n > 16171 && n < 16209)))
	; 5391..5403 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16172 && n < 16210)))
	; 5392..5404 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16174)
	; 5392 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16176)
	; 5392 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16177)
	; 5393 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16180)
	; 5394 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16182)
	; 5394 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16183)
	; 5395 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16186)
	; 5396 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16188)
	; 5396 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16189)
	; 5397 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16192)
	; 5398 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16194)
	; 5398 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16195)
	; 5399 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16198)
	; 5400 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16200)
	; 5400 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16201)
	; 5401 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16204)
	; 5402 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16206)
	; 5402 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16207)
	; 5403 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16210)
	; 5404 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 16211 to 16250
.if((n .mod 3 = 2) && ((n > 16210 && n < 16251)))
	; 5405..5417 bytes
	delay_n (n-2)
	nop
.elseif(n = 16212)
	; 5404 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16213)
	; 5405 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 16214 && n < 16246)))
	; 5406..5416 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16216)
	; 5406 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16218)
	; 5406 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16219)
	; 5407 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16222)
	; 5408 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16224)
	; 5408 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16225)
	; 5409 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16228)
	; 5410 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16230)
	; 5410 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16231)
	; 5411 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16234)
	; 5412 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16236)
	; 5412 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16237)
	; 5413 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16240)
	; 5414 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16242)
	; 5414 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16243)
	; 5415 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16246)
	; 5416 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16248)
	; 5416 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16249)
	; 5417 bytes
	delay_n 103
	delay_n (n-103)
.endif
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
.if((n >= 16251) && (n < 16290))
; 16251 to 16289
.if((n .mod 6 = 3) && ((n > 16250 && n < 16288)))
	; 5418..5430 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16252)
	; 5418 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 16252 && n < 16290)))
	; 5419..5431 bytes
	delay_n (n-2)
	nop
.elseif(n = 16254)
	; 5418 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16255)
	; 5419 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16258)
	; 5420 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16260)
	; 5420 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16261)
	; 5421 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16264)
	; 5422 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16266)
	; 5422 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16267)
	; 5423 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16270)
	; 5424 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16272)
	; 5424 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16273)
	; 5425 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16276)
	; 5426 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16278)
	; 5426 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16279)
	; 5427 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16282)
	; 5428 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16284)
	; 5428 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16285)
	; 5429 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16288)
	; 5430 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 16290 to 16328
.if(n = 16290)
	; 5430 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16291)
	; 5431 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 16291 && n < 16329)))
	; 5431..5443 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16292 && n < 16324)))
	; 5432..5442 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16294)
	; 5432 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16296)
	; 5432 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16297)
	; 5433 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16300)
	; 5434 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16302)
	; 5434 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16303)
	; 5435 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16306)
	; 5436 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16308)
	; 5436 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16309)
	; 5437 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16312)
	; 5438 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16314)
	; 5438 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16315)
	; 5439 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16318)
	; 5440 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16320)
	; 5440 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16321)
	; 5441 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16324)
	; 5442 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16326)
	; 5442 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16327)
	; 5443 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
.if((n >= 16329) && (n < 16368))
; 16329 to 16367
.if((n .mod 6 = 3) && ((n > 16328 && n < 16366)))
	; 5444..5456 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16330)
	; 5444 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 16330 && n < 16368)))
	; 5445..5457 bytes
	delay_n (n-2)
	nop
.elseif(n = 16332)
	; 5444 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16333)
	; 5445 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16336)
	; 5446 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16338)
	; 5446 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16339)
	; 5447 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16342)
	; 5448 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16344)
	; 5448 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16345)
	; 5449 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16348)
	; 5450 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16350)
	; 5450 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16351)
	; 5451 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16354)
	; 5452 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16356)
	; 5452 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16357)
	; 5453 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16360)
	; 5454 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16362)
	; 5454 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16363)
	; 5455 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16366)
	; 5456 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 16368 to 16406
.if(n = 16368)
	; 5456 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16369)
	; 5457 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 16369 && n < 16407)))
	; 5457..5469 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16370 && n < 16402)))
	; 5458..5468 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16372)
	; 5458 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16374)
	; 5458 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16375)
	; 5459 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16378)
	; 5460 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16380)
	; 5460 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16381)
	; 5461 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16384)
	; 5462 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16386)
	; 5462 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16387)
	; 5463 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16390)
	; 5464 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16392)
	; 5464 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16393)
	; 5465 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16396)
	; 5466 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16398)
	; 5466 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16399)
	; 5467 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16402)
	; 5468 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16404)
	; 5468 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16405)
	; 5469 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.endif
.else
.if((n >= 16407) && (n < 16485))
.if((n >= 16407) && (n < 16446))
; 16407 to 16445
.if((n .mod 6 = 3) && ((n > 16406 && n < 16444)))
	; 5470..5482 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16408)
	; 5470 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 16408 && n < 16446)))
	; 5471..5483 bytes
	delay_n (n-2)
	nop
.elseif(n = 16410)
	; 5470 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16411)
	; 5471 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16414)
	; 5472 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16416)
	; 5472 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16417)
	; 5473 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16420)
	; 5474 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16422)
	; 5474 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16423)
	; 5475 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16426)
	; 5476 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16428)
	; 5476 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16429)
	; 5477 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16432)
	; 5478 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16434)
	; 5478 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16435)
	; 5479 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16438)
	; 5480 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16440)
	; 5480 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16441)
	; 5481 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16444)
	; 5482 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 16446 to 16484
.if(n = 16446)
	; 5482 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16447)
	; 5483 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 16447 && n < 16485)))
	; 5483..5495 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16448 && n < 16480)))
	; 5484..5494 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16450)
	; 5484 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16452)
	; 5484 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16453)
	; 5485 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16456)
	; 5486 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16458)
	; 5486 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16459)
	; 5487 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16462)
	; 5488 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16464)
	; 5488 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16465)
	; 5489 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16468)
	; 5490 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16470)
	; 5490 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16471)
	; 5491 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16474)
	; 5492 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16476)
	; 5492 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16477)
	; 5493 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16480)
	; 5494 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16482)
	; 5494 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16483)
	; 5495 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 16485) && (n < 16524))
; 16485 to 16523
.if((n .mod 6 = 3) && ((n > 16484 && n < 16522)))
	; 5496..5508 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16486)
	; 5496 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 16486 && n < 16524)))
	; 5497..5509 bytes
	delay_n (n-2)
	nop
.elseif(n = 16488)
	; 5496 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16489)
	; 5497 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16492)
	; 5498 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16494)
	; 5498 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16495)
	; 5499 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16498)
	; 5500 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16500)
	; 5500 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16501)
	; 5501 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16504)
	; 5502 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16506)
	; 5502 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16507)
	; 5503 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16510)
	; 5504 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16512)
	; 5504 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16513)
	; 5505 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16516)
	; 5506 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16518)
	; 5506 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16519)
	; 5507 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16522)
	; 5508 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 16524 to 16562
.if(n = 16524)
	; 5508 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16525)
	; 5509 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 16525 && n < 16563)))
	; 5509..5521 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16526 && n < 16558)))
	; 5510..5520 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16528)
	; 5510 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16530)
	; 5510 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16531)
	; 5511 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16534)
	; 5512 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16536)
	; 5512 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16537)
	; 5513 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16540)
	; 5514 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16542)
	; 5514 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16543)
	; 5515 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16546)
	; 5516 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16548)
	; 5516 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16549)
	; 5517 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16552)
	; 5518 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16554)
	; 5518 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16555)
	; 5519 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16558)
	; 5520 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16560)
	; 5520 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16561)
	; 5521 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
.if((n >= 16563) && (n < 16602))
; 16563 to 16601
.if((n .mod 6 = 3) && ((n > 16562 && n < 16600)))
	; 5522..5534 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16564)
	; 5522 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 16564 && n < 16602)))
	; 5523..5535 bytes
	delay_n (n-2)
	nop
.elseif(n = 16566)
	; 5522 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16567)
	; 5523 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16570)
	; 5524 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16572)
	; 5524 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16573)
	; 5525 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16576)
	; 5526 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16578)
	; 5526 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16579)
	; 5527 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16582)
	; 5528 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16584)
	; 5528 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16585)
	; 5529 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16588)
	; 5530 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16590)
	; 5530 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16591)
	; 5531 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16594)
	; 5532 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16596)
	; 5532 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16597)
	; 5533 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16600)
	; 5534 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 16602 to 16640
.if(n = 16602)
	; 5534 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16603)
	; 5535 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 16603 && n < 16641)))
	; 5535..5547 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16604 && n < 16636)))
	; 5536..5546 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16606)
	; 5536 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16608)
	; 5536 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16609)
	; 5537 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16612)
	; 5538 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16614)
	; 5538 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16615)
	; 5539 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16618)
	; 5540 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16620)
	; 5540 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16621)
	; 5541 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16624)
	; 5542 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16626)
	; 5542 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16627)
	; 5543 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16630)
	; 5544 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16632)
	; 5544 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16633)
	; 5545 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16636)
	; 5546 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16638)
	; 5546 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16639)
	; 5547 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.else
.if((n >= 16641) && (n < 16680))
; 16641 to 16679
.if((n .mod 6 = 3) && ((n > 16640 && n < 16678)))
	; 5548..5560 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16642)
	; 5548 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 16642 && n < 16680)))
	; 5549..5561 bytes
	delay_n (n-2)
	nop
.elseif(n = 16644)
	; 5548 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16645)
	; 5549 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16648)
	; 5550 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16650)
	; 5550 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16651)
	; 5551 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16654)
	; 5552 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16656)
	; 5552 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16657)
	; 5553 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16660)
	; 5554 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16662)
	; 5554 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16663)
	; 5555 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16666)
	; 5556 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16668)
	; 5556 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16669)
	; 5557 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16672)
	; 5558 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16674)
	; 5558 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16675)
	; 5559 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16678)
	; 5560 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 16680 to 16718
.if(n = 16680)
	; 5560 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16681)
	; 5561 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 16681 && n < 16719)))
	; 5561..5573 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16682 && n < 16714)))
	; 5562..5572 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16684)
	; 5562 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16686)
	; 5562 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16687)
	; 5563 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16690)
	; 5564 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16692)
	; 5564 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16693)
	; 5565 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16696)
	; 5566 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16698)
	; 5566 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16699)
	; 5567 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16702)
	; 5568 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16704)
	; 5568 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16705)
	; 5569 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16708)
	; 5570 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16710)
	; 5570 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16711)
	; 5571 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16714)
	; 5572 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16716)
	; 5572 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16717)
	; 5573 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.endif
.else
.if((n >= 16719) && (n < 16797))
.if((n >= 16719) && (n < 16758))
; 16719 to 16757
.if((n .mod 6 = 3) && ((n > 16718 && n < 16756)))
	; 5574..5586 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16720)
	; 5574 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 16720 && n < 16758)))
	; 5575..5587 bytes
	delay_n (n-2)
	nop
.elseif(n = 16722)
	; 5574 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16723)
	; 5575 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16726)
	; 5576 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16728)
	; 5576 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16729)
	; 5577 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16732)
	; 5578 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16734)
	; 5578 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16735)
	; 5579 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16738)
	; 5580 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16740)
	; 5580 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16741)
	; 5581 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16744)
	; 5582 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16746)
	; 5582 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16747)
	; 5583 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16750)
	; 5584 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16752)
	; 5584 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16753)
	; 5585 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16756)
	; 5586 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 16758 to 16796
.if(n = 16758)
	; 5586 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16759)
	; 5587 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 16759 && n < 16797)))
	; 5587..5599 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16760 && n < 16792)))
	; 5588..5598 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16762)
	; 5588 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16764)
	; 5588 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16765)
	; 5589 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16768)
	; 5590 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16770)
	; 5590 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16771)
	; 5591 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16774)
	; 5592 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16776)
	; 5592 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16777)
	; 5593 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16780)
	; 5594 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16782)
	; 5594 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16783)
	; 5595 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16786)
	; 5596 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16788)
	; 5596 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16789)
	; 5597 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16792)
	; 5598 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16794)
	; 5598 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16795)
	; 5599 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 16797) && (n < 16836))
; 16797 to 16835
.if((n .mod 6 = 3) && ((n > 16796 && n < 16834)))
	; 5600..5612 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16798)
	; 5600 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 16798 && n < 16836)))
	; 5601..5613 bytes
	delay_n (n-2)
	nop
.elseif(n = 16800)
	; 5600 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16801)
	; 5601 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16804)
	; 5602 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16806)
	; 5602 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16807)
	; 5603 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16810)
	; 5604 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16812)
	; 5604 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16813)
	; 5605 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16816)
	; 5606 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16818)
	; 5606 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16819)
	; 5607 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16822)
	; 5608 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16824)
	; 5608 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16825)
	; 5609 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16828)
	; 5610 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16830)
	; 5610 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16831)
	; 5611 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16834)
	; 5612 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 16836 to 16875
.if(n = 16836)
	; 5612 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16837)
	; 5613 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 16837 && n < 16875)))
	; 5613..5625 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16838 && n < 16876)))
	; 5614..5626 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16840)
	; 5614 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16842)
	; 5614 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16843)
	; 5615 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16846)
	; 5616 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16848)
	; 5616 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16849)
	; 5617 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16852)
	; 5618 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16854)
	; 5618 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16855)
	; 5619 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16858)
	; 5620 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16860)
	; 5620 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16861)
	; 5621 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16864)
	; 5622 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16866)
	; 5622 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16867)
	; 5623 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16870)
	; 5624 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16872)
	; 5624 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16873)
	; 5625 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 16876) && (n < 17188))
.if((n >= 16876) && (n < 17032))
.if((n >= 16876) && (n < 16954))
.if((n >= 16876) && (n < 16915))
; 16876 to 16914
.if(n = 16876)
	; 5626 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 16876 && n < 16914)))
	; 5627..5639 bytes
	delay_n (n-2)
	nop
.elseif(n = 16878)
	; 5626 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16879)
	; 5627 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 16880 && n < 16912)))
	; 5628..5638 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16882)
	; 5628 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16884)
	; 5628 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16885)
	; 5629 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16888)
	; 5630 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16890)
	; 5630 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16891)
	; 5631 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16894)
	; 5632 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16896)
	; 5632 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16897)
	; 5633 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16900)
	; 5634 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16902)
	; 5634 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16903)
	; 5635 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16906)
	; 5636 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16908)
	; 5636 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16909)
	; 5637 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16912)
	; 5638 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16914)
	; 5638 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 16915 to 16953
.if(n = 16915)
	; 5639 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 16915 && n < 16953)))
	; 5639..5651 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16916 && n < 16954)))
	; 5640..5652 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16918)
	; 5640 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16920)
	; 5640 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16921)
	; 5641 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16924)
	; 5642 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16926)
	; 5642 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16927)
	; 5643 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16930)
	; 5644 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16932)
	; 5644 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16933)
	; 5645 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16936)
	; 5646 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16938)
	; 5646 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16939)
	; 5647 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16942)
	; 5648 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16944)
	; 5648 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16945)
	; 5649 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 16948)
	; 5650 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16950)
	; 5650 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16951)
	; 5651 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 16954) && (n < 16993))
; 16954 to 16992
.if(n = 16954)
	; 5652 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 16954 && n < 16992)))
	; 5653..5665 bytes
	delay_n (n-2)
	nop
.elseif(n = 16956)
	; 5652 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16957)
	; 5653 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 16958 && n < 16990)))
	; 5654..5664 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16960)
	; 5654 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16962)
	; 5654 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16963)
	; 5655 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 16966)
	; 5656 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16968)
	; 5656 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16969)
	; 5657 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16972)
	; 5658 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16974)
	; 5658 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16975)
	; 5659 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16978)
	; 5660 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16980)
	; 5660 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16981)
	; 5661 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16984)
	; 5662 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16986)
	; 5662 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16987)
	; 5663 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16990)
	; 5664 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16992)
	; 5664 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 16993 to 17031
.if(n = 16993)
	; 5665 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 16993 && n < 17031)))
	; 5665..5677 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 16994 && n < 17032)))
	; 5666..5678 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16996)
	; 5666 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16998)
	; 5666 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16999)
	; 5667 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17002)
	; 5668 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17004)
	; 5668 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17005)
	; 5669 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17008)
	; 5670 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17010)
	; 5670 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17011)
	; 5671 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17014)
	; 5672 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17016)
	; 5672 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17017)
	; 5673 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17020)
	; 5674 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17022)
	; 5674 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17023)
	; 5675 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17026)
	; 5676 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17028)
	; 5676 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17029)
	; 5677 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
.if((n >= 17032) && (n < 17071))
; 17032 to 17070
.if(n = 17032)
	; 5678 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 17032 && n < 17070)))
	; 5679..5691 bytes
	delay_n (n-2)
	nop
.elseif(n = 17034)
	; 5678 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17035)
	; 5679 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 17036 && n < 17068)))
	; 5680..5690 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17038)
	; 5680 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17040)
	; 5680 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17041)
	; 5681 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17044)
	; 5682 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17046)
	; 5682 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17047)
	; 5683 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17050)
	; 5684 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17052)
	; 5684 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17053)
	; 5685 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17056)
	; 5686 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17058)
	; 5686 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17059)
	; 5687 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17062)
	; 5688 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17064)
	; 5688 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17065)
	; 5689 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17068)
	; 5690 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17070)
	; 5690 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 17071 to 17109
.if(n = 17071)
	; 5691 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 17071 && n < 17109)))
	; 5691..5703 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17072 && n < 17110)))
	; 5692..5704 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17074)
	; 5692 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17076)
	; 5692 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17077)
	; 5693 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17080)
	; 5694 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17082)
	; 5694 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17083)
	; 5695 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17086)
	; 5696 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17088)
	; 5696 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17089)
	; 5697 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17092)
	; 5698 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17094)
	; 5698 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17095)
	; 5699 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17098)
	; 5700 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17100)
	; 5700 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17101)
	; 5701 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17104)
	; 5702 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17106)
	; 5702 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17107)
	; 5703 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.else
.if((n >= 17110) && (n < 17149))
; 17110 to 17148
.if(n = 17110)
	; 5704 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 17110 && n < 17148)))
	; 5705..5717 bytes
	delay_n (n-2)
	nop
.elseif(n = 17112)
	; 5704 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17113)
	; 5705 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 17114 && n < 17146)))
	; 5706..5716 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17116)
	; 5706 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17118)
	; 5706 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17119)
	; 5707 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17122)
	; 5708 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17124)
	; 5708 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17125)
	; 5709 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17128)
	; 5710 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17130)
	; 5710 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17131)
	; 5711 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17134)
	; 5712 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17136)
	; 5712 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17137)
	; 5713 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17140)
	; 5714 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17142)
	; 5714 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17143)
	; 5715 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17146)
	; 5716 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17148)
	; 5716 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 17149 to 17187
.if(n = 17149)
	; 5717 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 17149 && n < 17187)))
	; 5717..5729 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17150 && n < 17188)))
	; 5718..5730 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17152)
	; 5718 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17154)
	; 5718 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17155)
	; 5719 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17158)
	; 5720 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17160)
	; 5720 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17161)
	; 5721 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17164)
	; 5722 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17166)
	; 5722 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17167)
	; 5723 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17170)
	; 5724 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17172)
	; 5724 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17173)
	; 5725 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17176)
	; 5726 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17178)
	; 5726 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17179)
	; 5727 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17182)
	; 5728 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17184)
	; 5728 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17185)
	; 5729 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
.if((n >= 17188) && (n < 17266))
.if((n >= 17188) && (n < 17227))
; 17188 to 17226
.if(n = 17188)
	; 5730 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 17188 && n < 17226)))
	; 5731..5743 bytes
	delay_n (n-2)
	nop
.elseif(n = 17190)
	; 5730 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17191)
	; 5731 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 17192 && n < 17224)))
	; 5732..5742 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17194)
	; 5732 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17196)
	; 5732 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17197)
	; 5733 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17200)
	; 5734 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17202)
	; 5734 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17203)
	; 5735 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17206)
	; 5736 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17208)
	; 5736 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17209)
	; 5737 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17212)
	; 5738 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17214)
	; 5738 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17215)
	; 5739 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17218)
	; 5740 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17220)
	; 5740 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17221)
	; 5741 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17224)
	; 5742 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17226)
	; 5742 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 17227 to 17265
.if(n = 17227)
	; 5743 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 17227 && n < 17265)))
	; 5743..5755 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17228 && n < 17266)))
	; 5744..5756 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17230)
	; 5744 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17232)
	; 5744 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17233)
	; 5745 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17236)
	; 5746 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17238)
	; 5746 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17239)
	; 5747 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17242)
	; 5748 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17244)
	; 5748 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17245)
	; 5749 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17248)
	; 5750 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17250)
	; 5750 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17251)
	; 5751 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17254)
	; 5752 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17256)
	; 5752 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17257)
	; 5753 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17260)
	; 5754 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17262)
	; 5754 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17263)
	; 5755 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 17266) && (n < 17305))
; 17266 to 17304
.if(n = 17266)
	; 5756 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 17266 && n < 17304)))
	; 5757..5769 bytes
	delay_n (n-2)
	nop
.elseif(n = 17268)
	; 5756 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17269)
	; 5757 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 17270 && n < 17302)))
	; 5758..5768 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17272)
	; 5758 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17274)
	; 5758 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17275)
	; 5759 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17278)
	; 5760 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17280)
	; 5760 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17281)
	; 5761 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17284)
	; 5762 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17286)
	; 5762 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17287)
	; 5763 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17290)
	; 5764 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17292)
	; 5764 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17293)
	; 5765 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17296)
	; 5766 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17298)
	; 5766 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17299)
	; 5767 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17302)
	; 5768 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17304)
	; 5768 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 17305 to 17343
.if(n = 17305)
	; 5769 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 17305 && n < 17343)))
	; 5769..5781 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17306 && n < 17344)))
	; 5770..5782 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17308)
	; 5770 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17310)
	; 5770 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17311)
	; 5771 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17314)
	; 5772 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17316)
	; 5772 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17317)
	; 5773 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17320)
	; 5774 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17322)
	; 5774 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17323)
	; 5775 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17326)
	; 5776 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17328)
	; 5776 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17329)
	; 5777 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17332)
	; 5778 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17334)
	; 5778 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17335)
	; 5779 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17338)
	; 5780 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17340)
	; 5780 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17341)
	; 5781 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
; 17344 to 17382
.if(n = 17344)
	; 5782 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 17344 && n < 17382)))
	; 5783..5795 bytes
	delay_n (n-2)
	nop
.elseif(n = 17346)
	; 5782 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17347)
	; 5783 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 17348 && n < 17380)))
	; 5784..5794 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17350)
	; 5784 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17352)
	; 5784 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17353)
	; 5785 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17356)
	; 5786 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17358)
	; 5786 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17359)
	; 5787 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17362)
	; 5788 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17364)
	; 5788 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17365)
	; 5789 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17368)
	; 5790 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17370)
	; 5790 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17371)
	; 5791 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17374)
	; 5792 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17376)
	; 5792 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17377)
	; 5793 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17380)
	; 5794 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17382)
	; 5794 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 17383 to 17421
.if(n = 17383)
	; 5795 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 17383 && n < 17421)))
	; 5795..5807 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17384 && n < 17422)))
	; 5796..5808 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17386)
	; 5796 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17388)
	; 5796 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17389)
	; 5797 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17392)
	; 5798 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17394)
	; 5798 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17395)
	; 5799 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17398)
	; 5800 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17400)
	; 5800 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17401)
	; 5801 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17404)
	; 5802 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17406)
	; 5802 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17407)
	; 5803 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17410)
	; 5804 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17412)
	; 5804 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17413)
	; 5805 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17416)
	; 5806 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17418)
	; 5806 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17419)
	; 5807 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.else
.if((n >= 17422) && (n < 17461))
; 17422 to 17460
.if(n = 17422)
	; 5808 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 17422 && n < 17460)))
	; 5809..5821 bytes
	delay_n (n-2)
	nop
.elseif(n = 17424)
	; 5808 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17425)
	; 5809 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 17426 && n < 17458)))
	; 5810..5820 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17428)
	; 5810 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17430)
	; 5810 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17431)
	; 5811 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17434)
	; 5812 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17436)
	; 5812 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17437)
	; 5813 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17440)
	; 5814 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17442)
	; 5814 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17443)
	; 5815 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17446)
	; 5816 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17448)
	; 5816 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17449)
	; 5817 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17452)
	; 5818 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17454)
	; 5818 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17455)
	; 5819 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17458)
	; 5820 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17460)
	; 5820 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 17461 to 17500
.if(n = 17461)
	; 5821 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 17461 && n < 17499)))
	; 5821..5833 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17462 && n < 17500)))
	; 5822..5834 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17464)
	; 5822 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17466)
	; 5822 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17467)
	; 5823 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17470)
	; 5824 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17472)
	; 5824 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17473)
	; 5825 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17476)
	; 5826 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17478)
	; 5826 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17479)
	; 5827 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17482)
	; 5828 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17484)
	; 5828 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17485)
	; 5829 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17488)
	; 5830 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17490)
	; 5830 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17491)
	; 5831 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17494)
	; 5832 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17496)
	; 5832 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17497)
	; 5833 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17500)
	; 5834 bytes
	delay_n 58
	delay_n (n-58)
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
.if((n >= 17501) && (n < 17579))
.if((n >= 17501) && (n < 17540))
; 17501 to 17539
.if((n .mod 3 = 2) && ((n > 17500 && n < 17538)))
	; 5835..5847 bytes
	delay_n (n-2)
	nop
.elseif(n = 17502)
	; 5834 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17503)
	; 5835 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 17504 && n < 17536)))
	; 5836..5846 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17506)
	; 5836 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17508)
	; 5836 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17509)
	; 5837 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17512)
	; 5838 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17514)
	; 5838 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17515)
	; 5839 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17518)
	; 5840 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17520)
	; 5840 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17521)
	; 5841 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17524)
	; 5842 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17526)
	; 5842 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17527)
	; 5843 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17530)
	; 5844 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17532)
	; 5844 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17533)
	; 5845 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17536)
	; 5846 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17538)
	; 5846 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17539)
	; 5847 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 17540 to 17578
.if((n .mod 3 = 2) && ((n > 17539 && n < 17577)))
	; 5847..5859 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17540 && n < 17578)))
	; 5848..5860 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17542)
	; 5848 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17544)
	; 5848 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17545)
	; 5849 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17548)
	; 5850 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17550)
	; 5850 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17551)
	; 5851 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17554)
	; 5852 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17556)
	; 5852 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17557)
	; 5853 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17560)
	; 5854 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17562)
	; 5854 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17563)
	; 5855 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17566)
	; 5856 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17568)
	; 5856 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17569)
	; 5857 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17572)
	; 5858 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17574)
	; 5858 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17575)
	; 5859 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17578)
	; 5860 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 17579) && (n < 17618))
; 17579 to 17617
.if((n .mod 3 = 2) && ((n > 17578 && n < 17616)))
	; 5861..5873 bytes
	delay_n (n-2)
	nop
.elseif(n = 17580)
	; 5860 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17581)
	; 5861 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 17582 && n < 17614)))
	; 5862..5872 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17584)
	; 5862 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17586)
	; 5862 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17587)
	; 5863 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17590)
	; 5864 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17592)
	; 5864 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17593)
	; 5865 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17596)
	; 5866 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17598)
	; 5866 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17599)
	; 5867 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17602)
	; 5868 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17604)
	; 5868 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17605)
	; 5869 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17608)
	; 5870 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17610)
	; 5870 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17611)
	; 5871 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17614)
	; 5872 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17616)
	; 5872 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17617)
	; 5873 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 17618 to 17656
.if((n .mod 3 = 2) && ((n > 17617 && n < 17655)))
	; 5873..5885 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17618 && n < 17656)))
	; 5874..5886 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17620)
	; 5874 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17622)
	; 5874 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17623)
	; 5875 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17626)
	; 5876 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17628)
	; 5876 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17629)
	; 5877 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17632)
	; 5878 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17634)
	; 5878 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17635)
	; 5879 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17638)
	; 5880 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17640)
	; 5880 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17641)
	; 5881 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17644)
	; 5882 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17646)
	; 5882 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17647)
	; 5883 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17650)
	; 5884 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17652)
	; 5884 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17653)
	; 5885 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17656)
	; 5886 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 17657) && (n < 17735))
.if((n >= 17657) && (n < 17696))
; 17657 to 17695
.if((n .mod 3 = 2) && ((n > 17656 && n < 17694)))
	; 5887..5899 bytes
	delay_n (n-2)
	nop
.elseif(n = 17658)
	; 5886 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17659)
	; 5887 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 17660 && n < 17692)))
	; 5888..5898 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17662)
	; 5888 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17664)
	; 5888 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17665)
	; 5889 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17668)
	; 5890 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17670)
	; 5890 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17671)
	; 5891 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17674)
	; 5892 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17676)
	; 5892 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17677)
	; 5893 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17680)
	; 5894 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17682)
	; 5894 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17683)
	; 5895 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17686)
	; 5896 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17688)
	; 5896 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17689)
	; 5897 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17692)
	; 5898 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17694)
	; 5898 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17695)
	; 5899 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 17696 to 17734
.if((n .mod 3 = 2) && ((n > 17695 && n < 17733)))
	; 5899..5911 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17696 && n < 17734)))
	; 5900..5912 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17698)
	; 5900 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17700)
	; 5900 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17701)
	; 5901 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17704)
	; 5902 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17706)
	; 5902 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17707)
	; 5903 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17710)
	; 5904 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17712)
	; 5904 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17713)
	; 5905 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17716)
	; 5906 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17718)
	; 5906 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17719)
	; 5907 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17722)
	; 5908 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17724)
	; 5908 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17725)
	; 5909 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17728)
	; 5910 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17730)
	; 5910 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17731)
	; 5911 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17734)
	; 5912 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 17735) && (n < 17774))
; 17735 to 17773
.if((n .mod 3 = 2) && ((n > 17734 && n < 17772)))
	; 5913..5925 bytes
	delay_n (n-2)
	nop
.elseif(n = 17736)
	; 5912 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17737)
	; 5913 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 17738 && n < 17770)))
	; 5914..5924 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17740)
	; 5914 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17742)
	; 5914 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17743)
	; 5915 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17746)
	; 5916 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17748)
	; 5916 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17749)
	; 5917 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17752)
	; 5918 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17754)
	; 5918 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17755)
	; 5919 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17758)
	; 5920 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17760)
	; 5920 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17761)
	; 5921 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17764)
	; 5922 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17766)
	; 5922 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17767)
	; 5923 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17770)
	; 5924 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17772)
	; 5924 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17773)
	; 5925 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 17774 to 17812
.if((n .mod 3 = 2) && ((n > 17773 && n < 17811)))
	; 5925..5937 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17774 && n < 17812)))
	; 5926..5938 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17776)
	; 5926 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17778)
	; 5926 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17779)
	; 5927 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17782)
	; 5928 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17784)
	; 5928 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17785)
	; 5929 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17788)
	; 5930 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17790)
	; 5930 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17791)
	; 5931 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17794)
	; 5932 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17796)
	; 5932 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17797)
	; 5933 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17800)
	; 5934 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17802)
	; 5934 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17803)
	; 5935 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17806)
	; 5936 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17808)
	; 5936 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17809)
	; 5937 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17812)
	; 5938 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
.if((n >= 17813) && (n < 17852))
; 17813 to 17851
.if((n .mod 3 = 2) && ((n > 17812 && n < 17850)))
	; 5939..5951 bytes
	delay_n (n-2)
	nop
.elseif(n = 17814)
	; 5938 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17815)
	; 5939 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 17816 && n < 17848)))
	; 5940..5950 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17818)
	; 5940 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17820)
	; 5940 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17821)
	; 5941 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17824)
	; 5942 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17826)
	; 5942 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17827)
	; 5943 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17830)
	; 5944 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17832)
	; 5944 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17833)
	; 5945 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17836)
	; 5946 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17838)
	; 5946 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17839)
	; 5947 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17842)
	; 5948 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17844)
	; 5948 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17845)
	; 5949 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17848)
	; 5950 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17850)
	; 5950 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17851)
	; 5951 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 17852 to 17890
.if((n .mod 3 = 2) && ((n > 17851 && n < 17889)))
	; 5951..5963 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17852 && n < 17890)))
	; 5952..5964 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17854)
	; 5952 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17856)
	; 5952 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17857)
	; 5953 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17860)
	; 5954 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17862)
	; 5954 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17863)
	; 5955 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17866)
	; 5956 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17868)
	; 5956 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17869)
	; 5957 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17872)
	; 5958 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17874)
	; 5958 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17875)
	; 5959 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17878)
	; 5960 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17880)
	; 5960 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17881)
	; 5961 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17884)
	; 5962 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17886)
	; 5962 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17887)
	; 5963 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17890)
	; 5964 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 17891) && (n < 17930))
; 17891 to 17929
.if((n .mod 3 = 2) && ((n > 17890 && n < 17928)))
	; 5965..5977 bytes
	delay_n (n-2)
	nop
.elseif(n = 17892)
	; 5964 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17893)
	; 5965 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 17894 && n < 17926)))
	; 5966..5976 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17896)
	; 5966 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17898)
	; 5966 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17899)
	; 5967 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17902)
	; 5968 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17904)
	; 5968 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17905)
	; 5969 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17908)
	; 5970 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17910)
	; 5970 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17911)
	; 5971 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17914)
	; 5972 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17916)
	; 5972 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17917)
	; 5973 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 17920)
	; 5974 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17922)
	; 5974 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17923)
	; 5975 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17926)
	; 5976 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17928)
	; 5976 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17929)
	; 5977 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 17930 to 17968
.if((n .mod 3 = 2) && ((n > 17929 && n < 17967)))
	; 5977..5989 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 17930 && n < 17968)))
	; 5978..5990 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17932)
	; 5978 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17934)
	; 5978 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17935)
	; 5979 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17938)
	; 5980 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17940)
	; 5980 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17941)
	; 5981 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17944)
	; 5982 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17946)
	; 5982 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17947)
	; 5983 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17950)
	; 5984 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17952)
	; 5984 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17953)
	; 5985 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17956)
	; 5986 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17958)
	; 5986 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17959)
	; 5987 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17962)
	; 5988 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17964)
	; 5988 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17965)
	; 5989 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17968)
	; 5990 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.endif
.else
.if((n >= 17969) && (n < 18047))
.if((n >= 17969) && (n < 18008))
; 17969 to 18007
.if((n .mod 3 = 2) && ((n > 17968 && n < 18006)))
	; 5991..6003 bytes
	delay_n (n-2)
	nop
.elseif(n = 17970)
	; 5990 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17971)
	; 5991 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 17972 && n < 18004)))
	; 5992..6002 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 17974)
	; 5992 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17976)
	; 5992 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17977)
	; 5993 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17980)
	; 5994 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17982)
	; 5994 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17983)
	; 5995 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17986)
	; 5996 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17988)
	; 5996 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17989)
	; 5997 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 17992)
	; 5998 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17994)
	; 5998 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17995)
	; 5999 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17998)
	; 6000 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18000)
	; 6000 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18001)
	; 6001 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18004)
	; 6002 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18006)
	; 6002 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18007)
	; 6003 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 18008 to 18046
.if((n .mod 3 = 2) && ((n > 18007 && n < 18045)))
	; 6003..6015 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18008 && n < 18046)))
	; 6004..6016 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18010)
	; 6004 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18012)
	; 6004 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18013)
	; 6005 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18016)
	; 6006 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18018)
	; 6006 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18019)
	; 6007 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18022)
	; 6008 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18024)
	; 6008 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18025)
	; 6009 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18028)
	; 6010 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18030)
	; 6010 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18031)
	; 6011 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18034)
	; 6012 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18036)
	; 6012 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18037)
	; 6013 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18040)
	; 6014 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18042)
	; 6014 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18043)
	; 6015 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18046)
	; 6016 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 18047) && (n < 18086))
; 18047 to 18085
.if((n .mod 3 = 2) && ((n > 18046 && n < 18084)))
	; 6017..6029 bytes
	delay_n (n-2)
	nop
.elseif(n = 18048)
	; 6016 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18049)
	; 6017 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 18050 && n < 18082)))
	; 6018..6028 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18052)
	; 6018 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18054)
	; 6018 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18055)
	; 6019 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18058)
	; 6020 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18060)
	; 6020 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18061)
	; 6021 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18064)
	; 6022 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18066)
	; 6022 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18067)
	; 6023 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18070)
	; 6024 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18072)
	; 6024 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18073)
	; 6025 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18076)
	; 6026 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18078)
	; 6026 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18079)
	; 6027 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18082)
	; 6028 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18084)
	; 6028 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18085)
	; 6029 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 18086 to 18125
.if((n .mod 3 = 2) && ((n > 18085 && n < 18126)))
	; 6029..6043 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18086 && n < 18124)))
	; 6030..6042 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18088)
	; 6030 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18090)
	; 6030 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18091)
	; 6031 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18094)
	; 6032 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18096)
	; 6032 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18097)
	; 6033 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18100)
	; 6034 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18102)
	; 6034 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18103)
	; 6035 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18106)
	; 6036 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18108)
	; 6036 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18109)
	; 6037 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18112)
	; 6038 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18114)
	; 6038 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18115)
	; 6039 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18118)
	; 6040 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18120)
	; 6040 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18121)
	; 6041 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18124)
	; 6042 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 18126) && (n < 18438))
.if((n >= 18126) && (n < 18282))
.if((n >= 18126) && (n < 18204))
.if((n >= 18126) && (n < 18165))
; 18126 to 18164
.if(n = 18126)
	; 6042 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18127)
	; 6043 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 18127 && n < 18165)))
	; 6043..6055 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18128 && n < 18160)))
	; 6044..6054 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18130)
	; 6044 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18132)
	; 6044 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18133)
	; 6045 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18136)
	; 6046 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18138)
	; 6046 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18139)
	; 6047 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18142)
	; 6048 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18144)
	; 6048 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18145)
	; 6049 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18148)
	; 6050 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18150)
	; 6050 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18151)
	; 6051 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18154)
	; 6052 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18156)
	; 6052 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18157)
	; 6053 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18160)
	; 6054 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18162)
	; 6054 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18163)
	; 6055 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 18165 to 18203
.if((n .mod 6 = 3) && ((n > 18164 && n < 18202)))
	; 6056..6068 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18166)
	; 6056 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 18166 && n < 18204)))
	; 6057..6069 bytes
	delay_n (n-2)
	nop
.elseif(n = 18168)
	; 6056 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18169)
	; 6057 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18172)
	; 6058 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18174)
	; 6058 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18175)
	; 6059 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18178)
	; 6060 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18180)
	; 6060 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18181)
	; 6061 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18184)
	; 6062 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18186)
	; 6062 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18187)
	; 6063 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18190)
	; 6064 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18192)
	; 6064 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18193)
	; 6065 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18196)
	; 6066 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18198)
	; 6066 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18199)
	; 6067 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18202)
	; 6068 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 18204) && (n < 18243))
; 18204 to 18242
.if(n = 18204)
	; 6068 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18205)
	; 6069 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 18205 && n < 18243)))
	; 6069..6081 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18206 && n < 18238)))
	; 6070..6080 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18208)
	; 6070 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18210)
	; 6070 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18211)
	; 6071 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18214)
	; 6072 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18216)
	; 6072 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18217)
	; 6073 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18220)
	; 6074 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18222)
	; 6074 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18223)
	; 6075 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18226)
	; 6076 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18228)
	; 6076 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18229)
	; 6077 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18232)
	; 6078 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18234)
	; 6078 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18235)
	; 6079 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18238)
	; 6080 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18240)
	; 6080 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18241)
	; 6081 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 18243 to 18281
.if((n .mod 6 = 3) && ((n > 18242 && n < 18280)))
	; 6082..6094 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18244)
	; 6082 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 18244 && n < 18282)))
	; 6083..6095 bytes
	delay_n (n-2)
	nop
.elseif(n = 18246)
	; 6082 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18247)
	; 6083 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18250)
	; 6084 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18252)
	; 6084 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18253)
	; 6085 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18256)
	; 6086 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18258)
	; 6086 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18259)
	; 6087 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18262)
	; 6088 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18264)
	; 6088 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18265)
	; 6089 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18268)
	; 6090 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18270)
	; 6090 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18271)
	; 6091 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18274)
	; 6092 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18276)
	; 6092 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18277)
	; 6093 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18280)
	; 6094 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
.if((n >= 18282) && (n < 18321))
; 18282 to 18320
.if(n = 18282)
	; 6094 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18283)
	; 6095 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 18283 && n < 18321)))
	; 6095..6107 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18284 && n < 18316)))
	; 6096..6106 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18286)
	; 6096 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18288)
	; 6096 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18289)
	; 6097 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18292)
	; 6098 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18294)
	; 6098 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18295)
	; 6099 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18298)
	; 6100 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18300)
	; 6100 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18301)
	; 6101 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18304)
	; 6102 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18306)
	; 6102 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18307)
	; 6103 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18310)
	; 6104 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18312)
	; 6104 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18313)
	; 6105 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18316)
	; 6106 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18318)
	; 6106 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18319)
	; 6107 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 18321 to 18359
.if((n .mod 6 = 3) && ((n > 18320 && n < 18358)))
	; 6108..6120 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18322)
	; 6108 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 18322 && n < 18360)))
	; 6109..6121 bytes
	delay_n (n-2)
	nop
.elseif(n = 18324)
	; 6108 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18325)
	; 6109 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18328)
	; 6110 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18330)
	; 6110 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18331)
	; 6111 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18334)
	; 6112 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18336)
	; 6112 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18337)
	; 6113 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18340)
	; 6114 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18342)
	; 6114 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18343)
	; 6115 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18346)
	; 6116 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18348)
	; 6116 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18349)
	; 6117 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18352)
	; 6118 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18354)
	; 6118 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18355)
	; 6119 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18358)
	; 6120 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if(n = 18360)
	; 6120 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18361)
	; 6121 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 18361 && n < 18399)))
	; 6121..6133 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18362 && n < 18394)))
	; 6122..6132 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18364)
	; 6122 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18366)
	; 6122 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18367)
	; 6123 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18370)
	; 6124 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18372)
	; 6124 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18373)
	; 6125 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18376)
	; 6126 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18378)
	; 6126 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18379)
	; 6127 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18382)
	; 6128 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18384)
	; 6128 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18385)
	; 6129 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18388)
	; 6130 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18390)
	; 6130 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18391)
	; 6131 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18394)
	; 6132 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18396)
	; 6132 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18397)
	; 6133 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 18399 to 18437
.if((n .mod 6 = 3) && ((n > 18398 && n < 18436)))
	; 6134..6146 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18400)
	; 6134 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 18400 && n < 18438)))
	; 6135..6147 bytes
	delay_n (n-2)
	nop
.elseif(n = 18402)
	; 6134 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18403)
	; 6135 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18406)
	; 6136 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18408)
	; 6136 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18409)
	; 6137 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18412)
	; 6138 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18414)
	; 6138 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18415)
	; 6139 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18418)
	; 6140 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18420)
	; 6140 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18421)
	; 6141 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18424)
	; 6142 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18426)
	; 6142 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18427)
	; 6143 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18430)
	; 6144 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18432)
	; 6144 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18433)
	; 6145 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18436)
	; 6146 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.endif
.else
.if((n >= 18438) && (n < 18594))
.if((n >= 18438) && (n < 18516))
.if((n >= 18438) && (n < 18477))
; 18438 to 18476
.if(n = 18438)
	; 6146 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18439)
	; 6147 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 18439 && n < 18477)))
	; 6147..6159 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18440 && n < 18472)))
	; 6148..6158 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18442)
	; 6148 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18444)
	; 6148 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18445)
	; 6149 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18448)
	; 6150 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18450)
	; 6150 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18451)
	; 6151 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18454)
	; 6152 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18456)
	; 6152 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18457)
	; 6153 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18460)
	; 6154 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18462)
	; 6154 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18463)
	; 6155 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18466)
	; 6156 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18468)
	; 6156 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18469)
	; 6157 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18472)
	; 6158 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18474)
	; 6158 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18475)
	; 6159 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 18477 to 18515
.if((n .mod 6 = 3) && ((n > 18476 && n < 18514)))
	; 6160..6172 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18478)
	; 6160 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 18478 && n < 18516)))
	; 6161..6173 bytes
	delay_n (n-2)
	nop
.elseif(n = 18480)
	; 6160 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18481)
	; 6161 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18484)
	; 6162 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18486)
	; 6162 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18487)
	; 6163 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18490)
	; 6164 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18492)
	; 6164 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18493)
	; 6165 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18496)
	; 6166 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18498)
	; 6166 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18499)
	; 6167 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18502)
	; 6168 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18504)
	; 6168 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18505)
	; 6169 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18508)
	; 6170 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18510)
	; 6170 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18511)
	; 6171 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18514)
	; 6172 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 18516) && (n < 18555))
; 18516 to 18554
.if(n = 18516)
	; 6172 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18517)
	; 6173 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 18517 && n < 18555)))
	; 6173..6185 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18518 && n < 18550)))
	; 6174..6184 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18520)
	; 6174 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18522)
	; 6174 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18523)
	; 6175 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18526)
	; 6176 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18528)
	; 6176 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18529)
	; 6177 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18532)
	; 6178 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18534)
	; 6178 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18535)
	; 6179 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18538)
	; 6180 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18540)
	; 6180 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18541)
	; 6181 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18544)
	; 6182 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18546)
	; 6182 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18547)
	; 6183 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18550)
	; 6184 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18552)
	; 6184 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18553)
	; 6185 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 18555 to 18593
.if((n .mod 6 = 3) && ((n > 18554 && n < 18592)))
	; 6186..6198 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18556)
	; 6186 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 18556 && n < 18594)))
	; 6187..6199 bytes
	delay_n (n-2)
	nop
.elseif(n = 18558)
	; 6186 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18559)
	; 6187 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18562)
	; 6188 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18564)
	; 6188 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18565)
	; 6189 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18568)
	; 6190 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18570)
	; 6190 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18571)
	; 6191 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18574)
	; 6192 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18576)
	; 6192 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18577)
	; 6193 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18580)
	; 6194 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18582)
	; 6194 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18583)
	; 6195 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18586)
	; 6196 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18588)
	; 6196 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18589)
	; 6197 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18592)
	; 6198 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
.if((n >= 18594) && (n < 18633))
; 18594 to 18632
.if(n = 18594)
	; 6198 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18595)
	; 6199 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 18595 && n < 18633)))
	; 6199..6211 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18596 && n < 18628)))
	; 6200..6210 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18598)
	; 6200 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18600)
	; 6200 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18601)
	; 6201 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18604)
	; 6202 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18606)
	; 6202 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18607)
	; 6203 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18610)
	; 6204 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18612)
	; 6204 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18613)
	; 6205 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18616)
	; 6206 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18618)
	; 6206 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18619)
	; 6207 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18622)
	; 6208 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18624)
	; 6208 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18625)
	; 6209 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18628)
	; 6210 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18630)
	; 6210 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18631)
	; 6211 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 18633 to 18671
.if((n .mod 6 = 3) && ((n > 18632 && n < 18670)))
	; 6212..6224 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18634)
	; 6212 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 18634 && n < 18672)))
	; 6213..6225 bytes
	delay_n (n-2)
	nop
.elseif(n = 18636)
	; 6212 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18637)
	; 6213 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18640)
	; 6214 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18642)
	; 6214 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18643)
	; 6215 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18646)
	; 6216 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18648)
	; 6216 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18649)
	; 6217 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18652)
	; 6218 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18654)
	; 6218 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18655)
	; 6219 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18658)
	; 6220 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18660)
	; 6220 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18661)
	; 6221 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18664)
	; 6222 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18666)
	; 6222 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18667)
	; 6223 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18670)
	; 6224 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 18672) && (n < 18711))
; 18672 to 18710
.if(n = 18672)
	; 6224 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18673)
	; 6225 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 3 = 2) && ((n > 18673 && n < 18711)))
	; 6225..6237 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18674 && n < 18706)))
	; 6226..6236 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18676)
	; 6226 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18678)
	; 6226 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18679)
	; 6227 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18682)
	; 6228 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18684)
	; 6228 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18685)
	; 6229 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18688)
	; 6230 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18690)
	; 6230 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18691)
	; 6231 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18694)
	; 6232 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18696)
	; 6232 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18697)
	; 6233 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18700)
	; 6234 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18702)
	; 6234 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18703)
	; 6235 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18706)
	; 6236 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18708)
	; 6236 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18709)
	; 6237 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 18711 to 18750
.if((n .mod 6 = 3) && ((n > 18710 && n < 18748)))
	; 6238..6250 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18712)
	; 6238 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 18712 && n < 18750)))
	; 6239..6251 bytes
	delay_n (n-2)
	nop
.elseif(n = 18714)
	; 6238 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18715)
	; 6239 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18718)
	; 6240 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18720)
	; 6240 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18721)
	; 6241 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18724)
	; 6242 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18726)
	; 6242 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18727)
	; 6243 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18730)
	; 6244 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18732)
	; 6244 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18733)
	; 6245 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18736)
	; 6246 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18738)
	; 6246 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18739)
	; 6247 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18742)
	; 6248 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18744)
	; 6248 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18745)
	; 6249 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18748)
	; 6250 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18750)
	; 6250 bytes
	delay_n 12
	delay_n (n-12)
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
.if((n >= 18751) && (n < 18829))
.if((n >= 18751) && (n < 18790))
; 18751 to 18789
.if(n = 18751)
	; 6251 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 18751 && n < 18789)))
	; 6251..6263 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18752 && n < 18790)))
	; 6252..6264 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18754)
	; 6252 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18756)
	; 6252 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18757)
	; 6253 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18760)
	; 6254 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18762)
	; 6254 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18763)
	; 6255 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18766)
	; 6256 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18768)
	; 6256 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18769)
	; 6257 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18772)
	; 6258 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18774)
	; 6258 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18775)
	; 6259 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18778)
	; 6260 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18780)
	; 6260 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18781)
	; 6261 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18784)
	; 6262 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18786)
	; 6262 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18787)
	; 6263 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 18790 to 18828
.if(n = 18790)
	; 6264 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 18790 && n < 18828)))
	; 6265..6277 bytes
	delay_n (n-2)
	nop
.elseif(n = 18792)
	; 6264 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18793)
	; 6265 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 18794 && n < 18826)))
	; 6266..6276 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18796)
	; 6266 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18798)
	; 6266 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18799)
	; 6267 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18802)
	; 6268 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18804)
	; 6268 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18805)
	; 6269 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18808)
	; 6270 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18810)
	; 6270 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18811)
	; 6271 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18814)
	; 6272 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18816)
	; 6272 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18817)
	; 6273 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18820)
	; 6274 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18822)
	; 6274 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18823)
	; 6275 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18826)
	; 6276 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18828)
	; 6276 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.else
.if((n >= 18829) && (n < 18868))
; 18829 to 18867
.if(n = 18829)
	; 6277 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 18829 && n < 18867)))
	; 6277..6289 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18830 && n < 18868)))
	; 6278..6290 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18832)
	; 6278 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18834)
	; 6278 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18835)
	; 6279 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18838)
	; 6280 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18840)
	; 6280 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18841)
	; 6281 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18844)
	; 6282 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18846)
	; 6282 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18847)
	; 6283 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18850)
	; 6284 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18852)
	; 6284 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18853)
	; 6285 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18856)
	; 6286 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18858)
	; 6286 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18859)
	; 6287 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18862)
	; 6288 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18864)
	; 6288 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18865)
	; 6289 bytes
	delay_n 19
	delay_n (n-19)
.endif
.else
; 18868 to 18906
.if(n = 18868)
	; 6290 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 18868 && n < 18906)))
	; 6291..6303 bytes
	delay_n (n-2)
	nop
.elseif(n = 18870)
	; 6290 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18871)
	; 6291 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 18872 && n < 18904)))
	; 6292..6302 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18874)
	; 6292 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18876)
	; 6292 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18877)
	; 6293 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18880)
	; 6294 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18882)
	; 6294 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18883)
	; 6295 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18886)
	; 6296 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18888)
	; 6296 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18889)
	; 6297 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 18892)
	; 6298 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18894)
	; 6298 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18895)
	; 6299 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18898)
	; 6300 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18900)
	; 6300 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18901)
	; 6301 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18904)
	; 6302 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18906)
	; 6302 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
; 18907 to 18945
.if(n = 18907)
	; 6303 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 3 = 2) && ((n > 18907 && n < 18945)))
	; 6303..6315 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18908 && n < 18946)))
	; 6304..6316 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18910)
	; 6304 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18912)
	; 6304 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18913)
	; 6305 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18916)
	; 6306 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18918)
	; 6306 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18919)
	; 6307 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18922)
	; 6308 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18924)
	; 6308 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18925)
	; 6309 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18928)
	; 6310 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18930)
	; 6310 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18931)
	; 6311 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18934)
	; 6312 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18936)
	; 6312 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18937)
	; 6313 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18940)
	; 6314 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18942)
	; 6314 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18943)
	; 6315 bytes
	delay_n 43
	delay_n (n-43)
.endif
.else
; 18946 to 18984
.if(n = 18946)
	; 6316 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 18946 && n < 18984)))
	; 6317..6329 bytes
	delay_n (n-2)
	nop
.elseif(n = 18948)
	; 6316 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18949)
	; 6317 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 18950 && n < 18982)))
	; 6318..6328 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18952)
	; 6318 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18954)
	; 6318 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18955)
	; 6319 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18958)
	; 6320 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18960)
	; 6320 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18961)
	; 6321 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 18964)
	; 6322 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18966)
	; 6322 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18967)
	; 6323 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18970)
	; 6324 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18972)
	; 6324 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18973)
	; 6325 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18976)
	; 6326 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18978)
	; 6326 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18979)
	; 6327 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18982)
	; 6328 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18984)
	; 6328 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.else
.if((n >= 18985) && (n < 19024))
; 18985 to 19023
.if(n = 18985)
	; 6329 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 3 = 2) && ((n > 18985 && n < 19023)))
	; 6329..6341 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 18986 && n < 19024)))
	; 6330..6342 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 18988)
	; 6330 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18990)
	; 6330 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18991)
	; 6331 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18994)
	; 6332 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18996)
	; 6332 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18997)
	; 6333 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19000)
	; 6334 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19002)
	; 6334 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19003)
	; 6335 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19006)
	; 6336 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19008)
	; 6336 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19009)
	; 6337 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19012)
	; 6338 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19014)
	; 6338 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19015)
	; 6339 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19018)
	; 6340 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19020)
	; 6340 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19021)
	; 6341 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 19024 to 19062
.if(n = 19024)
	; 6342 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 19024 && n < 19062)))
	; 6343..6355 bytes
	delay_n (n-2)
	nop
.elseif(n = 19026)
	; 6342 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19027)
	; 6343 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 19028 && n < 19060)))
	; 6344..6354 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19030)
	; 6344 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19032)
	; 6344 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19033)
	; 6345 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19036)
	; 6346 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19038)
	; 6346 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19039)
	; 6347 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19042)
	; 6348 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19044)
	; 6348 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19045)
	; 6349 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19048)
	; 6350 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19050)
	; 6350 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19051)
	; 6351 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19054)
	; 6352 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19056)
	; 6352 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19057)
	; 6353 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19060)
	; 6354 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19062)
	; 6354 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.endif
.endif
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
.if((n >= 19063) && (n < 19102))
; 19063 to 19101
.if(n = 19063)
	; 6355 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 19063 && n < 19101)))
	; 6355..6367 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19064 && n < 19102)))
	; 6356..6368 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19066)
	; 6356 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19068)
	; 6356 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19069)
	; 6357 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19072)
	; 6358 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19074)
	; 6358 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19075)
	; 6359 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19078)
	; 6360 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19080)
	; 6360 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19081)
	; 6361 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19084)
	; 6362 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19086)
	; 6362 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19087)
	; 6363 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19090)
	; 6364 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19092)
	; 6364 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19093)
	; 6365 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19096)
	; 6366 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19098)
	; 6366 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19099)
	; 6367 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 19102 to 19140
.if(n = 19102)
	; 6368 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 19102 && n < 19140)))
	; 6369..6381 bytes
	delay_n (n-2)
	nop
.elseif(n = 19104)
	; 6368 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19105)
	; 6369 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 19106 && n < 19138)))
	; 6370..6380 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19108)
	; 6370 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19110)
	; 6370 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19111)
	; 6371 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19114)
	; 6372 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19116)
	; 6372 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19117)
	; 6373 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19120)
	; 6374 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19122)
	; 6374 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19123)
	; 6375 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19126)
	; 6376 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19128)
	; 6376 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19129)
	; 6377 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19132)
	; 6378 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19134)
	; 6378 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19135)
	; 6379 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19138)
	; 6380 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19140)
	; 6380 bytes
	delay_n 24
	delay_n (n-24)
.endif
.endif
.else
.if((n >= 19141) && (n < 19180))
; 19141 to 19179
.if(n = 19141)
	; 6381 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 19141 && n < 19179)))
	; 6381..6393 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19142 && n < 19180)))
	; 6382..6394 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19144)
	; 6382 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19146)
	; 6382 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19147)
	; 6383 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19150)
	; 6384 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19152)
	; 6384 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19153)
	; 6385 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19156)
	; 6386 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19158)
	; 6386 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19159)
	; 6387 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19162)
	; 6388 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19164)
	; 6388 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19165)
	; 6389 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19168)
	; 6390 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19170)
	; 6390 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19171)
	; 6391 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19174)
	; 6392 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19176)
	; 6392 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19177)
	; 6393 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.else
; 19180 to 19218
.if(n = 19180)
	; 6394 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 19180 && n < 19218)))
	; 6395..6407 bytes
	delay_n (n-2)
	nop
.elseif(n = 19182)
	; 6394 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19183)
	; 6395 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 19184 && n < 19216)))
	; 6396..6406 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19186)
	; 6396 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19188)
	; 6396 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19189)
	; 6397 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19192)
	; 6398 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19194)
	; 6398 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19195)
	; 6399 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19198)
	; 6400 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19200)
	; 6400 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19201)
	; 6401 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19204)
	; 6402 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19206)
	; 6402 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19207)
	; 6403 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19210)
	; 6404 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19212)
	; 6404 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19213)
	; 6405 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19216)
	; 6406 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19218)
	; 6406 bytes
	delay_n 48
	delay_n (n-48)
.endif
.endif
.endif
.else
.if((n >= 19219) && (n < 19297))
.if((n >= 19219) && (n < 19258))
; 19219 to 19257
.if(n = 19219)
	; 6407 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 19219 && n < 19257)))
	; 6407..6419 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19220 && n < 19258)))
	; 6408..6420 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19222)
	; 6408 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19224)
	; 6408 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19225)
	; 6409 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19228)
	; 6410 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19230)
	; 6410 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19231)
	; 6411 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19234)
	; 6412 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19236)
	; 6412 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19237)
	; 6413 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19240)
	; 6414 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19242)
	; 6414 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19243)
	; 6415 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19246)
	; 6416 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19248)
	; 6416 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19249)
	; 6417 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19252)
	; 6418 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19254)
	; 6418 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19255)
	; 6419 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 19258 to 19296
.if(n = 19258)
	; 6420 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 19258 && n < 19296)))
	; 6421..6433 bytes
	delay_n (n-2)
	nop
.elseif(n = 19260)
	; 6420 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19261)
	; 6421 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 19262 && n < 19294)))
	; 6422..6432 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19264)
	; 6422 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19266)
	; 6422 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19267)
	; 6423 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19270)
	; 6424 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19272)
	; 6424 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19273)
	; 6425 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19276)
	; 6426 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19278)
	; 6426 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19279)
	; 6427 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19282)
	; 6428 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19284)
	; 6428 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19285)
	; 6429 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19288)
	; 6430 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19290)
	; 6430 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19291)
	; 6431 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19294)
	; 6432 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19296)
	; 6432 bytes
	delay_n 72
	delay_n (n-72)
.endif
.endif
.else
.if((n >= 19297) && (n < 19336))
; 19297 to 19335
.if(n = 19297)
	; 6433 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 3 = 2) && ((n > 19297 && n < 19335)))
	; 6433..6445 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19298 && n < 19336)))
	; 6434..6446 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19300)
	; 6434 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19302)
	; 6434 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19303)
	; 6435 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19306)
	; 6436 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19308)
	; 6436 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19309)
	; 6437 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19312)
	; 6438 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19314)
	; 6438 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19315)
	; 6439 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19318)
	; 6440 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19320)
	; 6440 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19321)
	; 6441 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19324)
	; 6442 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19326)
	; 6442 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19327)
	; 6443 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19330)
	; 6444 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19332)
	; 6444 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19333)
	; 6445 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 19336 to 19375
.if(n = 19336)
	; 6446 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 19336 && n < 19374)))
	; 6447..6459 bytes
	delay_n (n-2)
	nop
.elseif(n = 19338)
	; 6446 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19339)
	; 6447 bytes
	inc $0700,X
	delay_n (n-7)
.elseif((n .mod 6 = 3) && ((n > 19340 && n < 19372)))
	; 6448..6458 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19342)
	; 6448 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19344)
	; 6448 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19345)
	; 6449 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19348)
	; 6450 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19350)
	; 6450 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19351)
	; 6451 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19354)
	; 6452 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19356)
	; 6452 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19357)
	; 6453 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19360)
	; 6454 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19362)
	; 6454 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19363)
	; 6455 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19366)
	; 6456 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19368)
	; 6456 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19369)
	; 6457 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19372)
	; 6458 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19374)
	; 6458 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19375)
	; 6459 bytes
	delay_n 43
	delay_n (n-43)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 19376) && (n < 19688))
.if((n >= 19376) && (n < 19532))
.if((n >= 19376) && (n < 19454))
.if((n >= 19376) && (n < 19415))
; 19376 to 19414
.if((n .mod 3 = 2) && ((n > 19375 && n < 19413)))
	; 6459..6471 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19376 && n < 19414)))
	; 6460..6472 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19378)
	; 6460 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19380)
	; 6460 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19381)
	; 6461 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19384)
	; 6462 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19386)
	; 6462 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19387)
	; 6463 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19390)
	; 6464 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19392)
	; 6464 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19393)
	; 6465 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19396)
	; 6466 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19398)
	; 6466 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19399)
	; 6467 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19402)
	; 6468 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19404)
	; 6468 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19405)
	; 6469 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19408)
	; 6470 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19410)
	; 6470 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19411)
	; 6471 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19414)
	; 6472 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 19415 to 19453
.if((n .mod 3 = 2) && ((n > 19414 && n < 19452)))
	; 6473..6485 bytes
	delay_n (n-2)
	nop
.elseif(n = 19416)
	; 6472 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19417)
	; 6473 bytes
	delay_n 31
	delay_n (n-31)
.elseif((n .mod 6 = 3) && ((n > 19418 && n < 19450)))
	; 6474..6484 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19420)
	; 6474 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19422)
	; 6474 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19423)
	; 6475 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19426)
	; 6476 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19428)
	; 6476 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19429)
	; 6477 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19432)
	; 6478 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19434)
	; 6478 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19435)
	; 6479 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19438)
	; 6480 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19440)
	; 6480 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19441)
	; 6481 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19444)
	; 6482 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19446)
	; 6482 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19447)
	; 6483 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19450)
	; 6484 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19452)
	; 6484 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19453)
	; 6485 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 19454) && (n < 19493))
; 19454 to 19492
.if((n .mod 3 = 2) && ((n > 19453 && n < 19491)))
	; 6485..6497 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19454 && n < 19492)))
	; 6486..6498 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19456)
	; 6486 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19458)
	; 6486 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19459)
	; 6487 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19462)
	; 6488 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19464)
	; 6488 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19465)
	; 6489 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19468)
	; 6490 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19470)
	; 6490 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19471)
	; 6491 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19474)
	; 6492 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19476)
	; 6492 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19477)
	; 6493 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19480)
	; 6494 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19482)
	; 6494 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19483)
	; 6495 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19486)
	; 6496 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19488)
	; 6496 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19489)
	; 6497 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19492)
	; 6498 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 19493 to 19531
.if((n .mod 3 = 2) && ((n > 19492 && n < 19530)))
	; 6499..6511 bytes
	delay_n (n-2)
	nop
.elseif(n = 19494)
	; 6498 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19495)
	; 6499 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 6 = 3) && ((n > 19496 && n < 19528)))
	; 6500..6510 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19498)
	; 6500 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19500)
	; 6500 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19501)
	; 6501 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19504)
	; 6502 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19506)
	; 6502 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19507)
	; 6503 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19510)
	; 6504 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19512)
	; 6504 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19513)
	; 6505 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19516)
	; 6506 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19518)
	; 6506 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19519)
	; 6507 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19522)
	; 6508 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19524)
	; 6508 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19525)
	; 6509 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19528)
	; 6510 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19530)
	; 6510 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19531)
	; 6511 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 19532) && (n < 19610))
.if((n >= 19532) && (n < 19571))
; 19532 to 19570
.if((n .mod 3 = 2) && ((n > 19531 && n < 19569)))
	; 6511..6523 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19532 && n < 19570)))
	; 6512..6524 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19534)
	; 6512 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19536)
	; 6512 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19537)
	; 6513 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19540)
	; 6514 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19542)
	; 6514 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19543)
	; 6515 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19546)
	; 6516 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19548)
	; 6516 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19549)
	; 6517 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19552)
	; 6518 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19554)
	; 6518 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19555)
	; 6519 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19558)
	; 6520 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19560)
	; 6520 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19561)
	; 6521 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19564)
	; 6522 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19566)
	; 6522 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19567)
	; 6523 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19570)
	; 6524 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 19571 to 19609
.if((n .mod 3 = 2) && ((n > 19570 && n < 19608)))
	; 6525..6537 bytes
	delay_n (n-2)
	nop
.elseif(n = 19572)
	; 6524 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19573)
	; 6525 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 6 = 3) && ((n > 19574 && n < 19606)))
	; 6526..6536 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19576)
	; 6526 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19578)
	; 6526 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19579)
	; 6527 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19582)
	; 6528 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19584)
	; 6528 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19585)
	; 6529 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19588)
	; 6530 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19590)
	; 6530 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19591)
	; 6531 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19594)
	; 6532 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19596)
	; 6532 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19597)
	; 6533 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19600)
	; 6534 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19602)
	; 6534 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19603)
	; 6535 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19606)
	; 6536 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19608)
	; 6536 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19609)
	; 6537 bytes
	inc $0700,X
	delay_n (n-7)
.endif
.endif
.else
.if((n >= 19610) && (n < 19649))
; 19610 to 19648
.if((n .mod 3 = 2) && ((n > 19609 && n < 19647)))
	; 6537..6549 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19610 && n < 19648)))
	; 6538..6550 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19612)
	; 6538 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19614)
	; 6538 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19615)
	; 6539 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19618)
	; 6540 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19620)
	; 6540 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19621)
	; 6541 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19624)
	; 6542 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19626)
	; 6542 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19627)
	; 6543 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19630)
	; 6544 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19632)
	; 6544 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19633)
	; 6545 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19636)
	; 6546 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19638)
	; 6546 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19639)
	; 6547 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19642)
	; 6548 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19644)
	; 6548 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19645)
	; 6549 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19648)
	; 6550 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 19649 to 19687
.if((n .mod 3 = 2) && ((n > 19648 && n < 19686)))
	; 6551..6563 bytes
	delay_n (n-2)
	nop
.elseif(n = 19650)
	; 6550 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19651)
	; 6551 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 6 = 3) && ((n > 19652 && n < 19684)))
	; 6552..6562 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19654)
	; 6552 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19656)
	; 6552 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19657)
	; 6553 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19660)
	; 6554 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19662)
	; 6554 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19663)
	; 6555 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19666)
	; 6556 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19668)
	; 6556 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19669)
	; 6557 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19672)
	; 6558 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19674)
	; 6558 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19675)
	; 6559 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19678)
	; 6560 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19680)
	; 6560 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19681)
	; 6561 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19684)
	; 6562 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19686)
	; 6562 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19687)
	; 6563 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.endif
.endif
.else
.if((n >= 19688) && (n < 19844))
.if((n >= 19688) && (n < 19766))
.if((n >= 19688) && (n < 19727))
; 19688 to 19726
.if((n .mod 3 = 2) && ((n > 19687 && n < 19725)))
	; 6563..6575 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19688 && n < 19726)))
	; 6564..6576 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19690)
	; 6564 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19692)
	; 6564 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19693)
	; 6565 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19696)
	; 6566 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19698)
	; 6566 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19699)
	; 6567 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19702)
	; 6568 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19704)
	; 6568 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19705)
	; 6569 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19708)
	; 6570 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19710)
	; 6570 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19711)
	; 6571 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19714)
	; 6572 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19716)
	; 6572 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19717)
	; 6573 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19720)
	; 6574 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19722)
	; 6574 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19723)
	; 6575 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19726)
	; 6576 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 19727 to 19765
.if((n .mod 3 = 2) && ((n > 19726 && n < 19764)))
	; 6577..6589 bytes
	delay_n (n-2)
	nop
.elseif(n = 19728)
	; 6576 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19729)
	; 6577 bytes
	delay_n 19
	delay_n (n-19)
.elseif((n .mod 6 = 3) && ((n > 19730 && n < 19762)))
	; 6578..6588 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19732)
	; 6578 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19734)
	; 6578 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19735)
	; 6579 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19738)
	; 6580 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19740)
	; 6580 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19741)
	; 6581 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19744)
	; 6582 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19746)
	; 6582 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19747)
	; 6583 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19750)
	; 6584 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19752)
	; 6584 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19753)
	; 6585 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19756)
	; 6586 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19758)
	; 6586 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19759)
	; 6587 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19762)
	; 6588 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19764)
	; 6588 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19765)
	; 6589 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 19766) && (n < 19805))
; 19766 to 19804
.if((n .mod 3 = 2) && ((n > 19765 && n < 19803)))
	; 6589..6601 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19766 && n < 19804)))
	; 6590..6602 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19768)
	; 6590 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19770)
	; 6590 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19771)
	; 6591 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19774)
	; 6592 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19776)
	; 6592 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19777)
	; 6593 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19780)
	; 6594 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19782)
	; 6594 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19783)
	; 6595 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19786)
	; 6596 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19788)
	; 6596 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19789)
	; 6597 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19792)
	; 6598 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19794)
	; 6598 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19795)
	; 6599 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19798)
	; 6600 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19800)
	; 6600 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19801)
	; 6601 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19804)
	; 6602 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 19805 to 19843
.if((n .mod 3 = 2) && ((n > 19804 && n < 19842)))
	; 6603..6615 bytes
	delay_n (n-2)
	nop
.elseif(n = 19806)
	; 6602 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19807)
	; 6603 bytes
	delay_n 43
	delay_n (n-43)
.elseif((n .mod 6 = 3) && ((n > 19808 && n < 19840)))
	; 6604..6614 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19810)
	; 6604 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19812)
	; 6604 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19813)
	; 6605 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19816)
	; 6606 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19818)
	; 6606 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19819)
	; 6607 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19822)
	; 6608 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19824)
	; 6608 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19825)
	; 6609 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19828)
	; 6610 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19830)
	; 6610 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19831)
	; 6611 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19834)
	; 6612 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19836)
	; 6612 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19837)
	; 6613 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19840)
	; 6614 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19842)
	; 6614 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19843)
	; 6615 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
.if((n >= 19844) && (n < 19883))
; 19844 to 19882
.if((n .mod 3 = 2) && ((n > 19843 && n < 19881)))
	; 6615..6627 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19844 && n < 19882)))
	; 6616..6628 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19846)
	; 6616 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19848)
	; 6616 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19849)
	; 6617 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19852)
	; 6618 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19854)
	; 6618 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19855)
	; 6619 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19858)
	; 6620 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19860)
	; 6620 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19861)
	; 6621 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19864)
	; 6622 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19866)
	; 6622 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19867)
	; 6623 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19870)
	; 6624 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19872)
	; 6624 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19873)
	; 6625 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19876)
	; 6626 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19878)
	; 6626 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19879)
	; 6627 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19882)
	; 6628 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 19883 to 19921
.if((n .mod 3 = 2) && ((n > 19882 && n < 19920)))
	; 6629..6641 bytes
	delay_n (n-2)
	nop
.elseif(n = 19884)
	; 6628 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19885)
	; 6629 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 6 = 3) && ((n > 19886 && n < 19918)))
	; 6630..6640 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19888)
	; 6630 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19890)
	; 6630 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19891)
	; 6631 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19894)
	; 6632 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19896)
	; 6632 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19897)
	; 6633 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19900)
	; 6634 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19902)
	; 6634 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19903)
	; 6635 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19906)
	; 6636 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19908)
	; 6636 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19909)
	; 6637 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19912)
	; 6638 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19914)
	; 6638 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19915)
	; 6639 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19918)
	; 6640 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19920)
	; 6640 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19921)
	; 6641 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 19922) && (n < 19961))
; 19922 to 19960
.if((n .mod 3 = 2) && ((n > 19921 && n < 19959)))
	; 6641..6653 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 19922 && n < 19960)))
	; 6642..6654 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19924)
	; 6642 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19926)
	; 6642 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19927)
	; 6643 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19930)
	; 6644 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19932)
	; 6644 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19933)
	; 6645 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19936)
	; 6646 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19938)
	; 6646 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19939)
	; 6647 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19942)
	; 6648 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19944)
	; 6648 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19945)
	; 6649 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19948)
	; 6650 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19950)
	; 6650 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19951)
	; 6651 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19954)
	; 6652 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19956)
	; 6652 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19957)
	; 6653 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19960)
	; 6654 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 19961 to 20000
.if((n .mod 3 = 2) && ((n > 19960 && n < 20001)))
	; 6655..6667 bytes
	delay_n (n-2)
	nop
.elseif(n = 19962)
	; 6654 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19963)
	; 6655 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 6 = 3) && ((n > 19964 && n < 19996)))
	; 6656..6666 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 19966)
	; 6656 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19968)
	; 6656 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19969)
	; 6657 bytes
	delay_n 43
	delay_n (n-43)
.elseif(n = 19972)
	; 6658 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19974)
	; 6658 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19975)
	; 6659 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19978)
	; 6660 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19980)
	; 6660 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19981)
	; 6661 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19984)
	; 6662 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19986)
	; 6662 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19987)
	; 6663 bytes
	inc $0700,X
	delay_n (n-7)
.elseif(n = 19990)
	; 6664 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19992)
	; 6664 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19993)
	; 6665 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19996)
	; 6666 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19998)
	; 6666 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19999)
	; 6667 bytes
	delay_n 19
	delay_n (n-19)
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
