; delay_n, delays n cycles with small code
; Will clobber Z+N
; Will preserve A, X, Y, S, C, D, V, and I (interrupt-disable flag)
;
; This version of delay_n does not change any bytes in the $100 page.
;
; Generated code is:
; -209 bytes or less when n <= 618
; -417 bytes or less when n <= 1242
; -626 bytes or less when n <= 1870
; -834 bytes or less when n <= 2494
; -1043 bytes or less when n <= 3120
; -1251 bytes or less when n <= 3744
; -1460 bytes or less when n <= 4372
; -1668 bytes or less when n <= 4996
; -1876 bytes or less when n <= 5620
; -2085 bytes or less when n <= 6246
; -2293 bytes or less when n <= 6870
; -2502 bytes or less when n <= 7498
; -2710 bytes or less when n <= 8122
; -2919 bytes or less when n <= 8748
; -3127 bytes or less when n <= 9372
; -3335 bytes or less when n <= 9996
; -3544 bytes or less when n <= 10624
; -3752 bytes or less when n <= 11248
; -3961 bytes or less when n <= 11874
; -4169 bytes or less when n <= 12498
; -4378 bytes or less when n <= 13126
; -4586 bytes or less when n <= 13750
; -4795 bytes or less when n <= 14376
; -5003 bytes or less when n <= 15000
; -5211 bytes or less when n <= 15624
; -5420 bytes or less when n <= 16252
; -5628 bytes or less when n <= 16876
; -5837 bytes or less when n <= 17502
; -6045 bytes or less when n <= 18126
; -6254 bytes or less when n <= 18754
; -6462 bytes or less when n <= 19378
; -6670 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=0, avg=1.6, max=4
; - When 11 <= n <= 20: min=0, avg=3.0, max=8
; - When 21 <= n <= 30: min=0, avg=4.6, max=11
; - When 31 <= n <= 50: min=0, avg=10.0, max=20
; - When 51 <= n <= 75: min=18, avg=23.1, max=29
; - When 76 <= n <= 100: min=26, avg=31.2, max=36
; - When 101 <= n <= 200: min=34, avg=52.0, max=70
; - When 201 <= n <= 300: min=68, avg=85.3, max=103
; - When 301 <= n <= 500: min=101, avg=135.3, max=170
; - When 501 <= n <= 750: min=168, avg=210.3, max=253
; - When 751 <= n <= 1000: min=251, avg=293.7, max=336
; - When 1001 <= n <= 2000: min=334, avg=502.0, max=670
; - When 2001 <= n <= 3000: min=668, avg=835.3, max=1003
; - When 3001 <= n <= 5000: min=1001, avg=1335.3, max=1670
; - When 5001 <= n <= 7500: min=1668, avg=2085.3, max=2503
; - When 7501 <= n <= 10000: min=2501, avg=2918.7, max=3336
; - When 10001 <= n <= 20000: min=3334, avg=5002.0, max=6670
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
.elseif((n .mod 2 = 1) && ((n > 2 && n < 44)))
.error .sprintf("delay_n: INTERNAL ERROR: UNABLE TO DEVISE DELAY CODE FOR %d CYCLES", n)
.elseif(n = 4 || n = 6 || n = 14 || n = 16 || n = 18 || n = 26 || n = 28 || n = 30 || n = 38 || n = 40 || n = 42 || n = 47 || (n > 48 && n < 53) || n = 56 || n = 59 || (n > 60 && n < 64) || n = 68 || n = 71 || (n > 72 && n < 76))
	; 2..29 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 20)
	; 8 bytes
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
.elseif(n = 22)
	; 8 bytes
	delay_n (n-12)
	delay_n 12
.elseif((n .mod 12 = 0) && ((n > 23 && n < 73)))
	; 8..24 bytes
	.repeat n/12
	 rol $36,X
	 ror $36,X
	.endrepeat
.elseif(n = 32)
	; 12 bytes
	delay_n (n-20)
	delay_n 20
.elseif(n = 34 || n = 46 || n = 55 || n = 58 || n = 64 || n = 70)
	; 12..24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 44 || n = 65 || n = 77)
	; 16..30 bytes
	delay_n 20
	delay_n (n-20)
.elseif(n = 45)
	; 18 bytes
	.repeat 9
	 rol $26
	.endrepeat
.elseif(n = 53)
	; 22 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 54)
	; 18 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 57 || n = 66)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 67 || n = 76)
	; 26 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 69 || n = 78)
	; 26 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 79 to 156
.if(n = 79 || n = 88 || n = 142)
	; 30..48 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 80 || n = 83 || (n > 84 && n < 88) || n = 92 || n = 95 || n = 97 || n = 98 || n = 101 || n = 104 || n = 107 || n = 110 || n = 113 || n = 116 || n = 119 || n = 122 || n = 125 || n = 128 || n = 131 || n = 134 || n = 137 || n = 140 || n = 143 || n = 146 || n = 149 || n = 152 || n = 155)
	; 27..55 bytes
	delay_n (n-2)
	nop
.elseif(n = 81 || n = 90 || n = 144)
	; 30..48 bytes
	delay_n 36
	delay_n (n-36)
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
.elseif(n = 89)
	; 34 bytes
	delay_n 44
	delay_n (n-44)
.elseif(n = 91 || n = 93 || n = 103 || n = 105 || n = 115 || n = 117 || n = 127 || n = 129 || n = 139 || n = 141 || n = 151)
	; 34..54 bytes
	delay_n (n-45)
	delay_n 45
.elseif(n = 99 || n = 153)
	; 36..54 bytes
	delay_n 45
	delay_n (n-45)
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
.elseif(n = 109 || n = 111 || n = 112 || n = 114 || n = 121 || n = 123 || n = 124 || n = 126 || n = 133 || n = 135 || n = 136 || n = 138 || n = 145 || n = 147 || n = 148 || n = 150)
	; 38..52 bytes
	delay_n (n-54)
	delay_n 54
.elseif(n = 120)
	; 40 bytes
	delay_n 12
	delay_n (n-12)
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
.if(n = 157 || n = 159 || n = 160 || n = 169 || n = 171 || n = 181 || n = 183 || n = 193 || n = 195)
	; 54..68 bytes
	delay_n (n-54)
	delay_n 54
.elseif((n .mod 3 = 2) && ((n > 157 && n < 195)))
	; 53..67 bytes
	delay_n (n-2)
	nop
.elseif(n = 162)
	; 54 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 163)
	; 58 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 165)
	; 58 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 166)
	; 56 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 168)
	; 56 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 172)
	; 58 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 174)
	; 58 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 175)
	; 62 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 177)
	; 62 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 178)
	; 60 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 180)
	; 60 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 184)
	; 62 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 186)
	; 62 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 187)
	; 66 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 189)
	; 66 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 190)
	; 64 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 192)
	; 64 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
.if((n >= 196) && (n < 215))
; 196 to 214
.if(n = 196)
	; 66 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 196 && n < 213)))
	; 67..73 bytes
	delay_n (n-2)
	nop
.elseif(n = 198)
	; 66 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 199)
	; 70 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 201)
	; 70 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 202)
	; 68 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 204)
	; 68 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 205)
	; 72 bytes
	delay_n (n-54)
	delay_n 54
.elseif(n = 207)
	; 72 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 208)
	; 70 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 210)
	; 70 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 211)
	; 74 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 213)
	; 74 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 214)
	; 72 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 215 to 234
.if((n .mod 3 = 2) && ((n > 214 && n < 234)))
	; 73..81 bytes
	delay_n (n-2)
	nop
.elseif(n = 216)
	; 72 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 217)
	; 76 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 219)
	; 76 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 220)
	; 74 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 222)
	; 74 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 223 || n = 225)
	; 78 bytes
	delay_n (n-108)
	delay_n 108
.elseif(n = 226)
	; 76 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 228)
	; 76 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 229)
	; 80 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 231)
	; 80 bytes
	delay_n 69
	delay_n (n-69)
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
.endif
.else
.if((n >= 235) && (n < 274))
; 235 to 273
.if(n = 235 || n = 237 || n = 247 || n = 249 || n = 259)
	; 82..90 bytes
	delay_n (n-108)
	delay_n 108
.elseif((n .mod 3 = 2) && ((n > 235 && n < 273)))
	; 79..93 bytes
	delay_n (n-2)
	nop
.elseif(n = 238)
	; 80 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 240)
	; 80 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 241)
	; 84 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 243)
	; 84 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 244)
	; 82 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 246)
	; 82 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 250)
	; 84 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 252)
	; 84 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 253)
	; 88 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 255)
	; 88 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 256)
	; 86 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 258)
	; 86 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 261)
	; 90 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 262)
	; 88 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 264)
	; 88 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 265)
	; 92 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 267)
	; 92 bytes
	delay_n 105
	delay_n (n-105)
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
	; 94 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 273)
	; 94 bytes
	delay_n 57
	delay_n (n-57)
.endif
.else
.if((n >= 274) && (n < 293))
; 274 to 292
.if(n = 274)
	; 92 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 274 && n < 291)))
	; 93..99 bytes
	delay_n (n-2)
	nop
.elseif(n = 276)
	; 92 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 277)
	; 96 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 279)
	; 96 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 280)
	; 94 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 282)
	; 94 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 283)
	; 98 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 285)
	; 98 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 286)
	; 96 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 288)
	; 96 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 289)
	; 100 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 291)
	; 100 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 292)
	; 98 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 293 to 312
.if((n .mod 3 = 2) && ((n > 292 && n < 312)))
	; 99..107 bytes
	delay_n (n-2)
	nop
.elseif(n = 294)
	; 98 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 295)
	; 102 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 297)
	; 102 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 298)
	; 100 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 300)
	; 100 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 301)
	; 104 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 303)
	; 104 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 304)
	; 102 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 306)
	; 102 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 307)
	; 106 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 309)
	; 106 bytes
	delay_n 93
	delay_n (n-93)
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
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
.if((n >= 313) && (n < 332))
; 313 to 331
.if(n = 313)
	; 108 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 313 && n < 330)))
	; 105..113 bytes
	delay_n (n-2)
	nop
.elseif(n = 315)
	; 108 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 316)
	; 106 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 318)
	; 106 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 319)
	; 110 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 321)
	; 110 bytes
	delay_n 105
	delay_n (n-105)
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
	; 112 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 327)
	; 112 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 328)
	; 110 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 330)
	; 110 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 331)
	; 114 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 332 to 351
.if((n .mod 3 = 2) && ((n > 331 && n < 351)))
	; 111..119 bytes
	delay_n (n-2)
	nop
.elseif(n = 333)
	; 114 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 334)
	; 112 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 336)
	; 112 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 337)
	; 116 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 339)
	; 116 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 340)
	; 114 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 342)
	; 114 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 343)
	; 118 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 345)
	; 118 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 346)
	; 116 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 348)
	; 116 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 349)
	; 120 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 351)
	; 120 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.else
.if((n >= 352) && (n < 371))
; 352 to 370
.if(n = 352)
	; 118 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 352 && n < 369)))
	; 119..125 bytes
	delay_n (n-2)
	nop
.elseif(n = 354)
	; 118 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 355)
	; 122 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 357)
	; 122 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 358)
	; 120 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 360)
	; 120 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 361)
	; 124 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 363)
	; 124 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 364)
	; 122 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 366)
	; 122 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 367)
	; 126 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 369)
	; 126 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 370)
	; 124 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 371 to 390
.if((n .mod 3 = 2) && ((n > 370 && n < 390)))
	; 125..133 bytes
	delay_n (n-2)
	nop
.elseif(n = 372)
	; 124 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 373)
	; 128 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 375)
	; 128 bytes
	delay_n 105
	delay_n (n-105)
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
	; 130 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 381)
	; 130 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 382)
	; 128 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 384)
	; 128 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 385)
	; 132 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 387)
	; 132 bytes
	delay_n 117
	delay_n (n-117)
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
.endif
.else
.if((n >= 391) && (n < 430))
.if((n >= 391) && (n < 410))
; 391 to 409
.if(n = 391)
	; 134 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 391 && n < 408)))
	; 131..139 bytes
	delay_n (n-2)
	nop
.elseif(n = 393)
	; 134 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 394)
	; 132 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 396)
	; 132 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 397)
	; 136 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 399)
	; 136 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 400)
	; 134 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 402)
	; 134 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 403)
	; 138 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 405)
	; 138 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 406)
	; 136 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 408)
	; 136 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 409)
	; 140 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 410 to 429
.if((n .mod 3 = 2) && ((n > 409 && n < 429)))
	; 137..145 bytes
	delay_n (n-2)
	nop
.elseif(n = 411)
	; 140 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 412)
	; 138 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 414)
	; 138 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 415)
	; 142 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 417)
	; 142 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 418)
	; 140 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 420)
	; 140 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 421)
	; 144 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 423)
	; 144 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 424)
	; 142 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 426)
	; 142 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 427)
	; 146 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 429)
	; 146 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.else
.if((n >= 430) && (n < 449))
; 430 to 448
.if(n = 430)
	; 144 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 430 && n < 447)))
	; 145..151 bytes
	delay_n (n-2)
	nop
.elseif(n = 432)
	; 144 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 433)
	; 148 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 435)
	; 148 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 436)
	; 146 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 438)
	; 146 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 439)
	; 150 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 441)
	; 150 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 442)
	; 148 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 444)
	; 148 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 445)
	; 152 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 447)
	; 152 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 448)
	; 150 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 449 to 468
.if((n .mod 3 = 2) && ((n > 448 && n < 468)))
	; 151..159 bytes
	delay_n (n-2)
	nop
.elseif(n = 450)
	; 150 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 451)
	; 154 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 453)
	; 154 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 454)
	; 152 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 456)
	; 152 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 457)
	; 156 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 459)
	; 156 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 460)
	; 154 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 462)
	; 154 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 463)
	; 158 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 465)
	; 158 bytes
	delay_n 141
	delay_n (n-141)
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
.endif
.else
.if((n >= 469) && (n < 547))
.if((n >= 469) && (n < 508))
.if((n >= 469) && (n < 488))
; 469 to 487
.if(n = 469)
	; 160 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 469 && n < 486)))
	; 157..165 bytes
	delay_n (n-2)
	nop
.elseif(n = 471)
	; 160 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 472)
	; 158 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 474)
	; 158 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 475)
	; 162 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 477)
	; 162 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 478)
	; 160 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 480)
	; 160 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 481)
	; 164 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 483)
	; 164 bytes
	delay_n 105
	delay_n (n-105)
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
	; 166 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 488 to 507
.if((n .mod 3 = 2) && ((n > 487 && n < 507)))
	; 163..171 bytes
	delay_n (n-2)
	nop
.elseif(n = 489)
	; 166 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 490)
	; 164 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 492)
	; 164 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 493)
	; 168 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 495)
	; 168 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 496)
	; 166 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 498)
	; 166 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 499)
	; 170 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 501)
	; 170 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 502)
	; 168 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 504)
	; 168 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 505)
	; 172 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 507)
	; 172 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.else
.if((n >= 508) && (n < 527))
; 508 to 526
.if(n = 508)
	; 170 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 508 && n < 525)))
	; 171..177 bytes
	delay_n (n-2)
	nop
.elseif(n = 510)
	; 170 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 511)
	; 174 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 513)
	; 174 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 514)
	; 172 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 516)
	; 172 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 517)
	; 176 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 519)
	; 176 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 520)
	; 174 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 522)
	; 174 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 523)
	; 178 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 525)
	; 178 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 526)
	; 176 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 527 to 546
.if((n .mod 3 = 2) && ((n > 526 && n < 546)))
	; 177..185 bytes
	delay_n (n-2)
	nop
.elseif(n = 528)
	; 176 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 529)
	; 180 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 531)
	; 180 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 532)
	; 178 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 534)
	; 178 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 535)
	; 182 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 537)
	; 182 bytes
	delay_n 105
	delay_n (n-105)
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
	; 184 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 543)
	; 184 bytes
	delay_n 57
	delay_n (n-57)
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
.endif
.else
.if((n >= 547) && (n < 586))
.if((n >= 547) && (n < 566))
; 547 to 565
.if(n = 547)
	; 186 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 547 && n < 564)))
	; 183..191 bytes
	delay_n (n-2)
	nop
.elseif(n = 549)
	; 186 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 550)
	; 184 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 552)
	; 184 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 553)
	; 188 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 555)
	; 188 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 556)
	; 186 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 558)
	; 186 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 559)
	; 190 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 561)
	; 190 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 562)
	; 188 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 564)
	; 188 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 565)
	; 192 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 566 to 585
.if((n .mod 3 = 2) && ((n > 565 && n < 585)))
	; 189..197 bytes
	delay_n (n-2)
	nop
.elseif(n = 567)
	; 192 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 568)
	; 190 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 570)
	; 190 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 571)
	; 194 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 573)
	; 194 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 574)
	; 192 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 576)
	; 192 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 577)
	; 196 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 579)
	; 196 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 580)
	; 194 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 582)
	; 194 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 583)
	; 198 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 585)
	; 198 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.else
.if((n >= 586) && (n < 606))
; 586 to 605
.if(n = 586)
	; 196 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 586 && n < 606)))
	; 197..205 bytes
	delay_n (n-2)
	nop
.elseif(n = 588)
	; 196 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 589)
	; 200 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 591)
	; 200 bytes
	delay_n 105
	delay_n (n-105)
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
	; 202 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 597)
	; 202 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 598)
	; 200 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 600)
	; 200 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 601)
	; 204 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 603)
	; 204 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 604)
	; 202 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 606 to 625
.if(n = 606)
	; 202 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 607)
	; 206 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 607 && n < 624)))
	; 203..211 bytes
	delay_n (n-2)
	nop
.elseif(n = 609)
	; 206 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 610)
	; 204 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 612)
	; 204 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 613)
	; 208 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 615)
	; 208 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 616)
	; 206 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 618)
	; 206 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 619)
	; 210 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 621)
	; 210 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 622)
	; 208 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 624)
	; 208 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 625)
	; 212 bytes
	delay_n 139
	delay_n (n-139)
.endif
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
.if((n >= 626) && (n < 645))
; 626 to 644
.if((n .mod 3 = 2) && ((n > 625 && n < 645)))
	; 209..217 bytes
	delay_n (n-2)
	nop
.elseif(n = 627)
	; 212 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 628)
	; 210 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 630)
	; 210 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 631)
	; 214 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 633)
	; 214 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 634)
	; 212 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 636)
	; 212 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 637)
	; 216 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 639)
	; 216 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 640)
	; 214 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 642)
	; 214 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 643)
	; 218 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 645 to 664
.if(n = 645)
	; 218 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 646)
	; 216 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 646 && n < 663)))
	; 217..223 bytes
	delay_n (n-2)
	nop
.elseif(n = 648)
	; 216 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 649)
	; 220 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 651)
	; 220 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 652)
	; 218 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 654)
	; 218 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 655)
	; 222 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 657)
	; 222 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 658)
	; 220 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 660)
	; 220 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 661)
	; 224 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 663)
	; 224 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 664)
	; 222 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 665) && (n < 684))
; 665 to 683
.if((n .mod 3 = 2) && ((n > 664 && n < 684)))
	; 223..231 bytes
	delay_n (n-2)
	nop
.elseif(n = 666)
	; 222 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 667)
	; 226 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 669)
	; 226 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 670)
	; 224 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 672)
	; 224 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 673)
	; 228 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 675)
	; 228 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 676)
	; 226 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 678)
	; 226 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 679)
	; 230 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 681)
	; 230 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 682)
	; 228 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 684 to 703
.if(n = 684)
	; 228 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 685)
	; 232 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 685 && n < 702)))
	; 229..237 bytes
	delay_n (n-2)
	nop
.elseif(n = 687)
	; 232 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 688)
	; 230 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 690)
	; 230 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 691)
	; 234 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 693)
	; 234 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 694)
	; 232 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 696)
	; 232 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 697)
	; 236 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 699)
	; 236 bytes
	delay_n 105
	delay_n (n-105)
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
	; 238 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.else
.if((n >= 704) && (n < 743))
.if((n >= 704) && (n < 723))
; 704 to 722
.if((n .mod 3 = 2) && ((n > 703 && n < 723)))
	; 235..243 bytes
	delay_n (n-2)
	nop
.elseif(n = 705)
	; 238 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 706)
	; 236 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 708)
	; 236 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 709)
	; 240 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 711)
	; 240 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 712)
	; 238 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 714)
	; 238 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 715)
	; 242 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 717)
	; 242 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 718)
	; 240 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 720)
	; 240 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 721)
	; 244 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 723 to 742
.if(n = 723)
	; 244 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 724)
	; 242 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 724 && n < 741)))
	; 243..249 bytes
	delay_n (n-2)
	nop
.elseif(n = 726)
	; 242 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 727)
	; 246 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 729)
	; 246 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 730)
	; 244 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 732)
	; 244 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 733)
	; 248 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 735)
	; 248 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 736)
	; 246 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 738)
	; 246 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 739)
	; 250 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 741)
	; 250 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 742)
	; 248 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 743) && (n < 762))
; 743 to 761
.if((n .mod 3 = 2) && ((n > 742 && n < 762)))
	; 249..257 bytes
	delay_n (n-2)
	nop
.elseif(n = 744)
	; 248 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 745)
	; 252 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 747)
	; 252 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 748)
	; 250 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 750)
	; 250 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 751)
	; 254 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 753)
	; 254 bytes
	delay_n 105
	delay_n (n-105)
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
	; 256 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 759)
	; 256 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 760)
	; 254 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 762 to 781
.if(n = 762)
	; 254 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 763)
	; 258 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 763 && n < 780)))
	; 255..263 bytes
	delay_n (n-2)
	nop
.elseif(n = 765)
	; 258 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 766)
	; 256 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 768)
	; 256 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 769)
	; 260 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 771)
	; 260 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 772)
	; 258 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 774)
	; 258 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 775)
	; 262 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 777)
	; 262 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 778)
	; 260 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 780)
	; 260 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 781)
	; 264 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.endif
.else
.if((n >= 782) && (n < 860))
.if((n >= 782) && (n < 821))
.if((n >= 782) && (n < 801))
; 782 to 800
.if((n .mod 3 = 2) && ((n > 781 && n < 801)))
	; 261..269 bytes
	delay_n (n-2)
	nop
.elseif(n = 783)
	; 264 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 784)
	; 262 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 786)
	; 262 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 787)
	; 266 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 789)
	; 266 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 790)
	; 264 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 792)
	; 264 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 793)
	; 268 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 795)
	; 268 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 796)
	; 266 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 798)
	; 266 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 799)
	; 270 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 801 to 820
.if(n = 801)
	; 270 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 802)
	; 268 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 802 && n < 819)))
	; 269..275 bytes
	delay_n (n-2)
	nop
.elseif(n = 804)
	; 268 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 805)
	; 272 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 807)
	; 272 bytes
	delay_n 105
	delay_n (n-105)
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
	; 274 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 813)
	; 274 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 814)
	; 272 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 816)
	; 272 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 817)
	; 276 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 819)
	; 276 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 820)
	; 274 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 821) && (n < 840))
; 821 to 839
.if((n .mod 3 = 2) && ((n > 820 && n < 840)))
	; 275..283 bytes
	delay_n (n-2)
	nop
.elseif(n = 822)
	; 274 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 823)
	; 278 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 825)
	; 278 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 826)
	; 276 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 828)
	; 276 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 829)
	; 280 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 831)
	; 280 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 832)
	; 278 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 834)
	; 278 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 835)
	; 282 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 837)
	; 282 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 838)
	; 280 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 840 to 859
.if(n = 840)
	; 280 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 841)
	; 284 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 841 && n < 858)))
	; 281..289 bytes
	delay_n (n-2)
	nop
.elseif(n = 843)
	; 284 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 844)
	; 282 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 846)
	; 282 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 847)
	; 286 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 849)
	; 286 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 850)
	; 284 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 852)
	; 284 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 853)
	; 288 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 855)
	; 288 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 856)
	; 286 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 858)
	; 286 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 859)
	; 290 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.else
.if((n >= 860) && (n < 899))
.if((n >= 860) && (n < 879))
; 860 to 878
.if((n .mod 3 = 2) && ((n > 859 && n < 879)))
	; 287..295 bytes
	delay_n (n-2)
	nop
.elseif(n = 861)
	; 290 bytes
	delay_n 105
	delay_n (n-105)
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
	; 292 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 867)
	; 292 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 868)
	; 290 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 870)
	; 290 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 871)
	; 294 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 873)
	; 294 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 874)
	; 292 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 876)
	; 292 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 877)
	; 296 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 879 to 898
.if(n = 879)
	; 296 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 880)
	; 294 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 880 && n < 897)))
	; 295..301 bytes
	delay_n (n-2)
	nop
.elseif(n = 882)
	; 294 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 883)
	; 298 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 885)
	; 298 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 886)
	; 296 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 888)
	; 296 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 889)
	; 300 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 891)
	; 300 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 892)
	; 298 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 894)
	; 298 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 895)
	; 302 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 897)
	; 302 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 898)
	; 300 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 899) && (n < 918))
; 899 to 917
.if((n .mod 3 = 2) && ((n > 898 && n < 918)))
	; 301..309 bytes
	delay_n (n-2)
	nop
.elseif(n = 900)
	; 300 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 901)
	; 304 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 903)
	; 304 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 904)
	; 302 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 906)
	; 302 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 907)
	; 306 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 909)
	; 306 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 910)
	; 304 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 912)
	; 304 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 913)
	; 308 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 915)
	; 308 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 916)
	; 306 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 918 to 937
.if(n = 918)
	; 306 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 919)
	; 310 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 919 && n < 936)))
	; 307..315 bytes
	delay_n (n-2)
	nop
.elseif(n = 921)
	; 310 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 922)
	; 308 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 924)
	; 308 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 925)
	; 312 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 927)
	; 312 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 928)
	; 310 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 930)
	; 310 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 931)
	; 314 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 933)
	; 314 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 934)
	; 312 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 936)
	; 312 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 937)
	; 316 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
.if((n >= 938) && (n < 977))
.if((n >= 938) && (n < 957))
; 938 to 956
.if((n .mod 3 = 2) && ((n > 937 && n < 957)))
	; 313..321 bytes
	delay_n (n-2)
	nop
.elseif(n = 939)
	; 316 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 940)
	; 314 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 942)
	; 314 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 943)
	; 318 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 945)
	; 318 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 946)
	; 316 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 948)
	; 316 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 949)
	; 320 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 951)
	; 320 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 952)
	; 318 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 954)
	; 318 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 955)
	; 322 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 957 to 976
.if(n = 957)
	; 322 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 958)
	; 320 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 958 && n < 975)))
	; 321..327 bytes
	delay_n (n-2)
	nop
.elseif(n = 960)
	; 320 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 961)
	; 324 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 963)
	; 324 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 964)
	; 322 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 966)
	; 322 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 967)
	; 326 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 969)
	; 326 bytes
	delay_n 105
	delay_n (n-105)
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
	; 328 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 975)
	; 328 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 976)
	; 326 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 977) && (n < 996))
; 977 to 995
.if((n .mod 3 = 2) && ((n > 976 && n < 996)))
	; 327..335 bytes
	delay_n (n-2)
	nop
.elseif(n = 978)
	; 326 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 979)
	; 330 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 981)
	; 330 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 982)
	; 328 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 984)
	; 328 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 985)
	; 332 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 987)
	; 332 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 988)
	; 330 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 990)
	; 330 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 991)
	; 334 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 993)
	; 334 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 994)
	; 332 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 996 to 1015
.if(n = 996)
	; 332 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 997)
	; 336 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 997 && n < 1014)))
	; 333..341 bytes
	delay_n (n-2)
	nop
.elseif(n = 999)
	; 336 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1000)
	; 334 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1002)
	; 334 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1003)
	; 338 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1005)
	; 338 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1006)
	; 336 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1008)
	; 336 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1009)
	; 340 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1011)
	; 340 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1012)
	; 338 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1014)
	; 338 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1015)
	; 342 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.else
.if((n >= 1016) && (n < 1055))
.if((n >= 1016) && (n < 1035))
; 1016 to 1034
.if((n .mod 3 = 2) && ((n > 1015 && n < 1035)))
	; 339..347 bytes
	delay_n (n-2)
	nop
.elseif(n = 1017)
	; 342 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1018)
	; 340 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1020)
	; 340 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1021)
	; 344 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1023)
	; 344 bytes
	delay_n 105
	delay_n (n-105)
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
	; 346 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1029)
	; 346 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1030)
	; 344 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1032)
	; 344 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1033)
	; 348 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 1035 to 1054
.if(n = 1035)
	; 348 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1036)
	; 346 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 1036 && n < 1053)))
	; 347..353 bytes
	delay_n (n-2)
	nop
.elseif(n = 1038)
	; 346 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1039)
	; 350 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1041)
	; 350 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1042)
	; 348 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1044)
	; 348 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1045)
	; 352 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1047)
	; 352 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1048)
	; 350 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1050)
	; 350 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1051)
	; 354 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1053)
	; 354 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1054)
	; 352 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 1055) && (n < 1074))
; 1055 to 1073
.if((n .mod 3 = 2) && ((n > 1054 && n < 1074)))
	; 353..361 bytes
	delay_n (n-2)
	nop
.elseif(n = 1056)
	; 352 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1057)
	; 356 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1059)
	; 356 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1060)
	; 354 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1062)
	; 354 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1063)
	; 358 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1065)
	; 358 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1066)
	; 356 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1068)
	; 356 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1069)
	; 360 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1071)
	; 360 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1072)
	; 358 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 1074 to 1093
.if(n = 1074)
	; 358 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1075)
	; 362 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 1075 && n < 1092)))
	; 359..367 bytes
	delay_n (n-2)
	nop
.elseif(n = 1077)
	; 362 bytes
	delay_n 105
	delay_n (n-105)
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
	; 364 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1083)
	; 364 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1084)
	; 362 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1086)
	; 362 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1087)
	; 366 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1089)
	; 366 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1090)
	; 364 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1092)
	; 364 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1093)
	; 368 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.endif
.else
.if((n >= 1094) && (n < 1172))
.if((n >= 1094) && (n < 1133))
.if((n >= 1094) && (n < 1113))
; 1094 to 1112
.if((n .mod 3 = 2) && ((n > 1093 && n < 1113)))
	; 365..373 bytes
	delay_n (n-2)
	nop
.elseif(n = 1095)
	; 368 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1096)
	; 366 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1098)
	; 366 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1099)
	; 370 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1101)
	; 370 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1102)
	; 368 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1104)
	; 368 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1105)
	; 372 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1107)
	; 372 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1108)
	; 370 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1110)
	; 370 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1111)
	; 374 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 1113 to 1132
.if(n = 1113)
	; 374 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1114)
	; 372 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 1114 && n < 1131)))
	; 373..379 bytes
	delay_n (n-2)
	nop
.elseif(n = 1116)
	; 372 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1117)
	; 376 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1119)
	; 376 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1120)
	; 374 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1122)
	; 374 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1123)
	; 378 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1125)
	; 378 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1126)
	; 376 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1128)
	; 376 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1129)
	; 380 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1131)
	; 380 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 1132)
	; 378 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 1133) && (n < 1152))
; 1133 to 1151
.if((n .mod 3 = 2) && ((n > 1132 && n < 1152)))
	; 379..387 bytes
	delay_n (n-2)
	nop
.elseif(n = 1134)
	; 378 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1135)
	; 382 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1137)
	; 382 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1138)
	; 380 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1140)
	; 380 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1141)
	; 384 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1143)
	; 384 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1144)
	; 382 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1146)
	; 382 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1147)
	; 386 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1149)
	; 386 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1150)
	; 384 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 1152 to 1171
.if(n = 1152)
	; 384 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1153)
	; 388 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 1153 && n < 1170)))
	; 385..393 bytes
	delay_n (n-2)
	nop
.elseif(n = 1155)
	; 388 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1156)
	; 386 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1158)
	; 386 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1159)
	; 390 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1161)
	; 390 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1162)
	; 388 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1164)
	; 388 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1165)
	; 392 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1167)
	; 392 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1168)
	; 390 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1170)
	; 390 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1171)
	; 394 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 1172) && (n < 1211))
.if((n >= 1172) && (n < 1191))
; 1172 to 1190
.if((n .mod 3 = 2) && ((n > 1171 && n < 1191)))
	; 391..399 bytes
	delay_n (n-2)
	nop
.elseif(n = 1173)
	; 394 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1174)
	; 392 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1176)
	; 392 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1177)
	; 396 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1179)
	; 396 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1180)
	; 394 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1182)
	; 394 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1183)
	; 398 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1185)
	; 398 bytes
	delay_n 105
	delay_n (n-105)
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
	; 400 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 1191 to 1210
.if(n = 1191)
	; 400 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1192)
	; 398 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 1192 && n < 1209)))
	; 399..405 bytes
	delay_n (n-2)
	nop
.elseif(n = 1194)
	; 398 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1195)
	; 402 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1197)
	; 402 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1198)
	; 400 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1200)
	; 400 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1201)
	; 404 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1203)
	; 404 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1204)
	; 402 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1206)
	; 402 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1207)
	; 406 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1209)
	; 406 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1210)
	; 404 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 1211) && (n < 1231))
; 1211 to 1230
.if((n .mod 3 = 2) && ((n > 1210 && n < 1230)))
	; 405..413 bytes
	delay_n (n-2)
	nop
.elseif(n = 1212)
	; 404 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1213)
	; 408 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1215)
	; 408 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1216)
	; 406 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1218)
	; 406 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1219)
	; 410 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1221)
	; 410 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1222)
	; 408 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1224)
	; 408 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1225)
	; 412 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1227)
	; 412 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1228)
	; 410 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1230)
	; 410 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 1231 to 1250
.if(n = 1231)
	; 414 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 1231 && n < 1251)))
	; 411..419 bytes
	delay_n (n-2)
	nop
.elseif(n = 1233)
	; 414 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1234)
	; 412 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1236)
	; 412 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1237)
	; 416 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1239)
	; 416 bytes
	delay_n 105
	delay_n (n-105)
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
	; 418 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1245)
	; 418 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1246)
	; 416 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1248)
	; 416 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1249)
	; 420 bytes
	delay_n 115
	delay_n (n-115)
.endif
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
.if((n >= 1251) && (n < 1270))
; 1251 to 1269
.if(n = 1251)
	; 420 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1252)
	; 418 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 1252 && n < 1269)))
	; 419..425 bytes
	delay_n (n-2)
	nop
.elseif(n = 1254)
	; 418 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1255)
	; 422 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1257)
	; 422 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1258)
	; 420 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1260)
	; 420 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1261)
	; 424 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1263)
	; 424 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1264)
	; 422 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1266)
	; 422 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1267)
	; 426 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1269)
	; 426 bytes
	delay_n 81
	delay_n (n-81)
.endif
.else
; 1270 to 1289
.if(n = 1270)
	; 424 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 1270 && n < 1290)))
	; 425..433 bytes
	delay_n (n-2)
	nop
.elseif(n = 1272)
	; 424 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1273)
	; 428 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1275)
	; 428 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1276)
	; 426 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1278)
	; 426 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1279)
	; 430 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1281)
	; 430 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1282)
	; 428 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1284)
	; 428 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1285)
	; 432 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1287)
	; 432 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1288)
	; 430 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 1290) && (n < 1309))
; 1290 to 1308
.if(n = 1290)
	; 430 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1291)
	; 434 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 1291 && n < 1308)))
	; 431..439 bytes
	delay_n (n-2)
	nop
.elseif(n = 1293)
	; 434 bytes
	delay_n 105
	delay_n (n-105)
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
	; 436 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1299)
	; 436 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1300)
	; 434 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1302)
	; 434 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1303)
	; 438 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1305)
	; 438 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1306)
	; 436 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1308)
	; 436 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 1309 to 1328
.if(n = 1309)
	; 440 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 1309 && n < 1329)))
	; 437..445 bytes
	delay_n (n-2)
	nop
.elseif(n = 1311)
	; 440 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1312)
	; 438 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1314)
	; 438 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1315)
	; 442 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1317)
	; 442 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1318)
	; 440 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1320)
	; 440 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1321)
	; 444 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1323)
	; 444 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1324)
	; 442 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1326)
	; 442 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1327)
	; 446 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.endif
.else
.if((n >= 1329) && (n < 1368))
.if((n >= 1329) && (n < 1348))
; 1329 to 1347
.if(n = 1329)
	; 446 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1330)
	; 444 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 1330 && n < 1347)))
	; 445..451 bytes
	delay_n (n-2)
	nop
.elseif(n = 1332)
	; 444 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1333)
	; 448 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1335)
	; 448 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1336)
	; 446 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1338)
	; 446 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1339)
	; 450 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1341)
	; 450 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1342)
	; 448 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1344)
	; 448 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1345)
	; 452 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1347)
	; 452 bytes
	delay_n 105
	delay_n (n-105)
.endif
.else
; 1348 to 1367
.if(n = 1348)
	; 450 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 1348 && n < 1368)))
	; 451..459 bytes
	delay_n (n-2)
	nop
.elseif(n = 1350)
	; 450 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1351)
	; 454 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1353)
	; 454 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1354)
	; 452 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1356)
	; 452 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1357)
	; 456 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1359)
	; 456 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1360)
	; 454 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1362)
	; 454 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1363)
	; 458 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1365)
	; 458 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1366)
	; 456 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 1368) && (n < 1387))
; 1368 to 1386
.if(n = 1368)
	; 456 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1369)
	; 460 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 1369 && n < 1386)))
	; 457..465 bytes
	delay_n (n-2)
	nop
.elseif(n = 1371)
	; 460 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1372)
	; 458 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1374)
	; 458 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1375)
	; 462 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1377)
	; 462 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1378)
	; 460 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1380)
	; 460 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1381)
	; 464 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1383)
	; 464 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1384)
	; 462 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1386)
	; 462 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 1387 to 1406
.if(n = 1387)
	; 466 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 1387 && n < 1407)))
	; 463..471 bytes
	delay_n (n-2)
	nop
.elseif(n = 1389)
	; 466 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1390)
	; 464 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1392)
	; 464 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1393)
	; 468 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1395)
	; 468 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1396)
	; 466 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1398)
	; 466 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1399)
	; 470 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1401)
	; 470 bytes
	delay_n 105
	delay_n (n-105)
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
	; 472 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
.if((n >= 1407) && (n < 1426))
; 1407 to 1425
.if(n = 1407)
	; 472 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1408)
	; 470 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 1408 && n < 1425)))
	; 471..477 bytes
	delay_n (n-2)
	nop
.elseif(n = 1410)
	; 470 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1411)
	; 474 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1413)
	; 474 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1414)
	; 472 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1416)
	; 472 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1417)
	; 476 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1419)
	; 476 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1420)
	; 474 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1422)
	; 474 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1423)
	; 478 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1425)
	; 478 bytes
	delay_n 129
	delay_n (n-129)
.endif
.else
; 1426 to 1445
.if(n = 1426)
	; 476 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 1426 && n < 1446)))
	; 477..485 bytes
	delay_n (n-2)
	nop
.elseif(n = 1428)
	; 476 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1429)
	; 480 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1431)
	; 480 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1432)
	; 478 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1434)
	; 478 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1435)
	; 482 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1437)
	; 482 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1438)
	; 480 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1440)
	; 480 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1441)
	; 484 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1443)
	; 484 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1444)
	; 482 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 1446) && (n < 1465))
; 1446 to 1464
.if(n = 1446)
	; 482 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1447)
	; 486 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 1447 && n < 1464)))
	; 483..491 bytes
	delay_n (n-2)
	nop
.elseif(n = 1449)
	; 486 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1450)
	; 484 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1452)
	; 484 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1453)
	; 488 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1455)
	; 488 bytes
	delay_n 105
	delay_n (n-105)
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
	; 490 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1461)
	; 490 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1462)
	; 488 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1464)
	; 488 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 1465 to 1484
.if(n = 1465)
	; 492 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 1465 && n < 1485)))
	; 489..497 bytes
	delay_n (n-2)
	nop
.elseif(n = 1467)
	; 492 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1468)
	; 490 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1470)
	; 490 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1471)
	; 494 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1473)
	; 494 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1474)
	; 492 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1476)
	; 492 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1477)
	; 496 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1479)
	; 496 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1480)
	; 494 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1482)
	; 494 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1483)
	; 498 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
.if((n >= 1485) && (n < 1504))
; 1485 to 1503
.if(n = 1485)
	; 498 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1486)
	; 496 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 1486 && n < 1503)))
	; 497..503 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488)
	; 496 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1489)
	; 500 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1491)
	; 500 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1492)
	; 498 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1494)
	; 498 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1495)
	; 502 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1497)
	; 502 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1498)
	; 500 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1500)
	; 500 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1501)
	; 504 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1503)
	; 504 bytes
	delay_n 45
	delay_n (n-45)
.endif
.else
; 1504 to 1523
.if(n = 1504)
	; 502 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 1504 && n < 1524)))
	; 503..511 bytes
	delay_n (n-2)
	nop
.elseif(n = 1506)
	; 502 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1507)
	; 506 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1509)
	; 506 bytes
	delay_n 105
	delay_n (n-105)
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
	; 508 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1515)
	; 508 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1516)
	; 506 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1518)
	; 506 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1519)
	; 510 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1521)
	; 510 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1522)
	; 508 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 1524) && (n < 1543))
; 1524 to 1542
.if(n = 1524)
	; 508 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1525)
	; 512 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 1525 && n < 1542)))
	; 509..517 bytes
	delay_n (n-2)
	nop
.elseif(n = 1527)
	; 512 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1528)
	; 510 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1530)
	; 510 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1531)
	; 514 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1533)
	; 514 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1534)
	; 512 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1536)
	; 512 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1537)
	; 516 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1539)
	; 516 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1540)
	; 514 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1542)
	; 514 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 1543 to 1562
.if(n = 1543)
	; 518 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 1543 && n < 1563)))
	; 515..523 bytes
	delay_n (n-2)
	nop
.elseif(n = 1545)
	; 518 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1546)
	; 516 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1548)
	; 516 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1549)
	; 520 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1551)
	; 520 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1552)
	; 518 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1554)
	; 518 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1555)
	; 522 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1557)
	; 522 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1558)
	; 520 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1560)
	; 520 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1561)
	; 524 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
.if((n >= 1563) && (n < 1582))
; 1563 to 1581
.if(n = 1563)
	; 524 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 1564)
	; 522 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 1564 && n < 1581)))
	; 523..529 bytes
	delay_n (n-2)
	nop
.elseif(n = 1566)
	; 522 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1567)
	; 526 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1569)
	; 526 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1570)
	; 524 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1572)
	; 524 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1573)
	; 528 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1575)
	; 528 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1576)
	; 526 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1578)
	; 526 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1579)
	; 530 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1581)
	; 530 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 1582 to 1601
.if(n = 1582)
	; 528 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 1582 && n < 1602)))
	; 529..537 bytes
	delay_n (n-2)
	nop
.elseif(n = 1584)
	; 528 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1585)
	; 532 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1587)
	; 532 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1588)
	; 530 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1590)
	; 530 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1591)
	; 534 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1593)
	; 534 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1594)
	; 532 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1596)
	; 532 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1597)
	; 536 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1599)
	; 536 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1600)
	; 534 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 1602) && (n < 1621))
; 1602 to 1620
.if(n = 1602)
	; 534 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1603)
	; 538 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 1603 && n < 1620)))
	; 535..543 bytes
	delay_n (n-2)
	nop
.elseif(n = 1605)
	; 538 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1606)
	; 536 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1608)
	; 536 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1609)
	; 540 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1611)
	; 540 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1612)
	; 538 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1614)
	; 538 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1615)
	; 542 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1617)
	; 542 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 1618)
	; 540 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 1620)
	; 540 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 1621 to 1640
.if(n = 1621)
	; 544 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 1621 && n < 1641)))
	; 541..549 bytes
	delay_n (n-2)
	nop
.elseif(n = 1623)
	; 544 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1624)
	; 542 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1626)
	; 542 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1627)
	; 546 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1629)
	; 546 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1630)
	; 544 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1632)
	; 544 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1633)
	; 548 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1635)
	; 548 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1636)
	; 546 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1638)
	; 546 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1639)
	; 550 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
.if((n >= 1641) && (n < 1660))
; 1641 to 1659
.if(n = 1641)
	; 550 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1642)
	; 548 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 1642 && n < 1659)))
	; 549..555 bytes
	delay_n (n-2)
	nop
.elseif(n = 1644)
	; 548 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1645)
	; 552 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1647)
	; 552 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1648)
	; 550 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1650)
	; 550 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1651)
	; 554 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1653)
	; 554 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1654)
	; 552 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1656)
	; 552 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1657)
	; 556 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1659)
	; 556 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 1660 to 1679
.if(n = 1660)
	; 554 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 1660 && n < 1680)))
	; 555..563 bytes
	delay_n (n-2)
	nop
.elseif(n = 1662)
	; 554 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1663)
	; 558 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1665)
	; 558 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1666)
	; 556 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1668)
	; 556 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1669)
	; 560 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1671)
	; 560 bytes
	delay_n 105
	delay_n (n-105)
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
	; 562 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1677)
	; 562 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1678)
	; 560 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 1680) && (n < 1699))
; 1680 to 1698
.if(n = 1680)
	; 560 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1681)
	; 564 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 1681 && n < 1698)))
	; 561..569 bytes
	delay_n (n-2)
	nop
.elseif(n = 1683)
	; 564 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1684)
	; 562 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1686)
	; 562 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1687)
	; 566 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1689)
	; 566 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1690)
	; 564 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1692)
	; 564 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1693)
	; 568 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1695)
	; 568 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1696)
	; 566 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1698)
	; 566 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 1699 to 1718
.if(n = 1699)
	; 570 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 1699 && n < 1719)))
	; 567..575 bytes
	delay_n (n-2)
	nop
.elseif(n = 1701)
	; 570 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1702)
	; 568 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1704)
	; 568 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1705)
	; 572 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1707)
	; 572 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1708)
	; 570 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1710)
	; 570 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1711)
	; 574 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1713)
	; 574 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1714)
	; 572 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1716)
	; 572 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1717)
	; 576 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
.if((n >= 1719) && (n < 1738))
; 1719 to 1737
.if(n = 1719)
	; 576 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1720)
	; 574 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 1720 && n < 1737)))
	; 575..581 bytes
	delay_n (n-2)
	nop
.elseif(n = 1722)
	; 574 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1723)
	; 578 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1725)
	; 578 bytes
	delay_n 105
	delay_n (n-105)
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
	; 580 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1731)
	; 580 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1732)
	; 578 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1734)
	; 578 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1735)
	; 582 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1737)
	; 582 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 1738 to 1757
.if(n = 1738)
	; 580 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 1738 && n < 1758)))
	; 581..589 bytes
	delay_n (n-2)
	nop
.elseif(n = 1740)
	; 580 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1741)
	; 584 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1743)
	; 584 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1744)
	; 582 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1746)
	; 582 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1747)
	; 586 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1749)
	; 586 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1750)
	; 584 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1752)
	; 584 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1753)
	; 588 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1755)
	; 588 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1756)
	; 586 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 1758) && (n < 1777))
; 1758 to 1776
.if(n = 1758)
	; 586 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1759)
	; 590 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 1759 && n < 1776)))
	; 587..595 bytes
	delay_n (n-2)
	nop
.elseif(n = 1761)
	; 590 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1762)
	; 588 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1764)
	; 588 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1765)
	; 592 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1767)
	; 592 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1768)
	; 590 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1770)
	; 590 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1771)
	; 594 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1773)
	; 594 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1774)
	; 592 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1776)
	; 592 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 1777 to 1796
.if(n = 1777)
	; 596 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 1777 && n < 1797)))
	; 593..601 bytes
	delay_n (n-2)
	nop
.elseif(n = 1779)
	; 596 bytes
	delay_n 105
	delay_n (n-105)
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
	; 598 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1785)
	; 598 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1786)
	; 596 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1788)
	; 596 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1789)
	; 600 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1791)
	; 600 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1792)
	; 598 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1794)
	; 598 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1795)
	; 602 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
.if((n >= 1797) && (n < 1816))
; 1797 to 1815
.if(n = 1797)
	; 602 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1798)
	; 600 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 1798 && n < 1815)))
	; 601..607 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800)
	; 600 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1801)
	; 604 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1803)
	; 604 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1804)
	; 602 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1806)
	; 602 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1807)
	; 606 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1809)
	; 606 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1810)
	; 604 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1812)
	; 604 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1813)
	; 608 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1815)
	; 608 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 1816 to 1835
.if(n = 1816)
	; 606 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 1816 && n < 1836)))
	; 607..615 bytes
	delay_n (n-2)
	nop
.elseif(n = 1818)
	; 606 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1819)
	; 610 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1821)
	; 610 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1822)
	; 608 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1824)
	; 608 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1825)
	; 612 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1827)
	; 612 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1828)
	; 610 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1830)
	; 610 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1831)
	; 614 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1833)
	; 614 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 1834)
	; 612 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 1836) && (n < 1856))
; 1836 to 1855
.if(n = 1836)
	; 612 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 1837)
	; 616 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 1837 && n < 1854)))
	; 613..621 bytes
	delay_n (n-2)
	nop
.elseif(n = 1839)
	; 616 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1840)
	; 614 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1842)
	; 614 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1843)
	; 618 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1845)
	; 618 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1846)
	; 616 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1848)
	; 616 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1849)
	; 620 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1851)
	; 620 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1852)
	; 618 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1854)
	; 618 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1855)
	; 622 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 1856 to 1875
.if((n .mod 3 = 2) && ((n > 1855 && n < 1875)))
	; 619..627 bytes
	delay_n (n-2)
	nop
.elseif(n = 1857)
	; 622 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1858)
	; 620 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1860)
	; 620 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1861)
	; 624 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1863)
	; 624 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1864)
	; 622 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1866)
	; 622 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1867)
	; 626 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1869)
	; 626 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1870)
	; 624 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1872)
	; 624 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1873)
	; 628 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1875)
	; 628 bytes
	delay_n 93
	delay_n (n-93)
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
.if((n >= 1876) && (n < 1895))
; 1876 to 1894
.if(n = 1876)
	; 626 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 1876 && n < 1893)))
	; 627..633 bytes
	delay_n (n-2)
	nop
.elseif(n = 1878)
	; 626 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1879)
	; 630 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1881)
	; 630 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1882)
	; 628 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1884)
	; 628 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1885)
	; 632 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1887)
	; 632 bytes
	delay_n 105
	delay_n (n-105)
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
	; 634 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1893)
	; 634 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1894)
	; 632 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 1895 to 1914
.if((n .mod 3 = 2) && ((n > 1894 && n < 1914)))
	; 633..641 bytes
	delay_n (n-2)
	nop
.elseif(n = 1896)
	; 632 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1897)
	; 636 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1899)
	; 636 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 1900)
	; 634 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1902)
	; 634 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1903)
	; 638 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1905)
	; 638 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1906)
	; 636 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1908)
	; 636 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1909)
	; 640 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1911)
	; 640 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1912)
	; 638 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1914)
	; 638 bytes
	delay_n 24
	delay_n (n-24)
.endif
.endif
.else
.if((n >= 1915) && (n < 1934))
; 1915 to 1933
.if(n = 1915)
	; 642 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 1915 && n < 1932)))
	; 639..647 bytes
	delay_n (n-2)
	nop
.elseif(n = 1917)
	; 642 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1918)
	; 640 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 1920)
	; 640 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1921)
	; 644 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1923)
	; 644 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1924)
	; 642 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1926)
	; 642 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1927)
	; 646 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1929)
	; 646 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1930)
	; 644 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1932)
	; 644 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1933)
	; 648 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 1934 to 1953
.if((n .mod 3 = 2) && ((n > 1933 && n < 1953)))
	; 645..653 bytes
	delay_n (n-2)
	nop
.elseif(n = 1935)
	; 648 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1936)
	; 646 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1938)
	; 646 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 1939)
	; 650 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 1941)
	; 650 bytes
	delay_n 105
	delay_n (n-105)
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
	; 652 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 1947)
	; 652 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 1948)
	; 650 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 1950)
	; 650 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 1951)
	; 654 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 1953)
	; 654 bytes
	delay_n 117
	delay_n (n-117)
.endif
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
.if((n >= 1954) && (n < 1973))
; 1954 to 1972
.if(n = 1954)
	; 652 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 1954 && n < 1971)))
	; 653..659 bytes
	delay_n (n-2)
	nop
.elseif(n = 1956)
	; 652 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1957)
	; 656 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 1959)
	; 656 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 1960)
	; 654 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 1962)
	; 654 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 1963)
	; 658 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 1965)
	; 658 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 1966)
	; 656 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 1968)
	; 656 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 1969)
	; 660 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 1971)
	; 660 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 1972)
	; 658 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 1973 to 1992
.if((n .mod 3 = 2) && ((n > 1972 && n < 1992)))
	; 659..667 bytes
	delay_n (n-2)
	nop
.elseif(n = 1974)
	; 658 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 1975)
	; 662 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 1977)
	; 662 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1978)
	; 660 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 1980)
	; 660 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 1981)
	; 664 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1983)
	; 664 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 1984)
	; 662 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 1986)
	; 662 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 1987)
	; 666 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1989)
	; 666 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 1990)
	; 664 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 1992)
	; 664 bytes
	delay_n 48
	delay_n (n-48)
.endif
.endif
.else
.if((n >= 1993) && (n < 2012))
; 1993 to 2011
.if(n = 1993)
	; 668 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 1993 && n < 2010)))
	; 665..673 bytes
	delay_n (n-2)
	nop
.elseif(n = 1995)
	; 668 bytes
	delay_n 105
	delay_n (n-105)
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
	; 670 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2001)
	; 670 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2002)
	; 668 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2004)
	; 668 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2005)
	; 672 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2007)
	; 672 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2008)
	; 670 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2010)
	; 670 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2011)
	; 674 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 2012 to 2031
.if((n .mod 3 = 2) && ((n > 2011 && n < 2031)))
	; 671..679 bytes
	delay_n (n-2)
	nop
.elseif(n = 2013)
	; 674 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2014)
	; 672 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2016)
	; 672 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2017)
	; 676 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2019)
	; 676 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2020)
	; 674 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2022)
	; 674 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2023)
	; 678 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2025)
	; 678 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2026)
	; 676 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2028)
	; 676 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2029)
	; 680 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2031)
	; 680 bytes
	delay_n 141
	delay_n (n-141)
.endif
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032)
	; 678 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 2032 && n < 2049)))
	; 679..685 bytes
	delay_n (n-2)
	nop
.elseif(n = 2034)
	; 678 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2035)
	; 682 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2037)
	; 682 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2038)
	; 680 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2040)
	; 680 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2041)
	; 684 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2043)
	; 684 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2044)
	; 682 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2046)
	; 682 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2047)
	; 686 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2049)
	; 686 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 2050)
	; 684 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 2051 to 2070
.if((n .mod 3 = 2) && ((n > 2050 && n < 2070)))
	; 685..693 bytes
	delay_n (n-2)
	nop
.elseif(n = 2052)
	; 684 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2053)
	; 688 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2055)
	; 688 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2056)
	; 686 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2058)
	; 686 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2059)
	; 690 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2061)
	; 690 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2062)
	; 688 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2064)
	; 688 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2065)
	; 692 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2067)
	; 692 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2068)
	; 690 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2070)
	; 690 bytes
	delay_n 72
	delay_n (n-72)
.endif
.endif
.else
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
.if(n = 2071)
	; 694 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 2071 && n < 2088)))
	; 691..699 bytes
	delay_n (n-2)
	nop
.elseif(n = 2073)
	; 694 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2074)
	; 692 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2076)
	; 692 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2077)
	; 696 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2079)
	; 696 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2080)
	; 694 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2082)
	; 694 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2083)
	; 698 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2085)
	; 698 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2086)
	; 696 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2088)
	; 696 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2089)
	; 700 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 2090 to 2109
.if((n .mod 3 = 2) && ((n > 2089 && n < 2109)))
	; 697..705 bytes
	delay_n (n-2)
	nop
.elseif(n = 2091)
	; 700 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2092)
	; 698 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2094)
	; 698 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2095)
	; 702 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2097)
	; 702 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2098)
	; 700 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2100)
	; 700 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2101)
	; 704 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2103)
	; 704 bytes
	delay_n 105
	delay_n (n-105)
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
	; 706 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2109)
	; 706 bytes
	delay_n 57
	delay_n (n-57)
.endif
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
.if((n >= 2110) && (n < 2129))
; 2110 to 2128
.if(n = 2110)
	; 704 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 2110 && n < 2127)))
	; 705..711 bytes
	delay_n (n-2)
	nop
.elseif(n = 2112)
	; 704 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2113)
	; 708 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2115)
	; 708 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2116)
	; 706 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2118)
	; 706 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2119)
	; 710 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2121)
	; 710 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2122)
	; 708 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2124)
	; 708 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2125)
	; 712 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2127)
	; 712 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2128)
	; 710 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 2129 to 2148
.if((n .mod 3 = 2) && ((n > 2128 && n < 2148)))
	; 711..719 bytes
	delay_n (n-2)
	nop
.elseif(n = 2130)
	; 710 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2131)
	; 714 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2133)
	; 714 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2134)
	; 712 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2136)
	; 712 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2137)
	; 716 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2139)
	; 716 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2140)
	; 714 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2142)
	; 714 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2143)
	; 718 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2145)
	; 718 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2146)
	; 716 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2148)
	; 716 bytes
	delay_n 96
	delay_n (n-96)
.endif
.endif
.else
.if((n >= 2149) && (n < 2168))
; 2149 to 2167
.if(n = 2149)
	; 720 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 2149 && n < 2166)))
	; 717..725 bytes
	delay_n (n-2)
	nop
.elseif(n = 2151)
	; 720 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2152)
	; 718 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2154)
	; 718 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2155)
	; 722 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2157)
	; 722 bytes
	delay_n 105
	delay_n (n-105)
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
	; 724 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2163)
	; 724 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2164)
	; 722 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2166)
	; 722 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2167)
	; 726 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 2168 to 2187
.if((n .mod 3 = 2) && ((n > 2167 && n < 2187)))
	; 723..731 bytes
	delay_n (n-2)
	nop
.elseif(n = 2169)
	; 726 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2170)
	; 724 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2172)
	; 724 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2173)
	; 728 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2175)
	; 728 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2176)
	; 726 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2178)
	; 726 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2179)
	; 730 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2181)
	; 730 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2182)
	; 728 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2184)
	; 728 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2185)
	; 732 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2187)
	; 732 bytes
	delay_n 81
	delay_n (n-81)
.endif
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
	; 730 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 2188 && n < 2205)))
	; 731..737 bytes
	delay_n (n-2)
	nop
.elseif(n = 2190)
	; 730 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2191)
	; 734 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2193)
	; 734 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2194)
	; 732 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2196)
	; 732 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2197)
	; 736 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2199)
	; 736 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2200)
	; 734 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2202)
	; 734 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2203)
	; 738 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2205)
	; 738 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2206)
	; 736 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 2207 to 2226
.if((n .mod 3 = 2) && ((n > 2206 && n < 2226)))
	; 737..745 bytes
	delay_n (n-2)
	nop
.elseif(n = 2208)
	; 736 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2209)
	; 740 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2211)
	; 740 bytes
	delay_n 105
	delay_n (n-105)
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
	; 742 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2217)
	; 742 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2218)
	; 740 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2220)
	; 740 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2221)
	; 744 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2223)
	; 744 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2224)
	; 742 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2226)
	; 742 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227)
	; 746 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 2227 && n < 2244)))
	; 743..751 bytes
	delay_n (n-2)
	nop
.elseif(n = 2229)
	; 746 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2230)
	; 744 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2232)
	; 744 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2233)
	; 748 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2235)
	; 748 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2236)
	; 746 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2238)
	; 746 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2239)
	; 750 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2241)
	; 750 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2242)
	; 748 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2244)
	; 748 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2245)
	; 752 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 2246 to 2265
.if((n .mod 3 = 2) && ((n > 2245 && n < 2265)))
	; 749..757 bytes
	delay_n (n-2)
	nop
.elseif(n = 2247)
	; 752 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2248)
	; 750 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2250)
	; 750 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2251)
	; 754 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2253)
	; 754 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2254)
	; 752 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2256)
	; 752 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2257)
	; 756 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2259)
	; 756 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2260)
	; 754 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2262)
	; 754 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2263)
	; 758 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2265)
	; 758 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
.if((n >= 2266) && (n < 2285))
; 2266 to 2284
.if(n = 2266)
	; 756 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 2266 && n < 2283)))
	; 757..763 bytes
	delay_n (n-2)
	nop
.elseif(n = 2268)
	; 756 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2269)
	; 760 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2271)
	; 760 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2272)
	; 758 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2274)
	; 758 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2275)
	; 762 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2277)
	; 762 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2278)
	; 760 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2280)
	; 760 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2281)
	; 764 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2283)
	; 764 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2284)
	; 762 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 2285 to 2304
.if((n .mod 3 = 2) && ((n > 2284 && n < 2304)))
	; 763..771 bytes
	delay_n (n-2)
	nop
.elseif(n = 2286)
	; 762 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2287)
	; 766 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2289)
	; 766 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2290)
	; 764 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2292)
	; 764 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2293)
	; 768 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2295)
	; 768 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2296)
	; 766 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2298)
	; 766 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2299)
	; 770 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2301)
	; 770 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2302)
	; 768 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2304)
	; 768 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 772 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 2305 && n < 2322)))
	; 769..777 bytes
	delay_n (n-2)
	nop
.elseif(n = 2307)
	; 772 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2308)
	; 770 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2310)
	; 770 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2311)
	; 774 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2313)
	; 774 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2314)
	; 772 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2316)
	; 772 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2317)
	; 776 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2319)
	; 776 bytes
	delay_n 105
	delay_n (n-105)
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
	; 778 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 2324 to 2343
.if((n .mod 3 = 2) && ((n > 2323 && n < 2343)))
	; 775..783 bytes
	delay_n (n-2)
	nop
.elseif(n = 2325)
	; 778 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2326)
	; 776 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2328)
	; 776 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2329)
	; 780 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2331)
	; 780 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2332)
	; 778 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2334)
	; 778 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2335)
	; 782 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2337)
	; 782 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2338)
	; 780 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2340)
	; 780 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2341)
	; 784 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2343)
	; 784 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
.if((n >= 2344) && (n < 2363))
; 2344 to 2362
.if(n = 2344)
	; 782 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 2344 && n < 2361)))
	; 783..789 bytes
	delay_n (n-2)
	nop
.elseif(n = 2346)
	; 782 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2347)
	; 786 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2349)
	; 786 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2350)
	; 784 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2352)
	; 784 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2353)
	; 788 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2355)
	; 788 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2356)
	; 786 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2358)
	; 786 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2359)
	; 790 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2361)
	; 790 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2362)
	; 788 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 2363 to 2382
.if((n .mod 3 = 2) && ((n > 2362 && n < 2382)))
	; 789..797 bytes
	delay_n (n-2)
	nop
.elseif(n = 2364)
	; 788 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2365)
	; 792 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2367)
	; 792 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2368)
	; 790 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2370)
	; 790 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2371)
	; 794 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2373)
	; 794 bytes
	delay_n 105
	delay_n (n-105)
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
	; 796 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2379)
	; 796 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2380)
	; 794 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2382)
	; 794 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.else
.if((n >= 2383) && (n < 2402))
; 2383 to 2401
.if(n = 2383)
	; 798 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 2383 && n < 2400)))
	; 795..803 bytes
	delay_n (n-2)
	nop
.elseif(n = 2385)
	; 798 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2386)
	; 796 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2388)
	; 796 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2389)
	; 800 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2391)
	; 800 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2392)
	; 798 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2394)
	; 798 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2395)
	; 802 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2397)
	; 802 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2398)
	; 800 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2400)
	; 800 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2401)
	; 804 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 2402 to 2421
.if((n .mod 3 = 2) && ((n > 2401 && n < 2421)))
	; 801..809 bytes
	delay_n (n-2)
	nop
.elseif(n = 2403)
	; 804 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2404)
	; 802 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2406)
	; 802 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2407)
	; 806 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2409)
	; 806 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2410)
	; 804 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2412)
	; 804 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2413)
	; 808 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2415)
	; 808 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2416)
	; 806 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2418)
	; 806 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2419)
	; 810 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2421)
	; 810 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
.if((n >= 2422) && (n < 2441))
; 2422 to 2440
.if(n = 2422)
	; 808 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 2422 && n < 2439)))
	; 809..815 bytes
	delay_n (n-2)
	nop
.elseif(n = 2424)
	; 808 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2425)
	; 812 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2427)
	; 812 bytes
	delay_n 105
	delay_n (n-105)
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
	; 814 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2433)
	; 814 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2434)
	; 812 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2436)
	; 812 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2437)
	; 816 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2439)
	; 816 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2440)
	; 814 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 2441 to 2460
.if((n .mod 3 = 2) && ((n > 2440 && n < 2460)))
	; 815..823 bytes
	delay_n (n-2)
	nop
.elseif(n = 2442)
	; 814 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2443)
	; 818 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2445)
	; 818 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2446)
	; 816 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2448)
	; 816 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2449)
	; 820 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2451)
	; 820 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2452)
	; 818 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2454)
	; 818 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2455)
	; 822 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2457)
	; 822 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2458)
	; 820 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2460)
	; 820 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.else
.if((n >= 2461) && (n < 2481))
; 2461 to 2480
.if(n = 2461)
	; 824 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 2461 && n < 2481)))
	; 821..829 bytes
	delay_n (n-2)
	nop
.elseif(n = 2463)
	; 824 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2464)
	; 822 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2466)
	; 822 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2467)
	; 826 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2469)
	; 826 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2470)
	; 824 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2472)
	; 824 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2473)
	; 828 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2475)
	; 828 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2476)
	; 826 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2478)
	; 826 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2479)
	; 830 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 2481 to 2500
.if(n = 2481)
	; 830 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 2482)
	; 828 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 2482 && n < 2499)))
	; 829..835 bytes
	delay_n (n-2)
	nop
.elseif(n = 2484)
	; 828 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2485)
	; 832 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2487)
	; 832 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2488)
	; 830 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2490)
	; 830 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2491)
	; 834 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2493)
	; 834 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2494)
	; 832 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2496)
	; 832 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2497)
	; 836 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2499)
	; 836 bytes
	delay_n 69
	delay_n (n-69)
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
.endif
.else
.if((n >= 2501) && (n < 3751))
.if((n >= 2501) && (n < 3126))
.if((n >= 2501) && (n < 2813))
.if((n >= 2501) && (n < 2657))
.if((n >= 2501) && (n < 2579))
.if((n >= 2501) && (n < 2540))
.if((n >= 2501) && (n < 2520))
; 2501 to 2519
.if((n .mod 3 = 2) && ((n > 2500 && n < 2520)))
	; 835..843 bytes
	delay_n (n-2)
	nop
.elseif(n = 2502)
	; 834 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2503)
	; 838 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2505)
	; 838 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2506)
	; 836 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2508)
	; 836 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2509)
	; 840 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2511)
	; 840 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2512)
	; 838 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2514)
	; 838 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2515)
	; 842 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2517)
	; 842 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2518)
	; 840 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 2520 to 2539
.if(n = 2520)
	; 840 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2521)
	; 844 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 2521 && n < 2538)))
	; 841..849 bytes
	delay_n (n-2)
	nop
.elseif(n = 2523)
	; 844 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2524)
	; 842 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2526)
	; 842 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2527)
	; 846 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2529)
	; 846 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2530)
	; 844 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2532)
	; 844 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2533)
	; 848 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2535)
	; 848 bytes
	delay_n 105
	delay_n (n-105)
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
	; 850 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if((n .mod 3 = 2) && ((n > 2539 && n < 2559)))
	; 847..855 bytes
	delay_n (n-2)
	nop
.elseif(n = 2541)
	; 850 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2542)
	; 848 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2544)
	; 848 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2545)
	; 852 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2547)
	; 852 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2548)
	; 850 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2550)
	; 850 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2551)
	; 854 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2553)
	; 854 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2554)
	; 852 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2556)
	; 852 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2557)
	; 856 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 2559 to 2578
.if(n = 2559)
	; 856 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2560)
	; 854 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 2560 && n < 2577)))
	; 855..861 bytes
	delay_n (n-2)
	nop
.elseif(n = 2562)
	; 854 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2563)
	; 858 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2565)
	; 858 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2566)
	; 856 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2568)
	; 856 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2569)
	; 860 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2571)
	; 860 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2572)
	; 858 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2574)
	; 858 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2575)
	; 862 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2577)
	; 862 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2578)
	; 860 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
.if((n >= 2579) && (n < 2598))
; 2579 to 2597
.if((n .mod 3 = 2) && ((n > 2578 && n < 2598)))
	; 861..869 bytes
	delay_n (n-2)
	nop
.elseif(n = 2580)
	; 860 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2581)
	; 864 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2583)
	; 864 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2584)
	; 862 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2586)
	; 862 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2587)
	; 866 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2589)
	; 866 bytes
	delay_n 105
	delay_n (n-105)
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
	; 868 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2595)
	; 868 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2596)
	; 866 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 2598 to 2617
.if(n = 2598)
	; 866 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2599)
	; 870 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 2599 && n < 2616)))
	; 867..875 bytes
	delay_n (n-2)
	nop
.elseif(n = 2601)
	; 870 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2602)
	; 868 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2604)
	; 868 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2605)
	; 872 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2607)
	; 872 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2608)
	; 870 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2610)
	; 870 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2611)
	; 874 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2613)
	; 874 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2614)
	; 872 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2616)
	; 872 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2617)
	; 876 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 2618) && (n < 2637))
; 2618 to 2636
.if((n .mod 3 = 2) && ((n > 2617 && n < 2637)))
	; 873..881 bytes
	delay_n (n-2)
	nop
.elseif(n = 2619)
	; 876 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2620)
	; 874 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2622)
	; 874 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2623)
	; 878 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2625)
	; 878 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2626)
	; 876 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2628)
	; 876 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2629)
	; 880 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2631)
	; 880 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2632)
	; 878 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2634)
	; 878 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2635)
	; 882 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 2637 to 2656
.if(n = 2637)
	; 882 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2638)
	; 880 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 2638 && n < 2655)))
	; 881..887 bytes
	delay_n (n-2)
	nop
.elseif(n = 2640)
	; 880 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2641)
	; 884 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2643)
	; 884 bytes
	delay_n 105
	delay_n (n-105)
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
	; 886 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2649)
	; 886 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2650)
	; 884 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2652)
	; 884 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2653)
	; 888 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2655)
	; 888 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2656)
	; 886 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
.if((n >= 2657) && (n < 2676))
; 2657 to 2675
.if((n .mod 3 = 2) && ((n > 2656 && n < 2676)))
	; 887..895 bytes
	delay_n (n-2)
	nop
.elseif(n = 2658)
	; 886 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2659)
	; 890 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2661)
	; 890 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2662)
	; 888 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2664)
	; 888 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2665)
	; 892 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2667)
	; 892 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2668)
	; 890 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2670)
	; 890 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2671)
	; 894 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2673)
	; 894 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2674)
	; 892 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 2676 to 2695
.if(n = 2676)
	; 892 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2677)
	; 896 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 2677 && n < 2694)))
	; 893..901 bytes
	delay_n (n-2)
	nop
.elseif(n = 2679)
	; 896 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2680)
	; 894 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2682)
	; 894 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2683)
	; 898 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2685)
	; 898 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2686)
	; 896 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2688)
	; 896 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2689)
	; 900 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2691)
	; 900 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2692)
	; 898 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2694)
	; 898 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2695)
	; 902 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 2696) && (n < 2715))
; 2696 to 2714
.if((n .mod 3 = 2) && ((n > 2695 && n < 2715)))
	; 899..907 bytes
	delay_n (n-2)
	nop
.elseif(n = 2697)
	; 902 bytes
	delay_n 105
	delay_n (n-105)
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
	; 904 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2703)
	; 904 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2704)
	; 902 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2706)
	; 902 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2707)
	; 906 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2709)
	; 906 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2710)
	; 904 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2712)
	; 904 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2713)
	; 908 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 2715 to 2734
.if(n = 2715)
	; 908 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2716)
	; 906 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 2716 && n < 2733)))
	; 907..913 bytes
	delay_n (n-2)
	nop
.elseif(n = 2718)
	; 906 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2719)
	; 910 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2721)
	; 910 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2722)
	; 908 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2724)
	; 908 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2725)
	; 912 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2727)
	; 912 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2728)
	; 910 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2730)
	; 910 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2731)
	; 914 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2733)
	; 914 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2734)
	; 912 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
.if((n >= 2735) && (n < 2754))
; 2735 to 2753
.if((n .mod 3 = 2) && ((n > 2734 && n < 2754)))
	; 913..921 bytes
	delay_n (n-2)
	nop
.elseif(n = 2736)
	; 912 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2737)
	; 916 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2739)
	; 916 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2740)
	; 914 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2742)
	; 914 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2743)
	; 918 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2745)
	; 918 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2746)
	; 916 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2748)
	; 916 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2749)
	; 920 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2751)
	; 920 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 2752)
	; 918 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 918 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2755)
	; 922 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 2755 && n < 2772)))
	; 919..927 bytes
	delay_n (n-2)
	nop
.elseif(n = 2757)
	; 922 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2758)
	; 920 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2760)
	; 920 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2761)
	; 924 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2763)
	; 924 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2764)
	; 922 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2766)
	; 922 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2767)
	; 926 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2769)
	; 926 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2770)
	; 924 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2772)
	; 924 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2773)
	; 928 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.else
.if((n >= 2774) && (n < 2793))
; 2774 to 2792
.if((n .mod 3 = 2) && ((n > 2773 && n < 2793)))
	; 925..933 bytes
	delay_n (n-2)
	nop
.elseif(n = 2775)
	; 928 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2776)
	; 926 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2778)
	; 926 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2779)
	; 930 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2781)
	; 930 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2782)
	; 928 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2784)
	; 928 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2785)
	; 932 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2787)
	; 932 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2788)
	; 930 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2790)
	; 930 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2791)
	; 934 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 2793 to 2812
.if(n = 2793)
	; 934 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2794)
	; 932 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 2794 && n < 2811)))
	; 933..939 bytes
	delay_n (n-2)
	nop
.elseif(n = 2796)
	; 932 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2797)
	; 936 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2799)
	; 936 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2800)
	; 934 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2802)
	; 934 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2803)
	; 938 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2805)
	; 938 bytes
	delay_n 105
	delay_n (n-105)
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
	; 940 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2811)
	; 940 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2812)
	; 938 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
.if((n >= 2813) && (n < 2832))
; 2813 to 2831
.if((n .mod 3 = 2) && ((n > 2812 && n < 2832)))
	; 939..947 bytes
	delay_n (n-2)
	nop
.elseif(n = 2814)
	; 938 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2815)
	; 942 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2817)
	; 942 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2818)
	; 940 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2820)
	; 940 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2821)
	; 944 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2823)
	; 944 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2824)
	; 942 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2826)
	; 942 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2827)
	; 946 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2829)
	; 946 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2830)
	; 944 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 2832 to 2851
.if(n = 2832)
	; 944 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2833)
	; 948 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 2833 && n < 2850)))
	; 945..953 bytes
	delay_n (n-2)
	nop
.elseif(n = 2835)
	; 948 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2836)
	; 946 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2838)
	; 946 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2839)
	; 950 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2841)
	; 950 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2842)
	; 948 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2844)
	; 948 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2845)
	; 952 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2847)
	; 952 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2848)
	; 950 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2850)
	; 950 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2851)
	; 954 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 2852) && (n < 2871))
; 2852 to 2870
.if((n .mod 3 = 2) && ((n > 2851 && n < 2871)))
	; 951..959 bytes
	delay_n (n-2)
	nop
.elseif(n = 2853)
	; 954 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2854)
	; 952 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2856)
	; 952 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2857)
	; 956 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2859)
	; 956 bytes
	delay_n 105
	delay_n (n-105)
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
	; 958 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2865)
	; 958 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2866)
	; 956 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2868)
	; 956 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2869)
	; 960 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 2871 to 2890
.if(n = 2871)
	; 960 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2872)
	; 958 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 2872 && n < 2889)))
	; 959..965 bytes
	delay_n (n-2)
	nop
.elseif(n = 2874)
	; 958 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2875)
	; 962 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2877)
	; 962 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2878)
	; 960 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2880)
	; 960 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2881)
	; 964 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2883)
	; 964 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2884)
	; 962 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2886)
	; 962 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2887)
	; 966 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2889)
	; 966 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2890)
	; 964 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
.if((n >= 2891) && (n < 2910))
; 2891 to 2909
.if((n .mod 3 = 2) && ((n > 2890 && n < 2910)))
	; 965..973 bytes
	delay_n (n-2)
	nop
.elseif(n = 2892)
	; 964 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2893)
	; 968 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 2895)
	; 968 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2896)
	; 966 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 2898)
	; 966 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2899)
	; 970 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2901)
	; 970 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2902)
	; 968 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2904)
	; 968 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2905)
	; 972 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2907)
	; 972 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2908)
	; 970 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 2910 to 2929
.if(n = 2910)
	; 970 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2911)
	; 974 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 2911 && n < 2928)))
	; 971..979 bytes
	delay_n (n-2)
	nop
.elseif(n = 2913)
	; 974 bytes
	delay_n 105
	delay_n (n-105)
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
	; 976 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2919)
	; 976 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2920)
	; 974 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2922)
	; 974 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2923)
	; 978 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2925)
	; 978 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2926)
	; 976 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2928)
	; 976 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2929)
	; 980 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 2930) && (n < 2949))
; 2930 to 2948
.if((n .mod 3 = 2) && ((n > 2929 && n < 2949)))
	; 977..985 bytes
	delay_n (n-2)
	nop
.elseif(n = 2931)
	; 980 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2932)
	; 978 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 2934)
	; 978 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2935)
	; 982 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 2937)
	; 982 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2938)
	; 980 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2940)
	; 980 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2941)
	; 984 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2943)
	; 984 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2944)
	; 982 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 2946)
	; 982 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 2947)
	; 986 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 2949 to 2968
.if(n = 2949)
	; 986 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2950)
	; 984 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 2950 && n < 2967)))
	; 985..991 bytes
	delay_n (n-2)
	nop
.elseif(n = 2952)
	; 984 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2953)
	; 988 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 2955)
	; 988 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 2956)
	; 986 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 2958)
	; 986 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 2959)
	; 990 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2961)
	; 990 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 2962)
	; 988 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 2964)
	; 988 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 2965)
	; 992 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 2967)
	; 992 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 2968)
	; 990 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
.if((n >= 2969) && (n < 2988))
; 2969 to 2987
.if((n .mod 3 = 2) && ((n > 2968 && n < 2988)))
	; 991..999 bytes
	delay_n (n-2)
	nop
.elseif(n = 2970)
	; 990 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 2971)
	; 994 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 2973)
	; 994 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 2974)
	; 992 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 2976)
	; 992 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 2977)
	; 996 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 2979)
	; 996 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 2980)
	; 994 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2982)
	; 994 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2983)
	; 998 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 2985)
	; 998 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 2986)
	; 996 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 2988 to 3007
.if(n = 2988)
	; 996 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 2989)
	; 1000 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 2989 && n < 3006)))
	; 997..1005 bytes
	delay_n (n-2)
	nop
.elseif(n = 2991)
	; 1000 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 2992)
	; 998 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 2994)
	; 998 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 2995)
	; 1002 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 2997)
	; 1002 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 2998)
	; 1000 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3000)
	; 1000 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3001)
	; 1004 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3003)
	; 1004 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3004)
	; 1002 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3006)
	; 1002 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3007)
	; 1006 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 3008) && (n < 3027))
; 3008 to 3026
.if((n .mod 3 = 2) && ((n > 3007 && n < 3027)))
	; 1003..1011 bytes
	delay_n (n-2)
	nop
.elseif(n = 3009)
	; 1006 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3010)
	; 1004 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3012)
	; 1004 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3013)
	; 1008 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3015)
	; 1008 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3016)
	; 1006 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3018)
	; 1006 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3019)
	; 1010 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3021)
	; 1010 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1012 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 3027 to 3046
.if(n = 3027)
	; 1012 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3028)
	; 1010 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 3028 && n < 3045)))
	; 1011..1017 bytes
	delay_n (n-2)
	nop
.elseif(n = 3030)
	; 1010 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3031)
	; 1014 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3033)
	; 1014 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3034)
	; 1012 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3036)
	; 1012 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3037)
	; 1016 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3039)
	; 1016 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3040)
	; 1014 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3042)
	; 1014 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3043)
	; 1018 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3045)
	; 1018 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3046)
	; 1016 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
.if((n .mod 3 = 2) && ((n > 3046 && n < 3066)))
	; 1017..1025 bytes
	delay_n (n-2)
	nop
.elseif(n = 3048)
	; 1016 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3049)
	; 1020 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3051)
	; 1020 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3052)
	; 1018 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3054)
	; 1018 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3055)
	; 1022 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3057)
	; 1022 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3058)
	; 1020 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3060)
	; 1020 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3061)
	; 1024 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3063)
	; 1024 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3064)
	; 1022 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 3066 to 3085
.if(n = 3066)
	; 1022 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3067)
	; 1026 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 3067 && n < 3084)))
	; 1023..1031 bytes
	delay_n (n-2)
	nop
.elseif(n = 3069)
	; 1026 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3070)
	; 1024 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3072)
	; 1024 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3073)
	; 1028 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3075)
	; 1028 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1030 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3081)
	; 1030 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3082)
	; 1028 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3084)
	; 1028 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3085)
	; 1032 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.else
.if((n >= 3086) && (n < 3106))
; 3086 to 3105
.if((n .mod 3 = 2) && ((n > 3085 && n < 3105)))
	; 1029..1037 bytes
	delay_n (n-2)
	nop
.elseif(n = 3087)
	; 1032 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3088)
	; 1030 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3090)
	; 1030 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3091)
	; 1034 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3093)
	; 1034 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3094)
	; 1032 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3096)
	; 1032 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3097)
	; 1036 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3099)
	; 1036 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3100)
	; 1034 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3102)
	; 1034 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3103)
	; 1038 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3105)
	; 1038 bytes
	delay_n 81
	delay_n (n-81)
.endif
.else
; 3106 to 3125
.if(n = 3106)
	; 1036 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 3106 && n < 3126)))
	; 1037..1045 bytes
	delay_n (n-2)
	nop
.elseif(n = 3108)
	; 1036 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3109)
	; 1040 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3111)
	; 1040 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3112)
	; 1038 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3114)
	; 1038 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3115)
	; 1042 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3117)
	; 1042 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3118)
	; 1040 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3120)
	; 1040 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3121)
	; 1044 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3123)
	; 1044 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3124)
	; 1042 bytes
	delay_n 46
	delay_n (n-46)
.endif
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
.if((n >= 3126) && (n < 3145))
; 3126 to 3144
.if(n = 3126)
	; 1042 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3127)
	; 1046 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 3127 && n < 3144)))
	; 1043..1051 bytes
	delay_n (n-2)
	nop
.elseif(n = 3129)
	; 1046 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1048 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3135)
	; 1048 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3136)
	; 1046 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3138)
	; 1046 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3139)
	; 1050 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3141)
	; 1050 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3142)
	; 1048 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3144)
	; 1048 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 3145 to 3164
.if(n = 3145)
	; 1052 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 3145 && n < 3165)))
	; 1049..1057 bytes
	delay_n (n-2)
	nop
.elseif(n = 3147)
	; 1052 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3148)
	; 1050 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3150)
	; 1050 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3151)
	; 1054 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3153)
	; 1054 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3154)
	; 1052 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3156)
	; 1052 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3157)
	; 1056 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3159)
	; 1056 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3160)
	; 1054 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3162)
	; 1054 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3163)
	; 1058 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.else
.if((n >= 3165) && (n < 3184))
; 3165 to 3183
.if(n = 3165)
	; 1058 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3166)
	; 1056 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 3166 && n < 3183)))
	; 1057..1063 bytes
	delay_n (n-2)
	nop
.elseif(n = 3168)
	; 1056 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3169)
	; 1060 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3171)
	; 1060 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3172)
	; 1058 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3174)
	; 1058 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3175)
	; 1062 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3177)
	; 1062 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3178)
	; 1060 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3180)
	; 1060 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3181)
	; 1064 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3183)
	; 1064 bytes
	delay_n 105
	delay_n (n-105)
.endif
.else
; 3184 to 3203
.if(n = 3184)
	; 1062 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 3184 && n < 3204)))
	; 1063..1071 bytes
	delay_n (n-2)
	nop
.elseif(n = 3186)
	; 1062 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3187)
	; 1066 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3189)
	; 1066 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3190)
	; 1064 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3192)
	; 1064 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3193)
	; 1068 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3195)
	; 1068 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3196)
	; 1066 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3198)
	; 1066 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3199)
	; 1070 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3201)
	; 1070 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3202)
	; 1068 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
.if((n >= 3204) && (n < 3223))
; 3204 to 3222
.if(n = 3204)
	; 1068 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3205)
	; 1072 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 3205 && n < 3222)))
	; 1069..1077 bytes
	delay_n (n-2)
	nop
.elseif(n = 3207)
	; 1072 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3208)
	; 1070 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3210)
	; 1070 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3211)
	; 1074 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3213)
	; 1074 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3214)
	; 1072 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3216)
	; 1072 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3217)
	; 1076 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3219)
	; 1076 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3220)
	; 1074 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3222)
	; 1074 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 3223 to 3242
.if(n = 3223)
	; 1078 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 3223 && n < 3243)))
	; 1075..1083 bytes
	delay_n (n-2)
	nop
.elseif(n = 3225)
	; 1078 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3226)
	; 1076 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3228)
	; 1076 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3229)
	; 1080 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3231)
	; 1080 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3232)
	; 1078 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3234)
	; 1078 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3235)
	; 1082 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3237)
	; 1082 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1084 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 3243) && (n < 3262))
; 3243 to 3261
.if(n = 3243)
	; 1084 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3244)
	; 1082 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 3244 && n < 3261)))
	; 1083..1089 bytes
	delay_n (n-2)
	nop
.elseif(n = 3246)
	; 1082 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3247)
	; 1086 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3249)
	; 1086 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3250)
	; 1084 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3252)
	; 1084 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3253)
	; 1088 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3255)
	; 1088 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3256)
	; 1086 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3258)
	; 1086 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3259)
	; 1090 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3261)
	; 1090 bytes
	delay_n 129
	delay_n (n-129)
.endif
.else
; 3262 to 3281
.if(n = 3262)
	; 1088 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 3262 && n < 3282)))
	; 1089..1097 bytes
	delay_n (n-2)
	nop
.elseif(n = 3264)
	; 1088 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3265)
	; 1092 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3267)
	; 1092 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3268)
	; 1090 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3270)
	; 1090 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3271)
	; 1094 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3273)
	; 1094 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3274)
	; 1092 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3276)
	; 1092 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3277)
	; 1096 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3279)
	; 1096 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3280)
	; 1094 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
.if((n >= 3282) && (n < 3301))
; 3282 to 3300
.if(n = 3282)
	; 1094 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3283)
	; 1098 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 3283 && n < 3300)))
	; 1095..1103 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285)
	; 1098 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3286)
	; 1096 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3288)
	; 1096 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3289)
	; 1100 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3291)
	; 1100 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1102 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3297)
	; 1102 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3298)
	; 1100 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3300)
	; 1100 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 3301 to 3320
.if(n = 3301)
	; 1104 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 3301 && n < 3321)))
	; 1101..1109 bytes
	delay_n (n-2)
	nop
.elseif(n = 3303)
	; 1104 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3304)
	; 1102 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3306)
	; 1102 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3307)
	; 1106 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3309)
	; 1106 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3310)
	; 1104 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3312)
	; 1104 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3313)
	; 1108 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3315)
	; 1108 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3316)
	; 1106 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3318)
	; 1106 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3319)
	; 1110 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 3321) && (n < 3340))
; 3321 to 3339
.if(n = 3321)
	; 1110 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3322)
	; 1108 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 3322 && n < 3339)))
	; 1109..1115 bytes
	delay_n (n-2)
	nop
.elseif(n = 3324)
	; 1108 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3325)
	; 1112 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3327)
	; 1112 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3328)
	; 1110 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3330)
	; 1110 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3331)
	; 1114 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3333)
	; 1114 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3334)
	; 1112 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3336)
	; 1112 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3337)
	; 1116 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3339)
	; 1116 bytes
	delay_n 45
	delay_n (n-45)
.endif
.else
; 3340 to 3359
.if(n = 3340)
	; 1114 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 3340 && n < 3360)))
	; 1115..1123 bytes
	delay_n (n-2)
	nop
.elseif(n = 3342)
	; 1114 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3343)
	; 1118 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3345)
	; 1118 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1120 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3351)
	; 1120 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3352)
	; 1118 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3354)
	; 1118 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3355)
	; 1122 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3357)
	; 1122 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3358)
	; 1120 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
.if((n >= 3360) && (n < 3379))
; 3360 to 3378
.if(n = 3360)
	; 1120 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3361)
	; 1124 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 3361 && n < 3378)))
	; 1121..1129 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363)
	; 1124 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3364)
	; 1122 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3366)
	; 1122 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3367)
	; 1126 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3369)
	; 1126 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3370)
	; 1124 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3372)
	; 1124 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3373)
	; 1128 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3375)
	; 1128 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3376)
	; 1126 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3378)
	; 1126 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 3379 to 3398
.if(n = 3379)
	; 1130 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 3379 && n < 3399)))
	; 1127..1135 bytes
	delay_n (n-2)
	nop
.elseif(n = 3381)
	; 1130 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3382)
	; 1128 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3384)
	; 1128 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3385)
	; 1132 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3387)
	; 1132 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3388)
	; 1130 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3390)
	; 1130 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3391)
	; 1134 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3393)
	; 1134 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3394)
	; 1132 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3396)
	; 1132 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3397)
	; 1136 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 3399) && (n < 3418))
; 3399 to 3417
.if(n = 3399)
	; 1136 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 3400)
	; 1134 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 3400 && n < 3417)))
	; 1135..1141 bytes
	delay_n (n-2)
	nop
.elseif(n = 3402)
	; 1134 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3403)
	; 1138 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3405)
	; 1138 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3406)
	; 1136 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3408)
	; 1136 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3409)
	; 1140 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3411)
	; 1140 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3412)
	; 1138 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3414)
	; 1138 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3415)
	; 1142 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3417)
	; 1142 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 3418 to 3437
.if(n = 3418)
	; 1140 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 3418 && n < 3438)))
	; 1141..1149 bytes
	delay_n (n-2)
	nop
.elseif(n = 3420)
	; 1140 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3421)
	; 1144 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3423)
	; 1144 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3424)
	; 1142 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3426)
	; 1142 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3427)
	; 1146 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3429)
	; 1146 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3430)
	; 1144 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3432)
	; 1144 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3433)
	; 1148 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3435)
	; 1148 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3436)
	; 1146 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
.if((n >= 3438) && (n < 3457))
; 3438 to 3456
.if(n = 3438)
	; 1146 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3439)
	; 1150 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 3439 && n < 3456)))
	; 1147..1155 bytes
	delay_n (n-2)
	nop
.elseif(n = 3441)
	; 1150 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3442)
	; 1148 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3444)
	; 1148 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3445)
	; 1152 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3447)
	; 1152 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3448)
	; 1150 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3450)
	; 1150 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3451)
	; 1154 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3453)
	; 1154 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 3454)
	; 1152 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 3456)
	; 1152 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 3457 to 3476
.if(n = 3457)
	; 1156 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 3457 && n < 3477)))
	; 1153..1161 bytes
	delay_n (n-2)
	nop
.elseif(n = 3459)
	; 1156 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3460)
	; 1154 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3462)
	; 1154 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3463)
	; 1158 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3465)
	; 1158 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3466)
	; 1156 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3468)
	; 1156 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3469)
	; 1160 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3471)
	; 1160 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3472)
	; 1158 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3474)
	; 1158 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3475)
	; 1162 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.else
.if((n >= 3477) && (n < 3496))
; 3477 to 3495
.if(n = 3477)
	; 1162 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3478)
	; 1160 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 3478 && n < 3495)))
	; 1161..1167 bytes
	delay_n (n-2)
	nop
.elseif(n = 3480)
	; 1160 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3481)
	; 1164 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3483)
	; 1164 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3484)
	; 1162 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3486)
	; 1162 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3487)
	; 1166 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3489)
	; 1166 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3490)
	; 1164 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3492)
	; 1164 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3493)
	; 1168 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3495)
	; 1168 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 3496 to 3515
.if(n = 3496)
	; 1166 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 3496 && n < 3516)))
	; 1167..1175 bytes
	delay_n (n-2)
	nop
.elseif(n = 3498)
	; 1166 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3499)
	; 1170 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3501)
	; 1170 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3502)
	; 1168 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3504)
	; 1168 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3505)
	; 1172 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3507)
	; 1172 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1174 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3513)
	; 1174 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3514)
	; 1172 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
.if((n >= 3516) && (n < 3535))
; 3516 to 3534
.if(n = 3516)
	; 1172 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3517)
	; 1176 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 3517 && n < 3534)))
	; 1173..1181 bytes
	delay_n (n-2)
	nop
.elseif(n = 3519)
	; 1176 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3520)
	; 1174 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3522)
	; 1174 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3523)
	; 1178 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3525)
	; 1178 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3526)
	; 1176 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3528)
	; 1176 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3529)
	; 1180 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3531)
	; 1180 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3532)
	; 1178 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3534)
	; 1178 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 3535 to 3554
.if(n = 3535)
	; 1182 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 3535 && n < 3555)))
	; 1179..1187 bytes
	delay_n (n-2)
	nop
.elseif(n = 3537)
	; 1182 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3538)
	; 1180 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3540)
	; 1180 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3541)
	; 1184 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3543)
	; 1184 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3544)
	; 1182 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3546)
	; 1182 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3547)
	; 1186 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3549)
	; 1186 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3550)
	; 1184 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3552)
	; 1184 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3553)
	; 1188 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 3555) && (n < 3574))
; 3555 to 3573
.if(n = 3555)
	; 1188 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3556)
	; 1186 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 3556 && n < 3573)))
	; 1187..1193 bytes
	delay_n (n-2)
	nop
.elseif(n = 3558)
	; 1186 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3559)
	; 1190 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3561)
	; 1190 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1192 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3567)
	; 1192 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3568)
	; 1190 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3570)
	; 1190 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3571)
	; 1194 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3573)
	; 1194 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 3574 to 3593
.if(n = 3574)
	; 1192 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 3574 && n < 3594)))
	; 1193..1201 bytes
	delay_n (n-2)
	nop
.elseif(n = 3576)
	; 1192 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3577)
	; 1196 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3579)
	; 1196 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3580)
	; 1194 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3582)
	; 1194 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3583)
	; 1198 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3585)
	; 1198 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3586)
	; 1196 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3588)
	; 1196 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3589)
	; 1200 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3591)
	; 1200 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3592)
	; 1198 bytes
	delay_n 82
	delay_n (n-82)
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
	; 1198 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3595)
	; 1202 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 3595 && n < 3612)))
	; 1199..1207 bytes
	delay_n (n-2)
	nop
.elseif(n = 3597)
	; 1202 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3598)
	; 1200 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3600)
	; 1200 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3601)
	; 1204 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3603)
	; 1204 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3604)
	; 1202 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3606)
	; 1202 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3607)
	; 1206 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3609)
	; 1206 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3610)
	; 1204 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3612)
	; 1204 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 3613 to 3632
.if(n = 3613)
	; 1208 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 3613 && n < 3633)))
	; 1205..1213 bytes
	delay_n (n-2)
	nop
.elseif(n = 3615)
	; 1208 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1210 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3621)
	; 1210 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3622)
	; 1208 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3624)
	; 1208 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3625)
	; 1212 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3627)
	; 1212 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3628)
	; 1210 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3630)
	; 1210 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3631)
	; 1214 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 3633) && (n < 3652))
; 3633 to 3651
.if(n = 3633)
	; 1214 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3634)
	; 1212 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 3634 && n < 3651)))
	; 1213..1219 bytes
	delay_n (n-2)
	nop
.elseif(n = 3636)
	; 1212 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3637)
	; 1216 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3639)
	; 1216 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3640)
	; 1214 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3642)
	; 1214 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3643)
	; 1218 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3645)
	; 1218 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3646)
	; 1216 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3648)
	; 1216 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3649)
	; 1220 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3651)
	; 1220 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 3652 to 3671
.if(n = 3652)
	; 1218 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 3652 && n < 3672)))
	; 1219..1227 bytes
	delay_n (n-2)
	nop
.elseif(n = 3654)
	; 1218 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3655)
	; 1222 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3657)
	; 1222 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3658)
	; 1220 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3660)
	; 1220 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3661)
	; 1224 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3663)
	; 1224 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3664)
	; 1222 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3666)
	; 1222 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3667)
	; 1226 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3669)
	; 1226 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 3670)
	; 1224 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
.if((n >= 3672) && (n < 3691))
; 3672 to 3690
.if(n = 3672)
	; 1224 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3673)
	; 1228 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 3673 && n < 3690)))
	; 1225..1233 bytes
	delay_n (n-2)
	nop
.elseif(n = 3675)
	; 1228 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3676)
	; 1226 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3678)
	; 1226 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3679)
	; 1230 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3681)
	; 1230 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3682)
	; 1228 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3684)
	; 1228 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3685)
	; 1232 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3687)
	; 1232 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3688)
	; 1230 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3690)
	; 1230 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 3691 to 3710
.if(n = 3691)
	; 1234 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 3691 && n < 3711)))
	; 1231..1239 bytes
	delay_n (n-2)
	nop
.elseif(n = 3693)
	; 1234 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3694)
	; 1232 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3696)
	; 1232 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3697)
	; 1236 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3699)
	; 1236 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3700)
	; 1234 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3702)
	; 1234 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3703)
	; 1238 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3705)
	; 1238 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3706)
	; 1236 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3708)
	; 1236 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3709)
	; 1240 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 3711) && (n < 3731))
; 3711 to 3730
.if(n = 3711)
	; 1240 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3712)
	; 1238 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 3712 && n < 3729)))
	; 1239..1245 bytes
	delay_n (n-2)
	nop
.elseif(n = 3714)
	; 1238 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3715)
	; 1242 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3717)
	; 1242 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3718)
	; 1240 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3720)
	; 1240 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3721)
	; 1244 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3723)
	; 1244 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1246 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3729)
	; 1246 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3730)
	; 1244 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 3731 to 3750
.if((n .mod 3 = 2) && ((n > 3730 && n < 3750)))
	; 1245..1253 bytes
	delay_n (n-2)
	nop
.elseif(n = 3732)
	; 1244 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3733)
	; 1248 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3735)
	; 1248 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3736)
	; 1246 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3738)
	; 1246 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3739)
	; 1250 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3741)
	; 1250 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3742)
	; 1248 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3744)
	; 1248 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3745)
	; 1252 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3747)
	; 1252 bytes
	delay_n 129
	delay_n (n-129)
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
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
.if((n >= 3751) && (n < 3790))
.if((n >= 3751) && (n < 3770))
; 3751 to 3769
.if(n = 3751)
	; 1254 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 3751 && n < 3768)))
	; 1251..1259 bytes
	delay_n (n-2)
	nop
.elseif(n = 3753)
	; 1254 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3754)
	; 1252 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3756)
	; 1252 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3757)
	; 1256 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3759)
	; 1256 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3760)
	; 1254 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3762)
	; 1254 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3763)
	; 1258 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3765)
	; 1258 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3766)
	; 1256 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3768)
	; 1256 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3769)
	; 1260 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 3770 to 3789
.if((n .mod 3 = 2) && ((n > 3769 && n < 3789)))
	; 1257..1265 bytes
	delay_n (n-2)
	nop
.elseif(n = 3771)
	; 1260 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3772)
	; 1258 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3774)
	; 1258 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3775)
	; 1262 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3777)
	; 1262 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1264 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3783)
	; 1264 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3784)
	; 1262 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3786)
	; 1262 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3787)
	; 1266 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3789)
	; 1266 bytes
	delay_n 117
	delay_n (n-117)
.endif
.endif
.else
.if((n >= 3790) && (n < 3809))
; 3790 to 3808
.if(n = 3790)
	; 1264 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 3790 && n < 3807)))
	; 1265..1271 bytes
	delay_n (n-2)
	nop
.elseif(n = 3792)
	; 1264 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3793)
	; 1268 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3795)
	; 1268 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3796)
	; 1266 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3798)
	; 1266 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3799)
	; 1270 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3801)
	; 1270 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3802)
	; 1268 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3804)
	; 1268 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3805)
	; 1272 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3807)
	; 1272 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3808)
	; 1270 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 3809 to 3828
.if((n .mod 3 = 2) && ((n > 3808 && n < 3828)))
	; 1271..1279 bytes
	delay_n (n-2)
	nop
.elseif(n = 3810)
	; 1270 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3811)
	; 1274 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3813)
	; 1274 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3814)
	; 1272 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3816)
	; 1272 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3817)
	; 1276 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3819)
	; 1276 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3820)
	; 1274 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3822)
	; 1274 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3823)
	; 1278 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3825)
	; 1278 bytes
	delay_n 45
	delay_n (n-45)
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
.endif
.else
.if((n >= 3829) && (n < 3868))
.if((n >= 3829) && (n < 3848))
; 3829 to 3847
.if(n = 3829)
	; 1280 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 3829 && n < 3846)))
	; 1277..1285 bytes
	delay_n (n-2)
	nop
.elseif(n = 3831)
	; 1280 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1282 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3837)
	; 1282 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3838)
	; 1280 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3840)
	; 1280 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3841)
	; 1284 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3843)
	; 1284 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3844)
	; 1282 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3846)
	; 1282 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3847)
	; 1286 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 3848 to 3867
.if((n .mod 3 = 2) && ((n > 3847 && n < 3867)))
	; 1283..1291 bytes
	delay_n (n-2)
	nop
.elseif(n = 3849)
	; 1286 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3850)
	; 1284 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3852)
	; 1284 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3853)
	; 1288 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3855)
	; 1288 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3856)
	; 1286 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3858)
	; 1286 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3859)
	; 1290 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3861)
	; 1290 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3862)
	; 1288 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3864)
	; 1288 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3865)
	; 1292 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3867)
	; 1292 bytes
	delay_n 141
	delay_n (n-141)
.endif
.endif
.else
.if((n >= 3868) && (n < 3887))
; 3868 to 3886
.if(n = 3868)
	; 1290 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 3868 && n < 3885)))
	; 1291..1297 bytes
	delay_n (n-2)
	nop
.elseif(n = 3870)
	; 1290 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3871)
	; 1294 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3873)
	; 1294 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3874)
	; 1292 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3876)
	; 1292 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3877)
	; 1296 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3879)
	; 1296 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3880)
	; 1294 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3882)
	; 1294 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3883)
	; 1298 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3885)
	; 1298 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 3886)
	; 1296 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 3887 to 3906
.if((n .mod 3 = 2) && ((n > 3886 && n < 3906)))
	; 1297..1305 bytes
	delay_n (n-2)
	nop
.elseif(n = 3888)
	; 1296 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 3889)
	; 1300 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3891)
	; 1300 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 3892)
	; 1298 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 3894)
	; 1298 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3895)
	; 1302 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3897)
	; 1302 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3898)
	; 1300 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3900)
	; 1300 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3901)
	; 1304 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3903)
	; 1304 bytes
	delay_n 69
	delay_n (n-69)
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
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
.if((n >= 3907) && (n < 3926))
; 3907 to 3925
.if(n = 3907)
	; 1306 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 3907 && n < 3924)))
	; 1303..1311 bytes
	delay_n (n-2)
	nop
.elseif(n = 3909)
	; 1306 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3910)
	; 1304 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 3912)
	; 1304 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3913)
	; 1308 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3915)
	; 1308 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3916)
	; 1306 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3918)
	; 1306 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3919)
	; 1310 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3921)
	; 1310 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3922)
	; 1308 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3924)
	; 1308 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3925)
	; 1312 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 3926 to 3945
.if((n .mod 3 = 2) && ((n > 3925 && n < 3945)))
	; 1309..1317 bytes
	delay_n (n-2)
	nop
.elseif(n = 3927)
	; 1312 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 3928)
	; 1310 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 3930)
	; 1310 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 3931)
	; 1314 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3933)
	; 1314 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3934)
	; 1312 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3936)
	; 1312 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3937)
	; 1316 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3939)
	; 1316 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1318 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3945)
	; 1318 bytes
	delay_n 57
	delay_n (n-57)
.endif
.endif
.else
.if((n >= 3946) && (n < 3965))
; 3946 to 3964
.if(n = 3946)
	; 1316 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 3946 && n < 3963)))
	; 1317..1323 bytes
	delay_n (n-2)
	nop
.elseif(n = 3948)
	; 1316 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 3949)
	; 1320 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 3951)
	; 1320 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 3952)
	; 1318 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3954)
	; 1318 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3955)
	; 1322 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 3957)
	; 1322 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 3958)
	; 1320 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 3960)
	; 1320 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 3961)
	; 1324 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 3963)
	; 1324 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 3964)
	; 1322 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 3965 to 3984
.if((n .mod 3 = 2) && ((n > 3964 && n < 3984)))
	; 1323..1331 bytes
	delay_n (n-2)
	nop
.elseif(n = 3966)
	; 1322 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 3967)
	; 1326 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 3969)
	; 1326 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3970)
	; 1324 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 3972)
	; 1324 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 3973)
	; 1328 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 3975)
	; 1328 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 3976)
	; 1326 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 3978)
	; 1326 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3979)
	; 1330 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 3981)
	; 1330 bytes
	delay_n 93
	delay_n (n-93)
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
.endif
.else
.if((n >= 3985) && (n < 4024))
.if((n >= 3985) && (n < 4004))
; 3985 to 4003
.if(n = 3985)
	; 1332 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 3985 && n < 4002)))
	; 1329..1337 bytes
	delay_n (n-2)
	nop
.elseif(n = 3987)
	; 1332 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 3988)
	; 1330 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 3990)
	; 1330 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 3991)
	; 1334 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 3993)
	; 1334 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1336 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 3999)
	; 1336 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4000)
	; 1334 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4002)
	; 1334 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4003)
	; 1338 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 4004 to 4023
.if((n .mod 3 = 2) && ((n > 4003 && n < 4023)))
	; 1335..1343 bytes
	delay_n (n-2)
	nop
.elseif(n = 4005)
	; 1338 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4006)
	; 1336 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4008)
	; 1336 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4009)
	; 1340 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4011)
	; 1340 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4012)
	; 1338 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4014)
	; 1338 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4015)
	; 1342 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4017)
	; 1342 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4018)
	; 1340 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4020)
	; 1340 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4021)
	; 1344 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4023)
	; 1344 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.else
.if((n >= 4024) && (n < 4043))
; 4024 to 4042
.if(n = 4024)
	; 1342 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 4024 && n < 4041)))
	; 1343..1349 bytes
	delay_n (n-2)
	nop
.elseif(n = 4026)
	; 1342 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4027)
	; 1346 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4029)
	; 1346 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4030)
	; 1344 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4032)
	; 1344 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4033)
	; 1348 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4035)
	; 1348 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4036)
	; 1346 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4038)
	; 1346 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4039)
	; 1350 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4041)
	; 1350 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4042)
	; 1348 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 4043 to 4062
.if((n .mod 3 = 2) && ((n > 4042 && n < 4062)))
	; 1349..1357 bytes
	delay_n (n-2)
	nop
.elseif(n = 4044)
	; 1348 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4045)
	; 1352 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4047)
	; 1352 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1354 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4053)
	; 1354 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4054)
	; 1352 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4056)
	; 1352 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4057)
	; 1356 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4059)
	; 1356 bytes
	delay_n 117
	delay_n (n-117)
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
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
.if((n >= 4063) && (n < 4082))
; 4063 to 4081
.if(n = 4063)
	; 1358 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 4063 && n < 4080)))
	; 1355..1363 bytes
	delay_n (n-2)
	nop
.elseif(n = 4065)
	; 1358 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4066)
	; 1356 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4068)
	; 1356 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4069)
	; 1360 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4071)
	; 1360 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4072)
	; 1358 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4074)
	; 1358 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4075)
	; 1362 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4077)
	; 1362 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4078)
	; 1360 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4080)
	; 1360 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4081)
	; 1364 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 4082 to 4101
.if((n .mod 3 = 2) && ((n > 4081 && n < 4101)))
	; 1361..1369 bytes
	delay_n (n-2)
	nop
.elseif(n = 4083)
	; 1364 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4084)
	; 1362 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4086)
	; 1362 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4087)
	; 1366 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4089)
	; 1366 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4090)
	; 1364 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4092)
	; 1364 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4093)
	; 1368 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4095)
	; 1368 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4096)
	; 1366 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4098)
	; 1366 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4099)
	; 1370 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4101)
	; 1370 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.else
.if((n >= 4102) && (n < 4121))
; 4102 to 4120
.if(n = 4102)
	; 1368 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 4102 && n < 4119)))
	; 1369..1375 bytes
	delay_n (n-2)
	nop
.elseif(n = 4104)
	; 1368 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4105)
	; 1372 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4107)
	; 1372 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4108)
	; 1370 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4110)
	; 1370 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4111)
	; 1374 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4113)
	; 1374 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4114)
	; 1372 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4116)
	; 1372 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4117)
	; 1376 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4119)
	; 1376 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4120)
	; 1374 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 4121 to 4140
.if((n .mod 3 = 2) && ((n > 4120 && n < 4140)))
	; 1375..1383 bytes
	delay_n (n-2)
	nop
.elseif(n = 4122)
	; 1374 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4123)
	; 1378 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4125)
	; 1378 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4126)
	; 1376 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4128)
	; 1376 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4129)
	; 1380 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4131)
	; 1380 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4132)
	; 1378 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4134)
	; 1378 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4135)
	; 1382 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4137)
	; 1382 bytes
	delay_n 141
	delay_n (n-141)
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
.endif
.else
.if((n >= 4141) && (n < 4180))
.if((n >= 4141) && (n < 4160))
; 4141 to 4159
.if(n = 4141)
	; 1384 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 4141 && n < 4158)))
	; 1381..1389 bytes
	delay_n (n-2)
	nop
.elseif(n = 4143)
	; 1384 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4144)
	; 1382 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4146)
	; 1382 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4147)
	; 1386 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4149)
	; 1386 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4150)
	; 1384 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4152)
	; 1384 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4153)
	; 1388 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4155)
	; 1388 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1390 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 4160 to 4179
.if((n .mod 3 = 2) && ((n > 4159 && n < 4179)))
	; 1387..1395 bytes
	delay_n (n-2)
	nop
.elseif(n = 4161)
	; 1390 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4162)
	; 1388 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4164)
	; 1388 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4165)
	; 1392 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4167)
	; 1392 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4168)
	; 1390 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4170)
	; 1390 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4171)
	; 1394 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4173)
	; 1394 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4174)
	; 1392 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4176)
	; 1392 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4177)
	; 1396 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4179)
	; 1396 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.else
.if((n >= 4180) && (n < 4199))
; 4180 to 4198
.if(n = 4180)
	; 1394 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 4180 && n < 4197)))
	; 1395..1401 bytes
	delay_n (n-2)
	nop
.elseif(n = 4182)
	; 1394 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4183)
	; 1398 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4185)
	; 1398 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4186)
	; 1396 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4188)
	; 1396 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4189)
	; 1400 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4191)
	; 1400 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4192)
	; 1398 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4194)
	; 1398 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4195)
	; 1402 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4197)
	; 1402 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4198)
	; 1400 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 4199 to 4218
.if((n .mod 3 = 2) && ((n > 4198 && n < 4218)))
	; 1401..1409 bytes
	delay_n (n-2)
	nop
.elseif(n = 4200)
	; 1400 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4201)
	; 1404 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4203)
	; 1404 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4204)
	; 1402 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4206)
	; 1402 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4207)
	; 1406 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4209)
	; 1406 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1408 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4215)
	; 1408 bytes
	delay_n 57
	delay_n (n-57)
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
.endif
.else
.if((n >= 4219) && (n < 4297))
.if((n >= 4219) && (n < 4258))
.if((n >= 4219) && (n < 4238))
; 4219 to 4237
.if(n = 4219)
	; 1410 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 4219 && n < 4236)))
	; 1407..1415 bytes
	delay_n (n-2)
	nop
.elseif(n = 4221)
	; 1410 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4222)
	; 1408 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4224)
	; 1408 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4225)
	; 1412 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4227)
	; 1412 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4228)
	; 1410 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4230)
	; 1410 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4231)
	; 1414 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4233)
	; 1414 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4234)
	; 1412 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4236)
	; 1412 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4237)
	; 1416 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 4238 to 4257
.if((n .mod 3 = 2) && ((n > 4237 && n < 4257)))
	; 1413..1421 bytes
	delay_n (n-2)
	nop
.elseif(n = 4239)
	; 1416 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4240)
	; 1414 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4242)
	; 1414 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4243)
	; 1418 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4245)
	; 1418 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4246)
	; 1416 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4248)
	; 1416 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4249)
	; 1420 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4251)
	; 1420 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4252)
	; 1418 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4254)
	; 1418 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4255)
	; 1422 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4257)
	; 1422 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.else
.if((n >= 4258) && (n < 4277))
; 4258 to 4276
.if(n = 4258)
	; 1420 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 4258 && n < 4275)))
	; 1421..1427 bytes
	delay_n (n-2)
	nop
.elseif(n = 4260)
	; 1420 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4261)
	; 1424 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4263)
	; 1424 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1426 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4269)
	; 1426 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4270)
	; 1424 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4272)
	; 1424 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4273)
	; 1428 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4275)
	; 1428 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4276)
	; 1426 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 4277 to 4296
.if((n .mod 3 = 2) && ((n > 4276 && n < 4296)))
	; 1427..1435 bytes
	delay_n (n-2)
	nop
.elseif(n = 4278)
	; 1426 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4279)
	; 1430 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4281)
	; 1430 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4282)
	; 1428 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4284)
	; 1428 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4285)
	; 1432 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4287)
	; 1432 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4288)
	; 1430 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4290)
	; 1430 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4291)
	; 1434 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4293)
	; 1434 bytes
	delay_n 81
	delay_n (n-81)
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
.endif
.else
.if((n >= 4297) && (n < 4336))
.if((n >= 4297) && (n < 4316))
; 4297 to 4315
.if(n = 4297)
	; 1436 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 4297 && n < 4314)))
	; 1433..1441 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299)
	; 1436 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4300)
	; 1434 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4302)
	; 1434 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4303)
	; 1438 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4305)
	; 1438 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4306)
	; 1436 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4308)
	; 1436 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4309)
	; 1440 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4311)
	; 1440 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4312)
	; 1438 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4314)
	; 1438 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4315)
	; 1442 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 4316 to 4335
.if((n .mod 3 = 2) && ((n > 4315 && n < 4335)))
	; 1439..1447 bytes
	delay_n (n-2)
	nop
.elseif(n = 4317)
	; 1442 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1444 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4323)
	; 1444 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4324)
	; 1442 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4326)
	; 1442 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4327)
	; 1446 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4329)
	; 1446 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4330)
	; 1444 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4332)
	; 1444 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4333)
	; 1448 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4335)
	; 1448 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.else
.if((n >= 4336) && (n < 4356))
; 4336 to 4355
.if(n = 4336)
	; 1446 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 4336 && n < 4356)))
	; 1447..1455 bytes
	delay_n (n-2)
	nop
.elseif(n = 4338)
	; 1446 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4339)
	; 1450 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4341)
	; 1450 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4342)
	; 1448 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4344)
	; 1448 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4345)
	; 1452 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4347)
	; 1452 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4348)
	; 1450 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4350)
	; 1450 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4351)
	; 1454 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4353)
	; 1454 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4354)
	; 1452 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 4356 to 4375
.if(n = 4356)
	; 1452 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4357)
	; 1456 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 4357 && n < 4374)))
	; 1453..1461 bytes
	delay_n (n-2)
	nop
.elseif(n = 4359)
	; 1456 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4360)
	; 1454 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4362)
	; 1454 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4363)
	; 1458 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4365)
	; 1458 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4366)
	; 1456 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4368)
	; 1456 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4369)
	; 1460 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4371)
	; 1460 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1462 bytes
	delay_n 55
	delay_n (n-55)
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
.if((n >= 4376) && (n < 4395))
; 4376 to 4394
.if((n .mod 3 = 2) && ((n > 4375 && n < 4395)))
	; 1459..1467 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377)
	; 1462 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4378)
	; 1460 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4380)
	; 1460 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4381)
	; 1464 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4383)
	; 1464 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4384)
	; 1462 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4386)
	; 1462 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4387)
	; 1466 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4389)
	; 1466 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4390)
	; 1464 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4392)
	; 1464 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4393)
	; 1468 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 4395 to 4414
.if(n = 4395)
	; 1468 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4396)
	; 1466 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 4396 && n < 4413)))
	; 1467..1473 bytes
	delay_n (n-2)
	nop
.elseif(n = 4398)
	; 1466 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4399)
	; 1470 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4401)
	; 1470 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4402)
	; 1468 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4404)
	; 1468 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4405)
	; 1472 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4407)
	; 1472 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4408)
	; 1470 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4410)
	; 1470 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4411)
	; 1474 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4413)
	; 1474 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4414)
	; 1472 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 4415) && (n < 4434))
; 4415 to 4433
.if((n .mod 3 = 2) && ((n > 4414 && n < 4434)))
	; 1473..1481 bytes
	delay_n (n-2)
	nop
.elseif(n = 4416)
	; 1472 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4417)
	; 1476 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4419)
	; 1476 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4420)
	; 1474 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4422)
	; 1474 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4423)
	; 1478 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4425)
	; 1478 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1480 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4431)
	; 1480 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4432)
	; 1478 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 4434 to 4453
.if(n = 4434)
	; 1478 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4435)
	; 1482 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 4435 && n < 4452)))
	; 1479..1487 bytes
	delay_n (n-2)
	nop
.elseif(n = 4437)
	; 1482 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4438)
	; 1480 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4440)
	; 1480 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4441)
	; 1484 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4443)
	; 1484 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4444)
	; 1482 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4446)
	; 1482 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4447)
	; 1486 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4449)
	; 1486 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4450)
	; 1484 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4452)
	; 1484 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4453)
	; 1488 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
.if((n >= 4454) && (n < 4473))
; 4454 to 4472
.if((n .mod 3 = 2) && ((n > 4453 && n < 4473)))
	; 1485..1493 bytes
	delay_n (n-2)
	nop
.elseif(n = 4455)
	; 1488 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4456)
	; 1486 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4458)
	; 1486 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4459)
	; 1490 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4461)
	; 1490 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4462)
	; 1488 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4464)
	; 1488 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4465)
	; 1492 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4467)
	; 1492 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4468)
	; 1490 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4470)
	; 1490 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4471)
	; 1494 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 4473 to 4492
.if(n = 4473)
	; 1494 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4474)
	; 1492 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 4474 && n < 4491)))
	; 1493..1499 bytes
	delay_n (n-2)
	nop
.elseif(n = 4476)
	; 1492 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4477)
	; 1496 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4479)
	; 1496 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1498 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4485)
	; 1498 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4486)
	; 1496 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4488)
	; 1496 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4489)
	; 1500 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4491)
	; 1500 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4492)
	; 1498 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 4493) && (n < 4512))
; 4493 to 4511
.if((n .mod 3 = 2) && ((n > 4492 && n < 4512)))
	; 1499..1507 bytes
	delay_n (n-2)
	nop
.elseif(n = 4494)
	; 1498 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4495)
	; 1502 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4497)
	; 1502 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4498)
	; 1500 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4500)
	; 1500 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4501)
	; 1504 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4503)
	; 1504 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4504)
	; 1502 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4506)
	; 1502 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4507)
	; 1506 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4509)
	; 1506 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4510)
	; 1504 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 4512 to 4531
.if(n = 4512)
	; 1504 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4513)
	; 1508 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 4513 && n < 4530)))
	; 1505..1513 bytes
	delay_n (n-2)
	nop
.elseif(n = 4515)
	; 1508 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4516)
	; 1506 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4518)
	; 1506 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4519)
	; 1510 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4521)
	; 1510 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4522)
	; 1508 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4524)
	; 1508 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4525)
	; 1512 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4527)
	; 1512 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4528)
	; 1510 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4530)
	; 1510 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4531)
	; 1514 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
.if((n >= 4532) && (n < 4551))
; 4532 to 4550
.if((n .mod 3 = 2) && ((n > 4531 && n < 4551)))
	; 1511..1519 bytes
	delay_n (n-2)
	nop
.elseif(n = 4533)
	; 1514 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1516 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4539)
	; 1516 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4540)
	; 1514 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4542)
	; 1514 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4543)
	; 1518 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4545)
	; 1518 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4546)
	; 1516 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4548)
	; 1516 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4549)
	; 1520 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 4551 to 4570
.if(n = 4551)
	; 1520 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4552)
	; 1518 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 4552 && n < 4569)))
	; 1519..1525 bytes
	delay_n (n-2)
	nop
.elseif(n = 4554)
	; 1518 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4555)
	; 1522 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4557)
	; 1522 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4558)
	; 1520 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4560)
	; 1520 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4561)
	; 1524 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4563)
	; 1524 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4564)
	; 1522 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4566)
	; 1522 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4567)
	; 1526 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4569)
	; 1526 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4570)
	; 1524 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 4571) && (n < 4590))
; 4571 to 4589
.if((n .mod 3 = 2) && ((n > 4570 && n < 4590)))
	; 1525..1533 bytes
	delay_n (n-2)
	nop
.elseif(n = 4572)
	; 1524 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4573)
	; 1528 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4575)
	; 1528 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4576)
	; 1526 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4578)
	; 1526 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4579)
	; 1530 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4581)
	; 1530 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4582)
	; 1528 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4584)
	; 1528 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4585)
	; 1532 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4587)
	; 1532 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 4588)
	; 1530 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 4590 to 4609
.if(n = 4590)
	; 1530 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4591)
	; 1534 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 4591 && n < 4608)))
	; 1531..1539 bytes
	delay_n (n-2)
	nop
.elseif(n = 4593)
	; 1534 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4594)
	; 1532 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4596)
	; 1532 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4597)
	; 1536 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4599)
	; 1536 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4600)
	; 1534 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4602)
	; 1534 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4603)
	; 1538 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4605)
	; 1538 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4606)
	; 1536 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4608)
	; 1536 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4609)
	; 1540 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
.if((n >= 4610) && (n < 4629))
; 4610 to 4628
.if((n .mod 3 = 2) && ((n > 4609 && n < 4629)))
	; 1537..1545 bytes
	delay_n (n-2)
	nop
.elseif(n = 4611)
	; 1540 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4612)
	; 1538 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4614)
	; 1538 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4615)
	; 1542 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4617)
	; 1542 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4618)
	; 1540 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4620)
	; 1540 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4621)
	; 1544 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4623)
	; 1544 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4624)
	; 1542 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4626)
	; 1542 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4627)
	; 1546 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 4629 to 4648
.if(n = 4629)
	; 1546 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4630)
	; 1544 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 4630 && n < 4647)))
	; 1545..1551 bytes
	delay_n (n-2)
	nop
.elseif(n = 4632)
	; 1544 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4633)
	; 1548 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4635)
	; 1548 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4636)
	; 1546 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4638)
	; 1546 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4639)
	; 1550 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4641)
	; 1550 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1552 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4647)
	; 1552 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4648)
	; 1550 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 4649) && (n < 4668))
; 4649 to 4667
.if((n .mod 3 = 2) && ((n > 4648 && n < 4668)))
	; 1551..1559 bytes
	delay_n (n-2)
	nop
.elseif(n = 4650)
	; 1550 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4651)
	; 1554 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4653)
	; 1554 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4654)
	; 1552 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4656)
	; 1552 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4657)
	; 1556 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4659)
	; 1556 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4660)
	; 1554 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4662)
	; 1554 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4663)
	; 1558 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4665)
	; 1558 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4666)
	; 1556 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 4668 to 4687
.if(n = 4668)
	; 1556 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4669)
	; 1560 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 4669 && n < 4686)))
	; 1557..1565 bytes
	delay_n (n-2)
	nop
.elseif(n = 4671)
	; 1560 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4672)
	; 1558 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4674)
	; 1558 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4675)
	; 1562 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4677)
	; 1562 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4678)
	; 1560 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4680)
	; 1560 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4681)
	; 1564 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4683)
	; 1564 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4684)
	; 1562 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4686)
	; 1562 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4687)
	; 1566 bytes
	delay_n 151
	delay_n (n-151)
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
.if((n .mod 3 = 2) && ((n > 4687 && n < 4707)))
	; 1563..1571 bytes
	delay_n (n-2)
	nop
.elseif(n = 4689)
	; 1566 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4690)
	; 1564 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4692)
	; 1564 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4693)
	; 1568 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4695)
	; 1568 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1570 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4701)
	; 1570 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4702)
	; 1568 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4704)
	; 1568 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4705)
	; 1572 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 4707 to 4726
.if(n = 4707)
	; 1572 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4708)
	; 1570 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 4708 && n < 4725)))
	; 1571..1577 bytes
	delay_n (n-2)
	nop
.elseif(n = 4710)
	; 1570 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4711)
	; 1574 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4713)
	; 1574 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4714)
	; 1572 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4716)
	; 1572 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4717)
	; 1576 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4719)
	; 1576 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4720)
	; 1574 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4722)
	; 1574 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4723)
	; 1578 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4725)
	; 1578 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4726)
	; 1576 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 4727) && (n < 4746))
; 4727 to 4745
.if((n .mod 3 = 2) && ((n > 4726 && n < 4746)))
	; 1577..1585 bytes
	delay_n (n-2)
	nop
.elseif(n = 4728)
	; 1576 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4729)
	; 1580 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4731)
	; 1580 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4732)
	; 1578 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4734)
	; 1578 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4735)
	; 1582 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4737)
	; 1582 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4738)
	; 1580 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4740)
	; 1580 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4741)
	; 1584 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4743)
	; 1584 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4744)
	; 1582 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 4746 to 4765
.if(n = 4746)
	; 1582 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4747)
	; 1586 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 4747 && n < 4764)))
	; 1583..1591 bytes
	delay_n (n-2)
	nop
.elseif(n = 4749)
	; 1586 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1588 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4755)
	; 1588 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4756)
	; 1586 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4758)
	; 1586 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4759)
	; 1590 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4761)
	; 1590 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4762)
	; 1588 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4764)
	; 1588 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4765)
	; 1592 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
.if((n >= 4766) && (n < 4785))
; 4766 to 4784
.if((n .mod 3 = 2) && ((n > 4765 && n < 4785)))
	; 1589..1597 bytes
	delay_n (n-2)
	nop
.elseif(n = 4767)
	; 1592 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4768)
	; 1590 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4770)
	; 1590 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4771)
	; 1594 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4773)
	; 1594 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4774)
	; 1592 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4776)
	; 1592 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4777)
	; 1596 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4779)
	; 1596 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4780)
	; 1594 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4782)
	; 1594 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4783)
	; 1598 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 4785 to 4804
.if(n = 4785)
	; 1598 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4786)
	; 1596 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 4786 && n < 4803)))
	; 1597..1603 bytes
	delay_n (n-2)
	nop
.elseif(n = 4788)
	; 1596 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4789)
	; 1600 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4791)
	; 1600 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4792)
	; 1598 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4794)
	; 1598 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4795)
	; 1602 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4797)
	; 1602 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4798)
	; 1600 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4800)
	; 1600 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4801)
	; 1604 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4803)
	; 1604 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 4804)
	; 1602 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 4805) && (n < 4824))
; 4805 to 4823
.if((n .mod 3 = 2) && ((n > 4804 && n < 4824)))
	; 1603..1611 bytes
	delay_n (n-2)
	nop
.elseif(n = 4806)
	; 1602 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 4807)
	; 1606 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4809)
	; 1606 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4810)
	; 1604 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4812)
	; 1604 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4813)
	; 1608 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4815)
	; 1608 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4816)
	; 1606 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4818)
	; 1606 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4819)
	; 1610 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4821)
	; 1610 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4822)
	; 1608 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 4824 to 4843
.if(n = 4824)
	; 1608 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4825)
	; 1612 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 4825 && n < 4842)))
	; 1609..1617 bytes
	delay_n (n-2)
	nop
.elseif(n = 4827)
	; 1612 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4828)
	; 1610 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4830)
	; 1610 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4831)
	; 1614 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4833)
	; 1614 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4834)
	; 1612 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4836)
	; 1612 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4837)
	; 1616 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4839)
	; 1616 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4840)
	; 1614 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4842)
	; 1614 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4843)
	; 1618 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
.if((n >= 4844) && (n < 4863))
; 4844 to 4862
.if((n .mod 3 = 2) && ((n > 4843 && n < 4863)))
	; 1615..1623 bytes
	delay_n (n-2)
	nop
.elseif(n = 4845)
	; 1618 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4846)
	; 1616 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4848)
	; 1616 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4849)
	; 1620 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4851)
	; 1620 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4852)
	; 1618 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4854)
	; 1618 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4855)
	; 1622 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4857)
	; 1622 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1624 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 4863 to 4882
.if(n = 4863)
	; 1624 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4864)
	; 1622 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 4864 && n < 4881)))
	; 1623..1629 bytes
	delay_n (n-2)
	nop
.elseif(n = 4866)
	; 1622 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4867)
	; 1626 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4869)
	; 1626 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4870)
	; 1624 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4872)
	; 1624 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4873)
	; 1628 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4875)
	; 1628 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4876)
	; 1626 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4878)
	; 1626 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4879)
	; 1630 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4881)
	; 1630 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4882)
	; 1628 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 4883) && (n < 4902))
; 4883 to 4901
.if((n .mod 3 = 2) && ((n > 4882 && n < 4902)))
	; 1629..1637 bytes
	delay_n (n-2)
	nop
.elseif(n = 4884)
	; 1628 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4885)
	; 1632 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4887)
	; 1632 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4888)
	; 1630 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4890)
	; 1630 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4891)
	; 1634 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4893)
	; 1634 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4894)
	; 1632 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4896)
	; 1632 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4897)
	; 1636 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4899)
	; 1636 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4900)
	; 1634 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 4902 to 4921
.if(n = 4902)
	; 1634 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4903)
	; 1638 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 4903 && n < 4920)))
	; 1635..1643 bytes
	delay_n (n-2)
	nop
.elseif(n = 4905)
	; 1638 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4906)
	; 1636 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4908)
	; 1636 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4909)
	; 1640 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4911)
	; 1640 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1642 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4917)
	; 1642 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4918)
	; 1640 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4920)
	; 1640 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4921)
	; 1644 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
.if((n >= 4922) && (n < 4941))
; 4922 to 4940
.if((n .mod 3 = 2) && ((n > 4921 && n < 4941)))
	; 1641..1649 bytes
	delay_n (n-2)
	nop
.elseif(n = 4923)
	; 1644 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4924)
	; 1642 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4926)
	; 1642 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4927)
	; 1646 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 4929)
	; 1646 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4930)
	; 1644 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4932)
	; 1644 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4933)
	; 1648 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4935)
	; 1648 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4936)
	; 1646 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4938)
	; 1646 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4939)
	; 1650 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 4941 to 4960
.if(n = 4941)
	; 1650 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4942)
	; 1648 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 4942 && n < 4959)))
	; 1649..1655 bytes
	delay_n (n-2)
	nop
.elseif(n = 4944)
	; 1648 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4945)
	; 1652 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 4947)
	; 1652 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4948)
	; 1650 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4950)
	; 1650 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4951)
	; 1654 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4953)
	; 1654 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 4954)
	; 1652 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 4956)
	; 1652 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4957)
	; 1656 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4959)
	; 1656 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 4960)
	; 1654 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 4961) && (n < 4981))
; 4961 to 4980
.if((n .mod 3 = 2) && ((n > 4960 && n < 4980)))
	; 1655..1663 bytes
	delay_n (n-2)
	nop
.elseif(n = 4962)
	; 1654 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 4963)
	; 1658 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 4965)
	; 1658 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1660 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 4971)
	; 1660 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 4972)
	; 1658 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 4974)
	; 1658 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 4975)
	; 1662 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 4977)
	; 1662 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 4978)
	; 1660 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4980)
	; 1660 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 4981 to 5000
.if(n = 4981)
	; 1664 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 4981 && n < 5001)))
	; 1661..1669 bytes
	delay_n (n-2)
	nop
.elseif(n = 4983)
	; 1664 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 4984)
	; 1662 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 4986)
	; 1662 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 4987)
	; 1666 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 4989)
	; 1666 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 4990)
	; 1664 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 4992)
	; 1664 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 4993)
	; 1668 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 4995)
	; 1668 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4996)
	; 1666 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 4998)
	; 1666 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 4999)
	; 1670 bytes
	delay_n 139
	delay_n (n-139)
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
	; 1670 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5002)
	; 1668 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 5002 && n < 5019)))
	; 1669..1675 bytes
	delay_n (n-2)
	nop
.elseif(n = 5004)
	; 1668 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5005)
	; 1672 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5007)
	; 1672 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5008)
	; 1670 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5010)
	; 1670 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5011)
	; 1674 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5013)
	; 1674 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5014)
	; 1672 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5016)
	; 1672 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5017)
	; 1676 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5019)
	; 1676 bytes
	delay_n 105
	delay_n (n-105)
.endif
.else
; 5020 to 5039
.if(n = 5020)
	; 1674 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 5020 && n < 5040)))
	; 1675..1683 bytes
	delay_n (n-2)
	nop
.elseif(n = 5022)
	; 1674 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5023)
	; 1678 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5025)
	; 1678 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5026)
	; 1676 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5028)
	; 1676 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5029)
	; 1680 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5031)
	; 1680 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5032)
	; 1678 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5034)
	; 1678 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5035)
	; 1682 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5037)
	; 1682 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5038)
	; 1680 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 5040) && (n < 5059))
; 5040 to 5058
.if(n = 5040)
	; 1680 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5041)
	; 1684 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 5041 && n < 5058)))
	; 1681..1689 bytes
	delay_n (n-2)
	nop
.elseif(n = 5043)
	; 1684 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5044)
	; 1682 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5046)
	; 1682 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5047)
	; 1686 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5049)
	; 1686 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5050)
	; 1684 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5052)
	; 1684 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5053)
	; 1688 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5055)
	; 1688 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5056)
	; 1686 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5058)
	; 1686 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 5059 to 5078
.if(n = 5059)
	; 1690 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 5059 && n < 5079)))
	; 1687..1695 bytes
	delay_n (n-2)
	nop
.elseif(n = 5061)
	; 1690 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5062)
	; 1688 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5064)
	; 1688 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5065)
	; 1692 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5067)
	; 1692 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5068)
	; 1690 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5070)
	; 1690 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5071)
	; 1694 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5073)
	; 1694 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1696 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
.if((n >= 5079) && (n < 5098))
; 5079 to 5097
.if(n = 5079)
	; 1696 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5080)
	; 1694 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 5080 && n < 5097)))
	; 1695..1701 bytes
	delay_n (n-2)
	nop
.elseif(n = 5082)
	; 1694 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5083)
	; 1698 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5085)
	; 1698 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5086)
	; 1696 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5088)
	; 1696 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5089)
	; 1700 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5091)
	; 1700 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5092)
	; 1698 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5094)
	; 1698 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5095)
	; 1702 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5097)
	; 1702 bytes
	delay_n 129
	delay_n (n-129)
.endif
.else
; 5098 to 5117
.if(n = 5098)
	; 1700 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 5098 && n < 5118)))
	; 1701..1709 bytes
	delay_n (n-2)
	nop
.elseif(n = 5100)
	; 1700 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5101)
	; 1704 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5103)
	; 1704 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5104)
	; 1702 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5106)
	; 1702 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5107)
	; 1706 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5109)
	; 1706 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5110)
	; 1704 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5112)
	; 1704 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5113)
	; 1708 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5115)
	; 1708 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5116)
	; 1706 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 5118) && (n < 5137))
; 5118 to 5136
.if(n = 5118)
	; 1706 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5119)
	; 1710 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 5119 && n < 5136)))
	; 1707..1715 bytes
	delay_n (n-2)
	nop
.elseif(n = 5121)
	; 1710 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5122)
	; 1708 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5124)
	; 1708 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5125)
	; 1712 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5127)
	; 1712 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1714 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5133)
	; 1714 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5134)
	; 1712 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5136)
	; 1712 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 5137 to 5156
.if(n = 5137)
	; 1716 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 5137 && n < 5157)))
	; 1713..1721 bytes
	delay_n (n-2)
	nop
.elseif(n = 5139)
	; 1716 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5140)
	; 1714 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5142)
	; 1714 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5143)
	; 1718 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5145)
	; 1718 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5146)
	; 1716 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5148)
	; 1716 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5149)
	; 1720 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5151)
	; 1720 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5152)
	; 1718 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5154)
	; 1718 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5155)
	; 1722 bytes
	delay_n 79
	delay_n (n-79)
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
	; 1722 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5158)
	; 1720 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 5158 && n < 5175)))
	; 1721..1727 bytes
	delay_n (n-2)
	nop
.elseif(n = 5160)
	; 1720 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5161)
	; 1724 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5163)
	; 1724 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5164)
	; 1722 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5166)
	; 1722 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5167)
	; 1726 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5169)
	; 1726 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5170)
	; 1724 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5172)
	; 1724 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5173)
	; 1728 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5175)
	; 1728 bytes
	delay_n 45
	delay_n (n-45)
.endif
.else
; 5176 to 5195
.if(n = 5176)
	; 1726 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 5176 && n < 5196)))
	; 1727..1735 bytes
	delay_n (n-2)
	nop
.elseif(n = 5178)
	; 1726 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5179)
	; 1730 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5181)
	; 1730 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1732 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5187)
	; 1732 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5188)
	; 1730 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5190)
	; 1730 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5191)
	; 1734 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5193)
	; 1734 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5194)
	; 1732 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 5196) && (n < 5215))
; 5196 to 5214
.if(n = 5196)
	; 1732 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5197)
	; 1736 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 5197 && n < 5214)))
	; 1733..1741 bytes
	delay_n (n-2)
	nop
.elseif(n = 5199)
	; 1736 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5200)
	; 1734 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5202)
	; 1734 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5203)
	; 1738 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5205)
	; 1738 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5206)
	; 1736 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5208)
	; 1736 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5209)
	; 1740 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5211)
	; 1740 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5212)
	; 1738 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5214)
	; 1738 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 5215 to 5234
.if(n = 5215)
	; 1742 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 5215 && n < 5235)))
	; 1739..1747 bytes
	delay_n (n-2)
	nop
.elseif(n = 5217)
	; 1742 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5218)
	; 1740 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5220)
	; 1740 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5221)
	; 1744 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5223)
	; 1744 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5224)
	; 1742 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5226)
	; 1742 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5227)
	; 1746 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5229)
	; 1746 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5230)
	; 1744 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5232)
	; 1744 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5233)
	; 1748 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
.if((n >= 5235) && (n < 5254))
; 5235 to 5253
.if(n = 5235)
	; 1748 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 5236)
	; 1746 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 5236 && n < 5253)))
	; 1747..1753 bytes
	delay_n (n-2)
	nop
.elseif(n = 5238)
	; 1746 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5239)
	; 1750 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5241)
	; 1750 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5242)
	; 1748 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5244)
	; 1748 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5245)
	; 1752 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5247)
	; 1752 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5248)
	; 1750 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5250)
	; 1750 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5251)
	; 1754 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5253)
	; 1754 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 5254 to 5273
.if(n = 5254)
	; 1752 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 5254 && n < 5274)))
	; 1753..1761 bytes
	delay_n (n-2)
	nop
.elseif(n = 5256)
	; 1752 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5257)
	; 1756 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5259)
	; 1756 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5260)
	; 1754 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5262)
	; 1754 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5263)
	; 1758 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5265)
	; 1758 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5266)
	; 1756 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5268)
	; 1756 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5269)
	; 1760 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5271)
	; 1760 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5272)
	; 1758 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 5274) && (n < 5293))
; 5274 to 5292
.if(n = 5274)
	; 1758 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5275)
	; 1762 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 5275 && n < 5292)))
	; 1759..1767 bytes
	delay_n (n-2)
	nop
.elseif(n = 5277)
	; 1762 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5278)
	; 1760 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5280)
	; 1760 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5281)
	; 1764 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5283)
	; 1764 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5284)
	; 1762 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5286)
	; 1762 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5287)
	; 1766 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5289)
	; 1766 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 5290)
	; 1764 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5292)
	; 1764 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 5293 to 5312
.if(n = 5293)
	; 1768 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 5293 && n < 5313)))
	; 1765..1773 bytes
	delay_n (n-2)
	nop
.elseif(n = 5295)
	; 1768 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5296)
	; 1766 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5298)
	; 1766 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5299)
	; 1770 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5301)
	; 1770 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5302)
	; 1768 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5304)
	; 1768 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5305)
	; 1772 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5307)
	; 1772 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5308)
	; 1770 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5310)
	; 1770 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5311)
	; 1774 bytes
	delay_n 127
	delay_n (n-127)
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
	; 1774 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5314)
	; 1772 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 5314 && n < 5331)))
	; 1773..1779 bytes
	delay_n (n-2)
	nop
.elseif(n = 5316)
	; 1772 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5317)
	; 1776 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5319)
	; 1776 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5320)
	; 1774 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5322)
	; 1774 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5323)
	; 1778 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5325)
	; 1778 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5326)
	; 1776 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5328)
	; 1776 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5329)
	; 1780 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5331)
	; 1780 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 5332 to 5351
.if(n = 5332)
	; 1778 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 5332 && n < 5352)))
	; 1779..1787 bytes
	delay_n (n-2)
	nop
.elseif(n = 5334)
	; 1778 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5335)
	; 1782 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5337)
	; 1782 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5338)
	; 1780 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5340)
	; 1780 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5341)
	; 1784 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5343)
	; 1784 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1786 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5349)
	; 1786 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5350)
	; 1784 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 5352) && (n < 5371))
; 5352 to 5370
.if(n = 5352)
	; 1784 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5353)
	; 1788 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 5353 && n < 5370)))
	; 1785..1793 bytes
	delay_n (n-2)
	nop
.elseif(n = 5355)
	; 1788 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5356)
	; 1786 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5358)
	; 1786 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5359)
	; 1790 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5361)
	; 1790 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5362)
	; 1788 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5364)
	; 1788 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5365)
	; 1792 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5367)
	; 1792 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5368)
	; 1790 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5370)
	; 1790 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 5371 to 5390
.if(n = 5371)
	; 1794 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 5371 && n < 5391)))
	; 1791..1799 bytes
	delay_n (n-2)
	nop
.elseif(n = 5373)
	; 1794 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5374)
	; 1792 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5376)
	; 1792 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5377)
	; 1796 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5379)
	; 1796 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5380)
	; 1794 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5382)
	; 1794 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5383)
	; 1798 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5385)
	; 1798 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5386)
	; 1796 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5388)
	; 1796 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5389)
	; 1800 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
.if((n >= 5391) && (n < 5410))
; 5391 to 5409
.if(n = 5391)
	; 1800 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5392)
	; 1798 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 5392 && n < 5409)))
	; 1799..1805 bytes
	delay_n (n-2)
	nop
.elseif(n = 5394)
	; 1798 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5395)
	; 1802 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5397)
	; 1802 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1804 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5403)
	; 1804 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5404)
	; 1802 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5406)
	; 1802 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5407)
	; 1806 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5409)
	; 1806 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 5410 to 5429
.if(n = 5410)
	; 1804 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 5410 && n < 5430)))
	; 1805..1813 bytes
	delay_n (n-2)
	nop
.elseif(n = 5412)
	; 1804 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5413)
	; 1808 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5415)
	; 1808 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5416)
	; 1806 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5418)
	; 1806 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5419)
	; 1810 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5421)
	; 1810 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5422)
	; 1808 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5424)
	; 1808 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5425)
	; 1812 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5427)
	; 1812 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5428)
	; 1810 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 5430) && (n < 5449))
; 5430 to 5448
.if(n = 5430)
	; 1810 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5431)
	; 1814 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 5431 && n < 5448)))
	; 1811..1819 bytes
	delay_n (n-2)
	nop
.elseif(n = 5433)
	; 1814 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5434)
	; 1812 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5436)
	; 1812 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5437)
	; 1816 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5439)
	; 1816 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5440)
	; 1814 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5442)
	; 1814 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5443)
	; 1818 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5445)
	; 1818 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5446)
	; 1816 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5448)
	; 1816 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 5449 to 5468
.if(n = 5449)
	; 1820 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 5449 && n < 5469)))
	; 1817..1825 bytes
	delay_n (n-2)
	nop
.elseif(n = 5451)
	; 1820 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1822 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5457)
	; 1822 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5458)
	; 1820 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5460)
	; 1820 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5461)
	; 1824 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5463)
	; 1824 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5464)
	; 1822 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5466)
	; 1822 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5467)
	; 1826 bytes
	delay_n 67
	delay_n (n-67)
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
	; 1826 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5470)
	; 1824 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 5470 && n < 5487)))
	; 1825..1831 bytes
	delay_n (n-2)
	nop
.elseif(n = 5472)
	; 1824 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5473)
	; 1828 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5475)
	; 1828 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5476)
	; 1826 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5478)
	; 1826 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5479)
	; 1830 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5481)
	; 1830 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5482)
	; 1828 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5484)
	; 1828 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5485)
	; 1832 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5487)
	; 1832 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 5488 to 5507
.if(n = 5488)
	; 1830 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 5488 && n < 5508)))
	; 1831..1839 bytes
	delay_n (n-2)
	nop
.elseif(n = 5490)
	; 1830 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5491)
	; 1834 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5493)
	; 1834 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5494)
	; 1832 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5496)
	; 1832 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5497)
	; 1836 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5499)
	; 1836 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5500)
	; 1834 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5502)
	; 1834 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5503)
	; 1838 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5505)
	; 1838 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 5506)
	; 1836 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 5508) && (n < 5527))
; 5508 to 5526
.if(n = 5508)
	; 1836 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5509)
	; 1840 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 5509 && n < 5526)))
	; 1837..1845 bytes
	delay_n (n-2)
	nop
.elseif(n = 5511)
	; 1840 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5512)
	; 1838 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5514)
	; 1838 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5515)
	; 1842 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5517)
	; 1842 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5518)
	; 1840 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5520)
	; 1840 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5521)
	; 1844 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5523)
	; 1844 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5524)
	; 1842 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5526)
	; 1842 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 5527 to 5546
.if(n = 5527)
	; 1846 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 5527 && n < 5547)))
	; 1843..1851 bytes
	delay_n (n-2)
	nop
.elseif(n = 5529)
	; 1846 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5530)
	; 1844 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5532)
	; 1844 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5533)
	; 1848 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5535)
	; 1848 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5536)
	; 1846 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5538)
	; 1846 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5539)
	; 1850 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5541)
	; 1850 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5542)
	; 1848 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5544)
	; 1848 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5545)
	; 1852 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 5547) && (n < 5586))
.if((n >= 5547) && (n < 5566))
; 5547 to 5565
.if(n = 5547)
	; 1852 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5548)
	; 1850 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 5548 && n < 5565)))
	; 1851..1857 bytes
	delay_n (n-2)
	nop
.elseif(n = 5550)
	; 1850 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5551)
	; 1854 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5553)
	; 1854 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5554)
	; 1852 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5556)
	; 1852 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5557)
	; 1856 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5559)
	; 1856 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1858 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5565)
	; 1858 bytes
	delay_n 57
	delay_n (n-57)
.endif
.else
; 5566 to 5585
.if(n = 5566)
	; 1856 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 5566 && n < 5586)))
	; 1857..1865 bytes
	delay_n (n-2)
	nop
.elseif(n = 5568)
	; 1856 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5569)
	; 1860 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5571)
	; 1860 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5572)
	; 1858 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5574)
	; 1858 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5575)
	; 1862 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5577)
	; 1862 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5578)
	; 1860 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5580)
	; 1860 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5581)
	; 1864 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5583)
	; 1864 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5584)
	; 1862 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 5586) && (n < 5606))
; 5586 to 5605
.if(n = 5586)
	; 1862 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5587)
	; 1866 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 5587 && n < 5604)))
	; 1863..1871 bytes
	delay_n (n-2)
	nop
.elseif(n = 5589)
	; 1866 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5590)
	; 1864 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5592)
	; 1864 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5593)
	; 1868 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5595)
	; 1868 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5596)
	; 1866 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5598)
	; 1866 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5599)
	; 1870 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5601)
	; 1870 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5602)
	; 1868 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5604)
	; 1868 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5605)
	; 1872 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 5606 to 5625
.if((n .mod 3 = 2) && ((n > 5605 && n < 5625)))
	; 1869..1877 bytes
	delay_n (n-2)
	nop
.elseif(n = 5607)
	; 1872 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5608)
	; 1870 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5610)
	; 1870 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5611)
	; 1874 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5613)
	; 1874 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1876 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5619)
	; 1876 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5620)
	; 1874 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5622)
	; 1874 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5623)
	; 1878 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5625)
	; 1878 bytes
	delay_n 117
	delay_n (n-117)
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
	; 1876 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 5626 && n < 5643)))
	; 1877..1883 bytes
	delay_n (n-2)
	nop
.elseif(n = 5628)
	; 1876 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5629)
	; 1880 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5631)
	; 1880 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5632)
	; 1878 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5634)
	; 1878 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5635)
	; 1882 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5637)
	; 1882 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5638)
	; 1880 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5640)
	; 1880 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5641)
	; 1884 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5643)
	; 1884 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5644)
	; 1882 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 5645 to 5664
.if((n .mod 3 = 2) && ((n > 5644 && n < 5664)))
	; 1883..1891 bytes
	delay_n (n-2)
	nop
.elseif(n = 5646)
	; 1882 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5647)
	; 1886 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5649)
	; 1886 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5650)
	; 1884 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5652)
	; 1884 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5653)
	; 1888 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5655)
	; 1888 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5656)
	; 1886 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5658)
	; 1886 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5659)
	; 1890 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5661)
	; 1890 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5662)
	; 1888 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5664)
	; 1888 bytes
	delay_n 48
	delay_n (n-48)
.endif
.endif
.else
.if((n >= 5665) && (n < 5684))
; 5665 to 5683
.if(n = 5665)
	; 1892 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 5665 && n < 5682)))
	; 1889..1897 bytes
	delay_n (n-2)
	nop
.elseif(n = 5667)
	; 1892 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1894 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5673)
	; 1894 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5674)
	; 1892 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5676)
	; 1892 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5677)
	; 1896 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5679)
	; 1896 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5680)
	; 1894 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5682)
	; 1894 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5683)
	; 1898 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 5684 to 5703
.if((n .mod 3 = 2) && ((n > 5683 && n < 5703)))
	; 1895..1903 bytes
	delay_n (n-2)
	nop
.elseif(n = 5685)
	; 1898 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5686)
	; 1896 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5688)
	; 1896 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5689)
	; 1900 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5691)
	; 1900 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5692)
	; 1898 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5694)
	; 1898 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5695)
	; 1902 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5697)
	; 1902 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5698)
	; 1900 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5700)
	; 1900 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5701)
	; 1904 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5703)
	; 1904 bytes
	delay_n 141
	delay_n (n-141)
.endif
.endif
.endif
.else
.if((n >= 5704) && (n < 5743))
.if((n >= 5704) && (n < 5723))
; 5704 to 5722
.if(n = 5704)
	; 1902 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 5704 && n < 5721)))
	; 1903..1909 bytes
	delay_n (n-2)
	nop
.elseif(n = 5706)
	; 1902 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5707)
	; 1906 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5709)
	; 1906 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5710)
	; 1904 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5712)
	; 1904 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5713)
	; 1908 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5715)
	; 1908 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5716)
	; 1906 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5718)
	; 1906 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5719)
	; 1910 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5721)
	; 1910 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 5722)
	; 1908 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 5723 to 5742
.if((n .mod 3 = 2) && ((n > 5722 && n < 5742)))
	; 1909..1917 bytes
	delay_n (n-2)
	nop
.elseif(n = 5724)
	; 1908 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5725)
	; 1912 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5727)
	; 1912 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5728)
	; 1910 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5730)
	; 1910 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5731)
	; 1914 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5733)
	; 1914 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5734)
	; 1912 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5736)
	; 1912 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5737)
	; 1916 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5739)
	; 1916 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5740)
	; 1914 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5742)
	; 1914 bytes
	delay_n 72
	delay_n (n-72)
.endif
.endif
.else
.if((n >= 5743) && (n < 5762))
; 5743 to 5761
.if(n = 5743)
	; 1918 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 5743 && n < 5760)))
	; 1915..1923 bytes
	delay_n (n-2)
	nop
.elseif(n = 5745)
	; 1918 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5746)
	; 1916 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5748)
	; 1916 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5749)
	; 1920 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5751)
	; 1920 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5752)
	; 1918 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5754)
	; 1918 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5755)
	; 1922 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5757)
	; 1922 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5758)
	; 1920 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5760)
	; 1920 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5761)
	; 1924 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 5762 to 5781
.if((n .mod 3 = 2) && ((n > 5761 && n < 5781)))
	; 1921..1929 bytes
	delay_n (n-2)
	nop
.elseif(n = 5763)
	; 1924 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5764)
	; 1922 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5766)
	; 1922 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5767)
	; 1926 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5769)
	; 1926 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5770)
	; 1924 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5772)
	; 1924 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5773)
	; 1928 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5775)
	; 1928 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1930 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5781)
	; 1930 bytes
	delay_n 57
	delay_n (n-57)
.endif
.endif
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
.if((n >= 5782) && (n < 5821))
.if((n >= 5782) && (n < 5801))
; 5782 to 5800
.if(n = 5782)
	; 1928 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 5782 && n < 5799)))
	; 1929..1935 bytes
	delay_n (n-2)
	nop
.elseif(n = 5784)
	; 1928 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5785)
	; 1932 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5787)
	; 1932 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5788)
	; 1930 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5790)
	; 1930 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5791)
	; 1934 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5793)
	; 1934 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5794)
	; 1932 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5796)
	; 1932 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5797)
	; 1936 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5799)
	; 1936 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5800)
	; 1934 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 5801 to 5820
.if((n .mod 3 = 2) && ((n > 5800 && n < 5820)))
	; 1935..1943 bytes
	delay_n (n-2)
	nop
.elseif(n = 5802)
	; 1934 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5803)
	; 1938 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5805)
	; 1938 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5806)
	; 1936 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5808)
	; 1936 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5809)
	; 1940 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5811)
	; 1940 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5812)
	; 1938 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5814)
	; 1938 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5815)
	; 1942 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5817)
	; 1942 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5818)
	; 1940 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5820)
	; 1940 bytes
	delay_n 96
	delay_n (n-96)
.endif
.endif
.else
.if((n >= 5821) && (n < 5840))
; 5821 to 5839
.if(n = 5821)
	; 1944 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 5821 && n < 5838)))
	; 1941..1949 bytes
	delay_n (n-2)
	nop
.elseif(n = 5823)
	; 1944 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5824)
	; 1942 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5826)
	; 1942 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5827)
	; 1946 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5829)
	; 1946 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1948 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5835)
	; 1948 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5836)
	; 1946 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5838)
	; 1946 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5839)
	; 1950 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 5840 to 5859
.if((n .mod 3 = 2) && ((n > 5839 && n < 5859)))
	; 1947..1955 bytes
	delay_n (n-2)
	nop
.elseif(n = 5841)
	; 1950 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5842)
	; 1948 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5844)
	; 1948 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5845)
	; 1952 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5847)
	; 1952 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5848)
	; 1950 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5850)
	; 1950 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5851)
	; 1954 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5853)
	; 1954 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5854)
	; 1952 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5856)
	; 1952 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5857)
	; 1956 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5859)
	; 1956 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.endif
.else
.if((n >= 5860) && (n < 5899))
.if((n >= 5860) && (n < 5879))
; 5860 to 5878
.if(n = 5860)
	; 1954 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 5860 && n < 5877)))
	; 1955..1961 bytes
	delay_n (n-2)
	nop
.elseif(n = 5862)
	; 1954 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5863)
	; 1958 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5865)
	; 1958 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5866)
	; 1956 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5868)
	; 1956 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5869)
	; 1960 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5871)
	; 1960 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5872)
	; 1958 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5874)
	; 1958 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5875)
	; 1962 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5877)
	; 1962 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5878)
	; 1960 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 5879 to 5898
.if((n .mod 3 = 2) && ((n > 5878 && n < 5898)))
	; 1961..1969 bytes
	delay_n (n-2)
	nop
.elseif(n = 5880)
	; 1960 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5881)
	; 1964 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5883)
	; 1964 bytes
	delay_n 105
	delay_n (n-105)
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
	; 1966 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5889)
	; 1966 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5890)
	; 1964 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5892)
	; 1964 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5893)
	; 1968 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5895)
	; 1968 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5896)
	; 1966 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5898)
	; 1966 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 5899) && (n < 5918))
; 5899 to 5917
.if(n = 5899)
	; 1970 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 5899 && n < 5916)))
	; 1967..1975 bytes
	delay_n (n-2)
	nop
.elseif(n = 5901)
	; 1970 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5902)
	; 1968 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 5904)
	; 1968 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5905)
	; 1972 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5907)
	; 1972 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5908)
	; 1970 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5910)
	; 1970 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5911)
	; 1974 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5913)
	; 1974 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5914)
	; 1972 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5916)
	; 1972 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5917)
	; 1976 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 5918 to 5937
.if((n .mod 3 = 2) && ((n > 5917 && n < 5937)))
	; 1973..1981 bytes
	delay_n (n-2)
	nop
.elseif(n = 5919)
	; 1976 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5920)
	; 1974 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5922)
	; 1974 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5923)
	; 1978 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5925)
	; 1978 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5926)
	; 1976 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5928)
	; 1976 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5929)
	; 1980 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5931)
	; 1980 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5932)
	; 1978 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5934)
	; 1978 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5935)
	; 1982 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5937)
	; 1982 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
.if((n >= 5938) && (n < 5977))
.if((n >= 5938) && (n < 5957))
; 5938 to 5956
.if(n = 5938)
	; 1980 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 5938 && n < 5955)))
	; 1981..1987 bytes
	delay_n (n-2)
	nop
.elseif(n = 5940)
	; 1980 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 5941)
	; 1984 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 5943)
	; 1984 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5944)
	; 1982 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 5946)
	; 1982 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 5947)
	; 1986 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 5949)
	; 1986 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 5950)
	; 1984 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5952)
	; 1984 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5953)
	; 1988 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 5955)
	; 1988 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 5956)
	; 1986 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 5957 to 5976
.if((n .mod 3 = 2) && ((n > 5956 && n < 5976)))
	; 1987..1995 bytes
	delay_n (n-2)
	nop
.elseif(n = 5958)
	; 1986 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 5959)
	; 1990 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 5961)
	; 1990 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 5962)
	; 1988 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 5964)
	; 1988 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 5965)
	; 1992 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 5967)
	; 1992 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5968)
	; 1990 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 5970)
	; 1990 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 5971)
	; 1994 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 5973)
	; 1994 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5974)
	; 1992 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5976)
	; 1992 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.else
.if((n >= 5977) && (n < 5996))
; 5977 to 5995
.if(n = 5977)
	; 1996 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 5977 && n < 5994)))
	; 1993..2001 bytes
	delay_n (n-2)
	nop
.elseif(n = 5979)
	; 1996 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 5980)
	; 1994 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 5982)
	; 1994 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5983)
	; 1998 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5985)
	; 1998 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 5986)
	; 1996 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5988)
	; 1996 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 5989)
	; 2000 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 5991)
	; 2000 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2002 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 5996 to 6015
.if((n .mod 3 = 2) && ((n > 5995 && n < 6015)))
	; 1999..2007 bytes
	delay_n (n-2)
	nop
.elseif(n = 5997)
	; 2002 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 5998)
	; 2000 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6000)
	; 2000 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6001)
	; 2004 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6003)
	; 2004 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6004)
	; 2002 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6006)
	; 2002 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6007)
	; 2006 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6009)
	; 2006 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6010)
	; 2004 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6012)
	; 2004 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6013)
	; 2008 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6015)
	; 2008 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.endif
.else
.if((n >= 6016) && (n < 6055))
.if((n >= 6016) && (n < 6035))
; 6016 to 6034
.if(n = 6016)
	; 2006 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 6016 && n < 6033)))
	; 2007..2013 bytes
	delay_n (n-2)
	nop
.elseif(n = 6018)
	; 2006 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6019)
	; 2010 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6021)
	; 2010 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6022)
	; 2008 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6024)
	; 2008 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6025)
	; 2012 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6027)
	; 2012 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6028)
	; 2010 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6030)
	; 2010 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6031)
	; 2014 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6033)
	; 2014 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6034)
	; 2012 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 6035 to 6054
.if((n .mod 3 = 2) && ((n > 6034 && n < 6054)))
	; 2013..2021 bytes
	delay_n (n-2)
	nop
.elseif(n = 6036)
	; 2012 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6037)
	; 2016 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6039)
	; 2016 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6040)
	; 2014 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6042)
	; 2014 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6043)
	; 2018 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6045)
	; 2018 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2020 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6051)
	; 2020 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6052)
	; 2018 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6054)
	; 2018 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.else
.if((n >= 6055) && (n < 6074))
; 6055 to 6073
.if(n = 6055)
	; 2022 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 6055 && n < 6072)))
	; 2019..2027 bytes
	delay_n (n-2)
	nop
.elseif(n = 6057)
	; 2022 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6058)
	; 2020 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6060)
	; 2020 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6061)
	; 2024 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6063)
	; 2024 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6064)
	; 2022 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6066)
	; 2022 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6067)
	; 2026 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6069)
	; 2026 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6070)
	; 2024 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6072)
	; 2024 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6073)
	; 2028 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 6074 to 6093
.if((n .mod 3 = 2) && ((n > 6073 && n < 6093)))
	; 2025..2033 bytes
	delay_n (n-2)
	nop
.elseif(n = 6075)
	; 2028 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6076)
	; 2026 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6078)
	; 2026 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6079)
	; 2030 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6081)
	; 2030 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6082)
	; 2028 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6084)
	; 2028 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6085)
	; 2032 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6087)
	; 2032 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6088)
	; 2030 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6090)
	; 2030 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6091)
	; 2034 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6093)
	; 2034 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
.if((n >= 6094) && (n < 6113))
; 6094 to 6112
.if(n = 6094)
	; 2032 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 6094 && n < 6111)))
	; 2033..2039 bytes
	delay_n (n-2)
	nop
.elseif(n = 6096)
	; 2032 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6097)
	; 2036 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6099)
	; 2036 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2038 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6105)
	; 2038 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6106)
	; 2036 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6108)
	; 2036 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6109)
	; 2040 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6111)
	; 2040 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6112)
	; 2038 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 6113 to 6132
.if((n .mod 3 = 2) && ((n > 6112 && n < 6132)))
	; 2039..2047 bytes
	delay_n (n-2)
	nop
.elseif(n = 6114)
	; 2038 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6115)
	; 2042 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6117)
	; 2042 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6118)
	; 2040 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6120)
	; 2040 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6121)
	; 2044 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6123)
	; 2044 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6124)
	; 2042 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6126)
	; 2042 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6127)
	; 2046 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6129)
	; 2046 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6130)
	; 2044 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6132)
	; 2044 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.else
.if((n >= 6133) && (n < 6152))
; 6133 to 6151
.if(n = 6133)
	; 2048 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 6133 && n < 6150)))
	; 2045..2053 bytes
	delay_n (n-2)
	nop
.elseif(n = 6135)
	; 2048 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6136)
	; 2046 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6138)
	; 2046 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6139)
	; 2050 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6141)
	; 2050 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6142)
	; 2048 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6144)
	; 2048 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6145)
	; 2052 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6147)
	; 2052 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6148)
	; 2050 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6150)
	; 2050 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6151)
	; 2054 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 6152 to 6171
.if((n .mod 3 = 2) && ((n > 6151 && n < 6171)))
	; 2051..2059 bytes
	delay_n (n-2)
	nop
.elseif(n = 6153)
	; 2054 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2056 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6159)
	; 2056 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6160)
	; 2054 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6162)
	; 2054 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6163)
	; 2058 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6165)
	; 2058 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6166)
	; 2056 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6168)
	; 2056 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6169)
	; 2060 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6171)
	; 2060 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.endif
.else
.if((n >= 6172) && (n < 6211))
.if((n >= 6172) && (n < 6191))
; 6172 to 6190
.if(n = 6172)
	; 2058 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 6172 && n < 6189)))
	; 2059..2065 bytes
	delay_n (n-2)
	nop
.elseif(n = 6174)
	; 2058 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6175)
	; 2062 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6177)
	; 2062 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6178)
	; 2060 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6180)
	; 2060 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6181)
	; 2064 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6183)
	; 2064 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6184)
	; 2062 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6186)
	; 2062 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6187)
	; 2066 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6189)
	; 2066 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6190)
	; 2064 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 6191 to 6210
.if((n .mod 3 = 2) && ((n > 6190 && n < 6210)))
	; 2065..2073 bytes
	delay_n (n-2)
	nop
.elseif(n = 6192)
	; 2064 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6193)
	; 2068 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6195)
	; 2068 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6196)
	; 2066 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6198)
	; 2066 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6199)
	; 2070 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6201)
	; 2070 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6202)
	; 2068 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6204)
	; 2068 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6205)
	; 2072 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6207)
	; 2072 bytes
	delay_n 105
	delay_n (n-105)
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
.endif
.else
.if((n >= 6211) && (n < 6231))
; 6211 to 6230
.if(n = 6211)
	; 2074 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 6211 && n < 6231)))
	; 2071..2079 bytes
	delay_n (n-2)
	nop
.elseif(n = 6213)
	; 2074 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6214)
	; 2072 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6216)
	; 2072 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6217)
	; 2076 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6219)
	; 2076 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6220)
	; 2074 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6222)
	; 2074 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6223)
	; 2078 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6225)
	; 2078 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6226)
	; 2076 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6228)
	; 2076 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6229)
	; 2080 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 6231 to 6250
.if(n = 6231)
	; 2080 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6232)
	; 2078 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 6232 && n < 6249)))
	; 2079..2085 bytes
	delay_n (n-2)
	nop
.elseif(n = 6234)
	; 2078 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6235)
	; 2082 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6237)
	; 2082 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6238)
	; 2080 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6240)
	; 2080 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6241)
	; 2084 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6243)
	; 2084 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6244)
	; 2082 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6246)
	; 2082 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6247)
	; 2086 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6249)
	; 2086 bytes
	delay_n 93
	delay_n (n-93)
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
.endif
.else
.if((n >= 6251) && (n < 6876))
.if((n >= 6251) && (n < 6563))
.if((n >= 6251) && (n < 6407))
.if((n >= 6251) && (n < 6329))
.if((n >= 6251) && (n < 6290))
.if((n >= 6251) && (n < 6270))
; 6251 to 6269
.if((n .mod 3 = 2) && ((n > 6250 && n < 6270)))
	; 2085..2093 bytes
	delay_n (n-2)
	nop
.elseif(n = 6252)
	; 2084 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6253)
	; 2088 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6255)
	; 2088 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6256)
	; 2086 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6258)
	; 2086 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6259)
	; 2090 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6261)
	; 2090 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2092 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6267)
	; 2092 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6268)
	; 2090 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 6270 to 6289
.if(n = 6270)
	; 2090 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6271)
	; 2094 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 6271 && n < 6288)))
	; 2091..2099 bytes
	delay_n (n-2)
	nop
.elseif(n = 6273)
	; 2094 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6274)
	; 2092 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6276)
	; 2092 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6277)
	; 2096 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6279)
	; 2096 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6280)
	; 2094 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6282)
	; 2094 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6283)
	; 2098 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6285)
	; 2098 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6286)
	; 2096 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6288)
	; 2096 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6289)
	; 2100 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 6290) && (n < 6309))
; 6290 to 6308
.if((n .mod 3 = 2) && ((n > 6289 && n < 6309)))
	; 2097..2105 bytes
	delay_n (n-2)
	nop
.elseif(n = 6291)
	; 2100 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6292)
	; 2098 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6294)
	; 2098 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6295)
	; 2102 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6297)
	; 2102 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6298)
	; 2100 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6300)
	; 2100 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6301)
	; 2104 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6303)
	; 2104 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6304)
	; 2102 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6306)
	; 2102 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6307)
	; 2106 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 6309 to 6328
.if(n = 6309)
	; 2106 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6310)
	; 2104 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 6310 && n < 6327)))
	; 2105..2111 bytes
	delay_n (n-2)
	nop
.elseif(n = 6312)
	; 2104 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6313)
	; 2108 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6315)
	; 2108 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2110 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6321)
	; 2110 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6322)
	; 2108 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6324)
	; 2108 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6325)
	; 2112 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6327)
	; 2112 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6328)
	; 2110 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.else
.if((n >= 6329) && (n < 6368))
.if((n >= 6329) && (n < 6348))
; 6329 to 6347
.if((n .mod 3 = 2) && ((n > 6328 && n < 6348)))
	; 2111..2119 bytes
	delay_n (n-2)
	nop
.elseif(n = 6330)
	; 2110 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6331)
	; 2114 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6333)
	; 2114 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6334)
	; 2112 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6336)
	; 2112 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6337)
	; 2116 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6339)
	; 2116 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6340)
	; 2114 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6342)
	; 2114 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6343)
	; 2118 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6345)
	; 2118 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6346)
	; 2116 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 6348 to 6367
.if(n = 6348)
	; 2116 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6349)
	; 2120 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 6349 && n < 6366)))
	; 2117..2125 bytes
	delay_n (n-2)
	nop
.elseif(n = 6351)
	; 2120 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6352)
	; 2118 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6354)
	; 2118 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6355)
	; 2122 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6357)
	; 2122 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6358)
	; 2120 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6360)
	; 2120 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6361)
	; 2124 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6363)
	; 2124 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6364)
	; 2122 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6366)
	; 2122 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6367)
	; 2126 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 6368) && (n < 6387))
; 6368 to 6386
.if((n .mod 3 = 2) && ((n > 6367 && n < 6387)))
	; 2123..2131 bytes
	delay_n (n-2)
	nop
.elseif(n = 6369)
	; 2126 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2128 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6375)
	; 2128 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6376)
	; 2126 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6378)
	; 2126 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6379)
	; 2130 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6381)
	; 2130 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6382)
	; 2128 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6384)
	; 2128 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6385)
	; 2132 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 6387 to 6406
.if(n = 6387)
	; 2132 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6388)
	; 2130 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 6388 && n < 6405)))
	; 2131..2137 bytes
	delay_n (n-2)
	nop
.elseif(n = 6390)
	; 2130 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6391)
	; 2134 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6393)
	; 2134 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6394)
	; 2132 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6396)
	; 2132 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6397)
	; 2136 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6399)
	; 2136 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6400)
	; 2134 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6402)
	; 2134 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6403)
	; 2138 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6405)
	; 2138 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6406)
	; 2136 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
.if((n >= 6407) && (n < 6426))
; 6407 to 6425
.if((n .mod 3 = 2) && ((n > 6406 && n < 6426)))
	; 2137..2145 bytes
	delay_n (n-2)
	nop
.elseif(n = 6408)
	; 2136 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6409)
	; 2140 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6411)
	; 2140 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6412)
	; 2138 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6414)
	; 2138 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6415)
	; 2142 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6417)
	; 2142 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6418)
	; 2140 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6420)
	; 2140 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6421)
	; 2144 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6423)
	; 2144 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 6424)
	; 2142 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 6426 to 6445
.if(n = 6426)
	; 2142 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6427)
	; 2146 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 6427 && n < 6444)))
	; 2143..2151 bytes
	delay_n (n-2)
	nop
.elseif(n = 6429)
	; 2146 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6430)
	; 2144 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6432)
	; 2144 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6433)
	; 2148 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6435)
	; 2148 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6436)
	; 2146 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6438)
	; 2146 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6439)
	; 2150 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6441)
	; 2150 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6442)
	; 2148 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6444)
	; 2148 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6445)
	; 2152 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.else
.if((n >= 6446) && (n < 6465))
; 6446 to 6464
.if((n .mod 3 = 2) && ((n > 6445 && n < 6465)))
	; 2149..2157 bytes
	delay_n (n-2)
	nop
.elseif(n = 6447)
	; 2152 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6448)
	; 2150 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6450)
	; 2150 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6451)
	; 2154 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6453)
	; 2154 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6454)
	; 2152 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6456)
	; 2152 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6457)
	; 2156 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6459)
	; 2156 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6460)
	; 2154 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6462)
	; 2154 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6463)
	; 2158 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 6465 to 6484
.if(n = 6465)
	; 2158 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6466)
	; 2156 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 6466 && n < 6483)))
	; 2157..2163 bytes
	delay_n (n-2)
	nop
.elseif(n = 6468)
	; 2156 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6469)
	; 2160 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6471)
	; 2160 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6472)
	; 2158 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6474)
	; 2158 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6475)
	; 2162 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6477)
	; 2162 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2164 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6483)
	; 2164 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6484)
	; 2162 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.else
.if((n >= 6485) && (n < 6524))
.if((n >= 6485) && (n < 6504))
; 6485 to 6503
.if((n .mod 3 = 2) && ((n > 6484 && n < 6504)))
	; 2163..2171 bytes
	delay_n (n-2)
	nop
.elseif(n = 6486)
	; 2162 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6487)
	; 2166 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6489)
	; 2166 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6490)
	; 2164 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6492)
	; 2164 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6493)
	; 2168 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6495)
	; 2168 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6496)
	; 2166 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6498)
	; 2166 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6499)
	; 2170 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6501)
	; 2170 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6502)
	; 2168 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 6504 to 6523
.if(n = 6504)
	; 2168 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6505)
	; 2172 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 6505 && n < 6522)))
	; 2169..2177 bytes
	delay_n (n-2)
	nop
.elseif(n = 6507)
	; 2172 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6508)
	; 2170 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6510)
	; 2170 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6511)
	; 2174 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6513)
	; 2174 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6514)
	; 2172 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6516)
	; 2172 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6517)
	; 2176 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6519)
	; 2176 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6520)
	; 2174 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6522)
	; 2174 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6523)
	; 2178 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 6524) && (n < 6543))
; 6524 to 6542
.if((n .mod 3 = 2) && ((n > 6523 && n < 6543)))
	; 2175..2183 bytes
	delay_n (n-2)
	nop
.elseif(n = 6525)
	; 2178 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6526)
	; 2176 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6528)
	; 2176 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6529)
	; 2180 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6531)
	; 2180 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2182 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6537)
	; 2182 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6538)
	; 2180 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6540)
	; 2180 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6541)
	; 2184 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 6543 to 6562
.if(n = 6543)
	; 2184 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6544)
	; 2182 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 6544 && n < 6561)))
	; 2183..2189 bytes
	delay_n (n-2)
	nop
.elseif(n = 6546)
	; 2182 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6547)
	; 2186 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6549)
	; 2186 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6550)
	; 2184 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6552)
	; 2184 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6553)
	; 2188 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6555)
	; 2188 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6556)
	; 2186 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6558)
	; 2186 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6559)
	; 2190 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6561)
	; 2190 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6562)
	; 2188 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
.if((n >= 6563) && (n < 6602))
.if((n >= 6563) && (n < 6582))
; 6563 to 6581
.if((n .mod 3 = 2) && ((n > 6562 && n < 6582)))
	; 2189..2197 bytes
	delay_n (n-2)
	nop
.elseif(n = 6564)
	; 2188 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6565)
	; 2192 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6567)
	; 2192 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6568)
	; 2190 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6570)
	; 2190 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6571)
	; 2194 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6573)
	; 2194 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6574)
	; 2192 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6576)
	; 2192 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6577)
	; 2196 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6579)
	; 2196 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6580)
	; 2194 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 6582 to 6601
.if(n = 6582)
	; 2194 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6583)
	; 2198 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 6583 && n < 6600)))
	; 2195..2203 bytes
	delay_n (n-2)
	nop
.elseif(n = 6585)
	; 2198 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2200 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6591)
	; 2200 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6592)
	; 2198 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6594)
	; 2198 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6595)
	; 2202 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6597)
	; 2202 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6598)
	; 2200 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6600)
	; 2200 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6601)
	; 2204 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 6602) && (n < 6621))
; 6602 to 6620
.if((n .mod 3 = 2) && ((n > 6601 && n < 6621)))
	; 2201..2209 bytes
	delay_n (n-2)
	nop
.elseif(n = 6603)
	; 2204 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6604)
	; 2202 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6606)
	; 2202 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6607)
	; 2206 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6609)
	; 2206 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6610)
	; 2204 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6612)
	; 2204 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6613)
	; 2208 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6615)
	; 2208 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6616)
	; 2206 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6618)
	; 2206 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6619)
	; 2210 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 6621 to 6640
.if(n = 6621)
	; 2210 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6622)
	; 2208 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 6622 && n < 6639)))
	; 2209..2215 bytes
	delay_n (n-2)
	nop
.elseif(n = 6624)
	; 2208 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6625)
	; 2212 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6627)
	; 2212 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6628)
	; 2210 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6630)
	; 2210 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6631)
	; 2214 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6633)
	; 2214 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6634)
	; 2212 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6636)
	; 2212 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6637)
	; 2216 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6639)
	; 2216 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 6640)
	; 2214 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 6641) && (n < 6680))
.if((n >= 6641) && (n < 6660))
; 6641 to 6659
.if((n .mod 3 = 2) && ((n > 6640 && n < 6660)))
	; 2215..2223 bytes
	delay_n (n-2)
	nop
.elseif(n = 6642)
	; 2214 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6643)
	; 2218 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6645)
	; 2218 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6646)
	; 2216 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6648)
	; 2216 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6649)
	; 2220 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6651)
	; 2220 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6652)
	; 2218 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6654)
	; 2218 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6655)
	; 2222 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6657)
	; 2222 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6658)
	; 2220 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 6660 to 6679
.if(n = 6660)
	; 2220 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6661)
	; 2224 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 6661 && n < 6678)))
	; 2221..2229 bytes
	delay_n (n-2)
	nop
.elseif(n = 6663)
	; 2224 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6664)
	; 2222 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6666)
	; 2222 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6667)
	; 2226 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6669)
	; 2226 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6670)
	; 2224 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6672)
	; 2224 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6673)
	; 2228 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6675)
	; 2228 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6676)
	; 2226 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6678)
	; 2226 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6679)
	; 2230 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 6680) && (n < 6699))
; 6680 to 6698
.if((n .mod 3 = 2) && ((n > 6679 && n < 6699)))
	; 2227..2235 bytes
	delay_n (n-2)
	nop
.elseif(n = 6681)
	; 2230 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6682)
	; 2228 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6684)
	; 2228 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6685)
	; 2232 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6687)
	; 2232 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6688)
	; 2230 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6690)
	; 2230 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6691)
	; 2234 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6693)
	; 2234 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2236 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 6699 to 6718
.if(n = 6699)
	; 2236 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6700)
	; 2234 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 6700 && n < 6717)))
	; 2235..2241 bytes
	delay_n (n-2)
	nop
.elseif(n = 6702)
	; 2234 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6703)
	; 2238 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6705)
	; 2238 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6706)
	; 2236 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6708)
	; 2236 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6709)
	; 2240 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6711)
	; 2240 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6712)
	; 2238 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6714)
	; 2238 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6715)
	; 2242 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6717)
	; 2242 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6718)
	; 2240 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.endif
.else
.if((n >= 6719) && (n < 6797))
.if((n >= 6719) && (n < 6758))
.if((n >= 6719) && (n < 6738))
; 6719 to 6737
.if((n .mod 3 = 2) && ((n > 6718 && n < 6738)))
	; 2241..2249 bytes
	delay_n (n-2)
	nop
.elseif(n = 6720)
	; 2240 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6721)
	; 2244 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6723)
	; 2244 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6724)
	; 2242 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6726)
	; 2242 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6727)
	; 2246 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6729)
	; 2246 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6730)
	; 2244 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6732)
	; 2244 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6733)
	; 2248 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6735)
	; 2248 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6736)
	; 2246 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 6738 to 6757
.if(n = 6738)
	; 2246 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6739)
	; 2250 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 6739 && n < 6756)))
	; 2247..2255 bytes
	delay_n (n-2)
	nop
.elseif(n = 6741)
	; 2250 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6742)
	; 2248 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6744)
	; 2248 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6745)
	; 2252 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6747)
	; 2252 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2254 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6753)
	; 2254 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6754)
	; 2252 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6756)
	; 2252 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6757)
	; 2256 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.else
.if((n >= 6758) && (n < 6777))
; 6758 to 6776
.if((n .mod 3 = 2) && ((n > 6757 && n < 6777)))
	; 2253..2261 bytes
	delay_n (n-2)
	nop
.elseif(n = 6759)
	; 2256 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6760)
	; 2254 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6762)
	; 2254 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6763)
	; 2258 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6765)
	; 2258 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6766)
	; 2256 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6768)
	; 2256 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6769)
	; 2260 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6771)
	; 2260 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6772)
	; 2258 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6774)
	; 2258 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6775)
	; 2262 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 6777 to 6796
.if(n = 6777)
	; 2262 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6778)
	; 2260 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 6778 && n < 6795)))
	; 2261..2267 bytes
	delay_n (n-2)
	nop
.elseif(n = 6780)
	; 2260 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6781)
	; 2264 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6783)
	; 2264 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6784)
	; 2262 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6786)
	; 2262 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6787)
	; 2266 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6789)
	; 2266 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6790)
	; 2264 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6792)
	; 2264 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6793)
	; 2268 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6795)
	; 2268 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6796)
	; 2266 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.else
.if((n >= 6797) && (n < 6836))
.if((n >= 6797) && (n < 6816))
; 6797 to 6815
.if((n .mod 3 = 2) && ((n > 6796 && n < 6816)))
	; 2267..2275 bytes
	delay_n (n-2)
	nop
.elseif(n = 6798)
	; 2266 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6799)
	; 2270 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6801)
	; 2270 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2272 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6807)
	; 2272 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6808)
	; 2270 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6810)
	; 2270 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6811)
	; 2274 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6813)
	; 2274 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6814)
	; 2272 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 6816 to 6835
.if(n = 6816)
	; 2272 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6817)
	; 2276 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 6817 && n < 6834)))
	; 2273..2281 bytes
	delay_n (n-2)
	nop
.elseif(n = 6819)
	; 2276 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6820)
	; 2274 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6822)
	; 2274 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6823)
	; 2278 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6825)
	; 2278 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6826)
	; 2276 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6828)
	; 2276 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6829)
	; 2280 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6831)
	; 2280 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6832)
	; 2278 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6834)
	; 2278 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6835)
	; 2282 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.else
.if((n >= 6836) && (n < 6856))
; 6836 to 6855
.if((n .mod 3 = 2) && ((n > 6835 && n < 6855)))
	; 2279..2287 bytes
	delay_n (n-2)
	nop
.elseif(n = 6837)
	; 2282 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6838)
	; 2280 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6840)
	; 2280 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6841)
	; 2284 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6843)
	; 2284 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6844)
	; 2282 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6846)
	; 2282 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6847)
	; 2286 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6849)
	; 2286 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6850)
	; 2284 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6852)
	; 2284 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6853)
	; 2288 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6855)
	; 2288 bytes
	delay_n 105
	delay_n (n-105)
.endif
.else
; 6856 to 6875
.if(n = 6856)
	; 2286 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 6856 && n < 6876)))
	; 2287..2295 bytes
	delay_n (n-2)
	nop
.elseif(n = 6858)
	; 2286 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 6859)
	; 2290 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6861)
	; 2290 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6862)
	; 2288 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6864)
	; 2288 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6865)
	; 2292 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6867)
	; 2292 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6868)
	; 2290 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6870)
	; 2290 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6871)
	; 2294 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6873)
	; 2294 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6874)
	; 2292 bytes
	delay_n 70
	delay_n (n-70)
.endif
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
.if((n >= 6876) && (n < 6895))
; 6876 to 6894
.if(n = 6876)
	; 2292 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6877)
	; 2296 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 6877 && n < 6894)))
	; 2293..2301 bytes
	delay_n (n-2)
	nop
.elseif(n = 6879)
	; 2296 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6880)
	; 2294 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6882)
	; 2294 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6883)
	; 2298 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6885)
	; 2298 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6886)
	; 2296 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6888)
	; 2296 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6889)
	; 2300 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6891)
	; 2300 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6892)
	; 2298 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6894)
	; 2298 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 6895 to 6914
.if(n = 6895)
	; 2302 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 6895 && n < 6915)))
	; 2299..2307 bytes
	delay_n (n-2)
	nop
.elseif(n = 6897)
	; 2302 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6898)
	; 2300 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 6900)
	; 2300 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6901)
	; 2304 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6903)
	; 2304 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6904)
	; 2302 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6906)
	; 2302 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6907)
	; 2306 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6909)
	; 2306 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2308 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 6915) && (n < 6934))
; 6915 to 6933
.if(n = 6915)
	; 2308 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6916)
	; 2306 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 6916 && n < 6933)))
	; 2307..2313 bytes
	delay_n (n-2)
	nop
.elseif(n = 6918)
	; 2306 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 6919)
	; 2310 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 6921)
	; 2310 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6922)
	; 2308 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6924)
	; 2308 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6925)
	; 2312 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6927)
	; 2312 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6928)
	; 2310 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6930)
	; 2310 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6931)
	; 2314 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6933)
	; 2314 bytes
	delay_n 129
	delay_n (n-129)
.endif
.else
; 6934 to 6953
.if(n = 6934)
	; 2312 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 6934 && n < 6954)))
	; 2313..2321 bytes
	delay_n (n-2)
	nop
.elseif(n = 6936)
	; 2312 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6937)
	; 2316 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 6939)
	; 2316 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6940)
	; 2314 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 6942)
	; 2314 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6943)
	; 2318 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6945)
	; 2318 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6946)
	; 2316 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 6948)
	; 2316 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6949)
	; 2320 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6951)
	; 2320 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 6952)
	; 2318 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.endif
.else
.if((n >= 6954) && (n < 6993))
.if((n >= 6954) && (n < 6973))
; 6954 to 6972
.if(n = 6954)
	; 2318 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6955)
	; 2322 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 6955 && n < 6972)))
	; 2319..2327 bytes
	delay_n (n-2)
	nop
.elseif(n = 6957)
	; 2322 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 6958)
	; 2320 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6960)
	; 2320 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 6961)
	; 2324 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 6963)
	; 2324 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2326 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 6969)
	; 2326 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 6970)
	; 2324 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 6972)
	; 2324 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 6973 to 6992
.if(n = 6973)
	; 2328 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 6973 && n < 6993)))
	; 2325..2333 bytes
	delay_n (n-2)
	nop
.elseif(n = 6975)
	; 2328 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 6976)
	; 2326 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6978)
	; 2326 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6979)
	; 2330 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 6981)
	; 2330 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 6982)
	; 2328 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 6984)
	; 2328 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 6985)
	; 2332 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 6987)
	; 2332 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 6988)
	; 2330 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 6990)
	; 2330 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 6991)
	; 2334 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 6993) && (n < 7012))
; 6993 to 7011
.if(n = 6993)
	; 2334 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 6994)
	; 2332 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 6994 && n < 7011)))
	; 2333..2339 bytes
	delay_n (n-2)
	nop
.elseif(n = 6996)
	; 2332 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 6997)
	; 2336 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 6999)
	; 2336 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7000)
	; 2334 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7002)
	; 2334 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7003)
	; 2338 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7005)
	; 2338 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7006)
	; 2336 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7008)
	; 2336 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7009)
	; 2340 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7011)
	; 2340 bytes
	delay_n 45
	delay_n (n-45)
.endif
.else
; 7012 to 7031
.if(n = 7012)
	; 2338 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 7012 && n < 7032)))
	; 2339..2347 bytes
	delay_n (n-2)
	nop
.elseif(n = 7014)
	; 2338 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7015)
	; 2342 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7017)
	; 2342 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2344 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7023)
	; 2344 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7024)
	; 2342 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7026)
	; 2342 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7027)
	; 2346 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7029)
	; 2346 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7030)
	; 2344 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
.if((n >= 7032) && (n < 7071))
.if((n >= 7032) && (n < 7051))
; 7032 to 7050
.if(n = 7032)
	; 2344 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7033)
	; 2348 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 7033 && n < 7050)))
	; 2345..2353 bytes
	delay_n (n-2)
	nop
.elseif(n = 7035)
	; 2348 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7036)
	; 2346 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7038)
	; 2346 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7039)
	; 2350 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7041)
	; 2350 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7042)
	; 2348 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7044)
	; 2348 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7045)
	; 2352 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7047)
	; 2352 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7048)
	; 2350 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7050)
	; 2350 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 7051 to 7070
.if(n = 7051)
	; 2354 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 7051 && n < 7071)))
	; 2351..2359 bytes
	delay_n (n-2)
	nop
.elseif(n = 7053)
	; 2354 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7054)
	; 2352 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7056)
	; 2352 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7057)
	; 2356 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7059)
	; 2356 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7060)
	; 2354 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7062)
	; 2354 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7063)
	; 2358 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7065)
	; 2358 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7066)
	; 2356 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7068)
	; 2356 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7069)
	; 2360 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 7071) && (n < 7090))
; 7071 to 7089
.if(n = 7071)
	; 2360 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 7072)
	; 2358 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 7072 && n < 7089)))
	; 2359..2365 bytes
	delay_n (n-2)
	nop
.elseif(n = 7074)
	; 2358 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7075)
	; 2362 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7077)
	; 2362 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7078)
	; 2360 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7080)
	; 2360 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7081)
	; 2364 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7083)
	; 2364 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7084)
	; 2362 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7086)
	; 2362 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7087)
	; 2366 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7089)
	; 2366 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 7090 to 7109
.if(n = 7090)
	; 2364 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 7090 && n < 7110)))
	; 2365..2373 bytes
	delay_n (n-2)
	nop
.elseif(n = 7092)
	; 2364 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7093)
	; 2368 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7095)
	; 2368 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7096)
	; 2366 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7098)
	; 2366 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7099)
	; 2370 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7101)
	; 2370 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7102)
	; 2368 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7104)
	; 2368 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7105)
	; 2372 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7107)
	; 2372 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7108)
	; 2370 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.else
.if((n >= 7110) && (n < 7149))
.if((n >= 7110) && (n < 7129))
; 7110 to 7128
.if(n = 7110)
	; 2370 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7111)
	; 2374 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 7111 && n < 7128)))
	; 2371..2379 bytes
	delay_n (n-2)
	nop
.elseif(n = 7113)
	; 2374 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7114)
	; 2372 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7116)
	; 2372 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7117)
	; 2376 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7119)
	; 2376 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7120)
	; 2374 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7122)
	; 2374 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7123)
	; 2378 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7125)
	; 2378 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 7126)
	; 2376 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7128)
	; 2376 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 7129 to 7148
.if(n = 7129)
	; 2380 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 7129 && n < 7149)))
	; 2377..2385 bytes
	delay_n (n-2)
	nop
.elseif(n = 7131)
	; 2380 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7132)
	; 2378 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7134)
	; 2378 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7135)
	; 2382 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7137)
	; 2382 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7138)
	; 2380 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7140)
	; 2380 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7141)
	; 2384 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7143)
	; 2384 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7144)
	; 2382 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7146)
	; 2382 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7147)
	; 2386 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.else
.if((n >= 7149) && (n < 7168))
; 7149 to 7167
.if(n = 7149)
	; 2386 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7150)
	; 2384 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 7150 && n < 7167)))
	; 2385..2391 bytes
	delay_n (n-2)
	nop
.elseif(n = 7152)
	; 2384 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7153)
	; 2388 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7155)
	; 2388 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7156)
	; 2386 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7158)
	; 2386 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7159)
	; 2390 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7161)
	; 2390 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7162)
	; 2388 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7164)
	; 2388 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7165)
	; 2392 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7167)
	; 2392 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 7168 to 7187
.if(n = 7168)
	; 2390 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 7168 && n < 7188)))
	; 2391..2399 bytes
	delay_n (n-2)
	nop
.elseif(n = 7170)
	; 2390 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7171)
	; 2394 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7173)
	; 2394 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7174)
	; 2392 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7176)
	; 2392 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7177)
	; 2396 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7179)
	; 2396 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2398 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7185)
	; 2398 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7186)
	; 2396 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
.if((n >= 7188) && (n < 7266))
.if((n >= 7188) && (n < 7227))
.if((n >= 7188) && (n < 7207))
; 7188 to 7206
.if(n = 7188)
	; 2396 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7189)
	; 2400 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 7189 && n < 7206)))
	; 2397..2405 bytes
	delay_n (n-2)
	nop
.elseif(n = 7191)
	; 2400 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7192)
	; 2398 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7194)
	; 2398 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7195)
	; 2402 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7197)
	; 2402 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7198)
	; 2400 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7200)
	; 2400 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7201)
	; 2404 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7203)
	; 2404 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7204)
	; 2402 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7206)
	; 2402 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 7207 to 7226
.if(n = 7207)
	; 2406 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 7207 && n < 7227)))
	; 2403..2411 bytes
	delay_n (n-2)
	nop
.elseif(n = 7209)
	; 2406 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7210)
	; 2404 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7212)
	; 2404 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7213)
	; 2408 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7215)
	; 2408 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7216)
	; 2406 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7218)
	; 2406 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7219)
	; 2410 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7221)
	; 2410 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7222)
	; 2408 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7224)
	; 2408 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7225)
	; 2412 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 7227) && (n < 7246))
; 7227 to 7245
.if(n = 7227)
	; 2412 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7228)
	; 2410 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 7228 && n < 7245)))
	; 2411..2417 bytes
	delay_n (n-2)
	nop
.elseif(n = 7230)
	; 2410 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7231)
	; 2414 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7233)
	; 2414 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2416 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7239)
	; 2416 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7240)
	; 2414 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7242)
	; 2414 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7243)
	; 2418 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7245)
	; 2418 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 7246 to 7265
.if(n = 7246)
	; 2416 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 7246 && n < 7266)))
	; 2417..2425 bytes
	delay_n (n-2)
	nop
.elseif(n = 7248)
	; 2416 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7249)
	; 2420 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7251)
	; 2420 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7252)
	; 2418 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7254)
	; 2418 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7255)
	; 2422 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7257)
	; 2422 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7258)
	; 2420 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7260)
	; 2420 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7261)
	; 2424 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7263)
	; 2424 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7264)
	; 2422 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.else
.if((n >= 7266) && (n < 7305))
.if((n >= 7266) && (n < 7285))
; 7266 to 7284
.if(n = 7266)
	; 2422 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7267)
	; 2426 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 7267 && n < 7284)))
	; 2423..2431 bytes
	delay_n (n-2)
	nop
.elseif(n = 7269)
	; 2426 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7270)
	; 2424 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7272)
	; 2424 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7273)
	; 2428 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7275)
	; 2428 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7276)
	; 2426 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7278)
	; 2426 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7279)
	; 2430 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7281)
	; 2430 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7282)
	; 2428 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7284)
	; 2428 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 7285 to 7304
.if(n = 7285)
	; 2432 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 7285 && n < 7305)))
	; 2429..2437 bytes
	delay_n (n-2)
	nop
.elseif(n = 7287)
	; 2432 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2434 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7293)
	; 2434 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7294)
	; 2432 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7296)
	; 2432 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7297)
	; 2436 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7299)
	; 2436 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7300)
	; 2434 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7302)
	; 2434 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7303)
	; 2438 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 7305) && (n < 7324))
; 7305 to 7323
.if(n = 7305)
	; 2438 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7306)
	; 2436 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 7306 && n < 7323)))
	; 2437..2443 bytes
	delay_n (n-2)
	nop
.elseif(n = 7308)
	; 2436 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7309)
	; 2440 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7311)
	; 2440 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7312)
	; 2438 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7314)
	; 2438 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7315)
	; 2442 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7317)
	; 2442 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7318)
	; 2440 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7320)
	; 2440 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7321)
	; 2444 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7323)
	; 2444 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 7324 to 7343
.if(n = 7324)
	; 2442 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 7324 && n < 7344)))
	; 2443..2451 bytes
	delay_n (n-2)
	nop
.elseif(n = 7326)
	; 2442 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7327)
	; 2446 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7329)
	; 2446 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7330)
	; 2444 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7332)
	; 2444 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7333)
	; 2448 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7335)
	; 2448 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7336)
	; 2446 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7338)
	; 2446 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7339)
	; 2450 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7341)
	; 2450 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 7342)
	; 2448 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.endif
.else
.if((n >= 7344) && (n < 7422))
.if((n >= 7344) && (n < 7383))
.if((n >= 7344) && (n < 7363))
; 7344 to 7362
.if(n = 7344)
	; 2448 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7345)
	; 2452 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 7345 && n < 7362)))
	; 2449..2457 bytes
	delay_n (n-2)
	nop
.elseif(n = 7347)
	; 2452 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7348)
	; 2450 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7350)
	; 2450 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7351)
	; 2454 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7353)
	; 2454 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7354)
	; 2452 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7356)
	; 2452 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7357)
	; 2456 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7359)
	; 2456 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7360)
	; 2454 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7362)
	; 2454 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 7363 to 7382
.if(n = 7363)
	; 2458 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 7363 && n < 7383)))
	; 2455..2463 bytes
	delay_n (n-2)
	nop
.elseif(n = 7365)
	; 2458 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7366)
	; 2456 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7368)
	; 2456 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7369)
	; 2460 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7371)
	; 2460 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7372)
	; 2458 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7374)
	; 2458 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7375)
	; 2462 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7377)
	; 2462 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7378)
	; 2460 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7380)
	; 2460 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7381)
	; 2464 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 7383) && (n < 7402))
; 7383 to 7401
.if(n = 7383)
	; 2464 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7384)
	; 2462 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 7384 && n < 7401)))
	; 2463..2469 bytes
	delay_n (n-2)
	nop
.elseif(n = 7386)
	; 2462 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7387)
	; 2466 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7389)
	; 2466 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7390)
	; 2464 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7392)
	; 2464 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7393)
	; 2468 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7395)
	; 2468 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2470 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7401)
	; 2470 bytes
	delay_n 57
	delay_n (n-57)
.endif
.else
; 7402 to 7421
.if(n = 7402)
	; 2468 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 7402 && n < 7422)))
	; 2469..2477 bytes
	delay_n (n-2)
	nop
.elseif(n = 7404)
	; 2468 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7405)
	; 2472 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7407)
	; 2472 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7408)
	; 2470 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7410)
	; 2470 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7411)
	; 2474 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7413)
	; 2474 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7414)
	; 2472 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7416)
	; 2472 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7417)
	; 2476 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7419)
	; 2476 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7420)
	; 2474 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.else
.if((n >= 7422) && (n < 7461))
.if((n >= 7422) && (n < 7441))
; 7422 to 7440
.if(n = 7422)
	; 2474 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7423)
	; 2478 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 7423 && n < 7440)))
	; 2475..2483 bytes
	delay_n (n-2)
	nop
.elseif(n = 7425)
	; 2478 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7426)
	; 2476 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7428)
	; 2476 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7429)
	; 2480 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7431)
	; 2480 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7432)
	; 2478 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7434)
	; 2478 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7435)
	; 2482 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7437)
	; 2482 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7438)
	; 2480 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7440)
	; 2480 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 7441 to 7460
.if(n = 7441)
	; 2484 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 7441 && n < 7461)))
	; 2481..2489 bytes
	delay_n (n-2)
	nop
.elseif(n = 7443)
	; 2484 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7444)
	; 2482 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7446)
	; 2482 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7447)
	; 2486 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7449)
	; 2486 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2488 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7455)
	; 2488 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7456)
	; 2486 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7458)
	; 2486 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7459)
	; 2490 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.else
.if((n >= 7461) && (n < 7481))
; 7461 to 7480
.if(n = 7461)
	; 2490 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7462)
	; 2488 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 7462 && n < 7479)))
	; 2489..2495 bytes
	delay_n (n-2)
	nop
.elseif(n = 7464)
	; 2488 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7465)
	; 2492 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7467)
	; 2492 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7468)
	; 2490 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7470)
	; 2490 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7471)
	; 2494 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7473)
	; 2494 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7474)
	; 2492 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7476)
	; 2492 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7477)
	; 2496 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7479)
	; 2496 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7480)
	; 2494 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 7481 to 7500
.if((n .mod 3 = 2) && ((n > 7480 && n < 7500)))
	; 2495..2503 bytes
	delay_n (n-2)
	nop
.elseif(n = 7482)
	; 2494 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7483)
	; 2498 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7485)
	; 2498 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7486)
	; 2496 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7488)
	; 2496 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7489)
	; 2500 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7491)
	; 2500 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7492)
	; 2498 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7494)
	; 2498 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7495)
	; 2502 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7497)
	; 2502 bytes
	delay_n 45
	delay_n (n-45)
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
.endif
.else
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
.if((n >= 7501) && (n < 7813))
.if((n >= 7501) && (n < 7657))
.if((n >= 7501) && (n < 7579))
.if((n >= 7501) && (n < 7540))
.if((n >= 7501) && (n < 7520))
; 7501 to 7519
.if(n = 7501)
	; 2504 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 7501 && n < 7518)))
	; 2501..2509 bytes
	delay_n (n-2)
	nop
.elseif(n = 7503)
	; 2504 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2506 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7509)
	; 2506 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7510)
	; 2504 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7512)
	; 2504 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7513)
	; 2508 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7515)
	; 2508 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7516)
	; 2506 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7518)
	; 2506 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7519)
	; 2510 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 7520 to 7539
.if((n .mod 3 = 2) && ((n > 7519 && n < 7539)))
	; 2507..2515 bytes
	delay_n (n-2)
	nop
.elseif(n = 7521)
	; 2510 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7522)
	; 2508 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7524)
	; 2508 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7525)
	; 2512 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7527)
	; 2512 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7528)
	; 2510 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7530)
	; 2510 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7531)
	; 2514 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7533)
	; 2514 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7534)
	; 2512 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7536)
	; 2512 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7537)
	; 2516 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7539)
	; 2516 bytes
	delay_n 141
	delay_n (n-141)
.endif
.endif
.else
.if((n >= 7540) && (n < 7559))
; 7540 to 7558
.if(n = 7540)
	; 2514 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 7540 && n < 7557)))
	; 2515..2521 bytes
	delay_n (n-2)
	nop
.elseif(n = 7542)
	; 2514 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7543)
	; 2518 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7545)
	; 2518 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7546)
	; 2516 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7548)
	; 2516 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7549)
	; 2520 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7551)
	; 2520 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7552)
	; 2518 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7554)
	; 2518 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7555)
	; 2522 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7557)
	; 2522 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 7558)
	; 2520 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 7559 to 7578
.if((n .mod 3 = 2) && ((n > 7558 && n < 7578)))
	; 2521..2529 bytes
	delay_n (n-2)
	nop
.elseif(n = 7560)
	; 2520 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7561)
	; 2524 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7563)
	; 2524 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7564)
	; 2522 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7566)
	; 2522 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7567)
	; 2526 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7569)
	; 2526 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7570)
	; 2524 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7572)
	; 2524 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7573)
	; 2528 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7575)
	; 2528 bytes
	delay_n 69
	delay_n (n-69)
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
.endif
.else
.if((n >= 7579) && (n < 7618))
.if((n >= 7579) && (n < 7598))
; 7579 to 7597
.if(n = 7579)
	; 2530 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 7579 && n < 7596)))
	; 2527..2535 bytes
	delay_n (n-2)
	nop
.elseif(n = 7581)
	; 2530 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7582)
	; 2528 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7584)
	; 2528 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7585)
	; 2532 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7587)
	; 2532 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7588)
	; 2530 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7590)
	; 2530 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7591)
	; 2534 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7593)
	; 2534 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7594)
	; 2532 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7596)
	; 2532 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7597)
	; 2536 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 7598 to 7617
.if((n .mod 3 = 2) && ((n > 7597 && n < 7617)))
	; 2533..2541 bytes
	delay_n (n-2)
	nop
.elseif(n = 7599)
	; 2536 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7600)
	; 2534 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7602)
	; 2534 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7603)
	; 2538 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7605)
	; 2538 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7606)
	; 2536 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7608)
	; 2536 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7609)
	; 2540 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7611)
	; 2540 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2542 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7617)
	; 2542 bytes
	delay_n 57
	delay_n (n-57)
.endif
.endif
.else
.if((n >= 7618) && (n < 7637))
; 7618 to 7636
.if(n = 7618)
	; 2540 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 7618 && n < 7635)))
	; 2541..2547 bytes
	delay_n (n-2)
	nop
.elseif(n = 7620)
	; 2540 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7621)
	; 2544 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7623)
	; 2544 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7624)
	; 2542 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7626)
	; 2542 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7627)
	; 2546 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7629)
	; 2546 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7630)
	; 2544 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7632)
	; 2544 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7633)
	; 2548 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7635)
	; 2548 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7636)
	; 2546 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 7637 to 7656
.if((n .mod 3 = 2) && ((n > 7636 && n < 7656)))
	; 2547..2555 bytes
	delay_n (n-2)
	nop
.elseif(n = 7638)
	; 2546 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7639)
	; 2550 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7641)
	; 2550 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7642)
	; 2548 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7644)
	; 2548 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7645)
	; 2552 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7647)
	; 2552 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7648)
	; 2550 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7650)
	; 2550 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7651)
	; 2554 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7653)
	; 2554 bytes
	delay_n 93
	delay_n (n-93)
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
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
.if((n >= 7657) && (n < 7676))
; 7657 to 7675
.if(n = 7657)
	; 2556 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 7657 && n < 7674)))
	; 2553..2561 bytes
	delay_n (n-2)
	nop
.elseif(n = 7659)
	; 2556 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7660)
	; 2554 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7662)
	; 2554 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7663)
	; 2558 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7665)
	; 2558 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2560 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7671)
	; 2560 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7672)
	; 2558 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7674)
	; 2558 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7675)
	; 2562 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 7676 to 7695
.if((n .mod 3 = 2) && ((n > 7675 && n < 7695)))
	; 2559..2567 bytes
	delay_n (n-2)
	nop
.elseif(n = 7677)
	; 2562 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7678)
	; 2560 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7680)
	; 2560 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7681)
	; 2564 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7683)
	; 2564 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7684)
	; 2562 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7686)
	; 2562 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7687)
	; 2566 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7689)
	; 2566 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7690)
	; 2564 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7692)
	; 2564 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7693)
	; 2568 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7695)
	; 2568 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.else
.if((n >= 7696) && (n < 7715))
; 7696 to 7714
.if(n = 7696)
	; 2566 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 7696 && n < 7713)))
	; 2567..2573 bytes
	delay_n (n-2)
	nop
.elseif(n = 7698)
	; 2566 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7699)
	; 2570 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7701)
	; 2570 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7702)
	; 2568 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7704)
	; 2568 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7705)
	; 2572 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7707)
	; 2572 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7708)
	; 2570 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7710)
	; 2570 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7711)
	; 2574 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7713)
	; 2574 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7714)
	; 2572 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 7715 to 7734
.if((n .mod 3 = 2) && ((n > 7714 && n < 7734)))
	; 2573..2581 bytes
	delay_n (n-2)
	nop
.elseif(n = 7716)
	; 2572 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7717)
	; 2576 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7719)
	; 2576 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2578 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7725)
	; 2578 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7726)
	; 2576 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7728)
	; 2576 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7729)
	; 2580 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7731)
	; 2580 bytes
	delay_n 117
	delay_n (n-117)
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
.endif
.else
.if((n >= 7735) && (n < 7774))
.if((n >= 7735) && (n < 7754))
; 7735 to 7753
.if(n = 7735)
	; 2582 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 7735 && n < 7752)))
	; 2579..2587 bytes
	delay_n (n-2)
	nop
.elseif(n = 7737)
	; 2582 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7738)
	; 2580 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7740)
	; 2580 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7741)
	; 2584 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7743)
	; 2584 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7744)
	; 2582 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7746)
	; 2582 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7747)
	; 2586 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7749)
	; 2586 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7750)
	; 2584 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7752)
	; 2584 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7753)
	; 2588 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 7754 to 7773
.if((n .mod 3 = 2) && ((n > 7753 && n < 7773)))
	; 2585..2593 bytes
	delay_n (n-2)
	nop
.elseif(n = 7755)
	; 2588 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7756)
	; 2586 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7758)
	; 2586 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7759)
	; 2590 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7761)
	; 2590 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7762)
	; 2588 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7764)
	; 2588 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7765)
	; 2592 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7767)
	; 2592 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7768)
	; 2590 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7770)
	; 2590 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7771)
	; 2594 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7773)
	; 2594 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.else
.if((n >= 7774) && (n < 7793))
; 7774 to 7792
.if(n = 7774)
	; 2592 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 7774 && n < 7791)))
	; 2593..2599 bytes
	delay_n (n-2)
	nop
.elseif(n = 7776)
	; 2592 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 7777)
	; 2596 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7779)
	; 2596 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7780)
	; 2594 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7782)
	; 2594 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7783)
	; 2598 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7785)
	; 2598 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7786)
	; 2596 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7788)
	; 2596 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7789)
	; 2600 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7791)
	; 2600 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7792)
	; 2598 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 7793 to 7812
.if((n .mod 3 = 2) && ((n > 7792 && n < 7812)))
	; 2599..2607 bytes
	delay_n (n-2)
	nop
.elseif(n = 7794)
	; 2598 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7795)
	; 2602 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7797)
	; 2602 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7798)
	; 2600 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7800)
	; 2600 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7801)
	; 2604 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7803)
	; 2604 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7804)
	; 2602 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7806)
	; 2602 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7807)
	; 2606 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7809)
	; 2606 bytes
	delay_n 141
	delay_n (n-141)
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
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
.if((n >= 7813) && (n < 7852))
.if((n >= 7813) && (n < 7832))
; 7813 to 7831
.if(n = 7813)
	; 2608 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 7813 && n < 7830)))
	; 2605..2613 bytes
	delay_n (n-2)
	nop
.elseif(n = 7815)
	; 2608 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7816)
	; 2606 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7818)
	; 2606 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7819)
	; 2610 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7821)
	; 2610 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7822)
	; 2608 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7824)
	; 2608 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7825)
	; 2612 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7827)
	; 2612 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2614 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 7832 to 7851
.if((n .mod 3 = 2) && ((n > 7831 && n < 7851)))
	; 2611..2619 bytes
	delay_n (n-2)
	nop
.elseif(n = 7833)
	; 2614 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7834)
	; 2612 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7836)
	; 2612 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7837)
	; 2616 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7839)
	; 2616 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7840)
	; 2614 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7842)
	; 2614 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7843)
	; 2618 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7845)
	; 2618 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7846)
	; 2616 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7848)
	; 2616 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7849)
	; 2620 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7851)
	; 2620 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.else
.if((n >= 7852) && (n < 7871))
; 7852 to 7870
.if(n = 7852)
	; 2618 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 7852 && n < 7869)))
	; 2619..2625 bytes
	delay_n (n-2)
	nop
.elseif(n = 7854)
	; 2618 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7855)
	; 2622 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7857)
	; 2622 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7858)
	; 2620 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7860)
	; 2620 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7861)
	; 2624 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7863)
	; 2624 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7864)
	; 2622 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7866)
	; 2622 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7867)
	; 2626 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7869)
	; 2626 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7870)
	; 2624 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 7871 to 7890
.if((n .mod 3 = 2) && ((n > 7870 && n < 7890)))
	; 2625..2633 bytes
	delay_n (n-2)
	nop
.elseif(n = 7872)
	; 2624 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7873)
	; 2628 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7875)
	; 2628 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7876)
	; 2626 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7878)
	; 2626 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7879)
	; 2630 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7881)
	; 2630 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2632 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7887)
	; 2632 bytes
	delay_n 57
	delay_n (n-57)
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
.endif
.else
.if((n >= 7891) && (n < 7930))
.if((n >= 7891) && (n < 7910))
; 7891 to 7909
.if(n = 7891)
	; 2634 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 7891 && n < 7908)))
	; 2631..2639 bytes
	delay_n (n-2)
	nop
.elseif(n = 7893)
	; 2634 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7894)
	; 2632 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7896)
	; 2632 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7897)
	; 2636 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7899)
	; 2636 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7900)
	; 2634 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7902)
	; 2634 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7903)
	; 2638 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7905)
	; 2638 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7906)
	; 2636 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7908)
	; 2636 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7909)
	; 2640 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 7910 to 7929
.if((n .mod 3 = 2) && ((n > 7909 && n < 7929)))
	; 2637..2645 bytes
	delay_n (n-2)
	nop
.elseif(n = 7911)
	; 2640 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7912)
	; 2638 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 7914)
	; 2638 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 7915)
	; 2642 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 7917)
	; 2642 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7918)
	; 2640 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7920)
	; 2640 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7921)
	; 2644 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7923)
	; 2644 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7924)
	; 2642 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7926)
	; 2642 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7927)
	; 2646 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7929)
	; 2646 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.else
.if((n >= 7930) && (n < 7949))
; 7930 to 7948
.if(n = 7930)
	; 2644 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 7930 && n < 7947)))
	; 2645..2651 bytes
	delay_n (n-2)
	nop
.elseif(n = 7932)
	; 2644 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7933)
	; 2648 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 7935)
	; 2648 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2650 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7941)
	; 2650 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7942)
	; 2648 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7944)
	; 2648 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7945)
	; 2652 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 7947)
	; 2652 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 7948)
	; 2650 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 7949 to 7968
.if((n .mod 3 = 2) && ((n > 7948 && n < 7968)))
	; 2651..2659 bytes
	delay_n (n-2)
	nop
.elseif(n = 7950)
	; 2650 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7951)
	; 2654 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 7953)
	; 2654 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 7954)
	; 2652 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 7956)
	; 2652 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 7957)
	; 2656 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 7959)
	; 2656 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 7960)
	; 2654 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 7962)
	; 2654 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 7963)
	; 2658 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 7965)
	; 2658 bytes
	delay_n 81
	delay_n (n-81)
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
.endif
.else
.if((n >= 7969) && (n < 8047))
.if((n >= 7969) && (n < 8008))
.if((n >= 7969) && (n < 7988))
; 7969 to 7987
.if(n = 7969)
	; 2660 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 7969 && n < 7986)))
	; 2657..2665 bytes
	delay_n (n-2)
	nop
.elseif(n = 7971)
	; 2660 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7972)
	; 2658 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 7974)
	; 2658 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7975)
	; 2662 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7977)
	; 2662 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 7978)
	; 2660 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 7980)
	; 2660 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7981)
	; 2664 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7983)
	; 2664 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 7984)
	; 2662 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7986)
	; 2662 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 7987)
	; 2666 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 7988 to 8007
.if((n .mod 3 = 2) && ((n > 7987 && n < 8007)))
	; 2663..2671 bytes
	delay_n (n-2)
	nop
.elseif(n = 7989)
	; 2666 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2668 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7995)
	; 2668 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 7996)
	; 2666 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 7998)
	; 2666 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 7999)
	; 2670 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8001)
	; 2670 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8002)
	; 2668 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8004)
	; 2668 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8005)
	; 2672 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8007)
	; 2672 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.else
.if((n >= 8008) && (n < 8027))
; 8008 to 8026
.if(n = 8008)
	; 2670 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 8008 && n < 8025)))
	; 2671..2677 bytes
	delay_n (n-2)
	nop
.elseif(n = 8010)
	; 2670 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8011)
	; 2674 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8013)
	; 2674 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8014)
	; 2672 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8016)
	; 2672 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8017)
	; 2676 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8019)
	; 2676 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8020)
	; 2674 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8022)
	; 2674 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8023)
	; 2678 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8025)
	; 2678 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8026)
	; 2676 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 8027 to 8046
.if((n .mod 3 = 2) && ((n > 8026 && n < 8046)))
	; 2677..2685 bytes
	delay_n (n-2)
	nop
.elseif(n = 8028)
	; 2676 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8029)
	; 2680 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8031)
	; 2680 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8032)
	; 2678 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8034)
	; 2678 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8035)
	; 2682 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8037)
	; 2682 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8038)
	; 2680 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8040)
	; 2680 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8041)
	; 2684 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8043)
	; 2684 bytes
	delay_n 105
	delay_n (n-105)
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
.endif
.else
.if((n >= 8047) && (n < 8086))
.if((n >= 8047) && (n < 8066))
; 8047 to 8065
.if(n = 8047)
	; 2686 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 8047 && n < 8064)))
	; 2683..2691 bytes
	delay_n (n-2)
	nop
.elseif(n = 8049)
	; 2686 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8050)
	; 2684 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8052)
	; 2684 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8053)
	; 2688 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8055)
	; 2688 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8056)
	; 2686 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8058)
	; 2686 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8059)
	; 2690 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8061)
	; 2690 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8062)
	; 2688 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8064)
	; 2688 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8065)
	; 2692 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 8066 to 8085
.if((n .mod 3 = 2) && ((n > 8065 && n < 8085)))
	; 2689..2697 bytes
	delay_n (n-2)
	nop
.elseif(n = 8067)
	; 2692 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8068)
	; 2690 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8070)
	; 2690 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8071)
	; 2694 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8073)
	; 2694 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8074)
	; 2692 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8076)
	; 2692 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8077)
	; 2696 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8079)
	; 2696 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8080)
	; 2694 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8082)
	; 2694 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8083)
	; 2698 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8085)
	; 2698 bytes
	delay_n 93
	delay_n (n-93)
.endif
.endif
.else
.if((n >= 8086) && (n < 8106))
; 8086 to 8105
.if(n = 8086)
	; 2696 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 8086 && n < 8106)))
	; 2697..2705 bytes
	delay_n (n-2)
	nop
.elseif(n = 8088)
	; 2696 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8089)
	; 2700 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8091)
	; 2700 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8092)
	; 2698 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8094)
	; 2698 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8095)
	; 2702 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8097)
	; 2702 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2704 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8103)
	; 2704 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8104)
	; 2702 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 8106 to 8125
.if(n = 8106)
	; 2702 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8107)
	; 2706 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 8107 && n < 8124)))
	; 2703..2711 bytes
	delay_n (n-2)
	nop
.elseif(n = 8109)
	; 2706 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8110)
	; 2704 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8112)
	; 2704 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8113)
	; 2708 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8115)
	; 2708 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8116)
	; 2706 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8118)
	; 2706 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8119)
	; 2710 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8121)
	; 2710 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8122)
	; 2708 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8124)
	; 2708 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8125)
	; 2712 bytes
	delay_n 79
	delay_n (n-79)
.endif
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
.if((n >= 8126) && (n < 8145))
; 8126 to 8144
.if((n .mod 3 = 2) && ((n > 8125 && n < 8145)))
	; 2709..2717 bytes
	delay_n (n-2)
	nop
.elseif(n = 8127)
	; 2712 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8128)
	; 2710 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8130)
	; 2710 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8131)
	; 2714 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8133)
	; 2714 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8134)
	; 2712 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8136)
	; 2712 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8137)
	; 2716 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8139)
	; 2716 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8140)
	; 2714 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8142)
	; 2714 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8143)
	; 2718 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 8145 to 8164
.if(n = 8145)
	; 2718 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8146)
	; 2716 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 8146 && n < 8163)))
	; 2717..2723 bytes
	delay_n (n-2)
	nop
.elseif(n = 8148)
	; 2716 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8149)
	; 2720 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8151)
	; 2720 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2722 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8157)
	; 2722 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8158)
	; 2720 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8160)
	; 2720 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8161)
	; 2724 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8163)
	; 2724 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8164)
	; 2722 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 8165) && (n < 8184))
; 8165 to 8183
.if((n .mod 3 = 2) && ((n > 8164 && n < 8184)))
	; 2723..2731 bytes
	delay_n (n-2)
	nop
.elseif(n = 8166)
	; 2722 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8167)
	; 2726 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8169)
	; 2726 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8170)
	; 2724 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8172)
	; 2724 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8173)
	; 2728 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8175)
	; 2728 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8176)
	; 2726 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8178)
	; 2726 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8179)
	; 2730 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8181)
	; 2730 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8182)
	; 2728 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 8184 to 8203
.if(n = 8184)
	; 2728 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8185)
	; 2732 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 8185 && n < 8202)))
	; 2729..2737 bytes
	delay_n (n-2)
	nop
.elseif(n = 8187)
	; 2732 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8188)
	; 2730 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8190)
	; 2730 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8191)
	; 2734 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8193)
	; 2734 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8194)
	; 2732 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8196)
	; 2732 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8197)
	; 2736 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8199)
	; 2736 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8200)
	; 2734 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8202)
	; 2734 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8203)
	; 2738 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
.if((n >= 8204) && (n < 8223))
; 8204 to 8222
.if((n .mod 3 = 2) && ((n > 8203 && n < 8223)))
	; 2735..2743 bytes
	delay_n (n-2)
	nop
.elseif(n = 8205)
	; 2738 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2740 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8211)
	; 2740 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8212)
	; 2738 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8214)
	; 2738 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8215)
	; 2742 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8217)
	; 2742 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8218)
	; 2740 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8220)
	; 2740 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8221)
	; 2744 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 8223 to 8242
.if(n = 8223)
	; 2744 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8224)
	; 2742 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 8224 && n < 8241)))
	; 2743..2749 bytes
	delay_n (n-2)
	nop
.elseif(n = 8226)
	; 2742 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8227)
	; 2746 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8229)
	; 2746 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8230)
	; 2744 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8232)
	; 2744 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8233)
	; 2748 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8235)
	; 2748 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8236)
	; 2746 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8238)
	; 2746 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8239)
	; 2750 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8241)
	; 2750 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8242)
	; 2748 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 8243) && (n < 8262))
; 8243 to 8261
.if((n .mod 3 = 2) && ((n > 8242 && n < 8262)))
	; 2749..2757 bytes
	delay_n (n-2)
	nop
.elseif(n = 8244)
	; 2748 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8245)
	; 2752 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8247)
	; 2752 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8248)
	; 2750 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8250)
	; 2750 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8251)
	; 2754 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8253)
	; 2754 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8254)
	; 2752 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8256)
	; 2752 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8257)
	; 2756 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8259)
	; 2756 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 8260)
	; 2754 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 8262 to 8281
.if(n = 8262)
	; 2754 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8263)
	; 2758 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 8263 && n < 8280)))
	; 2755..2763 bytes
	delay_n (n-2)
	nop
.elseif(n = 8265)
	; 2758 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8266)
	; 2756 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8268)
	; 2756 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8269)
	; 2760 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8271)
	; 2760 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8272)
	; 2758 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8274)
	; 2758 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8275)
	; 2762 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8277)
	; 2762 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8278)
	; 2760 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8280)
	; 2760 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8281)
	; 2764 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.endif
.endif
.else
.if((n >= 8282) && (n < 8360))
.if((n >= 8282) && (n < 8321))
.if((n >= 8282) && (n < 8301))
; 8282 to 8300
.if((n .mod 3 = 2) && ((n > 8281 && n < 8301)))
	; 2761..2769 bytes
	delay_n (n-2)
	nop
.elseif(n = 8283)
	; 2764 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8284)
	; 2762 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8286)
	; 2762 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8287)
	; 2766 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8289)
	; 2766 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8290)
	; 2764 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8292)
	; 2764 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8293)
	; 2768 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8295)
	; 2768 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8296)
	; 2766 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8298)
	; 2766 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8299)
	; 2770 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 8301 to 8320
.if(n = 8301)
	; 2770 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8302)
	; 2768 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 8302 && n < 8319)))
	; 2769..2775 bytes
	delay_n (n-2)
	nop
.elseif(n = 8304)
	; 2768 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8305)
	; 2772 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8307)
	; 2772 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8308)
	; 2770 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8310)
	; 2770 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8311)
	; 2774 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8313)
	; 2774 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2776 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8319)
	; 2776 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8320)
	; 2774 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 8321) && (n < 8340))
; 8321 to 8339
.if((n .mod 3 = 2) && ((n > 8320 && n < 8340)))
	; 2775..2783 bytes
	delay_n (n-2)
	nop
.elseif(n = 8322)
	; 2774 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8323)
	; 2778 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8325)
	; 2778 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8326)
	; 2776 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8328)
	; 2776 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8329)
	; 2780 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8331)
	; 2780 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8332)
	; 2778 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8334)
	; 2778 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8335)
	; 2782 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8337)
	; 2782 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8338)
	; 2780 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 8340 to 8359
.if(n = 8340)
	; 2780 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8341)
	; 2784 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 8341 && n < 8358)))
	; 2781..2789 bytes
	delay_n (n-2)
	nop
.elseif(n = 8343)
	; 2784 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8344)
	; 2782 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8346)
	; 2782 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8347)
	; 2786 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8349)
	; 2786 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8350)
	; 2784 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8352)
	; 2784 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8353)
	; 2788 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8355)
	; 2788 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8356)
	; 2786 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8358)
	; 2786 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8359)
	; 2790 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.else
.if((n >= 8360) && (n < 8399))
.if((n >= 8360) && (n < 8379))
; 8360 to 8378
.if((n .mod 3 = 2) && ((n > 8359 && n < 8379)))
	; 2787..2795 bytes
	delay_n (n-2)
	nop
.elseif(n = 8361)
	; 2790 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8362)
	; 2788 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8364)
	; 2788 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8365)
	; 2792 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8367)
	; 2792 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2794 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8373)
	; 2794 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8374)
	; 2792 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8376)
	; 2792 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8377)
	; 2796 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 8379 to 8398
.if(n = 8379)
	; 2796 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8380)
	; 2794 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 8380 && n < 8397)))
	; 2795..2801 bytes
	delay_n (n-2)
	nop
.elseif(n = 8382)
	; 2794 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8383)
	; 2798 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8385)
	; 2798 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8386)
	; 2796 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8388)
	; 2796 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8389)
	; 2800 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8391)
	; 2800 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8392)
	; 2798 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8394)
	; 2798 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8395)
	; 2802 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8397)
	; 2802 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8398)
	; 2800 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 8399) && (n < 8418))
; 8399 to 8417
.if((n .mod 3 = 2) && ((n > 8398 && n < 8418)))
	; 2801..2809 bytes
	delay_n (n-2)
	nop
.elseif(n = 8400)
	; 2800 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8401)
	; 2804 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8403)
	; 2804 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8404)
	; 2802 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8406)
	; 2802 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8407)
	; 2806 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8409)
	; 2806 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8410)
	; 2804 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8412)
	; 2804 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8413)
	; 2808 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8415)
	; 2808 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8416)
	; 2806 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 8418 to 8437
.if(n = 8418)
	; 2806 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8419)
	; 2810 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 8419 && n < 8436)))
	; 2807..2815 bytes
	delay_n (n-2)
	nop
.elseif(n = 8421)
	; 2810 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2812 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8427)
	; 2812 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8428)
	; 2810 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8430)
	; 2810 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8431)
	; 2814 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8433)
	; 2814 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8434)
	; 2812 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8436)
	; 2812 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8437)
	; 2816 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
.if((n >= 8438) && (n < 8457))
; 8438 to 8456
.if((n .mod 3 = 2) && ((n > 8437 && n < 8457)))
	; 2813..2821 bytes
	delay_n (n-2)
	nop
.elseif(n = 8439)
	; 2816 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8440)
	; 2814 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8442)
	; 2814 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8443)
	; 2818 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8445)
	; 2818 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8446)
	; 2816 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8448)
	; 2816 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8449)
	; 2820 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8451)
	; 2820 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8452)
	; 2818 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8454)
	; 2818 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8455)
	; 2822 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 8457 to 8476
.if(n = 8457)
	; 2822 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8458)
	; 2820 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 8458 && n < 8475)))
	; 2821..2827 bytes
	delay_n (n-2)
	nop
.elseif(n = 8460)
	; 2820 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8461)
	; 2824 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8463)
	; 2824 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8464)
	; 2822 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8466)
	; 2822 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8467)
	; 2826 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8469)
	; 2826 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8470)
	; 2824 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8472)
	; 2824 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8473)
	; 2828 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8475)
	; 2828 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 8476)
	; 2826 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 8477) && (n < 8496))
; 8477 to 8495
.if((n .mod 3 = 2) && ((n > 8476 && n < 8496)))
	; 2827..2835 bytes
	delay_n (n-2)
	nop
.elseif(n = 8478)
	; 2826 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8479)
	; 2830 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8481)
	; 2830 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8482)
	; 2828 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8484)
	; 2828 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8485)
	; 2832 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8487)
	; 2832 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8488)
	; 2830 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8490)
	; 2830 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8491)
	; 2834 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8493)
	; 2834 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8494)
	; 2832 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 8496 to 8515
.if(n = 8496)
	; 2832 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8497)
	; 2836 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 8497 && n < 8514)))
	; 2833..2841 bytes
	delay_n (n-2)
	nop
.elseif(n = 8499)
	; 2836 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8500)
	; 2834 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8502)
	; 2834 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8503)
	; 2838 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8505)
	; 2838 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8506)
	; 2836 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8508)
	; 2836 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8509)
	; 2840 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8511)
	; 2840 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8512)
	; 2838 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8514)
	; 2838 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8515)
	; 2842 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 8516) && (n < 8555))
.if((n >= 8516) && (n < 8535))
; 8516 to 8534
.if((n .mod 3 = 2) && ((n > 8515 && n < 8535)))
	; 2839..2847 bytes
	delay_n (n-2)
	nop
.elseif(n = 8517)
	; 2842 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8518)
	; 2840 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8520)
	; 2840 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8521)
	; 2844 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8523)
	; 2844 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8524)
	; 2842 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8526)
	; 2842 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8527)
	; 2846 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8529)
	; 2846 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2848 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 8535 to 8554
.if(n = 8535)
	; 2848 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8536)
	; 2846 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 8536 && n < 8553)))
	; 2847..2853 bytes
	delay_n (n-2)
	nop
.elseif(n = 8538)
	; 2846 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8539)
	; 2850 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8541)
	; 2850 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8542)
	; 2848 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8544)
	; 2848 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8545)
	; 2852 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8547)
	; 2852 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8548)
	; 2850 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8550)
	; 2850 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8551)
	; 2854 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8553)
	; 2854 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8554)
	; 2852 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 8555) && (n < 8574))
; 8555 to 8573
.if((n .mod 3 = 2) && ((n > 8554 && n < 8574)))
	; 2853..2861 bytes
	delay_n (n-2)
	nop
.elseif(n = 8556)
	; 2852 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8557)
	; 2856 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8559)
	; 2856 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8560)
	; 2854 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8562)
	; 2854 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8563)
	; 2858 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8565)
	; 2858 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8566)
	; 2856 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8568)
	; 2856 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8569)
	; 2860 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8571)
	; 2860 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8572)
	; 2858 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 8574 to 8593
.if(n = 8574)
	; 2858 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8575)
	; 2862 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 8575 && n < 8592)))
	; 2859..2867 bytes
	delay_n (n-2)
	nop
.elseif(n = 8577)
	; 2862 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8578)
	; 2860 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8580)
	; 2860 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8581)
	; 2864 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8583)
	; 2864 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2866 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8589)
	; 2866 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8590)
	; 2864 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8592)
	; 2864 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8593)
	; 2868 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
.if((n >= 8594) && (n < 8633))
.if((n >= 8594) && (n < 8613))
; 8594 to 8612
.if((n .mod 3 = 2) && ((n > 8593 && n < 8613)))
	; 2865..2873 bytes
	delay_n (n-2)
	nop
.elseif(n = 8595)
	; 2868 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8596)
	; 2866 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8598)
	; 2866 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8599)
	; 2870 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8601)
	; 2870 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8602)
	; 2868 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8604)
	; 2868 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8605)
	; 2872 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8607)
	; 2872 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8608)
	; 2870 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8610)
	; 2870 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8611)
	; 2874 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 8613 to 8632
.if(n = 8613)
	; 2874 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8614)
	; 2872 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 8614 && n < 8631)))
	; 2873..2879 bytes
	delay_n (n-2)
	nop
.elseif(n = 8616)
	; 2872 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8617)
	; 2876 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8619)
	; 2876 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8620)
	; 2874 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8622)
	; 2874 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8623)
	; 2878 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8625)
	; 2878 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8626)
	; 2876 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8628)
	; 2876 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8629)
	; 2880 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8631)
	; 2880 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8632)
	; 2878 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 8633) && (n < 8652))
; 8633 to 8651
.if((n .mod 3 = 2) && ((n > 8632 && n < 8652)))
	; 2879..2887 bytes
	delay_n (n-2)
	nop
.elseif(n = 8634)
	; 2878 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8635)
	; 2882 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8637)
	; 2882 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2884 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8643)
	; 2884 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8644)
	; 2882 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8646)
	; 2882 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8647)
	; 2886 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8649)
	; 2886 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8650)
	; 2884 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 8652 to 8671
.if(n = 8652)
	; 2884 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8653)
	; 2888 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 8653 && n < 8670)))
	; 2885..2893 bytes
	delay_n (n-2)
	nop
.elseif(n = 8655)
	; 2888 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8656)
	; 2886 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8658)
	; 2886 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8659)
	; 2890 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8661)
	; 2890 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8662)
	; 2888 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8664)
	; 2888 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8665)
	; 2892 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8667)
	; 2892 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8668)
	; 2890 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8670)
	; 2890 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8671)
	; 2894 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.endif
.else
.if((n >= 8672) && (n < 8711))
.if((n >= 8672) && (n < 8691))
; 8672 to 8690
.if((n .mod 3 = 2) && ((n > 8671 && n < 8691)))
	; 2891..2899 bytes
	delay_n (n-2)
	nop
.elseif(n = 8673)
	; 2894 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8674)
	; 2892 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8676)
	; 2892 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8677)
	; 2896 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8679)
	; 2896 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8680)
	; 2894 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8682)
	; 2894 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8683)
	; 2898 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8685)
	; 2898 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8686)
	; 2896 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8688)
	; 2896 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8689)
	; 2900 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 8691 to 8710
.if(n = 8691)
	; 2900 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 8692)
	; 2898 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 8692 && n < 8709)))
	; 2899..2905 bytes
	delay_n (n-2)
	nop
.elseif(n = 8694)
	; 2898 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8695)
	; 2902 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8697)
	; 2902 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8698)
	; 2900 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8700)
	; 2900 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8701)
	; 2904 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8703)
	; 2904 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8704)
	; 2902 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8706)
	; 2902 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8707)
	; 2906 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8709)
	; 2906 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8710)
	; 2904 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 8711) && (n < 8731))
; 8711 to 8730
.if((n .mod 3 = 2) && ((n > 8710 && n < 8730)))
	; 2905..2913 bytes
	delay_n (n-2)
	nop
.elseif(n = 8712)
	; 2904 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8713)
	; 2908 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8715)
	; 2908 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8716)
	; 2906 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8718)
	; 2906 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8719)
	; 2910 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8721)
	; 2910 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8722)
	; 2908 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8724)
	; 2908 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8725)
	; 2912 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8727)
	; 2912 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8728)
	; 2910 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8730)
	; 2910 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 8731 to 8750
.if(n = 8731)
	; 2914 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 8731 && n < 8751)))
	; 2911..2919 bytes
	delay_n (n-2)
	nop
.elseif(n = 8733)
	; 2914 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8734)
	; 2912 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8736)
	; 2912 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8737)
	; 2916 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8739)
	; 2916 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8740)
	; 2914 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8742)
	; 2914 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8743)
	; 2918 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8745)
	; 2918 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2920 bytes
	delay_n 55
	delay_n (n-55)
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
.if((n >= 8751) && (n < 8829))
.if((n >= 8751) && (n < 8790))
.if((n >= 8751) && (n < 8770))
; 8751 to 8769
.if(n = 8751)
	; 2920 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8752)
	; 2918 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 8752 && n < 8769)))
	; 2919..2925 bytes
	delay_n (n-2)
	nop
.elseif(n = 8754)
	; 2918 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8755)
	; 2922 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8757)
	; 2922 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8758)
	; 2920 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8760)
	; 2920 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8761)
	; 2924 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8763)
	; 2924 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8764)
	; 2922 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8766)
	; 2922 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8767)
	; 2926 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8769)
	; 2926 bytes
	delay_n 129
	delay_n (n-129)
.endif
.else
; 8770 to 8789
.if(n = 8770)
	; 2924 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 8770 && n < 8790)))
	; 2925..2933 bytes
	delay_n (n-2)
	nop
.elseif(n = 8772)
	; 2924 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8773)
	; 2928 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8775)
	; 2928 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8776)
	; 2926 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8778)
	; 2926 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8779)
	; 2930 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8781)
	; 2930 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8782)
	; 2928 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8784)
	; 2928 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8785)
	; 2932 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8787)
	; 2932 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8788)
	; 2930 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 8790) && (n < 8809))
; 8790 to 8808
.if(n = 8790)
	; 2930 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8791)
	; 2934 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 8791 && n < 8808)))
	; 2931..2939 bytes
	delay_n (n-2)
	nop
.elseif(n = 8793)
	; 2934 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8794)
	; 2932 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8796)
	; 2932 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8797)
	; 2936 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8799)
	; 2936 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2938 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8805)
	; 2938 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8806)
	; 2936 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8808)
	; 2936 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 8809 to 8828
.if(n = 8809)
	; 2940 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 8809 && n < 8829)))
	; 2937..2945 bytes
	delay_n (n-2)
	nop
.elseif(n = 8811)
	; 2940 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8812)
	; 2938 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8814)
	; 2938 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8815)
	; 2942 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8817)
	; 2942 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8818)
	; 2940 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8820)
	; 2940 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8821)
	; 2944 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8823)
	; 2944 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8824)
	; 2942 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8826)
	; 2942 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8827)
	; 2946 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.else
.if((n >= 8829) && (n < 8868))
.if((n >= 8829) && (n < 8848))
; 8829 to 8847
.if(n = 8829)
	; 2946 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8830)
	; 2944 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 8830 && n < 8847)))
	; 2945..2951 bytes
	delay_n (n-2)
	nop
.elseif(n = 8832)
	; 2944 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8833)
	; 2948 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8835)
	; 2948 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8836)
	; 2946 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8838)
	; 2946 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8839)
	; 2950 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8841)
	; 2950 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8842)
	; 2948 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8844)
	; 2948 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8845)
	; 2952 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8847)
	; 2952 bytes
	delay_n 45
	delay_n (n-45)
.endif
.else
; 8848 to 8867
.if(n = 8848)
	; 2950 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 8848 && n < 8868)))
	; 2951..2959 bytes
	delay_n (n-2)
	nop
.elseif(n = 8850)
	; 2950 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8851)
	; 2954 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8853)
	; 2954 bytes
	delay_n 105
	delay_n (n-105)
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
	; 2956 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8859)
	; 2956 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8860)
	; 2954 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8862)
	; 2954 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8863)
	; 2958 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8865)
	; 2958 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8866)
	; 2956 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 8868) && (n < 8887))
; 8868 to 8886
.if(n = 8868)
	; 2956 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8869)
	; 2960 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 8869 && n < 8886)))
	; 2957..2965 bytes
	delay_n (n-2)
	nop
.elseif(n = 8871)
	; 2960 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8872)
	; 2958 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8874)
	; 2958 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8875)
	; 2962 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8877)
	; 2962 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8878)
	; 2960 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8880)
	; 2960 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8881)
	; 2964 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8883)
	; 2964 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8884)
	; 2962 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8886)
	; 2962 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 8887 to 8906
.if(n = 8887)
	; 2966 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 8887 && n < 8907)))
	; 2963..2971 bytes
	delay_n (n-2)
	nop
.elseif(n = 8889)
	; 2966 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8890)
	; 2964 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8892)
	; 2964 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8893)
	; 2968 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8895)
	; 2968 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8896)
	; 2966 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8898)
	; 2966 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8899)
	; 2970 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8901)
	; 2970 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8902)
	; 2968 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8904)
	; 2968 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8905)
	; 2972 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.endif
.else
.if((n >= 8907) && (n < 8985))
.if((n >= 8907) && (n < 8946))
.if((n >= 8907) && (n < 8926))
; 8907 to 8925
.if(n = 8907)
	; 2972 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 8908)
	; 2970 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 8908 && n < 8925)))
	; 2971..2977 bytes
	delay_n (n-2)
	nop
.elseif(n = 8910)
	; 2970 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 8911)
	; 2974 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 8913)
	; 2974 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8914)
	; 2972 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8916)
	; 2972 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8917)
	; 2976 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8919)
	; 2976 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8920)
	; 2974 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8922)
	; 2974 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8923)
	; 2978 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8925)
	; 2978 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 8926 to 8945
.if(n = 8926)
	; 2976 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 8926 && n < 8946)))
	; 2977..2985 bytes
	delay_n (n-2)
	nop
.elseif(n = 8928)
	; 2976 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8929)
	; 2980 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8931)
	; 2980 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8932)
	; 2978 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 8934)
	; 2978 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8935)
	; 2982 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8937)
	; 2982 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8938)
	; 2980 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8940)
	; 2980 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8941)
	; 2984 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8943)
	; 2984 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8944)
	; 2982 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 8946) && (n < 8965))
; 8946 to 8964
.if(n = 8946)
	; 2982 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8947)
	; 2986 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 8947 && n < 8964)))
	; 2983..2991 bytes
	delay_n (n-2)
	nop
.elseif(n = 8949)
	; 2986 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 8950)
	; 2984 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 8952)
	; 2984 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8953)
	; 2988 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8955)
	; 2988 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 8956)
	; 2986 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8958)
	; 2986 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 8959)
	; 2990 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 8961)
	; 2990 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 8962)
	; 2988 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8964)
	; 2988 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 8965 to 8984
.if(n = 8965)
	; 2992 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 8965 && n < 8985)))
	; 2989..2997 bytes
	delay_n (n-2)
	nop
.elseif(n = 8967)
	; 2992 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 8968)
	; 2990 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 8970)
	; 2990 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 8971)
	; 2994 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 8973)
	; 2994 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 8974)
	; 2992 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8976)
	; 2992 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8977)
	; 2996 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 8979)
	; 2996 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8980)
	; 2994 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 8982)
	; 2994 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 8983)
	; 2998 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.endif
.else
.if((n >= 8985) && (n < 9024))
.if((n >= 8985) && (n < 9004))
; 8985 to 9003
.if(n = 8985)
	; 2998 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 8986)
	; 2996 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 8986 && n < 9003)))
	; 2997..3003 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988)
	; 2996 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 8989)
	; 3000 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 8991)
	; 3000 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8992)
	; 2998 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 8994)
	; 2998 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 8995)
	; 3002 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 8997)
	; 3002 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8998)
	; 3000 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9000)
	; 3000 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9001)
	; 3004 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9003)
	; 3004 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 9004 to 9023
.if(n = 9004)
	; 3002 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 9004 && n < 9024)))
	; 3003..3011 bytes
	delay_n (n-2)
	nop
.elseif(n = 9006)
	; 3002 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9007)
	; 3006 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9009)
	; 3006 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9010)
	; 3004 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9012)
	; 3004 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9013)
	; 3008 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9015)
	; 3008 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3010 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9021)
	; 3010 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9022)
	; 3008 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 9024) && (n < 9043))
; 9024 to 9042
.if(n = 9024)
	; 3008 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9025)
	; 3012 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 9025 && n < 9042)))
	; 3009..3017 bytes
	delay_n (n-2)
	nop
.elseif(n = 9027)
	; 3012 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9028)
	; 3010 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9030)
	; 3010 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9031)
	; 3014 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9033)
	; 3014 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9034)
	; 3012 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9036)
	; 3012 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9037)
	; 3016 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9039)
	; 3016 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9040)
	; 3014 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9042)
	; 3014 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 9043 to 9062
.if(n = 9043)
	; 3018 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 9043 && n < 9063)))
	; 3015..3023 bytes
	delay_n (n-2)
	nop
.elseif(n = 9045)
	; 3018 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9046)
	; 3016 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9048)
	; 3016 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9049)
	; 3020 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9051)
	; 3020 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9052)
	; 3018 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9054)
	; 3018 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9055)
	; 3022 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9057)
	; 3022 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9058)
	; 3020 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9060)
	; 3020 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9061)
	; 3024 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
.if((n >= 9063) && (n < 9102))
.if((n >= 9063) && (n < 9082))
; 9063 to 9081
.if(n = 9063)
	; 3024 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9064)
	; 3022 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 9064 && n < 9081)))
	; 3023..3029 bytes
	delay_n (n-2)
	nop
.elseif(n = 9066)
	; 3022 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9067)
	; 3026 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9069)
	; 3026 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3028 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9075)
	; 3028 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9076)
	; 3026 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9078)
	; 3026 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9079)
	; 3030 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9081)
	; 3030 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 9082 to 9101
.if(n = 9082)
	; 3028 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 9082 && n < 9102)))
	; 3029..3037 bytes
	delay_n (n-2)
	nop
.elseif(n = 9084)
	; 3028 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9085)
	; 3032 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9087)
	; 3032 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9088)
	; 3030 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9090)
	; 3030 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9091)
	; 3034 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9093)
	; 3034 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9094)
	; 3032 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9096)
	; 3032 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9097)
	; 3036 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9099)
	; 3036 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9100)
	; 3034 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 9102) && (n < 9121))
; 9102 to 9120
.if(n = 9102)
	; 3034 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9103)
	; 3038 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 9103 && n < 9120)))
	; 3035..3043 bytes
	delay_n (n-2)
	nop
.elseif(n = 9105)
	; 3038 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9106)
	; 3036 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9108)
	; 3036 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9109)
	; 3040 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9111)
	; 3040 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9112)
	; 3038 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9114)
	; 3038 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9115)
	; 3042 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9117)
	; 3042 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9118)
	; 3040 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9120)
	; 3040 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 9121 to 9140
.if(n = 9121)
	; 3044 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 9121 && n < 9141)))
	; 3041..3049 bytes
	delay_n (n-2)
	nop
.elseif(n = 9123)
	; 3044 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3046 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9129)
	; 3046 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9130)
	; 3044 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9132)
	; 3044 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9133)
	; 3048 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9135)
	; 3048 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9136)
	; 3046 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9138)
	; 3046 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9139)
	; 3050 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 9141) && (n < 9180))
.if((n >= 9141) && (n < 9160))
; 9141 to 9159
.if(n = 9141)
	; 3050 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9142)
	; 3048 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 9142 && n < 9159)))
	; 3049..3055 bytes
	delay_n (n-2)
	nop
.elseif(n = 9144)
	; 3048 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9145)
	; 3052 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9147)
	; 3052 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9148)
	; 3050 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9150)
	; 3050 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9151)
	; 3054 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9153)
	; 3054 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9154)
	; 3052 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9156)
	; 3052 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9157)
	; 3056 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9159)
	; 3056 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 9160 to 9179
.if(n = 9160)
	; 3054 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 9160 && n < 9180)))
	; 3055..3063 bytes
	delay_n (n-2)
	nop
.elseif(n = 9162)
	; 3054 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9163)
	; 3058 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9165)
	; 3058 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9166)
	; 3056 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9168)
	; 3056 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9169)
	; 3060 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9171)
	; 3060 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9172)
	; 3058 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9174)
	; 3058 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9175)
	; 3062 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9177)
	; 3062 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 9178)
	; 3060 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 9180) && (n < 9199))
; 9180 to 9198
.if(n = 9180)
	; 3060 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9181)
	; 3064 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 9181 && n < 9198)))
	; 3061..3069 bytes
	delay_n (n-2)
	nop
.elseif(n = 9183)
	; 3064 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9184)
	; 3062 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9186)
	; 3062 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9187)
	; 3066 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9189)
	; 3066 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9190)
	; 3064 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9192)
	; 3064 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9193)
	; 3068 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9195)
	; 3068 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9196)
	; 3066 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9198)
	; 3066 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 9199 to 9218
.if(n = 9199)
	; 3070 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 9199 && n < 9219)))
	; 3067..3075 bytes
	delay_n (n-2)
	nop
.elseif(n = 9201)
	; 3070 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9202)
	; 3068 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9204)
	; 3068 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9205)
	; 3072 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9207)
	; 3072 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9208)
	; 3070 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9210)
	; 3070 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9211)
	; 3074 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9213)
	; 3074 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9214)
	; 3072 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9216)
	; 3072 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9217)
	; 3076 bytes
	delay_n 91
	delay_n (n-91)
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
	; 3076 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9220)
	; 3074 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 9220 && n < 9237)))
	; 3075..3081 bytes
	delay_n (n-2)
	nop
.elseif(n = 9222)
	; 3074 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9223)
	; 3078 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9225)
	; 3078 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9226)
	; 3076 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9228)
	; 3076 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9229)
	; 3080 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9231)
	; 3080 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3082 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9237)
	; 3082 bytes
	delay_n 57
	delay_n (n-57)
.endif
.else
; 9238 to 9257
.if(n = 9238)
	; 3080 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 9238 && n < 9258)))
	; 3081..3089 bytes
	delay_n (n-2)
	nop
.elseif(n = 9240)
	; 3080 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9241)
	; 3084 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9243)
	; 3084 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9244)
	; 3082 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9246)
	; 3082 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9247)
	; 3086 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9249)
	; 3086 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9250)
	; 3084 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9252)
	; 3084 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9253)
	; 3088 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9255)
	; 3088 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9256)
	; 3086 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 9258) && (n < 9277))
; 9258 to 9276
.if(n = 9258)
	; 3086 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9259)
	; 3090 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 9259 && n < 9276)))
	; 3087..3095 bytes
	delay_n (n-2)
	nop
.elseif(n = 9261)
	; 3090 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9262)
	; 3088 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9264)
	; 3088 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9265)
	; 3092 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9267)
	; 3092 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9268)
	; 3090 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9270)
	; 3090 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9271)
	; 3094 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9273)
	; 3094 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9274)
	; 3092 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9276)
	; 3092 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 9277 to 9296
.if(n = 9277)
	; 3096 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 9277 && n < 9297)))
	; 3093..3101 bytes
	delay_n (n-2)
	nop
.elseif(n = 9279)
	; 3096 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9280)
	; 3094 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9282)
	; 3094 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9283)
	; 3098 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9285)
	; 3098 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3100 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9291)
	; 3100 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9292)
	; 3098 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9294)
	; 3098 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9295)
	; 3102 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.endif
.else
.if((n >= 9297) && (n < 9336))
.if((n >= 9297) && (n < 9316))
; 9297 to 9315
.if(n = 9297)
	; 3102 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9298)
	; 3100 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 9298 && n < 9315)))
	; 3101..3107 bytes
	delay_n (n-2)
	nop
.elseif(n = 9300)
	; 3100 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9301)
	; 3104 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9303)
	; 3104 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9304)
	; 3102 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9306)
	; 3102 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9307)
	; 3106 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9309)
	; 3106 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9310)
	; 3104 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9312)
	; 3104 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9313)
	; 3108 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9315)
	; 3108 bytes
	delay_n 81
	delay_n (n-81)
.endif
.else
; 9316 to 9335
.if(n = 9316)
	; 3106 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 9316 && n < 9336)))
	; 3107..3115 bytes
	delay_n (n-2)
	nop
.elseif(n = 9318)
	; 3106 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9319)
	; 3110 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9321)
	; 3110 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9322)
	; 3108 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9324)
	; 3108 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9325)
	; 3112 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9327)
	; 3112 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9328)
	; 3110 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9330)
	; 3110 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9331)
	; 3114 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9333)
	; 3114 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9334)
	; 3112 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 9336) && (n < 9356))
; 9336 to 9355
.if(n = 9336)
	; 3112 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9337)
	; 3116 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 9337 && n < 9354)))
	; 3113..3121 bytes
	delay_n (n-2)
	nop
.elseif(n = 9339)
	; 3116 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3118 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9345)
	; 3118 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9346)
	; 3116 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9348)
	; 3116 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9349)
	; 3120 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9351)
	; 3120 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9352)
	; 3118 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9354)
	; 3118 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9355)
	; 3122 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 9356 to 9375
.if((n .mod 3 = 2) && ((n > 9355 && n < 9375)))
	; 3119..3127 bytes
	delay_n (n-2)
	nop
.elseif(n = 9357)
	; 3122 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9358)
	; 3120 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9360)
	; 3120 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9361)
	; 3124 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9363)
	; 3124 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9364)
	; 3122 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9366)
	; 3122 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9367)
	; 3126 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9369)
	; 3126 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9370)
	; 3124 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9372)
	; 3124 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9373)
	; 3128 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9375)
	; 3128 bytes
	delay_n 141
	delay_n (n-141)
.endif
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
.if((n >= 9376) && (n < 9395))
; 9376 to 9394
.if(n = 9376)
	; 3126 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 9376 && n < 9393)))
	; 3127..3133 bytes
	delay_n (n-2)
	nop
.elseif(n = 9378)
	; 3126 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9379)
	; 3130 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9381)
	; 3130 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9382)
	; 3128 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9384)
	; 3128 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9385)
	; 3132 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9387)
	; 3132 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9388)
	; 3130 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9390)
	; 3130 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9391)
	; 3134 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9393)
	; 3134 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 9394)
	; 3132 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 9395 to 9414
.if((n .mod 3 = 2) && ((n > 9394 && n < 9414)))
	; 3133..3141 bytes
	delay_n (n-2)
	nop
.elseif(n = 9396)
	; 3132 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9397)
	; 3136 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9399)
	; 3136 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9400)
	; 3134 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9402)
	; 3134 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9403)
	; 3138 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9405)
	; 3138 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9406)
	; 3136 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9408)
	; 3136 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9409)
	; 3140 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9411)
	; 3140 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9412)
	; 3138 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9414)
	; 3138 bytes
	delay_n 72
	delay_n (n-72)
.endif
.endif
.else
.if((n >= 9415) && (n < 9434))
; 9415 to 9433
.if(n = 9415)
	; 3142 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 9415 && n < 9432)))
	; 3139..3147 bytes
	delay_n (n-2)
	nop
.elseif(n = 9417)
	; 3142 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9418)
	; 3140 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9420)
	; 3140 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9421)
	; 3144 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9423)
	; 3144 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9424)
	; 3142 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9426)
	; 3142 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9427)
	; 3146 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9429)
	; 3146 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9430)
	; 3144 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9432)
	; 3144 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9433)
	; 3148 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 9434 to 9453
.if((n .mod 3 = 2) && ((n > 9433 && n < 9453)))
	; 3145..3153 bytes
	delay_n (n-2)
	nop
.elseif(n = 9435)
	; 3148 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9436)
	; 3146 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9438)
	; 3146 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9439)
	; 3150 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9441)
	; 3150 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9442)
	; 3148 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9444)
	; 3148 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9445)
	; 3152 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9447)
	; 3152 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3154 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9453)
	; 3154 bytes
	delay_n 57
	delay_n (n-57)
.endif
.endif
.endif
.else
.if((n >= 9454) && (n < 9493))
.if((n >= 9454) && (n < 9473))
; 9454 to 9472
.if(n = 9454)
	; 3152 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 9454 && n < 9471)))
	; 3153..3159 bytes
	delay_n (n-2)
	nop
.elseif(n = 9456)
	; 3152 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9457)
	; 3156 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9459)
	; 3156 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9460)
	; 3154 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9462)
	; 3154 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9463)
	; 3158 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9465)
	; 3158 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9466)
	; 3156 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9468)
	; 3156 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9469)
	; 3160 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9471)
	; 3160 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9472)
	; 3158 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 9473 to 9492
.if((n .mod 3 = 2) && ((n > 9472 && n < 9492)))
	; 3159..3167 bytes
	delay_n (n-2)
	nop
.elseif(n = 9474)
	; 3158 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9475)
	; 3162 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9477)
	; 3162 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9478)
	; 3160 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9480)
	; 3160 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9481)
	; 3164 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9483)
	; 3164 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9484)
	; 3162 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9486)
	; 3162 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9487)
	; 3166 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9489)
	; 3166 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9490)
	; 3164 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9492)
	; 3164 bytes
	delay_n 96
	delay_n (n-96)
.endif
.endif
.else
.if((n >= 9493) && (n < 9512))
; 9493 to 9511
.if(n = 9493)
	; 3168 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 9493 && n < 9510)))
	; 3165..3173 bytes
	delay_n (n-2)
	nop
.elseif(n = 9495)
	; 3168 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9496)
	; 3166 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9498)
	; 3166 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9499)
	; 3170 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9501)
	; 3170 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3172 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9507)
	; 3172 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9508)
	; 3170 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9510)
	; 3170 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9511)
	; 3174 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 9512 to 9531
.if((n .mod 3 = 2) && ((n > 9511 && n < 9531)))
	; 3171..3179 bytes
	delay_n (n-2)
	nop
.elseif(n = 9513)
	; 3174 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9514)
	; 3172 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9516)
	; 3172 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9517)
	; 3176 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9519)
	; 3176 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9520)
	; 3174 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9522)
	; 3174 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9523)
	; 3178 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9525)
	; 3178 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9526)
	; 3176 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9528)
	; 3176 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9529)
	; 3180 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9531)
	; 3180 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.endif
.endif
.else
.if((n >= 9532) && (n < 9610))
.if((n >= 9532) && (n < 9571))
.if((n >= 9532) && (n < 9551))
; 9532 to 9550
.if(n = 9532)
	; 3178 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 9532 && n < 9549)))
	; 3179..3185 bytes
	delay_n (n-2)
	nop
.elseif(n = 9534)
	; 3178 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9535)
	; 3182 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9537)
	; 3182 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9538)
	; 3180 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9540)
	; 3180 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9541)
	; 3184 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9543)
	; 3184 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9544)
	; 3182 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9546)
	; 3182 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9547)
	; 3186 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9549)
	; 3186 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9550)
	; 3184 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 9551 to 9570
.if((n .mod 3 = 2) && ((n > 9550 && n < 9570)))
	; 3185..3193 bytes
	delay_n (n-2)
	nop
.elseif(n = 9552)
	; 3184 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9553)
	; 3188 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9555)
	; 3188 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3190 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9561)
	; 3190 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9562)
	; 3188 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9564)
	; 3188 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9565)
	; 3192 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9567)
	; 3192 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9568)
	; 3190 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9570)
	; 3190 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 9571) && (n < 9590))
; 9571 to 9589
.if(n = 9571)
	; 3194 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 9571 && n < 9588)))
	; 3191..3199 bytes
	delay_n (n-2)
	nop
.elseif(n = 9573)
	; 3194 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9574)
	; 3192 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9576)
	; 3192 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9577)
	; 3196 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9579)
	; 3196 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9580)
	; 3194 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9582)
	; 3194 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9583)
	; 3198 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9585)
	; 3198 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9586)
	; 3196 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9588)
	; 3196 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9589)
	; 3200 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 9590 to 9609
.if((n .mod 3 = 2) && ((n > 9589 && n < 9609)))
	; 3197..3205 bytes
	delay_n (n-2)
	nop
.elseif(n = 9591)
	; 3200 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9592)
	; 3198 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9594)
	; 3198 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9595)
	; 3202 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9597)
	; 3202 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9598)
	; 3200 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9600)
	; 3200 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9601)
	; 3204 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9603)
	; 3204 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9604)
	; 3202 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9606)
	; 3202 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9607)
	; 3206 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9609)
	; 3206 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.endif
.else
.if((n >= 9610) && (n < 9649))
.if((n >= 9610) && (n < 9629))
; 9610 to 9628
.if(n = 9610)
	; 3204 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 9610 && n < 9627)))
	; 3205..3211 bytes
	delay_n (n-2)
	nop
.elseif(n = 9612)
	; 3204 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 9613)
	; 3208 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9615)
	; 3208 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9616)
	; 3206 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9618)
	; 3206 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9619)
	; 3210 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9621)
	; 3210 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9622)
	; 3208 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9624)
	; 3208 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9625)
	; 3212 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9627)
	; 3212 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9628)
	; 3210 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 9629 to 9648
.if((n .mod 3 = 2) && ((n > 9628 && n < 9648)))
	; 3211..3219 bytes
	delay_n (n-2)
	nop
.elseif(n = 9630)
	; 3210 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9631)
	; 3214 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9633)
	; 3214 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9634)
	; 3212 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9636)
	; 3212 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9637)
	; 3216 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9639)
	; 3216 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9640)
	; 3214 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9642)
	; 3214 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9643)
	; 3218 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9645)
	; 3218 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9646)
	; 3216 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9648)
	; 3216 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.else
.if((n >= 9649) && (n < 9668))
; 9649 to 9667
.if(n = 9649)
	; 3220 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 9649 && n < 9666)))
	; 3217..3225 bytes
	delay_n (n-2)
	nop
.elseif(n = 9651)
	; 3220 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9652)
	; 3218 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9654)
	; 3218 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9655)
	; 3222 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9657)
	; 3222 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9658)
	; 3220 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9660)
	; 3220 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9661)
	; 3224 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9663)
	; 3224 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3226 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 9668 to 9687
.if((n .mod 3 = 2) && ((n > 9667 && n < 9687)))
	; 3223..3231 bytes
	delay_n (n-2)
	nop
.elseif(n = 9669)
	; 3226 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9670)
	; 3224 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9672)
	; 3224 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9673)
	; 3228 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9675)
	; 3228 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9676)
	; 3226 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9678)
	; 3226 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9679)
	; 3230 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9681)
	; 3230 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9682)
	; 3228 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9684)
	; 3228 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9685)
	; 3232 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9687)
	; 3232 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 9688) && (n < 9844))
.if((n >= 9688) && (n < 9766))
.if((n >= 9688) && (n < 9727))
.if((n >= 9688) && (n < 9707))
; 9688 to 9706
.if(n = 9688)
	; 3230 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 9688 && n < 9705)))
	; 3231..3237 bytes
	delay_n (n-2)
	nop
.elseif(n = 9690)
	; 3230 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9691)
	; 3234 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9693)
	; 3234 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9694)
	; 3232 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9696)
	; 3232 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9697)
	; 3236 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9699)
	; 3236 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9700)
	; 3234 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9702)
	; 3234 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9703)
	; 3238 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9705)
	; 3238 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9706)
	; 3236 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 9707 to 9726
.if((n .mod 3 = 2) && ((n > 9706 && n < 9726)))
	; 3237..3245 bytes
	delay_n (n-2)
	nop
.elseif(n = 9708)
	; 3236 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9709)
	; 3240 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9711)
	; 3240 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9712)
	; 3238 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9714)
	; 3238 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9715)
	; 3242 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9717)
	; 3242 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3244 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9723)
	; 3244 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9724)
	; 3242 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9726)
	; 3242 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.else
.if((n >= 9727) && (n < 9746))
; 9727 to 9745
.if(n = 9727)
	; 3246 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 9727 && n < 9744)))
	; 3243..3251 bytes
	delay_n (n-2)
	nop
.elseif(n = 9729)
	; 3246 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9730)
	; 3244 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9732)
	; 3244 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9733)
	; 3248 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9735)
	; 3248 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9736)
	; 3246 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9738)
	; 3246 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9739)
	; 3250 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9741)
	; 3250 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9742)
	; 3248 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9744)
	; 3248 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9745)
	; 3252 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 9746 to 9765
.if((n .mod 3 = 2) && ((n > 9745 && n < 9765)))
	; 3249..3257 bytes
	delay_n (n-2)
	nop
.elseif(n = 9747)
	; 3252 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9748)
	; 3250 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9750)
	; 3250 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9751)
	; 3254 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9753)
	; 3254 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9754)
	; 3252 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9756)
	; 3252 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9757)
	; 3256 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9759)
	; 3256 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9760)
	; 3254 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9762)
	; 3254 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9763)
	; 3258 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9765)
	; 3258 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.endif
.else
.if((n >= 9766) && (n < 9805))
.if((n >= 9766) && (n < 9785))
; 9766 to 9784
.if(n = 9766)
	; 3256 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 9766 && n < 9783)))
	; 3257..3263 bytes
	delay_n (n-2)
	nop
.elseif(n = 9768)
	; 3256 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9769)
	; 3260 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9771)
	; 3260 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3262 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9777)
	; 3262 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9778)
	; 3260 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9780)
	; 3260 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9781)
	; 3264 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9783)
	; 3264 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9784)
	; 3262 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 9785 to 9804
.if((n .mod 3 = 2) && ((n > 9784 && n < 9804)))
	; 3263..3271 bytes
	delay_n (n-2)
	nop
.elseif(n = 9786)
	; 3262 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9787)
	; 3266 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9789)
	; 3266 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9790)
	; 3264 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9792)
	; 3264 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9793)
	; 3268 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9795)
	; 3268 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9796)
	; 3266 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9798)
	; 3266 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9799)
	; 3270 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9801)
	; 3270 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9802)
	; 3268 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9804)
	; 3268 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.else
.if((n >= 9805) && (n < 9824))
; 9805 to 9823
.if(n = 9805)
	; 3272 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 9805 && n < 9822)))
	; 3269..3277 bytes
	delay_n (n-2)
	nop
.elseif(n = 9807)
	; 3272 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9808)
	; 3270 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9810)
	; 3270 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9811)
	; 3274 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9813)
	; 3274 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9814)
	; 3272 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9816)
	; 3272 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9817)
	; 3276 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9819)
	; 3276 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9820)
	; 3274 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9822)
	; 3274 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9823)
	; 3278 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 9824 to 9843
.if((n .mod 3 = 2) && ((n > 9823 && n < 9843)))
	; 3275..3283 bytes
	delay_n (n-2)
	nop
.elseif(n = 9825)
	; 3278 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3280 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9831)
	; 3280 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9832)
	; 3278 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9834)
	; 3278 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9835)
	; 3282 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9837)
	; 3282 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9838)
	; 3280 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9840)
	; 3280 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9841)
	; 3284 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9843)
	; 3284 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
.if((n >= 9844) && (n < 9883))
.if((n >= 9844) && (n < 9863))
; 9844 to 9862
.if(n = 9844)
	; 3282 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 9844 && n < 9861)))
	; 3283..3289 bytes
	delay_n (n-2)
	nop
.elseif(n = 9846)
	; 3282 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9847)
	; 3286 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9849)
	; 3286 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9850)
	; 3284 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9852)
	; 3284 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9853)
	; 3288 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9855)
	; 3288 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9856)
	; 3286 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9858)
	; 3286 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9859)
	; 3290 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9861)
	; 3290 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9862)
	; 3288 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 9863 to 9882
.if((n .mod 3 = 2) && ((n > 9862 && n < 9882)))
	; 3289..3297 bytes
	delay_n (n-2)
	nop
.elseif(n = 9864)
	; 3288 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9865)
	; 3292 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9867)
	; 3292 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9868)
	; 3290 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9870)
	; 3290 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9871)
	; 3294 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9873)
	; 3294 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9874)
	; 3292 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9876)
	; 3292 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9877)
	; 3296 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9879)
	; 3296 bytes
	delay_n 105
	delay_n (n-105)
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
.endif
.else
.if((n >= 9883) && (n < 9902))
; 9883 to 9901
.if(n = 9883)
	; 3298 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 9883 && n < 9900)))
	; 3295..3303 bytes
	delay_n (n-2)
	nop
.elseif(n = 9885)
	; 3298 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9886)
	; 3296 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9888)
	; 3296 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9889)
	; 3300 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9891)
	; 3300 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9892)
	; 3298 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9894)
	; 3298 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9895)
	; 3302 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9897)
	; 3302 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9898)
	; 3300 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9900)
	; 3300 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9901)
	; 3304 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 9902 to 9921
.if((n .mod 3 = 2) && ((n > 9901 && n < 9921)))
	; 3301..3309 bytes
	delay_n (n-2)
	nop
.elseif(n = 9903)
	; 3304 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9904)
	; 3302 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9906)
	; 3302 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 9907)
	; 3306 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 9909)
	; 3306 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9910)
	; 3304 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9912)
	; 3304 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9913)
	; 3308 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9915)
	; 3308 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9916)
	; 3306 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9918)
	; 3306 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9919)
	; 3310 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9921)
	; 3310 bytes
	delay_n 93
	delay_n (n-93)
.endif
.endif
.endif
.else
.if((n >= 9922) && (n < 9961))
.if((n >= 9922) && (n < 9941))
; 9922 to 9940
.if(n = 9922)
	; 3308 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 9922 && n < 9939)))
	; 3309..3315 bytes
	delay_n (n-2)
	nop
.elseif(n = 9924)
	; 3308 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9925)
	; 3312 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9927)
	; 3312 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9928)
	; 3310 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9930)
	; 3310 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9931)
	; 3314 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9933)
	; 3314 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3316 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9939)
	; 3316 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9940)
	; 3314 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 9941 to 9960
.if((n .mod 3 = 2) && ((n > 9940 && n < 9960)))
	; 3315..3323 bytes
	delay_n (n-2)
	nop
.elseif(n = 9942)
	; 3314 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9943)
	; 3318 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9945)
	; 3318 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 9946)
	; 3316 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9948)
	; 3316 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9949)
	; 3320 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 9951)
	; 3320 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9952)
	; 3318 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 9954)
	; 3318 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 9955)
	; 3322 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 9957)
	; 3322 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 9958)
	; 3320 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 9960)
	; 3320 bytes
	delay_n 24
	delay_n (n-24)
.endif
.endif
.else
.if((n >= 9961) && (n < 9981))
; 9961 to 9980
.if(n = 9961)
	; 3324 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 9961 && n < 9981)))
	; 3321..3329 bytes
	delay_n (n-2)
	nop
.elseif(n = 9963)
	; 3324 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9964)
	; 3322 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 9966)
	; 3322 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 9967)
	; 3326 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 9969)
	; 3326 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9970)
	; 3324 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 9972)
	; 3324 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9973)
	; 3328 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9975)
	; 3328 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 9976)
	; 3326 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 9978)
	; 3326 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9979)
	; 3330 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 9981 to 10000
.if(n = 9981)
	; 3330 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 9982)
	; 3328 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 9982 && n < 9999)))
	; 3329..3335 bytes
	delay_n (n-2)
	nop
.elseif(n = 9984)
	; 3328 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 9985)
	; 3332 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 9987)
	; 3332 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3334 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 9993)
	; 3334 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 9994)
	; 3332 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 9996)
	; 3332 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 9997)
	; 3336 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 9999)
	; 3336 bytes
	delay_n 117
	delay_n (n-117)
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
.if((n .mod 3 = 2) && ((n > 10000 && n < 10020)))
	; 3335..3343 bytes
	delay_n (n-2)
	nop
.elseif(n = 10002)
	; 3334 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10003)
	; 3338 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10005)
	; 3338 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10006)
	; 3336 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10008)
	; 3336 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10009)
	; 3340 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10011)
	; 3340 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10012)
	; 3338 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10014)
	; 3338 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10015)
	; 3342 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10017)
	; 3342 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10018)
	; 3340 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 10020 to 10039
.if(n = 10020)
	; 3340 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10021)
	; 3344 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 10021 && n < 10038)))
	; 3341..3349 bytes
	delay_n (n-2)
	nop
.elseif(n = 10023)
	; 3344 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10024)
	; 3342 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10026)
	; 3342 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10027)
	; 3346 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10029)
	; 3346 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10030)
	; 3344 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10032)
	; 3344 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10033)
	; 3348 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10035)
	; 3348 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10036)
	; 3346 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10038)
	; 3346 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10039)
	; 3350 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 10040) && (n < 10059))
; 10040 to 10058
.if((n .mod 3 = 2) && ((n > 10039 && n < 10059)))
	; 3347..3355 bytes
	delay_n (n-2)
	nop
.elseif(n = 10041)
	; 3350 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3352 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10047)
	; 3352 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10048)
	; 3350 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10050)
	; 3350 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10051)
	; 3354 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10053)
	; 3354 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10054)
	; 3352 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10056)
	; 3352 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10057)
	; 3356 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 10059 to 10078
.if(n = 10059)
	; 3356 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10060)
	; 3354 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 10060 && n < 10077)))
	; 3355..3361 bytes
	delay_n (n-2)
	nop
.elseif(n = 10062)
	; 3354 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10063)
	; 3358 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10065)
	; 3358 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10066)
	; 3356 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10068)
	; 3356 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10069)
	; 3360 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10071)
	; 3360 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10072)
	; 3358 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10074)
	; 3358 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10075)
	; 3362 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10077)
	; 3362 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10078)
	; 3360 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.else
.if((n >= 10079) && (n < 10118))
.if((n >= 10079) && (n < 10098))
; 10079 to 10097
.if((n .mod 3 = 2) && ((n > 10078 && n < 10098)))
	; 3361..3369 bytes
	delay_n (n-2)
	nop
.elseif(n = 10080)
	; 3360 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10081)
	; 3364 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10083)
	; 3364 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10084)
	; 3362 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10086)
	; 3362 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10087)
	; 3366 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10089)
	; 3366 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10090)
	; 3364 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10092)
	; 3364 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10093)
	; 3368 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10095)
	; 3368 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 10096)
	; 3366 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 10098 to 10117
.if(n = 10098)
	; 3366 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10099)
	; 3370 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 10099 && n < 10116)))
	; 3367..3375 bytes
	delay_n (n-2)
	nop
.elseif(n = 10101)
	; 3370 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10102)
	; 3368 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10104)
	; 3368 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10105)
	; 3372 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10107)
	; 3372 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10108)
	; 3370 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10110)
	; 3370 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10111)
	; 3374 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10113)
	; 3374 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10114)
	; 3372 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10116)
	; 3372 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10117)
	; 3376 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.else
.if((n >= 10118) && (n < 10137))
; 10118 to 10136
.if((n .mod 3 = 2) && ((n > 10117 && n < 10137)))
	; 3373..3381 bytes
	delay_n (n-2)
	nop
.elseif(n = 10119)
	; 3376 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10120)
	; 3374 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10122)
	; 3374 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10123)
	; 3378 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10125)
	; 3378 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10126)
	; 3376 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10128)
	; 3376 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10129)
	; 3380 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10131)
	; 3380 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10132)
	; 3378 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10134)
	; 3378 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10135)
	; 3382 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 10137 to 10156
.if(n = 10137)
	; 3382 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10138)
	; 3380 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 10138 && n < 10155)))
	; 3381..3387 bytes
	delay_n (n-2)
	nop
.elseif(n = 10140)
	; 3380 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10141)
	; 3384 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10143)
	; 3384 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10144)
	; 3382 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10146)
	; 3382 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10147)
	; 3386 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10149)
	; 3386 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3388 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10155)
	; 3388 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10156)
	; 3386 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.endif
.else
.if((n >= 10157) && (n < 10235))
.if((n >= 10157) && (n < 10196))
.if((n >= 10157) && (n < 10176))
; 10157 to 10175
.if((n .mod 3 = 2) && ((n > 10156 && n < 10176)))
	; 3387..3395 bytes
	delay_n (n-2)
	nop
.elseif(n = 10158)
	; 3386 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10159)
	; 3390 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10161)
	; 3390 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10162)
	; 3388 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10164)
	; 3388 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10165)
	; 3392 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10167)
	; 3392 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10168)
	; 3390 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10170)
	; 3390 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10171)
	; 3394 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10173)
	; 3394 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10174)
	; 3392 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 10176 to 10195
.if(n = 10176)
	; 3392 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10177)
	; 3396 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 10177 && n < 10194)))
	; 3393..3401 bytes
	delay_n (n-2)
	nop
.elseif(n = 10179)
	; 3396 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10180)
	; 3394 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10182)
	; 3394 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10183)
	; 3398 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10185)
	; 3398 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10186)
	; 3396 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10188)
	; 3396 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10189)
	; 3400 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10191)
	; 3400 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10192)
	; 3398 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10194)
	; 3398 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10195)
	; 3402 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 10196) && (n < 10215))
; 10196 to 10214
.if((n .mod 3 = 2) && ((n > 10195 && n < 10215)))
	; 3399..3407 bytes
	delay_n (n-2)
	nop
.elseif(n = 10197)
	; 3402 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10198)
	; 3400 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10200)
	; 3400 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10201)
	; 3404 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10203)
	; 3404 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3406 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10209)
	; 3406 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10210)
	; 3404 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10212)
	; 3404 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10213)
	; 3408 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 10215 to 10234
.if(n = 10215)
	; 3408 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10216)
	; 3406 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 10216 && n < 10233)))
	; 3407..3413 bytes
	delay_n (n-2)
	nop
.elseif(n = 10218)
	; 3406 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10219)
	; 3410 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10221)
	; 3410 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10222)
	; 3408 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10224)
	; 3408 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10225)
	; 3412 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10227)
	; 3412 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10228)
	; 3410 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10230)
	; 3410 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10231)
	; 3414 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10233)
	; 3414 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10234)
	; 3412 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.else
.if((n >= 10235) && (n < 10274))
.if((n >= 10235) && (n < 10254))
; 10235 to 10253
.if((n .mod 3 = 2) && ((n > 10234 && n < 10254)))
	; 3413..3421 bytes
	delay_n (n-2)
	nop
.elseif(n = 10236)
	; 3412 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10237)
	; 3416 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10239)
	; 3416 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10240)
	; 3414 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10242)
	; 3414 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10243)
	; 3418 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10245)
	; 3418 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10246)
	; 3416 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10248)
	; 3416 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10249)
	; 3420 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10251)
	; 3420 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10252)
	; 3418 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 10254 to 10273
.if(n = 10254)
	; 3418 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10255)
	; 3422 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 10255 && n < 10272)))
	; 3419..3427 bytes
	delay_n (n-2)
	nop
.elseif(n = 10257)
	; 3422 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3424 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10263)
	; 3424 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10264)
	; 3422 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10266)
	; 3422 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10267)
	; 3426 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10269)
	; 3426 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10270)
	; 3424 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10272)
	; 3424 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10273)
	; 3428 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 10274) && (n < 10293))
; 10274 to 10292
.if((n .mod 3 = 2) && ((n > 10273 && n < 10293)))
	; 3425..3433 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275)
	; 3428 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10276)
	; 3426 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10278)
	; 3426 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10279)
	; 3430 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10281)
	; 3430 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10282)
	; 3428 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10284)
	; 3428 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10285)
	; 3432 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10287)
	; 3432 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10288)
	; 3430 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10290)
	; 3430 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10291)
	; 3434 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 10293 to 10312
.if(n = 10293)
	; 3434 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10294)
	; 3432 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 10294 && n < 10311)))
	; 3433..3439 bytes
	delay_n (n-2)
	nop
.elseif(n = 10296)
	; 3432 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10297)
	; 3436 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10299)
	; 3436 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10300)
	; 3434 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10302)
	; 3434 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10303)
	; 3438 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10305)
	; 3438 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10306)
	; 3436 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10308)
	; 3436 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10309)
	; 3440 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10311)
	; 3440 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 10312)
	; 3438 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
.if((n >= 10313) && (n < 10391))
.if((n >= 10313) && (n < 10352))
.if((n >= 10313) && (n < 10332))
; 10313 to 10331
.if((n .mod 3 = 2) && ((n > 10312 && n < 10332)))
	; 3439..3447 bytes
	delay_n (n-2)
	nop
.elseif(n = 10314)
	; 3438 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10315)
	; 3442 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10317)
	; 3442 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10318)
	; 3440 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10320)
	; 3440 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10321)
	; 3444 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10323)
	; 3444 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10324)
	; 3442 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10326)
	; 3442 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10327)
	; 3446 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10329)
	; 3446 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10330)
	; 3444 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 10332 to 10351
.if(n = 10332)
	; 3444 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10333)
	; 3448 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 10333 && n < 10350)))
	; 3445..3453 bytes
	delay_n (n-2)
	nop
.elseif(n = 10335)
	; 3448 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10336)
	; 3446 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10338)
	; 3446 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10339)
	; 3450 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10341)
	; 3450 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10342)
	; 3448 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10344)
	; 3448 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10345)
	; 3452 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10347)
	; 3452 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10348)
	; 3450 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10350)
	; 3450 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10351)
	; 3454 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 10352) && (n < 10371))
; 10352 to 10370
.if((n .mod 3 = 2) && ((n > 10351 && n < 10371)))
	; 3451..3459 bytes
	delay_n (n-2)
	nop
.elseif(n = 10353)
	; 3454 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10354)
	; 3452 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10356)
	; 3452 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10357)
	; 3456 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10359)
	; 3456 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10360)
	; 3454 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10362)
	; 3454 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10363)
	; 3458 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10365)
	; 3458 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3460 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 10371 to 10390
.if(n = 10371)
	; 3460 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10372)
	; 3458 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 10372 && n < 10389)))
	; 3459..3465 bytes
	delay_n (n-2)
	nop
.elseif(n = 10374)
	; 3458 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10375)
	; 3462 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10377)
	; 3462 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10378)
	; 3460 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10380)
	; 3460 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10381)
	; 3464 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10383)
	; 3464 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10384)
	; 3462 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10386)
	; 3462 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10387)
	; 3466 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10389)
	; 3466 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10390)
	; 3464 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.else
.if((n >= 10391) && (n < 10430))
.if((n >= 10391) && (n < 10410))
; 10391 to 10409
.if((n .mod 3 = 2) && ((n > 10390 && n < 10410)))
	; 3465..3473 bytes
	delay_n (n-2)
	nop
.elseif(n = 10392)
	; 3464 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10393)
	; 3468 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10395)
	; 3468 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10396)
	; 3466 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10398)
	; 3466 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10399)
	; 3470 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10401)
	; 3470 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10402)
	; 3468 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10404)
	; 3468 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10405)
	; 3472 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10407)
	; 3472 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10408)
	; 3470 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 10410 to 10429
.if(n = 10410)
	; 3470 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10411)
	; 3474 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 10411 && n < 10428)))
	; 3471..3479 bytes
	delay_n (n-2)
	nop
.elseif(n = 10413)
	; 3474 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10414)
	; 3472 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10416)
	; 3472 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10417)
	; 3476 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10419)
	; 3476 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3478 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10425)
	; 3478 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10426)
	; 3476 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10428)
	; 3476 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10429)
	; 3480 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.else
.if((n >= 10430) && (n < 10449))
; 10430 to 10448
.if((n .mod 3 = 2) && ((n > 10429 && n < 10449)))
	; 3477..3485 bytes
	delay_n (n-2)
	nop
.elseif(n = 10431)
	; 3480 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10432)
	; 3478 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10434)
	; 3478 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10435)
	; 3482 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10437)
	; 3482 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10438)
	; 3480 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10440)
	; 3480 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10441)
	; 3484 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10443)
	; 3484 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10444)
	; 3482 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10446)
	; 3482 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10447)
	; 3486 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 10449 to 10468
.if(n = 10449)
	; 3486 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10450)
	; 3484 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 10450 && n < 10467)))
	; 3485..3491 bytes
	delay_n (n-2)
	nop
.elseif(n = 10452)
	; 3484 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10453)
	; 3488 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10455)
	; 3488 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10456)
	; 3486 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10458)
	; 3486 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10459)
	; 3490 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10461)
	; 3490 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10462)
	; 3488 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10464)
	; 3488 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10465)
	; 3492 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10467)
	; 3492 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10468)
	; 3490 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.endif
.else
.if((n >= 10469) && (n < 10547))
.if((n >= 10469) && (n < 10508))
.if((n >= 10469) && (n < 10488))
; 10469 to 10487
.if((n .mod 3 = 2) && ((n > 10468 && n < 10488)))
	; 3491..3499 bytes
	delay_n (n-2)
	nop
.elseif(n = 10470)
	; 3490 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10471)
	; 3494 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10473)
	; 3494 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3496 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10479)
	; 3496 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10480)
	; 3494 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10482)
	; 3494 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10483)
	; 3498 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10485)
	; 3498 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10486)
	; 3496 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 10488 to 10507
.if(n = 10488)
	; 3496 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10489)
	; 3500 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 10489 && n < 10506)))
	; 3497..3505 bytes
	delay_n (n-2)
	nop
.elseif(n = 10491)
	; 3500 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10492)
	; 3498 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10494)
	; 3498 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10495)
	; 3502 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10497)
	; 3502 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10498)
	; 3500 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10500)
	; 3500 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10501)
	; 3504 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10503)
	; 3504 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10504)
	; 3502 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10506)
	; 3502 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10507)
	; 3506 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.else
.if((n >= 10508) && (n < 10527))
; 10508 to 10526
.if((n .mod 3 = 2) && ((n > 10507 && n < 10527)))
	; 3503..3511 bytes
	delay_n (n-2)
	nop
.elseif(n = 10509)
	; 3506 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10510)
	; 3504 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10512)
	; 3504 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10513)
	; 3508 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10515)
	; 3508 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10516)
	; 3506 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10518)
	; 3506 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10519)
	; 3510 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10521)
	; 3510 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10522)
	; 3508 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10524)
	; 3508 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10525)
	; 3512 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 10527 to 10546
.if(n = 10527)
	; 3512 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 10528)
	; 3510 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 10528 && n < 10545)))
	; 3511..3517 bytes
	delay_n (n-2)
	nop
.elseif(n = 10530)
	; 3510 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10531)
	; 3514 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10533)
	; 3514 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10534)
	; 3512 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10536)
	; 3512 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10537)
	; 3516 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10539)
	; 3516 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10540)
	; 3514 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10542)
	; 3514 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10543)
	; 3518 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10545)
	; 3518 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10546)
	; 3516 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.endif
.else
.if((n >= 10547) && (n < 10586))
.if((n >= 10547) && (n < 10566))
; 10547 to 10565
.if((n .mod 3 = 2) && ((n > 10546 && n < 10566)))
	; 3517..3525 bytes
	delay_n (n-2)
	nop
.elseif(n = 10548)
	; 3516 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10549)
	; 3520 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10551)
	; 3520 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10552)
	; 3518 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10554)
	; 3518 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10555)
	; 3522 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10557)
	; 3522 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10558)
	; 3520 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10560)
	; 3520 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10561)
	; 3524 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10563)
	; 3524 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10564)
	; 3522 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 10566 to 10585
.if(n = 10566)
	; 3522 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10567)
	; 3526 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 10567 && n < 10584)))
	; 3523..3531 bytes
	delay_n (n-2)
	nop
.elseif(n = 10569)
	; 3526 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10570)
	; 3524 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10572)
	; 3524 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10573)
	; 3528 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10575)
	; 3528 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10576)
	; 3526 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10578)
	; 3526 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10579)
	; 3530 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10581)
	; 3530 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3532 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 10586) && (n < 10606))
; 10586 to 10605
.if((n .mod 3 = 2) && ((n > 10585 && n < 10605)))
	; 3529..3537 bytes
	delay_n (n-2)
	nop
.elseif(n = 10587)
	; 3532 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10588)
	; 3530 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10590)
	; 3530 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10591)
	; 3534 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10593)
	; 3534 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10594)
	; 3532 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10596)
	; 3532 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10597)
	; 3536 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10599)
	; 3536 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10600)
	; 3534 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10602)
	; 3534 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10603)
	; 3538 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10605)
	; 3538 bytes
	delay_n 129
	delay_n (n-129)
.endif
.else
; 10606 to 10625
.if(n = 10606)
	; 3536 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 10606 && n < 10626)))
	; 3537..3545 bytes
	delay_n (n-2)
	nop
.elseif(n = 10608)
	; 3536 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10609)
	; 3540 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10611)
	; 3540 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10612)
	; 3538 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10614)
	; 3538 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10615)
	; 3542 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10617)
	; 3542 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10618)
	; 3540 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10620)
	; 3540 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10621)
	; 3544 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10623)
	; 3544 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10624)
	; 3542 bytes
	delay_n 94
	delay_n (n-94)
.endif
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
.if((n >= 10626) && (n < 10645))
; 10626 to 10644
.if(n = 10626)
	; 3542 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10627)
	; 3546 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 10627 && n < 10644)))
	; 3543..3551 bytes
	delay_n (n-2)
	nop
.elseif(n = 10629)
	; 3546 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10630)
	; 3544 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10632)
	; 3544 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10633)
	; 3548 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10635)
	; 3548 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3550 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10641)
	; 3550 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10642)
	; 3548 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10644)
	; 3548 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 10645 to 10664
.if(n = 10645)
	; 3552 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 10645 && n < 10665)))
	; 3549..3557 bytes
	delay_n (n-2)
	nop
.elseif(n = 10647)
	; 3552 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10648)
	; 3550 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10650)
	; 3550 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10651)
	; 3554 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10653)
	; 3554 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10654)
	; 3552 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10656)
	; 3552 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10657)
	; 3556 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10659)
	; 3556 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10660)
	; 3554 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10662)
	; 3554 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10663)
	; 3558 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 10665) && (n < 10684))
; 10665 to 10683
.if(n = 10665)
	; 3558 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10666)
	; 3556 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 10666 && n < 10683)))
	; 3557..3563 bytes
	delay_n (n-2)
	nop
.elseif(n = 10668)
	; 3556 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10669)
	; 3560 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10671)
	; 3560 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10672)
	; 3558 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10674)
	; 3558 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10675)
	; 3562 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10677)
	; 3562 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10678)
	; 3560 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10680)
	; 3560 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10681)
	; 3564 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10683)
	; 3564 bytes
	delay_n 45
	delay_n (n-45)
.endif
.else
; 10684 to 10703
.if(n = 10684)
	; 3562 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 10684 && n < 10704)))
	; 3563..3571 bytes
	delay_n (n-2)
	nop
.elseif(n = 10686)
	; 3562 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10687)
	; 3566 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10689)
	; 3566 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3568 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10695)
	; 3568 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10696)
	; 3566 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10698)
	; 3566 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10699)
	; 3570 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10701)
	; 3570 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10702)
	; 3568 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.else
.if((n >= 10704) && (n < 10743))
.if((n >= 10704) && (n < 10723))
; 10704 to 10722
.if(n = 10704)
	; 3568 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10705)
	; 3572 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 10705 && n < 10722)))
	; 3569..3577 bytes
	delay_n (n-2)
	nop
.elseif(n = 10707)
	; 3572 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10708)
	; 3570 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10710)
	; 3570 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10711)
	; 3574 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10713)
	; 3574 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10714)
	; 3572 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10716)
	; 3572 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10717)
	; 3576 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10719)
	; 3576 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10720)
	; 3574 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10722)
	; 3574 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 10723 to 10742
.if(n = 10723)
	; 3578 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 10723 && n < 10743)))
	; 3575..3583 bytes
	delay_n (n-2)
	nop
.elseif(n = 10725)
	; 3578 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10726)
	; 3576 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10728)
	; 3576 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10729)
	; 3580 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10731)
	; 3580 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10732)
	; 3578 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10734)
	; 3578 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10735)
	; 3582 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10737)
	; 3582 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10738)
	; 3580 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10740)
	; 3580 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10741)
	; 3584 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 10743) && (n < 10762))
; 10743 to 10761
.if(n = 10743)
	; 3584 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 10744)
	; 3582 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 10744 && n < 10761)))
	; 3583..3589 bytes
	delay_n (n-2)
	nop
.elseif(n = 10746)
	; 3582 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 10747)
	; 3586 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10749)
	; 3586 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10750)
	; 3584 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10752)
	; 3584 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10753)
	; 3588 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10755)
	; 3588 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10756)
	; 3586 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10758)
	; 3586 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10759)
	; 3590 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10761)
	; 3590 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 10762 to 10781
.if(n = 10762)
	; 3588 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 10762 && n < 10782)))
	; 3589..3597 bytes
	delay_n (n-2)
	nop
.elseif(n = 10764)
	; 3588 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10765)
	; 3592 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10767)
	; 3592 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10768)
	; 3590 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10770)
	; 3590 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10771)
	; 3594 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10773)
	; 3594 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10774)
	; 3592 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10776)
	; 3592 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10777)
	; 3596 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10779)
	; 3596 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10780)
	; 3594 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.endif
.else
.if((n >= 10782) && (n < 10860))
.if((n >= 10782) && (n < 10821))
.if((n >= 10782) && (n < 10801))
; 10782 to 10800
.if(n = 10782)
	; 3594 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10783)
	; 3598 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 10783 && n < 10800)))
	; 3595..3603 bytes
	delay_n (n-2)
	nop
.elseif(n = 10785)
	; 3598 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10786)
	; 3596 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10788)
	; 3596 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10789)
	; 3600 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10791)
	; 3600 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10792)
	; 3598 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10794)
	; 3598 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10795)
	; 3602 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10797)
	; 3602 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 10798)
	; 3600 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10800)
	; 3600 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 10801 to 10820
.if(n = 10801)
	; 3604 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 10801 && n < 10821)))
	; 3601..3609 bytes
	delay_n (n-2)
	nop
.elseif(n = 10803)
	; 3604 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10804)
	; 3602 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10806)
	; 3602 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10807)
	; 3606 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10809)
	; 3606 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10810)
	; 3604 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10812)
	; 3604 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10813)
	; 3608 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10815)
	; 3608 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10816)
	; 3606 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10818)
	; 3606 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10819)
	; 3610 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.else
.if((n >= 10821) && (n < 10840))
; 10821 to 10839
.if(n = 10821)
	; 3610 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10822)
	; 3608 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 10822 && n < 10839)))
	; 3609..3615 bytes
	delay_n (n-2)
	nop
.elseif(n = 10824)
	; 3608 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10825)
	; 3612 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10827)
	; 3612 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10828)
	; 3610 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10830)
	; 3610 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10831)
	; 3614 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10833)
	; 3614 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10834)
	; 3612 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10836)
	; 3612 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10837)
	; 3616 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10839)
	; 3616 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 10840 to 10859
.if(n = 10840)
	; 3614 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 10840 && n < 10860)))
	; 3615..3623 bytes
	delay_n (n-2)
	nop
.elseif(n = 10842)
	; 3614 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10843)
	; 3618 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10845)
	; 3618 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10846)
	; 3616 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10848)
	; 3616 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10849)
	; 3620 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10851)
	; 3620 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3622 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10857)
	; 3622 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10858)
	; 3620 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.else
.if((n >= 10860) && (n < 10899))
.if((n >= 10860) && (n < 10879))
; 10860 to 10878
.if(n = 10860)
	; 3620 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10861)
	; 3624 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 10861 && n < 10878)))
	; 3621..3629 bytes
	delay_n (n-2)
	nop
.elseif(n = 10863)
	; 3624 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10864)
	; 3622 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10866)
	; 3622 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10867)
	; 3626 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10869)
	; 3626 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10870)
	; 3624 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10872)
	; 3624 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10873)
	; 3628 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10875)
	; 3628 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10876)
	; 3626 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10878)
	; 3626 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 10879 to 10898
.if(n = 10879)
	; 3630 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 10879 && n < 10899)))
	; 3627..3635 bytes
	delay_n (n-2)
	nop
.elseif(n = 10881)
	; 3630 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10882)
	; 3628 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10884)
	; 3628 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10885)
	; 3632 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10887)
	; 3632 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10888)
	; 3630 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10890)
	; 3630 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10891)
	; 3634 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10893)
	; 3634 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10894)
	; 3632 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10896)
	; 3632 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10897)
	; 3636 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 10899) && (n < 10918))
; 10899 to 10917
.if(n = 10899)
	; 3636 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10900)
	; 3634 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 10900 && n < 10917)))
	; 3635..3641 bytes
	delay_n (n-2)
	nop
.elseif(n = 10902)
	; 3634 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 10903)
	; 3638 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 10905)
	; 3638 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3640 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10911)
	; 3640 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10912)
	; 3638 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10914)
	; 3638 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10915)
	; 3642 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10917)
	; 3642 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 10918 to 10937
.if(n = 10918)
	; 3640 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 10918 && n < 10938)))
	; 3641..3649 bytes
	delay_n (n-2)
	nop
.elseif(n = 10920)
	; 3640 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10921)
	; 3644 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 10923)
	; 3644 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10924)
	; 3642 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 10926)
	; 3642 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10927)
	; 3646 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10929)
	; 3646 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10930)
	; 3644 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10932)
	; 3644 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10933)
	; 3648 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10935)
	; 3648 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10936)
	; 3646 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
.if((n >= 10938) && (n < 10957))
; 10938 to 10956
.if(n = 10938)
	; 3646 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10939)
	; 3650 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 10939 && n < 10956)))
	; 3647..3655 bytes
	delay_n (n-2)
	nop
.elseif(n = 10941)
	; 3650 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10942)
	; 3648 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10944)
	; 3648 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10945)
	; 3652 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10947)
	; 3652 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 10948)
	; 3650 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 10950)
	; 3650 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10951)
	; 3654 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10953)
	; 3654 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 10954)
	; 3652 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10956)
	; 3652 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 10957 to 10976
.if(n = 10957)
	; 3656 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 10957 && n < 10977)))
	; 3653..3661 bytes
	delay_n (n-2)
	nop
.elseif(n = 10959)
	; 3656 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3658 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 10965)
	; 3658 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10966)
	; 3656 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 10968)
	; 3656 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 10969)
	; 3660 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 10971)
	; 3660 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 10972)
	; 3658 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10974)
	; 3658 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10975)
	; 3662 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 10977) && (n < 10996))
; 10977 to 10995
.if(n = 10977)
	; 3662 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 10978)
	; 3660 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 10978 && n < 10995)))
	; 3661..3667 bytes
	delay_n (n-2)
	nop
.elseif(n = 10980)
	; 3660 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 10981)
	; 3664 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10983)
	; 3664 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 10984)
	; 3662 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 10986)
	; 3662 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 10987)
	; 3666 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 10989)
	; 3666 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10990)
	; 3664 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 10992)
	; 3664 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 10993)
	; 3668 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 10995)
	; 3668 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 3666 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 10996 && n < 11016)))
	; 3667..3675 bytes
	delay_n (n-2)
	nop
.elseif(n = 10998)
	; 3666 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10999)
	; 3670 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11001)
	; 3670 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11002)
	; 3668 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11004)
	; 3668 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11005)
	; 3672 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11007)
	; 3672 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11008)
	; 3670 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11010)
	; 3670 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11011)
	; 3674 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11013)
	; 3674 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 11014)
	; 3672 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
.if((n >= 11016) && (n < 11035))
; 11016 to 11034
.if(n = 11016)
	; 3672 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11017)
	; 3676 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 11017 && n < 11034)))
	; 3673..3681 bytes
	delay_n (n-2)
	nop
.elseif(n = 11019)
	; 3676 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11020)
	; 3674 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11022)
	; 3674 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11023)
	; 3678 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11025)
	; 3678 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11026)
	; 3676 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11028)
	; 3676 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11029)
	; 3680 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11031)
	; 3680 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11032)
	; 3678 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11034)
	; 3678 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 11035 to 11054
.if(n = 11035)
	; 3682 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 11035 && n < 11055)))
	; 3679..3687 bytes
	delay_n (n-2)
	nop
.elseif(n = 11037)
	; 3682 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11038)
	; 3680 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11040)
	; 3680 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11041)
	; 3684 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11043)
	; 3684 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11044)
	; 3682 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11046)
	; 3682 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11047)
	; 3686 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11049)
	; 3686 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11050)
	; 3684 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11052)
	; 3684 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11053)
	; 3688 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 11055) && (n < 11074))
; 11055 to 11073
.if(n = 11055)
	; 3688 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11056)
	; 3686 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 11056 && n < 11073)))
	; 3687..3693 bytes
	delay_n (n-2)
	nop
.elseif(n = 11058)
	; 3686 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11059)
	; 3690 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11061)
	; 3690 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11062)
	; 3688 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11064)
	; 3688 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11065)
	; 3692 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11067)
	; 3692 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3694 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11073)
	; 3694 bytes
	delay_n 57
	delay_n (n-57)
.endif
.else
; 11074 to 11093
.if(n = 11074)
	; 3692 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 11074 && n < 11094)))
	; 3693..3701 bytes
	delay_n (n-2)
	nop
.elseif(n = 11076)
	; 3692 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11077)
	; 3696 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11079)
	; 3696 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11080)
	; 3694 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11082)
	; 3694 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11083)
	; 3698 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11085)
	; 3698 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11086)
	; 3696 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11088)
	; 3696 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11089)
	; 3700 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11091)
	; 3700 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11092)
	; 3698 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.endif
.else
.if((n >= 11094) && (n < 11172))
.if((n >= 11094) && (n < 11133))
.if((n >= 11094) && (n < 11113))
; 11094 to 11112
.if(n = 11094)
	; 3698 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11095)
	; 3702 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 11095 && n < 11112)))
	; 3699..3707 bytes
	delay_n (n-2)
	nop
.elseif(n = 11097)
	; 3702 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11098)
	; 3700 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11100)
	; 3700 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11101)
	; 3704 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11103)
	; 3704 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11104)
	; 3702 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11106)
	; 3702 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11107)
	; 3706 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11109)
	; 3706 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11110)
	; 3704 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11112)
	; 3704 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 11113 to 11132
.if(n = 11113)
	; 3708 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 11113 && n < 11133)))
	; 3705..3713 bytes
	delay_n (n-2)
	nop
.elseif(n = 11115)
	; 3708 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11116)
	; 3706 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11118)
	; 3706 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11119)
	; 3710 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11121)
	; 3710 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3712 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11127)
	; 3712 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11128)
	; 3710 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11130)
	; 3710 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11131)
	; 3714 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.else
.if((n >= 11133) && (n < 11152))
; 11133 to 11151
.if(n = 11133)
	; 3714 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11134)
	; 3712 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 11134 && n < 11151)))
	; 3713..3719 bytes
	delay_n (n-2)
	nop
.elseif(n = 11136)
	; 3712 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11137)
	; 3716 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11139)
	; 3716 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11140)
	; 3714 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11142)
	; 3714 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11143)
	; 3718 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11145)
	; 3718 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11146)
	; 3716 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11148)
	; 3716 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11149)
	; 3720 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11151)
	; 3720 bytes
	delay_n 81
	delay_n (n-81)
.endif
.else
; 11152 to 11171
.if(n = 11152)
	; 3718 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 11152 && n < 11172)))
	; 3719..3727 bytes
	delay_n (n-2)
	nop
.elseif(n = 11154)
	; 3718 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11155)
	; 3722 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11157)
	; 3722 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11158)
	; 3720 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11160)
	; 3720 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11161)
	; 3724 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11163)
	; 3724 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11164)
	; 3722 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11166)
	; 3722 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11167)
	; 3726 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11169)
	; 3726 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11170)
	; 3724 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.else
.if((n >= 11172) && (n < 11211))
.if((n >= 11172) && (n < 11191))
; 11172 to 11190
.if(n = 11172)
	; 3724 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11173)
	; 3728 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 11173 && n < 11190)))
	; 3725..3733 bytes
	delay_n (n-2)
	nop
.elseif(n = 11175)
	; 3728 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3730 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11181)
	; 3730 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11182)
	; 3728 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11184)
	; 3728 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11185)
	; 3732 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11187)
	; 3732 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11188)
	; 3730 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11190)
	; 3730 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 11191 to 11210
.if(n = 11191)
	; 3734 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 11191 && n < 11211)))
	; 3731..3739 bytes
	delay_n (n-2)
	nop
.elseif(n = 11193)
	; 3734 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11194)
	; 3732 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11196)
	; 3732 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11197)
	; 3736 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11199)
	; 3736 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11200)
	; 3734 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11202)
	; 3734 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11203)
	; 3738 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11205)
	; 3738 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11206)
	; 3736 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11208)
	; 3736 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11209)
	; 3740 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.else
.if((n >= 11211) && (n < 11231))
; 11211 to 11230
.if(n = 11211)
	; 3740 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11212)
	; 3738 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 11212 && n < 11229)))
	; 3739..3745 bytes
	delay_n (n-2)
	nop
.elseif(n = 11214)
	; 3738 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11215)
	; 3742 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11217)
	; 3742 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11218)
	; 3740 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11220)
	; 3740 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11221)
	; 3744 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11223)
	; 3744 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11224)
	; 3742 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11226)
	; 3742 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11227)
	; 3746 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11229)
	; 3746 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 11230)
	; 3744 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 11231 to 11250
.if((n .mod 3 = 2) && ((n > 11230 && n < 11250)))
	; 3745..3753 bytes
	delay_n (n-2)
	nop
.elseif(n = 11232)
	; 3744 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11233)
	; 3748 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11235)
	; 3748 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11236)
	; 3746 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11238)
	; 3746 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11239)
	; 3750 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11241)
	; 3750 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11242)
	; 3748 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11244)
	; 3748 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11245)
	; 3752 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11247)
	; 3752 bytes
	delay_n 69
	delay_n (n-69)
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
	; 3754 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 11251 && n < 11268)))
	; 3751..3759 bytes
	delay_n (n-2)
	nop
.elseif(n = 11253)
	; 3754 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11254)
	; 3752 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11256)
	; 3752 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11257)
	; 3756 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11259)
	; 3756 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11260)
	; 3754 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11262)
	; 3754 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11263)
	; 3758 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11265)
	; 3758 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11266)
	; 3756 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11268)
	; 3756 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11269)
	; 3760 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 11270 to 11289
.if((n .mod 3 = 2) && ((n > 11269 && n < 11289)))
	; 3757..3765 bytes
	delay_n (n-2)
	nop
.elseif(n = 11271)
	; 3760 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11272)
	; 3758 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11274)
	; 3758 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11275)
	; 3762 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11277)
	; 3762 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11278)
	; 3760 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11280)
	; 3760 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11281)
	; 3764 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11283)
	; 3764 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3766 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11289)
	; 3766 bytes
	delay_n 57
	delay_n (n-57)
.endif
.endif
.else
.if((n >= 11290) && (n < 11309))
; 11290 to 11308
.if(n = 11290)
	; 3764 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 11290 && n < 11307)))
	; 3765..3771 bytes
	delay_n (n-2)
	nop
.elseif(n = 11292)
	; 3764 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11293)
	; 3768 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11295)
	; 3768 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11296)
	; 3766 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11298)
	; 3766 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11299)
	; 3770 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11301)
	; 3770 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11302)
	; 3768 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11304)
	; 3768 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11305)
	; 3772 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11307)
	; 3772 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11308)
	; 3770 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 11309 to 11328
.if((n .mod 3 = 2) && ((n > 11308 && n < 11328)))
	; 3771..3779 bytes
	delay_n (n-2)
	nop
.elseif(n = 11310)
	; 3770 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11311)
	; 3774 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11313)
	; 3774 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11314)
	; 3772 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11316)
	; 3772 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11317)
	; 3776 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11319)
	; 3776 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11320)
	; 3774 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11322)
	; 3774 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11323)
	; 3778 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11325)
	; 3778 bytes
	delay_n 93
	delay_n (n-93)
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
.endif
.else
.if((n >= 11329) && (n < 11368))
.if((n >= 11329) && (n < 11348))
; 11329 to 11347
.if(n = 11329)
	; 3780 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 11329 && n < 11346)))
	; 3777..3785 bytes
	delay_n (n-2)
	nop
.elseif(n = 11331)
	; 3780 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11332)
	; 3778 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11334)
	; 3778 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11335)
	; 3782 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11337)
	; 3782 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3784 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11343)
	; 3784 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11344)
	; 3782 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11346)
	; 3782 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11347)
	; 3786 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 11348 to 11367
.if((n .mod 3 = 2) && ((n > 11347 && n < 11367)))
	; 3783..3791 bytes
	delay_n (n-2)
	nop
.elseif(n = 11349)
	; 3786 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11350)
	; 3784 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11352)
	; 3784 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11353)
	; 3788 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11355)
	; 3788 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11356)
	; 3786 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11358)
	; 3786 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11359)
	; 3790 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11361)
	; 3790 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11362)
	; 3788 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11364)
	; 3788 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11365)
	; 3792 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11367)
	; 3792 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.else
.if((n >= 11368) && (n < 11387))
; 11368 to 11386
.if(n = 11368)
	; 3790 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 11368 && n < 11385)))
	; 3791..3797 bytes
	delay_n (n-2)
	nop
.elseif(n = 11370)
	; 3790 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11371)
	; 3794 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11373)
	; 3794 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11374)
	; 3792 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11376)
	; 3792 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11377)
	; 3796 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11379)
	; 3796 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11380)
	; 3794 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11382)
	; 3794 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11383)
	; 3798 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11385)
	; 3798 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11386)
	; 3796 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 11387 to 11406
.if((n .mod 3 = 2) && ((n > 11386 && n < 11406)))
	; 3797..3805 bytes
	delay_n (n-2)
	nop
.elseif(n = 11388)
	; 3796 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11389)
	; 3800 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11391)
	; 3800 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3802 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11397)
	; 3802 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11398)
	; 3800 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11400)
	; 3800 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11401)
	; 3804 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11403)
	; 3804 bytes
	delay_n 117
	delay_n (n-117)
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
.endif
.else
.if((n >= 11407) && (n < 11485))
.if((n >= 11407) && (n < 11446))
.if((n >= 11407) && (n < 11426))
; 11407 to 11425
.if(n = 11407)
	; 3806 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 11407 && n < 11424)))
	; 3803..3811 bytes
	delay_n (n-2)
	nop
.elseif(n = 11409)
	; 3806 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11410)
	; 3804 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11412)
	; 3804 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11413)
	; 3808 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11415)
	; 3808 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11416)
	; 3806 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11418)
	; 3806 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11419)
	; 3810 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11421)
	; 3810 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11422)
	; 3808 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11424)
	; 3808 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11425)
	; 3812 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 11426 to 11445
.if((n .mod 3 = 2) && ((n > 11425 && n < 11445)))
	; 3809..3817 bytes
	delay_n (n-2)
	nop
.elseif(n = 11427)
	; 3812 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11428)
	; 3810 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11430)
	; 3810 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11431)
	; 3814 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11433)
	; 3814 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11434)
	; 3812 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11436)
	; 3812 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11437)
	; 3816 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11439)
	; 3816 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11440)
	; 3814 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11442)
	; 3814 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11443)
	; 3818 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11445)
	; 3818 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.else
.if((n >= 11446) && (n < 11465))
; 11446 to 11464
.if(n = 11446)
	; 3816 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 11446 && n < 11463)))
	; 3817..3823 bytes
	delay_n (n-2)
	nop
.elseif(n = 11448)
	; 3816 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11449)
	; 3820 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11451)
	; 3820 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11452)
	; 3818 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11454)
	; 3818 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11455)
	; 3822 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11457)
	; 3822 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11458)
	; 3820 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11460)
	; 3820 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11461)
	; 3824 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11463)
	; 3824 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11464)
	; 3822 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 11465 to 11484
.if((n .mod 3 = 2) && ((n > 11464 && n < 11484)))
	; 3823..3831 bytes
	delay_n (n-2)
	nop
.elseif(n = 11466)
	; 3822 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11467)
	; 3826 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11469)
	; 3826 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11470)
	; 3824 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11472)
	; 3824 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11473)
	; 3828 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11475)
	; 3828 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11476)
	; 3826 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11478)
	; 3826 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11479)
	; 3830 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11481)
	; 3830 bytes
	delay_n 141
	delay_n (n-141)
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
.endif
.else
.if((n >= 11485) && (n < 11524))
.if((n >= 11485) && (n < 11504))
; 11485 to 11503
.if(n = 11485)
	; 3832 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 11485 && n < 11502)))
	; 3829..3837 bytes
	delay_n (n-2)
	nop
.elseif(n = 11487)
	; 3832 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11488)
	; 3830 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11490)
	; 3830 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11491)
	; 3834 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11493)
	; 3834 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11494)
	; 3832 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11496)
	; 3832 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11497)
	; 3836 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11499)
	; 3836 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3838 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 11504 to 11523
.if((n .mod 3 = 2) && ((n > 11503 && n < 11523)))
	; 3835..3843 bytes
	delay_n (n-2)
	nop
.elseif(n = 11505)
	; 3838 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11506)
	; 3836 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11508)
	; 3836 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11509)
	; 3840 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11511)
	; 3840 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11512)
	; 3838 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11514)
	; 3838 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11515)
	; 3842 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11517)
	; 3842 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11518)
	; 3840 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11520)
	; 3840 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11521)
	; 3844 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11523)
	; 3844 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.else
.if((n >= 11524) && (n < 11543))
; 11524 to 11542
.if(n = 11524)
	; 3842 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 11524 && n < 11541)))
	; 3843..3849 bytes
	delay_n (n-2)
	nop
.elseif(n = 11526)
	; 3842 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11527)
	; 3846 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11529)
	; 3846 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11530)
	; 3844 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11532)
	; 3844 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11533)
	; 3848 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11535)
	; 3848 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11536)
	; 3846 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11538)
	; 3846 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11539)
	; 3850 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11541)
	; 3850 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11542)
	; 3848 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 11543 to 11562
.if((n .mod 3 = 2) && ((n > 11542 && n < 11562)))
	; 3849..3857 bytes
	delay_n (n-2)
	nop
.elseif(n = 11544)
	; 3848 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11545)
	; 3852 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11547)
	; 3852 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11548)
	; 3850 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11550)
	; 3850 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11551)
	; 3854 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11553)
	; 3854 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3856 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11559)
	; 3856 bytes
	delay_n 57
	delay_n (n-57)
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
.endif
.else
.if((n >= 11563) && (n < 11719))
.if((n >= 11563) && (n < 11641))
.if((n >= 11563) && (n < 11602))
.if((n >= 11563) && (n < 11582))
; 11563 to 11581
.if(n = 11563)
	; 3858 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 11563 && n < 11580)))
	; 3855..3863 bytes
	delay_n (n-2)
	nop
.elseif(n = 11565)
	; 3858 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11566)
	; 3856 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11568)
	; 3856 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11569)
	; 3860 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11571)
	; 3860 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11572)
	; 3858 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11574)
	; 3858 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11575)
	; 3862 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11577)
	; 3862 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11578)
	; 3860 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11580)
	; 3860 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11581)
	; 3864 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 11582 to 11601
.if((n .mod 3 = 2) && ((n > 11581 && n < 11601)))
	; 3861..3869 bytes
	delay_n (n-2)
	nop
.elseif(n = 11583)
	; 3864 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11584)
	; 3862 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11586)
	; 3862 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11587)
	; 3866 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11589)
	; 3866 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11590)
	; 3864 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11592)
	; 3864 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11593)
	; 3868 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11595)
	; 3868 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11596)
	; 3866 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11598)
	; 3866 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11599)
	; 3870 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11601)
	; 3870 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.else
.if((n >= 11602) && (n < 11621))
; 11602 to 11620
.if(n = 11602)
	; 3868 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 11602 && n < 11619)))
	; 3869..3875 bytes
	delay_n (n-2)
	nop
.elseif(n = 11604)
	; 3868 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11605)
	; 3872 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11607)
	; 3872 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3874 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11613)
	; 3874 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11614)
	; 3872 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11616)
	; 3872 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11617)
	; 3876 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11619)
	; 3876 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11620)
	; 3874 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 11621 to 11640
.if((n .mod 3 = 2) && ((n > 11620 && n < 11640)))
	; 3875..3883 bytes
	delay_n (n-2)
	nop
.elseif(n = 11622)
	; 3874 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11623)
	; 3878 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11625)
	; 3878 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11626)
	; 3876 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11628)
	; 3876 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11629)
	; 3880 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11631)
	; 3880 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11632)
	; 3878 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11634)
	; 3878 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11635)
	; 3882 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11637)
	; 3882 bytes
	delay_n 81
	delay_n (n-81)
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
.endif
.else
.if((n >= 11641) && (n < 11680))
.if((n >= 11641) && (n < 11660))
; 11641 to 11659
.if(n = 11641)
	; 3884 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 11641 && n < 11658)))
	; 3881..3889 bytes
	delay_n (n-2)
	nop
.elseif(n = 11643)
	; 3884 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11644)
	; 3882 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11646)
	; 3882 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11647)
	; 3886 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11649)
	; 3886 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11650)
	; 3884 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11652)
	; 3884 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11653)
	; 3888 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11655)
	; 3888 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11656)
	; 3886 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11658)
	; 3886 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11659)
	; 3890 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 11660 to 11679
.if((n .mod 3 = 2) && ((n > 11659 && n < 11679)))
	; 3887..3895 bytes
	delay_n (n-2)
	nop
.elseif(n = 11661)
	; 3890 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3892 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11667)
	; 3892 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11668)
	; 3890 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11670)
	; 3890 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11671)
	; 3894 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11673)
	; 3894 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11674)
	; 3892 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11676)
	; 3892 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11677)
	; 3896 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11679)
	; 3896 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.else
.if((n >= 11680) && (n < 11699))
; 11680 to 11698
.if(n = 11680)
	; 3894 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 11680 && n < 11697)))
	; 3895..3901 bytes
	delay_n (n-2)
	nop
.elseif(n = 11682)
	; 3894 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11683)
	; 3898 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11685)
	; 3898 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11686)
	; 3896 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11688)
	; 3896 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11689)
	; 3900 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11691)
	; 3900 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11692)
	; 3898 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11694)
	; 3898 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11695)
	; 3902 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11697)
	; 3902 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11698)
	; 3900 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 11699 to 11718
.if((n .mod 3 = 2) && ((n > 11698 && n < 11718)))
	; 3901..3909 bytes
	delay_n (n-2)
	nop
.elseif(n = 11700)
	; 3900 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11701)
	; 3904 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11703)
	; 3904 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11704)
	; 3902 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11706)
	; 3902 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11707)
	; 3906 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11709)
	; 3906 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11710)
	; 3904 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11712)
	; 3904 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11713)
	; 3908 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11715)
	; 3908 bytes
	delay_n 105
	delay_n (n-105)
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
.endif
.else
.if((n >= 11719) && (n < 11797))
.if((n >= 11719) && (n < 11758))
.if((n >= 11719) && (n < 11738))
; 11719 to 11737
.if(n = 11719)
	; 3910 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 11719 && n < 11736)))
	; 3907..3915 bytes
	delay_n (n-2)
	nop
.elseif(n = 11721)
	; 3910 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11722)
	; 3908 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11724)
	; 3908 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11725)
	; 3912 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11727)
	; 3912 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11728)
	; 3910 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11730)
	; 3910 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11731)
	; 3914 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11733)
	; 3914 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11734)
	; 3912 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11736)
	; 3912 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11737)
	; 3916 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 11738 to 11757
.if((n .mod 3 = 2) && ((n > 11737 && n < 11757)))
	; 3913..3921 bytes
	delay_n (n-2)
	nop
.elseif(n = 11739)
	; 3916 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11740)
	; 3914 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11742)
	; 3914 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11743)
	; 3918 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11745)
	; 3918 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11746)
	; 3916 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11748)
	; 3916 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11749)
	; 3920 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11751)
	; 3920 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11752)
	; 3918 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11754)
	; 3918 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11755)
	; 3922 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11757)
	; 3922 bytes
	delay_n 93
	delay_n (n-93)
.endif
.endif
.else
.if((n >= 11758) && (n < 11777))
; 11758 to 11776
.if(n = 11758)
	; 3920 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 11758 && n < 11775)))
	; 3921..3927 bytes
	delay_n (n-2)
	nop
.elseif(n = 11760)
	; 3920 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11761)
	; 3924 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11763)
	; 3924 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11764)
	; 3922 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11766)
	; 3922 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11767)
	; 3926 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11769)
	; 3926 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3928 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11775)
	; 3928 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11776)
	; 3926 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 11777 to 11796
.if((n .mod 3 = 2) && ((n > 11776 && n < 11796)))
	; 3927..3935 bytes
	delay_n (n-2)
	nop
.elseif(n = 11778)
	; 3926 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11779)
	; 3930 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11781)
	; 3930 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11782)
	; 3928 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11784)
	; 3928 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11785)
	; 3932 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11787)
	; 3932 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11788)
	; 3930 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11790)
	; 3930 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11791)
	; 3934 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11793)
	; 3934 bytes
	delay_n 129
	delay_n (n-129)
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
.endif
.else
.if((n >= 11797) && (n < 11836))
.if((n >= 11797) && (n < 11816))
; 11797 to 11815
.if(n = 11797)
	; 3936 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 11797 && n < 11814)))
	; 3933..3941 bytes
	delay_n (n-2)
	nop
.elseif(n = 11799)
	; 3936 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11800)
	; 3934 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11802)
	; 3934 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11803)
	; 3938 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11805)
	; 3938 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11806)
	; 3936 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11808)
	; 3936 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11809)
	; 3940 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11811)
	; 3940 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11812)
	; 3938 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11814)
	; 3938 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11815)
	; 3942 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 11816 to 11835
.if((n .mod 3 = 2) && ((n > 11815 && n < 11835)))
	; 3939..3947 bytes
	delay_n (n-2)
	nop
.elseif(n = 11817)
	; 3942 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11818)
	; 3940 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11820)
	; 3940 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11821)
	; 3944 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11823)
	; 3944 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3946 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11829)
	; 3946 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11830)
	; 3944 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11832)
	; 3944 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11833)
	; 3948 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11835)
	; 3948 bytes
	delay_n 117
	delay_n (n-117)
.endif
.endif
.else
.if((n >= 11836) && (n < 11856))
; 11836 to 11855
.if(n = 11836)
	; 3946 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 11836 && n < 11856)))
	; 3947..3955 bytes
	delay_n (n-2)
	nop
.elseif(n = 11838)
	; 3946 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11839)
	; 3950 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11841)
	; 3950 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11842)
	; 3948 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11844)
	; 3948 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11845)
	; 3952 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11847)
	; 3952 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11848)
	; 3950 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11850)
	; 3950 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11851)
	; 3954 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11853)
	; 3954 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11854)
	; 3952 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 11856 to 11875
.if(n = 11856)
	; 3952 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11857)
	; 3956 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 11857 && n < 11874)))
	; 3953..3961 bytes
	delay_n (n-2)
	nop
.elseif(n = 11859)
	; 3956 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11860)
	; 3954 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11862)
	; 3954 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11863)
	; 3958 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11865)
	; 3958 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11866)
	; 3956 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11868)
	; 3956 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11869)
	; 3960 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11871)
	; 3960 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11872)
	; 3958 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11874)
	; 3958 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11875)
	; 3962 bytes
	delay_n 103
	delay_n (n-103)
.endif
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
.if((n >= 11876) && (n < 11895))
; 11876 to 11894
.if((n .mod 3 = 2) && ((n > 11875 && n < 11895)))
	; 3959..3967 bytes
	delay_n (n-2)
	nop
.elseif(n = 11877)
	; 3962 bytes
	delay_n 105
	delay_n (n-105)
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
	; 3964 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11883)
	; 3964 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11884)
	; 3962 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11886)
	; 3962 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11887)
	; 3966 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11889)
	; 3966 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11890)
	; 3964 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11892)
	; 3964 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11893)
	; 3968 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 11895 to 11914
.if(n = 11895)
	; 3968 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11896)
	; 3966 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 11896 && n < 11913)))
	; 3967..3973 bytes
	delay_n (n-2)
	nop
.elseif(n = 11898)
	; 3966 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11899)
	; 3970 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11901)
	; 3970 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11902)
	; 3968 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11904)
	; 3968 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11905)
	; 3972 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11907)
	; 3972 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11908)
	; 3970 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11910)
	; 3970 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11911)
	; 3974 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11913)
	; 3974 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11914)
	; 3972 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 11915) && (n < 11934))
; 11915 to 11933
.if((n .mod 3 = 2) && ((n > 11914 && n < 11934)))
	; 3973..3981 bytes
	delay_n (n-2)
	nop
.elseif(n = 11916)
	; 3972 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11917)
	; 3976 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11919)
	; 3976 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11920)
	; 3974 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 11922)
	; 3974 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11923)
	; 3978 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11925)
	; 3978 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11926)
	; 3976 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11928)
	; 3976 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11929)
	; 3980 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11931)
	; 3980 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 11932)
	; 3978 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 11934 to 11953
.if(n = 11934)
	; 3978 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 11935)
	; 3982 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 11935 && n < 11952)))
	; 3979..3987 bytes
	delay_n (n-2)
	nop
.elseif(n = 11937)
	; 3982 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11938)
	; 3980 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 11940)
	; 3980 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11941)
	; 3984 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11943)
	; 3984 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11944)
	; 3982 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11946)
	; 3982 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11947)
	; 3986 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 11949)
	; 3986 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11950)
	; 3984 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 11952)
	; 3984 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 11953)
	; 3988 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.endif
.else
.if((n >= 11954) && (n < 11993))
.if((n >= 11954) && (n < 11973))
; 11954 to 11972
.if((n .mod 3 = 2) && ((n > 11953 && n < 11973)))
	; 3985..3993 bytes
	delay_n (n-2)
	nop
.elseif(n = 11955)
	; 3988 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 11956)
	; 3986 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11958)
	; 3986 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 11959)
	; 3990 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 11961)
	; 3990 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11962)
	; 3988 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 11964)
	; 3988 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 11965)
	; 3992 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 11967)
	; 3992 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11968)
	; 3990 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11970)
	; 3990 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11971)
	; 3994 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 11973 to 11992
.if(n = 11973)
	; 3994 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 11974)
	; 3992 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 11974 && n < 11991)))
	; 3993..3999 bytes
	delay_n (n-2)
	nop
.elseif(n = 11976)
	; 3992 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11977)
	; 3996 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11979)
	; 3996 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 11980)
	; 3994 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11982)
	; 3994 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 11983)
	; 3998 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 11985)
	; 3998 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4000 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11991)
	; 4000 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 11992)
	; 3998 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 11993) && (n < 12012))
; 11993 to 12011
.if((n .mod 3 = 2) && ((n > 11992 && n < 12012)))
	; 3999..4007 bytes
	delay_n (n-2)
	nop
.elseif(n = 11994)
	; 3998 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 11995)
	; 4002 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 11997)
	; 4002 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 11998)
	; 4000 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12000)
	; 4000 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12001)
	; 4004 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12003)
	; 4004 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12004)
	; 4002 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12006)
	; 4002 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12007)
	; 4006 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12009)
	; 4006 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12010)
	; 4004 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 12012 to 12031
.if(n = 12012)
	; 4004 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12013)
	; 4008 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 12013 && n < 12030)))
	; 4005..4013 bytes
	delay_n (n-2)
	nop
.elseif(n = 12015)
	; 4008 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12016)
	; 4006 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12018)
	; 4006 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12019)
	; 4010 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12021)
	; 4010 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12022)
	; 4008 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12024)
	; 4008 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12025)
	; 4012 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12027)
	; 4012 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12028)
	; 4010 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12030)
	; 4010 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12031)
	; 4014 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
.if((n >= 12032) && (n < 12051))
; 12032 to 12050
.if((n .mod 3 = 2) && ((n > 12031 && n < 12051)))
	; 4011..4019 bytes
	delay_n (n-2)
	nop
.elseif(n = 12033)
	; 4014 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12034)
	; 4012 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12036)
	; 4012 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12037)
	; 4016 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12039)
	; 4016 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4018 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12045)
	; 4018 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12046)
	; 4016 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12048)
	; 4016 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12049)
	; 4020 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 12051 to 12070
.if(n = 12051)
	; 4020 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12052)
	; 4018 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 12052 && n < 12069)))
	; 4019..4025 bytes
	delay_n (n-2)
	nop
.elseif(n = 12054)
	; 4018 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12055)
	; 4022 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12057)
	; 4022 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12058)
	; 4020 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12060)
	; 4020 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12061)
	; 4024 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12063)
	; 4024 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12064)
	; 4022 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12066)
	; 4022 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12067)
	; 4026 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12069)
	; 4026 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12070)
	; 4024 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 12071) && (n < 12090))
; 12071 to 12089
.if((n .mod 3 = 2) && ((n > 12070 && n < 12090)))
	; 4025..4033 bytes
	delay_n (n-2)
	nop
.elseif(n = 12072)
	; 4024 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12073)
	; 4028 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12075)
	; 4028 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12076)
	; 4026 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12078)
	; 4026 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12079)
	; 4030 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12081)
	; 4030 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12082)
	; 4028 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12084)
	; 4028 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12085)
	; 4032 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12087)
	; 4032 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12088)
	; 4030 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 12090 to 12109
.if(n = 12090)
	; 4030 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12091)
	; 4034 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 12091 && n < 12108)))
	; 4031..4039 bytes
	delay_n (n-2)
	nop
.elseif(n = 12093)
	; 4034 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4036 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12099)
	; 4036 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12100)
	; 4034 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12102)
	; 4034 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12103)
	; 4038 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12105)
	; 4038 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12106)
	; 4036 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12108)
	; 4036 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12109)
	; 4040 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 12110) && (n < 12149))
.if((n >= 12110) && (n < 12129))
; 12110 to 12128
.if((n .mod 3 = 2) && ((n > 12109 && n < 12129)))
	; 4037..4045 bytes
	delay_n (n-2)
	nop
.elseif(n = 12111)
	; 4040 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12112)
	; 4038 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12114)
	; 4038 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12115)
	; 4042 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12117)
	; 4042 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12118)
	; 4040 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12120)
	; 4040 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12121)
	; 4044 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12123)
	; 4044 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12124)
	; 4042 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12126)
	; 4042 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12127)
	; 4046 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 12129 to 12148
.if(n = 12129)
	; 4046 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12130)
	; 4044 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 12130 && n < 12147)))
	; 4045..4051 bytes
	delay_n (n-2)
	nop
.elseif(n = 12132)
	; 4044 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12133)
	; 4048 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12135)
	; 4048 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12136)
	; 4046 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12138)
	; 4046 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12139)
	; 4050 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12141)
	; 4050 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12142)
	; 4048 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12144)
	; 4048 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12145)
	; 4052 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12147)
	; 4052 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 12148)
	; 4050 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 12149) && (n < 12168))
; 12149 to 12167
.if((n .mod 3 = 2) && ((n > 12148 && n < 12168)))
	; 4051..4059 bytes
	delay_n (n-2)
	nop
.elseif(n = 12150)
	; 4050 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12151)
	; 4054 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12153)
	; 4054 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12154)
	; 4052 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12156)
	; 4052 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12157)
	; 4056 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12159)
	; 4056 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12160)
	; 4054 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12162)
	; 4054 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12163)
	; 4058 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12165)
	; 4058 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12166)
	; 4056 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 12168 to 12187
.if(n = 12168)
	; 4056 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12169)
	; 4060 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 12169 && n < 12186)))
	; 4057..4065 bytes
	delay_n (n-2)
	nop
.elseif(n = 12171)
	; 4060 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12172)
	; 4058 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12174)
	; 4058 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12175)
	; 4062 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12177)
	; 4062 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12178)
	; 4060 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12180)
	; 4060 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12181)
	; 4064 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12183)
	; 4064 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12184)
	; 4062 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12186)
	; 4062 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12187)
	; 4066 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
.if((n >= 12188) && (n < 12227))
.if((n >= 12188) && (n < 12207))
; 12188 to 12206
.if((n .mod 3 = 2) && ((n > 12187 && n < 12207)))
	; 4063..4071 bytes
	delay_n (n-2)
	nop
.elseif(n = 12189)
	; 4066 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12190)
	; 4064 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12192)
	; 4064 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12193)
	; 4068 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12195)
	; 4068 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12196)
	; 4066 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12198)
	; 4066 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12199)
	; 4070 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12201)
	; 4070 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4072 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 12207 to 12226
.if(n = 12207)
	; 4072 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12208)
	; 4070 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 12208 && n < 12225)))
	; 4071..4077 bytes
	delay_n (n-2)
	nop
.elseif(n = 12210)
	; 4070 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12211)
	; 4074 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12213)
	; 4074 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12214)
	; 4072 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12216)
	; 4072 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12217)
	; 4076 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12219)
	; 4076 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12220)
	; 4074 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12222)
	; 4074 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12223)
	; 4078 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12225)
	; 4078 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12226)
	; 4076 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 12227) && (n < 12246))
; 12227 to 12245
.if((n .mod 3 = 2) && ((n > 12226 && n < 12246)))
	; 4077..4085 bytes
	delay_n (n-2)
	nop
.elseif(n = 12228)
	; 4076 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12229)
	; 4080 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12231)
	; 4080 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12232)
	; 4078 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12234)
	; 4078 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12235)
	; 4082 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12237)
	; 4082 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12238)
	; 4080 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12240)
	; 4080 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12241)
	; 4084 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12243)
	; 4084 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12244)
	; 4082 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 12246 to 12265
.if(n = 12246)
	; 4082 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12247)
	; 4086 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 12247 && n < 12264)))
	; 4083..4091 bytes
	delay_n (n-2)
	nop
.elseif(n = 12249)
	; 4086 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12250)
	; 4084 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12252)
	; 4084 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12253)
	; 4088 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12255)
	; 4088 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4090 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12261)
	; 4090 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12262)
	; 4088 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12264)
	; 4088 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12265)
	; 4092 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.endif
.else
.if((n >= 12266) && (n < 12305))
.if((n >= 12266) && (n < 12285))
; 12266 to 12284
.if((n .mod 3 = 2) && ((n > 12265 && n < 12285)))
	; 4089..4097 bytes
	delay_n (n-2)
	nop
.elseif(n = 12267)
	; 4092 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12268)
	; 4090 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12270)
	; 4090 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12271)
	; 4094 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12273)
	; 4094 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12274)
	; 4092 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12276)
	; 4092 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12277)
	; 4096 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12279)
	; 4096 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12280)
	; 4094 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12282)
	; 4094 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12283)
	; 4098 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 12285 to 12304
.if(n = 12285)
	; 4098 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12286)
	; 4096 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 12286 && n < 12303)))
	; 4097..4103 bytes
	delay_n (n-2)
	nop
.elseif(n = 12288)
	; 4096 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12289)
	; 4100 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12291)
	; 4100 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12292)
	; 4098 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12294)
	; 4098 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12295)
	; 4102 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12297)
	; 4102 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12298)
	; 4100 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12300)
	; 4100 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12301)
	; 4104 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12303)
	; 4104 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12304)
	; 4102 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 12305) && (n < 12324))
; 12305 to 12323
.if((n .mod 3 = 2) && ((n > 12304 && n < 12324)))
	; 4103..4111 bytes
	delay_n (n-2)
	nop
.elseif(n = 12306)
	; 4102 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12307)
	; 4106 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12309)
	; 4106 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4108 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12315)
	; 4108 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12316)
	; 4106 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12318)
	; 4106 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12319)
	; 4110 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12321)
	; 4110 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12322)
	; 4108 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 12324 to 12343
.if(n = 12324)
	; 4108 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12325)
	; 4112 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 12325 && n < 12342)))
	; 4109..4117 bytes
	delay_n (n-2)
	nop
.elseif(n = 12327)
	; 4112 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12328)
	; 4110 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12330)
	; 4110 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12331)
	; 4114 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12333)
	; 4114 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12334)
	; 4112 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12336)
	; 4112 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12337)
	; 4116 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12339)
	; 4116 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12340)
	; 4114 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12342)
	; 4114 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12343)
	; 4118 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.endif
.endif
.else
.if((n >= 12344) && (n < 12422))
.if((n >= 12344) && (n < 12383))
.if((n >= 12344) && (n < 12363))
; 12344 to 12362
.if((n .mod 3 = 2) && ((n > 12343 && n < 12363)))
	; 4115..4123 bytes
	delay_n (n-2)
	nop
.elseif(n = 12345)
	; 4118 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12346)
	; 4116 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12348)
	; 4116 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12349)
	; 4120 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12351)
	; 4120 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12352)
	; 4118 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12354)
	; 4118 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12355)
	; 4122 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12357)
	; 4122 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12358)
	; 4120 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12360)
	; 4120 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12361)
	; 4124 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 12363 to 12382
.if(n = 12363)
	; 4124 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 12364)
	; 4122 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 12364 && n < 12381)))
	; 4123..4129 bytes
	delay_n (n-2)
	nop
.elseif(n = 12366)
	; 4122 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12367)
	; 4126 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12369)
	; 4126 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12370)
	; 4124 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12372)
	; 4124 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12373)
	; 4128 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12375)
	; 4128 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12376)
	; 4126 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12378)
	; 4126 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12379)
	; 4130 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12381)
	; 4130 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12382)
	; 4128 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 12383) && (n < 12402))
; 12383 to 12401
.if((n .mod 3 = 2) && ((n > 12382 && n < 12402)))
	; 4129..4137 bytes
	delay_n (n-2)
	nop
.elseif(n = 12384)
	; 4128 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12385)
	; 4132 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12387)
	; 4132 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12388)
	; 4130 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12390)
	; 4130 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12391)
	; 4134 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12393)
	; 4134 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12394)
	; 4132 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12396)
	; 4132 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12397)
	; 4136 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12399)
	; 4136 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12400)
	; 4134 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 12402 to 12421
.if(n = 12402)
	; 4134 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12403)
	; 4138 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 12403 && n < 12420)))
	; 4135..4143 bytes
	delay_n (n-2)
	nop
.elseif(n = 12405)
	; 4138 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12406)
	; 4136 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12408)
	; 4136 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12409)
	; 4140 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12411)
	; 4140 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12412)
	; 4138 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12414)
	; 4138 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12415)
	; 4142 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12417)
	; 4142 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4144 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.else
.if((n >= 12422) && (n < 12461))
.if((n >= 12422) && (n < 12441))
; 12422 to 12440
.if((n .mod 3 = 2) && ((n > 12421 && n < 12441)))
	; 4141..4149 bytes
	delay_n (n-2)
	nop
.elseif(n = 12423)
	; 4144 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12424)
	; 4142 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12426)
	; 4142 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12427)
	; 4146 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12429)
	; 4146 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12430)
	; 4144 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12432)
	; 4144 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12433)
	; 4148 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12435)
	; 4148 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12436)
	; 4146 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12438)
	; 4146 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12439)
	; 4150 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 12441 to 12460
.if(n = 12441)
	; 4150 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12442)
	; 4148 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 12442 && n < 12459)))
	; 4149..4155 bytes
	delay_n (n-2)
	nop
.elseif(n = 12444)
	; 4148 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12445)
	; 4152 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12447)
	; 4152 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12448)
	; 4150 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12450)
	; 4150 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12451)
	; 4154 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12453)
	; 4154 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12454)
	; 4152 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12456)
	; 4152 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12457)
	; 4156 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12459)
	; 4156 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12460)
	; 4154 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 12461) && (n < 12481))
; 12461 to 12480
.if((n .mod 3 = 2) && ((n > 12460 && n < 12480)))
	; 4155..4163 bytes
	delay_n (n-2)
	nop
.elseif(n = 12462)
	; 4154 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12463)
	; 4158 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12465)
	; 4158 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12466)
	; 4156 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12468)
	; 4156 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12469)
	; 4160 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12471)
	; 4160 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4162 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12477)
	; 4162 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12478)
	; 4160 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12480)
	; 4160 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 12481 to 12500
.if(n = 12481)
	; 4164 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 12481 && n < 12501)))
	; 4161..4169 bytes
	delay_n (n-2)
	nop
.elseif(n = 12483)
	; 4164 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12484)
	; 4162 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12486)
	; 4162 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12487)
	; 4166 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12489)
	; 4166 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12490)
	; 4164 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12492)
	; 4164 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12493)
	; 4168 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12495)
	; 4168 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12496)
	; 4166 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12498)
	; 4166 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12499)
	; 4170 bytes
	delay_n 79
	delay_n (n-79)
.endif
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
.if((n >= 12501) && (n < 12520))
; 12501 to 12519
.if(n = 12501)
	; 4170 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12502)
	; 4168 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 12502 && n < 12519)))
	; 4169..4175 bytes
	delay_n (n-2)
	nop
.elseif(n = 12504)
	; 4168 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12505)
	; 4172 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12507)
	; 4172 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12508)
	; 4170 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12510)
	; 4170 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12511)
	; 4174 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12513)
	; 4174 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12514)
	; 4172 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12516)
	; 4172 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12517)
	; 4176 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12519)
	; 4176 bytes
	delay_n 45
	delay_n (n-45)
.endif
.else
; 12520 to 12539
.if(n = 12520)
	; 4174 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 12520 && n < 12540)))
	; 4175..4183 bytes
	delay_n (n-2)
	nop
.elseif(n = 12522)
	; 4174 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12523)
	; 4178 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12525)
	; 4178 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4180 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12531)
	; 4180 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12532)
	; 4178 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12534)
	; 4178 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12535)
	; 4182 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12537)
	; 4182 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12538)
	; 4180 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 12540) && (n < 12559))
; 12540 to 12558
.if(n = 12540)
	; 4180 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12541)
	; 4184 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 12541 && n < 12558)))
	; 4181..4189 bytes
	delay_n (n-2)
	nop
.elseif(n = 12543)
	; 4184 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12544)
	; 4182 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12546)
	; 4182 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12547)
	; 4186 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12549)
	; 4186 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12550)
	; 4184 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12552)
	; 4184 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12553)
	; 4188 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12555)
	; 4188 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12556)
	; 4186 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12558)
	; 4186 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 12559 to 12578
.if(n = 12559)
	; 4190 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 12559 && n < 12579)))
	; 4187..4195 bytes
	delay_n (n-2)
	nop
.elseif(n = 12561)
	; 4190 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12562)
	; 4188 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12564)
	; 4188 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12565)
	; 4192 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12567)
	; 4192 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12568)
	; 4190 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12570)
	; 4190 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12571)
	; 4194 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12573)
	; 4194 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12574)
	; 4192 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12576)
	; 4192 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12577)
	; 4196 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.else
.if((n >= 12579) && (n < 12618))
.if((n >= 12579) && (n < 12598))
; 12579 to 12597
.if(n = 12579)
	; 4196 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 12580)
	; 4194 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 12580 && n < 12597)))
	; 4195..4201 bytes
	delay_n (n-2)
	nop
.elseif(n = 12582)
	; 4194 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12583)
	; 4198 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12585)
	; 4198 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12586)
	; 4196 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12588)
	; 4196 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12589)
	; 4200 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12591)
	; 4200 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12592)
	; 4198 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12594)
	; 4198 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12595)
	; 4202 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12597)
	; 4202 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 12598 to 12617
.if(n = 12598)
	; 4200 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 12598 && n < 12618)))
	; 4201..4209 bytes
	delay_n (n-2)
	nop
.elseif(n = 12600)
	; 4200 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12601)
	; 4204 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12603)
	; 4204 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12604)
	; 4202 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12606)
	; 4202 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12607)
	; 4206 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12609)
	; 4206 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12610)
	; 4204 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12612)
	; 4204 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12613)
	; 4208 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12615)
	; 4208 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12616)
	; 4206 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 12618) && (n < 12637))
; 12618 to 12636
.if(n = 12618)
	; 4206 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12619)
	; 4210 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 12619 && n < 12636)))
	; 4207..4215 bytes
	delay_n (n-2)
	nop
.elseif(n = 12621)
	; 4210 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12622)
	; 4208 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12624)
	; 4208 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12625)
	; 4212 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12627)
	; 4212 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12628)
	; 4210 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12630)
	; 4210 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12631)
	; 4214 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12633)
	; 4214 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 12634)
	; 4212 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 12636)
	; 4212 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 12637 to 12656
.if(n = 12637)
	; 4216 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 12637 && n < 12657)))
	; 4213..4221 bytes
	delay_n (n-2)
	nop
.elseif(n = 12639)
	; 4216 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12640)
	; 4214 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12642)
	; 4214 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12643)
	; 4218 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12645)
	; 4218 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12646)
	; 4216 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12648)
	; 4216 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12649)
	; 4220 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12651)
	; 4220 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12652)
	; 4218 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12654)
	; 4218 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12655)
	; 4222 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
.if((n >= 12657) && (n < 12696))
.if((n >= 12657) && (n < 12676))
; 12657 to 12675
.if(n = 12657)
	; 4222 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12658)
	; 4220 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 12658 && n < 12675)))
	; 4221..4227 bytes
	delay_n (n-2)
	nop
.elseif(n = 12660)
	; 4220 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12661)
	; 4224 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12663)
	; 4224 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12664)
	; 4222 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12666)
	; 4222 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12667)
	; 4226 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12669)
	; 4226 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12670)
	; 4224 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12672)
	; 4224 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12673)
	; 4228 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12675)
	; 4228 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 12676 to 12695
.if(n = 12676)
	; 4226 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 12676 && n < 12696)))
	; 4227..4235 bytes
	delay_n (n-2)
	nop
.elseif(n = 12678)
	; 4226 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12679)
	; 4230 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12681)
	; 4230 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12682)
	; 4228 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12684)
	; 4228 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12685)
	; 4232 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12687)
	; 4232 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4234 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12693)
	; 4234 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12694)
	; 4232 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 12696) && (n < 12715))
; 12696 to 12714
.if(n = 12696)
	; 4232 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12697)
	; 4236 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 12697 && n < 12714)))
	; 4233..4241 bytes
	delay_n (n-2)
	nop
.elseif(n = 12699)
	; 4236 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12700)
	; 4234 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12702)
	; 4234 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12703)
	; 4238 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12705)
	; 4238 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12706)
	; 4236 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12708)
	; 4236 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12709)
	; 4240 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12711)
	; 4240 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12712)
	; 4238 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12714)
	; 4238 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 12715 to 12734
.if(n = 12715)
	; 4242 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 12715 && n < 12735)))
	; 4239..4247 bytes
	delay_n (n-2)
	nop
.elseif(n = 12717)
	; 4242 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12718)
	; 4240 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12720)
	; 4240 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12721)
	; 4244 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12723)
	; 4244 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12724)
	; 4242 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12726)
	; 4242 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12727)
	; 4246 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12729)
	; 4246 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12730)
	; 4244 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12732)
	; 4244 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12733)
	; 4248 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.else
.if((n >= 12735) && (n < 12774))
.if((n >= 12735) && (n < 12754))
; 12735 to 12753
.if(n = 12735)
	; 4248 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12736)
	; 4246 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 12736 && n < 12753)))
	; 4247..4253 bytes
	delay_n (n-2)
	nop
.elseif(n = 12738)
	; 4246 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12739)
	; 4250 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12741)
	; 4250 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4252 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12747)
	; 4252 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12748)
	; 4250 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12750)
	; 4250 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12751)
	; 4254 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12753)
	; 4254 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 12754 to 12773
.if(n = 12754)
	; 4252 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 12754 && n < 12774)))
	; 4253..4261 bytes
	delay_n (n-2)
	nop
.elseif(n = 12756)
	; 4252 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12757)
	; 4256 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12759)
	; 4256 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12760)
	; 4254 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12762)
	; 4254 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12763)
	; 4258 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12765)
	; 4258 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12766)
	; 4256 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12768)
	; 4256 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12769)
	; 4260 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12771)
	; 4260 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12772)
	; 4258 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 12774) && (n < 12793))
; 12774 to 12792
.if(n = 12774)
	; 4258 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12775)
	; 4262 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 12775 && n < 12792)))
	; 4259..4267 bytes
	delay_n (n-2)
	nop
.elseif(n = 12777)
	; 4262 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12778)
	; 4260 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12780)
	; 4260 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12781)
	; 4264 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12783)
	; 4264 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12784)
	; 4262 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12786)
	; 4262 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12787)
	; 4266 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12789)
	; 4266 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12790)
	; 4264 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12792)
	; 4264 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 12793 to 12812
.if(n = 12793)
	; 4268 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 12793 && n < 12813)))
	; 4265..4273 bytes
	delay_n (n-2)
	nop
.elseif(n = 12795)
	; 4268 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4270 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12801)
	; 4270 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12802)
	; 4268 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12804)
	; 4268 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12805)
	; 4272 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12807)
	; 4272 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12808)
	; 4270 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12810)
	; 4270 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12811)
	; 4274 bytes
	delay_n 67
	delay_n (n-67)
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
	; 4274 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12814)
	; 4272 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 12814 && n < 12831)))
	; 4273..4279 bytes
	delay_n (n-2)
	nop
.elseif(n = 12816)
	; 4272 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12817)
	; 4276 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12819)
	; 4276 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12820)
	; 4274 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12822)
	; 4274 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12823)
	; 4278 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12825)
	; 4278 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12826)
	; 4276 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12828)
	; 4276 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12829)
	; 4280 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12831)
	; 4280 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 12832 to 12851
.if(n = 12832)
	; 4278 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 12832 && n < 12852)))
	; 4279..4287 bytes
	delay_n (n-2)
	nop
.elseif(n = 12834)
	; 4278 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12835)
	; 4282 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12837)
	; 4282 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12838)
	; 4280 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12840)
	; 4280 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12841)
	; 4284 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12843)
	; 4284 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12844)
	; 4282 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12846)
	; 4282 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12847)
	; 4286 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12849)
	; 4286 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 12850)
	; 4284 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 12852) && (n < 12871))
; 12852 to 12870
.if(n = 12852)
	; 4284 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 12853)
	; 4288 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 12853 && n < 12870)))
	; 4285..4293 bytes
	delay_n (n-2)
	nop
.elseif(n = 12855)
	; 4288 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12856)
	; 4286 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12858)
	; 4286 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12859)
	; 4290 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12861)
	; 4290 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12862)
	; 4288 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12864)
	; 4288 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12865)
	; 4292 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12867)
	; 4292 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12868)
	; 4290 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12870)
	; 4290 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 12871 to 12890
.if(n = 12871)
	; 4294 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 12871 && n < 12891)))
	; 4291..4299 bytes
	delay_n (n-2)
	nop
.elseif(n = 12873)
	; 4294 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12874)
	; 4292 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12876)
	; 4292 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12877)
	; 4296 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12879)
	; 4296 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12880)
	; 4294 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12882)
	; 4294 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12883)
	; 4298 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12885)
	; 4298 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12886)
	; 4296 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12888)
	; 4296 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12889)
	; 4300 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 12891) && (n < 12930))
.if((n >= 12891) && (n < 12910))
; 12891 to 12909
.if(n = 12891)
	; 4300 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12892)
	; 4298 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 12892 && n < 12909)))
	; 4299..4305 bytes
	delay_n (n-2)
	nop
.elseif(n = 12894)
	; 4298 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12895)
	; 4302 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12897)
	; 4302 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12898)
	; 4300 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12900)
	; 4300 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12901)
	; 4304 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12903)
	; 4304 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4306 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12909)
	; 4306 bytes
	delay_n 57
	delay_n (n-57)
.endif
.else
; 12910 to 12929
.if(n = 12910)
	; 4304 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 12910 && n < 12930)))
	; 4305..4313 bytes
	delay_n (n-2)
	nop
.elseif(n = 12912)
	; 4304 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12913)
	; 4308 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 12915)
	; 4308 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12916)
	; 4306 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12918)
	; 4306 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12919)
	; 4310 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12921)
	; 4310 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12922)
	; 4308 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12924)
	; 4308 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12925)
	; 4312 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12927)
	; 4312 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12928)
	; 4310 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 12930) && (n < 12949))
; 12930 to 12948
.if(n = 12930)
	; 4310 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12931)
	; 4314 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 12931 && n < 12948)))
	; 4311..4319 bytes
	delay_n (n-2)
	nop
.elseif(n = 12933)
	; 4314 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12934)
	; 4312 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 12936)
	; 4312 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12937)
	; 4316 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12939)
	; 4316 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12940)
	; 4314 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12942)
	; 4314 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12943)
	; 4318 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12945)
	; 4318 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 12946)
	; 4316 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 12948)
	; 4316 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 12949 to 12968
.if(n = 12949)
	; 4320 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 12949 && n < 12969)))
	; 4317..4325 bytes
	delay_n (n-2)
	nop
.elseif(n = 12951)
	; 4320 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 12952)
	; 4318 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12954)
	; 4318 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 12955)
	; 4322 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 12957)
	; 4322 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4324 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 12963)
	; 4324 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 12964)
	; 4322 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 12966)
	; 4322 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 12967)
	; 4326 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.endif
.endif
.else
.if((n >= 12969) && (n < 13047))
.if((n >= 12969) && (n < 13008))
.if((n >= 12969) && (n < 12988))
; 12969 to 12987
.if(n = 12969)
	; 4326 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 12970)
	; 4324 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 12970 && n < 12987)))
	; 4325..4331 bytes
	delay_n (n-2)
	nop
.elseif(n = 12972)
	; 4324 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12973)
	; 4328 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 12975)
	; 4328 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12976)
	; 4326 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 12978)
	; 4326 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 12979)
	; 4330 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 12981)
	; 4330 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 12982)
	; 4328 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12984)
	; 4328 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 12985)
	; 4332 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 12987)
	; 4332 bytes
	delay_n 81
	delay_n (n-81)
.endif
.else
; 12988 to 13007
.if(n = 12988)
	; 4330 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 12988 && n < 13008)))
	; 4331..4339 bytes
	delay_n (n-2)
	nop
.elseif(n = 12990)
	; 4330 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 12991)
	; 4334 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 12993)
	; 4334 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12994)
	; 4332 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12996)
	; 4332 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12997)
	; 4336 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12999)
	; 4336 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13000)
	; 4334 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13002)
	; 4334 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13003)
	; 4338 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13005)
	; 4338 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13006)
	; 4336 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 13008) && (n < 13027))
; 13008 to 13026
.if(n = 13008)
	; 4336 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13009)
	; 4340 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 13009 && n < 13026)))
	; 4337..4345 bytes
	delay_n (n-2)
	nop
.elseif(n = 13011)
	; 4340 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4342 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13017)
	; 4342 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13018)
	; 4340 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13020)
	; 4340 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13021)
	; 4344 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13023)
	; 4344 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13024)
	; 4342 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13026)
	; 4342 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 13027 to 13046
.if(n = 13027)
	; 4346 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 13027 && n < 13047)))
	; 4343..4351 bytes
	delay_n (n-2)
	nop
.elseif(n = 13029)
	; 4346 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13030)
	; 4344 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13032)
	; 4344 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13033)
	; 4348 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13035)
	; 4348 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13036)
	; 4346 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13038)
	; 4346 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13039)
	; 4350 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13041)
	; 4350 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13042)
	; 4348 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13044)
	; 4348 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13045)
	; 4352 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.endif
.else
.if((n >= 13047) && (n < 13086))
.if((n >= 13047) && (n < 13066))
; 13047 to 13065
.if(n = 13047)
	; 4352 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13048)
	; 4350 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 13048 && n < 13065)))
	; 4351..4357 bytes
	delay_n (n-2)
	nop
.elseif(n = 13050)
	; 4350 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13051)
	; 4354 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13053)
	; 4354 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13054)
	; 4352 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13056)
	; 4352 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13057)
	; 4356 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13059)
	; 4356 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13060)
	; 4354 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13062)
	; 4354 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13063)
	; 4358 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13065)
	; 4358 bytes
	delay_n 105
	delay_n (n-105)
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 4356 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 13066 && n < 13086)))
	; 4357..4365 bytes
	delay_n (n-2)
	nop
.elseif(n = 13068)
	; 4356 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13069)
	; 4360 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13071)
	; 4360 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13072)
	; 4358 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13074)
	; 4358 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13075)
	; 4362 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13077)
	; 4362 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13078)
	; 4360 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13080)
	; 4360 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13081)
	; 4364 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13083)
	; 4364 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13084)
	; 4362 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 13086) && (n < 13106))
; 13086 to 13105
.if(n = 13086)
	; 4362 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13087)
	; 4366 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 13087 && n < 13104)))
	; 4363..4371 bytes
	delay_n (n-2)
	nop
.elseif(n = 13089)
	; 4366 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13090)
	; 4364 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13092)
	; 4364 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13093)
	; 4368 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13095)
	; 4368 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13096)
	; 4366 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13098)
	; 4366 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13099)
	; 4370 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13101)
	; 4370 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13102)
	; 4368 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13104)
	; 4368 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13105)
	; 4372 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 13106 to 13125
.if((n .mod 3 = 2) && ((n > 13105 && n < 13125)))
	; 4369..4377 bytes
	delay_n (n-2)
	nop
.elseif(n = 13107)
	; 4372 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13108)
	; 4370 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13110)
	; 4370 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13111)
	; 4374 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13113)
	; 4374 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13114)
	; 4372 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13116)
	; 4372 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13117)
	; 4376 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13119)
	; 4376 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4378 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13125)
	; 4378 bytes
	delay_n 57
	delay_n (n-57)
.endif
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
.if((n >= 13126) && (n < 13145))
; 13126 to 13144
.if(n = 13126)
	; 4376 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 13126 && n < 13143)))
	; 4377..4383 bytes
	delay_n (n-2)
	nop
.elseif(n = 13128)
	; 4376 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13129)
	; 4380 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13131)
	; 4380 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13132)
	; 4378 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13134)
	; 4378 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13135)
	; 4382 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13137)
	; 4382 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13138)
	; 4380 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13140)
	; 4380 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13141)
	; 4384 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13143)
	; 4384 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13144)
	; 4382 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 13145 to 13164
.if((n .mod 3 = 2) && ((n > 13144 && n < 13164)))
	; 4383..4391 bytes
	delay_n (n-2)
	nop
.elseif(n = 13146)
	; 4382 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13147)
	; 4386 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13149)
	; 4386 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13150)
	; 4384 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13152)
	; 4384 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13153)
	; 4388 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13155)
	; 4388 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13156)
	; 4386 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13158)
	; 4386 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13159)
	; 4390 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13161)
	; 4390 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13162)
	; 4388 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13164)
	; 4388 bytes
	delay_n 96
	delay_n (n-96)
.endif
.endif
.else
.if((n >= 13165) && (n < 13184))
; 13165 to 13183
.if(n = 13165)
	; 4392 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 13165 && n < 13182)))
	; 4389..4397 bytes
	delay_n (n-2)
	nop
.elseif(n = 13167)
	; 4392 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13168)
	; 4390 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13170)
	; 4390 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13171)
	; 4394 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13173)
	; 4394 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4396 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13179)
	; 4396 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13180)
	; 4394 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13182)
	; 4394 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13183)
	; 4398 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 13184 to 13203
.if((n .mod 3 = 2) && ((n > 13183 && n < 13203)))
	; 4395..4403 bytes
	delay_n (n-2)
	nop
.elseif(n = 13185)
	; 4398 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13186)
	; 4396 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13188)
	; 4396 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13189)
	; 4400 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13191)
	; 4400 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13192)
	; 4398 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13194)
	; 4398 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13195)
	; 4402 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13197)
	; 4402 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13198)
	; 4400 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13200)
	; 4400 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13201)
	; 4404 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13203)
	; 4404 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.endif
.else
.if((n >= 13204) && (n < 13243))
.if((n >= 13204) && (n < 13223))
; 13204 to 13222
.if(n = 13204)
	; 4402 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 13204 && n < 13221)))
	; 4403..4409 bytes
	delay_n (n-2)
	nop
.elseif(n = 13206)
	; 4402 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13207)
	; 4406 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13209)
	; 4406 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13210)
	; 4404 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13212)
	; 4404 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13213)
	; 4408 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13215)
	; 4408 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13216)
	; 4406 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13218)
	; 4406 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13219)
	; 4410 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13221)
	; 4410 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13222)
	; 4408 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 13223 to 13242
.if((n .mod 3 = 2) && ((n > 13222 && n < 13242)))
	; 4409..4417 bytes
	delay_n (n-2)
	nop
.elseif(n = 13224)
	; 4408 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13225)
	; 4412 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13227)
	; 4412 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4414 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13233)
	; 4414 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13234)
	; 4412 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13236)
	; 4412 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13237)
	; 4416 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13239)
	; 4416 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13240)
	; 4414 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13242)
	; 4414 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 13243) && (n < 13262))
; 13243 to 13261
.if(n = 13243)
	; 4418 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 13243 && n < 13260)))
	; 4415..4423 bytes
	delay_n (n-2)
	nop
.elseif(n = 13245)
	; 4418 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13246)
	; 4416 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13248)
	; 4416 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13249)
	; 4420 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13251)
	; 4420 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13252)
	; 4418 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13254)
	; 4418 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13255)
	; 4422 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13257)
	; 4422 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13258)
	; 4420 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13260)
	; 4420 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13261)
	; 4424 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 13262 to 13281
.if((n .mod 3 = 2) && ((n > 13261 && n < 13281)))
	; 4421..4429 bytes
	delay_n (n-2)
	nop
.elseif(n = 13263)
	; 4424 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13264)
	; 4422 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13266)
	; 4422 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13267)
	; 4426 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13269)
	; 4426 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13270)
	; 4424 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13272)
	; 4424 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13273)
	; 4428 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13275)
	; 4428 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13276)
	; 4426 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13278)
	; 4426 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13279)
	; 4430 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13281)
	; 4430 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.endif
.endif
.else
.if((n >= 13282) && (n < 13360))
.if((n >= 13282) && (n < 13321))
.if((n >= 13282) && (n < 13301))
; 13282 to 13300
.if(n = 13282)
	; 4428 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 13282 && n < 13299)))
	; 4429..4435 bytes
	delay_n (n-2)
	nop
.elseif(n = 13284)
	; 4428 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13285)
	; 4432 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13287)
	; 4432 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13288)
	; 4430 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13290)
	; 4430 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13291)
	; 4434 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13293)
	; 4434 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13294)
	; 4432 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13296)
	; 4432 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13297)
	; 4436 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13299)
	; 4436 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13300)
	; 4434 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 13301 to 13320
.if((n .mod 3 = 2) && ((n > 13300 && n < 13320)))
	; 4435..4443 bytes
	delay_n (n-2)
	nop
.elseif(n = 13302)
	; 4434 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13303)
	; 4438 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13305)
	; 4438 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13306)
	; 4436 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13308)
	; 4436 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13309)
	; 4440 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13311)
	; 4440 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13312)
	; 4438 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13314)
	; 4438 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13315)
	; 4442 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13317)
	; 4442 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13318)
	; 4440 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13320)
	; 4440 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.else
.if((n >= 13321) && (n < 13340))
; 13321 to 13339
.if(n = 13321)
	; 4444 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 13321 && n < 13338)))
	; 4441..4449 bytes
	delay_n (n-2)
	nop
.elseif(n = 13323)
	; 4444 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13324)
	; 4442 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13326)
	; 4442 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13327)
	; 4446 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13329)
	; 4446 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13330)
	; 4444 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13332)
	; 4444 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13333)
	; 4448 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13335)
	; 4448 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4450 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 13340 to 13359
.if((n .mod 3 = 2) && ((n > 13339 && n < 13359)))
	; 4447..4455 bytes
	delay_n (n-2)
	nop
.elseif(n = 13341)
	; 4450 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13342)
	; 4448 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13344)
	; 4448 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13345)
	; 4452 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13347)
	; 4452 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13348)
	; 4450 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13350)
	; 4450 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13351)
	; 4454 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13353)
	; 4454 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13354)
	; 4452 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13356)
	; 4452 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13357)
	; 4456 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13359)
	; 4456 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.endif
.else
.if((n >= 13360) && (n < 13399))
.if((n >= 13360) && (n < 13379))
; 13360 to 13378
.if(n = 13360)
	; 4454 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 13360 && n < 13377)))
	; 4455..4461 bytes
	delay_n (n-2)
	nop
.elseif(n = 13362)
	; 4454 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13363)
	; 4458 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13365)
	; 4458 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13366)
	; 4456 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13368)
	; 4456 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13369)
	; 4460 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13371)
	; 4460 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13372)
	; 4458 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13374)
	; 4458 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13375)
	; 4462 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13377)
	; 4462 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13378)
	; 4460 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 13379 to 13398
.if((n .mod 3 = 2) && ((n > 13378 && n < 13398)))
	; 4461..4469 bytes
	delay_n (n-2)
	nop
.elseif(n = 13380)
	; 4460 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13381)
	; 4464 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13383)
	; 4464 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13384)
	; 4462 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13386)
	; 4462 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13387)
	; 4466 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13389)
	; 4466 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4468 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13395)
	; 4468 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13396)
	; 4466 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13398)
	; 4466 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.else
.if((n >= 13399) && (n < 13418))
; 13399 to 13417
.if(n = 13399)
	; 4470 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 13399 && n < 13416)))
	; 4467..4475 bytes
	delay_n (n-2)
	nop
.elseif(n = 13401)
	; 4470 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13402)
	; 4468 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13404)
	; 4468 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13405)
	; 4472 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13407)
	; 4472 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13408)
	; 4470 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13410)
	; 4470 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13411)
	; 4474 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13413)
	; 4474 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13414)
	; 4472 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13416)
	; 4472 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13417)
	; 4476 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 13418 to 13437
.if((n .mod 3 = 2) && ((n > 13417 && n < 13437)))
	; 4473..4481 bytes
	delay_n (n-2)
	nop
.elseif(n = 13419)
	; 4476 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13420)
	; 4474 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13422)
	; 4474 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13423)
	; 4478 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13425)
	; 4478 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13426)
	; 4476 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13428)
	; 4476 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13429)
	; 4480 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13431)
	; 4480 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13432)
	; 4478 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13434)
	; 4478 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13435)
	; 4482 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13437)
	; 4482 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
.if((n >= 13438) && (n < 13477))
.if((n >= 13438) && (n < 13457))
; 13438 to 13456
.if(n = 13438)
	; 4480 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 13438 && n < 13455)))
	; 4481..4487 bytes
	delay_n (n-2)
	nop
.elseif(n = 13440)
	; 4480 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13441)
	; 4484 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13443)
	; 4484 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4486 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13449)
	; 4486 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13450)
	; 4484 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13452)
	; 4484 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13453)
	; 4488 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13455)
	; 4488 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13456)
	; 4486 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 13457 to 13476
.if((n .mod 3 = 2) && ((n > 13456 && n < 13476)))
	; 4487..4495 bytes
	delay_n (n-2)
	nop
.elseif(n = 13458)
	; 4486 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13459)
	; 4490 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13461)
	; 4490 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13462)
	; 4488 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13464)
	; 4488 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13465)
	; 4492 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13467)
	; 4492 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13468)
	; 4490 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13470)
	; 4490 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13471)
	; 4494 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13473)
	; 4494 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13474)
	; 4492 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13476)
	; 4492 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.else
.if((n >= 13477) && (n < 13496))
; 13477 to 13495
.if(n = 13477)
	; 4496 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 13477 && n < 13494)))
	; 4493..4501 bytes
	delay_n (n-2)
	nop
.elseif(n = 13479)
	; 4496 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13480)
	; 4494 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13482)
	; 4494 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13483)
	; 4498 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13485)
	; 4498 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13486)
	; 4496 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13488)
	; 4496 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13489)
	; 4500 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13491)
	; 4500 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13492)
	; 4498 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13494)
	; 4498 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13495)
	; 4502 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 13496 to 13515
.if((n .mod 3 = 2) && ((n > 13495 && n < 13515)))
	; 4499..4507 bytes
	delay_n (n-2)
	nop
.elseif(n = 13497)
	; 4502 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4504 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13503)
	; 4504 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13504)
	; 4502 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13506)
	; 4502 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13507)
	; 4506 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13509)
	; 4506 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13510)
	; 4504 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13512)
	; 4504 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13513)
	; 4508 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13515)
	; 4508 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.endif
.else
.if((n >= 13516) && (n < 13555))
.if((n >= 13516) && (n < 13535))
; 13516 to 13534
.if(n = 13516)
	; 4506 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 13516 && n < 13533)))
	; 4507..4513 bytes
	delay_n (n-2)
	nop
.elseif(n = 13518)
	; 4506 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13519)
	; 4510 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13521)
	; 4510 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13522)
	; 4508 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13524)
	; 4508 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13525)
	; 4512 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13527)
	; 4512 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13528)
	; 4510 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13530)
	; 4510 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13531)
	; 4514 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13533)
	; 4514 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13534)
	; 4512 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 13535 to 13554
.if((n .mod 3 = 2) && ((n > 13534 && n < 13554)))
	; 4513..4521 bytes
	delay_n (n-2)
	nop
.elseif(n = 13536)
	; 4512 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13537)
	; 4516 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13539)
	; 4516 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13540)
	; 4514 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13542)
	; 4514 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13543)
	; 4518 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13545)
	; 4518 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13546)
	; 4516 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13548)
	; 4516 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13549)
	; 4520 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13551)
	; 4520 bytes
	delay_n 105
	delay_n (n-105)
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
.endif
.else
.if((n >= 13555) && (n < 13574))
; 13555 to 13573
.if(n = 13555)
	; 4522 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 13555 && n < 13572)))
	; 4519..4527 bytes
	delay_n (n-2)
	nop
.elseif(n = 13557)
	; 4522 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13558)
	; 4520 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13560)
	; 4520 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13561)
	; 4524 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13563)
	; 4524 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13564)
	; 4522 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13566)
	; 4522 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13567)
	; 4526 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13569)
	; 4526 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13570)
	; 4524 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13572)
	; 4524 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13573)
	; 4528 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 13574 to 13593
.if((n .mod 3 = 2) && ((n > 13573 && n < 13593)))
	; 4525..4533 bytes
	delay_n (n-2)
	nop
.elseif(n = 13575)
	; 4528 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13576)
	; 4526 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13578)
	; 4526 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13579)
	; 4530 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13581)
	; 4530 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13582)
	; 4528 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13584)
	; 4528 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13585)
	; 4532 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13587)
	; 4532 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13588)
	; 4530 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13590)
	; 4530 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13591)
	; 4534 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13593)
	; 4534 bytes
	delay_n 93
	delay_n (n-93)
.endif
.endif
.endif
.endif
.else
.if((n >= 13594) && (n < 13672))
.if((n >= 13594) && (n < 13633))
.if((n >= 13594) && (n < 13613))
; 13594 to 13612
.if(n = 13594)
	; 4532 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 13594 && n < 13611)))
	; 4533..4539 bytes
	delay_n (n-2)
	nop
.elseif(n = 13596)
	; 4532 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13597)
	; 4536 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13599)
	; 4536 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13600)
	; 4534 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13602)
	; 4534 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13603)
	; 4538 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13605)
	; 4538 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4540 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13611)
	; 4540 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13612)
	; 4538 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 13613 to 13632
.if((n .mod 3 = 2) && ((n > 13612 && n < 13632)))
	; 4539..4547 bytes
	delay_n (n-2)
	nop
.elseif(n = 13614)
	; 4538 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13615)
	; 4542 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13617)
	; 4542 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13618)
	; 4540 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13620)
	; 4540 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13621)
	; 4544 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13623)
	; 4544 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13624)
	; 4542 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13626)
	; 4542 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13627)
	; 4546 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13629)
	; 4546 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13630)
	; 4544 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13632)
	; 4544 bytes
	delay_n 24
	delay_n (n-24)
.endif
.endif
.else
.if((n >= 13633) && (n < 13652))
; 13633 to 13651
.if(n = 13633)
	; 4548 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 13633 && n < 13650)))
	; 4545..4553 bytes
	delay_n (n-2)
	nop
.elseif(n = 13635)
	; 4548 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13636)
	; 4546 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13638)
	; 4546 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13639)
	; 4550 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13641)
	; 4550 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13642)
	; 4548 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13644)
	; 4548 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13645)
	; 4552 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13647)
	; 4552 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13648)
	; 4550 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13650)
	; 4550 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13651)
	; 4554 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 13652 to 13671
.if((n .mod 3 = 2) && ((n > 13651 && n < 13671)))
	; 4551..4559 bytes
	delay_n (n-2)
	nop
.elseif(n = 13653)
	; 4554 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13654)
	; 4552 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13656)
	; 4552 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13657)
	; 4556 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13659)
	; 4556 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4558 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13665)
	; 4558 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13666)
	; 4556 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13668)
	; 4556 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13669)
	; 4560 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13671)
	; 4560 bytes
	delay_n 117
	delay_n (n-117)
.endif
.endif
.endif
.else
.if((n >= 13672) && (n < 13711))
.if((n >= 13672) && (n < 13691))
; 13672 to 13690
.if(n = 13672)
	; 4558 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 13672 && n < 13689)))
	; 4559..4565 bytes
	delay_n (n-2)
	nop
.elseif(n = 13674)
	; 4558 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13675)
	; 4562 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13677)
	; 4562 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13678)
	; 4560 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13680)
	; 4560 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13681)
	; 4564 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13683)
	; 4564 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13684)
	; 4562 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13686)
	; 4562 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13687)
	; 4566 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13689)
	; 4566 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13690)
	; 4564 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 13691 to 13710
.if((n .mod 3 = 2) && ((n > 13690 && n < 13710)))
	; 4565..4573 bytes
	delay_n (n-2)
	nop
.elseif(n = 13692)
	; 4564 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13693)
	; 4568 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13695)
	; 4568 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13696)
	; 4566 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13698)
	; 4566 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13699)
	; 4570 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13701)
	; 4570 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13702)
	; 4568 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13704)
	; 4568 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13705)
	; 4572 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13707)
	; 4572 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13708)
	; 4570 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13710)
	; 4570 bytes
	delay_n 48
	delay_n (n-48)
.endif
.endif
.else
.if((n >= 13711) && (n < 13731))
; 13711 to 13730
.if(n = 13711)
	; 4574 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 13711 && n < 13731)))
	; 4571..4579 bytes
	delay_n (n-2)
	nop
.elseif(n = 13713)
	; 4574 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4576 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13719)
	; 4576 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13720)
	; 4574 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13722)
	; 4574 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13723)
	; 4578 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13725)
	; 4578 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13726)
	; 4576 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13728)
	; 4576 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13729)
	; 4580 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 13731 to 13750
.if(n = 13731)
	; 4580 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13732)
	; 4578 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 13732 && n < 13749)))
	; 4579..4585 bytes
	delay_n (n-2)
	nop
.elseif(n = 13734)
	; 4578 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13735)
	; 4582 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13737)
	; 4582 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13738)
	; 4580 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13740)
	; 4580 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13741)
	; 4584 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13743)
	; 4584 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13744)
	; 4582 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13746)
	; 4582 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13747)
	; 4586 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13749)
	; 4586 bytes
	delay_n 141
	delay_n (n-141)
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
.endif
.else
.if((n >= 13751) && (n < 14376))
.if((n >= 13751) && (n < 14063))
.if((n >= 13751) && (n < 13907))
.if((n >= 13751) && (n < 13829))
.if((n >= 13751) && (n < 13790))
.if((n >= 13751) && (n < 13770))
; 13751 to 13769
.if((n .mod 3 = 2) && ((n > 13750 && n < 13770)))
	; 4585..4593 bytes
	delay_n (n-2)
	nop
.elseif(n = 13752)
	; 4584 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13753)
	; 4588 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13755)
	; 4588 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13756)
	; 4586 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13758)
	; 4586 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13759)
	; 4590 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13761)
	; 4590 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13762)
	; 4588 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13764)
	; 4588 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13765)
	; 4592 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13767)
	; 4592 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 13768)
	; 4590 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 13770 to 13789
.if(n = 13770)
	; 4590 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13771)
	; 4594 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 13771 && n < 13788)))
	; 4591..4599 bytes
	delay_n (n-2)
	nop
.elseif(n = 13773)
	; 4594 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13774)
	; 4592 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13776)
	; 4592 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13777)
	; 4596 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13779)
	; 4596 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13780)
	; 4594 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13782)
	; 4594 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13783)
	; 4598 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13785)
	; 4598 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13786)
	; 4596 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13788)
	; 4596 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13789)
	; 4600 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.else
.if((n >= 13790) && (n < 13809))
; 13790 to 13808
.if((n .mod 3 = 2) && ((n > 13789 && n < 13809)))
	; 4597..4605 bytes
	delay_n (n-2)
	nop
.elseif(n = 13791)
	; 4600 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13792)
	; 4598 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13794)
	; 4598 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13795)
	; 4602 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13797)
	; 4602 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13798)
	; 4600 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13800)
	; 4600 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13801)
	; 4604 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13803)
	; 4604 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13804)
	; 4602 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13806)
	; 4602 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13807)
	; 4606 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 4606 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13810)
	; 4604 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 13810 && n < 13827)))
	; 4605..4611 bytes
	delay_n (n-2)
	nop
.elseif(n = 13812)
	; 4604 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13813)
	; 4608 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13815)
	; 4608 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13816)
	; 4606 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13818)
	; 4606 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13819)
	; 4610 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13821)
	; 4610 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4612 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13827)
	; 4612 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13828)
	; 4610 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.else
.if((n >= 13829) && (n < 13868))
.if((n >= 13829) && (n < 13848))
; 13829 to 13847
.if((n .mod 3 = 2) && ((n > 13828 && n < 13848)))
	; 4611..4619 bytes
	delay_n (n-2)
	nop
.elseif(n = 13830)
	; 4610 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13831)
	; 4614 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13833)
	; 4614 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13834)
	; 4612 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13836)
	; 4612 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13837)
	; 4616 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13839)
	; 4616 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13840)
	; 4614 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13842)
	; 4614 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13843)
	; 4618 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13845)
	; 4618 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13846)
	; 4616 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 13848 to 13867
.if(n = 13848)
	; 4616 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13849)
	; 4620 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 13849 && n < 13866)))
	; 4617..4625 bytes
	delay_n (n-2)
	nop
.elseif(n = 13851)
	; 4620 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13852)
	; 4618 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13854)
	; 4618 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13855)
	; 4622 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13857)
	; 4622 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13858)
	; 4620 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13860)
	; 4620 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13861)
	; 4624 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13863)
	; 4624 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13864)
	; 4622 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13866)
	; 4622 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13867)
	; 4626 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 13868) && (n < 13887))
; 13868 to 13886
.if((n .mod 3 = 2) && ((n > 13867 && n < 13887)))
	; 4623..4631 bytes
	delay_n (n-2)
	nop
.elseif(n = 13869)
	; 4626 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13870)
	; 4624 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13872)
	; 4624 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13873)
	; 4628 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13875)
	; 4628 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4630 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13881)
	; 4630 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13882)
	; 4628 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13884)
	; 4628 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13885)
	; 4632 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 13887 to 13906
.if(n = 13887)
	; 4632 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13888)
	; 4630 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 13888 && n < 13905)))
	; 4631..4637 bytes
	delay_n (n-2)
	nop
.elseif(n = 13890)
	; 4630 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13891)
	; 4634 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 13893)
	; 4634 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13894)
	; 4632 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13896)
	; 4632 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13897)
	; 4636 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13899)
	; 4636 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13900)
	; 4634 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13902)
	; 4634 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13903)
	; 4638 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13905)
	; 4638 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13906)
	; 4636 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.endif
.else
.if((n >= 13907) && (n < 13985))
.if((n >= 13907) && (n < 13946))
.if((n >= 13907) && (n < 13926))
; 13907 to 13925
.if((n .mod 3 = 2) && ((n > 13906 && n < 13926)))
	; 4637..4645 bytes
	delay_n (n-2)
	nop
.elseif(n = 13908)
	; 4636 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13909)
	; 4640 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 13911)
	; 4640 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13912)
	; 4638 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13914)
	; 4638 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13915)
	; 4642 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13917)
	; 4642 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13918)
	; 4640 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13920)
	; 4640 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13921)
	; 4644 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13923)
	; 4644 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13924)
	; 4642 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 13926 to 13945
.if(n = 13926)
	; 4642 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13927)
	; 4646 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 13927 && n < 13944)))
	; 4643..4651 bytes
	delay_n (n-2)
	nop
.elseif(n = 13929)
	; 4646 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4648 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13935)
	; 4648 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13936)
	; 4646 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13938)
	; 4646 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13939)
	; 4650 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13941)
	; 4650 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13942)
	; 4648 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13944)
	; 4648 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13945)
	; 4652 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 13946) && (n < 13965))
; 13946 to 13964
.if((n .mod 3 = 2) && ((n > 13945 && n < 13965)))
	; 4649..4657 bytes
	delay_n (n-2)
	nop
.elseif(n = 13947)
	; 4652 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13948)
	; 4650 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 13950)
	; 4650 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 13951)
	; 4654 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13953)
	; 4654 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 13954)
	; 4652 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13956)
	; 4652 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 13957)
	; 4656 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 13959)
	; 4656 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13960)
	; 4654 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 13962)
	; 4654 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 13963)
	; 4658 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 13965 to 13984
.if(n = 13965)
	; 4658 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13966)
	; 4656 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 13966 && n < 13983)))
	; 4657..4663 bytes
	delay_n (n-2)
	nop
.elseif(n = 13968)
	; 4656 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13969)
	; 4660 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13971)
	; 4660 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 13972)
	; 4658 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 13974)
	; 4658 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13975)
	; 4662 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13977)
	; 4662 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 13978)
	; 4660 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13980)
	; 4660 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 13981)
	; 4664 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 13983)
	; 4664 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 13984)
	; 4662 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 13985) && (n < 14024))
.if((n >= 13985) && (n < 14004))
; 13985 to 14003
.if((n .mod 3 = 2) && ((n > 13984 && n < 14004)))
	; 4663..4671 bytes
	delay_n (n-2)
	nop
.elseif(n = 13986)
	; 4662 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 13987)
	; 4666 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 13989)
	; 4666 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 13990)
	; 4664 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 13992)
	; 4664 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 13993)
	; 4668 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 13995)
	; 4668 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 13996)
	; 4666 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13998)
	; 4666 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13999)
	; 4670 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14001)
	; 4670 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14002)
	; 4668 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 14004 to 14023
.if(n = 14004)
	; 4668 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14005)
	; 4672 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 14005 && n < 14022)))
	; 4669..4677 bytes
	delay_n (n-2)
	nop
.elseif(n = 14007)
	; 4672 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14008)
	; 4670 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14010)
	; 4670 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14011)
	; 4674 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14013)
	; 4674 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14014)
	; 4672 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14016)
	; 4672 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14017)
	; 4676 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14019)
	; 4676 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14020)
	; 4674 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14022)
	; 4674 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14023)
	; 4678 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 14024) && (n < 14043))
; 14024 to 14042
.if((n .mod 3 = 2) && ((n > 14023 && n < 14043)))
	; 4675..4683 bytes
	delay_n (n-2)
	nop
.elseif(n = 14025)
	; 4678 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14026)
	; 4676 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14028)
	; 4676 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14029)
	; 4680 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14031)
	; 4680 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14032)
	; 4678 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14034)
	; 4678 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14035)
	; 4682 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14037)
	; 4682 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4684 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 14043 to 14062
.if(n = 14043)
	; 4684 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14044)
	; 4682 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 14044 && n < 14061)))
	; 4683..4689 bytes
	delay_n (n-2)
	nop
.elseif(n = 14046)
	; 4682 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14047)
	; 4686 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14049)
	; 4686 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14050)
	; 4684 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14052)
	; 4684 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14053)
	; 4688 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14055)
	; 4688 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14056)
	; 4686 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14058)
	; 4686 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14059)
	; 4690 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14061)
	; 4690 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14062)
	; 4688 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 14063) && (n < 14219))
.if((n >= 14063) && (n < 14141))
.if((n >= 14063) && (n < 14102))
.if((n >= 14063) && (n < 14082))
; 14063 to 14081
.if((n .mod 3 = 2) && ((n > 14062 && n < 14082)))
	; 4689..4697 bytes
	delay_n (n-2)
	nop
.elseif(n = 14064)
	; 4688 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14065)
	; 4692 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14067)
	; 4692 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14068)
	; 4690 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14070)
	; 4690 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14071)
	; 4694 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14073)
	; 4694 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14074)
	; 4692 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14076)
	; 4692 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14077)
	; 4696 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14079)
	; 4696 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14080)
	; 4694 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 14082 to 14101
.if(n = 14082)
	; 4694 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14083)
	; 4698 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 14083 && n < 14100)))
	; 4695..4703 bytes
	delay_n (n-2)
	nop
.elseif(n = 14085)
	; 4698 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14086)
	; 4696 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14088)
	; 4696 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14089)
	; 4700 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14091)
	; 4700 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4702 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14097)
	; 4702 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14098)
	; 4700 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14100)
	; 4700 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14101)
	; 4704 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.else
.if((n >= 14102) && (n < 14121))
; 14102 to 14120
.if((n .mod 3 = 2) && ((n > 14101 && n < 14121)))
	; 4701..4709 bytes
	delay_n (n-2)
	nop
.elseif(n = 14103)
	; 4704 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14104)
	; 4702 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14106)
	; 4702 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14107)
	; 4706 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14109)
	; 4706 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14110)
	; 4704 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14112)
	; 4704 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14113)
	; 4708 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14115)
	; 4708 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14116)
	; 4706 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14118)
	; 4706 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14119)
	; 4710 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 14121 to 14140
.if(n = 14121)
	; 4710 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14122)
	; 4708 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 14122 && n < 14139)))
	; 4709..4715 bytes
	delay_n (n-2)
	nop
.elseif(n = 14124)
	; 4708 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14125)
	; 4712 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14127)
	; 4712 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14128)
	; 4710 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14130)
	; 4710 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14131)
	; 4714 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14133)
	; 4714 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14134)
	; 4712 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14136)
	; 4712 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14137)
	; 4716 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14139)
	; 4716 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14140)
	; 4714 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.else
.if((n >= 14141) && (n < 14180))
.if((n >= 14141) && (n < 14160))
; 14141 to 14159
.if((n .mod 3 = 2) && ((n > 14140 && n < 14160)))
	; 4715..4723 bytes
	delay_n (n-2)
	nop
.elseif(n = 14142)
	; 4714 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14143)
	; 4718 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14145)
	; 4718 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4720 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14151)
	; 4720 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14152)
	; 4718 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14154)
	; 4718 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14155)
	; 4722 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14157)
	; 4722 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14158)
	; 4720 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 14160 to 14179
.if(n = 14160)
	; 4720 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14161)
	; 4724 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 14161 && n < 14178)))
	; 4721..4729 bytes
	delay_n (n-2)
	nop
.elseif(n = 14163)
	; 4724 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14164)
	; 4722 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14166)
	; 4722 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14167)
	; 4726 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14169)
	; 4726 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14170)
	; 4724 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14172)
	; 4724 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14173)
	; 4728 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14175)
	; 4728 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14176)
	; 4726 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14178)
	; 4726 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14179)
	; 4730 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.else
.if((n >= 14180) && (n < 14199))
; 14180 to 14198
.if((n .mod 3 = 2) && ((n > 14179 && n < 14199)))
	; 4727..4735 bytes
	delay_n (n-2)
	nop
.elseif(n = 14181)
	; 4730 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14182)
	; 4728 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14184)
	; 4728 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14185)
	; 4732 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14187)
	; 4732 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14188)
	; 4730 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14190)
	; 4730 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14191)
	; 4734 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14193)
	; 4734 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14194)
	; 4732 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14196)
	; 4732 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14197)
	; 4736 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 14199 to 14218
.if(n = 14199)
	; 4736 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 14200)
	; 4734 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 14200 && n < 14217)))
	; 4735..4741 bytes
	delay_n (n-2)
	nop
.elseif(n = 14202)
	; 4734 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14203)
	; 4738 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14205)
	; 4738 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14206)
	; 4736 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14208)
	; 4736 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14209)
	; 4740 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14211)
	; 4740 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14212)
	; 4738 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14214)
	; 4738 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14215)
	; 4742 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14217)
	; 4742 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14218)
	; 4740 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
.if((n >= 14219) && (n < 14258))
.if((n >= 14219) && (n < 14238))
; 14219 to 14237
.if((n .mod 3 = 2) && ((n > 14218 && n < 14238)))
	; 4741..4749 bytes
	delay_n (n-2)
	nop
.elseif(n = 14220)
	; 4740 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14221)
	; 4744 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14223)
	; 4744 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14224)
	; 4742 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14226)
	; 4742 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14227)
	; 4746 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14229)
	; 4746 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14230)
	; 4744 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14232)
	; 4744 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14233)
	; 4748 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14235)
	; 4748 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14236)
	; 4746 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 14238 to 14257
.if(n = 14238)
	; 4746 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14239)
	; 4750 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 14239 && n < 14256)))
	; 4747..4755 bytes
	delay_n (n-2)
	nop
.elseif(n = 14241)
	; 4750 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14242)
	; 4748 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14244)
	; 4748 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14245)
	; 4752 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14247)
	; 4752 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14248)
	; 4750 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14250)
	; 4750 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14251)
	; 4754 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14253)
	; 4754 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4756 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 14258) && (n < 14277))
; 14258 to 14276
.if((n .mod 3 = 2) && ((n > 14257 && n < 14277)))
	; 4753..4761 bytes
	delay_n (n-2)
	nop
.elseif(n = 14259)
	; 4756 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14260)
	; 4754 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14262)
	; 4754 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14263)
	; 4758 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14265)
	; 4758 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14266)
	; 4756 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14268)
	; 4756 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14269)
	; 4760 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14271)
	; 4760 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14272)
	; 4758 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14274)
	; 4758 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14275)
	; 4762 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 14277 to 14296
.if(n = 14277)
	; 4762 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14278)
	; 4760 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 14278 && n < 14295)))
	; 4761..4767 bytes
	delay_n (n-2)
	nop
.elseif(n = 14280)
	; 4760 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14281)
	; 4764 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14283)
	; 4764 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14284)
	; 4762 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14286)
	; 4762 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14287)
	; 4766 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14289)
	; 4766 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14290)
	; 4764 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14292)
	; 4764 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14293)
	; 4768 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14295)
	; 4768 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14296)
	; 4766 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.endif
.else
.if((n >= 14297) && (n < 14336))
.if((n >= 14297) && (n < 14316))
; 14297 to 14315
.if((n .mod 3 = 2) && ((n > 14296 && n < 14316)))
	; 4767..4775 bytes
	delay_n (n-2)
	nop
.elseif(n = 14298)
	; 4766 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14299)
	; 4770 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14301)
	; 4770 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14302)
	; 4768 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14304)
	; 4768 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14305)
	; 4772 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14307)
	; 4772 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4774 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14313)
	; 4774 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14314)
	; 4772 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 14316 to 14335
.if(n = 14316)
	; 4772 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14317)
	; 4776 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 14317 && n < 14334)))
	; 4773..4781 bytes
	delay_n (n-2)
	nop
.elseif(n = 14319)
	; 4776 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14320)
	; 4774 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14322)
	; 4774 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14323)
	; 4778 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14325)
	; 4778 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14326)
	; 4776 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14328)
	; 4776 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14329)
	; 4780 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14331)
	; 4780 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14332)
	; 4778 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14334)
	; 4778 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14335)
	; 4782 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 14336) && (n < 14356))
; 14336 to 14355
.if((n .mod 3 = 2) && ((n > 14335 && n < 14355)))
	; 4779..4787 bytes
	delay_n (n-2)
	nop
.elseif(n = 14337)
	; 4782 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14338)
	; 4780 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14340)
	; 4780 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14341)
	; 4784 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14343)
	; 4784 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14344)
	; 4782 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14346)
	; 4782 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14347)
	; 4786 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14349)
	; 4786 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14350)
	; 4784 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14352)
	; 4784 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14353)
	; 4788 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14355)
	; 4788 bytes
	delay_n 45
	delay_n (n-45)
.endif
.else
; 14356 to 14375
.if(n = 14356)
	; 4786 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 14356 && n < 14376)))
	; 4787..4795 bytes
	delay_n (n-2)
	nop
.elseif(n = 14358)
	; 4786 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14359)
	; 4790 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14361)
	; 4790 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4792 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14367)
	; 4792 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14368)
	; 4790 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14370)
	; 4790 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14371)
	; 4794 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14373)
	; 4794 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14374)
	; 4792 bytes
	delay_n 10
	delay_n (n-10)
.endif
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
.if((n >= 14376) && (n < 14395))
; 14376 to 14394
.if(n = 14376)
	; 4792 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14377)
	; 4796 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 14377 && n < 14394)))
	; 4793..4801 bytes
	delay_n (n-2)
	nop
.elseif(n = 14379)
	; 4796 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14380)
	; 4794 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14382)
	; 4794 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14383)
	; 4798 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14385)
	; 4798 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14386)
	; 4796 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14388)
	; 4796 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14389)
	; 4800 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14391)
	; 4800 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14392)
	; 4798 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14394)
	; 4798 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 14395 to 14414
.if(n = 14395)
	; 4802 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 14395 && n < 14415)))
	; 4799..4807 bytes
	delay_n (n-2)
	nop
.elseif(n = 14397)
	; 4802 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14398)
	; 4800 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14400)
	; 4800 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14401)
	; 4804 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14403)
	; 4804 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14404)
	; 4802 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14406)
	; 4802 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14407)
	; 4806 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14409)
	; 4806 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14410)
	; 4804 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14412)
	; 4804 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14413)
	; 4808 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 14415) && (n < 14434))
; 14415 to 14433
.if(n = 14415)
	; 4808 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 14416)
	; 4806 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 14416 && n < 14433)))
	; 4807..4813 bytes
	delay_n (n-2)
	nop
.elseif(n = 14418)
	; 4806 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14419)
	; 4810 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14421)
	; 4810 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14422)
	; 4808 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14424)
	; 4808 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14425)
	; 4812 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14427)
	; 4812 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14428)
	; 4810 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14430)
	; 4810 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14431)
	; 4814 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14433)
	; 4814 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 14434 to 14453
.if(n = 14434)
	; 4812 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 14434 && n < 14454)))
	; 4813..4821 bytes
	delay_n (n-2)
	nop
.elseif(n = 14436)
	; 4812 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14437)
	; 4816 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14439)
	; 4816 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14440)
	; 4814 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14442)
	; 4814 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14443)
	; 4818 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14445)
	; 4818 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14446)
	; 4816 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14448)
	; 4816 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14449)
	; 4820 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14451)
	; 4820 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14452)
	; 4818 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.endif
.else
.if((n >= 14454) && (n < 14493))
.if((n >= 14454) && (n < 14473))
; 14454 to 14472
.if(n = 14454)
	; 4818 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14455)
	; 4822 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 14455 && n < 14472)))
	; 4819..4827 bytes
	delay_n (n-2)
	nop
.elseif(n = 14457)
	; 4822 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14458)
	; 4820 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14460)
	; 4820 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14461)
	; 4824 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14463)
	; 4824 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14464)
	; 4822 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14466)
	; 4822 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14467)
	; 4826 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14469)
	; 4826 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 14470)
	; 4824 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14472)
	; 4824 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 14473 to 14492
.if(n = 14473)
	; 4828 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 14473 && n < 14493)))
	; 4825..4833 bytes
	delay_n (n-2)
	nop
.elseif(n = 14475)
	; 4828 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14476)
	; 4826 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14478)
	; 4826 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14479)
	; 4830 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14481)
	; 4830 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14482)
	; 4828 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14484)
	; 4828 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14485)
	; 4832 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14487)
	; 4832 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14488)
	; 4830 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14490)
	; 4830 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14491)
	; 4834 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.else
.if((n >= 14493) && (n < 14512))
; 14493 to 14511
.if(n = 14493)
	; 4834 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14494)
	; 4832 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 14494 && n < 14511)))
	; 4833..4839 bytes
	delay_n (n-2)
	nop
.elseif(n = 14496)
	; 4832 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14497)
	; 4836 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14499)
	; 4836 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14500)
	; 4834 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14502)
	; 4834 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14503)
	; 4838 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14505)
	; 4838 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14506)
	; 4836 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14508)
	; 4836 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14509)
	; 4840 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14511)
	; 4840 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 14512 to 14531
.if(n = 14512)
	; 4838 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 14512 && n < 14532)))
	; 4839..4847 bytes
	delay_n (n-2)
	nop
.elseif(n = 14514)
	; 4838 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14515)
	; 4842 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14517)
	; 4842 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14518)
	; 4840 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14520)
	; 4840 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14521)
	; 4844 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14523)
	; 4844 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4846 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14529)
	; 4846 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14530)
	; 4844 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
.if((n >= 14532) && (n < 14551))
; 14532 to 14550
.if(n = 14532)
	; 4844 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14533)
	; 4848 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 14533 && n < 14550)))
	; 4845..4853 bytes
	delay_n (n-2)
	nop
.elseif(n = 14535)
	; 4848 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14536)
	; 4846 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14538)
	; 4846 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14539)
	; 4850 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14541)
	; 4850 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14542)
	; 4848 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14544)
	; 4848 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14545)
	; 4852 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14547)
	; 4852 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14548)
	; 4850 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14550)
	; 4850 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 14551 to 14570
.if(n = 14551)
	; 4854 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 14551 && n < 14571)))
	; 4851..4859 bytes
	delay_n (n-2)
	nop
.elseif(n = 14553)
	; 4854 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14554)
	; 4852 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14556)
	; 4852 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14557)
	; 4856 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14559)
	; 4856 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14560)
	; 4854 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14562)
	; 4854 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14563)
	; 4858 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14565)
	; 4858 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14566)
	; 4856 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14568)
	; 4856 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14569)
	; 4860 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 14571) && (n < 14590))
; 14571 to 14589
.if(n = 14571)
	; 4860 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14572)
	; 4858 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 14572 && n < 14589)))
	; 4859..4865 bytes
	delay_n (n-2)
	nop
.elseif(n = 14574)
	; 4858 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14575)
	; 4862 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14577)
	; 4862 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4864 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14583)
	; 4864 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14584)
	; 4862 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14586)
	; 4862 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14587)
	; 4866 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14589)
	; 4866 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 14590 to 14609
.if(n = 14590)
	; 4864 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 14590 && n < 14610)))
	; 4865..4873 bytes
	delay_n (n-2)
	nop
.elseif(n = 14592)
	; 4864 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14593)
	; 4868 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14595)
	; 4868 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14596)
	; 4866 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14598)
	; 4866 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14599)
	; 4870 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14601)
	; 4870 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14602)
	; 4868 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14604)
	; 4868 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14605)
	; 4872 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14607)
	; 4872 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14608)
	; 4870 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.else
.if((n >= 14610) && (n < 14649))
.if((n >= 14610) && (n < 14629))
; 14610 to 14628
.if(n = 14610)
	; 4870 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14611)
	; 4874 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 14611 && n < 14628)))
	; 4871..4879 bytes
	delay_n (n-2)
	nop
.elseif(n = 14613)
	; 4874 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14614)
	; 4872 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14616)
	; 4872 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14617)
	; 4876 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14619)
	; 4876 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14620)
	; 4874 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14622)
	; 4874 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14623)
	; 4878 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14625)
	; 4878 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14626)
	; 4876 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14628)
	; 4876 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 14629 to 14648
.if(n = 14629)
	; 4880 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 14629 && n < 14649)))
	; 4877..4885 bytes
	delay_n (n-2)
	nop
.elseif(n = 14631)
	; 4880 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4882 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14637)
	; 4882 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14638)
	; 4880 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14640)
	; 4880 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14641)
	; 4884 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14643)
	; 4884 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14644)
	; 4882 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14646)
	; 4882 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14647)
	; 4886 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 14649) && (n < 14668))
; 14649 to 14667
.if(n = 14649)
	; 4886 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14650)
	; 4884 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 14650 && n < 14667)))
	; 4885..4891 bytes
	delay_n (n-2)
	nop
.elseif(n = 14652)
	; 4884 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14653)
	; 4888 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14655)
	; 4888 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14656)
	; 4886 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14658)
	; 4886 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14659)
	; 4890 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14661)
	; 4890 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14662)
	; 4888 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14664)
	; 4888 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14665)
	; 4892 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14667)
	; 4892 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 14668 to 14687
.if(n = 14668)
	; 4890 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 14668 && n < 14688)))
	; 4891..4899 bytes
	delay_n (n-2)
	nop
.elseif(n = 14670)
	; 4890 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14671)
	; 4894 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14673)
	; 4894 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14674)
	; 4892 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14676)
	; 4892 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14677)
	; 4896 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14679)
	; 4896 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14680)
	; 4894 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14682)
	; 4894 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14683)
	; 4898 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14685)
	; 4898 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 14686)
	; 4896 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
.if((n >= 14688) && (n < 14766))
.if((n >= 14688) && (n < 14727))
.if((n >= 14688) && (n < 14707))
; 14688 to 14706
.if(n = 14688)
	; 4896 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14689)
	; 4900 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 14689 && n < 14706)))
	; 4897..4905 bytes
	delay_n (n-2)
	nop
.elseif(n = 14691)
	; 4900 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14692)
	; 4898 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14694)
	; 4898 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14695)
	; 4902 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14697)
	; 4902 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14698)
	; 4900 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14700)
	; 4900 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14701)
	; 4904 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14703)
	; 4904 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14704)
	; 4902 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14706)
	; 4902 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 14707 to 14726
.if(n = 14707)
	; 4906 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 14707 && n < 14727)))
	; 4903..4911 bytes
	delay_n (n-2)
	nop
.elseif(n = 14709)
	; 4906 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14710)
	; 4904 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14712)
	; 4904 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14713)
	; 4908 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14715)
	; 4908 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14716)
	; 4906 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14718)
	; 4906 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14719)
	; 4910 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14721)
	; 4910 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14722)
	; 4908 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14724)
	; 4908 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14725)
	; 4912 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 14727) && (n < 14746))
; 14727 to 14745
.if(n = 14727)
	; 4912 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14728)
	; 4910 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 14728 && n < 14745)))
	; 4911..4917 bytes
	delay_n (n-2)
	nop
.elseif(n = 14730)
	; 4910 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14731)
	; 4914 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14733)
	; 4914 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14734)
	; 4912 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14736)
	; 4912 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14737)
	; 4916 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14739)
	; 4916 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4918 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14745)
	; 4918 bytes
	delay_n 57
	delay_n (n-57)
.endif
.else
; 14746 to 14765
.if(n = 14746)
	; 4916 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 14746 && n < 14766)))
	; 4917..4925 bytes
	delay_n (n-2)
	nop
.elseif(n = 14748)
	; 4916 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14749)
	; 4920 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14751)
	; 4920 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14752)
	; 4918 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14754)
	; 4918 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14755)
	; 4922 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14757)
	; 4922 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14758)
	; 4920 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14760)
	; 4920 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14761)
	; 4924 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14763)
	; 4924 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14764)
	; 4922 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.else
.if((n >= 14766) && (n < 14805))
.if((n >= 14766) && (n < 14785))
; 14766 to 14784
.if(n = 14766)
	; 4922 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14767)
	; 4926 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 14767 && n < 14784)))
	; 4923..4931 bytes
	delay_n (n-2)
	nop
.elseif(n = 14769)
	; 4926 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14770)
	; 4924 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14772)
	; 4924 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14773)
	; 4928 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14775)
	; 4928 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14776)
	; 4926 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14778)
	; 4926 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14779)
	; 4930 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14781)
	; 4930 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14782)
	; 4928 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14784)
	; 4928 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 14785 to 14804
.if(n = 14785)
	; 4932 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 14785 && n < 14805)))
	; 4929..4937 bytes
	delay_n (n-2)
	nop
.elseif(n = 14787)
	; 4932 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14788)
	; 4930 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14790)
	; 4930 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14791)
	; 4934 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14793)
	; 4934 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4936 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14799)
	; 4936 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14800)
	; 4934 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14802)
	; 4934 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14803)
	; 4938 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.else
.if((n >= 14805) && (n < 14824))
; 14805 to 14823
.if(n = 14805)
	; 4938 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14806)
	; 4936 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 14806 && n < 14823)))
	; 4937..4943 bytes
	delay_n (n-2)
	nop
.elseif(n = 14808)
	; 4936 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14809)
	; 4940 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14811)
	; 4940 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14812)
	; 4938 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14814)
	; 4938 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14815)
	; 4942 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14817)
	; 4942 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14818)
	; 4940 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14820)
	; 4940 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14821)
	; 4944 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14823)
	; 4944 bytes
	delay_n 81
	delay_n (n-81)
.endif
.else
; 14824 to 14843
.if(n = 14824)
	; 4942 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 14824 && n < 14844)))
	; 4943..4951 bytes
	delay_n (n-2)
	nop
.elseif(n = 14826)
	; 4942 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14827)
	; 4946 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14829)
	; 4946 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14830)
	; 4944 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14832)
	; 4944 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14833)
	; 4948 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14835)
	; 4948 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14836)
	; 4946 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14838)
	; 4946 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14839)
	; 4950 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14841)
	; 4950 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14842)
	; 4948 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.endif
.else
.if((n >= 14844) && (n < 14922))
.if((n >= 14844) && (n < 14883))
.if((n >= 14844) && (n < 14863))
; 14844 to 14862
.if(n = 14844)
	; 4948 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14845)
	; 4952 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 14845 && n < 14862)))
	; 4949..4957 bytes
	delay_n (n-2)
	nop
.elseif(n = 14847)
	; 4952 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4954 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14853)
	; 4954 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14854)
	; 4952 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14856)
	; 4952 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14857)
	; 4956 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14859)
	; 4956 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14860)
	; 4954 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14862)
	; 4954 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 14863 to 14882
.if(n = 14863)
	; 4958 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 14863 && n < 14883)))
	; 4955..4963 bytes
	delay_n (n-2)
	nop
.elseif(n = 14865)
	; 4958 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14866)
	; 4956 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14868)
	; 4956 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14869)
	; 4960 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14871)
	; 4960 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14872)
	; 4958 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14874)
	; 4958 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14875)
	; 4962 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14877)
	; 4962 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14878)
	; 4960 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14880)
	; 4960 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14881)
	; 4964 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.else
.if((n >= 14883) && (n < 14902))
; 14883 to 14901
.if(n = 14883)
	; 4964 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14884)
	; 4962 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 14884 && n < 14901)))
	; 4963..4969 bytes
	delay_n (n-2)
	nop
.elseif(n = 14886)
	; 4962 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14887)
	; 4966 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14889)
	; 4966 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14890)
	; 4964 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14892)
	; 4964 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14893)
	; 4968 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14895)
	; 4968 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14896)
	; 4966 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14898)
	; 4966 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14899)
	; 4970 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14901)
	; 4970 bytes
	delay_n 105
	delay_n (n-105)
.endif
.else
; 14902 to 14921
.if(n = 14902)
	; 4968 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 14902 && n < 14922)))
	; 4969..4977 bytes
	delay_n (n-2)
	nop
.elseif(n = 14904)
	; 4968 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 14905)
	; 4972 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14907)
	; 4972 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14908)
	; 4970 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 14910)
	; 4970 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14911)
	; 4974 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14913)
	; 4974 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14914)
	; 4972 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14916)
	; 4972 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14917)
	; 4976 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14919)
	; 4976 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14920)
	; 4974 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.endif
.else
.if((n >= 14922) && (n < 14961))
.if((n >= 14922) && (n < 14941))
; 14922 to 14940
.if(n = 14922)
	; 4974 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14923)
	; 4978 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 14923 && n < 14940)))
	; 4975..4983 bytes
	delay_n (n-2)
	nop
.elseif(n = 14925)
	; 4978 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14926)
	; 4976 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 14928)
	; 4976 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14929)
	; 4980 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14931)
	; 4980 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14932)
	; 4978 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14934)
	; 4978 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14935)
	; 4982 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14937)
	; 4982 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14938)
	; 4980 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14940)
	; 4980 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 14941 to 14960
.if(n = 14941)
	; 4984 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 14941 && n < 14961)))
	; 4981..4989 bytes
	delay_n (n-2)
	nop
.elseif(n = 14943)
	; 4984 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 14944)
	; 4982 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 14946)
	; 4982 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14947)
	; 4986 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14949)
	; 4986 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 14950)
	; 4984 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14952)
	; 4984 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 14953)
	; 4988 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 14955)
	; 4988 bytes
	delay_n 105
	delay_n (n-105)
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
	; 4990 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 14961) && (n < 14981))
; 14961 to 14980
.if(n = 14961)
	; 4990 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 14962)
	; 4988 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 14962 && n < 14979)))
	; 4989..4995 bytes
	delay_n (n-2)
	nop
.elseif(n = 14964)
	; 4988 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 14965)
	; 4992 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 14967)
	; 4992 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 14968)
	; 4990 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14970)
	; 4990 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14971)
	; 4994 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 14973)
	; 4994 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14974)
	; 4992 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 14976)
	; 4992 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 14977)
	; 4996 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 14979)
	; 4996 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 14980)
	; 4994 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 14981 to 15000
.if((n .mod 3 = 2) && ((n > 14980 && n < 15000)))
	; 4995..5003 bytes
	delay_n (n-2)
	nop
.elseif(n = 14982)
	; 4994 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 14983)
	; 4998 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 14985)
	; 4998 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14986)
	; 4996 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 14988)
	; 4996 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 14989)
	; 5000 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 14991)
	; 5000 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14992)
	; 4998 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14994)
	; 4998 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14995)
	; 5002 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14997)
	; 5002 bytes
	delay_n 93
	delay_n (n-93)
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
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
.if((n >= 15001) && (n < 15626))
.if((n >= 15001) && (n < 15313))
.if((n >= 15001) && (n < 15157))
.if((n >= 15001) && (n < 15079))
.if((n >= 15001) && (n < 15040))
.if((n >= 15001) && (n < 15020))
; 15001 to 15019
.if(n = 15001)
	; 5004 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 15001 && n < 15018)))
	; 5001..5009 bytes
	delay_n (n-2)
	nop
.elseif(n = 15003)
	; 5004 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15004)
	; 5002 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15006)
	; 5002 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15007)
	; 5006 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15009)
	; 5006 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5008 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15015)
	; 5008 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15016)
	; 5006 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15018)
	; 5006 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15019)
	; 5010 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 15020 to 15039
.if((n .mod 3 = 2) && ((n > 15019 && n < 15039)))
	; 5007..5015 bytes
	delay_n (n-2)
	nop
.elseif(n = 15021)
	; 5010 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15022)
	; 5008 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15024)
	; 5008 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15025)
	; 5012 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15027)
	; 5012 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15028)
	; 5010 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15030)
	; 5010 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15031)
	; 5014 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15033)
	; 5014 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15034)
	; 5012 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15036)
	; 5012 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15037)
	; 5016 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15039)
	; 5016 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.else
.if((n >= 15040) && (n < 15059))
; 15040 to 15058
.if(n = 15040)
	; 5014 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 15040 && n < 15057)))
	; 5015..5021 bytes
	delay_n (n-2)
	nop
.elseif(n = 15042)
	; 5014 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15043)
	; 5018 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15045)
	; 5018 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15046)
	; 5016 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15048)
	; 5016 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15049)
	; 5020 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15051)
	; 5020 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15052)
	; 5018 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15054)
	; 5018 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15055)
	; 5022 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15057)
	; 5022 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15058)
	; 5020 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 15059 to 15078
.if((n .mod 3 = 2) && ((n > 15058 && n < 15078)))
	; 5021..5029 bytes
	delay_n (n-2)
	nop
.elseif(n = 15060)
	; 5020 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15061)
	; 5024 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15063)
	; 5024 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5026 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15069)
	; 5026 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15070)
	; 5024 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15072)
	; 5024 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15073)
	; 5028 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15075)
	; 5028 bytes
	delay_n 117
	delay_n (n-117)
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
.endif
.else
.if((n >= 15079) && (n < 15118))
.if((n >= 15079) && (n < 15098))
; 15079 to 15097
.if(n = 15079)
	; 5030 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 15079 && n < 15096)))
	; 5027..5035 bytes
	delay_n (n-2)
	nop
.elseif(n = 15081)
	; 5030 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15082)
	; 5028 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15084)
	; 5028 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15085)
	; 5032 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15087)
	; 5032 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15088)
	; 5030 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15090)
	; 5030 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15091)
	; 5034 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15093)
	; 5034 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15094)
	; 5032 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15096)
	; 5032 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15097)
	; 5036 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 15098 to 15117
.if((n .mod 3 = 2) && ((n > 15097 && n < 15117)))
	; 5033..5041 bytes
	delay_n (n-2)
	nop
.elseif(n = 15099)
	; 5036 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15100)
	; 5034 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15102)
	; 5034 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15103)
	; 5038 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15105)
	; 5038 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15106)
	; 5036 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15108)
	; 5036 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15109)
	; 5040 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15111)
	; 5040 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15112)
	; 5038 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15114)
	; 5038 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15115)
	; 5042 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15117)
	; 5042 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.else
.if((n >= 15118) && (n < 15137))
; 15118 to 15136
.if(n = 15118)
	; 5040 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 15118 && n < 15135)))
	; 5041..5047 bytes
	delay_n (n-2)
	nop
.elseif(n = 15120)
	; 5040 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15121)
	; 5044 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15123)
	; 5044 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15124)
	; 5042 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15126)
	; 5042 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15127)
	; 5046 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15129)
	; 5046 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15130)
	; 5044 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15132)
	; 5044 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15133)
	; 5048 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15135)
	; 5048 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15136)
	; 5046 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 15137 to 15156
.if((n .mod 3 = 2) && ((n > 15136 && n < 15156)))
	; 5047..5055 bytes
	delay_n (n-2)
	nop
.elseif(n = 15138)
	; 5046 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15139)
	; 5050 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15141)
	; 5050 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15142)
	; 5048 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15144)
	; 5048 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15145)
	; 5052 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15147)
	; 5052 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15148)
	; 5050 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15150)
	; 5050 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15151)
	; 5054 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15153)
	; 5054 bytes
	delay_n 141
	delay_n (n-141)
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
.endif
.else
.if((n >= 15157) && (n < 15235))
.if((n >= 15157) && (n < 15196))
.if((n >= 15157) && (n < 15176))
; 15157 to 15175
.if(n = 15157)
	; 5056 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 15157 && n < 15174)))
	; 5053..5061 bytes
	delay_n (n-2)
	nop
.elseif(n = 15159)
	; 5056 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15160)
	; 5054 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15162)
	; 5054 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15163)
	; 5058 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15165)
	; 5058 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15166)
	; 5056 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15168)
	; 5056 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15169)
	; 5060 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15171)
	; 5060 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5062 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 15176 to 15195
.if((n .mod 3 = 2) && ((n > 15175 && n < 15195)))
	; 5059..5067 bytes
	delay_n (n-2)
	nop
.elseif(n = 15177)
	; 5062 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15178)
	; 5060 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15180)
	; 5060 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15181)
	; 5064 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15183)
	; 5064 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15184)
	; 5062 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15186)
	; 5062 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15187)
	; 5066 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15189)
	; 5066 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15190)
	; 5064 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15192)
	; 5064 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15193)
	; 5068 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15195)
	; 5068 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.else
.if((n >= 15196) && (n < 15215))
; 15196 to 15214
.if(n = 15196)
	; 5066 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 15196 && n < 15213)))
	; 5067..5073 bytes
	delay_n (n-2)
	nop
.elseif(n = 15198)
	; 5066 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15199)
	; 5070 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15201)
	; 5070 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15202)
	; 5068 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15204)
	; 5068 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15205)
	; 5072 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15207)
	; 5072 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15208)
	; 5070 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15210)
	; 5070 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15211)
	; 5074 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15213)
	; 5074 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15214)
	; 5072 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 15215 to 15234
.if((n .mod 3 = 2) && ((n > 15214 && n < 15234)))
	; 5073..5081 bytes
	delay_n (n-2)
	nop
.elseif(n = 15216)
	; 5072 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15217)
	; 5076 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15219)
	; 5076 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15220)
	; 5074 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15222)
	; 5074 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15223)
	; 5078 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15225)
	; 5078 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5080 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15231)
	; 5080 bytes
	delay_n 57
	delay_n (n-57)
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
.endif
.else
.if((n >= 15235) && (n < 15274))
.if((n >= 15235) && (n < 15254))
; 15235 to 15253
.if(n = 15235)
	; 5082 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 15235 && n < 15252)))
	; 5079..5087 bytes
	delay_n (n-2)
	nop
.elseif(n = 15237)
	; 5082 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15238)
	; 5080 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15240)
	; 5080 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15241)
	; 5084 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15243)
	; 5084 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15244)
	; 5082 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15246)
	; 5082 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15247)
	; 5086 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15249)
	; 5086 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15250)
	; 5084 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15252)
	; 5084 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15253)
	; 5088 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 15254 to 15273
.if((n .mod 3 = 2) && ((n > 15253 && n < 15273)))
	; 5085..5093 bytes
	delay_n (n-2)
	nop
.elseif(n = 15255)
	; 5088 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15256)
	; 5086 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15258)
	; 5086 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15259)
	; 5090 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15261)
	; 5090 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15262)
	; 5088 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15264)
	; 5088 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15265)
	; 5092 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15267)
	; 5092 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15268)
	; 5090 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15270)
	; 5090 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15271)
	; 5094 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15273)
	; 5094 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.else
.if((n >= 15274) && (n < 15293))
; 15274 to 15292
.if(n = 15274)
	; 5092 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 15274 && n < 15291)))
	; 5093..5099 bytes
	delay_n (n-2)
	nop
.elseif(n = 15276)
	; 5092 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15277)
	; 5096 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15279)
	; 5096 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5098 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15285)
	; 5098 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15286)
	; 5096 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15288)
	; 5096 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15289)
	; 5100 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15291)
	; 5100 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15292)
	; 5098 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 15293 to 15312
.if((n .mod 3 = 2) && ((n > 15292 && n < 15312)))
	; 5099..5107 bytes
	delay_n (n-2)
	nop
.elseif(n = 15294)
	; 5098 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15295)
	; 5102 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15297)
	; 5102 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15298)
	; 5100 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15300)
	; 5100 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15301)
	; 5104 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15303)
	; 5104 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15304)
	; 5102 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15306)
	; 5102 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15307)
	; 5106 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15309)
	; 5106 bytes
	delay_n 81
	delay_n (n-81)
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
.endif
.else
.if((n >= 15313) && (n < 15469))
.if((n >= 15313) && (n < 15391))
.if((n >= 15313) && (n < 15352))
.if((n >= 15313) && (n < 15332))
; 15313 to 15331
.if(n = 15313)
	; 5108 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 15313 && n < 15330)))
	; 5105..5113 bytes
	delay_n (n-2)
	nop
.elseif(n = 15315)
	; 5108 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15316)
	; 5106 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15318)
	; 5106 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15319)
	; 5110 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15321)
	; 5110 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15322)
	; 5108 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15324)
	; 5108 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15325)
	; 5112 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15327)
	; 5112 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15328)
	; 5110 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15330)
	; 5110 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15331)
	; 5114 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 15332 to 15351
.if((n .mod 3 = 2) && ((n > 15331 && n < 15351)))
	; 5111..5119 bytes
	delay_n (n-2)
	nop
.elseif(n = 15333)
	; 5114 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5116 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15339)
	; 5116 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15340)
	; 5114 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15342)
	; 5114 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15343)
	; 5118 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15345)
	; 5118 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15346)
	; 5116 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15348)
	; 5116 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15349)
	; 5120 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15351)
	; 5120 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.else
.if((n >= 15352) && (n < 15371))
; 15352 to 15370
.if(n = 15352)
	; 5118 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 15352 && n < 15369)))
	; 5119..5125 bytes
	delay_n (n-2)
	nop
.elseif(n = 15354)
	; 5118 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15355)
	; 5122 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15357)
	; 5122 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15358)
	; 5120 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15360)
	; 5120 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15361)
	; 5124 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15363)
	; 5124 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15364)
	; 5122 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15366)
	; 5122 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15367)
	; 5126 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15369)
	; 5126 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15370)
	; 5124 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 15371 to 15390
.if((n .mod 3 = 2) && ((n > 15370 && n < 15390)))
	; 5125..5133 bytes
	delay_n (n-2)
	nop
.elseif(n = 15372)
	; 5124 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15373)
	; 5128 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15375)
	; 5128 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15376)
	; 5126 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15378)
	; 5126 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15379)
	; 5130 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15381)
	; 5130 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15382)
	; 5128 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15384)
	; 5128 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15385)
	; 5132 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15387)
	; 5132 bytes
	delay_n 105
	delay_n (n-105)
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
.endif
.else
.if((n >= 15391) && (n < 15430))
.if((n >= 15391) && (n < 15410))
; 15391 to 15409
.if(n = 15391)
	; 5134 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 15391 && n < 15408)))
	; 5131..5139 bytes
	delay_n (n-2)
	nop
.elseif(n = 15393)
	; 5134 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15394)
	; 5132 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15396)
	; 5132 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15397)
	; 5136 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15399)
	; 5136 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15400)
	; 5134 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15402)
	; 5134 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15403)
	; 5138 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15405)
	; 5138 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15406)
	; 5136 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15408)
	; 5136 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15409)
	; 5140 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 15410 to 15429
.if((n .mod 3 = 2) && ((n > 15409 && n < 15429)))
	; 5137..5145 bytes
	delay_n (n-2)
	nop
.elseif(n = 15411)
	; 5140 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15412)
	; 5138 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15414)
	; 5138 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15415)
	; 5142 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15417)
	; 5142 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15418)
	; 5140 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15420)
	; 5140 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15421)
	; 5144 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15423)
	; 5144 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15424)
	; 5142 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15426)
	; 5142 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15427)
	; 5146 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15429)
	; 5146 bytes
	delay_n 93
	delay_n (n-93)
.endif
.endif
.else
.if((n >= 15430) && (n < 15449))
; 15430 to 15448
.if(n = 15430)
	; 5144 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 15430 && n < 15447)))
	; 5145..5151 bytes
	delay_n (n-2)
	nop
.elseif(n = 15432)
	; 5144 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15433)
	; 5148 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15435)
	; 5148 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15436)
	; 5146 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15438)
	; 5146 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15439)
	; 5150 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15441)
	; 5150 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5152 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15447)
	; 5152 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15448)
	; 5150 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 15449 to 15468
.if((n .mod 3 = 2) && ((n > 15448 && n < 15468)))
	; 5151..5159 bytes
	delay_n (n-2)
	nop
.elseif(n = 15450)
	; 5150 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15451)
	; 5154 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15453)
	; 5154 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15454)
	; 5152 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15456)
	; 5152 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15457)
	; 5156 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15459)
	; 5156 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15460)
	; 5154 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15462)
	; 5154 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15463)
	; 5158 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15465)
	; 5158 bytes
	delay_n 129
	delay_n (n-129)
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
.endif
.else
.if((n >= 15469) && (n < 15547))
.if((n >= 15469) && (n < 15508))
.if((n >= 15469) && (n < 15488))
; 15469 to 15487
.if(n = 15469)
	; 5160 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 15469 && n < 15486)))
	; 5157..5165 bytes
	delay_n (n-2)
	nop
.elseif(n = 15471)
	; 5160 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15472)
	; 5158 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15474)
	; 5158 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15475)
	; 5162 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15477)
	; 5162 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15478)
	; 5160 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15480)
	; 5160 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15481)
	; 5164 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15483)
	; 5164 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15484)
	; 5162 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15486)
	; 5162 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15487)
	; 5166 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 15488 to 15507
.if((n .mod 3 = 2) && ((n > 15487 && n < 15507)))
	; 5163..5171 bytes
	delay_n (n-2)
	nop
.elseif(n = 15489)
	; 5166 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15490)
	; 5164 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15492)
	; 5164 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15493)
	; 5168 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15495)
	; 5168 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5170 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15501)
	; 5170 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15502)
	; 5168 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15504)
	; 5168 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15505)
	; 5172 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15507)
	; 5172 bytes
	delay_n 117
	delay_n (n-117)
.endif
.endif
.else
.if((n >= 15508) && (n < 15527))
; 15508 to 15526
.if(n = 15508)
	; 5170 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 15508 && n < 15525)))
	; 5171..5177 bytes
	delay_n (n-2)
	nop
.elseif(n = 15510)
	; 5170 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15511)
	; 5174 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15513)
	; 5174 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15514)
	; 5172 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15516)
	; 5172 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15517)
	; 5176 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15519)
	; 5176 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15520)
	; 5174 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15522)
	; 5174 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15523)
	; 5178 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15525)
	; 5178 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15526)
	; 5176 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 15527 to 15546
.if((n .mod 3 = 2) && ((n > 15526 && n < 15546)))
	; 5177..5185 bytes
	delay_n (n-2)
	nop
.elseif(n = 15528)
	; 5176 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15529)
	; 5180 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15531)
	; 5180 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15532)
	; 5178 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15534)
	; 5178 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15535)
	; 5182 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15537)
	; 5182 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15538)
	; 5180 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15540)
	; 5180 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15541)
	; 5184 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15543)
	; 5184 bytes
	delay_n 45
	delay_n (n-45)
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
.endif
.else
.if((n >= 15547) && (n < 15586))
.if((n >= 15547) && (n < 15566))
; 15547 to 15565
.if(n = 15547)
	; 5186 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 15547 && n < 15564)))
	; 5183..5191 bytes
	delay_n (n-2)
	nop
.elseif(n = 15549)
	; 5186 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5188 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15555)
	; 5188 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15556)
	; 5186 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15558)
	; 5186 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15559)
	; 5190 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15561)
	; 5190 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15562)
	; 5188 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15564)
	; 5188 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15565)
	; 5192 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 15566 to 15585
.if((n .mod 3 = 2) && ((n > 15565 && n < 15585)))
	; 5189..5197 bytes
	delay_n (n-2)
	nop
.elseif(n = 15567)
	; 5192 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15568)
	; 5190 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15570)
	; 5190 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15571)
	; 5194 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15573)
	; 5194 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15574)
	; 5192 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15576)
	; 5192 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15577)
	; 5196 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15579)
	; 5196 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15580)
	; 5194 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15582)
	; 5194 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15583)
	; 5198 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15585)
	; 5198 bytes
	delay_n 141
	delay_n (n-141)
.endif
.endif
.else
.if((n >= 15586) && (n < 15606))
; 15586 to 15605
.if(n = 15586)
	; 5196 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 15586 && n < 15606)))
	; 5197..5205 bytes
	delay_n (n-2)
	nop
.elseif(n = 15588)
	; 5196 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15589)
	; 5200 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15591)
	; 5200 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15592)
	; 5198 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15594)
	; 5198 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15595)
	; 5202 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15597)
	; 5202 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15598)
	; 5200 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15600)
	; 5200 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15601)
	; 5204 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15603)
	; 5204 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 15604)
	; 5202 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 15606 to 15625
.if(n = 15606)
	; 5202 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15607)
	; 5206 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 15607 && n < 15624)))
	; 5203..5211 bytes
	delay_n (n-2)
	nop
.elseif(n = 15609)
	; 5206 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15610)
	; 5204 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15612)
	; 5204 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15613)
	; 5208 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15615)
	; 5208 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15616)
	; 5206 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15618)
	; 5206 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15619)
	; 5210 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15621)
	; 5210 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15622)
	; 5208 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15624)
	; 5208 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15625)
	; 5212 bytes
	delay_n 127
	delay_n (n-127)
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
.if((n .mod 3 = 2) && ((n > 15625 && n < 15645)))
	; 5209..5217 bytes
	delay_n (n-2)
	nop
.elseif(n = 15627)
	; 5212 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15628)
	; 5210 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15630)
	; 5210 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15631)
	; 5214 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15633)
	; 5214 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15634)
	; 5212 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15636)
	; 5212 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15637)
	; 5216 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15639)
	; 5216 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15640)
	; 5214 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15642)
	; 5214 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15643)
	; 5218 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 15645 to 15664
.if(n = 15645)
	; 5218 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15646)
	; 5216 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 15646 && n < 15663)))
	; 5217..5223 bytes
	delay_n (n-2)
	nop
.elseif(n = 15648)
	; 5216 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15649)
	; 5220 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15651)
	; 5220 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15652)
	; 5218 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15654)
	; 5218 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15655)
	; 5222 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15657)
	; 5222 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5224 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15663)
	; 5224 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15664)
	; 5222 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 15665) && (n < 15684))
; 15665 to 15683
.if((n .mod 3 = 2) && ((n > 15664 && n < 15684)))
	; 5223..5231 bytes
	delay_n (n-2)
	nop
.elseif(n = 15666)
	; 5222 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15667)
	; 5226 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15669)
	; 5226 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15670)
	; 5224 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15672)
	; 5224 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15673)
	; 5228 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15675)
	; 5228 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15676)
	; 5226 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15678)
	; 5226 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15679)
	; 5230 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15681)
	; 5230 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15682)
	; 5228 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 15684 to 15703
.if(n = 15684)
	; 5228 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15685)
	; 5232 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 15685 && n < 15702)))
	; 5229..5237 bytes
	delay_n (n-2)
	nop
.elseif(n = 15687)
	; 5232 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15688)
	; 5230 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15690)
	; 5230 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15691)
	; 5234 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15693)
	; 5234 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15694)
	; 5232 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15696)
	; 5232 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15697)
	; 5236 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15699)
	; 5236 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15700)
	; 5234 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15702)
	; 5234 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15703)
	; 5238 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.else
.if((n >= 15704) && (n < 15743))
.if((n >= 15704) && (n < 15723))
; 15704 to 15722
.if((n .mod 3 = 2) && ((n > 15703 && n < 15723)))
	; 5235..5243 bytes
	delay_n (n-2)
	nop
.elseif(n = 15705)
	; 5238 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15706)
	; 5236 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15708)
	; 5236 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15709)
	; 5240 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15711)
	; 5240 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5242 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15717)
	; 5242 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15718)
	; 5240 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15720)
	; 5240 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15721)
	; 5244 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 15723 to 15742
.if(n = 15723)
	; 5244 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15724)
	; 5242 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 15724 && n < 15741)))
	; 5243..5249 bytes
	delay_n (n-2)
	nop
.elseif(n = 15726)
	; 5242 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15727)
	; 5246 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15729)
	; 5246 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15730)
	; 5244 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15732)
	; 5244 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15733)
	; 5248 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15735)
	; 5248 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15736)
	; 5246 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15738)
	; 5246 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15739)
	; 5250 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15741)
	; 5250 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15742)
	; 5248 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 15743) && (n < 15762))
; 15743 to 15761
.if((n .mod 3 = 2) && ((n > 15742 && n < 15762)))
	; 5249..5257 bytes
	delay_n (n-2)
	nop
.elseif(n = 15744)
	; 5248 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15745)
	; 5252 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15747)
	; 5252 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15748)
	; 5250 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15750)
	; 5250 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15751)
	; 5254 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15753)
	; 5254 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15754)
	; 5252 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15756)
	; 5252 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15757)
	; 5256 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15759)
	; 5256 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15760)
	; 5254 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 15762 to 15781
.if(n = 15762)
	; 5254 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15763)
	; 5258 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 15763 && n < 15780)))
	; 5255..5263 bytes
	delay_n (n-2)
	nop
.elseif(n = 15765)
	; 5258 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5260 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15771)
	; 5260 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15772)
	; 5258 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15774)
	; 5258 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15775)
	; 5262 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15777)
	; 5262 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15778)
	; 5260 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15780)
	; 5260 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15781)
	; 5264 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.endif
.else
.if((n >= 15782) && (n < 15860))
.if((n >= 15782) && (n < 15821))
.if((n >= 15782) && (n < 15801))
; 15782 to 15800
.if((n .mod 3 = 2) && ((n > 15781 && n < 15801)))
	; 5261..5269 bytes
	delay_n (n-2)
	nop
.elseif(n = 15783)
	; 5264 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15784)
	; 5262 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15786)
	; 5262 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15787)
	; 5266 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15789)
	; 5266 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15790)
	; 5264 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15792)
	; 5264 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15793)
	; 5268 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15795)
	; 5268 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15796)
	; 5266 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15798)
	; 5266 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15799)
	; 5270 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 15801 to 15820
.if(n = 15801)
	; 5270 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15802)
	; 5268 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 15802 && n < 15819)))
	; 5269..5275 bytes
	delay_n (n-2)
	nop
.elseif(n = 15804)
	; 5268 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15805)
	; 5272 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15807)
	; 5272 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15808)
	; 5270 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15810)
	; 5270 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15811)
	; 5274 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15813)
	; 5274 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15814)
	; 5272 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15816)
	; 5272 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15817)
	; 5276 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15819)
	; 5276 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 15820)
	; 5274 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 15821) && (n < 15840))
; 15821 to 15839
.if((n .mod 3 = 2) && ((n > 15820 && n < 15840)))
	; 5275..5283 bytes
	delay_n (n-2)
	nop
.elseif(n = 15822)
	; 5274 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 15823)
	; 5278 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15825)
	; 5278 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15826)
	; 5276 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15828)
	; 5276 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15829)
	; 5280 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15831)
	; 5280 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15832)
	; 5278 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15834)
	; 5278 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15835)
	; 5282 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15837)
	; 5282 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15838)
	; 5280 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 15840 to 15859
.if(n = 15840)
	; 5280 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15841)
	; 5284 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 15841 && n < 15858)))
	; 5281..5289 bytes
	delay_n (n-2)
	nop
.elseif(n = 15843)
	; 5284 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15844)
	; 5282 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15846)
	; 5282 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15847)
	; 5286 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15849)
	; 5286 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15850)
	; 5284 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15852)
	; 5284 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15853)
	; 5288 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15855)
	; 5288 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15856)
	; 5286 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15858)
	; 5286 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15859)
	; 5290 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 15860) && (n < 15899))
.if((n >= 15860) && (n < 15879))
; 15860 to 15878
.if((n .mod 3 = 2) && ((n > 15859 && n < 15879)))
	; 5287..5295 bytes
	delay_n (n-2)
	nop
.elseif(n = 15861)
	; 5290 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15862)
	; 5288 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15864)
	; 5288 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15865)
	; 5292 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15867)
	; 5292 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15868)
	; 5290 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15870)
	; 5290 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15871)
	; 5294 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15873)
	; 5294 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5296 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 15879 to 15898
.if(n = 15879)
	; 5296 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15880)
	; 5294 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 15880 && n < 15897)))
	; 5295..5301 bytes
	delay_n (n-2)
	nop
.elseif(n = 15882)
	; 5294 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15883)
	; 5298 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15885)
	; 5298 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15886)
	; 5296 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15888)
	; 5296 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15889)
	; 5300 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15891)
	; 5300 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15892)
	; 5298 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15894)
	; 5298 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15895)
	; 5302 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15897)
	; 5302 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15898)
	; 5300 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 15899) && (n < 15918))
; 15899 to 15917
.if((n .mod 3 = 2) && ((n > 15898 && n < 15918)))
	; 5301..5309 bytes
	delay_n (n-2)
	nop
.elseif(n = 15900)
	; 5300 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15901)
	; 5304 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 15903)
	; 5304 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15904)
	; 5302 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 15906)
	; 5302 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15907)
	; 5306 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15909)
	; 5306 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15910)
	; 5304 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15912)
	; 5304 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15913)
	; 5308 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15915)
	; 5308 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15916)
	; 5306 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 15918 to 15937
.if(n = 15918)
	; 5306 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15919)
	; 5310 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 15919 && n < 15936)))
	; 5307..5315 bytes
	delay_n (n-2)
	nop
.elseif(n = 15921)
	; 5310 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15922)
	; 5308 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15924)
	; 5308 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15925)
	; 5312 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15927)
	; 5312 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5314 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15933)
	; 5314 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15934)
	; 5312 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15936)
	; 5312 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15937)
	; 5316 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 15938) && (n < 16094))
.if((n >= 15938) && (n < 16016))
.if((n >= 15938) && (n < 15977))
.if((n >= 15938) && (n < 15957))
; 15938 to 15956
.if((n .mod 3 = 2) && ((n > 15937 && n < 15957)))
	; 5313..5321 bytes
	delay_n (n-2)
	nop
.elseif(n = 15939)
	; 5316 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15940)
	; 5314 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15942)
	; 5314 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15943)
	; 5318 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 15945)
	; 5318 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15946)
	; 5316 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 15948)
	; 5316 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 15949)
	; 5320 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 15951)
	; 5320 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 15952)
	; 5318 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15954)
	; 5318 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 15955)
	; 5322 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 15957 to 15976
.if(n = 15957)
	; 5322 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15958)
	; 5320 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 15958 && n < 15975)))
	; 5321..5327 bytes
	delay_n (n-2)
	nop
.elseif(n = 15960)
	; 5320 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 15961)
	; 5324 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 15963)
	; 5324 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15964)
	; 5322 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15966)
	; 5322 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15967)
	; 5326 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15969)
	; 5326 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 15970)
	; 5324 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 15972)
	; 5324 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15973)
	; 5328 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15975)
	; 5328 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 15976)
	; 5326 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 15977) && (n < 15996))
; 15977 to 15995
.if((n .mod 3 = 2) && ((n > 15976 && n < 15996)))
	; 5327..5335 bytes
	delay_n (n-2)
	nop
.elseif(n = 15978)
	; 5326 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 15979)
	; 5330 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 15981)
	; 5330 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5332 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15987)
	; 5332 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15988)
	; 5330 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 15990)
	; 5330 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 15991)
	; 5334 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 15993)
	; 5334 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 15994)
	; 5332 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 15996 to 16015
.if(n = 15996)
	; 5332 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15997)
	; 5336 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 15997 && n < 16014)))
	; 5333..5341 bytes
	delay_n (n-2)
	nop
.elseif(n = 15999)
	; 5336 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16000)
	; 5334 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16002)
	; 5334 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16003)
	; 5338 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16005)
	; 5338 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16006)
	; 5336 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16008)
	; 5336 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16009)
	; 5340 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16011)
	; 5340 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16012)
	; 5338 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16014)
	; 5338 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16015)
	; 5342 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.endif
.else
.if((n >= 16016) && (n < 16055))
.if((n >= 16016) && (n < 16035))
; 16016 to 16034
.if((n .mod 3 = 2) && ((n > 16015 && n < 16035)))
	; 5339..5347 bytes
	delay_n (n-2)
	nop
.elseif(n = 16017)
	; 5342 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16018)
	; 5340 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16020)
	; 5340 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16021)
	; 5344 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16023)
	; 5344 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16024)
	; 5342 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16026)
	; 5342 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16027)
	; 5346 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16029)
	; 5346 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16030)
	; 5344 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16032)
	; 5344 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16033)
	; 5348 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 16035 to 16054
.if(n = 16035)
	; 5348 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 16036)
	; 5346 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 16036 && n < 16053)))
	; 5347..5353 bytes
	delay_n (n-2)
	nop
.elseif(n = 16038)
	; 5346 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16039)
	; 5350 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16041)
	; 5350 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16042)
	; 5348 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16044)
	; 5348 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16045)
	; 5352 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16047)
	; 5352 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16048)
	; 5350 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16050)
	; 5350 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16051)
	; 5354 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16053)
	; 5354 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16054)
	; 5352 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 16055) && (n < 16074))
; 16055 to 16073
.if((n .mod 3 = 2) && ((n > 16054 && n < 16074)))
	; 5353..5361 bytes
	delay_n (n-2)
	nop
.elseif(n = 16056)
	; 5352 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16057)
	; 5356 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16059)
	; 5356 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16060)
	; 5354 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16062)
	; 5354 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16063)
	; 5358 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16065)
	; 5358 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16066)
	; 5356 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16068)
	; 5356 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16069)
	; 5360 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16071)
	; 5360 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16072)
	; 5358 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 16074 to 16093
.if(n = 16074)
	; 5358 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16075)
	; 5362 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 16075 && n < 16092)))
	; 5359..5367 bytes
	delay_n (n-2)
	nop
.elseif(n = 16077)
	; 5362 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16078)
	; 5360 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16080)
	; 5360 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16081)
	; 5364 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16083)
	; 5364 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16084)
	; 5362 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16086)
	; 5362 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16087)
	; 5366 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16089)
	; 5366 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5368 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.endif
.else
.if((n >= 16094) && (n < 16172))
.if((n >= 16094) && (n < 16133))
.if((n >= 16094) && (n < 16113))
; 16094 to 16112
.if((n .mod 3 = 2) && ((n > 16093 && n < 16113)))
	; 5365..5373 bytes
	delay_n (n-2)
	nop
.elseif(n = 16095)
	; 5368 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16096)
	; 5366 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16098)
	; 5366 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16099)
	; 5370 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16101)
	; 5370 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16102)
	; 5368 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16104)
	; 5368 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16105)
	; 5372 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16107)
	; 5372 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16108)
	; 5370 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16110)
	; 5370 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16111)
	; 5374 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 5374 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16114)
	; 5372 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 16114 && n < 16131)))
	; 5373..5379 bytes
	delay_n (n-2)
	nop
.elseif(n = 16116)
	; 5372 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16117)
	; 5376 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16119)
	; 5376 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16120)
	; 5374 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16122)
	; 5374 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16123)
	; 5378 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16125)
	; 5378 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16126)
	; 5376 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16128)
	; 5376 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16129)
	; 5380 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16131)
	; 5380 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16132)
	; 5378 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 16133) && (n < 16152))
; 16133 to 16151
.if((n .mod 3 = 2) && ((n > 16132 && n < 16152)))
	; 5379..5387 bytes
	delay_n (n-2)
	nop
.elseif(n = 16134)
	; 5378 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16135)
	; 5382 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16137)
	; 5382 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16138)
	; 5380 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16140)
	; 5380 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16141)
	; 5384 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16143)
	; 5384 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5386 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16149)
	; 5386 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16150)
	; 5384 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 16152 to 16171
.if(n = 16152)
	; 5384 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16153)
	; 5388 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 16153 && n < 16170)))
	; 5385..5393 bytes
	delay_n (n-2)
	nop
.elseif(n = 16155)
	; 5388 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16156)
	; 5386 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16158)
	; 5386 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16159)
	; 5390 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16161)
	; 5390 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16162)
	; 5388 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16164)
	; 5388 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16165)
	; 5392 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16167)
	; 5392 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16168)
	; 5390 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16170)
	; 5390 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16171)
	; 5394 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.else
.if((n >= 16172) && (n < 16211))
.if((n >= 16172) && (n < 16191))
; 16172 to 16190
.if((n .mod 3 = 2) && ((n > 16171 && n < 16191)))
	; 5391..5399 bytes
	delay_n (n-2)
	nop
.elseif(n = 16173)
	; 5394 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16174)
	; 5392 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16176)
	; 5392 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16177)
	; 5396 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16179)
	; 5396 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16180)
	; 5394 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16182)
	; 5394 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16183)
	; 5398 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16185)
	; 5398 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16186)
	; 5396 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16188)
	; 5396 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16189)
	; 5400 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 16191 to 16210
.if(n = 16191)
	; 5400 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16192)
	; 5398 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 16192 && n < 16209)))
	; 5399..5405 bytes
	delay_n (n-2)
	nop
.elseif(n = 16194)
	; 5398 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16195)
	; 5402 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16197)
	; 5402 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5404 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16203)
	; 5404 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16204)
	; 5402 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16206)
	; 5402 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16207)
	; 5406 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16209)
	; 5406 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16210)
	; 5404 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 16211) && (n < 16231))
; 16211 to 16230
.if((n .mod 3 = 2) && ((n > 16210 && n < 16230)))
	; 5405..5413 bytes
	delay_n (n-2)
	nop
.elseif(n = 16212)
	; 5404 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16213)
	; 5408 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16215)
	; 5408 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16216)
	; 5406 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16218)
	; 5406 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16219)
	; 5410 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16221)
	; 5410 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16222)
	; 5408 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16224)
	; 5408 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16225)
	; 5412 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16227)
	; 5412 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16228)
	; 5410 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16230)
	; 5410 bytes
	delay_n 84
	delay_n (n-84)
.endif
.else
; 16231 to 16250
.if(n = 16231)
	; 5414 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 16231 && n < 16251)))
	; 5411..5419 bytes
	delay_n (n-2)
	nop
.elseif(n = 16233)
	; 5414 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16234)
	; 5412 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16236)
	; 5412 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16237)
	; 5416 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16239)
	; 5416 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16240)
	; 5414 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16242)
	; 5414 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16243)
	; 5418 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16245)
	; 5418 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16246)
	; 5416 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16248)
	; 5416 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16249)
	; 5420 bytes
	delay_n 103
	delay_n (n-103)
.endif
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
.if((n >= 16251) && (n < 16270))
; 16251 to 16269
.if(n = 16251)
	; 5420 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 16252)
	; 5418 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 16252 && n < 16269)))
	; 5419..5425 bytes
	delay_n (n-2)
	nop
.elseif(n = 16254)
	; 5418 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16255)
	; 5422 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16257)
	; 5422 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16258)
	; 5420 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16260)
	; 5420 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16261)
	; 5424 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16263)
	; 5424 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16264)
	; 5422 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16266)
	; 5422 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16267)
	; 5426 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16269)
	; 5426 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 16270 to 16289
.if(n = 16270)
	; 5424 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 16270 && n < 16290)))
	; 5425..5433 bytes
	delay_n (n-2)
	nop
.elseif(n = 16272)
	; 5424 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16273)
	; 5428 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16275)
	; 5428 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16276)
	; 5426 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16278)
	; 5426 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16279)
	; 5430 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16281)
	; 5430 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16282)
	; 5428 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16284)
	; 5428 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16285)
	; 5432 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16287)
	; 5432 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16288)
	; 5430 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 16290) && (n < 16309))
; 16290 to 16308
.if(n = 16290)
	; 5430 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16291)
	; 5434 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 16291 && n < 16308)))
	; 5431..5439 bytes
	delay_n (n-2)
	nop
.elseif(n = 16293)
	; 5434 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16294)
	; 5432 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16296)
	; 5432 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16297)
	; 5436 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16299)
	; 5436 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16300)
	; 5434 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16302)
	; 5434 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16303)
	; 5438 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16305)
	; 5438 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 16306)
	; 5436 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16308)
	; 5436 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 16309 to 16328
.if(n = 16309)
	; 5440 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 16309 && n < 16329)))
	; 5437..5445 bytes
	delay_n (n-2)
	nop
.elseif(n = 16311)
	; 5440 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16312)
	; 5438 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16314)
	; 5438 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16315)
	; 5442 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16317)
	; 5442 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16318)
	; 5440 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16320)
	; 5440 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16321)
	; 5444 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16323)
	; 5444 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16324)
	; 5442 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16326)
	; 5442 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16327)
	; 5446 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.endif
.else
.if((n >= 16329) && (n < 16368))
.if((n >= 16329) && (n < 16348))
; 16329 to 16347
.if(n = 16329)
	; 5446 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16330)
	; 5444 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 16330 && n < 16347)))
	; 5445..5451 bytes
	delay_n (n-2)
	nop
.elseif(n = 16332)
	; 5444 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16333)
	; 5448 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16335)
	; 5448 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16336)
	; 5446 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16338)
	; 5446 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16339)
	; 5450 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16341)
	; 5450 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16342)
	; 5448 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16344)
	; 5448 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16345)
	; 5452 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16347)
	; 5452 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 16348 to 16367
.if(n = 16348)
	; 5450 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 16348 && n < 16368)))
	; 5451..5459 bytes
	delay_n (n-2)
	nop
.elseif(n = 16350)
	; 5450 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16351)
	; 5454 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16353)
	; 5454 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16354)
	; 5452 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16356)
	; 5452 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16357)
	; 5456 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16359)
	; 5456 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5458 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16365)
	; 5458 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16366)
	; 5456 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.else
.if((n >= 16368) && (n < 16387))
; 16368 to 16386
.if(n = 16368)
	; 5456 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16369)
	; 5460 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 16369 && n < 16386)))
	; 5457..5465 bytes
	delay_n (n-2)
	nop
.elseif(n = 16371)
	; 5460 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16372)
	; 5458 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16374)
	; 5458 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16375)
	; 5462 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16377)
	; 5462 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16378)
	; 5460 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16380)
	; 5460 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16381)
	; 5464 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16383)
	; 5464 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16384)
	; 5462 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16386)
	; 5462 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 16387 to 16406
.if(n = 16387)
	; 5466 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 16387 && n < 16407)))
	; 5463..5471 bytes
	delay_n (n-2)
	nop
.elseif(n = 16389)
	; 5466 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16390)
	; 5464 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16392)
	; 5464 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16393)
	; 5468 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16395)
	; 5468 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16396)
	; 5466 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16398)
	; 5466 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16399)
	; 5470 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16401)
	; 5470 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16402)
	; 5468 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16404)
	; 5468 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16405)
	; 5472 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.endif
.else
.if((n >= 16407) && (n < 16485))
.if((n >= 16407) && (n < 16446))
.if((n >= 16407) && (n < 16426))
; 16407 to 16425
.if(n = 16407)
	; 5472 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16408)
	; 5470 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 16408 && n < 16425)))
	; 5471..5477 bytes
	delay_n (n-2)
	nop
.elseif(n = 16410)
	; 5470 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16411)
	; 5474 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16413)
	; 5474 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5476 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16419)
	; 5476 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16420)
	; 5474 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16422)
	; 5474 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16423)
	; 5478 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16425)
	; 5478 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 16426 to 16445
.if(n = 16426)
	; 5476 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 16426 && n < 16446)))
	; 5477..5485 bytes
	delay_n (n-2)
	nop
.elseif(n = 16428)
	; 5476 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16429)
	; 5480 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16431)
	; 5480 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16432)
	; 5478 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16434)
	; 5478 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16435)
	; 5482 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16437)
	; 5482 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16438)
	; 5480 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16440)
	; 5480 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16441)
	; 5484 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16443)
	; 5484 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16444)
	; 5482 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 16446) && (n < 16465))
; 16446 to 16464
.if(n = 16446)
	; 5482 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16447)
	; 5486 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 16447 && n < 16464)))
	; 5483..5491 bytes
	delay_n (n-2)
	nop
.elseif(n = 16449)
	; 5486 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16450)
	; 5484 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16452)
	; 5484 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16453)
	; 5488 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16455)
	; 5488 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16456)
	; 5486 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16458)
	; 5486 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16459)
	; 5490 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16461)
	; 5490 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16462)
	; 5488 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16464)
	; 5488 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 16465 to 16484
.if(n = 16465)
	; 5492 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 16465 && n < 16485)))
	; 5489..5497 bytes
	delay_n (n-2)
	nop
.elseif(n = 16467)
	; 5492 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5494 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16473)
	; 5494 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16474)
	; 5492 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16476)
	; 5492 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16477)
	; 5496 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16479)
	; 5496 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16480)
	; 5494 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16482)
	; 5494 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16483)
	; 5498 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 16485) && (n < 16524))
.if((n >= 16485) && (n < 16504))
; 16485 to 16503
.if(n = 16485)
	; 5498 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16486)
	; 5496 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 16486 && n < 16503)))
	; 5497..5503 bytes
	delay_n (n-2)
	nop
.elseif(n = 16488)
	; 5496 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16489)
	; 5500 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16491)
	; 5500 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16492)
	; 5498 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16494)
	; 5498 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16495)
	; 5502 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16497)
	; 5502 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16498)
	; 5500 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16500)
	; 5500 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16501)
	; 5504 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16503)
	; 5504 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 16504 to 16523
.if(n = 16504)
	; 5502 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 16504 && n < 16524)))
	; 5503..5511 bytes
	delay_n (n-2)
	nop
.elseif(n = 16506)
	; 5502 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16507)
	; 5506 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16509)
	; 5506 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16510)
	; 5504 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16512)
	; 5504 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16513)
	; 5508 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16515)
	; 5508 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16516)
	; 5506 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16518)
	; 5506 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16519)
	; 5510 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16521)
	; 5510 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 16522)
	; 5508 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 16524) && (n < 16543))
; 16524 to 16542
.if(n = 16524)
	; 5508 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16525)
	; 5512 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 16525 && n < 16542)))
	; 5509..5517 bytes
	delay_n (n-2)
	nop
.elseif(n = 16527)
	; 5512 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16528)
	; 5510 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16530)
	; 5510 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16531)
	; 5514 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16533)
	; 5514 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16534)
	; 5512 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16536)
	; 5512 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16537)
	; 5516 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16539)
	; 5516 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16540)
	; 5514 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16542)
	; 5514 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 16543 to 16562
.if(n = 16543)
	; 5518 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 16543 && n < 16563)))
	; 5515..5523 bytes
	delay_n (n-2)
	nop
.elseif(n = 16545)
	; 5518 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16546)
	; 5516 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16548)
	; 5516 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16549)
	; 5520 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16551)
	; 5520 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16552)
	; 5518 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16554)
	; 5518 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16555)
	; 5522 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16557)
	; 5522 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16558)
	; 5520 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16560)
	; 5520 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16561)
	; 5524 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
.if((n >= 16563) && (n < 16602))
.if((n >= 16563) && (n < 16582))
; 16563 to 16581
.if(n = 16563)
	; 5524 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16564)
	; 5522 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 16564 && n < 16581)))
	; 5523..5529 bytes
	delay_n (n-2)
	nop
.elseif(n = 16566)
	; 5522 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16567)
	; 5526 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16569)
	; 5526 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16570)
	; 5524 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16572)
	; 5524 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16573)
	; 5528 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16575)
	; 5528 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5530 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16581)
	; 5530 bytes
	delay_n 57
	delay_n (n-57)
.endif
.else
; 16582 to 16601
.if(n = 16582)
	; 5528 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 16582 && n < 16602)))
	; 5529..5537 bytes
	delay_n (n-2)
	nop
.elseif(n = 16584)
	; 5528 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16585)
	; 5532 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16587)
	; 5532 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16588)
	; 5530 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16590)
	; 5530 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16591)
	; 5534 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16593)
	; 5534 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16594)
	; 5532 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16596)
	; 5532 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16597)
	; 5536 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16599)
	; 5536 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16600)
	; 5534 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 16602) && (n < 16621))
; 16602 to 16620
.if(n = 16602)
	; 5534 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16603)
	; 5538 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 16603 && n < 16620)))
	; 5535..5543 bytes
	delay_n (n-2)
	nop
.elseif(n = 16605)
	; 5538 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16606)
	; 5536 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16608)
	; 5536 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16609)
	; 5540 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16611)
	; 5540 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16612)
	; 5538 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16614)
	; 5538 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16615)
	; 5542 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16617)
	; 5542 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16618)
	; 5540 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16620)
	; 5540 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 16621 to 16640
.if(n = 16621)
	; 5544 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 16621 && n < 16641)))
	; 5541..5549 bytes
	delay_n (n-2)
	nop
.elseif(n = 16623)
	; 5544 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16624)
	; 5542 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16626)
	; 5542 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16627)
	; 5546 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16629)
	; 5546 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5548 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16635)
	; 5548 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16636)
	; 5546 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16638)
	; 5546 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16639)
	; 5550 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.endif
.else
.if((n >= 16641) && (n < 16680))
.if((n >= 16641) && (n < 16660))
; 16641 to 16659
.if(n = 16641)
	; 5550 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16642)
	; 5548 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 16642 && n < 16659)))
	; 5549..5555 bytes
	delay_n (n-2)
	nop
.elseif(n = 16644)
	; 5548 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16645)
	; 5552 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16647)
	; 5552 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16648)
	; 5550 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16650)
	; 5550 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16651)
	; 5554 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16653)
	; 5554 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16654)
	; 5552 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16656)
	; 5552 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16657)
	; 5556 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16659)
	; 5556 bytes
	delay_n 81
	delay_n (n-81)
.endif
.else
; 16660 to 16679
.if(n = 16660)
	; 5554 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 16660 && n < 16680)))
	; 5555..5563 bytes
	delay_n (n-2)
	nop
.elseif(n = 16662)
	; 5554 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16663)
	; 5558 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16665)
	; 5558 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16666)
	; 5556 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16668)
	; 5556 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16669)
	; 5560 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16671)
	; 5560 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16672)
	; 5558 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16674)
	; 5558 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16675)
	; 5562 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16677)
	; 5562 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16678)
	; 5560 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 16680) && (n < 16699))
; 16680 to 16698
.if(n = 16680)
	; 5560 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16681)
	; 5564 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 16681 && n < 16698)))
	; 5561..5569 bytes
	delay_n (n-2)
	nop
.elseif(n = 16683)
	; 5564 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5566 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16689)
	; 5566 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16690)
	; 5564 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16692)
	; 5564 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16693)
	; 5568 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16695)
	; 5568 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16696)
	; 5566 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16698)
	; 5566 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 16699 to 16718
.if(n = 16699)
	; 5570 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 16699 && n < 16719)))
	; 5567..5575 bytes
	delay_n (n-2)
	nop
.elseif(n = 16701)
	; 5570 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16702)
	; 5568 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16704)
	; 5568 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16705)
	; 5572 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16707)
	; 5572 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16708)
	; 5570 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16710)
	; 5570 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16711)
	; 5574 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16713)
	; 5574 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16714)
	; 5572 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16716)
	; 5572 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16717)
	; 5576 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.endif
.endif
.else
.if((n >= 16719) && (n < 16797))
.if((n >= 16719) && (n < 16758))
.if((n >= 16719) && (n < 16738))
; 16719 to 16737
.if(n = 16719)
	; 5576 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16720)
	; 5574 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 16720 && n < 16737)))
	; 5575..5581 bytes
	delay_n (n-2)
	nop
.elseif(n = 16722)
	; 5574 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16723)
	; 5578 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16725)
	; 5578 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16726)
	; 5576 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16728)
	; 5576 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16729)
	; 5580 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16731)
	; 5580 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16732)
	; 5578 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16734)
	; 5578 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16735)
	; 5582 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16737)
	; 5582 bytes
	delay_n 105
	delay_n (n-105)
.endif
.else
; 16738 to 16757
.if(n = 16738)
	; 5580 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 16738 && n < 16758)))
	; 5581..5589 bytes
	delay_n (n-2)
	nop
.elseif(n = 16740)
	; 5580 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16741)
	; 5584 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16743)
	; 5584 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16744)
	; 5582 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16746)
	; 5582 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16747)
	; 5586 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16749)
	; 5586 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16750)
	; 5584 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16752)
	; 5584 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16753)
	; 5588 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16755)
	; 5588 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16756)
	; 5586 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 16758) && (n < 16777))
; 16758 to 16776
.if(n = 16758)
	; 5586 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16759)
	; 5590 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 16759 && n < 16776)))
	; 5587..5595 bytes
	delay_n (n-2)
	nop
.elseif(n = 16761)
	; 5590 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16762)
	; 5588 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16764)
	; 5588 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16765)
	; 5592 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16767)
	; 5592 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16768)
	; 5590 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16770)
	; 5590 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16771)
	; 5594 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16773)
	; 5594 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16774)
	; 5592 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16776)
	; 5592 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 16777 to 16796
.if(n = 16777)
	; 5596 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 16777 && n < 16797)))
	; 5593..5601 bytes
	delay_n (n-2)
	nop
.elseif(n = 16779)
	; 5596 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16780)
	; 5594 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16782)
	; 5594 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16783)
	; 5598 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16785)
	; 5598 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16786)
	; 5596 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16788)
	; 5596 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16789)
	; 5600 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16791)
	; 5600 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5602 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.else
.if((n >= 16797) && (n < 16836))
.if((n >= 16797) && (n < 16816))
; 16797 to 16815
.if(n = 16797)
	; 5602 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16798)
	; 5600 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 16798 && n < 16815)))
	; 5601..5607 bytes
	delay_n (n-2)
	nop
.elseif(n = 16800)
	; 5600 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16801)
	; 5604 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16803)
	; 5604 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16804)
	; 5602 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16806)
	; 5602 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16807)
	; 5606 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16809)
	; 5606 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16810)
	; 5604 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16812)
	; 5604 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16813)
	; 5608 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16815)
	; 5608 bytes
	delay_n 129
	delay_n (n-129)
.endif
.else
; 16816 to 16835
.if(n = 16816)
	; 5606 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 16816 && n < 16836)))
	; 5607..5615 bytes
	delay_n (n-2)
	nop
.elseif(n = 16818)
	; 5606 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16819)
	; 5610 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16821)
	; 5610 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16822)
	; 5608 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16824)
	; 5608 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16825)
	; 5612 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16827)
	; 5612 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16828)
	; 5610 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16830)
	; 5610 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16831)
	; 5614 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16833)
	; 5614 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16834)
	; 5612 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 16836) && (n < 16856))
; 16836 to 16855
.if(n = 16836)
	; 5612 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16837)
	; 5616 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 16837 && n < 16854)))
	; 5613..5621 bytes
	delay_n (n-2)
	nop
.elseif(n = 16839)
	; 5616 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16840)
	; 5614 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16842)
	; 5614 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16843)
	; 5618 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16845)
	; 5618 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5620 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16851)
	; 5620 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16852)
	; 5618 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16854)
	; 5618 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16855)
	; 5622 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 16856 to 16875
.if((n .mod 3 = 2) && ((n > 16855 && n < 16875)))
	; 5619..5627 bytes
	delay_n (n-2)
	nop
.elseif(n = 16857)
	; 5622 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16858)
	; 5620 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16860)
	; 5620 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16861)
	; 5624 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16863)
	; 5624 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16864)
	; 5622 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16866)
	; 5622 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16867)
	; 5626 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16869)
	; 5626 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16870)
	; 5624 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16872)
	; 5624 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16873)
	; 5628 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16875)
	; 5628 bytes
	delay_n 81
	delay_n (n-81)
.endif
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
.if((n >= 16876) && (n < 16895))
; 16876 to 16894
.if(n = 16876)
	; 5626 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 16876 && n < 16893)))
	; 5627..5633 bytes
	delay_n (n-2)
	nop
.elseif(n = 16878)
	; 5626 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16879)
	; 5630 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16881)
	; 5630 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16882)
	; 5628 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16884)
	; 5628 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16885)
	; 5632 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16887)
	; 5632 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16888)
	; 5630 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16890)
	; 5630 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16891)
	; 5634 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16893)
	; 5634 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16894)
	; 5632 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 16895 to 16914
.if((n .mod 3 = 2) && ((n > 16894 && n < 16914)))
	; 5633..5641 bytes
	delay_n (n-2)
	nop
.elseif(n = 16896)
	; 5632 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16897)
	; 5636 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16899)
	; 5636 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5638 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16905)
	; 5638 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16906)
	; 5636 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16908)
	; 5636 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16909)
	; 5640 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16911)
	; 5640 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16912)
	; 5638 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16914)
	; 5638 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 16915) && (n < 16934))
; 16915 to 16933
.if(n = 16915)
	; 5642 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 16915 && n < 16932)))
	; 5639..5647 bytes
	delay_n (n-2)
	nop
.elseif(n = 16917)
	; 5642 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16918)
	; 5640 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 16920)
	; 5640 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16921)
	; 5644 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16923)
	; 5644 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16924)
	; 5642 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16926)
	; 5642 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16927)
	; 5646 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16929)
	; 5646 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16930)
	; 5644 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16932)
	; 5644 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16933)
	; 5648 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 16934 to 16953
.if((n .mod 3 = 2) && ((n > 16933 && n < 16953)))
	; 5645..5653 bytes
	delay_n (n-2)
	nop
.elseif(n = 16935)
	; 5648 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16936)
	; 5646 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16938)
	; 5646 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16939)
	; 5650 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16941)
	; 5650 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16942)
	; 5648 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16944)
	; 5648 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16945)
	; 5652 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16947)
	; 5652 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 16948)
	; 5650 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16950)
	; 5650 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 16951)
	; 5654 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 16953)
	; 5654 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.endif
.else
.if((n >= 16954) && (n < 16993))
.if((n >= 16954) && (n < 16973))
; 16954 to 16972
.if(n = 16954)
	; 5652 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 16954 && n < 16971)))
	; 5653..5659 bytes
	delay_n (n-2)
	nop
.elseif(n = 16956)
	; 5652 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 16957)
	; 5656 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16959)
	; 5656 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16960)
	; 5654 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 16962)
	; 5654 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 16963)
	; 5658 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 16965)
	; 5658 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 16966)
	; 5656 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16968)
	; 5656 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16969)
	; 5660 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 16971)
	; 5660 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 16972)
	; 5658 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 16973 to 16992
.if((n .mod 3 = 2) && ((n > 16972 && n < 16992)))
	; 5659..5667 bytes
	delay_n (n-2)
	nop
.elseif(n = 16974)
	; 5658 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 16975)
	; 5662 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16977)
	; 5662 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 16978)
	; 5660 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16980)
	; 5660 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 16981)
	; 5664 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 16983)
	; 5664 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16984)
	; 5662 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 16986)
	; 5662 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 16987)
	; 5666 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 16989)
	; 5666 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16990)
	; 5664 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16992)
	; 5664 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.else
.if((n >= 16993) && (n < 17012))
; 16993 to 17011
.if(n = 16993)
	; 5668 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 16993 && n < 17010)))
	; 5665..5673 bytes
	delay_n (n-2)
	nop
.elseif(n = 16995)
	; 5668 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 16996)
	; 5666 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 16998)
	; 5666 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16999)
	; 5670 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17001)
	; 5670 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17002)
	; 5668 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17004)
	; 5668 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17005)
	; 5672 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17007)
	; 5672 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5674 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 17012 to 17031
.if((n .mod 3 = 2) && ((n > 17011 && n < 17031)))
	; 5671..5679 bytes
	delay_n (n-2)
	nop
.elseif(n = 17013)
	; 5674 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17014)
	; 5672 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17016)
	; 5672 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17017)
	; 5676 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17019)
	; 5676 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17020)
	; 5674 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17022)
	; 5674 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17023)
	; 5678 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17025)
	; 5678 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17026)
	; 5676 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17028)
	; 5676 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17029)
	; 5680 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17031)
	; 5680 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
.if((n >= 17032) && (n < 17071))
.if((n >= 17032) && (n < 17051))
; 17032 to 17050
.if(n = 17032)
	; 5678 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 17032 && n < 17049)))
	; 5679..5685 bytes
	delay_n (n-2)
	nop
.elseif(n = 17034)
	; 5678 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17035)
	; 5682 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17037)
	; 5682 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17038)
	; 5680 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17040)
	; 5680 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17041)
	; 5684 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17043)
	; 5684 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17044)
	; 5682 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17046)
	; 5682 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17047)
	; 5686 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17049)
	; 5686 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17050)
	; 5684 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 17051 to 17070
.if((n .mod 3 = 2) && ((n > 17050 && n < 17070)))
	; 5685..5693 bytes
	delay_n (n-2)
	nop
.elseif(n = 17052)
	; 5684 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17053)
	; 5688 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17055)
	; 5688 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17056)
	; 5686 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17058)
	; 5686 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17059)
	; 5690 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17061)
	; 5690 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5692 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17067)
	; 5692 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17068)
	; 5690 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17070)
	; 5690 bytes
	delay_n 60
	delay_n (n-60)
.endif
.endif
.else
.if((n >= 17071) && (n < 17090))
; 17071 to 17089
.if(n = 17071)
	; 5694 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 17071 && n < 17088)))
	; 5691..5699 bytes
	delay_n (n-2)
	nop
.elseif(n = 17073)
	; 5694 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17074)
	; 5692 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17076)
	; 5692 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17077)
	; 5696 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17079)
	; 5696 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17080)
	; 5694 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17082)
	; 5694 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17083)
	; 5698 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17085)
	; 5698 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17086)
	; 5696 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17088)
	; 5696 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17089)
	; 5700 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 17090 to 17109
.if((n .mod 3 = 2) && ((n > 17089 && n < 17109)))
	; 5697..5705 bytes
	delay_n (n-2)
	nop
.elseif(n = 17091)
	; 5700 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17092)
	; 5698 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17094)
	; 5698 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17095)
	; 5702 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17097)
	; 5702 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17098)
	; 5700 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17100)
	; 5700 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17101)
	; 5704 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17103)
	; 5704 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17104)
	; 5702 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17106)
	; 5702 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17107)
	; 5706 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17109)
	; 5706 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.endif
.else
.if((n >= 17110) && (n < 17149))
.if((n >= 17110) && (n < 17129))
; 17110 to 17128
.if(n = 17110)
	; 5704 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 17110 && n < 17127)))
	; 5705..5711 bytes
	delay_n (n-2)
	nop
.elseif(n = 17112)
	; 5704 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17113)
	; 5708 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17115)
	; 5708 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5710 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17121)
	; 5710 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17122)
	; 5708 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17124)
	; 5708 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17125)
	; 5712 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17127)
	; 5712 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17128)
	; 5710 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 17129 to 17148
.if((n .mod 3 = 2) && ((n > 17128 && n < 17148)))
	; 5711..5719 bytes
	delay_n (n-2)
	nop
.elseif(n = 17130)
	; 5710 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17131)
	; 5714 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17133)
	; 5714 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17134)
	; 5712 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17136)
	; 5712 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17137)
	; 5716 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17139)
	; 5716 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17140)
	; 5714 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17142)
	; 5714 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17143)
	; 5718 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17145)
	; 5718 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17146)
	; 5716 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17148)
	; 5716 bytes
	delay_n 84
	delay_n (n-84)
.endif
.endif
.else
.if((n >= 17149) && (n < 17168))
; 17149 to 17167
.if(n = 17149)
	; 5720 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 17149 && n < 17166)))
	; 5717..5725 bytes
	delay_n (n-2)
	nop
.elseif(n = 17151)
	; 5720 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17152)
	; 5718 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17154)
	; 5718 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17155)
	; 5722 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17157)
	; 5722 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17158)
	; 5720 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17160)
	; 5720 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17161)
	; 5724 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17163)
	; 5724 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17164)
	; 5722 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17166)
	; 5722 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17167)
	; 5726 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 17168 to 17187
.if((n .mod 3 = 2) && ((n > 17167 && n < 17187)))
	; 5723..5731 bytes
	delay_n (n-2)
	nop
.elseif(n = 17169)
	; 5726 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5728 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17175)
	; 5728 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17176)
	; 5726 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17178)
	; 5726 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17179)
	; 5730 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17181)
	; 5730 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17182)
	; 5728 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17184)
	; 5728 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17185)
	; 5732 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17187)
	; 5732 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
.if((n >= 17188) && (n < 17266))
.if((n >= 17188) && (n < 17227))
.if((n >= 17188) && (n < 17207))
; 17188 to 17206
.if(n = 17188)
	; 5730 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 17188 && n < 17205)))
	; 5731..5737 bytes
	delay_n (n-2)
	nop
.elseif(n = 17190)
	; 5730 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17191)
	; 5734 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17193)
	; 5734 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17194)
	; 5732 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17196)
	; 5732 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17197)
	; 5736 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17199)
	; 5736 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17200)
	; 5734 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17202)
	; 5734 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17203)
	; 5738 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17205)
	; 5738 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17206)
	; 5736 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 17207 to 17226
.if((n .mod 3 = 2) && ((n > 17206 && n < 17226)))
	; 5737..5745 bytes
	delay_n (n-2)
	nop
.elseif(n = 17208)
	; 5736 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17209)
	; 5740 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17211)
	; 5740 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17212)
	; 5738 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17214)
	; 5738 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17215)
	; 5742 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17217)
	; 5742 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17218)
	; 5740 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17220)
	; 5740 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17221)
	; 5744 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17223)
	; 5744 bytes
	delay_n 105
	delay_n (n-105)
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
.endif
.else
.if((n >= 17227) && (n < 17246))
; 17227 to 17245
.if(n = 17227)
	; 5746 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 17227 && n < 17244)))
	; 5743..5751 bytes
	delay_n (n-2)
	nop
.elseif(n = 17229)
	; 5746 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17230)
	; 5744 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17232)
	; 5744 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17233)
	; 5748 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17235)
	; 5748 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17236)
	; 5746 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17238)
	; 5746 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17239)
	; 5750 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17241)
	; 5750 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17242)
	; 5748 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17244)
	; 5748 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17245)
	; 5752 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 17246 to 17265
.if((n .mod 3 = 2) && ((n > 17245 && n < 17265)))
	; 5749..5757 bytes
	delay_n (n-2)
	nop
.elseif(n = 17247)
	; 5752 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17248)
	; 5750 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17250)
	; 5750 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17251)
	; 5754 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17253)
	; 5754 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17254)
	; 5752 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17256)
	; 5752 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17257)
	; 5756 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17259)
	; 5756 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17260)
	; 5754 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17262)
	; 5754 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17263)
	; 5758 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17265)
	; 5758 bytes
	delay_n 93
	delay_n (n-93)
.endif
.endif
.endif
.else
.if((n >= 17266) && (n < 17305))
.if((n >= 17266) && (n < 17285))
; 17266 to 17284
.if(n = 17266)
	; 5756 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 17266 && n < 17283)))
	; 5757..5763 bytes
	delay_n (n-2)
	nop
.elseif(n = 17268)
	; 5756 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17269)
	; 5760 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17271)
	; 5760 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17272)
	; 5758 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17274)
	; 5758 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17275)
	; 5762 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17277)
	; 5762 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5764 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17283)
	; 5764 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17284)
	; 5762 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 17285 to 17304
.if((n .mod 3 = 2) && ((n > 17284 && n < 17304)))
	; 5763..5771 bytes
	delay_n (n-2)
	nop
.elseif(n = 17286)
	; 5762 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17287)
	; 5766 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17289)
	; 5766 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17290)
	; 5764 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17292)
	; 5764 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17293)
	; 5768 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17295)
	; 5768 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17296)
	; 5766 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17298)
	; 5766 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17299)
	; 5770 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17301)
	; 5770 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17302)
	; 5768 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17304)
	; 5768 bytes
	delay_n 24
	delay_n (n-24)
.endif
.endif
.else
.if((n >= 17305) && (n < 17324))
; 17305 to 17323
.if(n = 17305)
	; 5772 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 17305 && n < 17322)))
	; 5769..5777 bytes
	delay_n (n-2)
	nop
.elseif(n = 17307)
	; 5772 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17308)
	; 5770 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17310)
	; 5770 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17311)
	; 5774 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17313)
	; 5774 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17314)
	; 5772 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17316)
	; 5772 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17317)
	; 5776 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17319)
	; 5776 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17320)
	; 5774 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17322)
	; 5774 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17323)
	; 5778 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 17324 to 17343
.if((n .mod 3 = 2) && ((n > 17323 && n < 17343)))
	; 5775..5783 bytes
	delay_n (n-2)
	nop
.elseif(n = 17325)
	; 5778 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17326)
	; 5776 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17328)
	; 5776 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17329)
	; 5780 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17331)
	; 5780 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5782 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17337)
	; 5782 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17338)
	; 5780 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17340)
	; 5780 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17341)
	; 5784 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17343)
	; 5784 bytes
	delay_n 117
	delay_n (n-117)
.endif
.endif
.endif
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
.if((n >= 17344) && (n < 17363))
; 17344 to 17362
.if(n = 17344)
	; 5782 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 17344 && n < 17361)))
	; 5783..5789 bytes
	delay_n (n-2)
	nop
.elseif(n = 17346)
	; 5782 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17347)
	; 5786 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17349)
	; 5786 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17350)
	; 5784 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17352)
	; 5784 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17353)
	; 5788 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17355)
	; 5788 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17356)
	; 5786 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17358)
	; 5786 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17359)
	; 5790 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17361)
	; 5790 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17362)
	; 5788 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 17363 to 17382
.if((n .mod 3 = 2) && ((n > 17362 && n < 17382)))
	; 5789..5797 bytes
	delay_n (n-2)
	nop
.elseif(n = 17364)
	; 5788 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17365)
	; 5792 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17367)
	; 5792 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17368)
	; 5790 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17370)
	; 5790 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17371)
	; 5794 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17373)
	; 5794 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17374)
	; 5792 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17376)
	; 5792 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17377)
	; 5796 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17379)
	; 5796 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17380)
	; 5794 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17382)
	; 5794 bytes
	delay_n 48
	delay_n (n-48)
.endif
.endif
.else
.if((n >= 17383) && (n < 17402))
; 17383 to 17401
.if(n = 17383)
	; 5798 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 17383 && n < 17400)))
	; 5795..5803 bytes
	delay_n (n-2)
	nop
.elseif(n = 17385)
	; 5798 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5800 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17391)
	; 5800 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17392)
	; 5798 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17394)
	; 5798 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17395)
	; 5802 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17397)
	; 5802 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17398)
	; 5800 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17400)
	; 5800 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17401)
	; 5804 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 17402 to 17421
.if((n .mod 3 = 2) && ((n > 17401 && n < 17421)))
	; 5801..5809 bytes
	delay_n (n-2)
	nop
.elseif(n = 17403)
	; 5804 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17404)
	; 5802 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17406)
	; 5802 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17407)
	; 5806 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17409)
	; 5806 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17410)
	; 5804 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17412)
	; 5804 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17413)
	; 5808 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17415)
	; 5808 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17416)
	; 5806 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17418)
	; 5806 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17419)
	; 5810 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17421)
	; 5810 bytes
	delay_n 141
	delay_n (n-141)
.endif
.endif
.endif
.else
.if((n >= 17422) && (n < 17461))
.if((n >= 17422) && (n < 17441))
; 17422 to 17440
.if(n = 17422)
	; 5808 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 17422 && n < 17439)))
	; 5809..5815 bytes
	delay_n (n-2)
	nop
.elseif(n = 17424)
	; 5808 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17425)
	; 5812 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17427)
	; 5812 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17428)
	; 5810 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17430)
	; 5810 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17431)
	; 5814 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17433)
	; 5814 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17434)
	; 5812 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17436)
	; 5812 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17437)
	; 5816 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17439)
	; 5816 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 17440)
	; 5814 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 17441 to 17460
.if((n .mod 3 = 2) && ((n > 17440 && n < 17460)))
	; 5815..5823 bytes
	delay_n (n-2)
	nop
.elseif(n = 17442)
	; 5814 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17443)
	; 5818 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17445)
	; 5818 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17446)
	; 5816 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17448)
	; 5816 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17449)
	; 5820 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17451)
	; 5820 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17452)
	; 5818 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17454)
	; 5818 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17455)
	; 5822 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17457)
	; 5822 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17458)
	; 5820 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17460)
	; 5820 bytes
	delay_n 72
	delay_n (n-72)
.endif
.endif
.else
.if((n >= 17461) && (n < 17481))
; 17461 to 17480
.if(n = 17461)
	; 5824 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 17461 && n < 17481)))
	; 5821..5829 bytes
	delay_n (n-2)
	nop
.elseif(n = 17463)
	; 5824 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17464)
	; 5822 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17466)
	; 5822 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17467)
	; 5826 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17469)
	; 5826 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17470)
	; 5824 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17472)
	; 5824 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17473)
	; 5828 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17475)
	; 5828 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17476)
	; 5826 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17478)
	; 5826 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17479)
	; 5830 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 17481 to 17500
.if(n = 17481)
	; 5830 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17482)
	; 5828 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 17482 && n < 17499)))
	; 5829..5835 bytes
	delay_n (n-2)
	nop
.elseif(n = 17484)
	; 5828 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17485)
	; 5832 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17487)
	; 5832 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17488)
	; 5830 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17490)
	; 5830 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17491)
	; 5834 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17493)
	; 5834 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5836 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17499)
	; 5836 bytes
	delay_n 57
	delay_n (n-57)
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
.endif
.else
.if((n >= 17501) && (n < 18751))
.if((n >= 17501) && (n < 18126))
.if((n >= 17501) && (n < 17813))
.if((n >= 17501) && (n < 17657))
.if((n >= 17501) && (n < 17579))
.if((n >= 17501) && (n < 17540))
.if((n >= 17501) && (n < 17520))
; 17501 to 17519
.if((n .mod 3 = 2) && ((n > 17500 && n < 17520)))
	; 5835..5843 bytes
	delay_n (n-2)
	nop
.elseif(n = 17502)
	; 5834 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17503)
	; 5838 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17505)
	; 5838 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17506)
	; 5836 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17508)
	; 5836 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17509)
	; 5840 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17511)
	; 5840 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17512)
	; 5838 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17514)
	; 5838 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17515)
	; 5842 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17517)
	; 5842 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17518)
	; 5840 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 17520 to 17539
.if(n = 17520)
	; 5840 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17521)
	; 5844 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 17521 && n < 17538)))
	; 5841..5849 bytes
	delay_n (n-2)
	nop
.elseif(n = 17523)
	; 5844 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17524)
	; 5842 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17526)
	; 5842 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17527)
	; 5846 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17529)
	; 5846 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17530)
	; 5844 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17532)
	; 5844 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17533)
	; 5848 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17535)
	; 5848 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17536)
	; 5846 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17538)
	; 5846 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17539)
	; 5850 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 17540) && (n < 17559))
; 17540 to 17558
.if((n .mod 3 = 2) && ((n > 17539 && n < 17559)))
	; 5847..5855 bytes
	delay_n (n-2)
	nop
.elseif(n = 17541)
	; 5850 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17542)
	; 5848 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17544)
	; 5848 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17545)
	; 5852 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17547)
	; 5852 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5854 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17553)
	; 5854 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17554)
	; 5852 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17556)
	; 5852 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17557)
	; 5856 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 17559 to 17578
.if(n = 17559)
	; 5856 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17560)
	; 5854 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 17560 && n < 17577)))
	; 5855..5861 bytes
	delay_n (n-2)
	nop
.elseif(n = 17562)
	; 5854 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17563)
	; 5858 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17565)
	; 5858 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17566)
	; 5856 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17568)
	; 5856 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17569)
	; 5860 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17571)
	; 5860 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17572)
	; 5858 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17574)
	; 5858 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17575)
	; 5862 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17577)
	; 5862 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17578)
	; 5860 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.else
.if((n >= 17579) && (n < 17618))
.if((n >= 17579) && (n < 17598))
; 17579 to 17597
.if((n .mod 3 = 2) && ((n > 17578 && n < 17598)))
	; 5861..5869 bytes
	delay_n (n-2)
	nop
.elseif(n = 17580)
	; 5860 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17581)
	; 5864 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17583)
	; 5864 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17584)
	; 5862 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17586)
	; 5862 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17587)
	; 5866 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17589)
	; 5866 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17590)
	; 5864 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17592)
	; 5864 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17593)
	; 5868 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17595)
	; 5868 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17596)
	; 5866 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 17598 to 17617
.if(n = 17598)
	; 5866 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17599)
	; 5870 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 17599 && n < 17616)))
	; 5867..5875 bytes
	delay_n (n-2)
	nop
.elseif(n = 17601)
	; 5870 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5872 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17607)
	; 5872 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17608)
	; 5870 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17610)
	; 5870 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17611)
	; 5874 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17613)
	; 5874 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17614)
	; 5872 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17616)
	; 5872 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17617)
	; 5876 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 17618) && (n < 17637))
; 17618 to 17636
.if((n .mod 3 = 2) && ((n > 17617 && n < 17637)))
	; 5873..5881 bytes
	delay_n (n-2)
	nop
.elseif(n = 17619)
	; 5876 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17620)
	; 5874 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17622)
	; 5874 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17623)
	; 5878 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17625)
	; 5878 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17626)
	; 5876 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17628)
	; 5876 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17629)
	; 5880 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17631)
	; 5880 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17632)
	; 5878 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17634)
	; 5878 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17635)
	; 5882 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 17637 to 17656
.if(n = 17637)
	; 5882 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17638)
	; 5880 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 17638 && n < 17655)))
	; 5881..5887 bytes
	delay_n (n-2)
	nop
.elseif(n = 17640)
	; 5880 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17641)
	; 5884 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17643)
	; 5884 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17644)
	; 5882 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17646)
	; 5882 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17647)
	; 5886 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17649)
	; 5886 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17650)
	; 5884 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17652)
	; 5884 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17653)
	; 5888 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17655)
	; 5888 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 17656)
	; 5886 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.endif
.else
.if((n >= 17657) && (n < 17735))
.if((n >= 17657) && (n < 17696))
.if((n >= 17657) && (n < 17676))
; 17657 to 17675
.if((n .mod 3 = 2) && ((n > 17656 && n < 17676)))
	; 5887..5895 bytes
	delay_n (n-2)
	nop
.elseif(n = 17658)
	; 5886 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17659)
	; 5890 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17661)
	; 5890 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17662)
	; 5888 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17664)
	; 5888 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17665)
	; 5892 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17667)
	; 5892 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17668)
	; 5890 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17670)
	; 5890 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17671)
	; 5894 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17673)
	; 5894 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17674)
	; 5892 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 17676 to 17695
.if(n = 17676)
	; 5892 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17677)
	; 5896 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 17677 && n < 17694)))
	; 5893..5901 bytes
	delay_n (n-2)
	nop
.elseif(n = 17679)
	; 5896 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17680)
	; 5894 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17682)
	; 5894 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17683)
	; 5898 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17685)
	; 5898 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17686)
	; 5896 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17688)
	; 5896 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17689)
	; 5900 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17691)
	; 5900 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17692)
	; 5898 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17694)
	; 5898 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17695)
	; 5902 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 17696) && (n < 17715))
; 17696 to 17714
.if((n .mod 3 = 2) && ((n > 17695 && n < 17715)))
	; 5899..5907 bytes
	delay_n (n-2)
	nop
.elseif(n = 17697)
	; 5902 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17698)
	; 5900 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17700)
	; 5900 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17701)
	; 5904 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17703)
	; 5904 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17704)
	; 5902 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17706)
	; 5902 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17707)
	; 5906 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17709)
	; 5906 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5908 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 17715 to 17734
.if(n = 17715)
	; 5908 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17716)
	; 5906 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 17716 && n < 17733)))
	; 5907..5913 bytes
	delay_n (n-2)
	nop
.elseif(n = 17718)
	; 5906 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17719)
	; 5910 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17721)
	; 5910 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17722)
	; 5908 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17724)
	; 5908 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17725)
	; 5912 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17727)
	; 5912 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17728)
	; 5910 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17730)
	; 5910 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17731)
	; 5914 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17733)
	; 5914 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17734)
	; 5912 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.else
.if((n >= 17735) && (n < 17774))
.if((n >= 17735) && (n < 17754))
; 17735 to 17753
.if((n .mod 3 = 2) && ((n > 17734 && n < 17754)))
	; 5913..5921 bytes
	delay_n (n-2)
	nop
.elseif(n = 17736)
	; 5912 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17737)
	; 5916 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17739)
	; 5916 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17740)
	; 5914 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17742)
	; 5914 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17743)
	; 5918 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17745)
	; 5918 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17746)
	; 5916 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17748)
	; 5916 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17749)
	; 5920 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17751)
	; 5920 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17752)
	; 5918 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 17754 to 17773
.if(n = 17754)
	; 5918 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17755)
	; 5922 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 17755 && n < 17772)))
	; 5919..5927 bytes
	delay_n (n-2)
	nop
.elseif(n = 17757)
	; 5922 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17758)
	; 5920 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17760)
	; 5920 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17761)
	; 5924 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17763)
	; 5924 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5926 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17769)
	; 5926 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17770)
	; 5924 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17772)
	; 5924 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17773)
	; 5928 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.else
.if((n >= 17774) && (n < 17793))
; 17774 to 17792
.if((n .mod 3 = 2) && ((n > 17773 && n < 17793)))
	; 5925..5933 bytes
	delay_n (n-2)
	nop
.elseif(n = 17775)
	; 5928 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17776)
	; 5926 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17778)
	; 5926 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17779)
	; 5930 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17781)
	; 5930 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17782)
	; 5928 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17784)
	; 5928 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17785)
	; 5932 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17787)
	; 5932 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17788)
	; 5930 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17790)
	; 5930 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17791)
	; 5934 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 17793 to 17812
.if(n = 17793)
	; 5934 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17794)
	; 5932 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 17794 && n < 17811)))
	; 5933..5939 bytes
	delay_n (n-2)
	nop
.elseif(n = 17796)
	; 5932 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17797)
	; 5936 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17799)
	; 5936 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17800)
	; 5934 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17802)
	; 5934 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17803)
	; 5938 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17805)
	; 5938 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17806)
	; 5936 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17808)
	; 5936 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17809)
	; 5940 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17811)
	; 5940 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17812)
	; 5938 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
.if((n >= 17813) && (n < 17852))
.if((n >= 17813) && (n < 17832))
; 17813 to 17831
.if((n .mod 3 = 2) && ((n > 17812 && n < 17832)))
	; 5939..5947 bytes
	delay_n (n-2)
	nop
.elseif(n = 17814)
	; 5938 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17815)
	; 5942 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17817)
	; 5942 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5944 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17823)
	; 5944 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17824)
	; 5942 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17826)
	; 5942 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17827)
	; 5946 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17829)
	; 5946 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17830)
	; 5944 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 17832 to 17851
.if(n = 17832)
	; 5944 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17833)
	; 5948 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 17833 && n < 17850)))
	; 5945..5953 bytes
	delay_n (n-2)
	nop
.elseif(n = 17835)
	; 5948 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17836)
	; 5946 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17838)
	; 5946 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17839)
	; 5950 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17841)
	; 5950 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17842)
	; 5948 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17844)
	; 5948 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17845)
	; 5952 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17847)
	; 5952 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17848)
	; 5950 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17850)
	; 5950 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17851)
	; 5954 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.else
.if((n >= 17852) && (n < 17871))
; 17852 to 17870
.if((n .mod 3 = 2) && ((n > 17851 && n < 17871)))
	; 5951..5959 bytes
	delay_n (n-2)
	nop
.elseif(n = 17853)
	; 5954 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17854)
	; 5952 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17856)
	; 5952 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17857)
	; 5956 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17859)
	; 5956 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17860)
	; 5954 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17862)
	; 5954 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17863)
	; 5958 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17865)
	; 5958 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17866)
	; 5956 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17868)
	; 5956 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17869)
	; 5960 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 17871 to 17890
.if(n = 17871)
	; 5960 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 17872)
	; 5958 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 17872 && n < 17889)))
	; 5959..5965 bytes
	delay_n (n-2)
	nop
.elseif(n = 17874)
	; 5958 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 17875)
	; 5962 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17877)
	; 5962 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17878)
	; 5960 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17880)
	; 5960 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17881)
	; 5964 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17883)
	; 5964 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17884)
	; 5962 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17886)
	; 5962 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17887)
	; 5966 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17889)
	; 5966 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17890)
	; 5964 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.endif
.else
.if((n >= 17891) && (n < 17930))
.if((n >= 17891) && (n < 17910))
; 17891 to 17909
.if((n .mod 3 = 2) && ((n > 17890 && n < 17910)))
	; 5965..5973 bytes
	delay_n (n-2)
	nop
.elseif(n = 17892)
	; 5964 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17893)
	; 5968 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17895)
	; 5968 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17896)
	; 5966 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17898)
	; 5966 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17899)
	; 5970 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17901)
	; 5970 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17902)
	; 5968 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17904)
	; 5968 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17905)
	; 5972 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17907)
	; 5972 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17908)
	; 5970 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 17910 to 17929
.if(n = 17910)
	; 5970 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17911)
	; 5974 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 17911 && n < 17928)))
	; 5971..5979 bytes
	delay_n (n-2)
	nop
.elseif(n = 17913)
	; 5974 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17914)
	; 5972 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 17916)
	; 5972 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17917)
	; 5976 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17919)
	; 5976 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17920)
	; 5974 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17922)
	; 5974 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17923)
	; 5978 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17925)
	; 5978 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5980 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 17930) && (n < 17949))
; 17930 to 17948
.if((n .mod 3 = 2) && ((n > 17929 && n < 17949)))
	; 5977..5985 bytes
	delay_n (n-2)
	nop
.elseif(n = 17931)
	; 5980 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17932)
	; 5978 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 17934)
	; 5978 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17935)
	; 5982 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 17937)
	; 5982 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17938)
	; 5980 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17940)
	; 5980 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17941)
	; 5984 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17943)
	; 5984 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17944)
	; 5982 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 17946)
	; 5982 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 17947)
	; 5986 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 17949 to 17968
.if(n = 17949)
	; 5986 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 17950)
	; 5984 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 17950 && n < 17967)))
	; 5985..5991 bytes
	delay_n (n-2)
	nop
.elseif(n = 17952)
	; 5984 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 17953)
	; 5988 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 17955)
	; 5988 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17956)
	; 5986 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 17958)
	; 5986 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 17959)
	; 5990 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 17961)
	; 5990 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17962)
	; 5988 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17964)
	; 5988 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17965)
	; 5992 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17967)
	; 5992 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 17968)
	; 5990 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.endif
.endif
.else
.if((n >= 17969) && (n < 18047))
.if((n >= 17969) && (n < 18008))
.if((n >= 17969) && (n < 17988))
; 17969 to 17987
.if((n .mod 3 = 2) && ((n > 17968 && n < 17988)))
	; 5991..5999 bytes
	delay_n (n-2)
	nop
.elseif(n = 17970)
	; 5990 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17971)
	; 5994 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17973)
	; 5994 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 17974)
	; 5992 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17976)
	; 5992 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 17977)
	; 5996 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 17979)
	; 5996 bytes
	delay_n 105
	delay_n (n-105)
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
	; 5998 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17985)
	; 5998 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17986)
	; 5996 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 17988 to 18007
.if(n = 17988)
	; 5996 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 17989)
	; 6000 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 17989 && n < 18006)))
	; 5997..6005 bytes
	delay_n (n-2)
	nop
.elseif(n = 17991)
	; 6000 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 17992)
	; 5998 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17994)
	; 5998 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17995)
	; 6002 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 17997)
	; 6002 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17998)
	; 6000 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18000)
	; 6000 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18001)
	; 6004 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18003)
	; 6004 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18004)
	; 6002 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18006)
	; 6002 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18007)
	; 6006 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 18008) && (n < 18027))
; 18008 to 18026
.if((n .mod 3 = 2) && ((n > 18007 && n < 18027)))
	; 6003..6011 bytes
	delay_n (n-2)
	nop
.elseif(n = 18009)
	; 6006 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18010)
	; 6004 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18012)
	; 6004 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18013)
	; 6008 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18015)
	; 6008 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18016)
	; 6006 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18018)
	; 6006 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18019)
	; 6010 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18021)
	; 6010 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18022)
	; 6008 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18024)
	; 6008 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18025)
	; 6012 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 18027 to 18046
.if(n = 18027)
	; 6012 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18028)
	; 6010 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 18028 && n < 18045)))
	; 6011..6017 bytes
	delay_n (n-2)
	nop
.elseif(n = 18030)
	; 6010 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18031)
	; 6014 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18033)
	; 6014 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6016 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18039)
	; 6016 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18040)
	; 6014 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18042)
	; 6014 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18043)
	; 6018 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18045)
	; 6018 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18046)
	; 6016 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.else
.if((n >= 18047) && (n < 18086))
.if((n >= 18047) && (n < 18066))
; 18047 to 18065
.if((n .mod 3 = 2) && ((n > 18046 && n < 18066)))
	; 6017..6025 bytes
	delay_n (n-2)
	nop
.elseif(n = 18048)
	; 6016 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18049)
	; 6020 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18051)
	; 6020 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18052)
	; 6018 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18054)
	; 6018 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18055)
	; 6022 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18057)
	; 6022 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18058)
	; 6020 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18060)
	; 6020 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18061)
	; 6024 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18063)
	; 6024 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18064)
	; 6022 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 18066 to 18085
.if(n = 18066)
	; 6022 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18067)
	; 6026 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 18067 && n < 18084)))
	; 6023..6031 bytes
	delay_n (n-2)
	nop
.elseif(n = 18069)
	; 6026 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18070)
	; 6024 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18072)
	; 6024 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18073)
	; 6028 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18075)
	; 6028 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18076)
	; 6026 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18078)
	; 6026 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18079)
	; 6030 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18081)
	; 6030 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18082)
	; 6028 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18084)
	; 6028 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18085)
	; 6032 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.else
.if((n >= 18086) && (n < 18106))
; 18086 to 18105
.if((n .mod 3 = 2) && ((n > 18085 && n < 18105)))
	; 6029..6037 bytes
	delay_n (n-2)
	nop
.elseif(n = 18087)
	; 6032 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6034 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18093)
	; 6034 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18094)
	; 6032 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18096)
	; 6032 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18097)
	; 6036 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18099)
	; 6036 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18100)
	; 6034 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18102)
	; 6034 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18103)
	; 6038 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18105)
	; 6038 bytes
	delay_n 69
	delay_n (n-69)
.endif
.else
; 18106 to 18125
.if(n = 18106)
	; 6036 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 18106 && n < 18126)))
	; 6037..6045 bytes
	delay_n (n-2)
	nop
.elseif(n = 18108)
	; 6036 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18109)
	; 6040 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18111)
	; 6040 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18112)
	; 6038 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18114)
	; 6038 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18115)
	; 6042 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18117)
	; 6042 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18118)
	; 6040 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18120)
	; 6040 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18121)
	; 6044 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18123)
	; 6044 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18124)
	; 6042 bytes
	delay_n 34
	delay_n (n-34)
.endif
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
.if((n >= 18126) && (n < 18145))
; 18126 to 18144
.if(n = 18126)
	; 6042 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18127)
	; 6046 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 18127 && n < 18144)))
	; 6043..6051 bytes
	delay_n (n-2)
	nop
.elseif(n = 18129)
	; 6046 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18130)
	; 6044 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18132)
	; 6044 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18133)
	; 6048 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18135)
	; 6048 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18136)
	; 6046 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18138)
	; 6046 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18139)
	; 6050 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18141)
	; 6050 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 18142)
	; 6048 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18144)
	; 6048 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 18145 to 18164
.if(n = 18145)
	; 6052 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 18145 && n < 18165)))
	; 6049..6057 bytes
	delay_n (n-2)
	nop
.elseif(n = 18147)
	; 6052 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18148)
	; 6050 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18150)
	; 6050 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18151)
	; 6054 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18153)
	; 6054 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18154)
	; 6052 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18156)
	; 6052 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18157)
	; 6056 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18159)
	; 6056 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18160)
	; 6054 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18162)
	; 6054 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18163)
	; 6058 bytes
	delay_n 127
	delay_n (n-127)
.endif
.endif
.else
.if((n >= 18165) && (n < 18184))
; 18165 to 18183
.if(n = 18165)
	; 6058 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18166)
	; 6056 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 18166 && n < 18183)))
	; 6057..6063 bytes
	delay_n (n-2)
	nop
.elseif(n = 18168)
	; 6056 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18169)
	; 6060 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18171)
	; 6060 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18172)
	; 6058 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18174)
	; 6058 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18175)
	; 6062 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18177)
	; 6062 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18178)
	; 6060 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18180)
	; 6060 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18181)
	; 6064 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18183)
	; 6064 bytes
	delay_n 93
	delay_n (n-93)
.endif
.else
; 18184 to 18203
.if(n = 18184)
	; 6062 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 18184 && n < 18204)))
	; 6063..6071 bytes
	delay_n (n-2)
	nop
.elseif(n = 18186)
	; 6062 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18187)
	; 6066 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18189)
	; 6066 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18190)
	; 6064 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18192)
	; 6064 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18193)
	; 6068 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18195)
	; 6068 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6070 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18201)
	; 6070 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18202)
	; 6068 bytes
	delay_n 58
	delay_n (n-58)
.endif
.endif
.endif
.else
.if((n >= 18204) && (n < 18243))
.if((n >= 18204) && (n < 18223))
; 18204 to 18222
.if(n = 18204)
	; 6068 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18205)
	; 6072 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 18205 && n < 18222)))
	; 6069..6077 bytes
	delay_n (n-2)
	nop
.elseif(n = 18207)
	; 6072 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18208)
	; 6070 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18210)
	; 6070 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18211)
	; 6074 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18213)
	; 6074 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18214)
	; 6072 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18216)
	; 6072 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18217)
	; 6076 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18219)
	; 6076 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18220)
	; 6074 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18222)
	; 6074 bytes
	delay_n 24
	delay_n (n-24)
.endif
.else
; 18223 to 18242
.if(n = 18223)
	; 6078 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 18223 && n < 18243)))
	; 6075..6083 bytes
	delay_n (n-2)
	nop
.elseif(n = 18225)
	; 6078 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18226)
	; 6076 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18228)
	; 6076 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18229)
	; 6080 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18231)
	; 6080 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18232)
	; 6078 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18234)
	; 6078 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18235)
	; 6082 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18237)
	; 6082 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18238)
	; 6080 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18240)
	; 6080 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18241)
	; 6084 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 18243) && (n < 18262))
; 18243 to 18261
.if(n = 18243)
	; 6084 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18244)
	; 6082 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 18244 && n < 18261)))
	; 6083..6089 bytes
	delay_n (n-2)
	nop
.elseif(n = 18246)
	; 6082 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18247)
	; 6086 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18249)
	; 6086 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6088 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18255)
	; 6088 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18256)
	; 6086 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18258)
	; 6086 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18259)
	; 6090 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18261)
	; 6090 bytes
	delay_n 117
	delay_n (n-117)
.endif
.else
; 18262 to 18281
.if(n = 18262)
	; 6088 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 18262 && n < 18282)))
	; 6089..6097 bytes
	delay_n (n-2)
	nop
.elseif(n = 18264)
	; 6088 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18265)
	; 6092 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18267)
	; 6092 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18268)
	; 6090 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18270)
	; 6090 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18271)
	; 6094 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18273)
	; 6094 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18274)
	; 6092 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18276)
	; 6092 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18277)
	; 6096 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18279)
	; 6096 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18280)
	; 6094 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
.if((n >= 18282) && (n < 18321))
.if((n >= 18282) && (n < 18301))
; 18282 to 18300
.if(n = 18282)
	; 6094 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18283)
	; 6098 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 18283 && n < 18300)))
	; 6095..6103 bytes
	delay_n (n-2)
	nop
.elseif(n = 18285)
	; 6098 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18286)
	; 6096 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18288)
	; 6096 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18289)
	; 6100 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18291)
	; 6100 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18292)
	; 6098 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18294)
	; 6098 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18295)
	; 6102 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18297)
	; 6102 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18298)
	; 6100 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18300)
	; 6100 bytes
	delay_n 48
	delay_n (n-48)
.endif
.else
; 18301 to 18320
.if(n = 18301)
	; 6104 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 18301 && n < 18321)))
	; 6101..6109 bytes
	delay_n (n-2)
	nop
.elseif(n = 18303)
	; 6104 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6106 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18309)
	; 6106 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18310)
	; 6104 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18312)
	; 6104 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18313)
	; 6108 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18315)
	; 6108 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18316)
	; 6106 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18318)
	; 6106 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18319)
	; 6110 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.else
.if((n >= 18321) && (n < 18340))
; 18321 to 18339
.if(n = 18321)
	; 6110 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18322)
	; 6108 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 18322 && n < 18339)))
	; 6109..6115 bytes
	delay_n (n-2)
	nop
.elseif(n = 18324)
	; 6108 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18325)
	; 6112 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18327)
	; 6112 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18328)
	; 6110 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18330)
	; 6110 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18331)
	; 6114 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18333)
	; 6114 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18334)
	; 6112 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18336)
	; 6112 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18337)
	; 6116 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18339)
	; 6116 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 18340 to 18359
.if(n = 18340)
	; 6114 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 18340 && n < 18360)))
	; 6115..6123 bytes
	delay_n (n-2)
	nop
.elseif(n = 18342)
	; 6114 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18343)
	; 6118 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18345)
	; 6118 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18346)
	; 6116 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18348)
	; 6116 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18349)
	; 6120 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18351)
	; 6120 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18352)
	; 6118 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18354)
	; 6118 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18355)
	; 6122 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18357)
	; 6122 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 18358)
	; 6120 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 18360) && (n < 18399))
.if((n >= 18360) && (n < 18379))
; 18360 to 18378
.if(n = 18360)
	; 6120 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18361)
	; 6124 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 18361 && n < 18378)))
	; 6121..6129 bytes
	delay_n (n-2)
	nop
.elseif(n = 18363)
	; 6124 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18364)
	; 6122 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18366)
	; 6122 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18367)
	; 6126 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18369)
	; 6126 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18370)
	; 6124 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18372)
	; 6124 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18373)
	; 6128 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18375)
	; 6128 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18376)
	; 6126 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18378)
	; 6126 bytes
	delay_n 72
	delay_n (n-72)
.endif
.else
; 18379 to 18398
.if(n = 18379)
	; 6130 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 18379 && n < 18399)))
	; 6127..6135 bytes
	delay_n (n-2)
	nop
.elseif(n = 18381)
	; 6130 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18382)
	; 6128 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18384)
	; 6128 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18385)
	; 6132 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18387)
	; 6132 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18388)
	; 6130 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18390)
	; 6130 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18391)
	; 6134 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18393)
	; 6134 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18394)
	; 6132 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18396)
	; 6132 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18397)
	; 6136 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 18399) && (n < 18418))
; 18399 to 18417
.if(n = 18399)
	; 6136 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18400)
	; 6134 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 18400 && n < 18417)))
	; 6135..6141 bytes
	delay_n (n-2)
	nop
.elseif(n = 18402)
	; 6134 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18403)
	; 6138 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18405)
	; 6138 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18406)
	; 6136 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18408)
	; 6136 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18409)
	; 6140 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18411)
	; 6140 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6142 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18417)
	; 6142 bytes
	delay_n 57
	delay_n (n-57)
.endif
.else
; 18418 to 18437
.if(n = 18418)
	; 6140 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 18418 && n < 18438)))
	; 6141..6149 bytes
	delay_n (n-2)
	nop
.elseif(n = 18420)
	; 6140 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18421)
	; 6144 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18423)
	; 6144 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18424)
	; 6142 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18426)
	; 6142 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18427)
	; 6146 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18429)
	; 6146 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18430)
	; 6144 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18432)
	; 6144 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18433)
	; 6148 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18435)
	; 6148 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18436)
	; 6146 bytes
	delay_n 22
	delay_n (n-22)
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
	; 6146 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18439)
	; 6150 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 18439 && n < 18456)))
	; 6147..6155 bytes
	delay_n (n-2)
	nop
.elseif(n = 18441)
	; 6150 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18442)
	; 6148 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18444)
	; 6148 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18445)
	; 6152 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18447)
	; 6152 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18448)
	; 6150 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18450)
	; 6150 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18451)
	; 6154 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18453)
	; 6154 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18454)
	; 6152 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18456)
	; 6152 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 18457 to 18476
.if(n = 18457)
	; 6156 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 18457 && n < 18477)))
	; 6153..6161 bytes
	delay_n (n-2)
	nop
.elseif(n = 18459)
	; 6156 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18460)
	; 6154 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18462)
	; 6154 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18463)
	; 6158 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18465)
	; 6158 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6160 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18471)
	; 6160 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18472)
	; 6158 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18474)
	; 6158 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18475)
	; 6162 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.else
.if((n >= 18477) && (n < 18496))
; 18477 to 18495
.if(n = 18477)
	; 6162 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18478)
	; 6160 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 18478 && n < 18495)))
	; 6161..6167 bytes
	delay_n (n-2)
	nop
.elseif(n = 18480)
	; 6160 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18481)
	; 6164 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18483)
	; 6164 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18484)
	; 6162 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18486)
	; 6162 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18487)
	; 6166 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18489)
	; 6166 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18490)
	; 6164 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18492)
	; 6164 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18493)
	; 6168 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18495)
	; 6168 bytes
	delay_n 81
	delay_n (n-81)
.endif
.else
; 18496 to 18515
.if(n = 18496)
	; 6166 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 18496 && n < 18516)))
	; 6167..6175 bytes
	delay_n (n-2)
	nop
.elseif(n = 18498)
	; 6166 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18499)
	; 6170 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18501)
	; 6170 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18502)
	; 6168 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18504)
	; 6168 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18505)
	; 6172 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18507)
	; 6172 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18508)
	; 6170 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18510)
	; 6170 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18511)
	; 6174 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18513)
	; 6174 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18514)
	; 6172 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.else
.if((n >= 18516) && (n < 18555))
.if((n >= 18516) && (n < 18535))
; 18516 to 18534
.if(n = 18516)
	; 6172 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18517)
	; 6176 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 18517 && n < 18534)))
	; 6173..6181 bytes
	delay_n (n-2)
	nop
.elseif(n = 18519)
	; 6176 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6178 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18525)
	; 6178 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18526)
	; 6176 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18528)
	; 6176 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18529)
	; 6180 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18531)
	; 6180 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18532)
	; 6178 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18534)
	; 6178 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 18535 to 18554
.if(n = 18535)
	; 6182 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 18535 && n < 18555)))
	; 6179..6187 bytes
	delay_n (n-2)
	nop
.elseif(n = 18537)
	; 6182 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18538)
	; 6180 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18540)
	; 6180 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18541)
	; 6184 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18543)
	; 6184 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18544)
	; 6182 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18546)
	; 6182 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18547)
	; 6186 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18549)
	; 6186 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18550)
	; 6184 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18552)
	; 6184 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18553)
	; 6188 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.else
.if((n >= 18555) && (n < 18574))
; 18555 to 18573
.if(n = 18555)
	; 6188 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18556)
	; 6186 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 18556 && n < 18573)))
	; 6187..6193 bytes
	delay_n (n-2)
	nop
.elseif(n = 18558)
	; 6186 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18559)
	; 6190 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18561)
	; 6190 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18562)
	; 6188 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18564)
	; 6188 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18565)
	; 6192 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18567)
	; 6192 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18568)
	; 6190 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18570)
	; 6190 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18571)
	; 6194 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18573)
	; 6194 bytes
	delay_n 105
	delay_n (n-105)
.endif
.else
; 18574 to 18593
.if(n = 18574)
	; 6192 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 18574 && n < 18594)))
	; 6193..6201 bytes
	delay_n (n-2)
	nop
.elseif(n = 18576)
	; 6192 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18577)
	; 6196 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18579)
	; 6196 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18580)
	; 6194 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18582)
	; 6194 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18583)
	; 6198 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18585)
	; 6198 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18586)
	; 6196 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18588)
	; 6196 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18589)
	; 6200 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18591)
	; 6200 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18592)
	; 6198 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
.if((n >= 18594) && (n < 18633))
.if((n >= 18594) && (n < 18613))
; 18594 to 18612
.if(n = 18594)
	; 6198 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18595)
	; 6202 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 18595 && n < 18612)))
	; 6199..6207 bytes
	delay_n (n-2)
	nop
.elseif(n = 18597)
	; 6202 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18598)
	; 6200 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18600)
	; 6200 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18601)
	; 6204 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18603)
	; 6204 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18604)
	; 6202 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18606)
	; 6202 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18607)
	; 6206 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18609)
	; 6206 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18610)
	; 6204 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18612)
	; 6204 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 18613 to 18632
.if(n = 18613)
	; 6208 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 18613 && n < 18633)))
	; 6205..6213 bytes
	delay_n (n-2)
	nop
.elseif(n = 18615)
	; 6208 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18616)
	; 6206 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18618)
	; 6206 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18619)
	; 6210 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18621)
	; 6210 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18622)
	; 6208 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18624)
	; 6208 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18625)
	; 6212 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18627)
	; 6212 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6214 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.else
.if((n >= 18633) && (n < 18652))
; 18633 to 18651
.if(n = 18633)
	; 6214 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18634)
	; 6212 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 18634 && n < 18651)))
	; 6213..6219 bytes
	delay_n (n-2)
	nop
.elseif(n = 18636)
	; 6212 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18637)
	; 6216 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18639)
	; 6216 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18640)
	; 6214 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18642)
	; 6214 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18643)
	; 6218 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18645)
	; 6218 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18646)
	; 6216 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18648)
	; 6216 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18649)
	; 6220 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18651)
	; 6220 bytes
	delay_n 129
	delay_n (n-129)
.endif
.else
; 18652 to 18671
.if(n = 18652)
	; 6218 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 18652 && n < 18672)))
	; 6219..6227 bytes
	delay_n (n-2)
	nop
.elseif(n = 18654)
	; 6218 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18655)
	; 6222 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18657)
	; 6222 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18658)
	; 6220 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18660)
	; 6220 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18661)
	; 6224 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18663)
	; 6224 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18664)
	; 6222 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18666)
	; 6222 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18667)
	; 6226 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18669)
	; 6226 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18670)
	; 6224 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.endif
.else
.if((n >= 18672) && (n < 18711))
.if((n >= 18672) && (n < 18691))
; 18672 to 18690
.if(n = 18672)
	; 6224 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18673)
	; 6228 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 18673 && n < 18690)))
	; 6225..6233 bytes
	delay_n (n-2)
	nop
.elseif(n = 18675)
	; 6228 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18676)
	; 6226 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18678)
	; 6226 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18679)
	; 6230 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18681)
	; 6230 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6232 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18687)
	; 6232 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18688)
	; 6230 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18690)
	; 6230 bytes
	delay_n 60
	delay_n (n-60)
.endif
.else
; 18691 to 18710
.if(n = 18691)
	; 6234 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 18691 && n < 18711)))
	; 6231..6239 bytes
	delay_n (n-2)
	nop
.elseif(n = 18693)
	; 6234 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18694)
	; 6232 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18696)
	; 6232 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18697)
	; 6236 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18699)
	; 6236 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18700)
	; 6234 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18702)
	; 6234 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18703)
	; 6238 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18705)
	; 6238 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18706)
	; 6236 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18708)
	; 6236 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18709)
	; 6240 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.else
.if((n >= 18711) && (n < 18731))
; 18711 to 18730
.if(n = 18711)
	; 6240 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18712)
	; 6238 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 18712 && n < 18729)))
	; 6239..6245 bytes
	delay_n (n-2)
	nop
.elseif(n = 18714)
	; 6238 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18715)
	; 6242 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18717)
	; 6242 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18718)
	; 6240 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18720)
	; 6240 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18721)
	; 6244 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18723)
	; 6244 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18724)
	; 6242 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18726)
	; 6242 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18727)
	; 6246 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18729)
	; 6246 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18730)
	; 6244 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 18731 to 18750
.if((n .mod 3 = 2) && ((n > 18730 && n < 18750)))
	; 6245..6253 bytes
	delay_n (n-2)
	nop
.elseif(n = 18732)
	; 6244 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18733)
	; 6248 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18735)
	; 6248 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6250 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18741)
	; 6250 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18742)
	; 6248 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18744)
	; 6248 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18745)
	; 6252 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18747)
	; 6252 bytes
	delay_n 117
	delay_n (n-117)
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
.endif
.else
.if((n >= 18751) && (n < 19376))
.if((n >= 18751) && (n < 19063))
.if((n >= 18751) && (n < 18907))
.if((n >= 18751) && (n < 18829))
.if((n >= 18751) && (n < 18790))
.if((n >= 18751) && (n < 18770))
; 18751 to 18769
.if(n = 18751)
	; 6254 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 18751 && n < 18768)))
	; 6251..6259 bytes
	delay_n (n-2)
	nop
.elseif(n = 18753)
	; 6254 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18754)
	; 6252 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18756)
	; 6252 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18757)
	; 6256 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18759)
	; 6256 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18760)
	; 6254 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18762)
	; 6254 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18763)
	; 6258 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18765)
	; 6258 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18766)
	; 6256 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18768)
	; 6256 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18769)
	; 6260 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 18770 to 18789
.if((n .mod 3 = 2) && ((n > 18769 && n < 18789)))
	; 6257..6265 bytes
	delay_n (n-2)
	nop
.elseif(n = 18771)
	; 6260 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18772)
	; 6258 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18774)
	; 6258 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18775)
	; 6262 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18777)
	; 6262 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18778)
	; 6260 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18780)
	; 6260 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18781)
	; 6264 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18783)
	; 6264 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18784)
	; 6262 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18786)
	; 6262 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18787)
	; 6266 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18789)
	; 6266 bytes
	delay_n 105
	delay_n (n-105)
.endif
.endif
.else
.if((n >= 18790) && (n < 18809))
; 18790 to 18808
.if(n = 18790)
	; 6264 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 18790 && n < 18807)))
	; 6265..6271 bytes
	delay_n (n-2)
	nop
.elseif(n = 18792)
	; 6264 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 18793)
	; 6268 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18795)
	; 6268 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18796)
	; 6266 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18798)
	; 6266 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18799)
	; 6270 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18801)
	; 6270 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18802)
	; 6268 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18804)
	; 6268 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18805)
	; 6272 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18807)
	; 6272 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18808)
	; 6270 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 18809 to 18828
.if((n .mod 3 = 2) && ((n > 18808 && n < 18828)))
	; 6271..6279 bytes
	delay_n (n-2)
	nop
.elseif(n = 18810)
	; 6270 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18811)
	; 6274 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18813)
	; 6274 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18814)
	; 6272 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18816)
	; 6272 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18817)
	; 6276 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18819)
	; 6276 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18820)
	; 6274 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18822)
	; 6274 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18823)
	; 6278 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18825)
	; 6278 bytes
	delay_n 141
	delay_n (n-141)
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
.endif
.else
.if((n >= 18829) && (n < 18868))
.if((n >= 18829) && (n < 18848))
; 18829 to 18847
.if(n = 18829)
	; 6280 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 18829 && n < 18846)))
	; 6277..6285 bytes
	delay_n (n-2)
	nop
.elseif(n = 18831)
	; 6280 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18832)
	; 6278 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18834)
	; 6278 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18835)
	; 6282 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18837)
	; 6282 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18838)
	; 6280 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18840)
	; 6280 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18841)
	; 6284 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18843)
	; 6284 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6286 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 18848 to 18867
.if((n .mod 3 = 2) && ((n > 18847 && n < 18867)))
	; 6283..6291 bytes
	delay_n (n-2)
	nop
.elseif(n = 18849)
	; 6286 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18850)
	; 6284 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18852)
	; 6284 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18853)
	; 6288 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18855)
	; 6288 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18856)
	; 6286 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18858)
	; 6286 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18859)
	; 6290 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18861)
	; 6290 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18862)
	; 6288 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18864)
	; 6288 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18865)
	; 6292 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18867)
	; 6292 bytes
	delay_n 129
	delay_n (n-129)
.endif
.endif
.else
.if((n >= 18868) && (n < 18887))
; 18868 to 18886
.if(n = 18868)
	; 6290 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 18868 && n < 18885)))
	; 6291..6297 bytes
	delay_n (n-2)
	nop
.elseif(n = 18870)
	; 6290 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18871)
	; 6294 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18873)
	; 6294 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18874)
	; 6292 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18876)
	; 6292 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18877)
	; 6296 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18879)
	; 6296 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18880)
	; 6294 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18882)
	; 6294 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18883)
	; 6298 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18885)
	; 6298 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18886)
	; 6296 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 18887 to 18906
.if((n .mod 3 = 2) && ((n > 18886 && n < 18906)))
	; 6297..6305 bytes
	delay_n (n-2)
	nop
.elseif(n = 18888)
	; 6296 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18889)
	; 6300 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18891)
	; 6300 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 18892)
	; 6298 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18894)
	; 6298 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18895)
	; 6302 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18897)
	; 6302 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6304 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18903)
	; 6304 bytes
	delay_n 57
	delay_n (n-57)
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
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
.if((n >= 18907) && (n < 18926))
; 18907 to 18925
.if(n = 18907)
	; 6306 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 18907 && n < 18924)))
	; 6303..6311 bytes
	delay_n (n-2)
	nop
.elseif(n = 18909)
	; 6306 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18910)
	; 6304 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18912)
	; 6304 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18913)
	; 6308 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18915)
	; 6308 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18916)
	; 6306 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18918)
	; 6306 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18919)
	; 6310 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18921)
	; 6310 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18922)
	; 6308 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18924)
	; 6308 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18925)
	; 6312 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 18926 to 18945
.if((n .mod 3 = 2) && ((n > 18925 && n < 18945)))
	; 6309..6317 bytes
	delay_n (n-2)
	nop
.elseif(n = 18927)
	; 6312 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18928)
	; 6310 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 18930)
	; 6310 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 18931)
	; 6314 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 18933)
	; 6314 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18934)
	; 6312 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18936)
	; 6312 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18937)
	; 6316 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18939)
	; 6316 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18940)
	; 6314 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18942)
	; 6314 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18943)
	; 6318 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18945)
	; 6318 bytes
	delay_n 45
	delay_n (n-45)
.endif
.endif
.else
.if((n >= 18946) && (n < 18965))
; 18946 to 18964
.if(n = 18946)
	; 6316 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 18946 && n < 18963)))
	; 6317..6323 bytes
	delay_n (n-2)
	nop
.elseif(n = 18948)
	; 6316 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 18949)
	; 6320 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 18951)
	; 6320 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6322 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18957)
	; 6322 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18958)
	; 6320 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 18960)
	; 6320 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 18961)
	; 6324 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 18963)
	; 6324 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 18964)
	; 6322 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 18965 to 18984
.if((n .mod 3 = 2) && ((n > 18964 && n < 18984)))
	; 6323..6331 bytes
	delay_n (n-2)
	nop
.elseif(n = 18966)
	; 6322 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18967)
	; 6326 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 18969)
	; 6326 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18970)
	; 6324 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 18972)
	; 6324 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 18973)
	; 6328 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18975)
	; 6328 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 18976)
	; 6326 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18978)
	; 6326 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 18979)
	; 6330 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 18981)
	; 6330 bytes
	delay_n 81
	delay_n (n-81)
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
.endif
.else
.if((n >= 18985) && (n < 19024))
.if((n >= 18985) && (n < 19004))
; 18985 to 19003
.if(n = 18985)
	; 6332 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 18985 && n < 19002)))
	; 6329..6337 bytes
	delay_n (n-2)
	nop
.elseif(n = 18987)
	; 6332 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18988)
	; 6330 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 18990)
	; 6330 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18991)
	; 6334 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18993)
	; 6334 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 18994)
	; 6332 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 18996)
	; 6332 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18997)
	; 6336 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18999)
	; 6336 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19000)
	; 6334 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19002)
	; 6334 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19003)
	; 6338 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 19004 to 19023
.if((n .mod 3 = 2) && ((n > 19003 && n < 19023)))
	; 6335..6343 bytes
	delay_n (n-2)
	nop
.elseif(n = 19005)
	; 6338 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6340 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19011)
	; 6340 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19012)
	; 6338 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19014)
	; 6338 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19015)
	; 6342 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19017)
	; 6342 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19018)
	; 6340 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19020)
	; 6340 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19021)
	; 6344 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19023)
	; 6344 bytes
	delay_n 69
	delay_n (n-69)
.endif
.endif
.else
.if((n >= 19024) && (n < 19043))
; 19024 to 19042
.if(n = 19024)
	; 6342 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 19024 && n < 19041)))
	; 6343..6349 bytes
	delay_n (n-2)
	nop
.elseif(n = 19026)
	; 6342 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19027)
	; 6346 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19029)
	; 6346 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19030)
	; 6344 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19032)
	; 6344 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19033)
	; 6348 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19035)
	; 6348 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19036)
	; 6346 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19038)
	; 6346 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19039)
	; 6350 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19041)
	; 6350 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19042)
	; 6348 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 19043 to 19062
.if((n .mod 3 = 2) && ((n > 19042 && n < 19062)))
	; 6349..6357 bytes
	delay_n (n-2)
	nop
.elseif(n = 19044)
	; 6348 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19045)
	; 6352 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19047)
	; 6352 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19048)
	; 6350 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19050)
	; 6350 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19051)
	; 6354 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19053)
	; 6354 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19054)
	; 6352 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19056)
	; 6352 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19057)
	; 6356 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19059)
	; 6356 bytes
	delay_n 105
	delay_n (n-105)
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
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
.if((n >= 19063) && (n < 19102))
.if((n >= 19063) && (n < 19082))
; 19063 to 19081
.if(n = 19063)
	; 6358 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 19063 && n < 19080)))
	; 6355..6363 bytes
	delay_n (n-2)
	nop
.elseif(n = 19065)
	; 6358 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19066)
	; 6356 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19068)
	; 6356 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19069)
	; 6360 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19071)
	; 6360 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19072)
	; 6358 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19074)
	; 6358 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19075)
	; 6362 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19077)
	; 6362 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19078)
	; 6360 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19080)
	; 6360 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19081)
	; 6364 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 19082 to 19101
.if((n .mod 3 = 2) && ((n > 19081 && n < 19101)))
	; 6361..6369 bytes
	delay_n (n-2)
	nop
.elseif(n = 19083)
	; 6364 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19084)
	; 6362 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19086)
	; 6362 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19087)
	; 6366 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19089)
	; 6366 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19090)
	; 6364 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19092)
	; 6364 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19093)
	; 6368 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19095)
	; 6368 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19096)
	; 6366 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19098)
	; 6366 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19099)
	; 6370 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19101)
	; 6370 bytes
	delay_n 93
	delay_n (n-93)
.endif
.endif
.else
.if((n >= 19102) && (n < 19121))
; 19102 to 19120
.if(n = 19102)
	; 6368 bytes
	delay_n 94
	delay_n (n-94)
.elseif((n .mod 3 = 2) && ((n > 19102 && n < 19119)))
	; 6369..6375 bytes
	delay_n (n-2)
	nop
.elseif(n = 19104)
	; 6368 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19105)
	; 6372 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19107)
	; 6372 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19108)
	; 6370 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19110)
	; 6370 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19111)
	; 6374 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19113)
	; 6374 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6376 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19119)
	; 6376 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19120)
	; 6374 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 19121 to 19140
.if((n .mod 3 = 2) && ((n > 19120 && n < 19140)))
	; 6375..6383 bytes
	delay_n (n-2)
	nop
.elseif(n = 19122)
	; 6374 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19123)
	; 6378 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19125)
	; 6378 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19126)
	; 6376 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19128)
	; 6376 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19129)
	; 6380 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19131)
	; 6380 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19132)
	; 6378 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19134)
	; 6378 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19135)
	; 6382 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19137)
	; 6382 bytes
	delay_n 129
	delay_n (n-129)
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
.endif
.else
.if((n >= 19141) && (n < 19180))
.if((n >= 19141) && (n < 19160))
; 19141 to 19159
.if(n = 19141)
	; 6384 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 19141 && n < 19158)))
	; 6381..6389 bytes
	delay_n (n-2)
	nop
.elseif(n = 19143)
	; 6384 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19144)
	; 6382 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19146)
	; 6382 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19147)
	; 6386 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19149)
	; 6386 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19150)
	; 6384 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19152)
	; 6384 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19153)
	; 6388 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19155)
	; 6388 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19156)
	; 6386 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19158)
	; 6386 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19159)
	; 6390 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 19160 to 19179
.if((n .mod 3 = 2) && ((n > 19159 && n < 19179)))
	; 6387..6395 bytes
	delay_n (n-2)
	nop
.elseif(n = 19161)
	; 6390 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19162)
	; 6388 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19164)
	; 6388 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19165)
	; 6392 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19167)
	; 6392 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6394 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19173)
	; 6394 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19174)
	; 6392 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19176)
	; 6392 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19177)
	; 6396 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19179)
	; 6396 bytes
	delay_n 117
	delay_n (n-117)
.endif
.endif
.else
.if((n >= 19180) && (n < 19199))
; 19180 to 19198
.if(n = 19180)
	; 6394 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 19180 && n < 19197)))
	; 6395..6401 bytes
	delay_n (n-2)
	nop
.elseif(n = 19182)
	; 6394 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19183)
	; 6398 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19185)
	; 6398 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19186)
	; 6396 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19188)
	; 6396 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19189)
	; 6400 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19191)
	; 6400 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19192)
	; 6398 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19194)
	; 6398 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19195)
	; 6402 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19197)
	; 6402 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19198)
	; 6400 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 19199 to 19218
.if((n .mod 3 = 2) && ((n > 19198 && n < 19218)))
	; 6401..6409 bytes
	delay_n (n-2)
	nop
.elseif(n = 19200)
	; 6400 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19201)
	; 6404 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19203)
	; 6404 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19204)
	; 6402 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19206)
	; 6402 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19207)
	; 6406 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19209)
	; 6406 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19210)
	; 6404 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19212)
	; 6404 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19213)
	; 6408 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19215)
	; 6408 bytes
	delay_n 45
	delay_n (n-45)
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
.endif
.else
.if((n >= 19219) && (n < 19297))
.if((n >= 19219) && (n < 19258))
.if((n >= 19219) && (n < 19238))
; 19219 to 19237
.if(n = 19219)
	; 6410 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 19219 && n < 19236)))
	; 6407..6415 bytes
	delay_n (n-2)
	nop
.elseif(n = 19221)
	; 6410 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6412 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19227)
	; 6412 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19228)
	; 6410 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19230)
	; 6410 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19231)
	; 6414 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19233)
	; 6414 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19234)
	; 6412 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19236)
	; 6412 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19237)
	; 6416 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 19238 to 19257
.if((n .mod 3 = 2) && ((n > 19237 && n < 19257)))
	; 6413..6421 bytes
	delay_n (n-2)
	nop
.elseif(n = 19239)
	; 6416 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19240)
	; 6414 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19242)
	; 6414 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19243)
	; 6418 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19245)
	; 6418 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19246)
	; 6416 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19248)
	; 6416 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19249)
	; 6420 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19251)
	; 6420 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19252)
	; 6418 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19254)
	; 6418 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19255)
	; 6422 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19257)
	; 6422 bytes
	delay_n 141
	delay_n (n-141)
.endif
.endif
.else
.if((n >= 19258) && (n < 19277))
; 19258 to 19276
.if(n = 19258)
	; 6420 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 19258 && n < 19275)))
	; 6421..6427 bytes
	delay_n (n-2)
	nop
.elseif(n = 19260)
	; 6420 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19261)
	; 6424 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19263)
	; 6424 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19264)
	; 6422 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19266)
	; 6422 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19267)
	; 6426 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19269)
	; 6426 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19270)
	; 6424 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19272)
	; 6424 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19273)
	; 6428 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19275)
	; 6428 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 19276)
	; 6426 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 19277 to 19296
.if((n .mod 3 = 2) && ((n > 19276 && n < 19296)))
	; 6427..6435 bytes
	delay_n (n-2)
	nop
.elseif(n = 19278)
	; 6426 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19279)
	; 6430 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19281)
	; 6430 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19282)
	; 6428 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19284)
	; 6428 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19285)
	; 6432 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19287)
	; 6432 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19288)
	; 6430 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19290)
	; 6430 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19291)
	; 6434 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19293)
	; 6434 bytes
	delay_n 69
	delay_n (n-69)
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
.endif
.else
.if((n >= 19297) && (n < 19336))
.if((n >= 19297) && (n < 19316))
; 19297 to 19315
.if(n = 19297)
	; 6436 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 19297 && n < 19314)))
	; 6433..6441 bytes
	delay_n (n-2)
	nop
.elseif(n = 19299)
	; 6436 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19300)
	; 6434 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19302)
	; 6434 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19303)
	; 6438 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19305)
	; 6438 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19306)
	; 6436 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19308)
	; 6436 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19309)
	; 6440 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19311)
	; 6440 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19312)
	; 6438 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19314)
	; 6438 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19315)
	; 6442 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 19316 to 19335
.if((n .mod 3 = 2) && ((n > 19315 && n < 19335)))
	; 6439..6447 bytes
	delay_n (n-2)
	nop
.elseif(n = 19317)
	; 6442 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19318)
	; 6440 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19320)
	; 6440 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19321)
	; 6444 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19323)
	; 6444 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19324)
	; 6442 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19326)
	; 6442 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19327)
	; 6446 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19329)
	; 6446 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6448 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19335)
	; 6448 bytes
	delay_n 57
	delay_n (n-57)
.endif
.endif
.else
.if((n >= 19336) && (n < 19356))
; 19336 to 19355
.if(n = 19336)
	; 6446 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 19336 && n < 19356)))
	; 6447..6455 bytes
	delay_n (n-2)
	nop
.elseif(n = 19338)
	; 6446 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19339)
	; 6450 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19341)
	; 6450 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19342)
	; 6448 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19344)
	; 6448 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19345)
	; 6452 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19347)
	; 6452 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19348)
	; 6450 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19350)
	; 6450 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19351)
	; 6454 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19353)
	; 6454 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19354)
	; 6452 bytes
	delay_n 22
	delay_n (n-22)
.endif
.else
; 19356 to 19375
.if(n = 19356)
	; 6452 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19357)
	; 6456 bytes
	delay_n 79
	delay_n (n-79)
.elseif((n .mod 3 = 2) && ((n > 19357 && n < 19374)))
	; 6453..6461 bytes
	delay_n (n-2)
	nop
.elseif(n = 19359)
	; 6456 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19360)
	; 6454 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19362)
	; 6454 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19363)
	; 6458 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19365)
	; 6458 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19366)
	; 6456 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19368)
	; 6456 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19369)
	; 6460 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19371)
	; 6460 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19372)
	; 6458 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19374)
	; 6458 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19375)
	; 6462 bytes
	delay_n 151
	delay_n (n-151)
.endif
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
.if((n >= 19376) && (n < 19395))
; 19376 to 19394
.if((n .mod 3 = 2) && ((n > 19375 && n < 19395)))
	; 6459..6467 bytes
	delay_n (n-2)
	nop
.elseif(n = 19377)
	; 6462 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19378)
	; 6460 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19380)
	; 6460 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19381)
	; 6464 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19383)
	; 6464 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6466 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19389)
	; 6466 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19390)
	; 6464 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19392)
	; 6464 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19393)
	; 6468 bytes
	delay_n 115
	delay_n (n-115)
.endif
.else
; 19395 to 19414
.if(n = 19395)
	; 6468 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19396)
	; 6466 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 3 = 2) && ((n > 19396 && n < 19413)))
	; 6467..6473 bytes
	delay_n (n-2)
	nop
.elseif(n = 19398)
	; 6466 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19399)
	; 6470 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19401)
	; 6470 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19402)
	; 6468 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19404)
	; 6468 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19405)
	; 6472 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19407)
	; 6472 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19408)
	; 6470 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19410)
	; 6470 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19411)
	; 6474 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19413)
	; 6474 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19414)
	; 6472 bytes
	delay_n 82
	delay_n (n-82)
.endif
.endif
.else
.if((n >= 19415) && (n < 19434))
; 19415 to 19433
.if((n .mod 3 = 2) && ((n > 19414 && n < 19434)))
	; 6473..6481 bytes
	delay_n (n-2)
	nop
.elseif(n = 19416)
	; 6472 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19417)
	; 6476 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19419)
	; 6476 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19420)
	; 6474 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19422)
	; 6474 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19423)
	; 6478 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19425)
	; 6478 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19426)
	; 6476 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19428)
	; 6476 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19429)
	; 6480 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19431)
	; 6480 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19432)
	; 6478 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 19434 to 19453
.if(n = 19434)
	; 6478 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19435)
	; 6482 bytes
	delay_n 103
	delay_n (n-103)
.elseif((n .mod 3 = 2) && ((n > 19435 && n < 19452)))
	; 6479..6487 bytes
	delay_n (n-2)
	nop
.elseif(n = 19437)
	; 6482 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6484 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19443)
	; 6484 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19444)
	; 6482 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19446)
	; 6482 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19447)
	; 6486 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19449)
	; 6486 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19450)
	; 6484 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19452)
	; 6484 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19453)
	; 6488 bytes
	delay_n 67
	delay_n (n-67)
.endif
.endif
.endif
.else
.if((n >= 19454) && (n < 19493))
.if((n >= 19454) && (n < 19473))
; 19454 to 19472
.if((n .mod 3 = 2) && ((n > 19453 && n < 19473)))
	; 6485..6493 bytes
	delay_n (n-2)
	nop
.elseif(n = 19455)
	; 6488 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19456)
	; 6486 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19458)
	; 6486 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19459)
	; 6490 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19461)
	; 6490 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19462)
	; 6488 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19464)
	; 6488 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19465)
	; 6492 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19467)
	; 6492 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19468)
	; 6490 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19470)
	; 6490 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19471)
	; 6494 bytes
	delay_n 139
	delay_n (n-139)
.endif
.else
; 19473 to 19492
.if(n = 19473)
	; 6494 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19474)
	; 6492 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 3 = 2) && ((n > 19474 && n < 19491)))
	; 6493..6499 bytes
	delay_n (n-2)
	nop
.elseif(n = 19476)
	; 6492 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19477)
	; 6496 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19479)
	; 6496 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19480)
	; 6494 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19482)
	; 6494 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19483)
	; 6498 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19485)
	; 6498 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19486)
	; 6496 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19488)
	; 6496 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19489)
	; 6500 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19491)
	; 6500 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 19492)
	; 6498 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 19493) && (n < 19512))
; 19493 to 19511
.if((n .mod 3 = 2) && ((n > 19492 && n < 19512)))
	; 6499..6507 bytes
	delay_n (n-2)
	nop
.elseif(n = 19494)
	; 6498 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19495)
	; 6502 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19497)
	; 6502 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19498)
	; 6500 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19500)
	; 6500 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19501)
	; 6504 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19503)
	; 6504 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19504)
	; 6502 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19506)
	; 6502 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19507)
	; 6506 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19509)
	; 6506 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19510)
	; 6504 bytes
	delay_n 70
	delay_n (n-70)
.endif
.else
; 19512 to 19531
.if(n = 19512)
	; 6504 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19513)
	; 6508 bytes
	delay_n 127
	delay_n (n-127)
.elseif((n .mod 3 = 2) && ((n > 19513 && n < 19530)))
	; 6505..6513 bytes
	delay_n (n-2)
	nop
.elseif(n = 19515)
	; 6508 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19516)
	; 6506 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19518)
	; 6506 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19519)
	; 6510 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19521)
	; 6510 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19522)
	; 6508 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19524)
	; 6508 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19525)
	; 6512 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19527)
	; 6512 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19528)
	; 6510 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19530)
	; 6510 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19531)
	; 6514 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.endif
.else
.if((n >= 19532) && (n < 19610))
.if((n >= 19532) && (n < 19571))
.if((n >= 19532) && (n < 19551))
; 19532 to 19550
.if((n .mod 3 = 2) && ((n > 19531 && n < 19551)))
	; 6511..6519 bytes
	delay_n (n-2)
	nop
.elseif(n = 19533)
	; 6514 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19534)
	; 6512 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19536)
	; 6512 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19537)
	; 6516 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19539)
	; 6516 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19540)
	; 6514 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19542)
	; 6514 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19543)
	; 6518 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19545)
	; 6518 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6520 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 19551 to 19570
.if(n = 19551)
	; 6520 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19552)
	; 6518 bytes
	delay_n 58
	delay_n (n-58)
.elseif((n .mod 3 = 2) && ((n > 19552 && n < 19569)))
	; 6519..6525 bytes
	delay_n (n-2)
	nop
.elseif(n = 19554)
	; 6518 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19555)
	; 6522 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19557)
	; 6522 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19558)
	; 6520 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19560)
	; 6520 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19561)
	; 6524 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19563)
	; 6524 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19564)
	; 6522 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19566)
	; 6522 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19567)
	; 6526 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19569)
	; 6526 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19570)
	; 6524 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.else
.if((n >= 19571) && (n < 19590))
; 19571 to 19589
.if((n .mod 3 = 2) && ((n > 19570 && n < 19590)))
	; 6525..6533 bytes
	delay_n (n-2)
	nop
.elseif(n = 19572)
	; 6524 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19573)
	; 6528 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19575)
	; 6528 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19576)
	; 6526 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19578)
	; 6526 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19579)
	; 6530 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19581)
	; 6530 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19582)
	; 6528 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19584)
	; 6528 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19585)
	; 6532 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19587)
	; 6532 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19588)
	; 6530 bytes
	delay_n 94
	delay_n (n-94)
.endif
.else
; 19590 to 19609
.if(n = 19590)
	; 6530 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19591)
	; 6534 bytes
	delay_n 151
	delay_n (n-151)
.elseif((n .mod 3 = 2) && ((n > 19591 && n < 19608)))
	; 6531..6539 bytes
	delay_n (n-2)
	nop
.elseif(n = 19593)
	; 6534 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19594)
	; 6532 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19596)
	; 6532 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19597)
	; 6536 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19599)
	; 6536 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6538 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19605)
	; 6538 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19606)
	; 6536 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19608)
	; 6536 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19609)
	; 6540 bytes
	delay_n 115
	delay_n (n-115)
.endif
.endif
.endif
.else
.if((n >= 19610) && (n < 19649))
.if((n >= 19610) && (n < 19629))
; 19610 to 19628
.if((n .mod 3 = 2) && ((n > 19609 && n < 19629)))
	; 6537..6545 bytes
	delay_n (n-2)
	nop
.elseif(n = 19611)
	; 6540 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19612)
	; 6538 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19614)
	; 6538 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19615)
	; 6542 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19617)
	; 6542 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19618)
	; 6540 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19620)
	; 6540 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19621)
	; 6544 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19623)
	; 6544 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19624)
	; 6542 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19626)
	; 6542 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19627)
	; 6546 bytes
	delay_n 79
	delay_n (n-79)
.endif
.else
; 19629 to 19648
.if(n = 19629)
	; 6546 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19630)
	; 6544 bytes
	delay_n 82
	delay_n (n-82)
.elseif((n .mod 3 = 2) && ((n > 19630 && n < 19647)))
	; 6545..6551 bytes
	delay_n (n-2)
	nop
.elseif(n = 19632)
	; 6544 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19633)
	; 6548 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19635)
	; 6548 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19636)
	; 6546 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19638)
	; 6546 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19639)
	; 6550 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19641)
	; 6550 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19642)
	; 6548 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19644)
	; 6548 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19645)
	; 6552 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19647)
	; 6552 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19648)
	; 6550 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 19649) && (n < 19668))
; 19649 to 19667
.if((n .mod 3 = 2) && ((n > 19648 && n < 19668)))
	; 6551..6559 bytes
	delay_n (n-2)
	nop
.elseif(n = 19650)
	; 6550 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19651)
	; 6554 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19653)
	; 6554 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6556 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19659)
	; 6556 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19660)
	; 6554 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19662)
	; 6554 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19663)
	; 6558 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19665)
	; 6558 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19666)
	; 6556 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 19668 to 19687
.if(n = 19668)
	; 6556 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19669)
	; 6560 bytes
	delay_n 67
	delay_n (n-67)
.elseif((n .mod 3 = 2) && ((n > 19669 && n < 19686)))
	; 6557..6565 bytes
	delay_n (n-2)
	nop
.elseif(n = 19671)
	; 6560 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19672)
	; 6558 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19674)
	; 6558 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19675)
	; 6562 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19677)
	; 6562 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19678)
	; 6560 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19680)
	; 6560 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19681)
	; 6564 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19683)
	; 6564 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19684)
	; 6562 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19686)
	; 6562 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19687)
	; 6566 bytes
	delay_n 139
	delay_n (n-139)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 19688) && (n < 19844))
.if((n >= 19688) && (n < 19766))
.if((n >= 19688) && (n < 19727))
.if((n >= 19688) && (n < 19707))
; 19688 to 19706
.if((n .mod 3 = 2) && ((n > 19687 && n < 19707)))
	; 6563..6571 bytes
	delay_n (n-2)
	nop
.elseif(n = 19689)
	; 6566 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19690)
	; 6564 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19692)
	; 6564 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19693)
	; 6568 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19695)
	; 6568 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19696)
	; 6566 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19698)
	; 6566 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19699)
	; 6570 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19701)
	; 6570 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19702)
	; 6568 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19704)
	; 6568 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19705)
	; 6572 bytes
	delay_n 103
	delay_n (n-103)
.endif
.else
; 19707 to 19726
.if(n = 19707)
	; 6572 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 19708)
	; 6570 bytes
	delay_n 106
	delay_n (n-106)
.elseif((n .mod 3 = 2) && ((n > 19708 && n < 19725)))
	; 6571..6577 bytes
	delay_n (n-2)
	nop
.elseif(n = 19710)
	; 6570 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.elseif(n = 19711)
	; 6574 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19713)
	; 6574 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19714)
	; 6572 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19716)
	; 6572 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19717)
	; 6576 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19719)
	; 6576 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19720)
	; 6574 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19722)
	; 6574 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19723)
	; 6578 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19725)
	; 6578 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19726)
	; 6576 bytes
	delay_n 70
	delay_n (n-70)
.endif
.endif
.else
.if((n >= 19727) && (n < 19746))
; 19727 to 19745
.if((n .mod 3 = 2) && ((n > 19726 && n < 19746)))
	; 6577..6585 bytes
	delay_n (n-2)
	nop
.elseif(n = 19728)
	; 6576 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19729)
	; 6580 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19731)
	; 6580 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19732)
	; 6578 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19734)
	; 6578 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19735)
	; 6582 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19737)
	; 6582 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19738)
	; 6580 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19740)
	; 6580 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19741)
	; 6584 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19743)
	; 6584 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19744)
	; 6582 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 19746 to 19765
.if(n = 19746)
	; 6582 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19747)
	; 6586 bytes
	delay_n 91
	delay_n (n-91)
.elseif((n .mod 3 = 2) && ((n > 19747 && n < 19764)))
	; 6583..6591 bytes
	delay_n (n-2)
	nop
.elseif(n = 19749)
	; 6586 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19750)
	; 6584 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19752)
	; 6584 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19753)
	; 6588 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19755)
	; 6588 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19756)
	; 6586 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19758)
	; 6586 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19759)
	; 6590 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19761)
	; 6590 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6592 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.else
.if((n >= 19766) && (n < 19805))
.if((n >= 19766) && (n < 19785))
; 19766 to 19784
.if((n .mod 3 = 2) && ((n > 19765 && n < 19785)))
	; 6589..6597 bytes
	delay_n (n-2)
	nop
.elseif(n = 19767)
	; 6592 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19768)
	; 6590 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19770)
	; 6590 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19771)
	; 6594 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19773)
	; 6594 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19774)
	; 6592 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19776)
	; 6592 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19777)
	; 6596 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19779)
	; 6596 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19780)
	; 6594 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19782)
	; 6594 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19783)
	; 6598 bytes
	delay_n 127
	delay_n (n-127)
.endif
.else
; 19785 to 19804
.if(n = 19785)
	; 6598 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19786)
	; 6596 bytes
	delay_n 22
	delay_n (n-22)
.elseif((n .mod 3 = 2) && ((n > 19786 && n < 19803)))
	; 6597..6603 bytes
	delay_n (n-2)
	nop
.elseif(n = 19788)
	; 6596 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19789)
	; 6600 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19791)
	; 6600 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19792)
	; 6598 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19794)
	; 6598 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19795)
	; 6602 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19797)
	; 6602 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19798)
	; 6600 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19800)
	; 6600 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19801)
	; 6604 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19803)
	; 6604 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19804)
	; 6602 bytes
	delay_n 94
	delay_n (n-94)
.endif
.endif
.else
.if((n >= 19805) && (n < 19824))
; 19805 to 19823
.if((n .mod 3 = 2) && ((n > 19804 && n < 19824)))
	; 6603..6611 bytes
	delay_n (n-2)
	nop
.elseif(n = 19806)
	; 6602 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19807)
	; 6606 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19809)
	; 6606 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19810)
	; 6604 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19812)
	; 6604 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19813)
	; 6608 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19815)
	; 6608 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6610 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19821)
	; 6610 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19822)
	; 6608 bytes
	delay_n 58
	delay_n (n-58)
.endif
.else
; 19824 to 19843
.if(n = 19824)
	; 6608 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19825)
	; 6612 bytes
	delay_n 115
	delay_n (n-115)
.elseif((n .mod 3 = 2) && ((n > 19825 && n < 19842)))
	; 6609..6617 bytes
	delay_n (n-2)
	nop
.elseif(n = 19827)
	; 6612 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19828)
	; 6610 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19830)
	; 6610 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19831)
	; 6614 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19833)
	; 6614 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19834)
	; 6612 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19836)
	; 6612 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19837)
	; 6616 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19839)
	; 6616 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19840)
	; 6614 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19842)
	; 6614 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19843)
	; 6618 bytes
	delay_n 79
	delay_n (n-79)
.endif
.endif
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
.if((n >= 19844) && (n < 19883))
.if((n >= 19844) && (n < 19863))
; 19844 to 19862
.if((n .mod 3 = 2) && ((n > 19843 && n < 19863)))
	; 6615..6623 bytes
	delay_n (n-2)
	nop
.elseif(n = 19845)
	; 6618 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19846)
	; 6616 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19848)
	; 6616 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19849)
	; 6620 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19851)
	; 6620 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19852)
	; 6618 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19854)
	; 6618 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19855)
	; 6622 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19857)
	; 6622 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19858)
	; 6620 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19860)
	; 6620 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19861)
	; 6624 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 19863 to 19882
.if(n = 19863)
	; 6624 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19864)
	; 6622 bytes
	delay_n 46
	delay_n (n-46)
.elseif((n .mod 3 = 2) && ((n > 19864 && n < 19881)))
	; 6623..6629 bytes
	delay_n (n-2)
	nop
.elseif(n = 19866)
	; 6622 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19867)
	; 6626 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19869)
	; 6626 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6628 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19875)
	; 6628 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19876)
	; 6626 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19878)
	; 6626 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19879)
	; 6630 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19881)
	; 6630 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19882)
	; 6628 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 19883) && (n < 19902))
; 19883 to 19901
.if((n .mod 3 = 2) && ((n > 19882 && n < 19902)))
	; 6629..6637 bytes
	delay_n (n-2)
	nop
.elseif(n = 19884)
	; 6628 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19885)
	; 6632 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19887)
	; 6632 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19888)
	; 6630 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19890)
	; 6630 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19891)
	; 6634 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19893)
	; 6634 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19894)
	; 6632 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19896)
	; 6632 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19897)
	; 6636 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19899)
	; 6636 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19900)
	; 6634 bytes
	delay_n 82
	delay_n (n-82)
.endif
.else
; 19902 to 19921
.if(n = 19902)
	; 6634 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19903)
	; 6638 bytes
	delay_n 139
	delay_n (n-139)
.elseif((n .mod 3 = 2) && ((n > 19903 && n < 19920)))
	; 6635..6643 bytes
	delay_n (n-2)
	nop
.elseif(n = 19905)
	; 6638 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19906)
	; 6636 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19908)
	; 6636 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19909)
	; 6640 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19911)
	; 6640 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19912)
	; 6638 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19914)
	; 6638 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19915)
	; 6642 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19917)
	; 6642 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19918)
	; 6640 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19920)
	; 6640 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19921)
	; 6644 bytes
	delay_n 103
	delay_n (n-103)
.endif
.endif
.endif
.else
.if((n >= 19922) && (n < 19961))
.if((n >= 19922) && (n < 19941))
; 19922 to 19940
.if((n .mod 3 = 2) && ((n > 19921 && n < 19941)))
	; 6641..6649 bytes
	delay_n (n-2)
	nop
.elseif(n = 19923)
	; 6644 bytes
	delay_n 105
	delay_n (n-105)
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
	; 6646 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19929)
	; 6646 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19930)
	; 6644 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19932)
	; 6644 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19933)
	; 6648 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19935)
	; 6648 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19936)
	; 6646 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19938)
	; 6646 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19939)
	; 6650 bytes
	delay_n 67
	delay_n (n-67)
.endif
.else
; 19941 to 19960
.if(n = 19941)
	; 6650 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19942)
	; 6648 bytes
	delay_n 70
	delay_n (n-70)
.elseif((n .mod 3 = 2) && ((n > 19942 && n < 19959)))
	; 6649..6655 bytes
	delay_n (n-2)
	nop
.elseif(n = 19944)
	; 6648 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19945)
	; 6652 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 19947)
	; 6652 bytes
	delay_n 129
	delay_n (n-129)
.elseif(n = 19948)
	; 6650 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19950)
	; 6650 bytes
	delay_n 24
	delay_n (n-24)
.elseif(n = 19951)
	; 6654 bytes
	delay_n 79
	delay_n (n-79)
.elseif(n = 19953)
	; 6654 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19954)
	; 6652 bytes
	delay_n 82
	delay_n (n-82)
.elseif(n = 19956)
	; 6652 bytes
	delay_n 84
	delay_n (n-84)
.elseif(n = 19957)
	; 6656 bytes
	delay_n 139
	delay_n (n-139)
.elseif(n = 19959)
	; 6656 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19960)
	; 6654 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
.if((n >= 19961) && (n < 19981))
; 19961 to 19980
.if((n .mod 3 = 2) && ((n > 19960 && n < 19980)))
	; 6655..6663 bytes
	delay_n (n-2)
	nop
.elseif(n = 19962)
	; 6654 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19963)
	; 6658 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19965)
	; 6658 bytes
	delay_n 93
	delay_n (n-93)
.elseif(n = 19966)
	; 6656 bytes
	delay_n 94
	delay_n (n-94)
.elseif(n = 19968)
	; 6656 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19969)
	; 6660 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19971)
	; 6660 bytes
	delay_n 45
	delay_n (n-45)
.elseif(n = 19972)
	; 6658 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19974)
	; 6658 bytes
	delay_n 48
	delay_n (n-48)
.elseif(n = 19975)
	; 6662 bytes
	delay_n 103
	delay_n (n-103)
.elseif(n = 19977)
	; 6662 bytes
	delay_n 105
	delay_n (n-105)
.elseif(n = 19978)
	; 6660 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19980)
	; 6660 bytes
	.repeat n/6
	 rol $36,X
	.endrepeat
.endif
.else
; 19981 to 20000
.if(n = 19981)
	; 6664 bytes
	delay_n 55
	delay_n (n-55)
.elseif((n .mod 3 = 2) && ((n > 19981 && n < 20001)))
	; 6661..6669 bytes
	delay_n (n-2)
	nop
.elseif(n = 19983)
	; 6664 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19984)
	; 6662 bytes
	delay_n 58
	delay_n (n-58)
.elseif(n = 19986)
	; 6662 bytes
	delay_n 60
	delay_n (n-60)
.elseif(n = 19987)
	; 6666 bytes
	delay_n 115
	delay_n (n-115)
.elseif(n = 19989)
	; 6666 bytes
	delay_n 117
	delay_n (n-117)
.elseif(n = 19990)
	; 6664 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19992)
	; 6664 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19993)
	; 6668 bytes
	delay_n 67
	delay_n (n-67)
.elseif(n = 19995)
	; 6668 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19996)
	; 6666 bytes
	delay_n 70
	delay_n (n-70)
.elseif(n = 19998)
	; 6666 bytes
	delay_n 72
	delay_n (n-72)
.elseif(n = 19999)
	; 6670 bytes
	delay_n 127
	delay_n (n-127)
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
