; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Z+N, and C
; Will preserve A, Y, S, D, V, and I (interrupt-disable flag)
;
; WARNING: If an interrupt happens in the middle of this code, data corruption can occur.
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
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 16
; - 6 bytes or less when n <= 19
; - 7 bytes or less when n <= 41
; - 8 bytes or less when n <= 1301
; - 9 bytes or less when n <= 1765
; -10 bytes or less when n <= 2315
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=4, avg=5.2, max=7
; - When 21 <= n <= 30: min=5, avg=6.2, max=7
; - When 31 <= n <= 50: min=5, avg=6.5, max=8
; - When 51 <= n <= 75: min=5, avg=6.4, max=8
; - When 76 <= n <= 100: min=5, avg=6.3, max=8
; - When 101 <= n <= 200: min=4, avg=6.2, max=8
; - When 201 <= n <= 300: min=5, avg=6.3, max=8
; - When 301 <= n <= 500: min=4, avg=6.3, max=8
; - When 501 <= n <= 750: min=5, avg=6.3, max=8
; - When 751 <= n <= 1000: min=5, avg=6.3, max=8
; - When 1001 <= n <= 2000: min=4, avg=7.2, max=10
; - When 2001 <= n <= 3000: min=6, avg=8.5, max=11
; - When 3001 <= n <= 5000: min=7, avg=10.2, max=11
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
.if((n >= 1) && (n < 79))
.if((n >= 1) && (n < 40))
; 1 to 39
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif((n > 3 && n < 7) || n = 23 || n = 25 || n = 28 || n = 30 || n = 33 || n = 35 || n = 38)
	; 2..7 bytes
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
.elseif(n = 9 || n = 11 || n = 13)
	; 3..5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10)
	; 4 bytes
	php
	cmp $C5
	plp
.elseif(n = 12)
	; 4 bytes
	rol $36,X
	ror $36,X
.elseif(n = 14)
	; 4 bytes
	.repeat 2
	 php
	 plp
	.endrepeat
.elseif(n = 15)
	; 5 bytes
	php
	tsx
	plp
	txs
	plp
.elseif(n = 16 || n = 18 || n = 20)
	; 5..7 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 17)
	; 6 bytes
	php
	rol $26
	ror $26
	plp
.elseif(n = 19)
	; 6 bytes
	php
	delay_n 12
	plp
.elseif(n = 21 || n = 26 || n = 36)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 22 || n = 29)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 24 || n = 34 || n = 39)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 27)
	; 7 bytes
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
.elseif(n = 31)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 32)
	; 7 bytes
	ldx #250 ;hides 'nop'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 37)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.endif
.else
; 40 to 78
.if(n = 40 || n = 47 || n = 48 || n = 52 || n = 53 || n = 55 || n = 58 || n = 60 || n = 63 || n = 65 || n = 68 || n = 70 || n = 73 || n = 75 || n = 77)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 41 || n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 42)
	; 8 bytes
	pha
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
	pla
.elseif(n = 43 || n = 57)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 44 || n = 49 || n = 54 || n = 59 || n = 67 || n = 69 || n = 74)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 45)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 62)
	; 7 bytes
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
.elseif(n = 72)
	; 7 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	dex
	Jpl *-4
.endif
.endif
.else
; 79 to 156
.if((n .mod 5 = 4) && ((n > 78 && n < 95) || (n > 103 && n < 120) || n = 129 || (n > 138 && n < 155)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 83 || n = 87 || n = 88 || n = 90 || n = 93 || n = 95 || n = 98 || n = 100 || n = 103 || n = 105 || n = 108 || n = 110 || n = 112 || n = 118 || n = 122 || n = 123 || n = 128 || n = 130 || n = 132 || n = 133 || n = 135 || n = 138 || n = 140 || n = 143 || n = 145 || n = 147 || n = 152)
	; 6..8 bytes
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
.elseif(n = 92 || n = 120 || n = 127 || n = 148 || n = 155)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 97)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 102)
	; 8 bytes
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
	pla
.elseif(n = 107)
	; 6 bytes
	ldx #24 ;hides 'clc'
	dex
	dex
	Jne *-3
.elseif(n = 117)
	; 7 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	dex
	Jpl *-4
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
.elseif(n = 137)
	; 7 bytes
	ldx #(n-9)/8
	cmp $C5
	dex
	Jpl *-3
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
; 157 to 312
.if(n = 157 || n = 158 || n = 163 || n = 165 || n = 168 || n = 173 || n = 175 || n = 178 || n = 180 || n = 182 || n = 187 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 202 || n = 203 || n = 205 || n = 208 || n = 210 || n = 212 || n = 213 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233 || n = 235 || n = 237 || n = 238 || n = 240 || n = 243 || n = 245 || n = 247 || n = 248 || n = 250 || n = 258 || n = 263 || n = 268 || n = 270 || n = 273 || n = 275 || n = 278 || n = 280 || n = 282 || n = 283 || n = 285 || n = 292 || n = 293 || n = 297 || n = 298 || n = 300 || n = 303 || n = 305 || n = 308 || n = 310)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 177 || n = 184 || n = 189 || n = 194 || n = 207 || n = 209 || n = 214 || n = 219 || n = 229 || n = 234 || n = 244 || n = 249 || n = 254 || n = 259 || n = 264 || n = 279 || n = 284 || n = 289 || n = 294 || n = 304 || n = 312)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 160 || n = 222 || n = 272)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 5 = 1) && ((n > 160 && n < 172) || n = 201 || n = 226 || n = 241 || n = 251 || n = 256 || (n > 265 && n < 282) || n = 301))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 7 = 1) && (n = 162 || n = 190 || n = 197 || (n > 252 && n < 275) || (n > 287 && n < 310)))
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
.elseif(n = 169 || n = 183 || n = 204 || n = 218 || n = 225 || n = 232 || n = 239)
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
.elseif(n = 172 || n = 217 || n = 262 || n = 307)
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
.elseif((n .mod 5 = 1) && ((n > 175 && n < 197) || (n > 205 && n < 222) || n = 231 || n = 236 || n = 246 || n = 261 || (n > 285 && n < 297) || n = 306 || n = 311))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 179)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 185 || n = 220 || n = 255 || n = 290)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 188)
	; 5 bytes
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jpl *-2
.elseif(n = 199)
	; 6 bytes
	ldx #232 ;hides 'inx'
	nop $04
	Jne *-3
.elseif(n = 215)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	dex
	Jne *-3
.elseif(n = 224)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	dex
	Jpl *-3
.elseif(n = 242)
	; 6 bytes
	ldx #(n-8)/9 ;hides 'nop'
	nop
	dex
	Jpl *-3
.elseif(n = 252)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	inx
	Jpl *-4
.elseif(n = 257)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
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
.elseif(n = 277)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 287)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 299)
	; 6 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
.endif
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 315 || n = 317 || n = 318 || n = 320 || n = 322 || n = 327 || n = 328 || n = 332 || n = 333 || n = 335 || n = 338 || n = 340 || n = 342 || n = 343 || n = 345 || n = 348 || n = 350 || n = 353 || n = 355 || n = 357 || n = 362 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 380 || n = 383 || n = 385 || n = 387 || n = 388 || n = 390 || n = 392 || n = 402 || n = 403 || n = 408 || n = 410 || n = 413 || n = 415 || n = 418 || n = 420 || n = 422 || n = 423 || n = 425 || n = 427 || n = 432 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 447 || n = 448 || n = 450 || n = 453 || n = 455 || n = 458 || n = 460 || n = 462 || n = 468)
	; 6..8 bytes
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
.elseif(n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 347 || n = 349 || n = 354 || n = 359 || n = 364 || n = 369 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 409 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 452 || n = 454 || n = 459)
	; 6..8 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 325 || n = 360 || n = 430 || n = 465)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 377)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 395)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 398)
	; 5 bytes
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
.elseif(n = 404)
	; 6 bytes
	ldx #90 ;hides 'nop'
	dex
	dex
	Jne *-3
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
.elseif(n = 417)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
.elseif(n = 457)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 464)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	inx
	Jne *-3
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
.if(n = 469 || n = 474 || n = 479 || n = 494 || n = 499 || n = 504 || n = 509 || n = 514 || n = 522 || n = 524 || n = 529 || n = 534 || n = 539)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #197 ;hides 'cmp zp'
	nop
	inx
	Jne *-3
.elseif(n = 473 || n = 475 || n = 478 || n = 482 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 502 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 527 || n = 528 || n = 535 || n = 538 || n = 542 || n = 543 || n = 545)
	; 6..8 bytes
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
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	inx
	Jne *-3
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
.elseif(n = 492)
	; 7 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 497)
	; 7 bytes
	ldx #(n-9)/8
	cmp $C5
	dex
	Jpl *-3
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
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 544)
	; 6 bytes
	ldx #n/8 ;hides 'nop zp'
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
.elseif(n = 548 || n = 553 || n = 555 || n = 558 || n = 562 || n = 563 || n = 565 || n = 570 || n = 572 || n = 573 || n = 578 || n = 580 || n = 583 || n = 585 || n = 587 || n = 588 || n = 590 || n = 593 || n = 597 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623 || n = 625)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 549 || n = 559 || n = 564 || n = 569 || n = 574 || n = 579 || n = 584 || n = 592 || n = 594 || n = 599 || n = 604 || n = 609 || n = 614 || n = 619)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 550 || n = 595 || n = 622)
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
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
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
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
.elseif(n = 567)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 568 || n = 582 || n = 589 || n = 603 || n = 610 || n = 624)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 577)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 602)
	; 6 bytes
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 607)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop $04
	Jmi *-3
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
.elseif(n = 627 || n = 634 || n = 639 || n = 644 || n = 649 || n = 654 || n = 662 || n = 664 || n = 669 || n = 679 || n = 684 || n = 689 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 732 || n = 739 || n = 744 || n = 749 || n = 754 || n = 759 || n = 767 || n = 769 || n = 774 || n = 779)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 628 || n = 630 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 665 || n = 668 || n = 670 || n = 675 || n = 677 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 693 || n = 695 || n = 698 || n = 700 || n = 702 || n = 703 || n = 705 || n = 707 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 742 || n = 747 || n = 752 || n = 753 || n = 758 || n = 760 || n = 762 || n = 763 || n = 765 || n = 768 || n = 770 || n = 772 || n = 773)
	; 6..8 bytes
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
.elseif(n = 632)
	; 7 bytes
	delay_n (n-3)
	nop $04
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
.elseif(n = 640 || n = 730 || n = 775)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 642 || n = 710 || n = 745 || n = 780)
	; 7..8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 652 || n = 659 || n = 729)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657 || n = 737 || n = 777)
	; 7 bytes
	ldx #(n-9)/8
	cmp $C5
	dex
	Jpl *-3
