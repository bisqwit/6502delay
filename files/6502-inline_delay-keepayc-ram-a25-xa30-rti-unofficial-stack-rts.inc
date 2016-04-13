; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, S, Z+N, D, and V
; Will preserve A, Y, C, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

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
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 36
; - 7 bytes or less when n <= 1301
; - 8 bytes or less when n <= 1977
; - 9 bytes or less when n <= 2997
; -10 bytes or less when n <= 4190
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.2, max=6
; - When 31 <= n <= 50: min=5, avg=6.0, max=7
; - When 51 <= n <= 75: min=5, avg=6.0, max=7
; - When 76 <= n <= 100: min=5, avg=6.1, max=7
; - When 101 <= n <= 200: min=4, avg=6.0, max=7
; - When 201 <= n <= 300: min=5, avg=6.0, max=7
; - When 301 <= n <= 500: min=4, avg=6.0, max=7
; - When 501 <= n <= 750: min=5, avg=6.0, max=7
; - When 751 <= n <= 1000: min=5, avg=6.1, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.6, max=9
; - When 2001 <= n <= 3000: min=6, avg=7.7, max=10
; - When 3001 <= n <= 5000: min=6, avg=8.8, max=11
; - When 5001 <= n <= 7500: min=7, avg=10.2, max=11
; - When 7501 <= n <= 10000: min=8, avg=10.9, max=11
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
.if((n >= 1) && (n < 79))
.if((n >= 1) && (n < 40))
; 1 to 39
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 1 bytes
	pha
.elseif(n = 4 || n = 8 || n = 15 || n = 17 || n = 28 || n = 30 || n = 33 || n = 38)
	; 2..6 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 6)
	; 2 bytes
	delay_n (n-3)
	pha
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif((n > 8 && n < 12) || n = 20 || n = 22)
	; 3..5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 13)
	; 2 bytes
	brk
.elseif(n = 14)
	; 4 bytes
	.repeat 2
	 php
	 plp
	.endrepeat
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 39)
	; 3..6 bytes
	delay_n (n-13)
	brk
.elseif(n = 18)
	; 4 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 21 || n = 36)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 31)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 34)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 35)
	; 6 bytes
	ldx #248 ;hides 'sed'
	inx
	inx
	Jne *-3
.elseif(n = 37)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.endif
.else
; 40 to 78
.if(n = 40 || n = 48 || n = 52 || n = 53 || n = 58 || n = 60 || n = 63 || n = 68 || n = 70 || n = 73 || n = 75)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 41 || n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 42)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 43 || n = 57)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 44 || n = 54 || n = 59 || n = 69 || n = 74)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 45)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 47 || n = 67 || n = 77)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 49 || n = 65)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 55)
	; 5 bytes
	ldx #248 ;hides 'sed'
	inx
	Jne *-2
.elseif(n = 62)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 72)
	; 7 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	dex
	Jpl *-4
.endif
.endif
.else
; 79 to 156
.if(n = 79 || n = 84 || n = 94 || n = 104 || n = 109 || n = 114 || n = 119 || n = 127 || n = 139 || n = 144 || n = 149 || n = 154)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 80 || n = 115 || n = 150)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 81 || n = 96 || n = 121 || n = 131 || n = 146 || n = 151 || n = 156)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 82)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 83 || n = 87 || n = 88 || n = 90 || n = 93 || n = 95 || n = 98 || n = 100 || n = 103 || n = 108 || n = 110 || n = 118 || n = 122 || n = 123 || n = 128 || n = 133 || n = 135 || n = 138 || n = 140 || n = 143)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 85 || n = 99 || n = 113 || n = 134)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 86 || n = 91 || n = 101 || n = 106 || n = 111 || n = 116 || n = 126 || n = 136 || n = 141)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 89 || n = 105 || n = 145)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 92 || n = 120 || n = 148 || n = 155)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 97)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 102 || n = 130 || n = 152)
	; 6..7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 107)
	; 6 bytes
	ldx #24 ;hides 'clc'
	dex
	dex
	Jne *-3
.elseif(n = 112 || n = 117 || n = 132 || n = 147)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 124)
	; 4 bytes
	ldx #232 ;hides 'inx'
	Jne *-1
.elseif(n = 125)
	; 6 bytes
	ldx #26 ;hides 'nop'
	dex
	dex
	Jpl *-3
.elseif(n = 129)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 137)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 142)
	; 7 bytes
	ldx #24 ;hides 'clc'
	nop
	dex
	dex
	Jpl *-4
.elseif(n = 153)
	; 5 bytes
	ldx #234 ;hides 'nop'
	inx
	Jne *-2
.endif
.endif
.else
.if((n >= 157) && (n < 235))
; 157 to 234
.if(n = 157 || n = 158 || n = 163 || n = 165 || n = 168 || n = 173 || n = 175 || n = 178 || n = 180 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 203 || n = 205 || n = 208 || n = 210 || n = 213 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 184 || n = 189 || n = 194 || n = 202 || n = 214 || n = 219 || n = 229 || n = 234)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 160)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 161 || n = 166 || n = 171 || n = 201 || n = 226)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 162 || n = 190 || n = 197)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 167)
	; 5 bytes
	ldx #24 ;hides 'clc'
	dex
	Jne *-2
.elseif(n = 169 || n = 183 || n = 204 || n = 218 || n = 225 || n = 232)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 170)
	; 6 bytes
	ldx #90 ;hides 'nop'
	inx
	inx
	Jpl *-3
.elseif(n = 172)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 174)
	; 5 bytes
	ldx #24 ;hides 'clc'
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 175 && n < 197) || (n > 205 && n < 222) || n = 231))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 177 || n = 185 || n = 217)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 179)
	; 6 bytes
	ldx #216 ;hides 'cld'
	inx
	inx
	Jne *-3
.elseif(n = 182 || n = 187 || n = 212 || n = 222)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 188)
	; 5 bytes
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jpl *-2
.elseif(n = 199)
	; 5 bytes
	ldx #232 ;hides 'inx'
	pha
	Jne *-2
.elseif(n = 207)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 209)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 215)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	dex
	Jne *-3
.elseif(n = 220)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 224)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	dex
	Jpl *-3
.endif
.else
; 235 to 312
.if(n = 235 || n = 238 || n = 240 || n = 243 || n = 245 || n = 248 || n = 250 || n = 258 || n = 263 || n = 268 || n = 270 || n = 273 || n = 275 || n = 278 || n = 283 || n = 285 || n = 293 || n = 298 || n = 300 || n = 303 || n = 308 || n = 310)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 236 || n = 246 || n = 261 || n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 237 || n = 252 || n = 292 || n = 312)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 239)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 241 || n = 251 || n = 256 || n = 266 || n = 271 || n = 276 || n = 281 || n = 301)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 242)
	; 6 bytes
	ldx #(n-8)/9 ;hides 'nop'
	nop
	dex
	Jpl *-3
.elseif(n = 244 || n = 254 || n = 259 || n = 264 || n = 282 || n = 284 || n = 294)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 247 || n = 287)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 249 || n = 257)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 7 = 1) && ((n > 252 && n < 275) || (n > 287 && n < 310)))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 255 || n = 290)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 262 || n = 307)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 265)
	; 5 bytes
	ldx #202 ;hides 'dex'
	dex
	Jmi *-2
.elseif(n = 269)
	; 6 bytes
	ldx #58 ;hides 'nop'
	dex
	dex
	Jpl *-3
.elseif(n = 272)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 277)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 279)
	; 5 bytes
	ldx #216 ;hides 'cld'
	inx
	Jne *-2
.elseif(n = 280)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 289 || n = 297 || n = 305)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 299)
	; 6 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
.elseif(n = 304)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	dex
	Jpl *-3
.endif
.endif
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 315 || n = 318 || n = 320 || n = 328 || n = 332 || n = 333 || n = 335 || n = 338 || n = 340 || n = 343 || n = 348 || n = 350 || n = 353 || n = 355 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 380 || n = 383 || n = 388 || n = 390 || n = 402 || n = 403 || n = 408 || n = 410 || n = 413 || n = 418 || n = 420 || n = 423 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 448 || n = 450 || n = 453 || n = 455 || n = 458 || n = 460 || n = 468)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314)
	; 6 bytes
	ldx #58 ;hides 'nop'
	inx
	inx
	Jpl *-3
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 322 || n = 347 || n = 357 || n = 387 || n = 432 || n = 447 || n = 462)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 319 || n = 324 || n = 334 || n = 339 || n = 349 || n = 354 || n = 364 || n = 374 || n = 382 || n = 389 || n = 394 || n = 399 || n = 415 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459 || n = 464)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 321 || n = 326 || n = 336 || n = 356 || n = 406 || n = 416 || n = 421 || n = 431 || n = 436 || n = 446 || n = 466)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 323 || n = 330 || n = 344 || n = 365 || n = 400 || n = 407 || n = 414 || n = 428 || n = 435 || n = 442 || n = 463)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 325 || n = 360 || n = 430)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 327 || n = 342 || n = 362 || n = 422 || n = 427 || n = 452)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 329 || n = 345 || n = 369 || n = 377 || n = 417 || n = 457 || n = 465)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 337 || n = 358 || n = 372 || n = 393 || n = 449)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 352 || n = 397)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 359)
	; 6 bytes
	ldx #216 ;hides 'cld'
	nop
	inx
	Jne *-3
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 384)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 385 || n = 409 || n = 425)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 392)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 395)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 398)
	; 5 bytes
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
.elseif(n = 404)
	; 6 bytes
	ldx #216 ;hides 'cld'
	dex
	dex
	Jmi *-3
.elseif(n = 405)
	; 5 bytes
	ldx #58 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 412)
	; 5 bytes
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jpl *-2
.elseif(n = 467)
	; 6 bytes
	ldx #24 ;hides 'clc'
	inx
	inx
	Jpl *-3
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 474 || n = 479 || n = 492 || n = 494 || n = 499 || n = 504 || n = 509 || n = 514 || n = 524 || n = 534 || n = 539)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 470 || n = 498)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 471 || n = 501 || n = 521)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 472)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	inx
	Jpl *-3
.elseif(n = 473 || n = 475 || n = 478 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 528 || n = 535 || n = 538 || n = 542 || n = 543)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 475 && n < 497) || (n > 505 && n < 517) || (n > 525 && n < 547)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 477 || n = 484 || n = 505 || n = 512 || n = 519 || n = 533 || n = 540)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 480)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	nop
	inx
	Jpl *-3
.elseif(n = 482 || n = 502 || n = 522)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 485)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 487)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 489)
	; 5 bytes
	ldx #58 ;hides 'nop'
	inx
	Jpl *-2
.elseif(n = 497 || n = 529 || n = 545)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 500)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 503)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
.elseif(n = 510)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 517)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 527)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 530)
	; 6 bytes
	ldx #(n-8)/9 ;hides 'nop'
	nop
	dex
	Jpl *-3
.elseif(n = 532)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 537)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 544)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	nop
	dex
	Jne *-3
.endif
.else
; 547 to 625
.if(n = 547 || n = 554 || n = 575 || n = 617)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 548 || n = 553 || n = 555 || n = 558 || n = 563 || n = 565 || n = 570 || n = 573 || n = 578 || n = 580 || n = 583 || n = 588 || n = 590 || n = 593 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 549 || n = 559 || n = 564 || n = 574 || n = 579 || n = 584 || n = 594 || n = 599 || n = 604 || n = 614 || n = 619)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 550 || n = 595)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 551 || n = 566 || n = 586 || n = 591 || n = 601 || n = 621)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 552)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	nop
	dex
	Jpl *-3
.elseif(n = 556 || n = 561 || n = 571 || n = 576 || n = 581 || n = 596 || n = 606 || n = 611 || n = 616)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 557)
	; 6 bytes
	ldx #250 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 560)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 562 || n = 567 || n = 587)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 568 || n = 582 || n = 589 || n = 603 || n = 610 || n = 624)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 569)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 572 || n = 592 || n = 597)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 577 || n = 585 || n = 609 || n = 625)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 602)
	; 6 bytes
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 607)
	; 5 bytes
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
.elseif(n = 622)
	; 5 bytes
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
; 626 to 781
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 627 || n = 642 || n = 662 || n = 672 || n = 677 || n = 702 || n = 707 || n = 732 || n = 742 || n = 747 || n = 762 || n = 767 || n = 772)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 628 || n = 630 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 668 || n = 670 || n = 675 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 693 || n = 695 || n = 698 || n = 700 || n = 703 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 752 || n = 753 || n = 758 || n = 760 || n = 763 || n = 765 || n = 768 || n = 770 || n = 773)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 5 bytes
	ldx #90 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 632 || n = 634 || n = 639 || n = 644 || n = 654 || n = 664 || n = 669 || n = 679 || n = 684 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 730 || n = 734 || n = 739 || n = 749 || n = 754 || n = 759 || n = 774 || n = 779)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 637)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && (n = 638 || n = 645 || (n > 672 && n < 695) || (n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 640 || n = 775)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 649 || n = 665 || n = 689 || n = 697 || n = 745)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 652 || n = 659 || n = 729)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657 || n = 705 || n = 737 || n = 769 || n = 777)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 667 || n = 685 || n = 712)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 674)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jpl *-3
.elseif(n = 692)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jpl *-3
.elseif(n = 710 || n = 780)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 713)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
.elseif(n = 727)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
.elseif(n = 744)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
.elseif(n = 748)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
.elseif(n = 755)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
.endif
.else
; 782 to 937
.if(n = 782 || n = 807 || n = 812 || n = 837 || n = 842 || n = 852 || n = 867 || n = 872 || n = 882 || n = 887 || n = 902 || n = 907 || n = 912 || n = 922)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 810 || n = 818 || n = 822 || n = 823 || n = 828 || n = 830 || n = 833 || n = 838 || n = 840 || n = 843 || n = 845 || n = 858 || n = 863 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 903 || n = 908 || n = 910 || n = 913 || n = 915 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 795) || n = 804 || (n > 813 && n < 830) || n = 839 || n = 844 || (n > 853 && n < 865) || (n > 873 && n < 885) || n = 894 || (n > 908 && n < 925) || n = 934))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 785 || n = 799 || n = 813 || n = 834 || n = 848 || n = 855 || n = 862 || n = 883 || n = 904)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 785 && n < 937)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 792 || n = 820 || n = 827 || n = 869 || n = 890 || n = 897 || n = 918 || n = 925 || n = 932)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 797)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	dex
	Jpl *-4
.elseif(n = 800)
	; 6 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
	Jmi *-3
.elseif(n = 802 || n = 847)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 809)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 815 || n = 850 || n = 885 || n = 920)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 817 || n = 825 || n = 849 || n = 857 || n = 865 || n = 889 || n = 929)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 832)
	; 7 bytes
	pha
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
	pla
.elseif(n = 835)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jmi *-3
.elseif(n = 853)
	; 5 bytes
	ldx #122 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 860)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
.elseif(n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 892)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 899)
	; 6 bytes
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
.elseif(n = 905)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 917)
	; 6 bytes
	ldx #26 ;hides 'nop'
	nop
	inx
	Jpl *-3
.elseif(n = 935)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
.elseif(n = 937)
	; 5 bytes
	ldx #122 ;hides 'nop'
	inx
	Jne *-2
.endif
.endif
.else
; 938 to 1250
.if(n = 938 || n = 943 || n = 948 || n = 950 || n = 958 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1063 || n = 1067 || n = 1068 || n = 1070 || n = 1073 || n = 1075 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1088 || n = 1090 || n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1110 || n = 1113 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1133 || n = 1138 || n = 1140 || n = 1143 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1158 || n = 1160 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1183 || n = 1188 || n = 1190 || n = 1193 || n = 1195 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1228 || n = 1230 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || (n > 1092 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1024 || n = 1029 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942 || n = 947 || n = 952 || n = 957 || n = 987 || n = 992 || n = 1012 || n = 1020 || n = 1022 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1082 || n = 1087 || n = 1092 || n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1152 || n = 1157 || n = 1167 || n = 1182 || n = 1192 || n = 1197 || n = 1202 || n = 1222 || n = 1227 || n = 1232)
	; 6..7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 945 || n = 1025 || n = 1049 || n = 1057 || n = 1089 || n = 1105 || n = 1129 || n = 1137 || n = 1145 || n = 1169 || n = 1185 || n = 1209 || n = 1217 || n = 1225 || n = 1249)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 955 || n = 990 || n = 1060 || n = 1095 || n = 1130 || n = 1165 || n = 1200 || n = 1235)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 962)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jmi *-3
.elseif(n = 969 || n = 977 || n = 985)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 972)
	; 7 bytes
	pha
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
	pla
.elseif(n = 980)
	; 6 bytes
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 982 || n = 1162)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 1007)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jpl *-4
.elseif(n = 1014)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jne *-1
.elseif(n = 1019)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1027)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 1034)
	; 6 bytes
	ldx #26 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 1052)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 1097)
	; 6 bytes
	ldx #122 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 1115)
	; 6 bytes
	ldx #248 ;hides 'sed'
	dex
	dex
	Jne *-3
.elseif(n = 1117 || n = 1207)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1124)
	; 6 bytes
	ldx #250 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 1187)
	; 7 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1237)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1250)
	; 6 bytes
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
; 1251 to 1328
.if((n .mod 5 = 1) && ((n > 1250 && n < 1277)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1252 || n = 1306 || n = 1315)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1263 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1291 || n = 1298 || n = 1307 || n = 1312 || n = 1318 || n = 1319 || n = 1326 || n = 1328)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1325)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && (n = 1257 || n = 1265 || (n > 1296 && n < 1322)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1259)
	; 6 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1293 || n = 1302)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290 || n = 1308 || n = 1316 || n = 1320 || n = 1327)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1270)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1281)
	; 5 bytes
	ldx #0
	dex
	Jne *-1
.elseif(n = 1286)
	; 7 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1287)
	; 5 bytes
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
.elseif(n = 1288 || n = 1294)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1292)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1295)
	; 7 bytes
	ldx #<$2C ;hides 'bit $90A2'
	ldx #144
	dex
	Jne *-4
.elseif(n = 1299)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1300)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1301 || n = 1311)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1304)
	; 7 bytes
	ldx #<$2C ;hides 'bit $91A2'
	ldx #145
	dex
	Jne *-4
.elseif(n = 1309)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1314)
	; 7 bytes
	bit <$2C ;hides 'bit $92A2'
	ldx #146
	dex
	Jne *-4
.elseif(n = 1322)
	; 7 bytes
	ldx #<$2C ;hides 'bit $93A2'
	ldx #147
	dex
	Jne *-4
.elseif(n = 1323)
	; 7 bytes
	bit <$2C ;hides 'bit $6DA2'
	ldx #109
	inx
	Jne *-4
.elseif(n = 1325)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 6
	inx
	Jpl *-4
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1337 || n = 1353 || n = 1361)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1330)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jmi *-4
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1367)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1332)
	; 7 bytes
	bit <$2C ;hides 'bit $6CA2'
	ldx #108
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1332 && n < 1361)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1334 || n = 1346 || n = 1348 || n = 1356 || n = 1362 || n = 1364)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1335)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1336)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1339)
	; 6 bytes
	ldx #122 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1340 || n = 1344 || n = 1347 || n = 1354 || n = 1355)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1341)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1343)
	; 7 bytes
	ldx #121
	delay_n 6
	dex
	Jpl *-3
.elseif(n = 1349)
	; 7 bytes
	ldx #<$2C ;hides 'bit $6AA2'
	ldx #106
	inx
	Jne *-4
.elseif(n = 1350)
	; 7 bytes
	bit <$2C ;hides 'bit $6AA2'
	ldx #106
	inx
	Jne *-4
.elseif(n = 1357)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1358)
	; 7 bytes
	ldx #<$2C ;hides 'bit $97A2'
	ldx #151
	dex
	Jne *-4
.elseif(n = 1363)
	; 7 bytes
	ldx #248 ;hides 'sed'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1365)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1367)
	; 6 bytes
	ldx #202 ;hides 'dex'
	brk
	Jmi *-3
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1375 || n = 1379 || n = 1382 || n = 1384 || n = 1389 || n = 1403)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1369 || n = 1377 || n = 1393)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1370)
	; 7 bytes
	ldx #137 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 1371 || n = 1391)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1372 || n = 1383 || n = 1388 || n = 1397 || n = 1402)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 1) && ((n > 1372 && n < 1402)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1374)
	; 7 bytes
	ldx #250 ;hides 'nop'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1376)
	; 6 bytes
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
.elseif(n = 1378 || n = 1396 || n = 1405)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1381)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1385)
	; 5 bytes
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
.elseif(n = 1386)
	; 7 bytes
	bit <$2C ;hides 'bit $9AA2'
	ldx #154
	dex
	Jne *-4
.elseif(n = 1390 || n = 1404)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1392 || n = 1406)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1395)
	; 7 bytes
	bit <$2C ;hides 'bit $9BA2'
	ldx #155
	dex
	Jne *-4
.elseif(n = 1398)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.elseif(n = 1400)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1410 || n = 1416 || n = 1419 || n = 1424 || n = 1431 || n = 1434 || n = 1435 || n = 1437 || n = 1438 || n = 1445)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1444)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && ((n > 1408 && n < 1442)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1411 || n = 1421)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1412)
	; 7 bytes
	ldx #<$2C ;hides 'bit $63A2'
	ldx #99
	inx
	Jne *-4
.elseif(n = 1413)
	; 7 bytes
	bit <$2C ;hides 'bit $63A2'
	ldx #99
	inx
	Jne *-4
.elseif(n = 1414 || n = 1432)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1418 || n = 1423 || n = 1428 || n = 1444)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1426)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1427)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1430)
	; 7 bytes
	ldx #<$2C ;hides 'bit $9FA2'
	ldx #159
	dex
	Jne *-4
.elseif(n = 1439)
	; 7 bytes
	ldx #<$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
.elseif(n = 1440)
	; 7 bytes
	bit <$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
.elseif(n = 1442)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.endif
.else
; 1446 to 1484
.if(n = 1446 || n = 1460 || n = 1474)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1447 || n = 1452 || n = 1454 || n = 1466 || n = 1470 || n = 1479 || n = 1480 || n = 1482 || n = 1483)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1448)
	; 7 bytes
	ldx #<$2C ;hides 'bit $A1A2'
	ldx #161
	dex
	Jne *-4
.elseif(n = 1449 || n = 1465 || n = 1473 || n = 1481)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 7 = 1) && ((n > 1449 && n < 1479)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1451 || n = 1461)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1453)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1455)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jpl *-4
.elseif(n = 1456)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1458)
	; 7 bytes
	bit <$2C ;hides 'bit $A2A2'
	ldx #162
	dex
	Jne *-4
.elseif(n = 1459 || n = 1468 || n = 1477)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1462 || n = 1469)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1463)
	; 7 bytes
	ldx #122 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 1467)
	; 7 bytes
	bit <$2C ;hides 'bit $A3A2'
	ldx #163
	dex
	Jne *-4
.elseif(n = 1472)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1475)
	; 7 bytes
	ldx #<$2C ;hides 'bit $A4A2'
	ldx #164
	dex
	Jne *-4
.elseif(n = 1476)
	; 7 bytes
	bit <$2C ;hides 'bit $A4A2'
	ldx #164
	dex
	Jne *-4
.elseif(n = 1484)
	; 7 bytes
	ldx #<$2C ;hides 'bit $A5A2'
	ldx #165
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if((n .mod 7 = 1) && ((n > 1484 && n < 1521)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1486 || n = 1495 || n = 1522)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1487 || n = 1494 || n = 1498 || n = 1501 || n = 1507 || n = 1508 || n = 1510 || n = 1515 || n = 1523)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488 || n = 1500 || n = 1509 || n = 1514)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1489 || n = 1497 || n = 1505 || n = 1521)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1490 || n = 1516)
	; 7..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1491)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1493)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 1496)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1502)
	; 7 bytes
	ldx #<$2C ;hides 'bit $A7A2'
	ldx #167
	dex
	Jne *-4
.elseif(n = 1503)
	; 7 bytes
	bit <$2C ;hides 'bit $A7A2'
	ldx #167
	dex
	Jne *-4
.elseif(n = 1504)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	nop
	inx
	Jne *-3
.elseif(n = 1511)
	; 5 bytes
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
.elseif(n = 1512)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1517)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1518)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1519)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	brk
	Jmi *-4
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1526)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1528 || n = 1544)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && ((n > 1528 && n < 1562)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1530)
	; 7 bytes
	bit <$2C ;hides 'bit $AAA2'
	ldx #170
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1530 && n < 1559)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1532)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1533 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1557)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1535)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1538)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1539)
	; 7 bytes
	bit <$2C ;hides 'bit $ABA2'
	ldx #171
	dex
	Jne *-4