.elseif(n = 667 || n = 685 || n = 712)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 672)
	; 8 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-4
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
.elseif(n = 697)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.elseif(n = 734)
	; 7 bytes
	php
	delay_n (n-7)
	plp
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
.if(n = 782)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.elseif(n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 810 || n = 812 || n = 818 || n = 822 || n = 823 || n = 825 || n = 828 || n = 830 || n = 833 || n = 835 || n = 838 || n = 840 || n = 842 || n = 843 || n = 845 || n = 852 || n = 857 || n = 858 || n = 863 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 882 || n = 887 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 902 || n = 903 || n = 905 || n = 908 || n = 910 || n = 913 || n = 915 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 784 || n = 789 || n = 794 || n = 804 || n = 814 || n = 819 || n = 824 || n = 829 || n = 837 || n = 839 || n = 844 || n = 849 || n = 854 || n = 859 || n = 864 || n = 872 || n = 874 || n = 879 || n = 884 || n = 889 || n = 894 || n = 907 || n = 909 || n = 914 || n = 919 || n = 924 || n = 929 || n = 934)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #n/8 ;hides 'nop zp'
	nop
	dex
	Jne *-3
.elseif(n = 802 || n = 847)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 807)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jmi *-3
	plp
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
.elseif(n = 817 || n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 832)
	; 7 bytes
	pha
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
	pla
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
.elseif(n = 865 || n = 892)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 867)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 899)
	; 6 bytes
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
.elseif(n = 912)
	; 7 bytes
	ldx #90 ;hides 'nop'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 917)
	; 6 bytes
	ldx #26 ;hides 'nop'
	nop
	inx
	Jpl *-3
.elseif(n = 922)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
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
.if(n = 938 || n = 940 || n = 943 || n = 945 || n = 948 || n = 950 || n = 952 || n = 957 || n = 958 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 983 || n = 985 || n = 987 || n = 992 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1020 || n = 1022 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1042 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1062 || n = 1063 || n = 1067 || n = 1068 || n = 1070 || n = 1073 || n = 1075 || n = 1077 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1087 || n = 1088 || n = 1090 || n = 1092 || n = 1098 || n = 1102 || n = 1103 || n = 1105 || n = 1108 || n = 1110 || n = 1112 || n = 1113 || n = 1115 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1127 || n = 1132 || n = 1133 || n = 1137 || n = 1138 || n = 1140 || n = 1143 || n = 1145 || n = 1147 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1157 || n = 1158 || n = 1160 || n = 1167 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1182 || n = 1183 || n = 1185 || n = 1188 || n = 1190 || n = 1192 || n = 1193 || n = 1195 || n = 1197 || n = 1202 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1225 || n = 1227 || n = 1228 || n = 1230 || n = 1232 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || (n > 1092 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1012 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1039 || n = 1047 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1082 || n = 1084 || n = 1089 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1129 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1209 || n = 1214 || n = 1222 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244 || n = 1249)
	; 6..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 947)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 955 || n = 990 || n = 1025 || n = 1060 || n = 1095 || n = 1130 || n = 1165 || n = 1200 || n = 1235)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 962)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jmi *-3
.elseif(n = 972)
	; 7 bytes
	pha
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
	pla
.elseif(n = 977)
	; 7 bytes
	ldx #(n-9)/8
	cmp $C5
	dex
	Jpl *-3
.elseif(n = 980)
	; 6 bytes
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 982 || n = 1027 || n = 1162)
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
.elseif(n = 1057 || n = 1237)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1097)
	; 6 bytes
	ldx #122 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 1117 || n = 1207)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1122)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1124)
	; 6 bytes
	ldx #250 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 1152)
	; 7 bytes
	ldx #128 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1187)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jpl *-4
.elseif(n = 1217)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
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
.elseif(n = 1252 || n = 1297 || n = 1306 || n = 1315)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1262 || n = 1263 || n = 1265 || n = 1267 || n = 1272 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1287 || n = 1290 || n = 1291 || n = 1293 || n = 1298 || n = 1300 || n = 1302 || n = 1305 || n = 1307 || n = 1308 || n = 1311 || n = 1312 || n = 1314 || n = 1319 || n = 1323 || n = 1326 || n = 1328)
	; 6..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1325)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1257 || n = 1313)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1259)
	; 6 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1292 || n = 1299 || n = 1318 || n = 1320 || n = 1327)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1288 || n = 1304 || n = 1316 || n = 1322)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1294)
	; 7 bytes
	php
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1295)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jpl *-5
.elseif(n = 1301)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1309)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1321)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1325)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 6
	inx
	Jpl *-5
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1337 || n = 1353 || n = 1377)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1330)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	nop
	inx
	Jpl *-5
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1402)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1332 || n = 1343)
	; 8 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-4
.elseif((n .mod 9 = 1) && ((n > 1332 && n < 1379) || n = 1396 || n = 1405))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1334 || n = 1348 || n = 1355 || n = 1362 || n = 1372 || n = 1379 || n = 1390 || n = 1397 || n = 1402 || n = 1404)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1335 || n = 1398)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1336 || n = 1400)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1339)
	; 7 bytes
	ldx #122 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 1340 || n = 1344 || n = 1346 || n = 1347 || n = 1349 || n = 1350 || n = 1354 || n = 1356 || n = 1361 || n = 1363 || n = 1368 || n = 1370 || n = 1371 || n = 1375 || n = 1382 || n = 1384 || n = 1389 || n = 1391 || n = 1395 || n = 1403)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1341)
	; 7 bytes
	ldx #122 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1357)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1358 || n = 1367 || n = 1383 || n = 1392 || n = 1406)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1364)
	; 8 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	nop
	inx
	Jpl *-5
.elseif(n = 1365)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 1381 || n = 1393)
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
	; 8 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	nop
	inx
	Jpl *-5
.elseif(n = 1388)
	; 7 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1410 || n = 1412 || n = 1416 || n = 1417 || n = 1419 || n = 1424 || n = 1426 || n = 1428 || n = 1431 || n = 1434 || n = 1435 || n = 1437 || n = 1438 || n = 1440 || n = 1445 || n = 1447 || n = 1452 || n = 1454 || n = 1466 || n = 1469 || n = 1470 || n = 1473 || n = 1475 || n = 1476 || n = 1479 || n = 1480 || n = 1482 || n = 1483)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1479)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1409 || n = 1425 || n = 1433 || n = 1449 || n = 1481)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1411 || n = 1418 || n = 1439 || n = 1444 || n = 1446 || n = 1458 || n = 1460 || n = 1467 || n = 1474)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1413)
	; 9 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442) || (n > 1458 && n < 1478)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1421 || n = 1451)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1427 || n = 1448 || n = 1462 || n = 1484)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1430)
	; 8 bytes
	ldx #n/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 1442)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1453 || n = 1465)
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
.elseif(n = 1456 || n = 1472)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1461)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1463)
	; 8 bytes
	ldx #122 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
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
.elseif(n = 1487 || n = 1490 || n = 1491 || n = 1494 || n = 1498 || n = 1501 || n = 1503 || n = 1508 || n = 1510 || n = 1514 || n = 1515 || n = 1517)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 4) && (n = 1488 || (n > 1501 && n < 1524)))
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1489)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
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
.elseif(n = 1497)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jmi *-4
.elseif(n = 1500)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1504)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	nop
	inx
	Jne *-3
.elseif(n = 1505 || n = 1521)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1507)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	inx
	Jne *-5
.elseif(n = 1511)
	; 8 bytes
	pha
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 1512)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1518)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1519)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1526 || n = 1529 || n = 1533 || n = 1535 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1552 || n = 1557)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1528)
	; 7 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1530 || n = 1544 || n = 1551)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1537 || n = 1561)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1538)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1539)
	; 7 bytes
	ldx #116 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 1545 || n = 1553)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1546)
	; 9 bytes
	ldx #102
	rol $26
	ror $26
	dex
	Jpl *-5
.elseif(n = 1547)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	inx
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
; 1563 to 1640
.if(n = 1563 || n = 1574 || n = 1596)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1564 || n = 1566 || n = 1570 || n = 1571 || n = 1578 || n = 1580 || n = 1586 || n = 1588 || n = 1589 || n = 1592 || n = 1595 || n = 1599 || n = 1602 || n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1615 || n = 1619 || n = 1620 || n = 1622 || n = 1627 || n = 1629 || n = 1634 || n = 1636 || n = 1638)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1572 || n = 1579 || n = 1600 || n = 1607 || n = 1614 || n = 1626 || n = 1628 || n = 1635 || n = 1640)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1584 && n < 1622)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1568)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1640)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1573 || n = 1633)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1575 || n = 1591 || n = 1616 || n = 1630)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1577 || n = 1593 || n = 1601 || n = 1617)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1581 || n = 1631)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1582)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 1584)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1587)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1598)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jmi *-5
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
.elseif(n = 1623)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
.elseif(n = 1624)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.endif
.else
; 1641 to 1718
.if(n = 1641 || n = 1643 || n = 1647 || n = 1655 || n = 1658 || n = 1662 || n = 1664 || n = 1668 || n = 1669 || n = 1671 || n = 1676 || n = 1678 || n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1692 || n = 1694 || n = 1697 || n = 1699 || n = 1703 || n = 1704 || n = 1706 || n = 1708 || n = 1714 || n = 1715 || n = 1718)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1642 || n = 1656 || n = 1663 || n = 1670 || n = 1677 || n = 1687 || n = 1691 || n = 1698 || n = 1712)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1644 || n = 1652 || n = 1700)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1645 || n = 1705 || n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1645 && n < 1717)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1647 && n < 1694) || n = 1711))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1649 || n = 1665 || n = 1673 || n = 1689 || n = 1713)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1650)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1651)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1654)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 1659)
	; 7 bytes
	ldx #90 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 1661 || n = 1701)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1672 || n = 1680 || n = 1696)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1679)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jne *-5
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1707)
	; 7 bytes
	ldx #122 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 1710)
	; 9 bytes
	delay_n (n-3)
	nop $04
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1726 || n = 1733 || n = 1740 || n = 1754 || n = 1768 || n = 1780 || n = 1782 || n = 1796)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 9 = 1) && ((n > 1719 && n < 1757) || (n > 1773 && n < 1793)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1721 || n = 1745 || n = 1761 || n = 1769 || n = 1785)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1722 || n = 1725 || n = 1727 || n = 1731 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1748 || n = 1753 || n = 1755 || n = 1757 || n = 1760 || (n > 1761 && n < 1765) || n = 1767 || n = 1770 || n = 1778 || n = 1787 || n = 1788 || n = 1790 || n = 1791 || n = 1795)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724 || n = 1735 || n = 1736)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1728)
	; 7 bytes
	pha
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	pla
.elseif(n = 1741 || n = 1777 || n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1742)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
	plp