.elseif(n = 1546)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1547)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1551)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1552)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1554)
	; 8 bytes
	pha
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 1556)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1559)
	; 7 bytes
	ldx #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1560)
	; 7 bytes
	ldx #100 ;hides 'nop zp'
	delay_n 4
	inx
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563 || n = 1596)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1564 || n = 1571 || n = 1578 || n = 1579 || n = 1587 || n = 1592 || n = 1595 || n = 1599)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565)
	; 7 bytes
	ldx #<$2C ;hides 'bit $AEA2'
	ldx #174
	dex
	Jne *-4
.elseif(n = 1566)
	; 7 bytes
	bit <$2C ;hides 'bit $AEA2'
	ldx #174
	dex
	Jne *-4
.elseif(n = 1567 || n = 1594)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1568 || n = 1582 || n = 1589)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1598)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1570)
	; 7 bytes
	ldx #100 ;hides 'nop zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1572 || n = 1580 || n = 1586 || n = 1588 || n = 1600)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1573)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1574)
	; 7 bytes
	ldx #<$2C ;hides 'bit $AFA2'
	ldx #175
	dex
	Jne *-4
.elseif(n = 1575)
	; 7 bytes
	bit <$2C ;hides 'bit $AFA2'
	ldx #175
	dex
	Jne *-4
.elseif((n .mod 8 = 1) && ((n > 1576 && n < 1602)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1581 || n = 1591)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1584)
	; 7 bytes
	bit <$2C ;hides 'bit $B0A2'
	ldx #176
	dex
	Jne *-4
.elseif(n = 1598)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jmi *-4
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 7 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	dex
	Jne *-4
.elseif(n = 1603 || n = 1621)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1603 && n < 1640)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1620 || n = 1627 || n = 1634 || n = 1635)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1607)
	; 7 bytes
	ldx #122 ;hides 'nop'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 1609)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
.elseif(n = 1610)
	; 6 bytes
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
.elseif(n = 1612 || n = 1626 || n = 1640)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1614 || n = 1628)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1615 || n = 1624 || n = 1629)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1616 || n = 1630)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1617 || n = 1633)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1619)
	; 7 bytes
	ldx #<$2C ;hides 'bit $B4A2'
	ldx #180
	dex
	Jne *-4
.elseif(n = 1622 || n = 1636)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1623)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
.elseif(n = 1631)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1638)
	; 7 bytes
	bit <$2C ;hides 'bit $B6A2'
	ldx #182
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if((n .mod 8 = 1) && ((n > 1640 && n < 1674)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1642 || n = 1672)
	; 7..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1643)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1644)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1645)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1645 && n < 1675)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1647)
	; 7 bytes
	bit <$2C ;hides 'bit $B7A2'
	ldx #183
	dex
	Jne *-4
.elseif(n = 1648 || n = 1666 || n = 1675)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1650)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1651 || n = 1661 || n = 1671)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1652 || n = 1658 || n = 1663 || n = 1668 || n = 1670 || n = 1677)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1654 || n = 1662 || n = 1669 || n = 1676)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1655)
	; 6 bytes
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
.elseif(n = 1656)
	; 7 bytes
	bit <$2C ;hides 'bit $B8A2'
	ldx #184
	dex
	Jne *-4
.elseif(n = 1659)
	; 6 bytes
	ldx #90 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1664)
	; 7 bytes
	ldx #<$2C ;hides 'bit $B9A2'
	ldx #185
	dex
	Jne *-4
.elseif(n = 1678)
	; 7 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 1679)
	; 7 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	pha
	inx
	Jne *-4
.endif
.else
; 1680 to 1718
.if(n = 1680 || n = 1687 || n = 1694)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 7 = 1) && ((n > 1680 && n < 1717)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1699 || n = 1703 || n = 1704 || n = 1715 || n = 1718)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1693 || n = 1711)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 8 = 1) && ((n > 1688 && n < 1714)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1691 || n = 1701)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1692)
	; 7 bytes
	bit <$2C ;hides 'bit $BCA2'
	ldx #188
	dex
	Jne *-4
.elseif(n = 1696)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 1698 || n = 1708 || n = 1712)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1700)
	; 7 bytes
	ldx #<$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jne *-4
.elseif(n = 1706)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1707)
	; 7 bytes
	ldx #122 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 1710)
	; 7 bytes
	bit <$2C ;hides 'bit $BEA2'
	ldx #190
	dex
	Jne *-4
.elseif(n = 1714)
	; 7 bytes
	ldx #100 ;hides 'nop zp'
	nop $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719)
	; 7 bytes
	bit <$2C ;hides 'bit $BFA2'
	ldx #191
	dex
	Jne *-4
.elseif(n = 1720 || n = 1747 || n = 1756)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1721 || n = 1729 || n = 1745 || n = 1753)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1722 || n = 1725 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1754)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1752)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724 || n = 1726 || n = 1733 || n = 1738)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1727)
	; 7 bytes
	ldx #<$2C ;hides 'bit $C0A2'
	ldx #192
	dex
	Jne *-4
.elseif(n = 1728)
	; 7 bytes
	bit <$2C ;hides 'bit $C0A2'
	ldx #192
	dex
	Jne *-4
.elseif(n = 1731)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1735)
	; 5 bytes
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
.elseif(n = 1736)
	; 7 bytes
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	dex
	Jne *-4
.elseif(n = 1740)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1741)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1742)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1748)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1749)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1750)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1752)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1755)
	; 7 bytes
	bit <$2C ;hides 'bit $C3A2'
	ldx #195
	dex
	Jne *-4
.elseif(n = 1757)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.endif
.else
; 1758 to 1796
.if((n .mod 7 = 1) && ((n > 1757 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1759)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1760)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
.elseif((n .mod 8 = 1) && ((n > 1760 && n < 1786)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1762 || n = 1767 || n = 1770 || n = 1787 || n = 1788 || n = 1795)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1763)
	; 7 bytes
	ldx #<$2C ;hides 'bit $C4A2'
	ldx #196
	dex
	Jne *-4
.elseif(n = 1764)
	; 7 bytes
	bit <$2C ;hides 'bit $C4A2'
	ldx #196
	dex
	Jne *-4
.elseif(n = 1766 || n = 1778)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1768)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1771 || n = 1791)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1773)
	; 7 bytes
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	dex
	Jne *-4
.elseif(n = 1774 || n = 1783 || n = 1792)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1775 || n = 1780 || n = 1784 || n = 1796)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1776)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1781)
	; 6 bytes
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 1782)
	; 7 bytes
	bit <$2C ;hides 'bit $C6A2'
	ldx #198
	dex
	Jne *-4
.elseif(n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1790)
	; 7 bytes
	ldx #<$2C ;hides 'bit $C7A2'
	ldx #199
	dex
	Jne *-4
.elseif(n = 1793)
	; 6 bytes
	ldx #0
	nop
	dex
	Jne *-2
.elseif(n = 1794)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797 || n = 1803 || n = 1815 || n = 1822 || n = 1830)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1798)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1800)
	; 7 bytes
	bit <$2C ;hides 'bit $C8A2'
	ldx #200
	dex
	Jne *-4
.elseif((n .mod 8 = 1) && ((n > 1800 && n < 1834)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1802 || n = 1804 || n = 1812 || n = 1820 || n = 1829)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1805 || n = 1816)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1806)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1807)
	; 8 bytes
	ldx #128
	delay_n 9
	dex
	Jpl *-4
.elseif(n = 1808)
	; 7 bytes
	ldx #<$2C ;hides 'bit $C9A2'
	ldx #201
	dex
	Jne *-4
.elseif(n = 1810 || n = 1819 || n = 1828)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1811 || n = 1821 || n = 1831)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1813)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1814)
	; 8 bytes
	ldx #<$2C ;hides 'bit $A5A2'
	ldx #165
	nop
	dex
	Jne *-5
.elseif(n = 1818)
	; 7 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	dex
	Jne *-4
.elseif(n = 1823)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1824)
	; 8 bytes
	pha
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 1826)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
.elseif(n = 1827)
	; 7 bytes
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	dex
	Jne *-4
.elseif(n = 1832)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1834)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1835)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 7 bytes
	bit <$2C ;hides 'bit $CCA2'
	ldx #204
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1865)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1838 || n = 1860)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1839)
	; 6 bytes
	ldx #184 ;hides 'clv'
	pha
	dex
	Jne *-3
.elseif(n = 1840)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif((n .mod 8 = 1) && ((n > 1840 && n < 1874)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1842 || n = 1847 || n = 1852 || n = 1868)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1843 || n = 1848 || n = 1856 || n = 1859 || n = 1866 || n = 1867 || n = 1874)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1844)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1845)
	; 7 bytes
	bit <$2C ;hides 'bit $CDA2'
	ldx #205
	dex
	Jne *-4
.elseif(n = 1850)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1851 || n = 1871)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1853)
	; 7 bytes
	ldx #<$2C ;hides 'bit $CEA2'
	ldx #206
	dex
	Jne *-4
.elseif(n = 1854)
	; 7 bytes
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	dex
	Jne *-4
.elseif(n = 1858)
	; 8 bytes
	ldx #<$2C ;hides 'bit $A9A2'
	ldx #169
	nop
	dex
	Jne *-5
.elseif(n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1862)
	; 7 bytes
	ldx #<$2C ;hides 'bit $CFA2'
	ldx #207
	dex
	Jne *-4
.elseif(n = 1863)
	; 7 bytes
	bit <$2C ;hides 'bit $CFA2'
	ldx #207
	dex
	Jne *-4
.elseif(n = 1869)
	; 8 bytes
	ldx #<$2C ;hides 'bit $AAA2'
	ldx #170
	nop
	dex
	Jne *-5
.elseif(n = 1870)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1872)
	; 7 bytes
	bit <$2C ;hides 'bit $D0A2'
	ldx #208
	dex
	Jne *-4
.elseif(n = 1875)
	; 7 bytes
	ldx #122 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
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
.if(n = 1876 || n = 1884 || n = 1892 || n = 1896)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1877)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1878 || n = 1883 || n = 1887 || n = 1894 || n = 1903 || n = 1912)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1880)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D1A2'
	ldx #209
	dex
	Jne *-4
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1914)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1910)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1885)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1886)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1888)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1890)
	; 7 bytes
	bit <$2C ;hides 'bit $D2A2'
	ldx #210
	dex
	Jne *-4
.elseif(n = 1893 || n = 1904)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1895)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1898)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1899)
	; 7 bytes
	bit <$2C ;hides 'bit $D3A2'
	ldx #211
	dex
	Jne *-4
.elseif(n = 1901 || n = 1911)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1902)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1906)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 1907)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1908)
	; 7 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
.elseif(n = 1910)
	; 7 bytes
	ldx #44 ;hides 'bit abs'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 1914)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1948)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1916)
	; 6 bytes
	ldx #(n-8)/9 ;hides 'nop zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1917)
	; 7 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	dex
	Jne *-4
.elseif(n = 1918 || n = 1927 || n = 1936)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1919 || n = 1924 || n = 1930 || n = 1932 || n = 1940 || n = 1946)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1920 || n = 1923 || n = 1928 || n = 1938 || n = 1939 || n = 1942 || n = 1947)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 1) && ((n > 1920 && n < 1954)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1922)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 1925)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D6A2'
	ldx #214
	dex
	Jne *-4
.elseif(n = 1926)
	; 7 bytes
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	dex
	Jne *-4
.elseif(n = 1931 || n = 1941 || n = 1951)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1933)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1934)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	dex
	Jne *-4
.elseif(n = 1935)
	; 7 bytes
	bit <$2C ;hides 'bit $D7A2'
	ldx #215
	dex
	Jne *-4
.elseif(n = 1943)
	; 6 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
.elseif(n = 1944)
	; 7 bytes
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	dex
	Jne *-4
.elseif(n = 1949)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1950)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1952)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if((n .mod 9 = 1) && ((n > 1953 && n < 1991)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1955 || n = 1965 || n = 1973 || n = 1974 || n = 1983 || n = 1987)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1956 || n = 1960 || n = 1982 || n = 1984)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1957)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1958)
	; 8 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	nop
	dex
	Jne *-5
.elseif(n = 1959 || n = 1992)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1961)
	; 6 bytes
	ldx #218 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 1962)
	; 7 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	dex
	Jne *-4
.elseif(n = 1964)
	; 7 bytes
	pha
	ldx #245
	pha
	dex
	Jne *-2
.elseif(n = 1966)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1967)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 9
	dex
	Jmi *-5
.elseif(n = 1968)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 1969 || n = 1977 || n = 1985)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1970)
	; 7 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	dex
	Jne *-4
.elseif(n = 1971 || n = 1991)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1975)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1976 || n = 1986)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1978)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1979)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1980)
	; 7 bytes
	bit <$2C ;hides 'bit $DCA2'
	ldx #220
	dex
	Jne *-4
.elseif(n = 1988)
	; 7 bytes
	ldx #<$2C ;hides 'bit $DDA2'
	ldx #221
	dex
	Jne *-4
.elseif(n = 1989)
	; 7 bytes
	bit <$2C ;hides 'bit $DDA2'
	ldx #221
	dex
	Jne *-4
.endif
.else
; 1993 to 2031
.if((n .mod 8 = 1) && ((n > 1992 && n < 2026)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1994 || n = 1995 || n = 2000 || n = 2010 || n = 2013 || n = 2018 || n = 2019 || n = 2022 || n = 2027 || n = 2030)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1996 || n = 2002 || n = 2004 || n = 2012 || n = 2020 || n = 2028)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1997)
	; 7 bytes
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	dex
	Jne *-4
.elseif(n = 1998)
	; 7 bytes
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	dex
	Jne *-4
.elseif(n = 1999)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pha
	inx
	Jne *-3
.elseif(n = 2003 || n = 2014)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2005)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2006)
	; 7 bytes
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	dex
	Jne *-4
.elseif(n = 2007)
	; 7 bytes
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	dex
	Jne *-4
.elseif(n = 2008 || n = 2026)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2011 || n = 2021 || n = 2031)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2015)
	; 7 bytes
	ldx #<$2C ;hides 'bit $E0A2'
	ldx #224
	dex
	Jne *-4
.elseif(n = 2016)
	; 7 bytes
	bit <$2C ;hides 'bit $E0A2'
	ldx #224
	dex
	Jne *-4
.elseif(n = 2023)
	; 7 bytes
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2024)
	; 7 bytes
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	inx
	Jne *-4
.elseif(n = 2029)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop
	pha
	Jne *-3
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2050)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2033 || n = 2041)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 2034)
	; 7 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2063)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2036 || n = 2047 || n = 2058)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2037 || n = 2038 || n = 2046 || n = 2054 || n = 2055 || n = 2064)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2039)
	; 7 bytes
	ldx #26 ;hides 'nop'
	brk
	inx
	Jpl *-4
.elseif(n = 2040 || n = 2048)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2042)
	; 7 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
.elseif(n = 2043)
	; 7 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
.elseif(n = 2045 || n = 2059)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2049)
	; 6 bytes
	ldx #0
	pha
	dex
	Jne *-2
.elseif(n = 2051 || n = 2061)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2052)
	; 7 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	dex
	Jne *-4
.elseif(n = 2056)
	; 8 bytes
	ldx #<$2C ;hides 'bit $BBA2'
	ldx #187
	nop
	dex
	Jne *-5
.elseif(n = 2057)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2060)
	; 7 bytes
	ldx #<$2C ;hides 'bit $E5A2'
	ldx #229
	dex
	Jne *-4
.elseif(n = 2063)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	pha
	inx
	Jne *-4
.elseif(n = 2065)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2066)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	nop $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2067)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2068)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	nop
	pha
	inx
	Jne *-4
.elseif(n = 2069)
	; 6 bytes
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 2070)
	; 7 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inx
	Jne *-4
.endif
.else
; 2071 to 2109
.if((n .mod 9 = 1) && ((n > 2070 && n < 2108)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2072 || n = 2090 || n = 2108)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2073 || n = 2082 || n = 2083 || n = 2093 || n = 2099 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074 || n = 2092)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2075)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2076 || n = 2085 || n = 2094)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2077 || n = 2101)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2078)
	; 6 bytes
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
.elseif(n = 2079)
	; 7 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
.elseif(n = 2081 || n = 2091)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2084)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2086)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2087)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
.elseif(n = 2088)
	; 7 bytes
	bit <$2C ;hides 'bit $E8A2'
	ldx #232
	dex
	Jne *-4
.elseif(n = 2095)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 2096)
	; 7 bytes
	ldx #<$2C ;hides 'bit $E9A2'
	ldx #233
	dex
	Jne *-4
.elseif(n = 2097)
	; 7 bytes
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	inx
	Jne *-4
.elseif(n = 2102)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2105)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2106)
	; 7 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2119 || n = 2128)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif((n .mod 10 = 1) && ((n > 2110 && n < 2142)))
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2112)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2113 || n = 2137)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2114)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 2115)
	; 7 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2115 && n < 2144)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2117 || n = 2126 || n = 2140)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2118 || n = 2127 || n = 2136 || n = 2144 || n = 2145 || n = 2148)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2120 || n = 2138)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2122)
	; 8 bytes
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	nop
	dex
	Jne *-5
.elseif(n = 2123)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 2124)
	; 7 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	dex
	Jne *-4
.elseif(n = 2129)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2130)
	; 8 bytes
	pha
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 2132)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2133)
	; 7 bytes
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	dex
	Jne *-4
.elseif(n = 2135 || n = 2146)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2139)
	; 7 bytes
	ldx #234 ;hides 'nop'
	brk
	dex
	Jmi *-4
.elseif(n = 2142)
	; 7 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	inx
	Jne *-4
.elseif(n = 2147)
	; 7 bytes
	ldx #232 ;hides 'inx'
	pha
	inx
	inx
	Jne *-4
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2173 || n = 2185)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2150)
	; 7 bytes
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	inx
	Jne *-4
.elseif(n = 2151 || n = 2171 || n = 2181)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2152 || n = 2161 || n = 2170)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2153 || n = 2154 || n = 2163 || n = 2172 || n = 2175 || n = 2180 || n = 2183)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2155)
	; 8 bytes
	ldx #<$2C ;hides 'bit $C4A2'
	ldx #196
	nop
	dex
	Jne *-5
.elseif(n = 2156)
	; 8 bytes
	bit <$2C ;hides 'bit $C4A2'
	ldx #196
	nop
	dex
	Jne *-5
.elseif(n = 2157)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2158)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2159)
	; 6 bytes
	ldx #216 ;hides 'cld'
	pha
	dex
	Jne *-3
.elseif(n = 2160)
	; 7 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	dex
	Jne *-4
.elseif(n = 2162 || n = 2164 || n = 2182)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2165)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2166)
	; 8 bytes
	ldx #<$2C ;hides 'bit $C5A2'
	ldx #197
	nop
	dex
	Jne *-5
.elseif(n = 2167)
	; 8 bytes
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	nop
	dex
	Jne *-5
.elseif(n = 2168)
	; 7 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	dex
	Jne *-4
.elseif(n = 2169)
	; 7 bytes
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	inx
	Jne *-4
.elseif(n = 2174 || n = 2184)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2176)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2177)
	; 7 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
.elseif(n = 2178)
	; 7 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
.elseif(n = 2179)
	; 6 bytes
	ldx #218 ;hides 'nop'
	pha
	dex
	Jne *-3
.elseif(n = 2186)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 2187)
	; 7 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	inx
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if((n .mod 9 = 1) && ((n > 2187 && n < 2225)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2189 || n = 2218)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2190)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2191 || n = 2201 || n = 2211)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2192 || n = 2193 || n = 2202 || n = 2203 || n = 2208 || n = 2216 || n = 2217 || n = 2225 || n = 2226)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2194)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 2195)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 2196)
	; 7 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	dex
	Jne *-4
.elseif(n = 2198)
	; 7 bytes
	ldx #12 ;hides 'nop abs'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2199)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2200)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2204)
	; 6 bytes
	ldx #(n-8)/9 ;hides 'nop zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 2205)
	; 7 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	dex
	Jne *-4
.elseif(n = 2207)
	; 7 bytes
	ldx #184 ;hides 'clv'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2209 || n = 2221)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2210)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2212)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2213)
	; 7 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jne *-4
.elseif(n = 2214)
	; 7 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	dex
	Jne *-4
.elseif(n = 2219)
	; 8 bytes
	ldx #<$2C ;hides 'bit $B9A2'
	ldx #185
	pha
	dex
	Jne *-5
.elseif(n = 2220)
	; 8 bytes
	bit <$2C ;hides 'bit $B9A2'
	ldx #185
	pha
	dex
	Jne *-5
.elseif(n = 2222)
	; 7 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	inx
	Jne *-4
.elseif(n = 2223)
	; 7 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	dex
	Jne *-4
.endif
.else
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2228)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2229 || n = 2244)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2230)
	; 9 bytes
	pha
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
.elseif(n = 2231)
	; 6 bytes
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
.elseif(n = 2232)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
.elseif(n = 2233 || n = 2242)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2234)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2235)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2236)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2237)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2238)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2239)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 2240)
	; 7 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2241)
	; 7 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2243)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2245)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.endif
.else
; 2246 to 2265
.if(n = 2246 || n = 2252)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2247 || n = 2253 || n = 2262 || n = 2263)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2248)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 2249)
	; 6 bytes
	ldx #250 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2250)
	; 7 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2251 || n = 2260)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2254)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2255)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2256)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2257)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2258)
	; 7 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	inx
	Jne *-4
.elseif(n = 2259)
	; 7 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	inx
	Jne *-4
.elseif(n = 2261)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2264)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2265)
	; 8 bytes
	ldx #<$2C ;hides 'bit $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266)
	; 8 bytes
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
.elseif(n = 2267)
	; 7 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
.elseif(n = 2268)
	; 7 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2268 && n < 2297)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2270 || n = 2273 || n = 2279 || n = 2280 || n = 2282 || n = 2288 || n = 2298)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2271 || n = 2291 || n = 2301)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2272 || n = 2284 || n = 2290 || n = 2292 || n = 2302)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2274)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2275)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2276)
	; 7 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	inx
	Jne *-4
.elseif(n = 2277)
	; 7 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	inx
	Jne *-4
.elseif(n = 2281 || n = 2293)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2283 || n = 2297)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2285)
	; 7 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2286)
	; 7 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2289 || n = 2300)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2294)
	; 7 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2295)
	; 7 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2299)
	; 6 bytes
	ldx #26 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 2303)
	; 7 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	dex
	Jne *-4
.elseif(n = 2304)
	; 7 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	inx
	Jne *-4
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 7 bytes
	ldx #0
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2306 || n = 2307 || n = 2313)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2309)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
.elseif(n = 2310)
	; 8 bytes
	bit <$2C ;hides 'bit $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
.elseif(n = 2311 || n = 2321)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2312)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 2314)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2315)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2316)
	; 8 bytes
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	pha
	dex
	Jne *-5
.elseif(n = 2317)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2318)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2319)
	; 6 bytes
	ldx #24 ;hides 'clc'
	pha
	inx
	Jne *-3
.elseif(n = 2320)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2322)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2323)
	; 7 bytes
	ldx #90 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.endif
.else
; 2324 to 2343
.if(n = 2324 || n = 2343)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2325)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2326)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	inx
	inx
	Jne *-5
.elseif(n = 2327)
	; 8 bytes
	ldx #<$2C ;hides 'bit $C2A2'
	ldx #194
	pha
	dex
	Jne *-5
.elseif(n = 2328)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2329 || n = 2341)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2330)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2331)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2332)
	; 8 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
.elseif(n = 2333)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2334)
	; 8 bytes
	ldx #44 ;hides 'bit abs'
	inc @zptemp
	nop
	inx
	Jne *-5
.elseif(n = 2335)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 2336)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
.elseif(n = 2337)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2338)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2339)
	; 6 bytes
	ldx #234 ;hides 'nop'
	pha
	dex
	Jne *-3
.elseif(n = 2340)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 2342)
	; 7 bytes
	ldx #(n-10)/11 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	dex
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2355 || n = 2366)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2345 || n = 2350)
	; 8..9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2346 || n = 2357 || n = 2363 || n = 2368 || n = 2373 || n = 2379)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2348 || n = 2360 || n = 2372 || n = 2382)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2349)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif((n .mod 10 = 1) && ((n > 2350 && n < 2382)))
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2352)
	; 8 bytes
	bit <$2C ;hides 'bit $C4A2'
	ldx #196
	pha
	dex
	Jne *-5
.elseif(n = 2353 || n = 2365 || n = 2377)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2354)
	; 8 bytes
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	nop
	dex
	Jne *-5
.elseif(n = 2356)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2358 || n = 2362 || n = 2369 || n = 2378)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2359)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2364)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	nop
	dex
	Jne *-5
.elseif(n = 2367)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2370)
	; 8 bytes
	ldx #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 2374)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2375)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2376)
	; 8 bytes
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	nop
	dex
	Jne *-5
.elseif(n = 2380)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.endif
.else
.if((n >= 2383) && (n < 2402))
; 2383 to 2401
.if(n = 2383 || n = 2390 || n = 2393)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2394)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2385)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2386)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
.elseif(n = 2387)
	; 8 bytes
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
.elseif(n = 2388)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2389 || n = 2401)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2391)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2392)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2395)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2396)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2397)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2398)
	; 8 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	nop
	dex
	Jne *-5
.elseif(n = 2399)
	; 7 bytes
	ldx #56 ;hides 'sec'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2400)
	; 8 bytes
	bit <$2C ;hides 'bit $C8A2'
	ldx #200
	pha
	dex
	Jne *-5
.endif
.else
; 2402 to 2421
.if(n = 2402)
	; 8 bytes
	pha
	ldx #(n-4)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2403 || n = 2412 || n = 2415 || n = 2417)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2404)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2405)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 2406)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2407)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2408)
	; 8 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	nop
	dex
	Jne *-5
.elseif(n = 2409)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2410)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2411 || n = 2421)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2413)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2414)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2416)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2418)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2419)
	; 7 bytes
	ldx #248 ;hides 'sed'
	brk
	dex
	Jmi *-4
.elseif(n = 2420)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	nop
	pha
	inx
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2427 || n = 2429 || n = 2433 || n = 2434 || n = 2453 || n = 2455 || n = 2456)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2424)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2425 || n = 2437 || n = 2449)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2426 || n = 2440)
	; 8..9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2428 || n = 2438 || n = 2450)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2430)
	; 8 bytes
	ldx #<$2C ;hides 'bit $DDA2'
	ldx #221
	nop
	dex
	Jne *-5
.elseif(n = 2431 || n = 2441 || n = 2451)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2432 || n = 2443 || n = 2454)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2436)
	; 8 bytes
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	pha
	dex
	Jne *-5
.elseif(n = 2439)
	; 7 bytes
	ldx #122 ;hides 'nop'
	brk
	dex
	Jne *-4
.elseif(n = 2442)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2444)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	nop
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2445)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2446)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2447)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	pha
	inx
	Jne *-4
.elseif(n = 2448)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 2452)
	; 8 bytes
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
.elseif(n = 2457)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2458)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2459)
	; 7 bytes
	ldx #250 ;hides 'nop'
	brk
	dex
	Jmi *-4
.elseif(n = 2460)
	; 8 bytes
	bit <$2C ;hides 'bit $CDA2'
	ldx #205
	pha
	dex
	Jne *-5
.endif
.else
; 2461 to 2500
.if((n .mod 12 = 1) && ((n > 2460 && n < 2498)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2462)
	; 8 bytes
	pha
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2463 || n = 2469 || n = 2475 || n = 2477 || n = 2478 || n = 2483 || n = 2489)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2464)
	; 8 bytes
	bit <$2C ;hides 'bit $E0A2'
	ldx #224
	nop
	dex
	Jne *-5
.elseif((n .mod 11 = 1) && ((n > 2464 && n < 2499)))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2466 || n = 2500)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2467)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2468 || n = 2470 || n = 2482 || n = 2488 || n = 2490)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2471 || n = 2481 || n = 2491)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2472)
	; 8 bytes
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	pha
	dex
	Jne *-5
.elseif(n = 2474)
	; 8 bytes
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	nop
	inx
	Jne *-5
.elseif(n = 2479)
	; 6 bytes
	ldx #248 ;hides 'sed'
	pha
	dex
	Jne *-3
.elseif(n = 2480)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2484)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2486)
	; 8 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
.elseif(n = 2492)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2494)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2495)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D0A2'
	ldx #208
	pha
	dex
	Jne *-5
.elseif(n = 2496)
	; 8 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	inx
	Jne *-5
.elseif(n = 2499)
	; 6 bytes
	ldx #250 ;hides 'nop'
	pha
	dex
	Jne *-3
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
.if(n = 2501 || n = 2511 || n = 2531)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2502 || n = 2512 || n = 2524)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2503 || n = 2539)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2504 || n = 2514)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2505)
	; 8 bytes
	ldx #232 ;hides 'inx'
	inc @zptemp
	inx
	inx
	Jne *-5
.elseif(n = 2506)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2507 || n = 2535)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2508)
	; 8 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	nop
	dex
	Jne *-5
.elseif(n = 2509 || n = 2521 || n = 2533)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2510)
	; 8 bytes
	ldx #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 2513 || n = 2515 || n = 2522 || n = 2523 || n = 2525)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2516 || n = 2528 || n = 2538)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2517 || n = 2534)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2518)
	; 8 bytes
	ldx #<$2C ;hides 'bit $E5A2'
	ldx #229
	nop
	dex
	Jne *-5
.elseif(n = 2519)
	; 8 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	nop
	inx
	Jne *-5
.elseif(n = 2520)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2526)
	; 9 bytes
	ldx #252
	inc @zptemp
	dex
	Jne *-3
	inc @zptemp
.elseif(n = 2527)
	; 9 bytes
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
	pla
.elseif(n = 2529)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2530)
	; 8 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
.elseif(n = 2532)
	; 8 bytes
	bit <$2C ;hides 'bit $D3A2'
	ldx #211
	pha
	dex
	Jne *-5
.elseif(n = 2536)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2537)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 2540 to 2578
.if(n = 2540)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 2541)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2542 || n = 2553 || n = 2564)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2543)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2544 || n = 2547 || n = 2559 || n = 2565 || n = 2571)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2545 || n = 2557 || n = 2569)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2546)
	; 8 bytes
	ldx #44 ;hides 'bit abs'
	inc @zptemp
	pha
	inx
	Jne *-5
.elseif(n = 2548 || n = 2554 || n = 2556 || n = 2558 || n = 2560 || n = 2567 || n = 2572 || n = 2576)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2549 || n = 2563 || n = 2577)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2550)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 2551)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2552)
	; 8 bytes
	bit <$2C ;hides 'bit $E8A2'
	ldx #232
	nop
	dex
	Jne *-5
.elseif(n = 2555)
	; 7 bytes
	ldx #(n-11)/12 ;hides 'nop zp,x'
	inx ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2561)
	; 7 bytes
	ldx #0
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2562)
	; 8 bytes
	ldx #<$2C ;hides 'bit $E9A2'
	ldx #233
	nop
	dex
	Jne *-5
.elseif(n = 2566)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2568)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2570)
	; 8 bytes
	ldx #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jpl *-5
.elseif(n = 2573)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2574)
	; 8 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	nop
	dex
	Jne *-5
.elseif(n = 2575)
	; 7 bytes
	ldx #184 ;hides 'clv'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 2578)
	; 8 bytes
	pha
	ldx #(n-4)/11
	delay_n 6
	dex
	Jne *-3
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	pha
	dex
	Jne *-5
.elseif(n = 2580)
	; 8 bytes
	bit <$2C ;hides 'bit $D7A2'
	ldx #215
	pha
	dex
	Jne *-5
.elseif((n .mod 12 = 1) && ((n > 2580 && n < 2618)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2582 || n = 2612)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2583 || n = 2607 || n = 2610 || n = 2613)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2584)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2585)
	; 8 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	nop
	inx
	Jne *-5
.elseif(n = 2586 || n = 2597 || n = 2608)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2587 || n = 2589 || n = 2598 || n = 2600 || n = 2609)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2588 || n = 2614)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2590)
	; 8 bytes
	ldx #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 2591)
	; 7 bytes
	ldx #216 ;hides 'cld'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2592)
	; 8 bytes
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	pha
	dex
	Jne *-5
.elseif(n = 2594)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2595)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2596)
	; 8 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	nop
	dex
	Jne *-5
.elseif(n = 2599)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-4
.elseif(n = 2601)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2602)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2603)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	pha
	dex
	Jne *-5
.elseif(n = 2604)
	; 8 bytes
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	pha
	dex
	Jne *-5
.elseif(n = 2606)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2611)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2615)
	; 7 bytes
	ldx #218 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2616)
	; 8 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	pha
	dex
	Jne *-5
.endif
.else
; 2618 to 2656
.if(n = 2618)
	; 8 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	nop
	inx
	Jne *-5
.elseif(n = 2619 || n = 2630 || n = 2652)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2620 || n = 2622 || n = 2644)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2621)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2623 || n = 2642)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2624 || n = 2648)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2625)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2626 || n = 2656)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2627)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2628)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 2629 || n = 2641 || n = 2653)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2631 || n = 2632 || n = 2635 || n = 2643 || n = 2645 || n = 2654 || n = 2655)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2633)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2634)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2636)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2637)
	; 9 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2638)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 6
	Jne *-4
.elseif(n = 2639)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 2640)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	nop
	inx
	Jne *-5
.elseif(n = 2646)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2647)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2649)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	inx
	Jne *-5
.elseif(n = 2650)
	; 8 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	nop
	dex
	Jne *-5
.elseif(n = 2651)
	; 8 bytes
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	nop
	inx
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if(n = 2657 || n = 2667 || n = 2669 || n = 2676 || n = 2691 || n = 2693)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2658)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2659)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2660)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2661 || n = 2675)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2662)
	; 8 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	inx
	Jne *-5
.elseif(n = 2663 || n = 2674 || n = 2685)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2664)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 2665 || n = 2677 || n = 2689)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2666 || n = 2692)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2668 || n = 2680 || n = 2688)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2670 || n = 2687)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2671)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2672)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2673)
	; 8 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	nop
	inx
	Jne *-5
.elseif(n = 2678)
	; 9 bytes
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2679)
	; 7 bytes
	ldx #122 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 2681)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2682)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2683)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2684)
	; 8 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	nop
	dex
	Jne *-5
.elseif(n = 2686)
	; 8 bytes
	ldx #12 ;hides 'nop abs'
	inc @zptemp
	nop
	inx
	Jne *-5
.elseif(n = 2690)
	; 9 bytes
	ldx #<$2C ;hides 'bit $CFA2'
	ldx #207
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2694)
	; 7 bytes
	ldx #(n-10)/11 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2695)
	; 8 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	nop
	dex
	Jne *-5
.endif
.else
; 2696 to 2734
.if((n .mod 11 = 1) && ((n > 2695 && n < 2730)))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2697 || n = 2699 || n = 2722 || n = 2732)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2698 || n = 2709 || n = 2714 || n = 2715 || n = 2720 || n = 2733)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2700)
	; 8 bytes
	bit <$2C ;hides 'bit $E1A2'
	ldx #225
	pha
	dex
	Jne *-5
.elseif(n = 2701 || n = 2713 || n = 2725)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2702)
	; 9 bytes
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 2703 || n = 2717 || n = 2731)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2704 || n = 2721)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2705)
	; 8 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
.elseif(n = 2706)
	; 8 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	nop
	dex
	Jne *-5
.elseif(n = 2708 || n = 2726 || n = 2734)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2710)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 2711)
	; 8 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	pha
	inx
	Jne *-5
.elseif(n = 2712)
	; 8 bytes
	ldx #194 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2716)
	; 8 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	nop
	inx
	Jne *-5
.elseif(n = 2719)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2723)
	; 8 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	pha
	inx
	Jne *-5
.elseif(n = 2724)
	; 8 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	pha
	inx
	Jne *-5
.elseif(n = 2727)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2728)
	; 8 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
.elseif(n = 2730)
	; 8 bytes
	ldx #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
.if((n >= 2735) && (n < 2754))
; 2735 to 2753
.if(n = 2735)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 2736)
	; 8 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	pha
	dex
	Jne *-5
.elseif(n = 2737 || n = 2749)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2738)
	; 8 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	inx
	Jne *-5
.elseif(n = 2739)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2740 || n = 2751)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2741)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2742)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2743 || n = 2752)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2744)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2745)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2746)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 2747)
	; 8 bytes
	ldx #<$2C ;hides 'bit $E5A2'
	ldx #229
	pha
	dex
	Jne *-5
.elseif(n = 2748)
	; 8 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	pha
	inx
	Jne *-5
.elseif(n = 2750)
	; 8 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	inx
	Jne *-5
.elseif(n = 2753)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dex
	Jne *-5
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2755)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2756)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2757 || n = 2763 || n = 2764)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2758 || n = 2765)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2759)
	; 7 bytes
	ldx #26 ;hides 'nop'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2760)
	; 8 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	inx
	Jne *-5
.elseif(n = 2761 || n = 2773)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2762)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2766)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2767)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 2768)
	; 8 bytes
	ldx #(n-12)/13 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2769)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2770)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2771)
	; 7 bytes
	ldx #58 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2772)
	; 8 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	inx
	Jne *-5
.endif
.endif
.else
; 2774 to 2812
.if(n = 2774)
	; 8 bytes
	pha
	ldx #232 ;hides 'inx'
	pha
	dex
	dex
	Jne *-4
.elseif(n = 2775 || n = 2777 || n = 2786 || n = 2803 || n = 2808 || n = 2811)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2776)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2778)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2779)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2780 || n = 2790 || n = 2792)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2781)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2782)
	; 8 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	inx
	Jne *-5
.elseif(n = 2783)
	; 7 bytes
	ldx #24 ;hides 'clc'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2784 || n = 2795 || n = 2806)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2785 || n = 2797 || n = 2809)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2787 || n = 2801)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2788 || n = 2798 || n = 2800 || n = 2802 || n = 2812)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2789)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2791)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2793)
	; 8 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	inx
	Jne *-5
.elseif(n = 2794)
	; 8 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	inx
	Jne *-5
.elseif(n = 2796)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2799)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2804)
	; 8 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	inx
	Jne *-5
.elseif(n = 2805)
	; 8 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	inx
	Jne *-5
.elseif(n = 2807)
	; 7 bytes
	ldx #234 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2810)
	; 8 bytes
	pha
	ldx #234 ;hides 'nop'
	nop
	pha
	dex
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2818 || n = 2823 || n = 2825 || n = 2835 || n = 2837 || n = 2847 || n = 2849)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2814)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2815)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2816)
	; 8 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	inx
	Jne *-5
.elseif(n = 2817)
	; 7 bytes
	ldx #0
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2819)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2820)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 2821 || n = 2833 || n = 2845)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2822 || n = 2848)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2824 || n = 2834 || n = 2844 || n = 2846)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2826 || n = 2838 || n = 2850)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2827)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2828)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 16
	dex
	Jmi *-5
.elseif(n = 2829)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2830)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2831)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	pha
	inx
	Jne *-4
.elseif(n = 2832)
	; 8 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	pha
	dex
	Jne *-5
.elseif(n = 2836 || n = 2851)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2839)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2840)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2841)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
.elseif(n = 2842)
	; 9 bytes
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 2843)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.endif
.else
.if((n >= 2852) && (n < 2871))
; 2852 to 2870
.if(n = 2852)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2853)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 2854)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 2855)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2856)
	; 8 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	pha
	inx
	Jne *-5
.elseif(n = 2857 || n = 2869)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2858)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 2859 || n = 2865)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2860)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	nop
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2861)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2862)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2863)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2864)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2866)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2867)
	; 8 bytes
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	pha
	inx
	Jne *-5
.elseif(n = 2868)
	; 8 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	pha
	dex
	Jne *-5
.elseif(n = 2870)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.endif
.else
; 2871 to 2890
.if(n = 2871 || n = 2885)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2872 || n = 2884 || n = 2890)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2873)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2874)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2875)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2876 || n = 2888)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2877)
	; 9 bytes
	pha
	ldx #(n-4)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2878)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 2879)
	; 8 bytes
	ldx #<$2C ;hides 'bit $F0A2'
	ldx #240
	pha
	dex
	Jne *-5
.elseif(n = 2880)
	; 8 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	pha
	dex
	Jne *-5
.elseif(n = 2881)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2882)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 2883)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2886)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2887)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2889)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891 || n = 2908)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2892)
	; 8 bytes
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	pha
	inx
	Jne *-5
.elseif((n .mod 12 = 1) && ((n > 2892 && n < 2930)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2894)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 2895 || n = 2897 || n = 2907 || n = 2909 || n = 2919)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2896 || n = 2911)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2898 || n = 2910 || n = 2922)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2899)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2900)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2901 || n = 2921)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2902 || n = 2914 || n = 2918 || n = 2920)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2903)
	; 8 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	pha
	inx
	Jne *-5
.elseif(n = 2904)
	; 8 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	pha
	inx
	Jne *-5
.elseif(n = 2906 || n = 2912 || n = 2924)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2913)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2915)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2916)
	; 8 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	pha
	inx
	Jne *-5
.elseif(n = 2923)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2925)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2926)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2927)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2928)
	; 8 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	pha
	dex
	Jne *-5
.endif
.else
.if((n >= 2930) && (n < 2949))
; 2930 to 2948
.if(n = 2930 || n = 2932 || n = 2944)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2931 || n = 2937 || n = 2943 || n = 2945)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2933)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	inx
	inx
	Jne *-5
.elseif(n = 2934)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	pha
	inx
	inx
	Jne *-5
.elseif(n = 2935)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2936 || n = 2948)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2938)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2939)
	; 7 bytes
	ldx #(n-11)/12 ;hides 'nop zp,x'
	inx ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2940)
	; 8 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	pha
	dex
	Jne *-5
.elseif(n = 2941)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2942)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2946)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2947)
	; 8 bytes
	ldx #122 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.endif
.else
; 2949 to 2968
.if(n = 2949)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 2950)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 4
	inx
	Jne *-6
.elseif(n = 2951)
	; 8 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	pha
	inx
	Jne *-5
.elseif(n = 2952)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2953 || n = 2965)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2954)
	; 9 bytes
	bit <$2C ;hides 'bit $D3A2'
	ldx #211
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 2955)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2956)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2957)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2958)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2959)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2960)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2961)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2962)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 2963)
	; 8 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	pha
	inx
	Jne *-5
.elseif(n = 2964)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 2966)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	dex
	Jne *-5
.elseif(n = 2967)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2968)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.endif
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if(n = 2969 || n = 2983 || n = 2997)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2970 || n = 2994 || n = 3006)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2971 || n = 3007)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2972 || n = 2982 || n = 2984 || n = 2996)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2973 || n = 2979 || n = 2985 || n = 2991 || n = 2998 || n = 3003 || n = 3005)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2974 || n = 2980 || n = 2992)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2975)
	; 7 bytes
	ldx #248 ;hides 'sed'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2976 || n = 2993)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2977 || n = 2989 || n = 3001)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2978 || n = 3004)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2981)
	; 8 bytes
	ldx #(n-13)/14 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 2986)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2987)
	; 8 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	pha
	inx
	Jne *-5
.elseif(n = 2988)
	; 8 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	pha
	inx
	Jne *-5
.elseif(n = 2990)
	; 9 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 4
	inx
	Jne *-6
.elseif(n = 2995)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2999)
	; 7 bytes
	ldx #250 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 3000)
	; 8 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	pha
	inx
	Jne *-5
.elseif(n = 3002)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
.endif
.else
; 3008 to 3046
.if(n = 3008 || n = 3014 || n = 3020 || n = 3032)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3009 || n = 3027 || n = 3029 || n = 3033 || n = 3045)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3010)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3011 || n = 3039)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3012)
	; 8 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	pha
	inx
	Jne *-5
.elseif(n = 3013 || n = 3025 || n = 3037)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3015)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-4
.elseif(n = 3016 || n = 3031 || n = 3046)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3017 || n = 3030)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3018 || n = 3026 || n = 3028 || n = 3034 || n = 3040)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3019)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3021)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3022)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3023)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3024)
	; 8 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	pha
	inx
	Jne *-5
.elseif(n = 3035)
	; 8 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	pha
	inx
	Jne *-5
.elseif(n = 3036)
	; 8 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	pha
	inx
	Jne *-5
.elseif(n = 3038)
	; 9 bytes
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3041)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3042)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3043)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3044)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
.if(n = 3047 || n = 3052 || n = 3064)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3048)
	; 8 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	pha
	inx
	Jne *-5
.elseif(n = 3049 || n = 3061)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3050)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3051)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3053)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3054)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 3055)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3056)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3057)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3058)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	inx
	Jne *-5
.elseif(n = 3059)
	; 8 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	pha
	inx
	Jne *-5
.elseif(n = 3060)
	; 8 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	pha
	inx
	Jne *-5
.elseif(n = 3062)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3063)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3065)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	inx
	Jne *-5
.endif
.else
; 3066 to 3085
.if(n = 3066)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 3067 || n = 3081)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3068)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3069 || n = 3082)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3070)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3071)
	; 8 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	pha
	dex
	Jne *-5
.elseif(n = 3072)
	; 8 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	pha
	inx
	Jne *-5
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3074)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3075 || n = 3077 || n = 3083)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3076)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3078)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3079)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3080)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 3084)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3085)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.endif
.endif
.else
.if((n >= 3086) && (n < 3106))
; 3086 to 3105
.if(n = 3086)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3087)
	; 9 bytes
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3088)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3089 || n = 3105)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3090)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
.elseif(n = 3091)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3092)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3093)
	; 9 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3094)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3095)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3096 || n = 3099)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3097)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3098)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3100)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3101)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3102)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3103)
	; 9 bytes
	ldx #<$2C ;hides 'bit $C2A2'
	ldx #194
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3104)
	; 9 bytes
	bit <$2C ;hides 'bit $C2A2'
	ldx #194
	delay_n 7
	dex
	Jne *-6
.endif
.else
; 3106 to 3125
.if(n = 3106)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3107)
	; 9 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3108)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3109 || n = 3123)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3110)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3111 || n = 3114 || n = 3117 || n = 3125)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3113)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3115)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3116)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3118)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3119)
	; 9 bytes
	ldx #<$2C ;hides 'bit $F0A2'
	ldx #240
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3120)
	; 9 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3121)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3122)
	; 9 bytes
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3124)
	; 9 bytes
	bit <$2C ;hides 'bit $72A2'
	ldx #114
	brk
	inx
	Jne *-6
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
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3127)
	; 8 bytes
	ldx #184 ;hides 'clv'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3128)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3129)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3130)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif((n .mod 4 = 3) && ((n > 3130 && n < 3144)))
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3132)
	; 9 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3133)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3134)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3136)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3137)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3138 || n = 3142)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3140)
	; 9 bytes
	ldx #100 ;hides 'nop zp'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 3141)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3144)
	; 9 bytes
	pha
	delay_n (n-3)
.endif
.else
; 3145 to 3164
.if(n = 3145)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3146 || n = 3163)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3147)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3148 || n = 3162)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3149)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3150)
	; 9 bytes
	bit <$2C ;hides 'bit $E1A2'
	ldx #225
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3151)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3152)
	; 9 bytes
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3153)
	; 8 bytes
	ldx #90 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3154)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3155)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3156)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3157)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3158)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3159)
	; 9 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3160)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3161)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3164)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 3165) && (n < 3184))
; 3165 to 3183
.if(n = 3165 || n = 3179)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3166)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3167)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3168)
	; 9 bytes
	bit <$2C ;hides 'bit $C6A2'
	ldx #198
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3169)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3170)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 3171)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 4
	nop
	dex
	Jne *-5
.elseif(n = 3172)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3173)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3174)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3175)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	dex
	Jne *-5
.elseif(n = 3176)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3177)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3178)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3180)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3181)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3182)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3183)
	; 9 bytes
	ldx #<$2C ;hides 'bit $C7A2'
	ldx #199
	delay_n 7
	dex
	Jne *-6
.endif
.else
; 3184 to 3203
.if(n = 3184)
	; 8 bytes
	ldx #(n-12)/13 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3185)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3186)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3187)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3188 || n = 3189 || n = 3195 || n = 3203)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3190 || n = 3202)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3191)
	; 9 bytes
	ldx #<$2C ;hides 'bit $E4A2'
	ldx #228
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3192)
	; 9 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3193)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3194)
	; 8 bytes
	ldx #(n-14)/15 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3196)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3197)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3198)
	; 9 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3200)
	; 9 bytes
	bit <$2C ;hides 'bit $C8A2'
	ldx #200
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3201)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if(n = 3204 || n = 3208 || n = 3222 || n = 3242)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3205 || n = 3223 || n = 3241)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3206)
	; 9 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3207 || n = 3221 || n = 3235)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3209 || n = 3225 || n = 3227 || n = 3237 || n = 3240)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3210)
	; 9 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3211 || n = 3226)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3212 || n = 3238)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3213)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3214 || n = 3231)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3215)
	; 9 bytes
	ldx #<$2C ;hides 'bit $C9A2'
	ldx #201
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3216)
	; 9 bytes
	bit <$2C ;hides 'bit $C9A2'
	ldx #201
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3217)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3218)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3219)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3220)
	; 9 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3224)
	; 9 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3228)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3229)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3230)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3232)
	; 9 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3233)
	; 8 bytes
	ldx #232 ;hides 'inx'
	inc @zptemp
	dex
	dex
	Jne *-5