.elseif(n = 1749)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1750 || n = 1794)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1752 || n = 1784)
	; 7..8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1759)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1766)
	; 10 bytes
	php
	pha
	ldx #37 ;hides 'and zp'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 1771)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1773 || n = 1776)
	; 8..9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1775)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1781)
	; 6 bytes
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 1793)
	; 6 bytes
	ldx #0
	nop
	dex
	Jne *-2
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797 || n = 1798 || n = 1803 || n = 1812 || n = 1814 || n = 1815 || n = 1821 || n = 1830)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1800 || n = 1806 || n = 1808 || n = 1820)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 9 = 1) && ((n > 1800 && n < 1829)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1802 || n = 1804 || n = 1822 || n = 1829)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1805 || n = 1816 || n = 1827)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1807)
	; 8 bytes
	ldx #128
	delay_n 9
	dex
	Jpl *-4
.elseif(n = 1809 || n = 1817 || n = 1833)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1811)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1813 || n = 1825)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1818)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
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
.elseif(n = 1831)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1832)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 1834)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 1835)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.endif
.else
; 1836 to 1875
.if(n = 1836 || n = 1852 || n = 1858)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1874)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1838 || n = 1860 || n = 1871)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1839)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1840 || n = 1856)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1841 || n = 1857 || n = 1865)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1842)
	; 8 bytes
	pha
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 1843 || n = 1848 || n = 1850 || n = 1851 || n = 1859 || n = 1866 || n = 1867 || n = 1869)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1844)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1845)
	; 9 bytes
	php
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 1847)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1849 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1853 || n = 1862 || n = 1868)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1854)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 1863)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1870)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1872)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 1874)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	dex
	Jne *-6
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
; 1876 to 1953
.if(n = 1876 || n = 1906)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 1877 || n = 1919)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1878)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1879)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1880)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 1881 || n = 1889 || n = 1905 || n = 1913 || n = 1929 || n = 1937 || n = 1953)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1946)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1883 || n = 1884 || n = 1886 || n = 1887 || n = 1892 || n = 1899 || n = 1911 || n = 1917 || n = 1920 || n = 1935 || n = 1938 || n = 1939 || n = 1946 || n = 1947 || n = 1949)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1885 || n = 1897 || n = 1921 || n = 1933)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1888 || n = 1896 || n = 1912 || n = 1944)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1890)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 1892 && n < 1927) || n = 1948))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1894 || n = 1903 || n = 1908 || n = 1910 || n = 1924 || n = 1928 || n = 1930)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1895)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1898)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1901 || n = 1931 || n = 1941 || n = 1951)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1902)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1907)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1914 || n = 1923 || n = 1932 || n = 1934 || n = 1940 || n = 1943 || n = 1952)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1916)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1922)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 1925)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1942)
	; 9 bytes
	pha
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
.elseif(n = 1950)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
	plp
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
.elseif(n = 1955 || n = 1956 || n = 1958 || n = 1962 || n = 1965 || n = 1971 || n = 1974 || n = 1982 || n = 1983 || n = 1987)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1957 || n = 1969)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1959 || n = 1992)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1960)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1961)
	; 6 bytes
	ldx #218 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 1964)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1966)
	; 9 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $EA ;hides 'nop'
	dex
	Jne *-4
	plp
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
.elseif(n = 1970)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1973)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 1975 || n = 1989)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1976 || n = 1984)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1977 || n = 1985)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1978)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1979)
	; 7 bytes
	ldx #58 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 1980)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1986)
	; 9 bytes
	pha
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	pla
.elseif(n = 1988)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 1991)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 2005 || n = 2029)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1994 || n = 2013)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1995 || n = 2007 || n = 2010 || n = 2019 || n = 2027 || n = 2028)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1996)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 1997 || n = 1998 || n = 2006 || n = 2012 || n = 2018 || n = 2022 || n = 2024)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 9 = 1) && ((n > 1998 && n < 2027)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2000 || n = 2016)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2001 || n = 2009 || n = 2025)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 2002 || n = 2004 || n = 2020)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2003 || n = 2014)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2011 || n = 2021)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2015)
	; 8 bytes
	pha
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
	pla
.elseif(n = 2023)
	; 8 bytes
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 2030)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2031)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2040 || n = 2048)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2033)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 2034)
	; 7 bytes
	ldx #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2063)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2036 || n = 2047 || n = 2058)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2037 || n = 2039 || n = 2043 || n = 2046 || n = 2050 || n = 2051 || n = 2055 || n = 2064 || n = 2066 || n = 2067 || n = 2070)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2038 || n = 2052)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2041 || n = 2065)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2042 || n = 2054 || n = 2060)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2045 || n = 2059)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2049)
	; 7 bytes
	ldx #0
	cmp $C5
	dex
	Jne *-3
.elseif(n = 2056)
	; 9 bytes
	php
	ldx #0
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2057)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	nop $04
	inx
	Jne *-5
.elseif(n = 2061)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2063)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jne *-5
.elseif(n = 2068)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	nop
	nop $04
	inx
	Jne *-5
.elseif(n = 2069)
	; 6 bytes
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
.endif
.else
; 2071 to 2109
.if((n .mod 9 = 1) && ((n > 2070 && n < 2108)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2072)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2073 || n = 2079 || n = 2082 || n = 2095 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074 || n = 2081 || n = 2083 || n = 2090 || n = 2108)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2075)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	inx
	Jne *-5
.elseif(n = 2076 || n = 2084 || n = 2085 || n = 2094 || n = 2096)
	; 8..9 bytes
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
.elseif(n = 2086)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2087)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
.elseif(n = 2088)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2091 || n = 2102)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2092)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 2093)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2097)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2099)
	; 9 bytes
	ldx #116 ;hides 'nop zp,x'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 2105)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2106)
	; 9 bytes
	pha
	ldx #106 ;hides 'ror a'
	pla
	pha
	inx
	Jne *-4
	pla
.endif
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2119 || n = 2128 || n = 2140 || n = 2155 || n = 2164 || n = 2182)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2111 || n = 2131 || n = 2141)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2112 || n = 2120 || n = 2122 || n = 2142 || n = 2144 || n = 2150 || n = 2159 || n = 2180 || n = 2184)
	; 8..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2113 || n = 2137 || n = 2149 || n = 2173 || n = 2185)
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
.elseif(n = 2115 || n = 2129 || n = 2171)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2115 && n < 2180)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2117 || n = 2126)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2118 || n = 2127 || n = 2136 || n = 2138 || n = 2139 || n = 2145 || n = 2148 || n = 2151 || n = 2153 || n = 2154 || n = 2158 || n = 2162 || n = 2163 || n = 2165 || n = 2169 || n = 2172 || n = 2174 || n = 2175 || n = 2181 || n = 2183 || n = 2187)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2121)
	; 8 bytes
	pha
	ldx #21 ;hides 'ora zp,x'
	nop
	inx
	Jne *-3
	pla
.elseif(n = 2123)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif((n .mod 11 = 1) && ((n > 2123 && n < 2169)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
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
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2147)
	; 8 bytes
	ldx #232 ;hides 'inx'
	cmp $C5
	inx
	inx
	Jne *-5
.elseif(n = 2156)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2160)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2166)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2167)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2176)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2177)
	; 7 bytes
	ldx #58 ;hides 'nop'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2178)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2186)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
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
.elseif(n = 2189 || n = 2207)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif((n .mod 11 = 1) && ((n > 2189 && n < 2224)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2191)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2192 || n = 2193 || n = 2202 || n = 2216)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2194)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 2195)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 2196)
	; 7 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	inx
	Jne *-4
.elseif(n = 2198 || n = 2203 || n = 2208 || n = 2210 || n = 2211 || n = 2214 || n = 2217 || n = 2219 || n = 2225 || n = 2226)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2199)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2200 || n = 2218)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2204)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 2205)
	; 9 bytes
	ldx #115
	delay_n 14
	dex
	Jpl *-5
.elseif(n = 2209 || n = 2221)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2213)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2220)
	; 8 bytes
	pha
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
	pla
.elseif(n = 2222)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2255)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2228 || n = 2264)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2229 || n = 2235 || n = 2237 || n = 2244 || n = 2246 || n = 2247 || n = 2252 || n = 2253 || n = 2262)
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
.elseif(n = 2231 || n = 2261)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2232)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2232 && n < 2261)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2234 || n = 2256)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2236)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2238)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2239)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 2240)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2241)
	; 7 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2243)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2245 || n = 2257)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2254)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2258)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $05A2'
	ldx #5
	inx
	Jne *-4
.elseif(n = 2259)
	; 7 bytes
	cmp <$CD ;hides 'cmp $05A2'
	ldx #5
	inx
	Jne *-4
.elseif(n = 2263)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2265)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	inx
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	inx
	Jne *-4
	plp
.elseif(n = 2267)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $04A2'
	ldx #4
	inx
	Jne *-4
.elseif(n = 2268)
	; 7 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2268 && n < 2297)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2270 || n = 2271 || n = 2273 || n = 2279 || n = 2280 || n = 2282 || n = 2288 || n = 2298)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2272 || n = 2290)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2274)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
	plp
.elseif(n = 2275)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
	plp
.elseif(n = 2276)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $03A2'
	ldx #3
	inx
	Jne *-4
.elseif(n = 2277)
	; 7 bytes
	cmp <$CD ;hides 'cmp $03A2'
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
.elseif(n = 2284)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	inx
	Jne *-4
	plp
.elseif(n = 2285)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2286)
	; 7 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2289 || n = 2300)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2291 || n = 2301)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2292)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	inx
	Jne *-4
	plp
.elseif(n = 2294)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2295)
	; 7 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2299)
	; 7 bytes
	ldx #26 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 2302)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	inx
	Jne *-4
	plp
.elseif(n = 2303)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $00A2'
	ldx #0
	inx
	Jne *-4
.elseif(n = 2304)
	; 7 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	inx
	Jne *-4
.endif
.else
; 2305 to 2343
.if(n = 2305)
	; 7 bytes
	ldx #0
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2306 || n = 2307 || n = 2309 || n = 2314 || n = 2327 || n = 2332 || n = 2335 || n = 2343)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2334)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2310)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	dex
	Jne *-4
	plp