.elseif(n = 3234)
	; 9 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3236)
	; 9 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3239)
	; 8 bytes
	ldx #216 ;hides 'cld'
	nop
	delay_n 6
	dex
	Jne *-5
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3245 || n = 3260 || n = 3267 || n = 3273)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3244 || n = 3250 || n = 3252 || n = 3262 || n = 3268 || n = 3274 || n = 3278 || n = 3280)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3246)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3247)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3248 || n = 3265)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3249)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3251 || n = 3264)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3253)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3254 || n = 3266)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3255)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3256 || n = 3271)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3257)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3258)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3259 || n = 3277)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3261 || n = 3281)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3263)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3269)
	; 8 bytes
	ldx #218 ;hides 'nop'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3270)
	; 9 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	delay_n 6
	dex
	Jne *-6
.elseif(n = 3272)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3276)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 3279)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
.if((n >= 3282) && (n < 3301))
; 3282 to 3300
.if(n = 3282 || n = 3299)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283 || n = 3284 || n = 3287 || n = 3292 || n = 3293 || n = 3297)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3286)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3288)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3289)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3290)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3291)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3294)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3295)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3296)
	; 9 bytes
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3298)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3300)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 9
	inx
	Jne *-5
.endif
.else
; 3301 to 3320
.if(n = 3301)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3302)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 3303)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3304)
	; 9 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3305)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3306)
	; 9 bytes
	pha
	ldx #(n-4)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3307)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3308)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3309)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3310)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3311)
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 3312)
	; 9 bytes
	bit <$2C ;hides 'bit $CFA2'
	ldx #207
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3313)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3314)
	; 9 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3315)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3316)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3317)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3318)
	; 9 bytes
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3319)
	; 7 bytes
	ldx #90 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 3320)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3341)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3322 || n = 3334 || n = 3352)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3323)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3324 || n = 3354)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3325 || n = 3337 || n = 3339 || n = 3355)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3326)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3327)
	; 9 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3328)
	; 9 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3329)
	; 8 bytes
	ldx #0
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3330)
	; 9 bytes
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 6
	dex
	Jne *-6
.elseif(n = 3331 || n = 3349)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3332)
	; 9 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3333 || n = 3347)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3335 || n = 3342 || n = 3351 || n = 3353)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3336)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 3338 || n = 3356 || n = 3357)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3340)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3343)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D1A2'
	ldx #209
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3344)
	; 9 bytes
	bit <$2C ;hides 'bit $D1A2'
	ldx #209
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3345)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3346)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3348)
	; 9 bytes
	ldx #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	inx
	Jne *-6
.elseif(n = 3350)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3358)
	; 9 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	inx
	Jne *-6
.elseif(n = 3359)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D2A2'
	ldx #210
	delay_n 7
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
.if((n >= 3360) && (n < 3379))
; 3360 to 3378
.if(n = 3360)
	; 9 bytes
	bit <$2C ;hides 'bit $D2A2'
	ldx #210
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3361 || n = 3375)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3362)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3363 || n = 3369 || n = 3371 || n = 3378)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3364)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3365)
	; 8 bytes
	ldx #202 ;hides 'dex'
	brk
	dex
	dex
	Jne *-5
.elseif(n = 3366)
	; 9 bytes
	bit <$2C ;hides 'bit $99A2'
	ldx #153
	brk
	dex
	Jne *-6
.elseif(n = 3367)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3368)
	; 9 bytes
	pha
	ldx #58 ;hides 'nop'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3370)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3372)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3373)
	; 9 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3374)
	; 9 bytes
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3376)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3377)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.endif
.else
; 3379 to 3398
.if(n = 3379 || n = 3388)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3380)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3381)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3382)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3383)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3384)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3385)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3386)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3387 || n = 3395)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3389)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3390)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3391)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3392)
	; 9 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3393)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3394)
	; 9 bytes
	ldx #44 ;hides 'bit abs'
	inc @zptemp
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3396)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3397)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3398)
	; 9 bytes
	brk
	delay_n (n-13)
.endif
.endif
.else
.if((n >= 3399) && (n < 3418))
; 3399 to 3417
.if(n = 3399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3400)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	inx
	inx
	Jne *-6
.elseif(n = 3401)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3402)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3403)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3404)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3405)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3406)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3407)
	; 8 bytes
	ldx #(n-15)/16 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3408)
	; 9 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3409)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3410)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3411)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 3412)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 3413)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	dex
	Jne *-5
.elseif(n = 3414)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	dex
	Jne *-5
.elseif(n = 3415)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 3416)
	; 9 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3417)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.endif
.else
; 3418 to 3437
.if(n = 3418 || n = 3435)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3419 || n = 3420 || n = 3423 || n = 3433 || n = 3437)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3421)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3422)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3424)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3425)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3426)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3427)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3428)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3429)
	; 8 bytes
	ldx #(n-13)/14 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 3430)
	; 9 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3431)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3432)
	; 9 bytes
	bit <$2C ;hides 'bit $9CA2'
	ldx #156
	brk
	dex
	Jne *-6
.elseif(n = 3434)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3436)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3447 || n = 3454 || n = 3463 || n = 3468)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3439 || n = 3457 || n = 3475)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3440)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3441 || n = 3461)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3442 || n = 3448 || n = 3453 || n = 3460 || n = 3467 || n = 3474)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3443)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3444)
	; 9 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3445 || n = 3459 || n = 3473)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3446)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3449)
	; 8 bytes
	ldx #26 ;hides 'nop'
	nop
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3450)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 3451 || n = 3466)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3452 || n = 3469)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3455)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3456)
	; 9 bytes
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3458)
	; 9 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3462 || n = 3476)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3464)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-5
.elseif(n = 3465)
	; 9 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3470)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3471)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3472)
	; 9 bytes
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	delay_n 7
	dex
	Jne *-6
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3483 || n = 3507 || n = 3513)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3478 || n = 3490 || n = 3502 || n = 3514)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3479)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3480 || n = 3492 || n = 3498)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3481)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3482 || n = 3506)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3484 || n = 3512)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3485)
	; 9 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3486 || n = 3503)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3487 || n = 3501 || n = 3515)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3488)
	; 9 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3489 || n = 3505)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3491)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3493 || n = 3511)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3494)
	; 9 bytes
	ldx #<$2C ;hides 'bit $E9A2'
	ldx #233
	delay_n 6
	dex
	Jne *-6
.elseif(n = 3495)
	; 8 bytes
	ldx #184 ;hides 'clv'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 3496)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3497)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3499)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3500)
	; 9 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3504)
	; 9 bytes
	bit <$2C ;hides 'bit $DBA2'
	ldx #219
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3508)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3509)
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3510)
	; 9 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	delay_n 6
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
.if((n >= 3516) && (n < 3535))
; 3516 to 3534
.if(n = 3516)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3517 || n = 3522 || n = 3523 || n = 3531 || n = 3533)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3518)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3519)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DCA2'
	ldx #220
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3520)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3521)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3524)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3525)
	; 9 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3526)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3527)
	; 9 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3528)
	; 9 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3529)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3530)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 26
	dex
	Jmi *-6
.elseif(n = 3532)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3534)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.else
; 3535 to 3554
.if(n = 3535)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3536)
	; 9 bytes
	bit <$2C ;hides 'bit $DDA2'
	ldx #221
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3537 || n = 3554)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3538)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	inx
	Jne *-5
.elseif(n = 3539)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3540)
	; 9 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	delay_n 6
	dex
	Jne *-6
.elseif(n = 3541)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3542)
	; 9 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3543)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3544)
	; 9 bytes
	pha
	ldx #236
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3545 || n = 3549)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3546)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3547)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3548)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3550)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3551)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3552)
	; 9 bytes
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3553)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 3555) && (n < 3574))
; 3555 to 3573
.if(n = 3555)
	; 9 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3556)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3557 || n = 3571)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3558 || n = 3559 || n = 3567)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3560)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3561)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3562)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3563)
	; 9 bytes
	ldx #58 ;hides 'nop'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 3564)
	; 9 bytes
	bit <$2C ;hides 'bit $A2A2'
	ldx #162
	brk
	dex
	Jne *-6
.elseif(n = 3565)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3566)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3568)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3569)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3570)
	; 9 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3572)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3573)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.endif
.else
; 3574 to 3593
.if(n = 3574 || n = 3591)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3575)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3576)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3577)
	; 9 bytes
	ldx #100 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	inx
	Jne *-6
.elseif(n = 3578)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3579)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
.elseif(n = 3580 || n = 3587 || n = 3590)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3581)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3582)
	; 10 bytes
	bit <$2C ;hides 'bit $C7A2'
	ldx #199
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3583)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3584)
	; 9 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3586)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3588)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3589)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 3592)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3593)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
.if((n >= 3594) && (n < 3613))
; 3594 to 3612
.if(n = 3594 || n = 3603 || n = 3607)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3604)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3596)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3597)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3598)
	; 10 bytes
	ldx #108
	nop
	delay_n 26
	dex
	Jpl *-6
.elseif(n = 3599)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 3600)
	; 9 bytes
	bit <$2C ;hides 'bit $E1A2'
	ldx #225
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3601)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3602)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 6
	dex
	Jne *-6
.elseif(n = 3605)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3606 || n = 3610)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3608)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3609)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3611)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3612)
	; 9 bytes
	ldx #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
.endif
.else
; 3613 to 3632
.if(n = 3613)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3614)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3615)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3616 || n = 3631)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3617)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3618 || n = 3624)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3619)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3620)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	delay_n 6
	dex
	Jne *-6
.elseif(n = 3621)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3622)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3623)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3625)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3626)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3627)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3628)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 3629)
	; 9 bytes
	ldx #<$2C ;hides 'bit $A5A2'
	ldx #165
	brk
	dex
	Jne *-6
.elseif(n = 3630)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3632)
	; 9 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 7
	inx
	Jne *-6
.endif
.endif
.else
.if((n >= 3633) && (n < 3652))
; 3633 to 3651
.if(n = 3633 || n = 3649)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3634)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3635)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 3636 || n = 3643)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3637)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3638)
	; 10 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	brk
	inx
	Jne *-7
.elseif(n = 3639)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3640)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3641)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3642)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3644)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3645)
	; 9 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3646)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3647)
	; 9 bytes
	ldx #<$2C ;hides 'bit $E4A2'
	ldx #228
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3648)
	; 9 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3650)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3651)
	; 8 bytes
	ldx #90 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.endif
.else
; 3652 to 3671
.if(n = 3652)
	; 9 bytes
	bit <$2C ;hides 'bit $A6A2'
	ldx #166
	brk
	dex
	Jne *-6
.elseif(n = 3653)
	; 6 bytes
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
.elseif(n = 3654 || n = 3656)
	; 7..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3655)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3657)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3658)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3659)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3660 || n = 3662)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3661)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3663)
	; 9 bytes
	ldx #<$2C ;hides 'bit $E5A2'
	ldx #229
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3664)
	; 9 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3665)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3666)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 3667)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	pha
	inx
	Jne *-6
.elseif(n = 3668)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3669)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3670)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3671)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 10
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672)
	; 10 bytes
	bit <$2C ;hides 'bit $CCA2'
	ldx #204
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3673 || n = 3691 || n = 3709)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3674)
	; 8 bytes
	ldx #(n-14)/15 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3675 || n = 3677 || n = 3683 || n = 3692 || n = 3693 || n = 3699 || n = 3703 || n = 3708)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3676 || n = 3706)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3678 || n = 3684)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3679)
	; 7 bytes
	ldx #184 ;hides 'clv'
	brk
	dex
	Jne *-4
.elseif(n = 3680)
	; 9 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3681 || n = 3701)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3682 || n = 3694 || n = 3700 || n = 3710)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3685)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3686 || n = 3688 || n = 3698 || n = 3702)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3687)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3689)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3690 || n = 3707)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3695)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 3696)
	; 9 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3697)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3704)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3705)
	; 8 bytes
	ldx #218 ;hides 'nop'
	delay_n 10
	dex
	Jne *-5
.endif
.else
; 3711 to 3750
.if(n = 3711)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3712 || n = 3730 || n = 3742 || n = 3746 || n = 3748)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3713)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3714 || n = 3732)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3715 || n = 3733)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3716 || n = 3734)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3717)
	; 9 bytes
	ldx #<$2C ;hides 'bit $A9A2'
	ldx #169
	brk
	dex
	Jne *-6
.elseif(n = 3718 || n = 3739)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3719)
	; 8 bytes
	ldx #248 ;hides 'sed'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3720)
	; 9 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3722)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3723)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3724 || n = 3741)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3725 || n = 3744)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3726 || n = 3729 || n = 3731 || n = 3737 || n = 3738 || n = 3747)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3727 || n = 3745)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3728)
	; 9 bytes
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3735)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
.elseif(n = 3736)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3740)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 3743)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3750)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
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
.if(n = 3751 || n = 3766)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3752 || n = 3788)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3753 || n = 3765 || n = 3767 || n = 3771 || n = 3783 || n = 3785 || n = 3789)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3754 || n = 3778 || n = 3784)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3755)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inc @zptemp
	inx
	Jne *-7
.elseif(n = 3756)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 3757)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3758 || n = 3775)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3759)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3760)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3761)
	; 8 bytes
	ldx #58 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3762)
	; 9 bytes
	bit <$2C ;hides 'bit $ABA2'
	ldx #171
	brk
	dex
	Jne *-6
.elseif(n = 3763 || n = 3781)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3764)
	; 9 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3768 || n = 3786)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3769 || n = 3787)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3770)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3772)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3773)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 3774)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 3776)
	; 9 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3777)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3779)
	; 9 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3780)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 3782)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.endif
.else
.if((n >= 3790) && (n < 3809))
; 3790 to 3808
.if(n = 3790 || n = 3794 || n = 3798)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3791)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3792)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3793)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3795)
	; 9 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3796)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3797)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3799)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3800)
	; 10 bytes
	ldx #<$2C ;hides 'bit $B5A2'
	ldx #181
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3801)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3802)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3803)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3804)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3805)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3806)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3807)
	; 9 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3808)
	; 9 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	delay_n 7
	inx
	Jne *-6
.endif
.else
; 3809 to 3828
.if(n = 3809 || n = 3826)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3810)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
.elseif(n = 3811)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3812)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3813 || n = 3819 || n = 3828)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3814)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3815)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 3816)
	; 10 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 9
	inx
	Jne *-7
.elseif(n = 3817)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3818)
	; 10 bytes
	ldx #44 ;hides 'bit abs'
	inc @zptemp
	delay_n 9
	inx
	Jne *-7
.elseif(n = 3820)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3821)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3822)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3823)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3824)
	; 9 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3825)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3827)
	; 9 bytes
	ldx #<$2C ;hides 'bit $AEA2'
	ldx #174
	brk
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3851)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3830 || n = 3848 || n = 3854 || n = 3866)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 3831)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3832 || n = 3859)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3833)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3834)
	; 10 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3835 || n = 3853)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3836 || n = 3838 || n = 3846 || n = 3847 || n = 3864)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3837 || n = 3845 || n = 3855 || n = 3862 || n = 3867)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3839 || n = 3858)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3840)
	; 9 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3841 || n = 3861)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3842)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3843 || n = 3860)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3844 || n = 3865)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3849)
	; 9 bytes
	ldx #<$2C ;hides 'bit $AFA2'
	ldx #175
	brk
	dex
	Jne *-6
.elseif(n = 3850)
	; 9 bytes
	bit <$2C ;hides 'bit $AFA2'
	ldx #175
	brk
	dex
	Jne *-6
.elseif(n = 3852)
	; 10 bytes
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3856)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
.elseif(n = 3857)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3863)
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 14
	dex
	Jne *-6
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3874 || n = 3880 || n = 3890 || n = 3892 || n = 3897)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3869)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3870)
	; 8 bytes
	ldx #194 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3871 || n = 3889)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3872)
	; 9 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3873 || n = 3875 || n = 3879 || n = 3883 || n = 3891 || n = 3893 || n = 3900)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3876)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 3877 || n = 3894)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3878)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3881 || n = 3901)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3882 || n = 3899)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3884)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3885)
	; 10 bytes
	bit <$2C ;hides 'bit $B9A2'
	ldx #185
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3886)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3887)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3888)
	; 9 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3895)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3896)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3898)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3902)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inc @zptemp
	dex
	Jne *-6
.elseif(n = 3903)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3904)
	; 9 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3905)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3906)
	; 9 bytes
	ldx #12 ;hides 'nop abs'
	inc @zptemp
	delay_n 7
	inx
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
; 3907 to 3945
.if(n = 3907 || n = 3925 || n = 3943)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3908 || n = 3909 || n = 3913 || n = 3927 || n = 3929 || n = 3930)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3922 || n = 3924 || n = 3940 || n = 3944)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3911 || n = 3928 || n = 3945)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3912 || n = 3933)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3914 || n = 3918 || n = 3932)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3915 || n = 3934)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3916)
	; 9 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	brk
	dex
	Jne *-6
.elseif(n = 3917 || n = 3939)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3919)
	; 8 bytes
	ldx #(n-15)/16 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3920)
	; 9 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3921 || n = 3941)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3923)
	; 9 bytes
	ldx #218 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 3926)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3931)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3935)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3936)
	; 9 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3937)
	; 8 bytes
	ldx #232 ;hides 'inx'
	brk
	inx
	inx
	Jne *-5
.elseif(n = 3938)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 3942)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 10
	inx
	Jne *-6
.endif
.else
.if((n >= 3946) && (n < 3965))
; 3946 to 3964
.if(n = 3946 || n = 3964)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3947 || n = 3954 || n = 3963)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3948)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 3949)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3950)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3951)
	; 9 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3952)
	; 9 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3953)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3955)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	inx
	Jne *-6
.elseif(n = 3956)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3957)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 3958)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3959)
	; 7 bytes
	ldx #58 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 3960)
	; 9 bytes
	bit <$2C ;hides 'bit $B4A2'
	ldx #180
	brk
	dex
	Jne *-6
.elseif(n = 3961)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3962)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.endif
.else
; 3965 to 3984
.if(n = 3965 || n = 3971)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3966)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3967)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3968)
	; 9 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3969)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3970)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3972)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3973 || n = 3982)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3974 || n = 3975)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 3976)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3977)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3978)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 10
	inx
	Jne *-6
.elseif(n = 3979)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3980)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 3981)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3983)
	; 9 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3984)
	; 9 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3986 || n = 3988 || n = 4004 || n = 4006 || n = 4022)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3987 || n = 3993 || n = 3998 || n = 4003 || n = 4014 || n = 4017 || n = 4019 || n = 4023)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3989)
	; 10 bytes
	ldx #<$2C ;hides 'bit $BEA2'
	ldx #190
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3990)
	; 10 bytes
	bit <$2C ;hides 'bit $BEA2'
	ldx #190
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3991 || n = 4012)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3992)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3994)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3995)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3996 || n = 4013)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3997 || n = 4015)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3999)
	; 7 bytes
	ldx #56 ;hides 'sec'
	brk
	inx
	Jne *-4
.elseif(n = 4000 || n = 4002 || n = 4018)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4001 || n = 4021)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4005)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4007)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	nop
	inx
	Jne *-6
.elseif(n = 4008)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 10
	dex
	Jne *-6
.elseif(n = 4009)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	pha
	inx
	Jne *-6
.elseif(n = 4010)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4011)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 4
	delay_n 6
	inx
	Jne *-6
.elseif(n = 4016)
	; 9 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4020)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4036 || n = 4044 || n = 4050 || n = 4062)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4025)
	; 9 bytes
	ldx #<$2C ;hides 'bit $B7A2'
	ldx #183
	brk
	dex
	Jne *-6
.elseif(n = 4026)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4027)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4028 || n = 4046)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4029 || n = 4048)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4030 || n = 4047)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4031)
	; 9 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4032)
	; 9 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4033 || n = 4051)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4034 || n = 4035 || n = 4037 || n = 4042 || n = 4043 || n = 4045 || n = 4053 || n = 4055 || n = 4060)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4038 || n = 4052 || n = 4056)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4039 || n = 4057)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4040 || n = 4058)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4049)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4054)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4059)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	brk
	Jne *-4
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
	; 9 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4064 || n = 4081 || n = 4098)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4065)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4066 || n = 4071 || n = 4073 || n = 4077 || n = 4083 || n = 4088 || n = 4089 || n = 4091 || n = 4100)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4067 || n = 4086)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4068)
	; 10 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4069 || n = 4087)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4070)
	; 9 bytes
	bit <$2C ;hides 'bit $B9A2'
	ldx #185
	brk
	dex
	Jne *-6
.elseif(n = 4072 || n = 4078 || n = 4084 || n = 4090 || n = 4099)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4074 || n = 4092)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4075 || n = 4096)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4076 || n = 4094)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4079)
	; 9 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4080)
	; 9 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4082)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4085)
	; 10 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4093)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4095)
	; 9 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 7
	dex
	Jne *-6
.elseif(n = 4097)
	; 9 bytes
	ldx #0
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4101)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.endif
.else
; 4102 to 4140
.if(n = 4102 || n = 4107 || n = 4109 || n = 4119 || n = 4125 || n = 4127 || n = 4134 || n = 4140)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4103)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4104 || n = 4106 || n = 4108 || n = 4120 || n = 4126)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4105 || n = 4123)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4110 || n = 4128)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4111 || n = 4129)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4112 || n = 4130)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4113)
	; 9 bytes
	ldx #<$2C ;hides 'bit $BBA2'
	ldx #187
	brk
	dex
	Jne *-6
.elseif(n = 4114)
	; 9 bytes
	bit <$2C ;hides 'bit $BBA2'
	ldx #187
	brk
	dex
	Jne *-6
.elseif(n = 4115 || n = 4132)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4116)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	inx
	Jne *-6
.elseif(n = 4117 || n = 4138)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4118 || n = 4136)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4121)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4122)
	; 10 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4124)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4131)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	inx
	Jne *-7
.elseif(n = 4133)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
	plp
.elseif(n = 4135)
	; 9 bytes
	ldx #<$2C ;hides 'bit $BCA2'
	ldx #188
	brk
	dex
	Jne *-6
.elseif(n = 4137)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4139)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 11
	inx
	Jne *-6
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4159 || n = 4177)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4142)
	; 10 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4143 || n = 4145 || n = 4150 || n = 4151 || n = 4160 || n = 4168 || n = 4170 || n = 4174 || n = 4179)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4144 || n = 4152 || n = 4162 || n = 4169)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4146 || n = 4171)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4147 || n = 4165 || n = 4167)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4148 || n = 4156 || n = 4173)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4149 || n = 4166)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4153)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4154 || n = 4172)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4155)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4157)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
.elseif(n = 4158)
	; 9 bytes
	php
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4161)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4163)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4164)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4175)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 4176)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4178)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $EAEA ;hides two 'nop's
	delay_n 7
	inx
	Jne *-7
.endif
.else
; 4180 to 4218
.if(n = 4180)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4181 || n = 4201)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4182 || n = 4188 || n = 4205 || n = 4218)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4183 || n = 4200 || n = 4217)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4184 || n = 4207)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4185 || n = 4197 || n = 4202 || n = 4215)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 4186)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4187 || n = 4210)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4189 || n = 4206)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4190 || n = 4196 || n = 4208)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4191 || n = 4209)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4192)
	; 10 bytes
	ldx #127
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 4193)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E9A2'
	ldx #233
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4194)
	; 10 bytes
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4195 || n = 4213)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4198 || n = 4204 || n = 4216)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4199)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 4203)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4211)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4212)
	; 10 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4214)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop
	brk
	inx
	inx
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4220 || n = 4228 || n = 4256 || n = 4274 || n = 4292)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4221 || n = 4241 || n = 4261 || n = 4281)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4222 || n = 4243 || n = 4264)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4223 || n = 4227 || n = 4233 || n = 4235 || n = 4236 || n = 4240 || n = 4250 || n = 4253 || n = 4259 || n = 4260 || n = 4263 || n = 4266 || n = 4269 || n = 4271 || n = 4278 || n = 4282 || n = 4287 || n = 4289 || n = 4293 || n = 4294)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4224)
	; 9 bytes
	bit <$2C ;hides 'bit $C0A2'
	ldx #192
	brk
	dex
	Jne *-6
.elseif(n = 4225 || n = 4291)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4226 || n = 4244 || n = 4275 || n = 4280)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4229)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 4230)
	; 10 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4231 || n = 4249 || n = 4267 || n = 4285)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4232)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	delay_n 6
	inx
	Jne *-7
.elseif(n = 4234 || n = 4251 || n = 4268)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4237 || n = 4255 || n = 4273)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4239 || n = 4254 || n = 4272 || n = 4286 || n = 4290)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4242)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
.elseif(n = 4245)
	; 9 bytes
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	brk
	dex
	Jne *-6
.elseif(n = 4246)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4247)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 4248 || n = 4279)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4252 || n = 4265 || n = 4270 || n = 4277 || n = 4284 || n = 4288)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4258)
	; 9 bytes
	ldx #194 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	dex
	Jne *-6