.elseif(n = 2311 || n = 2322 || n = 2333)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2312 || n = 2318 || n = 2324 || n = 2336 || n = 2338)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2313)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2315 || n = 2328)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2316)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2317 || n = 2329 || n = 2341)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2319)
	; 7 bytes
	ldx #24 ;hides 'clc'
	cmp $C5
	inx
	Jne *-4
.elseif(n = 2320)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2321)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2323)
	; 7 bytes
	ldx #90 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
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
.elseif(n = 2330)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2331)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2337)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2339)
	; 7 bytes
	ldx #234 ;hides 'nop'
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2340)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 2342)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if((n .mod 11 = 1) && ((n > 2343 && n < 2367) || (n > 2387 && n < 2422)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2345 || n = 2350 || n = 2357 || n = 2368 || n = 2373 || n = 2378 || n = 2379 || n = 2383 || n = 2387 || n = 2390 || n = 2391 || n = 2394 || n = 2403 || n = 2405 || n = 2412 || n = 2415 || n = 2417)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2346)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2347 || n = 2398)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2348 || n = 2349 || n = 2358 || n = 2360 || n = 2362 || n = 2370 || n = 2372 || n = 2374 || n = 2384 || n = 2396 || n = 2402 || n = 2408 || n = 2418 || n = 2420)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2351 || n = 2361 || n = 2371 || n = 2411)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2352)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif((n .mod 12 = 1) && ((n > 2352 && n < 2414)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 13 = 1) && (n = 2354 || (n > 2379 && n < 2420)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2356 || n = 2386 || n = 2416)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2359)
	; 9 bytes
	php
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2363)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2364)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	delay_n 6
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 2366 && n < 2410)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2369)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2375)
	; 8 bytes
	ldx #58 ;hides 'nop'
	nop
	nop $04
	inx
	Jne *-5
.elseif(n = 2376)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2382)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2385)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2392 || n = 2404)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2397)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2400)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2407)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	inx
	Jne *-6
.elseif(n = 2414)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2427 || n = 2429 || n = 2434 || n = 2436 || n = 2439 || n = 2455 || n = 2460 || n = 2463 || n = 2467 || n = 2469 || n = 2470 || n = 2475 || n = 2477 || n = 2478 || n = 2482 || n = 2489 || n = 2495)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423 || n = 2451 || n = 2479 || n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2424 || n = 2426 || n = 2428 || n = 2438 || n = 2440 || n = 2452 || n = 2464 || n = 2474 || n = 2488 || n = 2496)
	; 8..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 12 = 1) && ((n > 2424 && n < 2498)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2430 || n = 2442 || n = 2444 || n = 2448 || n = 2450 || n = 2456 || n = 2468 || n = 2472 || n = 2480 || n = 2492 || n = 2494)
	; 8..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2431 || n = 2441 || n = 2471 || n = 2481 || n = 2491)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2431 && n < 2499)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2433)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2445 || n = 2458 || n = 2484)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2446)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2447)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 2453)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2457)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	nop
	nop $04
	inx
	Jne *-6
.elseif(n = 2459)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	inx
	Jne *-5
.elseif(n = 2462)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2466 || n = 2483 || n = 2500)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2486)
	; 8 bytes
	ldx #n/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2490)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2499)
	; 7 bytes
	ldx #250 ;hides 'nop'
	nop $04
	dex
	Jne *-4
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
.if(n = 2501)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2502 || n = 2532)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2503 || n = 2511 || n = 2513 || n = 2522 || n = 2523 || n = 2525 || n = 2530 || n = 2539)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2508 || n = 2514 || n = 2516 || n = 2528 || n = 2538)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2505)
	; 9 bytes
	ldx #232 ;hides 'inx'
	nop
	cmp $C5
	inx
	inx
	Jne *-6
.elseif(n = 2506 || n = 2536)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2507 || n = 2535)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2509 || n = 2521 || n = 2533)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2510)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2512 || n = 2524)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2515)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2517 || n = 2534)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2518)
	; 9 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 2519)
	; 9 bytes
	ldx #116 ;hides 'nop zp,x'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 2520 || n = 2531)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2526)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
	plp
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
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2541 && n < 2576)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2543)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2544 || n = 2546 || n = 2547 || n = 2559 || n = 2565 || n = 2571)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2545 || n = 2557 || n = 2569)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2548 || n = 2554 || n = 2560 || n = 2572)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2549 || n = 2563 || n = 2577)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2550 || n = 2552 || n = 2556 || n = 2558 || n = 2570 || n = 2576)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2551)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2555)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2561)
	; 8 bytes
	ldx #0
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2562)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2566)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2567)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2568)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2573)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2574)
	; 9 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	cmp $C5
	dex
	Jne *-6
.elseif(n = 2578)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	inx
	Jne *-6
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579 || n = 2583 || n = 2592 || n = 2596 || n = 2603 || n = 2607 || n = 2609 || n = 2610)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2580 || n = 2588 || n = 2600 || n = 2612)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 12 = 1) && ((n > 2580 && n < 2618)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2582)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 2584)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2585 || n = 2602)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2586 || n = 2597 || n = 2608)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2587 || n = 2598)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2589)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2590)
	; 9 bytes
	pha
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
.elseif(n = 2591)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
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
.elseif(n = 2599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2601 || n = 2614)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2604)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
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
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2613)
	; 9 bytes
	ldx #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2615)
	; 8 bytes
	ldx #218 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
.elseif(n = 2616)
	; 9 bytes
	php
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.else
; 2618 to 2656
.if(n = 2618)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 2619 || n = 2630 || n = 2652)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2620 || n = 2622 || n = 2644)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2621 || n = 2628 || n = 2631 || n = 2632 || n = 2634 || n = 2635 || n = 2643 || n = 2645 || n = 2649 || n = 2654 || n = 2655)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2623 || n = 2642)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2624 || n = 2636 || n = 2637 || n = 2648)
	; 9..10 bytes
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
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2627 || n = 2640)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2629 || n = 2641 || n = 2653)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2633 || n = 2647)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2638)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
.elseif(n = 2639)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2646)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 2650)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 2651)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 6
	inx
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if(n = 2657 || n = 2658 || n = 2662 || n = 2667 || n = 2669 || n = 2676 || n = 2678 || n = 2679 || n = 2681 || n = 2688 || n = 2691 || n = 2693 || n = 2695 || n = 2698 || n = 2700 || n = 2706 || n = 2709 || n = 2711 || n = 2715 || n = 2719 || n = 2722 || n = 2723 || n = 2727 || n = 2730 || n = 2733)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2659 || n = 2660 || n = 2690 || n = 2708 || n = 2710 || n = 2714 || n = 2720 || n = 2724 || n = 2732)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2661 || n = 2675 || n = 2703 || n = 2717 || n = 2731)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2662 && n < 2730)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2664)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif((n .mod 12 = 1) && ((n > 2664 && n < 2726)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2666 || n = 2692 || n = 2705)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2668 || n = 2680 || n = 2728)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2670 || n = 2687 || n = 2704 || n = 2721)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2671 || n = 2686 || n = 2716)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2672)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2673)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
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
	ldx #12 ;hides 'nop abs'
	nop $0
	nop
	inx
	Jne *-5
.elseif(n = 2694)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2697)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2699)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2702)
	; 10 bytes
	pha
	ldx #11 ;hides 'anc #imm'
	lda $EA ;hides 'nop'
	nop
	inx
	Jne *-5
	pla
.elseif(n = 2712)
	; 9 bytes
	pha
	ldx #10 ;hides 'asl a'
	delay_n 4
	inx
	Jne *-4
	pla
.elseif(n = 2726)
	; 10 bytes
	pha
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
	pla
.elseif(n = 2734)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jne *-4
	plp
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 2736 || n = 2744 || n = 2756 || n = 2766)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 12 = 1) && ((n > 2736 && n < 2774)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2738)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	nop
	inx
	Jne *-5
.elseif(n = 2739 || n = 2741 || n = 2747 || n = 2748 || n = 2753 || (n > 2762 && n < 2766))
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2740 || n = 2751 || n = 2762)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2742)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2743)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 6
	dex
	Jne *-5
	plp
.elseif(n = 2745 || n = 2759)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2746)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2750)
	; 8 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	nop
	inx
	Jne *-5
.elseif(n = 2752)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2754)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2755)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2757 || n = 2770)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2758)
	; 9 bytes
	ldx #197 ;hides 'cmp zp'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 2760)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $05A2'
	ldx #5
	nop
	inx
	Jne *-5
.elseif(n = 2767)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2768)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 6
	dex
	Jne *-5
.elseif(n = 2769)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2771)
	; 7 bytes
	ldx #58 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2772)
	; 8 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	nop
	inx
	Jne *-5
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2775 || n = 2777 || n = 2786 || n = 2803 || n = 2808 || n = 2810 || n = 2811)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2776 || n = 2791)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2778 || n = 2780 || n = 2790 || n = 2792)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2779)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2781)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2782)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $03A2'
	ldx #3
	nop
	inx
	Jne *-5
.elseif(n = 2783)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	cmp $C5
	inx
	Jne *-5
.elseif(n = 2784 || n = 2795 || n = 2806)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2789)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2793)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $02A2'
	ldx #2
	nop
	inx
	Jne *-5
.elseif(n = 2794)
	; 8 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	nop
	inx
	Jne *-5
.elseif(n = 2796)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2799)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2804)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	nop
	inx
	Jne *-5
.elseif(n = 2805)
	; 8 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	nop
	inx
	Jne *-5
.elseif(n = 2807)
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	cmp $C5
	dex
	Jne *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
; 2813 to 2890
.if(n = 2813 || n = 2818 || n = 2823 || (n > 2824 && n < 2828) || n = 2830 || n = 2834 || n = 2835 || (n > 2836 && n < 2840) || n = 2842 || n = 2847 || n = 2849 || n = 2853 || n = 2856 || n = 2859 || n = 2862 || n = 2867 || n = 2868 || n = 2875 || n = 2883 || n = 2886 || n = 2890)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2814 || n = 2828 || n = 2840 || n = 2848 || n = 2850 || n = 2852 || n = 2864 || n = 2876 || n = 2878 || n = 2888)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2815 || n = 2829 || n = 2843 || n = 2871 || n = 2885)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2816)
	; 8 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	nop
	inx
	Jne *-5
.elseif(n = 2817)
	; 8 bytes
	ldx #0
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2819)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop $04
	inx
	Jne *-5
.elseif(n = 2820)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	delay_n 9
	inx
	Jne *-5