.elseif(n = 4262)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-4
.elseif(n = 4283)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4296)
	; 10 bytes
	bit <$2C ;hides 'bit $B3A2'
	ldx #179
	nop
	brk
	dex
	Jne *-7
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if(n = 4297)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4298 || n = 4316 || n = 4332 || n = 4334)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4299)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	delay_n 16
	inx
	Jne *-6
.elseif(n = 4300 || n = 4304 || n = 4305 || n = 4307 || n = 4318 || n = 4323 || n = 4325 || n = 4329)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4301)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4302)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4303 || n = 4321)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4306 || n = 4327)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4308)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4309)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4310 || n = 4326 || n = 4328)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4311)
	; 9 bytes
	ldx #<$2C ;hides 'bit $C4A2'
	ldx #196
	brk
	dex
	Jne *-6
.elseif(n = 4312)
	; 9 bytes
	bit <$2C ;hides 'bit $C4A2'
	ldx #196
	brk
	dex
	Jne *-6
.elseif(n = 4313 || n = 4335)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4314 || n = 4333)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4315)
	; 9 bytes
	ldx #90 ;hides 'nop'
	delay_n 19
	inx
	Jne *-6
.elseif(n = 4317 || n = 4322 || n = 4324 || n = 4330)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4319)
	; 7 bytes
	ldx #216 ;hides 'cld'
	brk
	dex
	Jne *-4
.elseif(n = 4320)
	; 10 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4331)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.else
; 4336 to 4375
.if(n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4337)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4338 || n = 4340 || n = 4350 || n = 4354 || n = 4363)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4339 || n = 4357 || n = 4375)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4341 || n = 4361)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4342 || n = 4358 || n = 4360 || n = 4362)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4343)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4344)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4345 || n = 4365)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4346 || n = 4364 || n = 4366)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4347)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	nop
	brk
	inx
	Jne *-6
.elseif(n = 4348)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4349 || n = 4370)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4351)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4352 || n = 4371)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4353)
	; 8 bytes
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4355)
	; 8 bytes
	ldx #58 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4356)
	; 9 bytes
	bit <$2C ;hides 'bit $C6A2'
	ldx #198
	brk
	dex
	Jne *-6
.elseif(n = 4359)
	; 7 bytes
	ldx #218 ;hides 'nop'
	brk
	dex
	Jne *-4
.elseif(n = 4367)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D0A2'
	ldx #208
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4368)
	; 10 bytes
	bit <$2C ;hides 'bit $D0A2'
	ldx #208
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4369)
	; 8 bytes
	ldx #26 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4372)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4373)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 4374)
	; 10 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 9
	inx
	Jne *-7
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
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4377 || n = 4379 || n = 4383 || n = 4385 || n = 4392 || n = 4395 || n = 4397 || n = 4405 || n = 4413)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4378 || n = 4384 || n = 4396 || n = 4402 || n = 4404 || n = 4410 || n = 4414)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4380)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4381 || n = 4401)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4382 || n = 4408)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4386)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
.elseif(n = 4387)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	pha
	dex
	Jne *-6
.elseif(n = 4388 || n = 4406)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4389)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 6
	dex
	Jne *-6
.elseif(n = 4390)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4391)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4393 || n = 4411)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4394)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4398 || n = 4412)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4400)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4403)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4407)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4409)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.endif
.else
; 4415 to 4453
.if(n = 4415)
	; 9 bytes
	php
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4416 || n = 4452)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4417 || n = 4440)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4418 || n = 4436)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4419 || n = 4431 || n = 4433 || n = 4437 || n = 4438 || n = 4443 || n = 4449)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4420)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4421 || n = 4441)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4422)
	; 9 bytes
	bit <$2C ;hides 'bit $C9A2'
	ldx #201
	brk
	dex
	Jne *-6
.elseif(n = 4423)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4424 || n = 4442)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4425)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4426)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4427)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4428)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4429 || n = 4447)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4430)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4432 || n = 4453)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4434)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4435)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4439)
	; 10 bytes
	ldx #<$2C ;hides 'bit $B9A2'
	ldx #185
	nop
	brk
	dex
	Jne *-7
.elseif(n = 4444)
	; 9 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	brk
	dex
	Jne *-6
.elseif(n = 4445)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4446)
	; 10 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4448 || n = 4450)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4451)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454 || n = 4490)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4455 || n = 4467 || n = 4469 || n = 4485 || n = 4491 || n = 4492)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4456 || n = 4457 || n = 4468 || n = 4475 || n = 4477 || n = 4479 || n = 4484 || n = 4486)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4458 || n = 4460 || n = 4478)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4459)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	inx
	inx
	Jne *-7
.elseif(n = 4461 || n = 4481)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4462)
	; 9 bytes
	ldx #n/23 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	brk
	dex
	Jne *-6
.elseif(n = 4463)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4464)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 10
	inx
	Jne *-6
.elseif(n = 4465 || n = 4483)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4466)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4470 || n = 4488)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4471)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4472)
	; 9 bytes
	ldx #(n-20)/21 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4473)
	; 10 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4474)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4476)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4480)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inc @zptemp
	inx
	Jne *-7
.elseif(n = 4482)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4487)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4489)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.endif
.else
; 4493 to 4531
.if(n = 4493)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D6A2'
	ldx #214
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4494)
	; 10 bytes
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4495 || n = 4516)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4496 || n = 4527)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4497 || n = 4503 || n = 4505 || n = 4512 || n = 4518 || n = 4529)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4498 || n = 4504 || n = 4522 || n = 4530)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4499)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4500)
	; 10 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4501 || n = 4519)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4502)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 10
	inx
	Jne *-6
.elseif(n = 4506 || n = 4524)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4507 || n = 4525)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4508 || n = 4526 || n = 4528)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4509)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4510)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4511)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4513)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4514)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4515)
	; 10 bytes
	bit <$2C ;hides 'bit $D7A2'
	ldx #215
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4517)
	; 10 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4520)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4521)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4523)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4531)
	; 9 bytes
	ldx #<$2C ;hides 'bit $CEA2'
	ldx #206
	brk
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if(n = 4532 || n = 4550 || n = 4568)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4533)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4534 || n = 4539 || n = 4543 || n = 4545 || n = 4546 || n = 4552 || n = 4557 || n = 4559 || n = 4563 || n = 4565 || n = 4570)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4535)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 4536)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 16
	inx
	Jne *-6
.elseif(n = 4537 || n = 4555)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4538 || n = 4540 || n = 4556 || n = 4564)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4541 || n = 4561)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4542)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4544 || n = 4548 || n = 4562)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4547 || n = 4567)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4549)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4551)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4553)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 16
	inx
	Jne *-5
.elseif(n = 4554)
	; 9 bytes
	bit <$2C ;hides 'bit $CFA2'
	ldx #207
	brk
	dex
	Jne *-6
.elseif(n = 4558)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4560 || n = 4566)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4569)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.else
; 4571 to 4609
.if(n = 4571)
	; 10 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4572)
	; 10 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4573 || n = 4591)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4574)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4575 || n = 4582 || n = 4584 || n = 4588 || n = 4589 || n = 4593 || n = 4595 || n = 4600 || n = 4602 || n = 4606 || n = 4607)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 4576 || n = 4583 || n = 4590 || n = 4594 || n = 4601 || n = 4608)
	; 7..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4577)
	; 9 bytes
	ldx #218 ;hides 'nop'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 4578)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4579)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4580)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 4581)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4585 || n = 4596 || n = 4603)
	; 8..9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4586 || n = 4597 || n = 4604)
	; 8..9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4587 || n = 4605)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4592)
	; 9 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4598)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
.elseif(n = 4599)
	; 7 bytes
	ldx #26 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 4609)
	; 7 bytes
	ldx #0
	brk
	dex
	Jne *-3
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610)
	; 9 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4611 || n = 4613 || n = 4617 || n = 4623 || n = 4625)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4638)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4614 || n = 4644)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4615 || n = 4645)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4616 || n = 4627 || n = 4628 || n = 4646 || n = 4648)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4618 || n = 4637)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4619)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D2A2'
	ldx #210
	brk
	dex
	Jne *-6
.elseif(n = 4620)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4621 || n = 4641)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4622)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4624 || n = 4647)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4626)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4629)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 4630)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	dex
	Jne *-6
.elseif(n = 4631)
	; 10 bytes
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	nop
	brk
	dex
	Jne *-7
.elseif(n = 4632)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inc @zptemp
	dex
	Jne *-7
.elseif(n = 4633)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 4634)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 4635)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	delay_n 10
	dex
	Jne *-6
.elseif(n = 4636)
	; 10 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4639)
	; 7 bytes
	ldx #24 ;hides 'clc'
	brk
	inx
	Jne *-4
.elseif(n = 4640)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DDA2'
	ldx #221
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4642)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4643)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4653 || n = 4654 || n = 4658 || n = 4667 || n = 4674 || n = 4677 || n = 4678 || n = 4683)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4650)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4651)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4652)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4655 || n = 4660 || n = 4671 || n = 4682)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4656 || n = 4675)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4657)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4659 || n = 4686)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4661 || n = 4681)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4662)
	; 10 bytes
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4663)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4664)
	; 9 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	brk
	inx
	Jne *-6
.elseif(n = 4665 || n = 4687)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4666)
	; 9 bytes
	ldx #44 ;hides 'bit abs'
	inc @zptemp
	brk
	inx
	Jne *-6
.elseif(n = 4668)
	; 8 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 16
	Jne *-5
.elseif(n = 4669)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4670)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4672)
	; 9 bytes
	php
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4673)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 10
	inx
	Jne *-7
.elseif(n = 4676)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4679)
	; 7 bytes
	ldx #234 ;hides 'nop'
	brk
	dex
	Jne *-4
.elseif(n = 4680)
	; 10 bytes
	bit <$2C ;hides 'bit $C3A2'
	ldx #195
	nop
	brk
	dex
	Jne *-7
.elseif(n = 4684)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4685)
	; 8 bytes
	ldx #(n-21)/22 ;hides 'nop zp,x'
	inx ;first loop only
	brk
	dex
	Jne *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if(n = 4688 || n = 4697 || n = 4704 || n = 4714 || n = 4724)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4689 || n = 4695 || n = 4696 || n = 4703 || n = 4710 || n = 4715 || n = 4723)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4690)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4691)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	delay_n 14
	inx
	Jne *-7
.elseif(n = 4692 || n = 4698)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4693 || n = 4716)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4694 || n = 4713)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4699)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 11
	inx
	Jne *-7
.elseif(n = 4700)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	delay_n 4
	nop
	brk
	inx
	Jne *-7
.elseif(n = 4701 || n = 4721)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4702)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4705 || n = 4726)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4706)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4707)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D6A2'
	ldx #214
	brk
	dex
	Jne *-6
.elseif(n = 4708)
	; 9 bytes
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	brk
	dex
	Jne *-6
.elseif(n = 4709)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4711)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4712)
	; 10 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 10
	inx
	Jne *-7
.elseif(n = 4717)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4718)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4719)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	delay_n 11
	inx
	Jne *-7
.elseif(n = 4720)
	; 10 bytes
	ldx #143
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 4722)
	; 10 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 4725)
	; 10 bytes
	bit <$2C ;hides 'bit $E1A2'
	ldx #225
	jsr @nearby_rts
	dex
	Jne *-7
.endif
.else
; 4727 to 4765
.if(n = 4727 || n = 4755 || n = 4757)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4728 || n = 4733 || n = 4734 || n = 4743 || n = 4763 || n = 4765)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4729)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	brk
	dex
	Jne *-6
.elseif(n = 4730)
	; 9 bytes
	bit <$2C ;hides 'bit $D7A2'
	ldx #215
	brk
	dex
	Jne *-6
.elseif(n = 4731 || n = 4753)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4732)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4735 || n = 4750 || n = 4754 || n = 4764)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n > 4735 && n < 4739) || n = 4740)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4739 || n = 4762)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4741 || n = 4761)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4742)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dex
	Jne *-7
.elseif(n = 4744)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4745)
	; 10 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4746)
	; 10 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4747)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4748 || n = 4756 || n = 4758)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4751)
	; 8 bytes
	ldx #216 ;hides 'cld'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4752)
	; 9 bytes
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	brk
	dex
	Jne *-6
.elseif(n = 4759)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 4760)
	; 10 bytes
	brk
	delay_n (n-13)
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
.if((n >= 4766) && (n < 4785))
; 4766 to 4784
.if(n = 4766)
	; 10 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4767)
	; 10 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4768)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4769)
	; 10 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	inx
	Jne *-7
.elseif(n = 4770)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4771 || n = 4784)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4772 || n = 4778 || n = 4779 || n = 4783)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4773)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	brk
	dex
	Jne *-6
.elseif(n = 4774)
	; 9 bytes
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	brk
	dex
	Jne *-6
.elseif(n = 4775)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4776)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4777)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4780)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 4781)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4782)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.else
; 4785 to 4804
.if(n = 4785)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4786)
	; 10 bytes
	ldx #145
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 4787)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E4A2'
	ldx #228
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4788)
	; 10 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4789)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4790)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4791 || n = 4793 || n = 4803)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4792 || n = 4798 || n = 4804)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4794)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4795)
	; 8 bytes
	ldx #218 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4796)
	; 9 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	brk
	dex
	Jne *-6
.elseif(n = 4797)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4799)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	brk
	inx
	Jne *-6
.elseif(n = 4800)
	; 10 bytes
	bit <$2C ;hides 'bit $C8A2'
	ldx #200
	nop
	brk
	dex
	Jne *-7
.elseif(n = 4801)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4802)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 4805 to 4843
.if(n = 4805 || n = 4812 || n = 4815 || n = 4820 || n = 4823 || n = 4833 || n = 4836 || n = 4842 || n = 4843)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4806 || n = 4837)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4807)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 4808 || n = 4827)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4809)
	; 10 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4810 || n = 4831)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4811 || n = 4813 || n = 4822 || n = 4824 || n = 4834)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4814)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4816)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	brk
	dex
	Jne *-5
	plp
.elseif(n = 4817)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	brk
	dex
	Jne *-6
.elseif(n = 4818)
	; 9 bytes
	bit <$2C ;hides 'bit $DBA2'
	ldx #219
	brk
	dex
	Jne *-6
.elseif(n = 4819)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4821 || n = 4841)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4825)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4826)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4828)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4829)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 4830)
	; 10 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4832)
	; 10 bytes
	ldx #232 ;hides 'inx'
	inc @zptemp
	brk
	inx
	inx
	Jne *-7
.elseif(n = 4835)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	inx
	Jne *-7
.elseif(n = 4838)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4839)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DCA2'
	ldx #220
	brk
	dex
	Jne *-6
.elseif(n = 4840)
	; 9 bytes
	bit <$2C ;hides 'bit $DCA2'
	ldx #220
	brk
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4855 || n = 4864 || n = 4874)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4845 || n = 4848 || n = 4853 || n = 4856 || n = 4857 || n = 4867 || n = 4875)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4846)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4847)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4849)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4850)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
.elseif(n = 4851 || n = 4876)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4852 || n = 4873)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4854 || n = 4877)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4858)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4859)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 11
	dex
	Jne *-7
.elseif(n = 4860 || n = 4869 || n = 4882)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4861 || n = 4881)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4862)
	; 9 bytes
	bit <$2C ;hides 'bit $DDA2'
	ldx #221
	brk
	dex
	Jne *-6
.elseif(n = 4863)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4865)
	; 9 bytes
	ldx #0
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4866)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4868)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4870)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dex
	Jne *-7
.elseif(n = 4871)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 19
	Jne *-5
.elseif(n = 4872)
	; 10 bytes
	bit <$2C ;hides 'bit $E8A2'
	ldx #232
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4878)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4879)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	delay_n 11
	dex
	Jne *-7
.elseif(n = 4880)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.endif
.else
; 4883 to 4921
.if(n = 4883 || n = 4887 || n = 4896 || n = 4902 || n = 4903 || n = 4909 || n = 4917)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4884)
	; 9 bytes
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	brk
	dex
	Jne *-6
.elseif(n = 4885 || n = 4907)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4886)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4888 || n = 4908)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4889)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 4890 || n = 4911 || n = 4912)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4891)
	; 10 bytes
	ldx #163
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 4892)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E9A2'
	ldx #233
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4893)
	; 10 bytes
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4894 || n = 4915)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4895)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 4
	brk
	inx
	Jne *-6
.elseif(n = 4897)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4898)
	; 10 bytes
	ldx #(n-22)/23 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 14
	dex
	Jne *-7
.elseif(n = 4899)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4900)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4901 || n = 4921)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4904 || n = 4910 || n = 4916 || n = 4918)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4905)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	brk
	dex
	Jne *-6
.elseif(n = 4906)
	; 9 bytes
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	brk
	dex
	Jne *-6
.elseif(n = 4913)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 4914)
	; 10 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4919)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	brk
	inx
	Jne *-6
.elseif(n = 4920)
	; 10 bytes
	bit <$2C ;hides 'bit $CDA2'
	ldx #205
	nop
	brk
	dex
	Jne *-7
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
.if((n >= 4922) && (n < 4941))
; 4922 to 4940
.if(n = 4922)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4923 || n = 4925 || n = 4931 || n = 4938 || n = 4940)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4924 || n = 4932 || n = 4933 || n = 4939)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4926)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4927)
	; 9 bytes
	ldx #<$2C ;hides 'bit $E0A2'
	ldx #224
	brk
	dex
	Jne *-6
.elseif(n = 4928)
	; 9 bytes
	bit <$2C ;hides 'bit $E0A2'
	ldx #224
	brk
	dex
	Jne *-6
.elseif(n = 4929)
	; 9 bytes
	php
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4930)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4934)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4935)
	; 10 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4936)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4937)
	; 10 bytes
	pha
	ldx #9 ;hides 'ora #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
	pla
.endif
.else
; 4941 to 4960
.if(n = 4941)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4942)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 4943 || n = 4953)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4944 || n = 4960)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 4945)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4946)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4947)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4948)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4949)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	brk
	inx
	Jne *-6
.elseif(n = 4950)
	; 9 bytes
	bit <$2C ;hides 'bit $E1A2'
	ldx #225
	brk
	dex
	Jne *-6
.elseif(n = 4951)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4952)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	delay_n 7
	inx
	Jne *-7
.elseif(n = 4954)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4955)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4956)
	; 10 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4957)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4958)
	; 10 bytes
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	brk
	inx
	Jne *-7
.elseif(n = 4959)
	; 7 bytes
	ldx #248 ;hides 'sed'
	brk
	dex
	Jne *-4
.endif
.endif
.else
; 4961 to 5000
.if(n = 4961 || n = 4981)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4962 || n = 4970 || n = 4979 || n = 4984)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4963 || n = 4975 || n = 4980 || n = 4983 || n = 4985)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4964 || n = 4986)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4965 || n = 4987)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4966 || n = 4968 || n = 4988)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4967)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4969)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 4971)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	brk
	inx
	Jne *-6
.elseif(n = 4972 || n = 4974)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4973 || n = 4995)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4976)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4977)
	; 10 bytes
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4978)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4982)
	; 10 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	inx
	Jne *-7
.elseif(n = 4989 || n = 4990)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4991)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D0A2'
	ldx #208
	nop
	brk
	dex
	Jne *-7
.elseif(n = 4992)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4993)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	brk
	inx
	Jne *-6
.elseif(n = 4994)
	; 9 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	brk
	inx
	Jne *-6
.elseif(n = 4996)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 4997)
	; 10 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4998)
	; 10 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4999)
	; 7 bytes
	ldx #250 ;hides 'nop'
	brk
	dex
	Jne *-4
.elseif(n = 5000)
	; 10 bytes
	bit <$2C ;hides 'bit $C8A2'
	ldx #200
	delay_n 16
	dex
	Jne *-7
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
.if(n = 5001 || n = 5021)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5002 || n = 5009 || n = 5024 || n = 5029)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5003 || n = 5022 || n = 5025)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5004)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5005 || n = 5007 || n = 5027)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 5006 || n = 5008 || n = 5028)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5010 || (n > 5029 && n < 5033) || n = 5035 || n = 5036)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5011)
	; 10 bytes
	ldx #167
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5012 || n = 5014 || n = 5034)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5013)
	; 8 bytes
	ldx #218 ;hides 'nop'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5015 || n = 5038)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5016)
	; 9 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	brk
	dex
	Jne *-6
.elseif(n = 5017 || n = 5039)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5018)
	; 10 bytes
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5019)
	; 10 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5020)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5023)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5026)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5033)
	; 10 bytes
	ldx #148
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5037)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	brk
	inx
	Jne *-6
.endif
.else
; 5040 to 5078
.if(n = 5040)
	; 10 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5041 || n = 5061)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5042 || n = 5044 || n = 5052 || n = 5057 || n = 5077)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5043 || n = 5045 || n = 5053 || n = 5056 || n = 5063 || n = 5064 || n = 5078)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5046 || n = 5066)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5047 || n = 5067)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 5048 || n = 5068)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5049)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	inx
	Jne *-6
.elseif(n = 5050)
	; 10 bytes
	ldx #153
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5051 || n = 5076)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5054)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5055 || n = 5070 || n = 5073)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5058)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5059)
	; 8 bytes
	ldx #26 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5060)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	brk
	inx
	Jne *-6
.elseif(n = 5062)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5065)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5069)
	; 10 bytes
	ldx #181
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5071)
	; 10 bytes
	ldx #169
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5072 || n = 5075)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5074)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5095 || n = 5115)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5080 || n = 5096)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 5081 || n = 5101)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5082)
	; 9 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	brk
	inx
	Jne *-6
.elseif(n = 5083 || n = 5104)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5084 || n = 5107)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5085)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5086)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	dex
	Jne *-6
.elseif(n = 5087)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 4
	brk
	dex
	Jne *-6
.elseif(n = 5088)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	pha
	inx
	Jne *-6
.elseif(n = 5089)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5090)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 5091 || n = 5092 || n = 5094 || n = 5106 || n = 5111 || n = 5112 || n = 5114)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5093 || n = 5100 || n = 5113)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5097 || n = 5108 || n = 5116 || n = 5117)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5098)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	brk
	inx
	Jne *-6
.elseif(n = 5099)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	delay_n 16
	inx
	Jne *-6
.elseif(n = 5102)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5103)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5105)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5109)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 5110)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	dex
	Jne *-4
.endif
.else
; 5118 to 5156
.if(n = 5118)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inc @zptemp
	dex
	Jne *-7
.elseif(n = 5119 || n = 5129 || n = 5132 || n = 5139 || n = 5155)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5120)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	delay_n 7
	dex
	Jne *-7
.elseif(n = 5121)
	; 8 bytes
	ldx #0
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5122 || n = 5134 || n = 5138)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5123)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5124 || n = 5128 || n = 5140 || n = 5150)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5125 || n = 5146)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5126 || n = 5151)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5127 || n = 5149)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5130 || n = 5153)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5131)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5133 || n = 5156)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5135)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D6A2'
	ldx #214
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5136)
	; 10 bytes
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5137)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif((n > 5140 && n < 5144))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5144)
	; 9 bytes
	ldx #(n-20)/21 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5145)
	; 10 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5147)
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5148)
	; 9 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	brk
	dex
	Jne *-6
.elseif(n = 5152)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5154)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
; 5157 to 5195
.if(n = 5157 || n = 5162 || n = 5168 || n = 5177)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5158)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5159)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5160)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5161 || n = 5185)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5163 || n = 5173 || n = 5182 || n = 5190 || n = 5195)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5164 || n = 5172 || n = 5179 || n = 5181 || n = 5189 || n = 5194)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5165)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5166)
	; 10 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5167 || n = 5188)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5169)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5170)
	; 9 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	brk
	inx
	Jne *-6
.elseif(n = 5171 || n = 5193)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5174)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5175)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 5176)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5178)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5180)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 5183)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 4
	brk
	dex
	Jne *-6
.elseif(n = 5184)
	; 10 bytes
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5186)
	; 9 bytes
	php
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5187)
	; 10 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5191)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5192)
	; 9 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	brk
	dex
	Jne *-6
.endif
.else
.if((n >= 5196) && (n < 5215))
; 5196 to 5214
.if(n = 5196)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5197)
	; 9 bytes
	ldx #226 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	brk
	dex
	Jne *-6
.elseif(n = 5198)
	; 9 bytes
	ldx #n/23 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	brk
	dex
	Jne *-6
.elseif(n = 5199)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5200)
	; 10 bytes
	bit <$2C ;hides 'bit $D0A2'
	ldx #208
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5201)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5202)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5203 || n = 5211)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5204)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5205 || n = 5206)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5207)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 5208)
	; 10 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5209)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5210 || n = 5212)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5213)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5214)
	; 9 bytes
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	brk
	dex
	Jne *-6