.elseif((n .mod 12 = 1) && ((n > 2820 && n < 2882)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2822 || n = 2861 || n = 2887)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2824 || n = 2844 || n = 2846 || n = 2860 || n = 2872 || n = 2884)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2831)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jne *-5
.elseif(n = 2832 || n = 2870 || n = 2889)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2836 || n = 2851 || n = 2866)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2841)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
.elseif(n = 2854)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	inx
	Jne *-6
.elseif(n = 2855)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	inx
	Jne *-6
.elseif(n = 2858 || n = 2877 || n = 2879 || n = 2880 || n = 2882)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2863)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
.elseif(n = 2865)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2873)
	; 9 bytes
	php
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2874)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2908 || n = 2925 || n = 2942 || n = 2959)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2892 || n = 2900 || n = 2906 || n = 2912 || n = 2924 || n = 2934 || n = 2936 || n = 2947 || n = 2948 || n = 2960 || n = 2962)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 12 = 1) && ((n > 2892 && n < 2966)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2894 || n = 2904 || n = 2937 || n = 2949 || n = 2951)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2895 || n = 2897 || n = 2898 || (n > 2900 && n < 2904) || n = 2907 || n = 2909 || n = 2910 || n = 2914 || n = 2919 || (n > 2920 && n < 2924) || n = 2931 || n = 2933 || n = 2935 || n = 2943 || n = 2945 || n = 2950 || n = 2954 || n = 2958 || n = 2963 || n = 2967)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2896 || n = 2911 || n = 2926 || n = 2956)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2899 || n = 2913 || n = 2927 || n = 2955)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2915)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	dex
	Jne *-5
.elseif(n = 2916 || n = 2920 || n = 2930 || n = 2932 || n = 2944 || n = 2968)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2918)
	; 10 bytes
	pha
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	pla
	pha
	dex
	Jne *-5
	pla
.elseif(n = 2928)
	; 9 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	nop $04
	inx
	Jne *-6
.elseif(n = 2938)
	; 9 bytes
	ldx #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2939)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop $04
	dex
	Jne *-5
.elseif(n = 2940)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 2946)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2952)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2957)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2961)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2964)
	; 10 bytes
	ldx #100 ;hides 'nop zp'
	delay_n 13
	inx
	Jne *-7
.elseif(n = 2966)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dex
	Jne *-6
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
.elseif(n = 2970)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 2971 || n = 2986)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2972 || n = 2984 || n = 2990 || n = 2996)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2973 || n = 2974 || n = 2979 || n = 2981 || n = 2985 || n = 2991 || n = 2994 || n = 2998 || n = 3003 || (n > 3004 && n < 3008))
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2975)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2976 || n = 2993)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2977 || n = 2989 || n = 3001)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2978 || n = 3004)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2980 || n = 2992)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2982)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2987)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	cmp $C5
	inx
	Jne *-6
.elseif(n = 2988)
	; 9 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	cmp $C5
	inx
	Jne *-6
.elseif(n = 2995)
	; 9 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	nop
	cmp $C5
	dex
	Jne *-6
.elseif(n = 2999)
	; 8 bytes
	ldx #250 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
.elseif(n = 3000)
	; 9 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	cmp $C5
	inx
	Jne *-6
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
.if(n = 3008 || n = 3018 || n = 3020 || n = 3032)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3009)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3010)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3011 || n = 3039)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3012)
	; 9 bytes
	cmp <$CD ;hides 'cmp $05A2'
	ldx #5
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3013 || n = 3025 || n = 3037)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3014 || n = 3028 || n = 3040)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3015)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3016 || n = 3031 || n = 3046)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3017 || n = 3043)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3019 || n = 3026 || n = 3027 || n = 3029 || n = 3033 || n = 3034 || n = 3038 || n = 3045)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3021)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3022)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3023)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $04A2'
	ldx #4
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3024)
	; 9 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3030)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3035)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $03A2'
	ldx #3
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3036)
	; 9 bytes
	cmp <$CD ;hides 'cmp $03A2'
	ldx #3
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3041)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3042)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	inx
	inx
	Jne *-7
.elseif(n = 3044)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $02A2'
	ldx #2
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3048)
	; 9 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3049 || n = 3061)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3050 || n = 3055 || n = 3062 || n = 3063 || n = 3065 || n = 3070 || n = 3075 || n = 3077 || (n > 3082 && n < 3086))
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3051)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3052 || n = 3064)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3053 || n = 3067 || n = 3081)
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
.elseif(n = 3056 || n = 3058 || n = 3068)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3057)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3059)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3060 || n = 3079)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3066)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 3069 || n = 3082)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3071)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $00A2'
	ldx #0
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3072)
	; 9 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	cmp $C5
	inx
	Jne *-6
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3074)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3076)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3078)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3080)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3090 || n = 3092 || n = 3101 || n = 3103 || n = 3118 || n = 3122 || n = 3124)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3087 || n = 3088 || n = 3102 || n = 3116)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3089 || n = 3105)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3091 || n = 3106 || n = 3121)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3093 || n = 3096 || n = 3097 || n = 3099 || n = 3100 || n = 3107 || n = 3110 || n = 3111 || n = 3114 || n = 3119 || n = 3125)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3094)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3095 || n = 3109 || n = 3123)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3098 || n = 3117)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3104)
	; 9 bytes
	ldx #194 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3108)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3112)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3113)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3115)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3120)
	; 10 bytes
	ldx #100 ;hides 'nop zp'
	nop
	delay_n 12
	inx
	Jne *-7
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126 || n = 3140 || n = 3154 || n = 3202)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3127 || n = 3131 || n = 3132 || n = 3138 || n = 3139 || n = 3141 || n = 3148 || n = 3149 || n = 3157 || n = 3162 || n = 3168 || n = 3175 || n = 3176 || n = 3182 || n = 3183 || n = 3188 || n = 3195 || n = 3198 || n = 3203)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3128 || n = 3135 || n = 3142 || n = 3143 || n = 3145 || n = 3150 || n = 3156 || n = 3159 || n = 3189 || n = 3190 || n = 3192)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3129 || n = 3146 || n = 3163 || n = 3180 || n = 3197)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3130)
	; 9 bytes
	php
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3133 || n = 3187)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif((n .mod 13 = 1) && ((n > 3133 && n < 3187)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3136 || n = 3166 || n = 3181 || n = 3196)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 3136 && n < 3194)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3144 || n = 3177 || n = 3191 || n = 3200)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3152)
	; 10 bytes
	ldx #197 ;hides 'cmp zp'
	.repeat 5
	 nop
	.endrepeat
	dex
	Jne *-7
.elseif((n .mod 16 = 1) && ((n > 3152 && n < 3202)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3155 || n = 3174)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3158)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3161)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3164)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-6
.elseif(n = 3167)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
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
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3172)
	; 9 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3178)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3184)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3194)
	; 9 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
.endif
.else
; 3204 to 3281
.if(n = 3204)
	; 10 bytes
	pha
	ldx #10 ;hides 'asl a'
	delay_n 6
	inx
	Jne *-5
	pla
.elseif(n = 3205 || n = 3259)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3206 || n = 3228 || n = 3242 || n = 3254 || n = 3270)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 14 = 1) && ((n > 3206 && n < 3278)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3208 || n = 3220 || n = 3229 || n = 3232 || n = 3234 || (n > 3243 && n < 3247) || n = 3255 || n = 3257 || n = 3260 || n = 3261 || n = 3268 || n = 3272 || n = 3274)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3209 || n = 3215 || n = 3218 || n = 3223 || n = 3227 || n = 3237 || n = 3239 || n = 3240 || n = 3243 || n = 3253 || n = 3258 || n = 3266 || n = 3267 || n = 3273)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3210 || n = 3224 || n = 3252 || n = 3278 || n = 3280)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 15 = 1) && ((n > 3210 && n < 3272)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif((n .mod 13 = 1) && ((n > 3211 && n < 3265)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3213)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3214 || n = 3231 || n = 3248 || n = 3265)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3216)
	; 9 bytes
	ldx #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3217 || n = 3233 || n = 3281)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3219)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3222)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3230)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 6
	dex
	Jne *-5
	plp
.elseif(n = 3236)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3247)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3250 || n = 3269)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3262)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $05A2'
	ldx #5
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3276)
	; 9 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3279)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
; 3282 to 3359
.if(n = 3282 || n = 3284 || n = 3298 || n = 3312 || n = 3340 || n = 3354)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3283 || n = 3293 || n = 3295 || n = 3304 || n = 3318 || n = 3321 || n = 3323 || n = 3330 || n = 3334 || n = 3335 || n = 3337 || n = 3349 || n = 3351 || n = 3352)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285 || n = 3287 || n = 3296 || n = 3300 || n = 3306 || n = 3310 || n = 3311 || n = 3320 || n = 3324 || n = 3325 || n = 3338 || n = 3339 || (n > 3340 && n < 3345) || n = 3353 || (n > 3354 && n < 3360))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif((n .mod 15 = 1) && ((n > 3285 && n < 3347)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3288 || n = 3307 || n = 3326)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3289)
	; 9 bytes
	cmp <$CD ;hides 'cmp $03A2'
	ldx #3
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3290 || n = 3303)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 3290 && n < 3348)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3292)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3294 || n = 3308 || n = 3336)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3297 || n = 3313 || n = 3345)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3299 || n = 3350)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3302)
	; 9 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3309)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3314)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3315)
	; 9 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3317)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	nop $04
	inx
	Jne *-6
.elseif(n = 3322)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3327)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $00A2'
	ldx #0
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3328)
	; 9 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3329)
	; 9 bytes
	ldx #0
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3332)
	; 9 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3348)
	; 9 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360 || n = 3362 || (n > 3369 && n < 3375) || n = 3380 || n = 3381 || n = 3388 || n = 3397)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3361 || n = 3375 || n = 3389)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3363 || n = 3369 || n = 3378 || n = 3386 || n = 3395)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3364 || n = 3383)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3365)
	; 9 bytes
	ldx #58 ;hides 'nop'
	rol $26
	ror $26
	inx
	Jne *-6
.elseif(n = 3366)
	; 9 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3367 || n = 3384)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3368 || n = 3382 || n = 3396 || n = 3398)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3376)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3377 || n = 3393)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3379)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3385)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3387)
	; 9 bytes
	php
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3390)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-7
.elseif(n = 3391)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3392 || n = 3394)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.endif
.else
; 3399 to 3437
.if(n = 3399)
	; 9 bytes
	ldx #56 ;hides 'sec'
	rol $26
	ror $26
	inx
	Jne *-6
.elseif(n = 3400)
	; 10 bytes
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	inx
	inx
	Jne *-7