.endif
.else
; 5215 to 5234
.if(n = 5215)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5216)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 5217 || n = 5224 || n = 5232)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5218)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5219)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 5220)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5221)
	; 10 bytes
	ldx #174
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5222)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5223)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5225)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5226)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5227)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5228)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5229)
	; 10 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5230)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5231)
	; 9 bytes
	ldx #218 ;hides 'nop'
	delay_n 4
	brk
	dex
	Jne *-6
.elseif(n = 5233)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5234)
	; 10 bytes
	ldx #194 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
; 5235 to 5273
.if(n = 5235)
	; 9 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	brk
	dex
	Jne *-6
.elseif(n = 5236)
	; 9 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	brk
	inx
	Jne *-6
.elseif(n = 5237 || n = 5259)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5238 || n = 5242 || n = 5247 || n = 5253 || n = 5260 || n = 5261 || n = 5273)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5239 || n = 5266)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5240 || n = 5271)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5241 || n = 5244 || n = 5246 || n = 5263)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5243 || n = 5252 || n = 5254 || n = 5262)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5245 || n = 5268)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5248)
	; 10 bytes
	ldx #159
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5249)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 5250)
	; 10 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5251 || n = 5272)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5255)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5256)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 16
	inx
	Jne *-6
.elseif(n = 5257)
	; 9 bytes
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	brk
	inx
	Jne *-6
.elseif(n = 5258)
	; 9 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	brk
	dex
	Jne *-6
.elseif(n = 5264)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	delay_n 9
	brk
	inx
	Jne *-7
.elseif(n = 5265)
	; 10 bytes
	ldx #188
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5267)
	; 10 bytes
	pha
	ldx #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
	pla
.elseif(n = 5269)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	pha
	inx
	Jne *-6
.elseif(n = 5270)
	; 10 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	jsr @nearby_rts
	inx
	Jne *-7
.endif
.else
; 5274 to 5312
.if(n = 5274 || n = 5282 || n = 5283 || n = 5290 || n = 5295 || n = 5307)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5275 || n = 5284 || n = 5292 || n = 5296 || n = 5308)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5276 || n = 5301)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5277)
	; 9 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 19
	Jne *-6
.elseif(n = 5278)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	brk
	inx
	Jne *-6
.elseif(n = 5279)
	; 9 bytes
	ldx #<$2C ;hides 'bit $F0A2'
	ldx #240
	brk
	dex
	Jne *-6
.elseif(n = 5280)
	; 9 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	brk
	dex
	Jne *-6
.elseif(n = 5281 || n = 5303)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5285 || n = 5306)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5286 || n = 5287 || n = 5309 || n = 5310)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5288)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	pha
	dex
	Jne *-6
.elseif(n = 5289)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 16
	inx
	Jne *-5
.elseif(n = 5291)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5293)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5294)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5297)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	brk
	dex
	Jne *-6
.elseif(n = 5298)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	brk
	dex
	Jne *-6
.elseif(n = 5299)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5300)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5302)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5304)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 16
	inx
	Jne *-6
.elseif(n = 5305)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5311)
	; 10 bytes
	ldx #177
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5312)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 14
	dex
	dex
	Jne *-7
.endif
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
.if(n = 5313)
	; 10 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5314 || n = 5335)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5315 || n = 5322 || n = 5330 || n = 5343 || n = 5344)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5316 || n = 5318 || n = 5319 || n = 5327 || n = 5328 || n = 5339 || n = 5340 || n = 5347 || n = 5349 || n = 5350)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5317 || n = 5332 || n = 5336 || n = 5338 || n = 5348)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5320)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5321)
	; 10 bytes
	ldx #190
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5323)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
.elseif(n = 5324)
	; 9 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
.elseif(n = 5325)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5326 || n = 5351)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5329)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5331)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5333)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5334)
	; 10 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5337)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5341)
	; 10 bytes
	ldx #178
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5342)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5345)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 5346)
	; 9 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	brk
	inx
	Jne *-6
.endif
.else
.if((n >= 5352) && (n < 5371))
; 5352 to 5370
.if(n = 5352)
	; 9 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5353)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5354)
	; 10 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5355)
	; 10 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5356)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 5357)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5358 || n = 5362 || n = 5368)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5359)
	; 9 bytes
	pha
	ldx #255
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5360)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5361)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5363)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5364)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 5365)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 5366)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 5367)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5369)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5370)
	; 9 bytes
	ldx #12 ;hides 'nop abs'
	inc @zptemp
	brk
	inx
	Jne *-6
.endif
.else
; 5371 to 5390
.if(n = 5371 || n = 5379 || n = 5382 || n = 5385)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5372 || n = 5387)
	; 9..10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5373 || n = 5388)
	; 9..10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5374)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5375)
	; 10 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5376)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5377)
	; 8 bytes
	ldx #0
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5378)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 5380 || n = 5384)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 5381)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5383)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5386)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5389)
	; 8 bytes
	ldx #(n-21)/22 ;hides 'nop zp,x'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 5390)
	; 9 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	brk
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
; 5391 to 5429
.if(n = 5391 || n = 5413)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5392 || n = 5398 || n = 5416)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5393 || n = 5397 || n = 5403 || n = 5408 || n = 5414 || n = 5415 || n = 5428)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5394)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5395 || n = 5426)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5396)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5399)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	brk
	inx
	Jne *-6
.elseif(n = 5400)
	; 10 bytes
	bit <$2C ;hides 'bit $E1A2'
	ldx #225
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5401)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5402)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5404)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 5405)
	; 10 bytes
	ldx #193
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5406 || n = 5429)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5407)
	; 10 bytes
	ldx #159
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5409)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5410 || (n > 5416 && n < 5422))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5411)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	brk
	inx
	Jne *-6
.elseif(n = 5412)
	; 9 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	brk
	dex
	Jne *-6
.elseif(n = 5422)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	inx
	Jne *-7
.elseif(n = 5423)
	; 10 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5424)
	; 10 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5425)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5427)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	delay_n 14
	inx
	Jne *-7
.endif
.else
; 5430 to 5468
.if(n = 5430 || n = 5439 || n = 5440 || n = 5442 || n = 5444 || (n > 5462 && n < 5468))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5431 || n = 5436 || n = 5437 || n = 5445 || n = 5453 || n = 5454 || n = 5459)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5432)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5433)
	; 9 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	brk
	inx
	Jne *-6
.elseif(n = 5434)
	; 9 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	brk
	dex
	Jne *-6
.elseif(n = 5435)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5438 || n = 5446 || n = 5458)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5441)
	; 10 bytes
	ldx #160
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5443)
	; 9 bytes
	php
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5447)
	; 10 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5448)
	; 10 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5449)
	; 9 bytes
	ldx #218 ;hides 'nop'
	inc @zptemp
	brk
	dex
	Jne *-6
.elseif(n = 5450)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 5451)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5452)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5455)
	; 8 bytes
	ldx #248 ;hides 'sed'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5456)
	; 9 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	brk
	inx
	Jne *-6
.elseif(n = 5457)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5460)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5461)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 5462)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5468)
	; 10 bytes
	brk
	delay_n (n-13)
.endif
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
.if((n >= 5469) && (n < 5508))
.if((n >= 5469) && (n < 5488))
; 5469 to 5487
.if(n = 5469 || n = 5470 || n = 5484 || n = 5486)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5471)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E4A2'
	ldx #228
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5472)
	; 10 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5473)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5474)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5475)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5476)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5477)
	; 9 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	brk
	inx
	Jne *-6
.elseif(n = 5478)
	; 9 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	brk
	inx
	Jne *-6
.elseif(n = 5479)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5480)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 9
	brk
	Jne *-6
.elseif(n = 5481)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5482)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5483)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5485)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	brk
	dex
	Jne *-7
.elseif(n = 5487)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.endif
.else
; 5488 to 5507
.if(n = 5488)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5489)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	inx
	Jne *-7
.elseif(n = 5490 || n = 5503)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5491)
	; 10 bytes
	ldx #183
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif((n > 5491 && n < 5495))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5495)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E5A2'
	ldx #229
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5496)
	; 10 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5497)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5498)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5499)
	; 8 bytes
	ldx #250 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5500)
	; 9 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	brk
	inx
	Jne *-6
.elseif(n = 5501)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5502)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 5504)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5505)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 5506)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5507)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	inc @zptemp
	brk
	inx
	Jne *-7
.endif
.endif
.else
; 5508 to 5546
.if(n = 5508 || (n > 5513 && n < 5517) || n = 5518 || n = 5527 || (n > 5529 && n < 5534) || n = 5535 || n = 5538 || (n > 5539 && n < 5543))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5509 || n = 5536)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5510)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5511)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 4
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5512)
	; 10 bytes
	ldx #167
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5513 || n = 5539)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5517)
	; 10 bytes
	ldx #197
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5519)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5520)
	; 10 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5521)
	; 9 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	brk
	inx
	Jne *-6
.elseif(n = 5522)
	; 9 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	brk
	inx
	Jne *-6
.elseif(n = 5523 || n = 5545)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5524 || n = 5525 || n = 5528 || n = 5546)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5526)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5529)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5534)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inc @zptemp
	brk
	inx
	Jne *-7
.elseif(n = 5537)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	brk
	dex
	Jne *-7
.elseif(n = 5543)
	; 9 bytes
	ldx #232 ;hides 'inx'
	nop
	brk
	dex
	dex
	Jne *-6
.elseif(n = 5544)
	; 9 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	brk
	inx
	Jne *-6
.endif
.endif
.else
.if((n >= 5547) && (n < 5586))
; 5547 to 5585
.if(n = 5547 || n = 5552 || n = 5553 || n = 5568 || n = 5571 || n = 5583)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5548 || n = 5554 || n = 5570 || n = 5572 || n = 5579 || n = 5584)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5549)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5550 || n = 5581)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5551 || n = 5576)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif((n > 5554 && n < 5563) || n = 5564 || n = 5580 || n = 5582 || n = 5585)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5563)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5565)
	; 9 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	brk
	inx
	Jne *-6
.elseif(n = 5566)
	; 9 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	brk
	inx
	Jne *-6
.elseif(n = 5567)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5569)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5573)
	; 10 bytes
	ldx #199
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5574)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5575)
	; 10 bytes
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5577)
	; 10 bytes
	ldx #164
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5578)
	; 10 bytes
	ldx #169
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.else
; 5586 to 5625
.if(n = 5586 || n = 5594 || n = 5597 || n = 5598 || n = 5608)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5587)
	; 9 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	brk
	inx
	Jne *-6
.elseif(n = 5588)
	; 9 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	brk
	inx
	Jne *-6
.elseif(n = 5589 || n = 5611)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5590 || n = 5613)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5591 || n = 5592 || n = 5595 || n = 5602 || n = 5604 || n = 5614 || n = 5619 || n = 5623 || n = 5624)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5593 || n = 5617)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5596 || n = 5603 || n = 5618 || n = 5625)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	nop
	delay_n 19
	inx
	Jne *-7
.elseif(n = 5600)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 5601)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5605)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5606)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dex
	Jne *-7
.elseif(n = 5607)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5609)
	; 9 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	brk
	inx
	Jne *-6
.elseif(n = 5610)
	; 9 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	brk
	inx
	Jne *-6
.elseif(n = 5612)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5615)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	brk
	dex
	Jne *-6
.elseif(n = 5616)
	; 10 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5620)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	dex
	Jne *-5
.elseif(n = 5621)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 5622)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	nop
	dex
	Jne *-5
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
.if(n = 5626 || n = 5651)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5627)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5628 || n = 5629)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5630 || n = 5637 || n = 5647 || n = 5656 || n = 5658 || n = 5660)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5631)
	; 9 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	brk
	dex
	Jne *-6
.elseif(n = 5632)
	; 9 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	brk
	inx
	Jne *-6
.elseif(n = 5633)
	; 9 bytes
	ldx #0
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5634)
	; 10 bytes
	ldx #(n-22)/23 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 14
	dex
	Jne *-7
.elseif(n = 5635 || n = 5638 || n = 5645 || n = 5653 || n = 5654 || n = 5661)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5636 || n = 5659)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5639)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	brk
	inx
	Jne *-6
.elseif(n = 5640)
	; 10 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5641)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5642 || n = 5646 || n = 5655)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5643)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5644)
	; 10 bytes
	ldx #171
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5648)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	brk
	dex
	Jne *-7
.elseif(n = 5649)
	; 10 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5650)
	; 10 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5652)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	inx
	Jne *-6
.elseif(n = 5657)
	; 10 bytes
	ldx #202
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5662)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	inx
	Jne *-6
.elseif(n = 5663)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 4
	brk
	inx
	Jne *-6
.elseif(n = 5664)
	; 10 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	nop
	brk
	dex
	Jne *-7
.endif
.else
; 5665 to 5703
.if(n = 5665 || n = 5689)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5666 || n = 5668 || n = 5670 || n = 5686 || n = 5690 || n = 5692)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5667)
	; 9 bytes
	ldx #218 ;hides 'nop'
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5669 || n = 5695)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5671 || n = 5698)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5672 || n = 5673 || n = 5680 || n = 5681 || n = 5693 || n = 5697)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5674)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5675)
	; 10 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5676 || n = 5701)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5677)
	; 10 bytes
	ldx #172
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5678 || n = 5684 || n = 5691 || n = 5702)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5679)
	; 10 bytes
	ldx #167
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5682)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5683)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 23
	Jne *-6
.elseif(n = 5685)
	; 10 bytes
	ldx #203
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5687)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	brk
	inx
	Jne *-6
.elseif(n = 5688)
	; 10 bytes
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5694)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inc @zptemp
	brk
	inx
	Jne *-7
.elseif(n = 5696)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	inc @zptemp
	brk
	dex
	Jne *-7
.elseif(n = 5699)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E4A2'
	ldx #228
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5700)
	; 9 bytes
	php
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5703)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 16
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 5704) && (n < 5743))
; 5704 to 5742
.if(n = 5704 || n = 5706 || n = 5708 || n = 5716 || n = 5729 || n = 5739 || n = 5740)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5705 || n = 5736)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5707 || n = 5715 || n = 5730 || n = 5738)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5709)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	inc @zptemp
	inx
	Jne *-7
.elseif(n = 5710)
	; 10 bytes
	ldx #173
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5711)
	; 10 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5712)
	; 10 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5713 || n = 5737)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5714 || n = 5727 || (n > 5731 && n < 5735) || n = 5742)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5717)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	brk
	inx
	Jne *-7
.elseif(n = 5718)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5719)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5720)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	nop
	inc @zptemp
	brk
	inx
	Jne *-7
.elseif(n = 5721)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5722)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5723)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	inc @zptemp
	brk
	dex
	Jne *-7
.elseif(n = 5724)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E5A2'
	ldx #229
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5725)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5726)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5728)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5731)
	; 10 bytes
	ldx #191
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5735)
	; 10 bytes
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5741)
	; 10 bytes
	ldx #58 ;hides 'nop'
	delay_n 9
	brk
	inx
	Jne *-7
.endif
.else
; 5743 to 5781
.if(n = 5743)
	; 10 bytes
	ldx #174
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5744 || n = 5745 || n = 5748 || n = 5757 || n = 5758 || n = 5765 || n = 5766 || n = 5768 || n = 5771 || n = 5772 || n = 5780)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5746)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inc @zptemp
	brk
	inx
	Jne *-7
.elseif(n = 5747)
	; 10 bytes
	ldx #169
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5750)
	; 10 bytes
	ldx #(n-26)/27 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	brk
	dex
	Jne *-7
.elseif(n = 5751 || n = 5776)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5752 || n = 5754 || n = 5764 || n = 5770)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5753 || n = 5763 || n = 5769 || n = 5778)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5755)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5756)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5759)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F0A2'
	ldx #240
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5760)
	; 10 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5761)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5762)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5767)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5773)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5774)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5775)
	; 10 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5777)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5779)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5781)
	; 10 bytes
	ldx #170
	delay_n 29
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
.if((n >= 5782) && (n < 5821))
; 5782 to 5820
.if(n = 5782 || n = 5786 || n = 5789 || n = 5790 || (n > 5791 && n < 5797) || n = 5805 || n = 5810 || n = 5813 || n = 5814 || (n > 5815 && n < 5820))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5783)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5784)
	; 10 bytes
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5785 || n = 5809)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5787 || n = 5800 || n = 5803 || n = 5811)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5788 || n = 5804 || n = 5812)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5791)
	; 10 bytes
	ldx #193
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5797 || n = 5820)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5798)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5799)
	; 9 bytes
	ldx #24 ;hides 'clc'
	inc @zptemp
	brk
	inx
	Jne *-6
.elseif(n = 5801)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5802)
	; 10 bytes
	pha
	ldx #24 ;hides 'clc'
	nop
	delay_n 16
	inx
	Jne *-6
.elseif(n = 5806)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5807)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5808)
	; 10 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5815)
	; 10 bytes
	ldx #171
	delay_n 29
	dex
	Jne *-6
.endif
.else
; 5821 to 5859
.if(n = 5821)
	; 10 bytes
	ldx #194
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5822 || n = 5828 || n = 5835 || n = 5845 || n = 5846 || n = 5853 || n = 5857 || n = 5859)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5823)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5824)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E9A2'
	ldx #233
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5825)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5826 || n = 5851)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5827 || n = 5830 || (n > 5836 && n < 5842) || n = 5848)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5829)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5831)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	brk
	dex
	Jne *-6
.elseif(n = 5832)
	; 10 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5833)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5834 || n = 5836 || n = 5847)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5842)
	; 10 bytes
	ldx #177
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5843)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5844)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	dex
	Jne *-6
.elseif(n = 5849)
	; 9 bytes
	ldx #234 ;hides 'nop'
	inc @zptemp
	brk
	dex
	Jne *-6
.elseif(n = 5850)
	; 10 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5852)
	; 10 bytes
	pha
	ldx #(n-2)/25 ;hides 'nop'
	nop
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5854)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	dex
	Jne *-6
.elseif(n = 5855)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 5856)
	; 10 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5858)
	; 9 bytes
	pha
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	pha
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860 || n = 5891)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5861 || n = 5884)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 5862 || n = 5868 || n = 5880 || n = 5882 || n = 5893)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5863 || n = 5894)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5864 || n = 5865 || n = 5867 || (n > 5869 && n < 5873) || n = 5895 || n = 5896)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5866)
	; 9 bytes
	ldx #(n-1)/23
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5869)
	; 10 bytes
	pha
	ldx #(n-4)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5873)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	nop $4848 ;hides two 'pha's
	brk
	dex
	Jne *-7
.elseif(n = 5874)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 16
	inx
	Jne *-6
.elseif(n = 5875)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 5876)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	nop
	dex
	Jne *-5
.elseif(n = 5877)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 5878)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 5879)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	brk
	dex
	Jne *-6
.elseif(n = 5881)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5883)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5885)
	; 9 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	brk
	dex
	Jne *-6
.elseif(n = 5886)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
.elseif(n = 5887)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5888)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	pha
	inx
	Jne *-6
.elseif(n = 5889)
	; 9 bytes
	ldx #0
	inc @zptemp
	brk
	dex
	Jne *-5
.elseif(n = 5890)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5892)
	; 10 bytes
	pha
	ldx #0
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5897)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	brk
	inx
	Jne *-6
.elseif(n = 5898)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	brk
	inx
	Jne *-6
.endif
.else
; 5899 to 5937
.if(n = 5899)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	delay_n 16
	inx
	Jne *-6
.elseif(n = 5900)
	; 10 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5901 || n = 5926)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5902 || n = 5925 || n = 5932)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5903 || n = 5907 || n = 5928 || n = 5931)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5904)
	; 10 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	nop
	brk
	dex
	Jne *-7
.elseif(n = 5905 || n = 5929)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5906 || n = 5912 || n = 5916 || (n > 5917 && n < 5922) || n = 5923 || n = 5930)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5908)
	; 10 bytes
	ldx #179
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5909)
	; 10 bytes
	ldx #211
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5910)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	inx
	Jne *-7
.elseif(n = 5911)
	; 10 bytes
	ldx #197
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5913)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 19
	inx
	Jne *-7
.elseif(n = 5914)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5915)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 9
	brk
	inx
	Jne *-7
.elseif(n = 5917)
	; 10 bytes
	ldx #174
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5922)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5924)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 16
	inx
	Jne *-6
.elseif(n = 5927)
	; 10 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5933)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5934)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5935)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	delay_n 19
	dex
	Jne *-7
.elseif(n = 5936)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $4848 ;hides two 'pha's
	brk
	inx
	Jne *-7
.elseif(n = 5937)
	; 10 bytes
	ldx #212
	delay_n 23
	dex
	Jne *-6
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
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5939)
	; 10 bytes
	ldx #58 ;hides 'nop'
	delay_n 23
	inx
	Jne *-7
.elseif(n = 5940)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 19
	inx
	Jne *-7
.elseif(n = 5941)
	; 10 bytes
	ldx #180
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5942 || n = 5943 || (n > 5944 && n < 5949) || n = 5958 || n = 5961 || n = 5962 || n = 5964 || n = 5966 || n = 5969 || n = 5970 || n = 5972 || n = 5973)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5944)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	brk
	inx
	Jne *-7
.elseif(n = 5949)
	; 10 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5950)
	; 10 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5951 || n = 5976)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5952)
	; 10 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5953)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5954 || n = 5956 || n = 5960)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5955 || n = 5959)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5957)
	; 9 bytes
	php
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5963)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 19
	dex
	Jne *-7
.elseif(n = 5965)
	; 10 bytes
	ldx #213
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5967)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 19
	inx
	Jne *-7
.elseif(n = 5968)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5971)
	; 10 bytes
	ldx #199
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5974)
	; 10 bytes
	ldx #181
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5975)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	brk
	inx
	Jne *-7
.endif
.else
; 5977 to 6015
.if(n = 5977)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5978 || n = 5986 || n = 6003)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5979)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 19
	inx
	Jne *-6
.elseif(n = 5980 || n = 5982 || n = 5987 || n = 6002 || n = 6004)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5981)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5983 || (n > 5987 && n < 5993) || n = 5994 || (n > 5995 && n < 5999) || n = 6006 || (n > 6008 && n < 6015))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5984 || n = 6015)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5985)
	; 10 bytes
	ldx #176
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5993)
	; 10 bytes
	ldx #214
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5995)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5999)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 4
	brk
	dex
	Jne *-6
.elseif(n = 6000)
	; 10 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6001)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6005)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	brk
	dex
	dex
	Jne *-7
.elseif(n = 6007)
	; 10 bytes
	ldx #182
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6008)
	; 10 bytes
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 6016) && (n < 6055))
; 6016 to 6054
.if(n = 6016 || n = 6020 || n = 6030 || n = 6032 || (n > 6034 && n < 6040) || (n > 6040 && n < 6046))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6017 || n = 6027 || n = 6028 || n = 6048 || n = 6053)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6018 || n = 6029 || n = 6034 || n = 6052)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6019)
	; 10 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	delay_n 26
	inx
	Jne *-7
.elseif(n = 6021)
	; 10 bytes
	ldx #215
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6022)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6023)
	; 10 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6024)
	; 10 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6025 || n = 6049)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 6026 || n = 6051)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6031)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 19
	inx
	Jne *-6
.elseif(n = 6033)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6040)
	; 10 bytes
	ldx #183
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6046)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6047)
	; 10 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6050)
	; 10 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6054)
	; 10 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 26
	inx
	Jne *-7
.endif
.else
; 6055 to 6093
.if((n > 6054 && n < 6059) || n = 6060 || (n > 6061 && n < 6071) || n = 6081 || n = 6082 || n = 6084 || n = 6093)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6059 || n = 6085)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6061)
	; 10 bytes
	ldx #202
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6071)
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6072)
	; 10 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6073)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 6074)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 16
	dex
	Jne *-6
.elseif(n = 6075 || n = 6078 || n = 6079 || n = 6090)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6076)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6077)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6080 || n = 6086)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6083)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6087)
	; 10 bytes
	ldx #179
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6088)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	pha
	dex
	Jne *-6
.elseif(n = 6089)
	; 9 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	brk
	Jne *-6
.elseif(n = 6091)
	; 10 bytes
	ldx #203
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6092)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 7
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if(n = 6094 || n = 6107 || n = 6122 || n = 6131)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6095)
	; 10 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6096)
	; 10 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6097 || n = 6121)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 6098)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	brk
	dex
	Jne *-6
.elseif(n = 6099)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	delay_n 16
	dex
	Jne *-6
.elseif(n = 6100 || n = 6102 || n = 6104 || n = 6111 || n = 6113 || n = 6127 || n = 6129)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6101 || n = 6126)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6103)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6105)
	; 10 bytes
	ldx #218
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6106)
	; 10 bytes
	ldx #185
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6108)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6109)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	brk
	inx
	Jne *-7
.elseif(n = 6110)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 6112 || n = 6114 || n = 6115 || n = 6123 || n = 6128)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6116)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6117)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6118)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	brk
	dex
	Jne *-7
.elseif(n = 6119)
	; 10 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6120)
	; 10 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6124)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 16
	dex
	Jne *-6
.elseif(n = 6125)
	; 10 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	delay_n 16
	dex
	Jne *-7
.elseif(n = 6130)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	dex
	Jne *-5
.elseif(n = 6132)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.endif
.else
; 6133 to 6171
.if(n = 6133)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 6134)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 6135 || n = 6137 || n = 6148 || n = 6154)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6136 || n = 6138 || n = 6141 || n = 6153)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6139 || n = 6170)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6140)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6142)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dex
	Jne *-7
.elseif(n = 6143)
	; 10 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	brk
	dex
	Jne *-7
.elseif(n = 6144)
	; 10 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6145)
	; 9 bytes
	ldx #0
	delay_n 19
	dex
	Jne *-5
.elseif(n = 6146)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 6147)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 9
	brk
	dex
	Jne *-7
.elseif(n = 6149)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	inx
	Jne *-7
.elseif(n = 6150)
	; 10 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	delay_n 16
	dex
	Jne *-7
.elseif(n = 6151)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6152 || n = 6156 || n = 6158 || n = 6159 || n = 6162 || (n > 6163 && n < 6170) || n = 6171)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6155)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	inx
	Jne *-7
.elseif(n = 6157)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6160)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 9
	brk
	inx
	Jne *-7
.elseif(n = 6161)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	brk
	inx
	Jne *-7
.elseif(n = 6163)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 6172) && (n < 6211))
; 6172 to 6210
.if(n = 6172)
	; 10 bytes
	ldx #187
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6173 || n = 6179 || n = 6204)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6174)
	; 10 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6175)
	; 10 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	delay_n 16
	dex
	Jne *-7
.elseif(n = 6176)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6177 || n = 6180 || n = 6182 || n = 6183 || (n > 6184 && n < 6188) || (n > 6189 && n < 6199) || (n > 6205 && n < 6209) || n = 6210)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6178 || n = 6203)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6181)
	; 10 bytes
	ldx #206
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6184)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6188)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	brk
	inx
	Jne *-7
.elseif(n = 6189)
	; 10 bytes
	ldx #182
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6199)
	; 9 bytes
	ldx #248 ;hides 'sed'
	inc @zptemp
	brk
	dex
	Jne *-6
.elseif(n = 6200)
	; 10 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6201)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6202)
	; 10 bytes
	pha
	ldx #248 ;hides 'sed'
	nop
	delay_n 16
	dex
	Jne *-6
.elseif(n = 6205)
	; 10 bytes
	ldx #188
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6209)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	brk
	inx
	Jne *-6
.endif
.else
; 6211 to 6250
.if(n = 6211)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6212 || n = 6217 || n = 6229 || n = 6235)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6213 || (n > 6217 && n < 6223) || n = 6227 || n = 6230 || n = 6231 || n = 6233 || n = 6237 || n = 6239 || n = 6240 || (n > 6241 && n < 6245) || (n > 6245 && n < 6249))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6214)
	; 9 bytes
	php
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6215)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6216 || n = 6228 || n = 6234)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6223)
	; 10 bytes
	ldx #183
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6224)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6225)
	; 10 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6226)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6232)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6236)
	; 10 bytes
	ldx #232 ;hides 'inx'
	inc @zptemp
	brk
	dex
	dex
	Jne *-7
.elseif(n = 6238)
	; 10 bytes
	ldx #189
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6241)
	; 10 bytes
	ldx #208
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6245)
	; 10 bytes
	ldx #223
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6249)
	; 9 bytes
	ldx #250 ;hides 'nop'
	inc @zptemp
	brk
	dex
	Jne *-6
.elseif(n = 6250)
	; 10 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 16
	inx
	Jne *-7
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
; 6251 to 6328
.if(n = 6251 || n = 6276 || n = 6301 || n = 6326)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6252)
	; 10 bytes
	pha
	ldx #(n-2)/25 ;hides 'nop'
	nop
	delay_n 16
	dex
	Jne *-6
.elseif(n = 6253 || n = 6265 || n = 6278 || n = 6296 || n = 6303 || n = 6327 || n = 6328)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6254 || n = 6266 || n = 6279 || n = 6295 || n = 6320)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6255 || n = 6256 || (n > 6257 && n < 6263) || (n > 6267 && n < 6271) || n = 6272 || n = 6277 || (n > 6279 && n < 6291) || n = 6302 || (n > 6305 && n < 6317) || n = 6318 || n = 6322 || n = 6323)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6257)
	; 10 bytes
	ldx #184
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6263 || n = 6294 || n = 6325)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6264)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 26
	inx
	inx
	Jne *-7
.elseif(n = 6267 || n = 6293)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6271)
	; 10 bytes
	ldx #190
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6273)
	; 10 bytes
	ldx #224
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6274)
	; 10 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6275)
	; 10 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6291)
	; 10 bytes
	ldx #185
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6292)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
.elseif(n = 6297)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 6298)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 6299)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6300)
	; 10 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6304)
	; 10 bytes
	ldx #191
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6305)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 6317)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	brk
	dex
	Jne *-6
.elseif(n = 6319)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6321)
	; 10 bytes
	ldx #218 ;hides 'nop'
	delay_n 9
	brk
	dex
	Jne *-7
.elseif(n = 6324)
	; 10 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 16
	inx
	Jne *-7
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if(n = 6329)
	; 10 bytes
	ldx #226
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6330 || n = 6333 || n = 6334 || n = 6336 || (n > 6337 && n < 6341) || n = 6347 || n = 6348 || n = 6352 || n = 6355 || (n > 6361 && n < 6365) || n = 6366)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6331)
	; 10 bytes
	ldx #211
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6332)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 4
	dex
	Jne *-7
.elseif(n = 6335)
	; 10 bytes
	ldx #58 ;hides 'nop'
	jsr @nearby_rts
	brk
	inx
	Jne *-7
.elseif(n = 6337)
	; 10 bytes
	ldx #192
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6341)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	brk
	dex
	Jne *-7
.elseif(n = 6342)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	brk
	dex
	Jne *-7
.elseif(n = 6343)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 4
	nop
	brk
	dex
	Jne *-7
.elseif(n = 6344)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	brk
	inx
	Jne *-7
.elseif(n = 6345)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6346)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6349)
	; 10 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6350)
	; 10 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6351)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6353 || n = 6358 || n = 6367)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6354)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6356)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6357)
	; 10 bytes
	ldx #227
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6359)
	; 10 bytes
	ldx #187
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6360)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inc @zptemp
	inx
	Jne *-7
.elseif(n = 6361)
	; 10 bytes
	ldx #212
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6365)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.endif
.else
; 6368 to 6406
.if(n = 6368 || n = 6379 || n = 6393 || n = 6404)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6369 || n = 6378 || n = 6391 || n = 6392 || n = 6394)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6370)
	; 10 bytes
	ldx #193
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6371 || n = 6396)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 6372 || n = 6397)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6373 || n = 6400)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6374)
	; 10 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6375)
	; 10 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6376)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6377 || n = 6380 || (n > 6381 && n < 6385) || n = 6386 || n = 6402 || n = 6405 || n = 6406)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6381)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	inx
	Jne *-7
.elseif(n = 6385)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 6387)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6388)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 4
	dex
	Jne *-6
.elseif(n = 6389)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	inc @zptemp
	inx
	Jne *-6
.elseif(n = 6390)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 6395)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 6398)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 6399)
	; 10 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	brk
	inx
	Jne *-7
.elseif(n = 6401)
	; 9 bytes
	ldx #0
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6403)
	; 10 bytes
	ldx #194
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
; 6407 to 6484
.if((n > 6406 && n < 6413) || (n > 6413 && n < 6418) || n = 6419 || n = 6422 || n = 6424 || n = 6425 || (n > 6427 && n < 6436) || n = 6437 || n = 6438 || n = 6440 || (n > 6441 && n < 6447) || n = 6448 || n = 6453 || (n > 6454 && n < 6461) || (n > 6461 && n < 6467) || n = 6468 || n = 6470 || n = 6472 || (n > 6475 && n < 6479) || n = 6484)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6413)
	; 10 bytes
	ldx #229
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6418 || n = 6449 || n = 6480)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6420 || n = 6451 || n = 6473 || n = 6482)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6421)
	; 10 bytes
	ldx #214
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6423 || n = 6475)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6426)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 26
	inx
	Jne *-7
.elseif(n = 6427 || n = 6454 || n = 6481)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6436)
	; 10 bytes
	ldx #195
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6439)
	; 10 bytes
	ldx #184 ;hides 'clv'
	nop
	delay_n 26
	dex
	Jne *-7
.elseif(n = 6441)
	; 10 bytes
	ldx #230
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6447)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6450 || n = 6452 || n = 6474 || n = 6483)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6461)
	; 10 bytes
	ldx #190
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6467)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 19
	dex
	dex
	Jne *-7
.elseif(n = 6469)
	; 10 bytes
	ldx #196
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6471)
	; 9 bytes
	php
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6479)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 23
	dex
	Jne *-7
.endif
.else
; 6485 to 6562
.if((n > 6484 && n < 6495) || n = 6496 || n = 6498 || n = 6500 || (n > 6502 && n < 6508) || n = 6509 || n = 6510 || n = 6512 || (n > 6514 && n < 6525) || n = 6526 || n = 6528 || (n > 6529 && n < 6533) || n = 6534 || n = 6537 || n = 6538 || n = 6540 || n = 6543 || (n > 6545 && n < 6551) || n = 6552 || (n > 6553 && n < 6560) || n = 6561)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6495)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-7
.elseif(n = 6497)
	; 10 bytes
	ldx #232
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6499)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6501 || n = 6527 || n = 6553)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6502)
	; 10 bytes
	ldx #197
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6508 || n = 6562)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6511 || n = 6542)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6513 || n = 6544)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6514 || n = 6536 || n = 6545)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6525)
	; 10 bytes
	ldx #233
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6529)
	; 10 bytes
	ldx #192
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6533)
	; 9 bytes
	ldx #58 ;hides 'nop'
	delay_n 26
	inx
	Jne *-6
.elseif(n = 6535)
	; 10 bytes
	ldx #198
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6539)
	; 10 bytes
	ldx #218 ;hides 'nop'
	delay_n 23
	dex
	Jne *-7
.elseif(n = 6541)
	; 10 bytes
	ldx #218
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6551)
	; 10 bytes
	ldx #234 ;hides 'nop'
	nop
	delay_n 19
	dex
	Jne *-7
.elseif(n = 6560)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	inc @zptemp
	brk
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if(n = 6563)
	; 10 bytes
	ldx #193
	delay_n 29
	dex
	Jne *-6
.elseif((n > 6563 && n < 6568) || n = 6569 || n = 6570 || n = 6572 || n = 6574 || n = 6577 || n = 6578 || n = 6580 || n = 6582 || n = 6583 || n = 6588 || (n > 6589 && n < 6596) || n = 6598 || n = 6600 || n = 6608 || (n > 6609 && n < 6614) || n = 6615 || (n > 6616 && n < 6620) || n = 6621 || (n > 6623 && n < 6631) || n = 6632 || n = 6633 || n = 6636 || n = 6639 || n = 6640)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6568)
	; 10 bytes
	ldx #199
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6571)
	; 10 bytes
	ldx #219
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6573 || n = 6604 || n = 6635)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6575 || n = 6606 || n = 6622 || n = 6637)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6576)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inc @zptemp
	dex
	Jne *-7
.elseif(n = 6579 || n = 6605)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6581)
	; 10 bytes
	ldx #235
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6584)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	brk
	dex
	Jne *-7
.elseif(n = 6585)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	brk
	dex
	Jne *-7
.elseif(n = 6586)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 6587)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	inc @zptemp
	brk
	dex
	Jne *-7
.elseif(n = 6589 || n = 6616)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6596)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 6
	inx
	Jne *-7
.elseif(n = 6597)
	; 10 bytes
	ldx #194
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6599)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 26
	inx
	Jne *-6
.elseif(n = 6601)
	; 10 bytes
	ldx #200
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6602 || n = 6607 || n = 6623 || n = 6638)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6603)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	brk
	inx
	Jne *-7
.elseif(n = 6609)
	; 10 bytes
	ldx #236
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6614)
	; 10 bytes
	ldx #(n-26)/27 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	brk
	dex
	Jne *-7
.elseif(n = 6620)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	delay_n 6
	dex
	Jne *-6
.elseif(n = 6631)
	; 10 bytes
	ldx #221
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6634)
	; 10 bytes
	ldx #201
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.else
; 6641 to 6718
.if(n = 6641 || (n > 6649 && n < 6657) || (n > 6657 && n < 6661) || (n > 6661 && n < 6665) || (n > 6670 && n < 6691) || n = 6692 || n = 6694 || n = 6696 || (n > 6701 && n < 6719))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6642)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	nop
	dex
	Jne *-6
.elseif(n = 6643)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	pha
	inx
	Jne *-6
.elseif(n = 6644)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	nop
	pha
	dex
	Jne *-6
.elseif(n = 6645)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 6
	inx
	Jne *-6
.elseif(n = 6646)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	delay_n 6
	dex
	Jne *-6
.elseif(n = 6647 || n = 6648 || n = 6668 || n = 6699)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6649 || n = 6701)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6657)
	; 10 bytes
	ldx #0
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6661)
	; 10 bytes
	ldx #222
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6665)
	; 10 bytes
	ldx #196
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6666 || n = 6697)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6667)
	; 10 bytes
	ldx #202
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6669)
	; 10 bytes
	ldx #26 ;hides 'nop'
	delay_n 9
	brk
	inx
	Jne *-7
.elseif(n = 6670)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6691)
	; 10 bytes
	ldx #223
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6693)
	; 10 bytes
	ldx #239
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6695)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	brk
	dex
	Jne *-6
.elseif(n = 6698)
	; 9 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 26
	Jne *-6
.elseif(n = 6700)
	; 10 bytes
	ldx #203
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 6719) && (n < 6797))
; 6719 to 6796
.if(n = 6719 || n = 6720 || n = 6722 || n = 6723 || n = 6725 || n = 6726 || n = 6729 || n = 6732 || (n > 6733 && n < 6749) || n = 6750 || (n > 6752 && n < 6759) || n = 6760 || (n > 6762 && n < 6766) || (n > 6767 && n < 6777) || n = 6779 || n = 6780 || (n > 6781 && n < 6785) || (n > 6785 && n < 6790) || n = 6791 || (n > 6793 && n < 6797))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6721)
	; 10 bytes
	ldx #224
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6724 || n = 6751 || n = 6778)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6727)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	brk
	inx
	Jne *-7
.elseif(n = 6728)
	; 9 bytes
	php
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6730 || n = 6761 || n = 6792)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6731 || n = 6752 || n = 6762 || n = 6793)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6733)
	; 10 bytes
	ldx #204
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6749)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 7
	brk
	dex
	Jne *-6
.elseif(n = 6759 || n = 6790)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6766)
	; 10 bytes
	ldx #205
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6767)
	; 10 bytes
	ldx #199
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6777)
	; 10 bytes
	ldx #242
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6781)
	; 10 bytes
	ldx #226
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6785)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	brk
	dex
	Jne *-7
.endif
.else
.if((n >= 6797) && (n < 6836))
; 6797 to 6835
.if(n = 6797 || n = 6798 || n = 6800 || n = 6802 || n = 6804 || (n > 6805 && n < 6811) || n = 6812 || n = 6813 || (n > 6814 && n < 6820) || n = 6822 || n = 6825 || n = 6826 || n = 6828 || n = 6834)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6799)
	; 10 bytes
	ldx #206
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6801)
	; 10 bytes
	ldx #200
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6803)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 19
	dex
	Jne *-7
.elseif(n = 6805)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6811)
	; 10 bytes
	ldx #227
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6814)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	brk
	inx
	Jne *-7
.elseif(n = 6820)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 6
	dex
	Jne *-7
.elseif(n = 6821)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6823)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6824)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6827)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	brk
	dex
	Jne *-7
.elseif(n = 6829)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 6830)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 6831)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	delay_n 19
	dex
	Jne *-7
.elseif(n = 6832)
	; 10 bytes
	ldx #207
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6833)
	; 10 bytes
	ldx #244
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6835)
	; 10 bytes
	ldx #201
	delay_n 29
	dex
	Jne *-6
.endif
.else
; 6836 to 6875
.if(n = 6836 || n = 6837 || n = 6839 || n = 6840 || n = 6842 || (n > 6843 && n < 6852) || n = 6853 || (n > 6855 && n < 6859) || n = 6860 || (n > 6861 && n < 6865) || (n > 6865 && n < 6869) || n = 6870 || n = 6873 || n = 6874)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6838)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	inx
	Jne *-7
.elseif(n = 6841)
	; 10 bytes
	ldx #228
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6843)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 9
	brk
	inx
	Jne *-7
.elseif(n = 6852)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6854)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6855)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6859)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6861)
	; 10 bytes
	ldx #245
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6865)
	; 10 bytes
	ldx #208
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6869)
	; 10 bytes
	ldx #202
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6871)
	; 10 bytes
	ldx #229
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6872)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	brk
	inx
	Jne *-7
.elseif(n = 6875)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
; 6876 to 6953
.if(n = 6876 || (n > 6878 && n < 6883) || n = 6884 || n = 6887 || n = 6888 || (n > 6889 && n < 6897) || (n > 6905 && n < 6911) || n = 6912 || n = 6915 || (n > 6917 && n < 6929) || n = 6930 || (n > 6931 && n < 6937) || (n > 6937 && n < 6943) || n = 6944 || n = 6946 || (n > 6948 && n < 6954))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6877 || n = 6885 || n = 6899 || n = 6903 || n = 6904 || n = 6916 || n = 6947)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6878 || n = 6905 || n = 6917 || n = 6948)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6883 || n = 6914 || n = 6945)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6886)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6889)
	; 10 bytes
	ldx #246
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6897)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
.elseif(n = 6898)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	pha
	dex
	Jne *-6
.elseif(n = 6900)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 6
	dex
	Jne *-6
.elseif(n = 6901)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 6902)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 6911)
	; 10 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	brk
	dex
	Jne *-7
.elseif(n = 6913)
	; 10 bytes
	ldx #0
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6929)
	; 10 bytes
	ldx #58 ;hides 'nop'
	nop
	delay_n 26
	inx
	Jne *-7
.elseif(n = 6931)
	; 10 bytes
	ldx #210
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6937)
	; 10 bytes
	ldx #204
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6943)
	; 10 bytes
	ldx #248 ;hides 'sed'
	nop
	delay_n 19
	dex
	Jne *-7
.endif
.else
; 6954 to 7031
.if((n > 6953 && n < 6959) || n = 6960 || n = 6962 || n = 6963 || (n > 6964 && n < 6971) || n = 6972 || n = 6974 || n = 6977 || (n > 6979 && n < 6985) || n = 6986 || n = 6989 || n = 6990 || (n > 6991 && n < 6997) || n = 6998 || n = 7000 || (n > 7001 && n < 7005) || n = 7006 || n = 7008 || (n > 7010 && n < 7019) || n = 7020 || (n > 7021 && n < 7029) || n = 7031)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6959)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 23
	inx
	Jne *-7
.elseif(n = 6961)
	; 10 bytes
	ldx #232
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6964)
	; 10 bytes
	ldx #211
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6971)
	; 10 bytes
	ldx #205
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6973)
	; 10 bytes
	ldx #249
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6975)
	; 10 bytes
	ldx #218 ;hides 'nop'
	jsr @nearby_rts
	brk
	dex
	Jne *-7
.elseif(n = 6976 || n = 7007)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6978 || n = 6987 || n = 7009)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6979 || n = 6988 || n = 7010)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6985)
	; 9 bytes
	php
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6991)
	; 10 bytes
	ldx #233
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6997)
	; 10 bytes
	ldx #212
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6999)
	; 10 bytes
	ldx #56 ;hides 'sec'
	nop
	delay_n 26
	inx
	Jne *-7
.elseif(n = 7001)
	; 10 bytes
	ldx #250
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7005)
	; 10 bytes
	ldx #206
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7019)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 23
	dex
	Jne *-7
.elseif(n = 7021)
	; 10 bytes
	ldx #234
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7029)
	; 10 bytes
	ldx #251
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7030)
	; 10 bytes
	ldx #213
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
; 7032 to 7109
.if((n > 7031 && n < 7038) || (n > 7041 && n < 7046) || (n > 7046 && n < 7051) || (n > 7051 && n < 7057) || (n > 7057 && n < 7063) || (n > 7064 && n < 7069) || n = 7074 || (n > 7075 && n < 7081) || (n > 7081 && n < 7085) || (n > 7085 && n < 7096) || (n > 7096 && n < 7100) || n = 7101 || n = 7105 || n = 7106 || n = 7108 || n = 7109)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7038 || n = 7069 || n = 7100)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7039)
	; 10 bytes
	ldx #207
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7040 || n = 7071 || n = 7102)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7041 || n = 7072 || n = 7103)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7046)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	brk
	dex
	Jne *-7
.elseif(n = 7051)
	; 10 bytes
	ldx #235
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7057)
	; 10 bytes
	ldx #252
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7063)
	; 10 bytes
	ldx #214
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7064)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 7
	dex
	Jne *-7
.elseif(n = 7070)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dex
	Jne *-7
.elseif(n = 7073)
	; 10 bytes
	ldx #208
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7075)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	brk
	dex
	Jne *-7
.elseif(n = 7081)
	; 10 bytes
	ldx #236
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7085)
	; 10 bytes
	ldx #253
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7096)
	; 10 bytes
	ldx #215
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7104)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	delay_n 26
	Jne *-7
.elseif(n = 7107)
	; 10 bytes
	ldx #209
	delay_n 29
	dex
	Jne *-6
.endif
.else
; 7110 to 7187
.if(n = 7110 || n = 7112 || (n > 7113 && n < 7127) || (n > 7134 && n < 7139) || n = 7140 || (n > 7141 && n < 7153) || n = 7163 || (n > 7165 && n < 7169) || n = 7170 || n = 7172 || n = 7173 || (n > 7175 && n < 7188))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7111)
	; 10 bytes
	ldx #237
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7113)
	; 10 bytes
	ldx #254
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7127)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7128)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	inx
	Jne *-7
.elseif(n = 7129)
	; 10 bytes
	ldx #216
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7130)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 10
	dex
	Jne *-6
.elseif(n = 7131 || n = 7162)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7132 || n = 7133 || n = 7160 || n = 7164)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7134 || n = 7159 || n = 7161 || n = 7165)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7139)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	delay_n 26
	inx
	Jne *-7
.elseif(n = 7141)
	; 10 bytes
	ldx #238
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7153)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	dex
	Jne *-6
.elseif(n = 7154)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	delay_n 4
	dex
	Jne *-7
.elseif(n = 7155)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	inc @zptemp
	inx
	Jne *-7
.elseif(n = 7156)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 7157)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 7158)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 10
	dex
	Jne *-6
.elseif(n = 7169)
	; 10 bytes
	ldx #0
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7171)
	; 10 bytes
	ldx #239
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7174)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 26
	inx
	Jne *-7
.elseif(n = 7175)
	; 10 bytes
	ldx #211
	delay_n 29
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
; 7188 to 7343
.if((n > 7187 && n < 7191) || n = 7192 || n = 7194 || n = 7197 || n = 7198 || n = 7200 || (n > 7201 && n < 7209) || (n > 7209 && n < 7224) || n = 7225 || n = 7229 || n = 7230 || (n > 7231 && n < 7242) || (n > 7245 && n < 7249) || (n > 7249 && n < 7255) || n = 7256 || n = 7259 || n = 7260 || (n > 7261 && n < 7277) || (n > 7277 && n < 7286) || n = 7287 || n = 7290 || n = 7292 || n = 7293 || (n > 7294 && n < 7307) || (n > 7307 && n < 7311) || (n > 7311 && n < 7317) || n = 7318 || (n > 7321 && n < 7327) || (n > 7327 && n < 7344))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7191)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 9
	brk
	dex
	Jne *-7
.elseif(n = 7193 || n = 7224 || n = 7255 || n = 7286 || n = 7317)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7195)
	; 10 bytes
	ldx #218
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7196 || n = 7227 || n = 7245 || n = 7258 || n = 7289 || n = 7320)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7199)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 29
	inx
	Jne *-7
.elseif(n = 7201)
	; 10 bytes
	ldx #240
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7209)
	; 10 bytes
	ldx #212
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7226 || n = 7244 || n = 7257 || n = 7288 || n = 7319)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7228)
	; 10 bytes
	ldx #219
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7231)
	; 10 bytes
	ldx #241
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7242)
	; 9 bytes
	php
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7243)
	; 10 bytes
	ldx #213
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7249)
	; 10 bytes
	ldx #250 ;hides 'nop'
	delay_n 9
	brk
	dex
	Jne *-7