.elseif(n = 3401 || n = 3418 || n = 3435)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3402)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3403 || n = 3417 || n = 3431)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3404 || n = 3405 || n = 3408 || n = 3411 || n = 3419 || n = 3420 || n = 3433 || n = 3437)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3406 || n = 3421 || n = 3436)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3407)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3409 || n = 3425)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3410 || n = 3424 || n = 3430)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3412)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3413 || n = 3422 || n = 3428)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3414)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dex
	Jne *-6
.elseif(n = 3415)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	dex
	Jne *-6
.elseif(n = 3416)
	; 10 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	nop
	nop $04
	inx
	Jne *-7
.elseif(n = 3423)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3426 || n = 3434)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3427)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3429)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	nop $04
	dex
	Jne *-6
.elseif(n = 3432)
	; 10 bytes
	ldx #100 ;hides 'nop zp'
	delay_n 16
	inx
	Jne *-7
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
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3439)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3440)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3441 || n = 3457)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3442)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3443 || n = 3447 || n = 3453 || n = 3454 || n = 3461 || n = 3463 || n = 3468 || n = 3471 || n = 3475)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3444 || n = 3446 || n = 3455 || n = 3456 || n = 3458 || n = 3460 || n = 3465 || n = 3470 || n = 3472 || n = 3474)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3445 || n = 3459 || n = 3473)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3448 || n = 3476)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3449)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3450)
	; 9 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 10
	Jne *-6
.elseif(n = 3451 || n = 3466)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3452 || n = 3469)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3462)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3464)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
.elseif(n = 3467)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	.repeat 5
	 nop
	.endrepeat
	inx
	Jne *-7
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3483 || n = 3488 || n = 3498 || n = 3507 || n = 3513)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3478)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3479)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3480 || n = 3494 || n = 3506 || n = 3508)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3481 || n = 3496 || n = 3511)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3482 || n = 3495 || n = 3510 || n = 3512)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3484)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-7
.elseif(n = 3485)
	; 9 bytes
	ldx #90 ;hides 'nop'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 3486 || n = 3503)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3487 || n = 3501 || n = 3515)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3489 || n = 3505)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3490 || n = 3504)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3491)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3492)
	; 10 bytes
	ldx #n/18 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3493)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
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
	; 10 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	nop
	cmp $C5
	inx
	Jne *-7
.elseif(n = 3502)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3509)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3514)
	; 10 bytes
	cmp <$CD ;hides 'cmp $05A2'
	ldx #5
	nop
	cmp $C5
	inx
	Jne *-7
.endif
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3535 || n = 3573 || n = 3592)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3517 || n = 3519 || n = 3523 || n = 3528 || n = 3531 || n = 3533 || n = 3540 || n = 3545 || n = 3547 || n = 3555 || n = 3558 || n = 3559 || n = 3561 || n = 3587 || n = 3589 || n = 3590)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 4) && ((n > 3517 && n < 3575)))
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3520 || n = 3537 || n = 3554 || n = 3588)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3521 || n = 3553 || n = 3569)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif((n .mod 14 = 8) && ((n > 3521 && n < 3579)))
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3524)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3525 || n = 3530 || n = 3534 || n = 3544 || n = 3548 || n = 3549 || n = 3551 || n = 3552 || n = 3562 || (n > 3565 && n < 3569) || n = 3576 || n = 3577 || (n > 3579 && n < 3583) || n = 3593)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3526 || n = 3541 || n = 3556 || n = 3586)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3527)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $04A2'
	ldx #4
	nop
	cmp $C5
	inx
	Jne *-7
.elseif((n .mod 14 = 1) && ((n > 3528 && n < 3572)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3538)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3539)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3542)
	; 10 bytes
	cmp <$CD ;hides 'cmp $03A2'
	ldx #3
	nop
	cmp $C5
	inx
	Jne *-7
.elseif(n = 3563)
	; 9 bytes
	ldx #58 ;hides 'nop'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 3565 || n = 3583)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3570)
	; 10 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	nop
	cmp $C5
	inx
	Jne *-7
.elseif(n = 3572)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	delay_n 13
	inx
	Jne *-7
.elseif(n = 3575)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3579)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
.elseif(n = 3584)
	; 10 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	nop
	cmp $C5
	inx
	Jne *-7
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3591)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 11
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
; 3594 to 3671
.if(n = 3594 || n = 3603 || n = 3607 || n = 3618 || n = 3624 || n = 3632 || n = 3635 || n = 3641 || n = 3648 || n = 3651 || n = 3663 || n = 3667)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 3594 && n < 3599) || n = 3600 || n = 3604 || n = 3606 || (n > 3607 && n < 3611) || n = 3612 || n = 3614 || n = 3615 || n = 3621 || n = 3623 || (n > 3624 && n < 3630) || n = 3636 || n = 3638 || n = 3640 || n = 3642 || n = 3643 || n = 3645 || n = 3647 || n = 3650 || n = 3652 || n = 3654 || n = 3657 || n = 3662 || n = 3664 || n = 3666 || n = 3669)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3599)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-6
.elseif((n .mod 15 = 1) && ((n > 3600 && n < 3662)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3602)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3605 || n = 3622 || n = 3639 || n = 3656)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3611 || n = 3630 || n = 3668)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3613 || n = 3634)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif((n .mod 16 = 1) && ((n > 3616 && n < 3666)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3619 || n = 3637 || n = 3655)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3620)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	.repeat 5
	 nop
	.endrepeat
	dex
	Jne *-7
.elseif(n = 3644)
	; 9 bytes
	php
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3653)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
.elseif(n = 3658)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3659)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3660)
	; 10 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3670)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3671)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 11
	inx
	Jne *-6
.endif
.else
; 3672 to 3750
.if(n = 3672 || n = 3675 || n = 3678 || n = 3683 || n = 3692 || n = 3693 || n = 3699 || n = 3708 || n = 3709 || n = 3715 || n = 3723 || n = 3726 || n = 3731 || n = 3738 || n = 3746 || n = 3747)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3673 || n = 3690 || n = 3707 || n = 3724 || n = 3741)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3674)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-6
.elseif((n .mod 15 = 1) && ((n > 3675 && n < 3737)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3677 || n = 3680 || n = 3684 || n = 3685 || n = 3688 || n = 3694 || n = 3696 || n = 3700 || n = 3701 || n = 3703 || n = 3704 || n = 3710 || n = 3712 || n = 3716 || n = 3717 || n = 3719 || n = 3720 || n = 3722 || n = 3728 || n = 3730 || n = 3732 || n = 3733 || n = 3737 || n = 3748)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3679)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif((n .mod 16 = 1) && ((n > 3680 && n < 3746)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3682)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3686 || n = 3702 || n = 3718 || n = 3750)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3687 || n = 3725 || n = 3744)
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
.elseif(n = 3695)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 3698 || n = 3714)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3705)
	; 9 bytes
	ldx #218 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 3711)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3727)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3734)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3735)
	; 10 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3739)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3740)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	delay_n 12
	inx
	Jne *-7
.elseif(n = 3742)
	; 10 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3743)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3749)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 8
	dex
	Jne *-6
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
.if((n .mod 15 = 1) && ((n > 3750 && n < 3827)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3752 || (n > 3753 && n < 3757) || n = 3767 || (n > 3768 && n < 3772) || n = 3773 || n = 3776 || n = 3785 || n = 3786 || (n > 3787 && n < 3791) || n = 3800 || (n > 3803 && n < 3809) || n = 3812 || n = 3818 || n = 3819 || n = 3821)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3753 || n = 3760 || n = 3765 || n = 3768 || n = 3779 || n = 3783 || n = 3784 || n = 3794 || n = 3795 || n = 3798 || n = 3803 || n = 3813 || n = 3816 || n = 3822 || n = 3827 || n = 3828)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3757)
	; 10 bytes
	pha
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
	pla
.elseif(n = 3758 || n = 3792 || n = 3809)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3759)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3761 || n = 3777 || n = 3793 || n = 3825)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3762)
	; 10 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 11
	dex
	Jne *-7
.elseif(n = 3763 || n = 3782 || n = 3801 || n = 3820)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3764)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $05A2'
	ldx #5
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3772)
	; 10 bytes
	pha
	ldx #5 ;hides 'ora zp'
	nop
	pla
	pha
	inx
	Jne *-5
	pla
.elseif(n = 3774)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	nop $04
	inx
	Jne *-7
.elseif(n = 3775)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3778)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3780)
	; 10 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3787)
	; 10 bytes
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
	pla
.elseif(n = 3791)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3797)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3799)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 12
	inx
	Jne *-6
.elseif(n = 3802 || n = 3823)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3810)
	; 10 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3814)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3815)
	; 9 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3817)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3824)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	delay_n 6
	inx
	Jne *-7
.endif
.else
; 3829 to 3906
.if((n > 3828 && n < 3832) || (n > 3835 && n < 3839) || n = 3842 || (n > 3845 && n < 3851) || n = 3852 || n = 3854 || n = 3855 || n = 3861 || n = 3864 || (n > 3865 && n < 3871) || n = 3872 || n = 3874 || n = 3876 || n = 3878 || (n > 3879 && n < 3886) || n = 3888 || n = 3892 || n = 3893 || n = 3895 || n = 3897 || n = 3899 || n = 3900)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3832)
	; 9 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3833)
	; 9 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 3834 || n = 3845 || n = 3859 || n = 3862 || n = 3875 || n = 3879 || n = 3891 || n = 3898 || n = 3906)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3835 || n = 3853 || n = 3871)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3839 || n = 3858 || n = 3896)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3840)
	; 10 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3841)
	; 9 bytes
	ldx #0
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3843 || n = 3860 || n = 3877 || n = 3894)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3844 || n = 3865 || n = 3886)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3851)
	; 9 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3856)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
.elseif((n .mod 16 = 1) && ((n > 3856 && n < 3906)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3863)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3887)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3890)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3901)
	; 9 bytes
	php
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3902)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-7
.elseif(n = 3903)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3904)
	; 9 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	delay_n 7
	inx
	Jne *-6
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907 || n = 3925 || n = 3961)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3908 || n = 3912 || n = 3914 || n = 3916 || n = 3918 || n = 3924 || n = 3927 || n = 3929 || n = 3931 || n = 3933 || n = 3935 || n = 3938 || n = 3941 || n = 3944 || n = 3946 || n = 3948 || (n > 3949 && n < 3953) || n = 3954 || (n > 3955 && n < 3959) || n = 3963 || (n > 3964 && n < 3969) || n = 3973 || n = 3975 || n = 3976 || n = 3980 || n = 3982)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3909)
	; 9 bytes
	ldx #26 ;hides 'nop'
	rol $26
	ror $26
	inx
	Jne *-6
.elseif(n = 3910)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3979)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3913 || n = 3917 || n = 3930 || n = 3936 || n = 3939 || n = 3947 || n = 3955 || n = 3964 || n = 3971 || n = 3981)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3915 || n = 3934 || n = 3972)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3919)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3920)
	; 10 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 12
	dex
	Jne *-7
.elseif((n .mod 16 = 1) && ((n > 3920 && n < 3970)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3922)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3923)
	; 9 bytes
	ldx #218 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 3926)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-7
.elseif(n = 3932)
	; 10 bytes
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 3940)
	; 10 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3942)
	; 10 bytes
	pha
	ldx #10 ;hides 'asl a'
	nop
	pla
	pha
	inx
	Jne *-5
	pla
.elseif(n = 3943)
	; 9 bytes
	ldx #24 ;hides 'clc'
	rol $26
	ror $26
	inx
	Jne *-6
.elseif(n = 3949 || n = 3970)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3959)
	; 10 bytes
	ldx #58 ;hides 'nop'
	delay_n 13
	inx
	Jne *-7
.elseif(n = 3960)
	; 10 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3974)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3977)
	; 9 bytes
	ldx #234 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 3978)
	; 10 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 3983)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3984)
	; 9 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
.endif
.else
; 3985 to 4062
.if((n .mod 16 = 1) && ((n > 3984 && n < 4050)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3986 || n = 3987 || n = 3993 || n = 3998 || n = 4002 || n = 4003 || n = 4018 || n = 4019 || n = 4034 || n = 4035 || n = 4050)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 3987 && n < 3991) || n = 3992 || n = 3995 || n = 4004 || n = 4005 || n = 4007 || n = 4009 || n = 4014 || (n > 4019 && n < 4027) || (n > 4035 && n < 4046) || n = 4052 || n = 4053 || (n > 4054 && n < 4059) || (n > 4059 && n < 4063))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3994)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 8
	inx
	Jne *-7
.elseif(n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3997 || n = 4051)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3999)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 4000)
	; 9 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4006)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4008)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4011)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	.repeat 5
	 nop
	.endrepeat
	inx
	Jne *-7
.elseif(n = 4012 || n = 4054)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4015)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4016)
	; 9 bytes
	cmp <$CD ;hides 'cmp $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4027)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4028)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-7
.elseif(n = 4031)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4032)
	; 9 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4046)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4059)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 8
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
; 4063 to 4140
.if(n = 4063)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $02A2'
	ldx #2
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4065)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4066 || n = 4082 || n = 4083 || n = 4088 || n = 4099 || n = 4100 || n = 4107 || n = 4117 || n = 4126 || n = 4134)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4067 || n = 4086 || n = 4105 || n = 4124)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4068)
	; 10 bytes
	ldx #n/18 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4069 || n = 4087 || n = 4123)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif((n > 4069 && n < 4075) || n = 4076 || n = 4077 || n = 4084 || n = 4085 || (n > 4088 && n < 4095) || (n > 4100 && n < 4105) || n = 4106 || (n > 4107 && n < 4114) || (n > 4117 && n < 4123) || n = 4125 || (n > 4126 && n < 4130) || n = 4131 || n = 4133 || n = 4135 || n = 4137 || n = 4140)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4075 || n = 4138)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4078)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4079)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4080)
	; 9 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4095)
	; 9 bytes
	cmp #<$CD ;hides 'cmp $00A2'
	ldx #0
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4096)
	; 9 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4097)
	; 9 bytes
	ldx #0
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4114)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 4116)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
.elseif(n = 4130)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 8
	dex
	Jne *-7
.elseif(n = 4136)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	delay_n 16
	inx
	Jne *-7
.elseif(n = 4139)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 11
	inx
	Jne *-6
.endif
.else
; 4141 to 4218
.if(n = 4141)
	; 9 bytes
	ldx #218 ;hides 'nop'
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4142 || n = 4144 || n = 4148 || n = 4150 || (n > 4151 && n < 4157) || n = 4161 || n = 4163 || n = 4165 || n = 4167 || (n > 4168 && n < 4175) || n = 4176 || n = 4178 || n = 4179 || n = 4182 || n = 4184 || (n > 4185 && n < 4195) || (n > 4195 && n < 4199) || (n > 4202 && n < 4211) || n = 4212 || (n > 4213 && n < 4217) || n = 4218)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4143 || n = 4162 || n = 4181)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4145 || n = 4151 || n = 4160 || n = 4164 || n = 4168 || n = 4185 || n = 4202)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4146)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 6
	dex
	Jne *-7
.elseif(n = 4147)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	.repeat 5
	 nop
	.endrepeat
	dex
	Jne *-7
.elseif(n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
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
.elseif(n = 4159 || n = 4177 || n = 4195 || n = 4213)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4175)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 4180 || n = 4201)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4199)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 4211)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.endif
.endif
.else
; 4219 to 4375
.if(n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295 || n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4220 || (n > 4222 && n < 4229) || n = 4230 || n = 4232 || n = 4233 || n = 4235 || n = 4237 || n = 4241 || n = 4242 || n = 4244 || n = 4245 || n = 4250 || n = 4252 || (n > 4253 && n < 4257) || n = 4260 || n = 4261 || n = 4263 || n = 4266 || n = 4269 || (n > 4270 && n < 4276) || n = 4277 || (n > 4279 && n < 4283) || n = 4284 || n = 4286 || (n > 4287 && n < 4295) || n = 4296 || (n > 4297 && n < 4302) || n = 4305 || (n > 4306 && n < 4312) || n = 4313 || n = 4315 || n = 4317 || n = 4318 || n = 4320 || (n > 4321 && n < 4327) || (n > 4327 && n < 4333) || n = 4337 || (n > 4339 && n < 4348) || (n > 4348 && n < 4352) || n = 4358 || (n > 4359 && n < 4369) || n = 4370 || n = 4372 || n = 4374)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4221 || n = 4236 || n = 4240 || n = 4248 || n = 4253 || n = 4259 || n = 4270 || n = 4278 || n = 4287 || n = 4297 || n = 4304 || n = 4316 || n = 4321 || n = 4335 || n = 4338 || n = 4354 || n = 4355)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4222 || n = 4243 || n = 4264 || n = 4306 || n = 4327 || n = 4348)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4229)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 4231 || n = 4267 || n = 4303 || n = 4339 || n = 4357 || n = 4375)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4234 || n = 4251 || n = 4268 || n = 4285 || n = 4302 || n = 4319 || n = 4336)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4239)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 11
	dex
	Jne *-7
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
.elseif(n = 4249)
	; 9 bytes
	ldx #250 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 4258)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4262)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-6
.elseif(n = 4265)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 4279)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 11
	dex
	Jne *-7
.elseif(n = 4283)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4312)
	; 10 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 14
	dex
	Jne *-7
.elseif(n = 4334)
	; 10 bytes
	ldx #197 ;hides 'cmp zp'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 4353)
	; 9 bytes
	ldx #0
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4356)
	; 10 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 14
	dex
	Jne *-7
.elseif(n = 4359)
	; 10 bytes
	ldx #218 ;hides 'nop'
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4369)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 12
	inx
	Jne *-6
.elseif(n = 4373)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if((n > 4375 && n < 4388) || n = 4389 || (n > 4393 && n < 4407) || n = 4408 || n = 4410 || (n > 4411 && n < 4415) || n = 4416 || (n > 4417 && n < 4428) || n = 4431 || (n > 4432 && n < 4445) || n = 4446 || n = 4448 || n = 4450 || n = 4452 || (n > 4453 && n < 4464) || n = 4467 || (n > 4468 && n < 4472) || n = 4473 || (n > 4474 && n < 4481) || n = 4484 || n = 4486 || (n > 4487 && n < 4493) || n = 4494 || (n > 4495 && n < 4499) || n = 4503 || n = 4505 || (n > 4506 && n < 4516) || (n > 4519 && n < 4523) || n = 4524 || (n > 4525 && n < 4532))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4388)
	; 10 bytes
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
.elseif(n = 4390 || n = 4409 || n = 4428 || n = 4447 || n = 4466 || n = 4485 || n = 4504 || n = 4523)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4391)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4392 || n = 4417 || n = 4430 || n = 4449 || n = 4468 || n = 4487 || n = 4506 || n = 4525)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4393 || n = 4411 || n = 4429 || n = 4483 || n = 4501 || n = 4519)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4407)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 12
	inx
	Jne *-6
.elseif(n = 4415)
	; 9 bytes
	php
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4432 || n = 4453 || n = 4474 || n = 4495 || n = 4516)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4445)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4451)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4464)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	rol $26
	ror $26
	inx
	Jne *-7
.elseif(n = 4465)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 17
	Jne *-7
.elseif(n = 4472)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4481)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4482)
	; 10 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4493)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4499)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4500)
	; 10 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4502)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	rol $26
	ror $26
	inx
	Jne *-7
.elseif(n = 4517)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $05A2'
	ldx #5
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4518)
	; 10 bytes
	cmp <$CD ;hides 'cmp $05A2'
	ldx #5
	delay_n 9
	inx
	Jne *-7