.elseif(n = 7261)
	; 10 bytes
	ldx #220
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7277)
	; 10 bytes
	ldx #214
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7291)
	; 10 bytes
	ldx #243
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7294)
	; 10 bytes
	ldx #221
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7307)
	; 10 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 29
	Jne *-7
.elseif(n = 7311)
	; 10 bytes
	ldx #215
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7321)
	; 10 bytes
	ldx #244
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7327)
	; 10 bytes
	ldx #222
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if(n = 7344 || n = 7346 || n = 7347 || n = 7349 || (n > 7351 && n < 7359) || (n > 7360 && n < 7379) || n = 7380 || n = 7383 || (n > 7385 && n < 7393) || (n > 7393 && n < 7408) || (n > 7414 && n < 7419) || n = 7420 || n = 7421)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7345)
	; 10 bytes
	ldx #216
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7348 || n = 7379 || n = 7410)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7350 || n = 7381 || n = 7412)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7351)
	; 10 bytes
	ldx #245
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7359)
	; 10 bytes
	ldx #26 ;hides 'nop'
	jsr @nearby_rts
	brk
	inx
	Jne *-7
.elseif(n = 7360)
	; 10 bytes
	ldx #223
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7382)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7384)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 26
	dex
	Jne *-7
.elseif(n = 7385)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 11
	dex
	Jne *-7
.elseif(n = 7393)
	; 10 bytes
	ldx #224
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7408)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	dex
	Jne *-7
.elseif(n = 7409)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	dex
	Jne *-7
.elseif(n = 7411)
	; 10 bytes
	ldx #247
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7413)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 9
	inx
	Jne *-7
.elseif(n = 7414)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 11
	dex
	Jne *-7
.elseif(n = 7419)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	delay_n 26
	dex
	Jne *-7
.endif
.else
; 7422 to 7500
.if(n = 7422 || n = 7424 || n = 7425 || (n > 7426 && n < 7439) || n = 7440 || n = 7442 || n = 7445 || (n > 7448 && n < 7454) || (n > 7454 && n < 7459) || (n > 7459 && n < 7469) || n = 7470 || n = 7473 || (n > 7475 && n < 7480) || (n > 7481 && n < 7487) || (n > 7487 && n < 7492) || (n > 7492 && n < 7499) || n = 7500)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7423)
	; 10 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	brk
	inx
	Jne *-7
.elseif(n = 7426)
	; 10 bytes
	ldx #225
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7439)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 23
	dex
	Jne *-7
.elseif(n = 7441 || n = 7472)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7443 || n = 7474)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7444 || n = 7475)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7446)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 26
	inx
	Jne *-7
.elseif(n = 7447)
	; 10 bytes
	ldx #219
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7448)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	dex
	Jne *-7
.elseif(n = 7454)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 26
	dex
	Jne *-7
.elseif(n = 7459)
	; 10 bytes
	ldx #226
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7469)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	inx
	Jne *-7
.elseif(n = 7471)
	; 10 bytes
	ldx #249
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7480)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 26
	inx
	Jne *-7
.elseif(n = 7481)
	; 10 bytes
	ldx #220
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7487)
	; 10 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	brk
	dex
	Jne *-7
.elseif(n = 7492)
	; 10 bytes
	ldx #227
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7499)
	; 9 bytes
	php
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	plp
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
.if(n = 7501 || n = 7505 || n = 7536 || n = 7567 || n = 7598 || n = 7629 || n = 7642)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7502 || n = 7506 || n = 7537 || n = 7568 || n = 7592 || n = 7599 || n = 7630 || n = 7643)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7503 || n = 7534 || n = 7565 || n = 7596 || n = 7627)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7504 || (n > 7506 && n < 7510) || (n > 7510 && n < 7514) || (n > 7515 && n < 7525) || (n > 7525 && n < 7531) || n = 7532 || n = 7533 || n = 7535 || (n > 7537 && n < 7549) || (n > 7549 && n < 7558) || n = 7560 || (n > 7561 && n < 7565) || n = 7566 || (n > 7568 && n < 7583) || (n > 7583 && n < 7589) || n = 7590 || (n > 7592 && n < 7596) || n = 7597 || (n > 7599 && n < 7617) || (n > 7617 && n < 7621) || n = 7622 || n = 7623 || n = 7625 || n = 7626 || n = 7628 || (n > 7630 && n < 7640) || n = 7641 || (n > 7643 && n < 7651) || (n > 7651 && n < 7655) || n = 7656)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7510)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 32
	Jne *-7
.elseif(n = 7514)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 26
	inx
	Jne *-7
.elseif(n = 7515)
	; 10 bytes
	ldx #221
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7525)
	; 10 bytes
	ldx #228
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7531)
	; 10 bytes
	ldx #251
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7549)
	; 10 bytes
	ldx #222
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7558)
	; 10 bytes
	ldx #229
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7559)
	; 10 bytes
	ldx #216 ;hides 'cld'
	nop
	delay_n 26
	dex
	Jne *-7
.elseif(n = 7561)
	; 10 bytes
	ldx #252
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7583)
	; 10 bytes
	ldx #223
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7589)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 26
	inx
	Jne *-6
.elseif(n = 7591)
	; 10 bytes
	ldx #230
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7617)
	; 10 bytes
	ldx #224
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7621)
	; 10 bytes
	ldx #254
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7624)
	; 10 bytes
	ldx #231
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7640)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 7651)
	; 10 bytes
	ldx #255
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7655)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 26
	inx
	Jne *-6
.endif
.else
; 7657 to 7812
.if(n = 7657)
	; 10 bytes
	ldx #232
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7658 || n = 7689 || n = 7720 || n = 7751 || n = 7782)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7659 || (n > 7661 && n < 7665) || n = 7671 || (n > 7673 && n < 7681) || (n > 7681 && n < 7685) || (n > 7685 && n < 7689) || (n > 7692 && n < 7713) || (n > 7713 && n < 7719) || (n > 7724 && n < 7751) || n = 7752 || n = 7755 || n = 7757 || (n > 7759 && n < 7775) || (n > 7775 && n < 7782) || n = 7783 || n = 7786 || n = 7788 || (n > 7789 && n < 7813))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7660 || n = 7672 || n = 7691 || n = 7722 || n = 7753 || n = 7758 || n = 7784)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7661 || n = 7673 || n = 7692 || n = 7724 || n = 7754 || n = 7759 || n = 7785)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7665)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	inc @zptemp
	dex
	Jne *-7
.elseif(n = 7666)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	delay_n 6
	dex
	Jne *-7
.elseif(n = 7667)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	delay_n 7
	inx
	Jne *-7
.elseif(n = 7668)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 9
	dex
	Jne *-7
.elseif(n = 7669)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 10
	inx
	Jne *-7
.elseif(n = 7670)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 7681)
	; 10 bytes
	ldx #0
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7685)
	; 10 bytes
	ldx #226
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7690)
	; 10 bytes
	ldx #233
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7713)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 26
	Jne *-7
.elseif(n = 7719)
	; 10 bytes
	ldx #227
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7721)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7723)
	; 10 bytes
	ldx #234
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7756)
	; 9 bytes
	php
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7775)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 29
	dex
	Jne *-7
.elseif(n = 7787)
	; 10 bytes
	ldx #229
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7789)
	; 10 bytes
	ldx #236
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
; 7813 to 7968
.if(n = 7813 || n = 7844 || n = 7875 || n = 7906)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7814 || (n > 7816 && n < 7821) || (n > 7822 && n < 7844) || n = 7845 || (n > 7847 && n < 7855) || (n > 7855 && n < 7875) || n = 7876 || (n > 7878 && n < 7888) || (n > 7889 && n < 7895) || n = 7896 || (n > 7902 && n < 7906) || n = 7907 || (n > 7909 && n < 7915) || n = 7916 || n = 7927 || n = 7934 || n = 7936 || n = 7938 || (n > 7940 && n < 7954) || n = 7955 || n = 7956 || (n > 7957 && n < 7969))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7815 || n = 7846 || n = 7877 || n = 7897 || n = 7899 || n = 7908 || n = 7917 || n = 7919 || n = 7928 || n = 7930 || n = 7939)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7816 || n = 7847 || n = 7878 || n = 7898 || n = 7909 || n = 7918 || n = 7929 || n = 7940)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7821)
	; 10 bytes
	ldx #230
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7822)
	; 10 bytes
	ldx #237
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7855)
	; 10 bytes
	ldx #238
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7888)
	; 10 bytes
	ldx #239
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7889)
	; 10 bytes
	ldx #232
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7895)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 7900 || n = 7920 || n = 7931)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 7901 || n = 7932)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 7902 || n = 7922 || n = 7933)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7915)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 7921)
	; 10 bytes
	ldx #240
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7923)
	; 10 bytes
	ldx #233
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7924)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 10
	dex
	Jne *-7
.elseif(n = 7925)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	delay_n 10
	dex
	Jne *-7
.elseif(n = 7926)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 7935)
	; 10 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	brk
	dex
	Jne *-7
.elseif(n = 7937)
	; 9 bytes
	ldx #0
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7954)
	; 10 bytes
	ldx #241
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7957)
	; 10 bytes
	ldx #234
	delay_n 29
	dex
	Jne *-6
.endif
.else
; 7969 to 8125
.if((n > 7968 && n < 7987) || (n > 7987 && n < 7991) || (n > 7991 && n < 7999) || (n > 7999 && n < 8013) || n = 8014 || (n > 8016 && n < 8020) || (n > 8020 && n < 8025) || (n > 8025 && n < 8049) || (n > 8049 && n < 8053) || (n > 8053 && n < 8059) || (n > 8059 && n < 8084) || n = 8085 || (n > 8086 && n < 8093) || (n > 8093 && n < 8119) || (n > 8119 && n < 8126))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7987)
	; 10 bytes
	ldx #242
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7991)
	; 10 bytes
	ldx #235
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7999)
	; 10 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	brk
	dex
	Jne *-7
.elseif(n = 8013)
	; 9 bytes
	php
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8015)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8016)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8020)
	; 10 bytes
	ldx #243
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8025)
	; 10 bytes
	ldx #236
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8049)
	; 10 bytes
	ldx #26 ;hides 'nop'
	nop
	delay_n 26
	inx
	Jne *-7
.elseif(n = 8053)
	; 10 bytes
	ldx #244
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8059)
	; 10 bytes
	ldx #237
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8084)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 26
	dex
	dex
	Jne *-7
.elseif(n = 8086)
	; 10 bytes
	ldx #245
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8093)
	; 10 bytes
	ldx #238
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8119)
	; 10 bytes
	ldx #246
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
; 8126 to 8281
.if(n = 8126 || (n > 8127 && n < 8150) || n = 8151 || (n > 8152 && n < 8161) || (n > 8161 && n < 8177) || (n > 8177 && n < 8181) || n = 8184 || n = 8187 || n = 8188 || (n > 8189 && n < 8195) || (n > 8195 && n < 8218) || (n > 8218 && n < 8224) || (n > 8224 && n < 8229) || (n > 8229 && n < 8248) || n = 8250 || (n > 8252 && n < 8259) || (n > 8259 && n < 8263) || (n > 8263 && n < 8270) || n = 8271 || (n > 8273 && n < 8279) || n = 8280 || n = 8281)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8127)
	; 10 bytes
	ldx #239
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8150)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 14
	dex
	Jne *-7
.elseif(n = 8152)
	; 10 bytes
	ldx #247
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8161)
	; 10 bytes
	ldx #240
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8177)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dex
	Jne *-7
.elseif(n = 8181)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 8182)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 14
	dex
	Jne *-7
.elseif(n = 8183)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8185)
	; 10 bytes
	ldx #248
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8186 || n = 8252 || n = 8273)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8189)
	; 10 bytes
	ldx #234 ;hides 'nop'
	nop
	delay_n 26
	dex
	Jne *-7
.elseif(n = 8195)
	; 10 bytes
	ldx #241
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8218)
	; 10 bytes
	ldx #249
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8224)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 26
	inx
	Jne *-7
.elseif(n = 8229)
	; 10 bytes
	ldx #242
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8248)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	inx
	Jne *-7
.elseif(n = 8249)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8251)
	; 10 bytes
	ldx #250
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8259)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	delay_n 26
	inx
	Jne *-7
.elseif(n = 8263)
	; 10 bytes
	ldx #243
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8270)
	; 9 bytes
	php
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8272)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8279)
	; 10 bytes
	ldx #26 ;hides 'nop'
	delay_n 29
	inx
	Jne *-7
.endif
.else
; 8282 to 8437
.if(n = 8282 || n = 8283 || (n > 8284 && n < 8294) || n = 8295 || n = 8296 || (n > 8297 && n < 8317) || (n > 8317 && n < 8331) || (n > 8331 && n < 8350) || (n > 8351 && n < 8365) || (n > 8365 && n < 8383) || (n > 8383 && n < 8399) || (n > 8399 && n < 8405) || n = 8406 || (n > 8408 && n < 8416) || (n > 8416 && n < 8423) || (n > 8423 && n < 8427) || n = 8428 || n = 8431 || n = 8432 || n = 8434 || n = 8435)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8284)
	; 10 bytes
	ldx #251
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8294)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 26
	inx
	Jne *-7
.elseif(n = 8297)
	; 10 bytes
	ldx #244
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8317)
	; 10 bytes
	ldx #252
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8331)
	; 10 bytes
	ldx #245
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8350)
	; 10 bytes
	ldx #253
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8351)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 29
	inx
	Jne *-7
.elseif(n = 8365)
	; 10 bytes
	ldx #246
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8383)
	; 10 bytes
	ldx #254
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8399)
	; 10 bytes
	ldx #247
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8405)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	brk
	dex
	Jne *-6
.elseif(n = 8407 || n = 8429)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8408 || n = 8430)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8416)
	; 10 bytes
	ldx #255
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8423)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 29
	dex
	Jne *-7
.elseif(n = 8427)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	inx
	Jne *-6
.elseif(n = 8433)
	; 10 bytes
	ldx #248
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8436)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 8437)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk
	inx
	Jne *-6
.endif
.endif
.else
; 8438 to 8750
.if(n = 8438)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	brk
	dex
	Jne *-6
.elseif(n = 8439 || n = 8440 || n = 8529 || n = 8662 || n = 8685 || n = 8696)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8441 || n = 8530 || n = 8663 || n = 8686 || n = 8695 || n = 8697)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n > 8441 && n < 8449) || (n > 8449 && n < 8467) || (n > 8467 && n < 8501) || n = 8502 || n = 8503 || (n > 8504 && n < 8527) || (n > 8530 && n < 8535) || (n > 8535 && n < 8539) || (n > 8539 && n < 8569) || (n > 8569 && n < 8574) || (n > 8574 && n < 8603) || (n > 8603 && n < 8637) || (n > 8637 && n < 8660) || n = 8661 || (n > 8663 && n < 8671) || (n > 8671 && n < 8679) || (n > 8679 && n < 8683) || n = 8684 || (n > 8686 && n < 8692) || (n > 8697 && n < 8705) || (n > 8705 && n < 8749) || n = 8750)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8449)
	; 10 bytes
	ldx #0
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8467)
	; 10 bytes
	ldx #249
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8501)
	; 10 bytes
	ldx #250
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8504)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 26
	dex
	Jne *-7
.elseif(n = 8527)
	; 9 bytes
	php
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8528)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	dex
	Jne *-7
.elseif(n = 8535)
	; 10 bytes
	ldx #251
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8539)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	delay_n 26
	dex
	Jne *-7
.elseif(n = 8569)
	; 10 bytes
	ldx #252
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8574)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 26
	dex
	Jne *-7
.elseif(n = 8603)
	; 10 bytes
	ldx #253
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8637)
	; 10 bytes
	ldx #254
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8660)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 16
	dex
	Jne *-6
.elseif(n = 8671)
	; 10 bytes
	ldx #255
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8679)
	; 10 bytes
	ldx #248 ;hides 'sed'
	nop
	delay_n 26
	dex
	Jne *-7
.elseif(n = 8683)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	pha
	inx
	Jne *-6
.elseif(n = 8692)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	brk
	dex
	Jne *-6
.elseif(n = 8693)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	brk
	dex
	Jne *-6
.elseif(n = 8694)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 16
	dex
	Jne *-6
.elseif(n = 8705)
	; 10 bytes
	ldx #0
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8749)
	; 10 bytes
	ldx #250 ;hides 'nop'
	nop
	delay_n 26
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 8751) && (n < 9376))
; 8751 to 9375
.if((n > 8750 && n < 8784) || n = 8785 || (n > 8787 && n < 8915) || (n > 8915 && n < 8927) || (n > 8927 && n < 8931) || (n > 8931 && n < 8939) || (n > 8939 && n < 8948) || (n > 8950 && n < 8999) || (n > 8999 && n < 9041) || n = 9042 || (n > 9044 && n < 9170) || (n > 9170 && n < 9195) || (n > 9195 && n < 9203) || (n > 9206 && n < 9298) || n = 9299 || (n > 9301 && n < 9376))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8784)
	; 9 bytes
	php
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8786 || n = 9043 || n = 9300)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8787 || n = 9044 || n = 9301)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8915)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 4
	brk
	dex
	Jne *-7
.elseif(n = 8927)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 29
	dex
	Jne *-7
.elseif(n = 8931)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
.elseif(n = 8939)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	inx
	Jne *-7
.elseif(n = 8948)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	brk
	dex
	Jne *-7
.elseif(n = 8949)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	brk
	inx
	Jne *-7
.elseif(n = 8950)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	nop
	brk
	dex
	Jne *-7
.elseif(n = 8999)
	; 10 bytes
	ldx #250 ;hides 'nop'
	delay_n 29
	dex
	Jne *-7
.elseif(n = 9041)
	; 9 bytes
	php
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9170)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	inc @zptemp
	brk
	dex
	Jne *-7
.elseif(n = 9195)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	inc @zptemp
	inx
	Jne *-7
.elseif(n = 9203)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	brk
	inx
	Jne *-7
.elseif(n = 9204)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	nop
	brk
	dex
	Jne *-7
.elseif(n = 9205)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 16
	inx
	Jne *-7
.elseif(n = 9206)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	inc @zptemp
	brk
	dex
	Jne *-7
.elseif(n = 9298)
	; 9 bytes
	php
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.else
; 9376 to 10000
.if((n > 9375 && n < 9425) || (n > 9425 && n < 9451) || (n > 9451 && n < 9458) || n = 9459 || (n > 9462 && n < 9555) || n = 9556 || (n > 9558 && n < 9680) || (n > 9680 && n < 9707) || (n > 9707 && n < 9713) || (n > 9713 && n < 9718) || (n > 9718 && n < 9812) || n = 9813 || (n > 9815 && n < 10001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9425)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 19
	dex
	Jne *-7
.elseif(n = 9451)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 6
	inx
	Jne *-7
.elseif(n = 9458)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	brk
	dex
	Jne *-7
.elseif(n = 9460)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 9461)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 9462)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 19
	dex
	Jne *-7
.elseif(n = 9555)
	; 9 bytes
	php
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9557 || n = 9814)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9558 || n = 9815)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 9680)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 7
	brk
	dex
	Jne *-7
.elseif(n = 9707)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	inx
	Jne *-7
.elseif(n = 9713)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dex
	Jne *-7
.elseif(n = 9718)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 9
	brk
	dex
	Jne *-7
.elseif(n = 9812)
	; 9 bytes
	php
	pha
	lax #38
	jsr delay_256x_a_30_clocks
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
; 10001 to 12500
.if((n > 10000 && n < 10069) || n = 10070 || (n > 10072 && n < 10326) || n = 10327 || (n > 10329 && n < 10583) || n = 10584 || (n > 10586 && n < 10840) || n = 10841 || (n > 10843 && n < 11097) || n = 11098 || (n > 11100 && n < 11210) || (n > 11210 && n < 11243) || (n > 11243 && n < 11254) || (n > 11254 && n < 11354) || n = 11355 || (n > 11357 && n < 11611) || n = 11612 || (n > 11614 && n < 11868) || n = 11869 || (n > 11871 && n < 12125) || n = 12126 || (n > 12128 && n < 12382) || n = 12383 || (n > 12385 && n < 12501))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10069)
	; 9 bytes
	php
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10071 || n = 10328 || n = 10585 || n = 10842 || n = 11099 || n = 11356 || n = 11613 || n = 11870 || n = 12127 || n = 12384)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10072 || n = 10329 || n = 10586 || n = 10843 || n = 11100 || n = 11357 || n = 11614 || n = 11871 || n = 12128 || n = 12385)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10326)
	; 9 bytes
	php
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10583)
	; 9 bytes
	php
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10840)
	; 9 bytes
	php
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11097)
	; 9 bytes
	php
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11210)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 26
	dex
	Jne *-7
.elseif(n = 11243)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	inx
	Jne *-7
.elseif(n = 11254)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 26
	dex
	Jne *-7
.elseif(n = 11354)
	; 9 bytes
	php
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11611)
	; 9 bytes
	php
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11868)
	; 9 bytes
	php
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12125)
	; 9 bytes
	php
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12382)
	; 9 bytes
	php
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.else
; 12501 to 15000
.if((n > 12500 && n < 12639) || n = 12640 || (n > 12642 && n < 12896) || n = 12897 || (n > 12899 && n < 13153) || n = 13154 || (n > 13156 && n < 13410) || n = 13411 || (n > 13413 && n < 13667) || n = 13668 || (n > 13670 && n < 13924) || n = 13925 || (n > 13927 && n < 14181) || n = 14182 || (n > 14184 && n < 14438) || n = 14439 || (n > 14441 && n < 14695) || n = 14696 || (n > 14698 && n < 14952) || n = 14953 || (n > 14955 && n < 15001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12639)
	; 9 bytes
	php
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12641 || n = 12898 || n = 13155 || n = 13412 || n = 13669 || n = 13926 || n = 14183 || n = 14440 || n = 14697 || n = 14954)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12642 || n = 12899 || n = 13156 || n = 13413 || n = 13670 || n = 13927 || n = 14184 || n = 14441 || n = 14698 || n = 14955)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12896)
	; 9 bytes
	php
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13153)
	; 9 bytes
	php
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13410)
	; 9 bytes
	php
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13667)
	; 9 bytes
	php
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13924)
	; 9 bytes
	php
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14181)
	; 9 bytes
	php
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14438)
	; 9 bytes
	php
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14695)
	; 9 bytes
	php
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14952)
	; 9 bytes
	php
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15209) || n = 15210 || (n > 15212 && n < 15466) || n = 15467 || (n > 15469 && n < 15723) || n = 15724 || (n > 15726 && n < 15980) || n = 15981 || (n > 15983 && n < 16237) || n = 16238 || (n > 16240 && n < 16494) || n = 16495 || (n > 16497 && n < 16751) || n = 16752 || (n > 16754 && n < 17008) || n = 17009 || (n > 17011 && n < 17265) || n = 17266 || (n > 17268 && n < 17522) || n = 17523 || (n > 17525 && n < 17779) || n = 17780 || (n > 17782 && n < 18036) || n = 18037 || (n > 18039 && n < 18293) || n = 18294 || (n > 18296 && n < 18550) || n = 18551 || (n > 18553 && n < 18807) || n = 18808 || (n > 18810 && n < 19064) || n = 19065 || (n > 19067 && n < 19321) || n = 19322 || (n > 19324 && n < 19578) || n = 19579 || (n > 19581 && n < 19835) || n = 19836 || (n > 19838 && n < 20001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15209)
	; 9 bytes
	php
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15211 || n = 15468 || n = 15725 || n = 15982 || n = 16239 || n = 16496 || n = 16753 || n = 17010 || n = 17267 || n = 17524 || n = 17781 || n = 18038 || n = 18295 || n = 18552 || n = 18809 || n = 19066 || n = 19323 || n = 19580 || n = 19837)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15212 || n = 15469 || n = 15726 || n = 15983 || n = 16240 || n = 16497 || n = 16754 || n = 17011 || n = 17268 || n = 17525 || n = 17782 || n = 18039 || n = 18296 || n = 18553 || n = 18810 || n = 19067 || n = 19324 || n = 19581 || n = 19838)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15466)
	; 9 bytes
	php
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15723)
	; 9 bytes
	php
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15980)
	; 9 bytes
	php
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 16237)
	; 9 bytes
	php
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 16494)
	; 9 bytes
	php
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 16751)
	; 9 bytes
	php
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17008)
	; 9 bytes
	php
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17265)
	; 9 bytes
	php
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17522)
	; 9 bytes
	php
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17779)
	; 9 bytes
	php
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18036)
	; 9 bytes
	php
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18293)
	; 9 bytes
	php
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18550)
	; 9 bytes
	php
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18807)
	; 9 bytes
	php
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19064)
	; 9 bytes
	php
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19321)
	; 9 bytes
	php
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19578)
	; 9 bytes
	php
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19835)
	; 9 bytes
	php
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