.endif
.else
; 4532 to 4687
.if((n > 4531 && n < 4535) || (n > 4537 && n < 4542) || n = 4543 || (n > 4544 && n < 4553) || n = 4556 || n = 4557 || n = 4559 || n = 4560 || n = 4562 || (n > 4563 && n < 4571) || (n > 4573 && n < 4577) || n = 4578 || n = 4581 || (n > 4582 && n < 4589) || (n > 4591 && n < 4599) || (n > 4601 && n < 4607) || (n > 4609 && n < 4616) || n = 4617 || (n > 4621 && n < 4635) || n = 4636 || n = 4638 || n = 4640 || n = 4641 || (n > 4642 && n < 4654) || n = 4655 || n = 4657 || (n > 4658 && n < 4663) || (n > 4663 && n < 4668) || (n > 4668 && n < 4672) || n = 4673 || n = 4676 || n = 4678 || (n > 4679 && n < 4684) || (n > 4684 && n < 4688))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4535)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $04A2'
	ldx #4
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4536)
	; 10 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4537 || n = 4555 || n = 4573 || n = 4591)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4542 || n = 4561 || n = 4580 || n = 4599 || n = 4618 || n = 4637 || n = 4656 || n = 4675)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4544 || n = 4563 || n = 4582 || n = 4601 || n = 4620 || n = 4639 || n = 4658 || n = 4674 || n = 4677)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4553)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $03A2'
	ldx #3
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4554)
	; 10 bytes
	cmp <$CD ;hides 'cmp $03A2'
	ldx #3
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4558 || n = 4579 || n = 4600 || n = 4621 || n = 4642 || n = 4663 || n = 4684)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4571)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $02A2'
	ldx #2
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4572)
	; 10 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4577)
	; 9 bytes
	ldx #218 ;hides 'nop'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 4589)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4590)
	; 10 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4607)
	; 10 bytes
	cmp #<$CD ;hides 'cmp $00A2'
	ldx #0
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4608)
	; 10 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4609)
	; 10 bytes
	ldx #0
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4616)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4619)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
.elseif(n = 4635)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4654)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4668)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-7
.elseif(n = 4672)
	; 9 bytes
	php
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4679)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 13
	dex
	Jne *-7
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
; 4688 to 4843
.if((n > 4687 && n < 4691) || n = 4692 || n = 4693 || n = 4695 || n = 4697 || n = 4698 || (n > 4699 && n < 4705) || (n > 4705 && n < 4713) || n = 4716 || n = 4717 || (n > 4719 && n < 4726) || (n > 4726 && n < 4732) || n = 4733 || (n > 4734 && n < 4739) || (n > 4739 && n < 4747) || n = 4748 || n = 4750 || n = 4752 || (n > 4753 && n < 4768) || n = 4769 || n = 4771 || (n > 4772 && n < 4789) || n = 4790 || (n > 4791 && n < 4808) || n = 4809 || (n > 4810 && n < 4827) || n = 4828 || n = 4830 || (n > 4831 && n < 4844))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4691)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 16
	inx
	Jne *-7
.elseif(n = 4694 || n = 4713 || n = 4732 || n = 4751 || n = 4770 || n = 4789 || n = 4808 || n = 4827)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4696 || n = 4715 || n = 4734 || n = 4753 || n = 4772 || n = 4791)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4699)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 11
	inx
	Jne *-7
.elseif(n = 4705 || n = 4726 || n = 4747 || n = 4768 || n = 4810 || n = 4831)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4714)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 4718)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4719)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 13
	inx
	Jne *-7
.elseif(n = 4739)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 11
	inx
	Jne *-7
.elseif(n = 4749)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4829)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 14
	inx
	Jne *-6
.endif
.else
; 4844 to 5000
.if(n = 4844 || n = 4845 || n = 4847 || n = 4849 || n = 4851 || (n > 4852 && n < 4859) || (n > 4859 && n < 4865) || n = 4866 || (n > 4867 && n < 4871) || n = 4872 || n = 4874 || n = 4876 || n = 4877 || (n > 4879 && n < 4894) || (n > 4894 && n < 4898) || (n > 4899 && n < 4913) || n = 4914 || (n > 4915 && n < 4921) || (n > 4921 && n < 4929) || n = 4930 || n = 4932 || n = 4933 || n = 4935 || (n > 4936 && n < 4955) || n = 4956 || (n > 4957 && n < 4976) || n = 4977 || (n > 4978 && n < 4999) || n = 5000)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4846)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4848 || n = 4867 || n = 4931)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4850)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 12
	dex
	dex
	Jne *-7
.elseif(n = 4852 || n = 4873 || n = 4894 || n = 4915 || n = 4936 || n = 4957 || n = 4978 || n = 4999)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4859)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 11
	dex
	Jne *-7
.elseif(n = 4865)
	; 9 bytes
	ldx #0
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4871)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 4875)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 14
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
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4898)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 4899)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 11
	dex
	Jne *-7
.elseif(n = 4913)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 4921)
	; 10 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 14
	dex
	Jne *-7
.elseif(n = 4929)
	; 9 bytes
	php
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4934)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4955)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4976)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 12
	inx
	Jne *-7
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
; 5001 to 6250
.if((n > 5000 && n < 5013) || (n > 5013 && n < 5020) || (n > 5020 && n < 5041) || (n > 5041 && n < 5062) || (n > 5062 && n < 5083) || (n > 5083 && n < 5102) || n = 5103 || (n > 5104 && n < 5123) || n = 5124 || (n > 5125 && n < 5144) || n = 5145 || (n > 5146 && n < 5167) || (n > 5167 && n < 5186) || n = 5187 || (n > 5188 && n < 5209) || (n > 5209 && n < 5230) || (n > 5230 && n < 5249) || n = 5250 || (n > 5251 && n < 5272) || (n > 5272 && n < 5277) || (n > 5277 && n < 5289) || (n > 5289 && n < 5293) || (n > 5293 && n < 5312) || n = 5313 || (n > 5314 && n < 5335) || (n > 5335 && n < 5356) || (n > 5356 && n < 5377) || (n > 5377 && n < 5381) || (n > 5381 && n < 5404) || (n > 5404 && n < 5427) || (n > 5427 && n < 5443) || n = 5444 || (n > 5445 && n < 5450) || (n > 5450 && n < 5588) || (n > 5588 && n < 5611) || (n > 5611 && n < 5634) || (n > 5634 && n < 5700) || n = 5701 || (n > 5702 && n < 5749) || (n > 5749 && n < 5957) || n = 5958 || (n > 5959 && n < 6214) || n = 6215 || (n > 6216 && n < 6251))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5013)
	; 10 bytes
	ldx #218 ;hides 'nop'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5020 || n = 5041 || n = 5062 || n = 5083 || n = 5104 || n = 5125 || n = 5146 || n = 5167 || n = 5209 || n = 5230 || n = 5251 || n = 5272 || n = 5293 || n = 5314 || n = 5335 || n = 5356)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5102)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5123)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5144)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 12
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
.elseif(n = 5188 || n = 5445 || n = 5702 || n = 5959 || n = 6216)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5249)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 5277)
	; 10 bytes
	ldx #202 ;hides 'dex'
	.repeat 3
	 php
	 plp
	.endrepeat
	Jne *-7
.elseif(n = 5289)
	; 10 bytes
	ldx #26 ;hides 'nop'
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5312)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 14
	dex
	dex
	Jne *-7
.elseif(n = 5377)
	; 10 bytes
	ldx #0
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5381)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5404)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 5427)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5443)
	; 9 bytes
	php
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5450)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 5588)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 14
	dex
	Jne *-7
.elseif(n = 5611)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5634)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 14
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
.elseif(n = 5749)
	; 10 bytes
	ldx #250 ;hides 'nop'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5957)
	; 9 bytes
	php
	pha
	lax #23
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
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6471) || n = 6472 || (n > 6473 && n < 6728) || n = 6729 || (n > 6730 && n < 6985) || n = 6986 || (n > 6987 && n < 7242) || n = 7243 || (n > 7244 && n < 7499) || n = 7500)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6471)
	; 9 bytes
	php
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6473 || n = 6730 || n = 6987 || n = 7244)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6728)
	; 9 bytes
	php
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6985)
	; 9 bytes
	php
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7242)
	; 9 bytes
	php
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	plp
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
.else
; 7501 to 10000
.if(n = 7501 || n = 7758 || n = 8015 || n = 8272 || n = 8529 || n = 8786 || n = 9043 || n = 9300 || n = 9557 || n = 9814)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 7501 && n < 7756) || n = 7757 || (n > 7758 && n < 8013) || n = 8014 || (n > 8015 && n < 8270) || n = 8271 || (n > 8272 && n < 8527) || n = 8528 || (n > 8529 && n < 8784) || n = 8785 || (n > 8786 && n < 9041) || n = 9042 || (n > 9043 && n < 9298) || n = 9299 || (n > 9300 && n < 9555) || n = 9556 || (n > 9557 && n < 9812) || n = 9813 || (n > 9814 && n < 10001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7756)
	; 9 bytes
	php
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8013)
	; 9 bytes
	php
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8270)
	; 9 bytes
	php
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8527)
	; 9 bytes
	php
	pha
	lax #33
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
.elseif(n = 9041)
	; 9 bytes
	php
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9298)
	; 9 bytes
	php
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9555)
	; 9 bytes
	php
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	plp
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
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10069) || n = 10070 || (n > 10071 && n < 10326) || n = 10327 || (n > 10328 && n < 10583) || n = 10584 || (n > 10585 && n < 10840) || n = 10841 || (n > 10842 && n < 11097) || n = 11098 || (n > 11099 && n < 11354) || n = 11355 || (n > 11356 && n < 11611) || n = 11612 || (n > 11613 && n < 11868) || n = 11869 || (n > 11870 && n < 12125) || n = 12126 || (n > 12127 && n < 12382) || n = 12383 || (n > 12384 && n < 12639) || n = 12640 || (n > 12641 && n < 12896) || n = 12897 || (n > 12898 && n < 13153) || n = 13154 || (n > 13155 && n < 13410) || n = 13411 || (n > 13412 && n < 13667) || n = 13668 || (n > 13669 && n < 13924) || n = 13925 || (n > 13926 && n < 14181) || n = 14182 || (n > 14183 && n < 14438) || n = 14439 || (n > 14440 && n < 14695) || n = 14696 || (n > 14697 && n < 14952) || n = 14953 || (n > 14954 && n < 15001))
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
.elseif(n = 10071 || n = 10328 || n = 10585 || n = 10842 || n = 11099 || n = 11356 || n = 11613 || n = 11870 || n = 12127 || n = 12384 || n = 12641 || n = 12898 || n = 13155 || n = 13412 || n = 13669 || n = 13926 || n = 14183 || n = 14440 || n = 14697 || n = 14954)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 12639)
	; 9 bytes
	php
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	plp
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
.else
; 15001 to 20000
.if((n > 15000 && n < 15209) || n = 15210 || (n > 15211 && n < 15466) || n = 15467 || (n > 15468 && n < 15723) || n = 15724 || (n > 15725 && n < 15980) || n = 15981 || (n > 15982 && n < 16237) || n = 16238 || (n > 16239 && n < 16494) || n = 16495 || (n > 16496 && n < 16751) || n = 16752 || (n > 16753 && n < 17008) || n = 17009 || (n > 17010 && n < 17265) || n = 17266 || (n > 17267 && n < 17522) || n = 17523 || (n > 17524 && n < 17779) || n = 17780 || (n > 17781 && n < 18036) || n = 18037 || (n > 18038 && n < 18293) || n = 18294 || (n > 18295 && n < 18550) || n = 18551 || (n > 18552 && n < 18807) || n = 18808 || (n > 18809 && n < 19064) || n = 19065 || (n > 19066 && n < 19321) || n = 19322 || (n > 19323 && n < 19578) || n = 19579 || (n > 19580 && n < 19835) || n = 19836 || (n > 19837 && n < 20001))
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
