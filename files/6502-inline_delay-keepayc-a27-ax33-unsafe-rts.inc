; delay_n, delays n cycles with small code
; Will clobber X, Z+N, D, and V
; Will preserve A, Y, S, C, and I (interrupt-disable flag)
;
; Uses JMP (unrelocatable code).
;
; WARNING: If an interrupt happens in the middle of this code, data corruption can occur.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).
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
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 16
; - 6 bytes or less when n <= 19
; - 7 bytes or less when n <= 31
; - 8 bytes or less when n <= 1301
; - 9 bytes or less when n <= 2317
; -10 bytes or less when n <= 3091
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=5.0, max=7
; - When 21 <= n <= 30: min=5, avg=6.1, max=7
; - When 31 <= n <= 50: min=5, avg=6.5, max=8
; - When 51 <= n <= 75: min=5, avg=6.3, max=8
; - When 76 <= n <= 100: min=5, avg=6.3, max=8
; - When 101 <= n <= 200: min=4, avg=6.3, max=8
; - When 201 <= n <= 300: min=5, avg=6.3, max=8
; - When 301 <= n <= 500: min=4, avg=6.3, max=8
; - When 501 <= n <= 750: min=5, avg=6.3, max=8
; - When 751 <= n <= 1000: min=5, avg=6.4, max=8
; - When 1001 <= n <= 2000: min=4, avg=7.1, max=9
; - When 2001 <= n <= 3000: min=6, avg=8.3, max=10
; - When 3001 <= n <= 5000: min=7, avg=9.9, max=11
; - When 5001 <= n <= 7500: min=9, avg=11.0, max=11
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
	bit $24
.elseif((n > 3 && n < 7) || n = 23 || n = 25 || n = 28 || n = 30 || n = 33 || n = 38)
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
.elseif(n = 9 || n = 11 || n = 13 || n = 19)
	; 3..5 bytes
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
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 7 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	dex
	Jpl *-4
.elseif(n = 31)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 32 || n = 34 || n = 39)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
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
.if(n = 40 || n = 47 || n = 48 || n = 52 || n = 53 || n = 58 || n = 60 || n = 63 || n = 65 || n = 68 || n = 70 || n = 73 || n = 75 || n = 77)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 41 || n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 42 || n = 62)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 55)
	; 5 bytes
	ldx #248 ;hides 'sed'
	inx
	Jne *-2
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
.if(n = 79 || n = 84 || n = 89 || n = 94 || n = 102 || n = 104 || n = 109 || n = 114 || n = 119 || n = 129 || n = 139 || n = 144 || n = 149 || n = 154)
	; 7..8 bytes
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
.elseif(n = 83 || n = 87 || n = 88 || n = 90 || n = 93 || n = 95 || n = 98 || n = 100 || n = 103 || n = 105 || n = 108 || n = 110 || n = 112 || n = 118 || n = 122 || n = 123 || n = 125 || n = 128 || n = 130 || n = 132 || n = 133 || n = 135 || n = 138 || n = 140 || n = 143 || n = 145 || n = 147)
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
.elseif(n = 107)
	; 6 bytes
	ldx #24 ;hides 'clc'
	dex
	dex
	Jne *-3
.elseif(n = 117)
	; 7 bytes
	bit <$2C ;hides 'bit $0CA2'
	ldx #12
	dex
	Jpl *-4
.elseif(n = 124)
	; 4 bytes
	ldx #232 ;hides 'inx'
	Jne *-1
.elseif(n = 137)
	; 7 bytes
	ldx #(n-9)/8
	bit $24
	dex
	Jpl *-3
.elseif(n = 142)
	; 7 bytes
	ldx #24 ;hides 'clc'
	nop
	dex
	dex
	Jpl *-4
.elseif(n = 152)
	; 7 bytes
	ldx #<$2C ;hides 'bit $90A2'
	ldx #144
	dex
	Jmi *-4
.elseif(n = 153)
	; 5 bytes
	ldx #234 ;hides 'nop'
	inx
	Jne *-2
.endif
.endif
.else
; 157 to 312
.if(n = 157 || n = 158 || n = 163 || n = 165 || n = 168 || n = 170 || n = 173 || n = 175 || n = 178 || n = 180 || n = 182 || n = 187 || n = 188 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 202 || n = 203 || n = 205 || n = 208 || n = 210 || n = 212 || n = 213 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233 || n = 235 || n = 237 || n = 238 || n = 240 || n = 243 || n = 245 || n = 247 || n = 248 || n = 250 || n = 258 || n = 263 || n = 268 || n = 270 || n = 273 || n = 275 || n = 278 || n = 282 || n = 283 || n = 285 || n = 292 || n = 293 || n = 297 || n = 298 || n = 300 || n = 303 || n = 305 || n = 308 || n = 310)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 177 || n = 184 || n = 189 || n = 194 || n = 199 || n = 209 || n = 214 || n = 219 || n = 229 || n = 234 || n = 244 || n = 249 || n = 254 || n = 259 || n = 264 || n = 269 || n = 284 || n = 289 || n = 294 || n = 312)
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
	ldx #216 ;hides 'cld'
	inx
	inx
	Jne *-3
.elseif(n = 185 || n = 220 || n = 255 || n = 290)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 207)
	; 7 bytes
	bit <$2C ;hides 'bit $E9A2'
	ldx #233
	inx
	Jne *-4
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
	; 7 bytes
	ldx #<$2C ;hides 'bit $1AA2'
	ldx #26
	dex
	Jpl *-4
.elseif(n = 252)
	; 7 bytes
	bit <$2C ;hides 'bit $9BA2'
	ldx #155
	dex
	Jmi *-4
.elseif(n = 257)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 265)
	; 5 bytes
	ldx #202 ;hides 'dex'
	dex
	Jmi *-2
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
.elseif(n = 287)
	; 7 bytes
	ldx #<$2C ;hides 'bit $9FA2'
	ldx #159
	dex
	Jmi *-4
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
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 315 || n = 317 || n = 318 || n = 320 || n = 322 || n = 327 || n = 328 || n = 332 || n = 333 || n = 335 || n = 338 || n = 340 || n = 343 || n = 345 || n = 348 || n = 350 || n = 353 || n = 355 || n = 357 || n = 362 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 380 || n = 383 || n = 385 || n = 388 || n = 390 || n = 392 || n = 402 || n = 403 || n = 408 || n = 410 || n = 412 || n = 413 || n = 415 || n = 418 || n = 420 || n = 423 || n = 425 || n = 427 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 447 || n = 448 || n = 450 || n = 453 || n = 455 || n = 458 || n = 460 || n = 462 || n = 468)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 314 || n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 347 || n = 349 || n = 354 || n = 364 || n = 369 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 409 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 452 || n = 454 || n = 459 || n = 464)
	; 6..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
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
.elseif(n = 325 || n = 360 || n = 395 || n = 430 || n = 465)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 337 || n = 358 || n = 372 || n = 393 || n = 449)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 342)
	; 7 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	inx
	Jne *-4
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
.elseif(n = 377)
	; 7 bytes
	ldx #(n-9)/8
	bit $24
	dex
	Jpl *-3
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 387)
	; 7 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	inx
	Jne *-4
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
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 417)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
.elseif(n = 422)
	; 7 bytes
	ldx #<$2C ;hides 'bit $AEA2'
	ldx #174
	dex
	Jmi *-4
.elseif(n = 432)
	; 7 bytes
	bit <$2C ;hides 'bit $AFA2'
	ldx #175
	dex
	Jmi *-4
.elseif(n = 457)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 467)
	; 6 bytes
	ldx #24 ;hides 'clc'
	inx
	inx
	Jpl *-3
.endif
.else
; 469 to 625
.if(n = 469 || n = 474 || n = 479 || n = 489 || n = 494 || n = 499 || n = 504 || n = 509 || n = 514 || n = 524 || n = 529 || n = 534 || n = 539 || n = 544 || n = 549 || n = 559 || n = 564 || n = 569 || n = 574 || n = 579 || n = 584 || n = 592 || n = 594 || n = 599 || n = 604 || n = 609 || n = 614 || n = 619)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 470 || n = 498 || n = 568 || n = 582 || n = 589 || n = 603 || n = 610 || n = 624)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 471 || n = 501 || n = 521 || n = 551 || n = 566 || n = 586 || n = 591 || n = 601 || n = 621)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 472 || n = 473 || n = 475 || n = 478 || n = 480 || n = 482 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 502 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 527 || n = 528 || n = 530 || n = 535 || n = 538 || n = 542 || n = 543 || n = 545 || n = 548 || n = 552 || n = 553 || n = 555 || n = 558 || n = 560 || n = 563 || n = 565 || n = 570 || n = 572 || n = 573 || n = 578 || n = 580 || n = 583 || n = 585 || n = 587 || n = 588 || n = 590 || n = 593 || n = 597 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623 || n = 625)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 475 && n < 497) || (n > 505 && n < 517) || (n > 525 && n < 547) || n = 556 || n = 561 || (n > 570 && n < 582) || n = 596 || (n > 605 && n < 617)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif((n .mod 7 = 1) && (n = 477 || n = 484 || (n > 504 && n < 520) || (n > 532 && n < 555) || n = 575 || n = 617))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 485)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 487 || n = 577)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 492 || n = 510)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 497)
	; 7 bytes
	ldx #(n-9)/8
	bit $24
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
.elseif(n = 517)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 522)
	; 7 bytes
	bit <$2C ;hides 'bit $B9A2'
	ldx #185
	dex
	Jmi *-4
.elseif(n = 532 || n = 550 || n = 595)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 537)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 557)
	; 7 bytes
	ldx #<$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jmi *-4
.elseif(n = 562)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 567)
	; 7 bytes
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	inx
	Jne *-4
.elseif(n = 602)
	; 7 bytes
	ldx #<$2C ;hides 'bit $C2A2'
	ldx #194
	dex
	Jmi *-4
.elseif(n = 607)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jmp *+3
	Jmi *-4
.elseif(n = 622)
	; 5 bytes
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
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
.elseif(n = 627 || n = 634 || n = 639 || n = 644 || n = 649 || n = 654 || n = 662 || n = 664 || n = 669 || n = 674 || n = 679 || n = 684 || n = 689 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 732 || n = 739 || n = 749 || n = 754 || n = 759 || n = 767 || n = 769 || n = 774 || n = 779)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 628 || n = 630 || n = 632 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 665 || n = 668 || n = 670 || n = 675 || n = 677 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 693 || n = 695 || n = 698 || n = 700 || n = 703 || n = 705 || n = 707 || n = 713 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 742 || n = 752 || n = 753 || n = 758 || n = 760 || n = 762 || n = 763 || n = 765 || n = 768 || n = 770 || n = 772 || n = 773)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 629 || n = 734 || n = 755)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
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
	bit $24
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
.elseif(n = 692)
	; 7 bytes
	ldx #<$2C ;hides 'bit $CCA2'
	ldx #204
	dex
	Jmi *-4
.elseif(n = 697)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 702)
	; 7 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	inx
	Jne *-4
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
.elseif(n = 747)
	; 7 bytes
	bit <$2C ;hides 'bit $ADA2'
	ldx #173
	inx
	Jne *-4
.elseif(n = 748)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
.endif
.else
; 782 to 937
.if(n = 782)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D6A2'
	ldx #214
	dex
	Jmi *-4
.elseif(n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 810 || n = 812 || n = 818 || n = 822 || n = 823 || n = 825 || n = 828 || n = 830 || n = 833 || n = 835 || n = 838 || n = 840 || n = 842 || n = 843 || n = 845 || n = 852 || n = 853 || n = 857 || n = 858 || n = 860 || n = 863 || n = 867 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 887 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 902 || n = 903 || n = 905 || n = 908 || n = 910 || n = 912 || n = 913 || n = 915 || n = 922 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 784 || n = 789 || n = 794 || n = 804 || n = 809 || n = 814 || n = 819 || n = 824 || n = 829 || n = 839 || n = 844 || n = 849 || n = 854 || n = 859 || n = 864 || n = 874 || n = 879 || n = 884 || n = 889 || n = 894 || n = 907 || n = 909 || n = 914 || n = 919 || n = 924 || n = 929 || n = 934)
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
	ldx #216 ;hides 'cld'
	nop
	dex
	Jmi *-3
.elseif(n = 802 || n = 847 || n = 937)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 807)
	; 8 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 837)
	; 7 bytes
	bit <$2C ;hides 'bit $A3A2'
	ldx #163
	inx
	Jne *-4
.elseif(n = 865 || n = 892)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 872)
	; 7 bytes
	ldx #<$2C ;hides 'bit $61A2'
	ldx #97
	dex
	Jne *-4
.elseif(n = 882)
	; 7 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jpl *-4
.elseif(n = 899)
	; 6 bytes
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
.elseif(n = 917)
	; 7 bytes
	ldx #<$2C ;hides 'bit $66A2'
	ldx #102
	dex
	Jne *-4
.elseif(n = 935)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
.endif
.endif
.else
; 938 to 1250
.if(n = 938 || n = 940 || n = 943 || n = 945 || n = 948 || n = 950 || n = 952 || n = 957 || n = 958 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 980 || n = 983 || n = 985 || n = 987 || n = 992 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1020 || n = 1022 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1042 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1063 || n = 1067 || n = 1068 || n = 1070 || n = 1073 || n = 1075 || n = 1077 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1087 || n = 1088 || n = 1090 || n = 1092 || n = 1098 || n = 1102 || n = 1103 || n = 1105 || n = 1108 || n = 1110 || n = 1112 || n = 1113 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1127 || n = 1132 || n = 1133 || n = 1137 || n = 1138 || n = 1140 || n = 1143 || n = 1145 || n = 1147 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1157 || n = 1158 || n = 1160 || n = 1167 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1182 || n = 1183 || n = 1185 || n = 1188 || n = 1190 || n = 1192 || n = 1193 || n = 1195 || n = 1202 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1225 || n = 1227 || n = 1228 || n = 1230 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248 || n = 1250)
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
.elseif(n = 942 || n = 1122)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1012 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1047 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1082 || n = 1084 || n = 1089 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1124 || n = 1129 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1209 || n = 1214 || n = 1222 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244 || n = 1249)
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
	bit $24
	dex
	Jpl *-3
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
.elseif(n = 1062)
	; 7 bytes
	bit <$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jpl *-4
.elseif(n = 1097 || n = 1217)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
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
.elseif(n = 1152)
	; 7 bytes
	bit <$2C ;hides 'bit $80A2'
	ldx #128
	dex
	Jne *-4
.elseif(n = 1187)
	; 7 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1197)
	; 7 bytes
	bit <$2C ;hides 'bit $85A2'
	ldx #133
	dex
	Jne *-4
.elseif(n = 1232)
	; 7 bytes
	ldx #<$2C ;hides 'bit $89A2'
	ldx #137
	dex
	Jne *-4
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
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1262 || n = 1263 || n = 1265 || n = 1267 || n = 1272 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1291 || n = 1293 || n = 1298 || n = 1300 || n = 1302 || n = 1305 || n = 1307 || n = 1308 || n = 1311 || n = 1312 || n = 1316 || n = 1319 || n = 1325 || n = 1326 || n = 1328)
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
	bit $24
	dex
	Jne *-3
.elseif(n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290 || n = 1292 || n = 1299 || n = 1318 || n = 1320 || n = 1327)
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
.elseif(n = 1295)
	; 7 bytes
	ldx #<$2C ;hides 'bit $90A2'
	ldx #144
	dex
	Jne *-4
.elseif(n = 1301)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
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
.elseif(n = 1321)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1337 || n = 1353)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
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
.elseif(n = 1334 || n = 1348 || n = 1355 || n = 1362)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1335)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1336)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1339 || n = 1340 || n = 1344 || n = 1346 || n = 1347 || n = 1354 || n = 1356 || n = 1361)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1341)
	; 7 bytes
	bit <$2C ;hides 'bit $95A2'
	ldx #149
	dex
	Jne *-4
.elseif(n = 1343)
	; 8 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-4
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
.elseif(n = 1364)
	; 8 bytes
	bit <$2C ;hides 'bit $04A2'
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
.elseif(n = 1367)
	; 7 bytes
	ldx #<$2C ;hides 'bit $98A2'
	ldx #152
	dex
	Jne *-4
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1370 || n = 1371 || n = 1375 || n = 1382 || n = 1384 || n = 1388 || n = 1389 || n = 1391 || n = 1403)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1369 || n = 1378 || n = 1396 || n = 1405)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1372 || n = 1379 || n = 1390 || n = 1397 || n = 1402)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 7 = 1) && ((n > 1372 && n < 1402)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1374)
	; 8 bytes
	pha
	ldx #152 ;hides 'tya'
	nop
	dex
	Jne *-3
	pla
.elseif(n = 1376)
	; 6 bytes
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
.elseif(n = 1377 || n = 1385)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1381 || n = 1393)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1383 || n = 1392 || n = 1406)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1386)
	; 7 bytes
	bit <$2C ;hides 'bit $9AA2'
	ldx #154
	dex
	Jne *-4
.elseif(n = 1395)
	; 7 bytes
	bit <$2C ;hides 'bit $9BA2'
	ldx #155
	dex
	Jne *-4
.elseif(n = 1398)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.elseif(n = 1400)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1404)
	; 7 bytes
	bit <$2C ;hides 'bit $9CA2'
	ldx #156
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1410 || n = 1416 || n = 1417 || n = 1419 || n = 1424 || n = 1426 || n = 1428 || n = 1431 || n = 1434 || n = 1435 || n = 1437 || n = 1438 || n = 1445)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1444)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1409 || n = 1425 || n = 1433)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1411 || n = 1418 || n = 1444)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442)))
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
.elseif(n = 1421)
	; 7 bytes
	ldx #<$2C ;hides 'bit $9EA2'
	ldx #158
	dex
	Jne *-4
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.endif
.else
; 1446 to 1484
.if(n = 1446 || n = 1460 || n = 1474)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1447 || n = 1452 || n = 1454 || n = 1466 || n = 1469 || n = 1470 || n = 1473 || n = 1479 || n = 1480 || n = 1482 || n = 1483)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1448)
	; 7 bytes
	ldx #<$2C ;hides 'bit $A1A2'
	ldx #161
	dex
	Jne *-4
.elseif(n = 1449 || n = 1481)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1449 && n < 1479)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1451)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
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
.elseif(n = 1456)
	; 9 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1461)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1462)
	; 8 bytes
	ldx #<$2C ;hides 'bit $7BA2'
	ldx #123
	nop
	inx
	Jne *-5
.elseif(n = 1463)
	; 8 bytes
	bit <$2C ;hides 'bit $85A2'
	ldx #133
	nop
	dex
	Jne *-5
.elseif(n = 1467)
	; 7 bytes
	bit <$2C ;hides 'bit $A3A2'
	ldx #163
	dex
	Jne *-4
.elseif(n = 1472)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
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
; 1485 to 1562
.if((n .mod 7 = 1) && ((n > 1484 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1485 && n < 1505) || (n > 1521 && n < 1559)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1487 || n = 1490 || n = 1491 || n = 1494 || n = 1496 || n = 1501 || n = 1508 || n = 1510 || n = 1515 || n = 1517 || n = 1526 || n = 1529 || n = 1532 || n = 1533 || n = 1535 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1552 || n = 1557 || n = 1559 || n = 1560)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488 || n = 1498 || n = 1500 || n = 1509 || n = 1514 || n = 1516 || n = 1523 || n = 1544 || n = 1551)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1489 || n = 1525 || n = 1537 || n = 1561)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1493)
	; 7 bytes
	ldx #<$2C ;hides 'bit $A6A2'
	ldx #166
	dex
	Jne *-4
.elseif(n = 1497)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jmi *-4
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
.elseif(n = 1505 || n = 1521 || n = 1545 || n = 1553)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
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
	; 5 bytes
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
.elseif(n = 1512)
	; 7 bytes
	bit <$2C ;hides 'bit $A8A2'
	ldx #168
	dex
	Jne *-4
.elseif(n = 1518)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1519)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1524)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1528)
	; 8 bytes
	ldx #<$2C ;hides 'bit $75A2'
	ldx #117
	nop
	inx
	Jne *-5
.elseif(n = 1530)
	; 7 bytes
	bit <$2C ;hides 'bit $AAA2'
	ldx #170
	dex
	Jne *-4
.elseif(n = 1538)
	; 7 bytes
	ldx #<$2C ;hides 'bit $ABA2'
	ldx #171
	dex
	Jne *-4
.elseif(n = 1539)
	; 7 bytes
	bit <$2C ;hides 'bit $ABA2'
	ldx #171
	dex
	Jne *-4
.elseif(n = 1546)
	; 9 bytes
	ldx #102
	rol $26
	ror $26
	dex
	Jpl *-5
.elseif(n = 1547)
	; 7 bytes
	ldx #<$2C ;hides 'bit $ACA2'
	ldx #172
	dex
	Jne *-4
.elseif(n = 1554)
	; 8 bytes
	pha
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 1556)
	; 7 bytes
	ldx #<$2C ;hides 'bit $ADA2'
	ldx #173
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563 || n = 1596)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1564 || n = 1568 || n = 1571 || n = 1578 || n = 1580 || n = 1588 || n = 1589 || n = 1592 || n = 1595 || n = 1599)
	; 7..9 bytes
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
.elseif(n = 1567 || n = 1585 || n = 1594)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1598)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1570 || n = 1572 || n = 1579 || n = 1586 || n = 1600)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1577 || n = 1593 || n = 1601)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1581)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1582)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 1584)
	; 7 bytes
	bit <$2C ;hides 'bit $B0A2'
	ldx #176
	dex
	Jne *-4
.elseif(n = 1587 || n = 1598)
	; 8..9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1591)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 7 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	dex
	Jne *-4
.elseif(n = 1603 || n = 1612 || n = 1621)
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
.elseif(n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1615 || n = 1620 || n = 1622 || n = 1627 || n = 1634 || n = 1636)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1607 || n = 1614 || n = 1626 || n = 1635 || n = 1640)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1609 || n = 1633)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1610)
	; 6 bytes
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
.elseif(n = 1616)
	; 8 bytes
	ldx #<$2C ;hides 'bit $93A2'
	ldx #147
	nop
	dex
	Jne *-5
.elseif(n = 1617)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1619)
	; 7 bytes
	ldx #<$2C ;hides 'bit $B4A2'
	ldx #180
	dex
	Jne *-4
.elseif(n = 1623)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
.elseif(n = 1624 || n = 1630)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1628)
	; 7 bytes
	ldx #<$2C ;hides 'bit $B5A2'
	ldx #181
	dex
	Jne *-4
.elseif(n = 1629)
	; 7 bytes
	bit <$2C ;hides 'bit $B5A2'
	ldx #181
	dex
	Jne *-4
.elseif(n = 1631)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
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
; 1641 to 1718
.if(n = 1641 || n = 1643 || n = 1659 || n = 1662 || n = 1668 || n = 1669 || n = 1671 || n = 1676 || n = 1678 || n = 1679 || n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1697 || n = 1699 || n = 1703 || n = 1704 || (n > 1705 && n < 1709) || n = 1714 || n = 1715 || n = 1718)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1642 || n = 1658 || n = 1663 || n = 1670 || n = 1677 || n = 1687 || n = 1698 || n = 1712)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1644 || n = 1652 || n = 1680 || n = 1696)
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
.elseif(n = 1647)
	; 7 bytes
	bit <$2C ;hides 'bit $B7A2'
	ldx #183
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1647 && n < 1694) || n = 1711))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1649 || n = 1665 || n = 1673 || n = 1689 || n = 1713)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
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
.elseif(n = 1661)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1664)
	; 7 bytes
	ldx #<$2C ;hides 'bit $B9A2'
	ldx #185
	dex
	Jne *-4
.elseif(n = 1672)
	; 8 bytes
	bit <$2C ;hides 'bit $98A2'
	ldx #152
	nop
	dex
	Jne *-5
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1691)
	; 7 bytes
	ldx #<$2C ;hides 'bit $BCA2'
	ldx #188
	dex
	Jne *-4
.elseif(n = 1692)
	; 7 bytes
	bit <$2C ;hides 'bit $BCA2'
	ldx #188
	dex
	Jne *-4
.elseif(n = 1694)
	; 8 bytes
	bit <$2C ;hides 'bit $9AA2'
	ldx #154
	nop
	dex
	Jne *-5
.elseif(n = 1700)
	; 7 bytes
	ldx #<$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jne *-4
.elseif(n = 1701)
	; 7 bytes
	bit <$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jne *-4
.elseif(n = 1710)
	; 7 bytes
	bit <$2C ;hides 'bit $BEA2'
	ldx #190
	dex
	Jne *-4
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
.elseif((n .mod 9 = 1) && ((n > 1719 && n < 1757)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1721 || n = 1745)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1722 || n = 1725 || n = 1731 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1748 || n = 1749 || n = 1753 || n = 1754 || n = 1757)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1752)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724 || n = 1742)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1726 || n = 1733 || n = 1740)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1741)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1750)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
.endif
.else
; 1758 to 1796
.if((n .mod 7 = 1) && ((n > 1757 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1759 || n = 1775)
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
.elseif(n = 1761 || n = 1769 || n = 1785)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1762 || n = 1766 || n = 1767 || n = 1770 || n = 1778 || n = 1781 || n = 1784 || n = 1787 || n = 1788 || n = 1795)
	; 7..9 bytes
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
.elseif(n = 1768)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1771)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
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
.elseif(n = 1776)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1777 || n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1780 || n = 1796)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1782)
	; 7 bytes
	bit <$2C ;hides 'bit $C6A2'
	ldx #198
	dex
	Jne *-4
.elseif(n = 1790)
	; 7 bytes
	ldx #<$2C ;hides 'bit $C7A2'
	ldx #199
	dex
	Jne *-4
.elseif(n = 1791)
	; 7 bytes
	bit <$2C ;hides 'bit $C7A2'
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797 || n = 1798 || n = 1803 || n = 1812 || n = 1815 || n = 1820 || n = 1821 || n = 1830)
	; 8..9 bytes
	delay_n (n-2)
	nop
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
.elseif((n .mod 9 = 1) && ((n > 1800 && n < 1829)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1802 || n = 1822 || n = 1829)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1804)
	; 8 bytes
	bit <$2C ;hides 'bit $A4A2'
	ldx #164
	nop
	dex
	Jne *-5
.elseif(n = 1805 || n = 1816)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 1809 || n = 1817 || n = 1833)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1811)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1813 || n = 1825)
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
.elseif(n = 1831)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1832)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1834)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 1835)
	; 7 bytes
	ldx #<$2C ;hides 'bit $CCA2'
	ldx #204
	dex
	Jne *-4
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 7 bytes
	bit <$2C ;hides 'bit $CCA2'
	ldx #204
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1874)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1838 || n = 1860)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1839)
	; 7 bytes
	ldx #184 ;hides 'clv'
	bit $24
	dex
	Jne *-4
.elseif(n = 1840 || n = 1868)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1841 || n = 1857 || n = 1865)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
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
.elseif(n = 1843 || n = 1847 || n = 1848 || n = 1850 || n = 1851 || n = 1856 || n = 1859 || n = 1866 || n = 1867 || n = 1874 || n = 1875)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1844)
	; 7 bytes
	ldx #<$2C ;hides 'bit $CDA2'
	ldx #205
	dex
	Jne *-4
.elseif(n = 1845)
	; 7 bytes
	bit <$2C ;hides 'bit $CDA2'
	ldx #205
	dex
	Jne *-4
.elseif(n = 1849 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1852)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
	; 8 bytes
	bit <$2C ;hides 'bit $AAA2'
	ldx #170
	nop
	dex
	Jne *-5
.elseif(n = 1871)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D0A2'
	ldx #208
	dex
	Jne *-4
.elseif(n = 1872)
	; 7 bytes
	bit <$2C ;hides 'bit $D0A2'
	ldx #208
	dex
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
.if(n = 1876 || n = 1906)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 1877)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1878 || n = 1896 || n = 1912)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1879 || n = 1883 || n = 1884 || n = 1886 || n = 1887 || n = 1902 || n = 1910 || n = 1911)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1880)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D1A2'
	ldx #209
	dex
	Jne *-4
.elseif(n = 1881 || n = 1889 || n = 1905 || n = 1913)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1910)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1885 || n = 1897)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 1892)
	; 8 bytes
	bit <$2C ;hides 'bit $ACA2'
	ldx #172
	nop
	dex
	Jne *-5
.elseif(n = 1893 || n = 1904)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1894)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1895)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1898)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D3A2'
	ldx #211
	dex
	Jne *-4
.elseif(n = 1899)
	; 7 bytes
	bit <$2C ;hides 'bit $D3A2'
	ldx #211
	dex
	Jne *-4
.elseif(n = 1901)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1903)
	; 8 bytes
	bit <$2C ;hides 'bit $ADA2'
	ldx #173
	nop
	dex
	Jne *-5
.elseif(n = 1907)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D4A2'
	ldx #212
	dex
	Jne *-4
.elseif(n = 1908)
	; 7 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1916)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D5A2'
	ldx #213
	dex
	Jne *-4
.elseif(n = 1917)
	; 7 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1917 && n < 1946)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1919)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1920 || n = 1928 || n = 1938 || n = 1939 || n = 1947 || n = 1949)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1921 || n = 1933)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1922)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 1923)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1924)
	; 8 bytes
	ldx #<$2C ;hides 'bit $AFA2'
	ldx #175
	nop
	dex
	Jne *-5
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
.elseif(n = 1929 || n = 1937 || n = 1953)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1930 || n = 1946)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1931 || n = 1941 || n = 1951)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1932)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
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
.elseif(n = 1940 || n = 1950)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1942)
	; 9 bytes
	pha
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
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
.elseif(n = 1955 || n = 1956 || n = 1964 || n = 1965 || n = 1973 || n = 1974 || n = 1982 || n = 1983 || n = 1987)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1957 || n = 1969)
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1960 || n = 1978 || n = 1984)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1961 || n = 1977 || n = 1985)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1962)
	; 7 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	dex
	Jne *-4
.elseif(n = 1966)
	; 9 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 1967)
	; 9 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1968)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 1970)
	; 7 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	dex
	Jne *-4
.elseif(n = 1971)
	; 7 bytes
	bit <$2C ;hides 'bit $DBA2'
	ldx #219
	dex
	Jne *-4
.elseif(n = 1975)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1976)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1979)
	; 7 bytes
	ldx #<$2C ;hides 'bit $DCA2'
	ldx #220
	dex
	Jne *-4
.elseif(n = 1980)
	; 7 bytes
	bit <$2C ;hides 'bit $DCA2'
	ldx #220
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
.elseif(n = 1991)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
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
.elseif(n = 1994)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1995 || n = 2000 || n = 2010 || n = 2018 || n = 2019 || n = 2027 || n = 2028)
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
.elseif((n .mod 9 = 1) && ((n > 1998 && n < 2027)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2001 || n = 2009 || n = 2025)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 2002)
	; 8 bytes
	bit <$2C ;hides 'bit $B6A2'
	ldx #182
	nop
	dex
	Jne *-5
.elseif(n = 2003 || n = 2014)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2004 || n = 2020)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 2011 || n = 2021)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2012)
	; 8 bytes
	ldx #<$2C ;hides 'bit $B7A2'
	ldx #183
	nop
	dex
	Jne *-5
.elseif(n = 2013)
	; 8 bytes
	bit <$2C ;hides 'bit $B7A2'
	ldx #183
	nop
	dex
	Jne *-5
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
.elseif(n = 2022 || n = 2030)
	; 9 bytes
	php
	delay_n (n-7)
	plp
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
.if(n = 2032)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
.elseif(n = 2033)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2037 || n = 2039 || n = 2046 || n = 2054 || n = 2055 || n = 2063 || n = 2064 || n = 2066 || n = 2067)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2038)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2040 || n = 2050)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2041 || n = 2065)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 2048)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2049)
	; 7 bytes
	ldx #0
	bit $24
	dex
	Jne *-3
.elseif(n = 2051)
	; 7 bytes
	ldx #<$2C ;hides 'bit $E4A2'
	ldx #228
	dex
	Jne *-4
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
	; 8 bytes
	bit <$2C ;hides 'bit $BBA2'
	ldx #187
	nop
	dex
	Jne *-5
.elseif(n = 2060)
	; 7 bytes
	ldx #<$2C ;hides 'bit $E5A2'
	ldx #229
	dex
	Jne *-4
.elseif(n = 2061)
	; 7 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	inx
	Jne *-4
.elseif(n = 2068)
	; 8 bytes
	bit <$2C ;hides 'bit $BCA2'
	ldx #188
	nop
	dex
	Jne *-5
.elseif(n = 2069)
	; 7 bytes
	ldx #<$2C ;hides 'bit $E6A2'
	ldx #230
	dex
	Jne *-4
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
.elseif(n = 2072 || n = 2073 || n = 2082 || n = 2099 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074 || n = 2081 || n = 2083 || n = 2090 || n = 2108)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2075 || n = 2092)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 2076 || n = 2084 || n = 2085 || n = 2094)
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
.elseif(n = 2079)
	; 7 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
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
	; 7 bytes
	bit <$2C ;hides 'bit $E8A2'
	ldx #232
	dex
	Jne *-4
.elseif(n = 2091 || n = 2102)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2093)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2095)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	inx
	Jne *-4
	plp
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
.if(n = 2110 || n = 2119 || n = 2128 || n = 2140)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2111 || n = 2131)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2112 || n = 2120)
	; 8..9 bytes
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
	; 7 bytes
	ldx #<$2C ;hides 'bit $EBA2'
	ldx #235
	dex
	Jne *-4
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
.elseif(n = 2117 || n = 2118 || n = 2126 || n = 2127 || n = 2136 || n = 2138 || n = 2139 || n = 2144 || n = 2145 || n = 2148)
	; 8..9 bytes
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
.elseif(n = 2122)
	; 8 bytes
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	nop
	dex
	Jne *-5
.elseif(n = 2123)
	; 7 bytes
	ldx #<$2C ;hides 'bit $ECA2'
	ldx #236
	dex
	Jne *-4
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
	; 7 bytes
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	inx
	Jne *-4
.elseif(n = 2133)
	; 7 bytes
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	dex
	Jne *-4
.elseif(n = 2135 || n = 2146)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2141)
	; 7 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	dex
	Jne *-4
.elseif(n = 2142)
	; 7 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	inx
	Jne *-4
.elseif(n = 2147)
	; 8 bytes
	ldx #232 ;hides 'inx'
	bit $24
	inx
	inx
	Jne *-5
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
.elseif(n = 2151)
	; 7 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2151 && n < 2180)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2153 || n = 2154 || n = 2162 || n = 2163 || n = 2165 || n = 2172 || n = 2174 || n = 2175 || n = 2180 || n = 2181 || n = 2183)
	; 8..9 bytes
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2158 || n = 2184)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2159)
	; 7 bytes
	ldx #216 ;hides 'cld'
	bit $24
	dex
	Jne *-4
.elseif(n = 2160)
	; 7 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	dex
	Jne *-4
.elseif(n = 2164 || n = 2182)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 2171)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2176)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
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
.elseif(n = 2186)
	; 7 bytes
	ldx #<$2C ;hides 'bit $F3A2'
	ldx #243
	dex
	Jne *-4
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
.elseif(n = 2189 || n = 2192 || n = 2198 || n = 2202 || n = 2203 || n = 2208 || n = 2211 || n = 2216 || n = 2217 || n = 2219 || n = 2225 || n = 2226)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2190)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2191)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2193)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2194)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 2195)
	; 7 bytes
	ldx #<$2C ;hides 'bit $0CA2'
	ldx #12
	inx
	Jne *-4
.elseif(n = 2196)
	; 7 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	dex
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
.elseif(n = 2201 || n = 2212)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2204)
	; 7 bytes
	ldx #<$2C ;hides 'bit $F5A2'
	ldx #245
	dex
	Jne *-4
.elseif(n = 2205)
	; 7 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	dex
	Jne *-4
.elseif(n = 2207)
	; 8 bytes
	ldx #184 ;hides 'clv'
	nop
	bit $24
	dex
	Jne *-5
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
.elseif(n = 2218)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2220)
	; 9 bytes
	php
	delay_n (n-7)
	plp
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
; 2227 to 2265
.if(n = 2227 || n = 2255)
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
.elseif(n = 2229 || n = 2235 || n = 2237 || n = 2243 || n = 2244 || n = 2246 || n = 2247 || n = 2252 || n = 2253 || n = 2262)
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
.elseif(n = 2238 || n = 2239 || n = 2248 || n = 2264)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 2245 || n = 2257)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2249)
	; 7 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2250)
	; 7 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2254)
	; 8 bytes
	ldx #<$2C ;hides 'bit $CDA2'
	ldx #205
	nop
	dex
	Jne *-5
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
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2263)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2265)
	; 8 bytes
	ldx #<$2C ;hides 'bit $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
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
.elseif(n = 2270 || n = 2271 || n = 2273 || n = 2279 || n = 2280 || n = 2282 || n = 2288 || n = 2298)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2272 || n = 2290)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2274 || n = 2275 || n = 2284 || n = 2292 || n = 2302)
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2291 || n = 2301)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
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
	; 8 bytes
	bit <$2C ;hides 'bit $D1A2'
	ldx #209
	nop
	dex
	Jne *-5
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
; 2305 to 2343
.if(n = 2305)
	; 7 bytes
	ldx #0
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2306 || n = 2307 || n = 2323 || n = 2327 || n = 2331 || n = 2334 || n = 2335 || n = 2343)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 2311 || n = 2322 || n = 2333)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2312 || n = 2318 || n = 2324 || n = 2336)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2313 || n = 2330)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2314)
	; 9 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2315 || n = 2328)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2316)
	; 9 bytes
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	bit $24
	dex
	Jne *-6
.elseif(n = 2317 || n = 2329 || n = 2341)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2319)
	; 7 bytes
	ldx #24 ;hides 'clc'
	bit $24
	inx
	Jne *-4
.elseif(n = 2320)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D3A2'
	ldx #211
	nop
	dex
	Jne *-5
.elseif(n = 2321)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
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
.elseif(n = 2332)
	; 8 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
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
	; 7 bytes
	ldx #234 ;hides 'nop'
	bit $24
	dex
	Jne *-4
.elseif(n = 2340)
	; 9 bytes
	bit <$2C ;hides 'bit $C3A2'
	ldx #195
	bit $24
	dex
	Jne *-6
.elseif(n = 2342)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D5A2'
	ldx #213
	nop
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2355 || n = 2366)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2345 || n = 2350 || n = 2357 || n = 2363 || n = 2368 || n = 2373 || n = 2379)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2346 || n = 2348 || n = 2360 || n = 2362 || n = 2372 || n = 2374 || n = 2382)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2347)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2349)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2351 || n = 2361 || n = 2371)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2352)
	; 9 bytes
	bit <$2C ;hides 'bit $C4A2'
	ldx #196
	bit $24
	dex
	Jne *-6
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
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2358)
	; 10 bytes
	php
	ldx #235
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2359)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2364)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	nop
	dex
	Jne *-5
.elseif(n = 2367 || n = 2381)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2369)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2370)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
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
.elseif(n = 2378)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2380)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2390 || n = 2391 || n = 2394 || n = 2403 || n = 2405 || n = 2407 || n = 2412 || n = 2415 || n = 2417)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2396 || n = 2402)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
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
.elseif((n .mod 11 = 1) && ((n > 2387 && n < 2422)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2389 || n = 2401 || n = 2413)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2392 || n = 2404)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2393 || n = 2406)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2395 || n = 2409)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2397)
	; 8 bytes
	ldx #<$2C ;hides 'bit $DAA2'
	ldx #218
	nop
	dex
	Jne *-5
.elseif(n = 2398)
	; 8 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	nop
	dex
	Jne *-5
.elseif(n = 2400)
	; 9 bytes
	bit <$2C ;hides 'bit $C8A2'
	ldx #200
	bit $24
	dex
	Jne *-6
.elseif(n = 2408)
	; 8 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	nop
	dex
	Jne *-5
.elseif(n = 2411)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2414)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2416)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2418)
	; 9 bytes
	bit <$2C ;hides 'bit $BAA2'
	ldx #186
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2419)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	inx
	Jne *-5
.elseif(n = 2420)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2427 || n = 2429 || n = 2434 || n = 2439 || n = 2455)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423 || n = 2451)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2424)
	; 9 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	bit $24
	dex
	Jne *-6
.elseif(n = 2425 || n = 2437 || n = 2449)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2426 || n = 2428 || n = 2438 || n = 2440)
	; 8..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2430)
	; 8 bytes
	ldx #<$2C ;hides 'bit $DDA2'
	ldx #221
	nop
	dex
	Jne *-5
.elseif(n = 2431 || n = 2441)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2432 || n = 2443 || n = 2454)
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
.elseif(n = 2436)
	; 9 bytes
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	bit $24
	dex
	Jne *-6
.elseif(n = 2442 || n = 2444 || n = 2450 || n = 2456)
	; 8..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2445 || n = 2458)
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
.elseif(n = 2448)
	; 9 bytes
	bit <$2C ;hides 'bit $CCA2'
	ldx #204
	bit $24
	dex
	Jne *-6
.elseif(n = 2452)
	; 8 bytes
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
.elseif(n = 2453)
	; 8 bytes
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
.elseif(n = 2457)
	; 9 bytes
	bit <$2C ;hides 'bit $BDA2'
	ldx #189
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2459)
	; 9 bytes
	ldx #<$2C ;hides 'bit $CDA2'
	ldx #205
	bit $24
	dex
	Jne *-6
.elseif(n = 2460)
	; 9 bytes
	bit <$2C ;hides 'bit $CDA2'
	ldx #205
	bit $24
	dex
	Jne *-6
.endif
.else
; 2461 to 2500
.if((n .mod 12 = 1) && ((n > 2460 && n < 2498)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2462 || n = 2463 || n = 2467 || n = 2475 || n = 2477 || n = 2478 || n = 2489 || n = 2495 || n = 2499)
	; 8..10 bytes
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2466 || n = 2483 || n = 2500)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2468 || n = 2480 || n = 2492 || n = 2494)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2469)
	; 9 bytes
	ldx #<$2C ;hides 'bit $BEA2'
	ldx #190
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2470)
	; 9 bytes
	bit <$2C ;hides 'bit $BEA2'
	ldx #190
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2471 || n = 2481 || n = 2491)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2472)
	; 9 bytes
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	bit $24
	dex
	Jne *-6
.elseif(n = 2474)
	; 8 bytes
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	nop
	inx
	Jne *-5
.elseif(n = 2479)
	; 7 bytes
	ldx #248 ;hides 'sed'
	bit $24
	dex
	Jne *-4
.elseif(n = 2482 || n = 2488)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2484)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2486)
	; 8 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
.elseif(n = 2490)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2496)
	; 8 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	inx
	Jne *-5
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
.if(n = 2501)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2502 || n = 2503 || n = 2511 || n = 2513 || n = 2522 || n = 2523 || n = 2525 || n = 2539)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2514 || n = 2516 || n = 2528 || n = 2538)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2505)
	; 9 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
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
	; 8 bytes
	ldx #<$2C ;hides 'bit $E6A2'
	ldx #230
	nop
	dex
	Jne *-5
.elseif(n = 2530)
	; 8 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
.elseif(n = 2532)
	; 9 bytes
	bit <$2C ;hides 'bit $D3A2'
	ldx #211
	bit $24
	dex
	Jne *-6
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
	bit $24
	dex
	Jne *-4
.elseif(n = 2542 || n = 2553 || n = 2564)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2543 || n = 2548 || n = 2554 || n = 2560 || n = 2572 || n = 2578)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2544 || n = 2546 || n = 2547 || n = 2555 || n = 2559 || n = 2565 || n = 2570 || n = 2571)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2545 || n = 2557 || n = 2569)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2549 || n = 2563 || n = 2577)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2550)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	cmp $C5
	dex
	Jne *-5
	plp
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
.elseif(n = 2556)
	; 9 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	bit $24
	dex
	Jne *-6
.elseif(n = 2558 || n = 2576)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2561)
	; 8 bytes
	ldx #0
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2562)
	; 8 bytes
	ldx #<$2C ;hides 'bit $E9A2'
	ldx #233
	nop
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
	; 9 bytes
	ldx #<$2C ;hides 'bit $D6A2'
	ldx #214
	bit $24
	dex
	Jne *-6
.elseif(n = 2568)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579 || n = 2583 || n = 2587 || n = 2595 || n = 2598 || n = 2607 || n = 2609 || n = 2610)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2580 || n = 2582 || n = 2588 || n = 2600 || n = 2612)
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
.elseif(n = 2584)
	; 8 bytes
	ldx #<$2C ;hides 'bit $EBA2'
	ldx #235
	nop
	dex
	Jne *-5
.elseif(n = 2585)
	; 8 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	nop
	inx
	Jne *-5
.elseif(n = 2586 || n = 2597 || n = 2608)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2589 || n = 2594)
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
.elseif(n = 2592)
	; 9 bytes
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	bit $24
	dex
	Jne *-6
.elseif(n = 2596)
	; 8 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	nop
	dex
	Jne *-5
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
.elseif(n = 2602)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2603)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	bit $24
	dex
	Jne *-6
.elseif(n = 2604)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2606)
	; 8 bytes
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	nop
	inx
	Jne *-5
.elseif(n = 2611)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2613)
	; 9 bytes
	bit <$2C ;hides 'bit $C9A2'
	ldx #201
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2615)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DAA2'
	ldx #218
	bit $24
	dex
	Jne *-6
.elseif(n = 2616)
	; 9 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	bit $24
	dex
	Jne *-6
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2620 || n = 2622 || n = 2644)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2621 || n = 2631 || n = 2632 || n = 2634 || n = 2635 || n = 2643 || n = 2645 || n = 2649 || n = 2654 || n = 2655)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2623 || n = 2642)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2624 || n = 2637 || n = 2646 || n = 2648)
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
.elseif(n = 2627)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
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
.elseif(n = 2633 || n = 2647)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2636)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2638)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
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
	delay_n 6
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
.if(n = 2657 || n = 2658 || n = 2667 || n = 2669 || n = 2676 || n = 2679 || n = 2681 || n = 2691 || n = 2693)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2659 || n = 2660 || n = 2682)
	; 9..10 bytes
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2664)
	; 9 bytes
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	bit $24
	dex
	Jne *-6
.elseif(n = 2665 || n = 2677 || n = 2689)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2666 || n = 2692)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2668 || n = 2680)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2670 || n = 2687)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2671 || n = 2686)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2672)
	; 8 bytes
	ldx #<$2C ;hides 'bit $F3A2'
	ldx #243
	nop
	dex
	Jne *-5
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
.elseif(n = 2683)
	; 8 bytes
	ldx #<$2C ;hides 'bit $0CA2'
	ldx #12
	nop
	inx
	Jne *-5
.elseif(n = 2684)
	; 8 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	nop
	dex
	Jne *-5
.elseif(n = 2688)
	; 9 bytes
	bit <$2C ;hides 'bit $E0A2'
	ldx #224
	bit $24
	dex
	Jne *-6
.elseif(n = 2690)
	; 9 bytes
	ldx #<$2C ;hides 'bit $CFA2'
	ldx #207
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2694)
	; 8 bytes
	ldx #<$2C ;hides 'bit $F5A2'
	ldx #245
	nop
	dex
	Jne *-5
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2697 || n = 2698 || n = 2709 || n = 2715 || n = 2719 || n = 2722 || n = 2733)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2699)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2700)
	; 9 bytes
	bit <$2C ;hides 'bit $E1A2'
	ldx #225
	bit $24
	dex
	Jne *-6
.elseif(n = 2701 || n = 2713 || n = 2725)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2702)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	nop
	inx
	Jne *-5
	plp
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
.elseif(n = 2708 || n = 2710 || n = 2714 || n = 2720 || n = 2732 || n = 2734)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2711)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	bit $24
	inx
	Jne *-6
.elseif(n = 2712)
	; 9 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	bit $24
	inx
	Jne *-6
.elseif(n = 2716)
	; 8 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	nop
	inx
	Jne *-5
.elseif(n = 2723)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	bit $24
	inx
	Jne *-6
.elseif(n = 2724)
	; 9 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	bit $24
	inx
	Jne *-6
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
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
	; 9 bytes
	ldx #<$2C ;hides 'bit $E4A2'
	ldx #228
	bit $24
	dex
	Jne *-6
.elseif(n = 2736)
	; 9 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	bit $24
	dex
	Jne *-6
.elseif((n .mod 12 = 1) && ((n > 2736 && n < 2774)))
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
.elseif(n = 2739 || n = 2741 || n = 2742 || n = 2747 || n = 2753 || (n > 2762 && n < 2766))
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2740 || n = 2751 || n = 2762)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2743)
	; 9 bytes
	bit <$2C ;hides 'bit $D3A2'
	ldx #211
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2744 || n = 2756 || n = 2758 || n = 2766 || n = 2768)
	; 9..10 bytes
	php
	delay_n (n-7)
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
.elseif(n = 2748)
	; 9 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	bit $24
	inx
	Jne *-6
.elseif(n = 2750)
	; 8 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	inx
	Jne *-5
.elseif(n = 2752 || n = 2754)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2755)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2757 || n = 2770)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2760)
	; 8 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	inx
	Jne *-5
.elseif(n = 2767)
	; 9 bytes
	ldx #<$2C ;hides 'bit $ADA2'
	ldx #173
	delay_n 7
	dex
	Jne *-6
.elseif(n = 2769)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2771)
	; 8 bytes
	ldx #232 ;hides 'inx'
	bit $24
	dex
	dex
	Jne *-5
.elseif(n = 2772)
	; 8 bytes
	bit <$2C ;hides 'bit $04A2'
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
.elseif((n > 2777 && n < 2781) || n = 2792)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2781)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D6A2'
	ldx #214
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2782)
	; 8 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	inx
	Jne *-5
.elseif(n = 2783)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	bit $24
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
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2790)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	cmp $C5
	inx
	Jne *-5
	plp
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
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	bit $24
	dex
	Jne *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2818 || n = 2819 || n = 2823 || (n > 2824 && n < 2828) || n = 2830 || n = 2831 || n = 2835 || (n > 2836 && n < 2840) || n = 2842 || n = 2847 || n = 2849)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2814)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2815 || n = 2829 || n = 2843)
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
	; 8 bytes
	ldx #0
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2820)
	; 9 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	bit $24
	inx
	Jne *-6
.elseif(n = 2821 || n = 2833 || n = 2845)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2822)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2824 || n = 2844)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2828 || n = 2848 || n = 2850)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2832)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2834)
	; 9 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2836 || n = 2851)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
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
.elseif(n = 2846)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	delay_n 4
	dex
	Jne *-6
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2864 || n = 2876 || n = 2878 || n = 2888)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2853 || n = 2854 || n = 2858 || n = 2859 || n = 2862 || n = 2875 || n = 2882 || n = 2883 || n = 2890)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2855)
	; 9 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	bit $24
	dex
	Jne *-6
.elseif(n = 2856)
	; 9 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	bit $24
	inx
	Jne *-6
.elseif(n = 2857 || n = 2869 || n = 2881)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2860 || n = 2872 || n = 2877 || n = 2884)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2861 || n = 2887)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
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
.elseif(n = 2866)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2867)
	; 9 bytes
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	bit $24
	inx
	Jne *-6
.elseif(n = 2868)
	; 9 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	bit $24
	dex
	Jne *-6
.elseif(n = 2870 || n = 2889)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2871 || n = 2885)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2873)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2874)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2879)
	; 9 bytes
	ldx #<$2C ;hides 'bit $F0A2'
	ldx #240
	bit $24
	dex
	Jne *-6
.elseif(n = 2880)
	; 9 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	bit $24
	dex
	Jne *-6
.elseif(n = 2886)
	; 9 bytes
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 4
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891 || n = 2908 || n = 2925)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2892)
	; 9 bytes
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	bit $24
	inx
	Jne *-6
.elseif((n .mod 12 = 1) && ((n > 2892 && n < 2930)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2894 || n = 2895 || n = 2897 || n = 2901 || n = 2902 || n = 2907 || n = 2909 || n = 2910 || n = 2914 || n = 2915 || n = 2919 || (n > 2920 && n < 2924))
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2896 || n = 2911 || n = 2926)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2898)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2899 || n = 2913 || n = 2927)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2900 || n = 2906 || n = 2912 || n = 2924)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2903)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	bit $24
	inx
	Jne *-6
.elseif(n = 2904)
	; 9 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	bit $24
	inx
	Jne *-6
.elseif(n = 2916)
	; 9 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	bit $24
	inx
	Jne *-6
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
.elseif(n = 2920)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2928)
	; 9 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	bit $24
	dex
	Jne *-6
.endif
.else
; 2930 to 2968
.if(n = 2930 || n = 2932 || n = 2944 || n = 2968)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2931 || n = 2933 || n = 2935 || n = 2943 || n = 2945 || n = 2954 || n = 2958 || n = 2966 || n = 2967)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2934 || n = 2936 || n = 2948 || n = 2949 || n = 2960 || n = 2962)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2937)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 4
	inx
	Jne *-6
.elseif(n = 2938)
	; 9 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 4
	inx
	Jne *-6
.elseif(n = 2939 || n = 2952)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2940)
	; 9 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	bit $24
	dex
	Jne *-6
.elseif(n = 2941 || n = 2953 || n = 2965)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2942 || n = 2959)
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
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 2950)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 4
	inx
	Jne *-6
.elseif(n = 2951)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	bit $24
	inx
	Jne *-6
.elseif(n = 2955)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2956)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
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
.elseif(n = 2963)
	; 9 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	bit $24
	inx
	Jne *-6
.elseif(n = 2964)
	; 9 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	bit $24
	dex
	Jne *-6
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
.elseif(n = 2970 || n = 2973 || n = 2974 || n = 2979 || n = 2981 || n = 2985 || n = 2991 || n = 2994 || n = 2995 || n = 2998 || n = 3003 || n = 3005 || n = 3006)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2971 || n = 2986)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2972 || n = 2984 || n = 2996)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2975)
	; 8 bytes
	ldx #248 ;hides 'sed'
	nop
	bit $24
	dex
	Jne *-5
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
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	inx
	Jne *-6
.elseif(n = 2988)
	; 9 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	inx
	Jne *-6
.elseif(n = 2990)
	; 9 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 4
	inx
	Jne *-6
.elseif(n = 2999)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3000)
	; 9 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	inx
	Jne *-6
.elseif(n = 3002)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 3007)
	; 9 bytes
	ldx #<$2C ;hides 'bit $BCA2'
	ldx #188
	delay_n 7
	dex
	Jne *-6
.endif
.else
; 3008 to 3046
.if(n = 3008 || n = 3018 || n = 3020 || n = 3030 || n = 3032)
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
	; 9 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	inx
	Jne *-6
.elseif(n = 3013 || n = 3025 || n = 3037)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3014 || n = 3026 || n = 3028 || n = 3040)
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
.elseif(n = 3019 || n = 3027 || n = 3029 || n = 3033 || n = 3034 || n = 3038 || n = 3045)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3021)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
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
	; 9 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	inx
	Jne *-6
.elseif(n = 3035)
	; 9 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	inx
	Jne *-6
.elseif(n = 3036)
	; 9 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	inx
	Jne *-6
.elseif(n = 3041)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3042)
	; 9 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	delay_n 4
	dex
	Jne *-6
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
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	inx
	Jne *-6
.elseif(n = 3048)
	; 9 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	inx
	Jne *-6
.elseif(n = 3049 || n = 3061)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3050 || n = 3051 || n = 3055 || n = 3058 || n = 3062 || n = 3063 || n = 3065 || n = 3070 || n = 3075 || n = 3077 || (n > 3082 && n < 3086))
	; 8..10 bytes
	delay_n (n-2)
	nop
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
	; 9 bytes
	ldx #<$2C ;hides 'bit $EBA2'
	ldx #235
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3056 || n = 3068 || n = 3074 || n = 3080)
	; 9..10 bytes
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
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	inx
	Jne *-6
.elseif(n = 3060 || n = 3079)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3066)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3069 || n = 3082)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3071)
	; 9 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	dex
	Jne *-6
.elseif(n = 3072)
	; 9 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	inx
	Jne *-6
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3076)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3078)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.endif
.else
; 3086 to 3125
.if(n = 3086)
	; 10 bytes
	ldx #<$2C ;hides 'bit $93A2'
	ldx #147
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3087)
	; 9 bytes
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3088)
	; 9 bytes
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3089 || n = 3105)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3090)
	; 10 bytes
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3091 || n = 3106 || n = 3121)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3092 || n = 3118 || n = 3124)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3093)
	; 9 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3094)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3095 || n = 3109 || n = 3123)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3096 || n = 3097 || n = 3099 || n = 3100 || n = 3110 || n = 3111 || n = 3113 || n = 3114 || n = 3125)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3098 || n = 3117)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3101)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 3102 || n = 3116)
	; 10 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3107)
	; 9 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3108)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3115)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
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
.elseif(n = 3122)
	; 10 bytes
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	nop
	bit $24
	dex
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
.if((n >= 3126) && (n < 3165))
; 3126 to 3164
.if(n = 3126 || n = 3140 || n = 3154)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3127)
	; 9 bytes
	ldx #184 ;hides 'clv'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 3128)
	; 10 bytes
	ldx #<$2C ;hides 'bit $6BA2'
	ldx #107
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 3129 || n = 3146 || n = 3163)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3130 || n = 3144)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3131 || n = 3138 || n = 3139 || n = 3141 || n = 3148 || n = 3149 || n = 3157 || n = 3161 || n = 3162)
	; 9..10 bytes
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
	; 9 bytes
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3134 || n = 3147 || n = 3160)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3135)
	; 9 bytes
	ldx #<$2C ;hides 'bit $C4A2'
	ldx #196
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3136)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3137 || n = 3151)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3142 || n = 3143 || n = 3156)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3145)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3150)
	; 10 bytes
	bit <$2C ;hides 'bit $E1A2'
	ldx #225
	nop
	bit $24
	dex
	Jne *-7
.elseif(n = 3152)
	; 9 bytes
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3153)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3155)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3158)
	; 9 bytes
	ldx #<$2C ;hides 'bit $F3A2'
	ldx #243
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3159)
	; 9 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3164)
	; 10 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	bit $24
	inx
	Jne *-7
.endif
.else
; 3165 to 3203
.if(n = 3165 || n = 3179 || n = 3193)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3166 || n = 3181 || n = 3196)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3167 || n = 3170 || n = 3175 || n = 3176 || n = 3182 || n = 3188 || n = 3195 || n = 3203)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3168)
	; 9 bytes
	bit <$2C ;hides 'bit $C6A2'
	ldx #198
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3169 || n = 3185 || n = 3201)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3171)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0CA2'
	ldx #12
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3172)
	; 9 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3173 || n = 3186)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3174)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3177)
	; 10 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3178)
	; 10 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3180 || n = 3197)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3183)
	; 9 bytes
	ldx #<$2C ;hides 'bit $C7A2'
	ldx #199
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3184)
	; 9 bytes
	ldx #<$2C ;hides 'bit $F5A2'
	ldx #245
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3187)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3189 || n = 3190)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3191)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E4A2'
	ldx #228
	nop
	bit $24
	dex
	Jne *-7
.elseif(n = 3192)
	; 10 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	nop
	bit $24
	dex
	Jne *-7
.elseif(n = 3194)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D5A2'
	ldx #213
	delay_n 6
	dex
	Jne *-7
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
.elseif(n = 3202)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if(n = 3204)
	; 10 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3205)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3206 || n = 3228 || n = 3230 || n = 3242)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3207 || n = 3221 || n = 3235)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3208 || n = 3229)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3209 || n = 3213 || n = 3218 || n = 3219 || n = 3227 || n = 3237 || n = 3240)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3210)
	; 9 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3211 || n = 3226 || n = 3241)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3212 || n = 3225 || n = 3238)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
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
.elseif(n = 3217 || n = 3233)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3220)
	; 10 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3222)
	; 10 bytes
	bit <$2C ;hides 'bit $B3A2'
	ldx #179
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3223)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3224)
	; 9 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3232)
	; 9 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3234)
	; 10 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3236)
	; 9 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3239)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 8
	dex
	Jne *-6
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3253 || n = 3258 || n = 3266 || n = 3267 || n = 3273 || n = 3279)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n > 3243 && n < 3247) || n = 3260 || n = 3274)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 3249 || n = 3263 || n = 3277)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3250 || n = 3269)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3251 || n = 3264)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3252 || n = 3268 || n = 3278)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3254 || n = 3270 || n = 3272)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3255)
	; 10 bytes
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3256 || n = 3271)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3257)
	; 10 bytes
	ldx #<$2C ;hides 'bit $B5A2'
	ldx #181
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3259)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3261)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E9A2'
	ldx #233
	nop
	bit $24
	dex
	Jne *-7
.elseif(n = 3262)
	; 9 bytes
	ldx #<$2C ;hides 'bit $05A2'
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
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3280)
	; 9 bytes
	bit <$2C ;hides 'bit $CDA2'
	ldx #205
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3281)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3299 || n = 3316)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283 || n = 3284 || n = 3287 || n = 3293 || n = 3304 || n = 3317 || n = 3318 || n = 3320)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3286 || n = 3301)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3288 || n = 3307)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3289)
	; 9 bytes
	bit <$2C ;hides 'bit $03A2'
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
.elseif(n = 3291 || n = 3305 || n = 3319)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3292 || n = 3298 || n = 3306)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3294 || n = 3308)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3295)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3296)
	; 9 bytes
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3297 || n = 3313)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3300)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3302)
	; 9 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3309)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3310)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3323 || n = 3330 || n = 3335 || n = 3337 || n = 3348 || n = 3349 || n = 3351 || n = 3352)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3322 || n = 3340 || n = 3354 || n = 3357)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3324 || n = 3325 || n = 3334 || n = 3342 || n = 3355 || n = 3356 || n = 3358)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
	; 9 bytes
	ldx #0
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3331 || n = 3346)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3332)
	; 10 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3333 || n = 3347)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3336 || n = 3353)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3338)
	; 10 bytes
	ldx #<$2C ;hides 'bit $9FA2'
	ldx #159
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3339)
	; 10 bytes
	bit <$2C ;hides 'bit $61A2'
	ldx #97
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 3341)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
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
.elseif(n = 3350)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
; 3360 to 3398
.if(n = 3360)
	; 9 bytes
	bit <$2C ;hides 'bit $D2A2'
	ldx #210
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3361 || n = 3375 || n = 3389)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3362 || n = 3363 || n = 3365 || n = 3366 || n = 3369 || n = 3371 || n = 3378 || n = 3386 || n = 3394 || n = 3395)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3364 || n = 3383)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3367 || n = 3384)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3368 || n = 3382 || n = 3396)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3370)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3372 || n = 3397)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3373)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	nop
	bit $24
	dex
	Jne *-7
.elseif(n = 3374)
	; 10 bytes
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3376 || n = 3391)
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
.elseif(n = 3380)
	; 10 bytes
	ldx #<$2C ;hides 'bit $A1A2'
	ldx #161
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3381)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 3385)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3387)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3388)
	; 10 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3390)
	; 10 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3392)
	; 9 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3398)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
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
.elseif(n = 3403 || n = 3417 || n = 3431)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3404 || n = 3405 || n = 3411 || n = 3419 || n = 3420 || n = 3426 || n = 3433 || n = 3437)
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
	; 9 bytes
	ldx #<$2C ;hides 'bit $D5A2'
	ldx #213
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3408)
	; 9 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3409 || n = 3425)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3410)
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
.elseif(n = 3413 || n = 3428 || n = 3432)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3414)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3415)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0CA2'
	ldx #12
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3416)
	; 10 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	nop
	bit $24
	dex
	Jne *-7
.elseif(n = 3422)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3423)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D6A2'
	ldx #214
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3424)
	; 9 bytes
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3427)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3429)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F5A2'
	ldx #245
	nop
	bit $24
	dex
	Jne *-7
.elseif(n = 3430)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3434)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3462)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3439)
	; 9 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	delay_n 7
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
.elseif(n = 3443 || n = 3447 || n = 3449 || n = 3454 || n = 3461 || n = 3463 || n = 3468 || n = 3475)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3444)
	; 10 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	nop
	bit $24
	dex
	Jne *-7
.elseif(n = 3445 || n = 3459 || n = 3473)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3446 || n = 3460 || n = 3467 || n = 3470)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3448 || n = 3453 || n = 3458 || n = 3474 || n = 3476)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3450)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 3451 || n = 3466)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
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
.elseif(n = 3464)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
.elseif(n = 3465)
	; 10 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 6
	inx
	Jne *-7
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
.if(n = 3477 || n = 3483 || n = 3498 || n = 3499 || n = 3506 || n = 3507 || n = 3513)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3478 || n = 3480 || n = 3494 || n = 3502 || n = 3508)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3479)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3481 || n = 3496 || n = 3511)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3482 || n = 3484 || n = 3512)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3485)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	inx
	Jne *-7
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
.elseif(n = 3490)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3491)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3492)
	; 10 bytes
	bit <$2C ;hides 'bit $C2A2'
	ldx #194
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3493)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3495)
	; 8 bytes
	ldx #184 ;hides 'clv'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 3497)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3500)
	; 10 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3504)
	; 9 bytes
	bit <$2C ;hides 'bit $DBA2'
	ldx #219
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3509)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3510)
	; 10 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3514)
	; 10 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516 || n = 3535)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3517 || n = 3522 || n = 3523 || n = 3528 || n = 3531 || n = 3533 || n = 3538 || n = 3539 || n = 3545 || n = 3547)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3518 || n = 3532 || n = 3546)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3519)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DCA2'
	ldx #220
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3520 || n = 3537 || n = 3554)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3521 || n = 3553)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3524)
	; 10 bytes
	ldx #<$2C ;hides 'bit $EBA2'
	ldx #235
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3525)
	; 10 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3526 || n = 3541)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3527)
	; 10 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3529 || n = 3543)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3530 || n = 3534)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3536)
	; 9 bytes
	bit <$2C ;hides 'bit $DDA2'
	ldx #221
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3540)
	; 10 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3542)
	; 10 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3544 || n = 3550)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3548)
	; 10 bytes
	ldx #<$2C ;hides 'bit $A9A2'
	ldx #169
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3549)
	; 10 bytes
	bit <$2C ;hides 'bit $A9A2'
	ldx #169
	jsr @nearby_rts
	dex
	Jne *-7
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
.endif
.else
; 3555 to 3593
.if(n = 3555 || n = 3558 || n = 3559 || n = 3561 || n = 3587 || n = 3589 || n = 3590)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3556 || n = 3586)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3557 || n = 3571)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3560 || n = 3574 || n = 3591)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3562 || n = 3566 || n = 3572 || n = 3576 || n = 3580 || n = 3593)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3563)
	; 10 bytes
	ldx #<$2C ;hides 'bit $C6A2'
	ldx #198
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3564 || n = 3578)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3565)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3567)
	; 9 bytes
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3568)
	; 9 bytes
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3569)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3570)
	; 10 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3573 || n = 3592)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3575)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3577)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 3579)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
.elseif(n = 3581)
	; 10 bytes
	ldx #<$2C ;hides 'bit $C7A2'
	ldx #199
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3582)
	; 10 bytes
	bit <$2C ;hides 'bit $C7A2'
	ldx #199
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3583)
	; 9 bytes
	ldx #<$2C ;hides 'bit $E0A2'
	ldx #224
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3584)
	; 9 bytes
	bit <$2C ;hides 'bit $E0A2'
	ldx #224
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3588)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3602 || n = 3603 || n = 3607 || n = 3609 || n = 3618 || n = 3624 || n = 3626)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3612)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 3595 && n < 3599) || n = 3604 || n = 3606 || n = 3610 || n = 3620 || n = 3623 || n = 3627 || n = 3628)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 3601 || n = 3616 || n = 3631)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3605 || n = 3622)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3608 || n = 3625)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3611 || n = 3630)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3613)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3614)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3615)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3617)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3619)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3621)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 3629)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3632)
	; 9 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 7
	inx
	Jne *-6
.endif
.else
; 3633 to 3671
.if(n = 3633 || n = 3649 || n = 3665)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3634)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3635 || n = 3641 || n = 3643 || n = 3650 || n = 3651 || n = 3658 || n = 3666 || n = 3667 || n = 3670)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3636)
	; 10 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3637 || n = 3655)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3638 || n = 3640 || n = 3652 || n = 3657 || n = 3662 || n = 3669)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3639 || n = 3656)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3642)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3644)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F3A2'
	ldx #243
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3645)
	; 10 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3646 || n = 3661)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
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
.elseif(n = 3653)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
.elseif(n = 3654)
	; 10 bytes
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3659)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0CA2'
	ldx #12
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3660)
	; 10 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	delay_n 6
	dex
	Jne *-7
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
.elseif(n = 3668)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3671)
	; 9 bytes
	ldx #216 ;hides 'cld'
	rol $26
	ror $26
	dex
	Jne *-6
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
.elseif(n = 3673 || n = 3690 || n = 3707)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3674)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F5A2'
	ldx #245
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3675 || n = 3677 || n = 3678 || n = 3682 || n = 3683 || n = 3689 || (n > 3691 && n < 3695) || n = 3699 || n = 3708 || n = 3709)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3676 || n = 3691 || n = 3706)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3679)
	; 9 bytes
	ldx #<$2C ;hides 'bit $E6A2'
	ldx #230
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3680)
	; 9 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3681 || n = 3697)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif((n > 3683 && n < 3687) || n = 3688 || n = 3700 || n = 3703)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3687)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
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
.elseif(n = 3698 || n = 3710)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3701)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 3702)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3704)
	; 10 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3705)
	; 10 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	delay_n 6
	dex
	Jne *-7
.endif
.else
; 3711 to 3750
.if(n = 3711)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3712)
	; 9 bytes
	bit <$2C ;hides 'bit $E8A2'
	ldx #232
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3713 || n = 3729 || n = 3745)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3714)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3715 || n = 3726 || n = 3730 || n = 3731 || n = 3737 || n = 3738 || n = 3742 || n = 3746 || n = 3747)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3716)
	; 10 bytes
	ldx #<$2C ;hides 'bit $B1A2'
	ldx #177
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3717)
	; 10 bytes
	bit <$2C ;hides 'bit $B1A2'
	ldx #177
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3718 || n = 3748 || n = 3750)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3719)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3720)
	; 10 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3721 || n = 3736)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3722 || n = 3732 || n = 3733)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 3727)
	; 9 bytes
	ldx #<$2C ;hides 'bit $E9A2'
	ldx #233
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3728)
	; 9 bytes
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3734)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3735)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
.elseif(n = 3739)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
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
	; 10 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	inx
	Jne *-7
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
.if(n = 3751 || n = 3766 || n = 3781)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3752 || (n > 3753 && n < 3757) || n = 3770 || n = 3771 || n = 3773 || n = 3774 || n = 3785 || n = 3786 || n = 3788 || n = 3789)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3753 || n = 3762 || n = 3765 || n = 3768 || n = 3778 || n = 3779 || n = 3783 || n = 3784)
	; 10 bytes
	delay_n (n-2)
	nop
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
	; 9 bytes
	ldx #<$2C ;hides 'bit $EBA2'
	ldx #235
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3760)
	; 9 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3761 || n = 3777)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3763 || n = 3782)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3764)
	; 10 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3767)
	; 10 bytes
	pha
	ldx #5 ;hides 'ora zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	pla
.elseif(n = 3769)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
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
.elseif(n = 3776)
	; 9 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3780)
	; 10 bytes
	bit <$2C ;hides 'bit $04A2'
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
.endif
.else
; 3790 to 3828
.if(n = 3790 || n = 3804 || n = 3805 || n = 3814 || n = 3818 || n = 3819)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3791)
	; 9 bytes
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3792 || n = 3809 || n = 3826)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3793 || n = 3825)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3794 || n = 3795 || n = 3798 || n = 3803 || n = 3813 || n = 3822 || n = 3827 || n = 3828)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3796 || n = 3811)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3797)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D3A2'
	ldx #211
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3799)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3800)
	; 10 bytes
	ldx #<$2C ;hides 'bit $B5A2'
	ldx #181
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3801 || n = 3820)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3802)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3806)
	; 10 bytes
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
.elseif(n = 3810)
	; 10 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3812)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3815)
	; 10 bytes
	ldx #44 ;hides 'bit abs'
	bit $EAEA ;hides 'nop'
	delay_n 7
	inx
	Jne *-7
.elseif(n = 3816)
	; 10 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 9
	inx
	Jne *-7
.elseif(n = 3817)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3821)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 3823)
	; 9 bytes
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3824)
	; 9 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	delay_n 7
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3846)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3830 || n = 3842 || n = 3845 || n = 3847 || n = 3859 || n = 3862 || n = 3864)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3831 || n = 3832 || (n > 3835 && n < 3839) || n = 3848 || n = 3849 || n = 3854 || n = 3866)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3833)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D5A2'
	ldx #213
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3834)
	; 10 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3835 || n = 3853)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
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
.elseif(n = 3841)
	; 9 bytes
	ldx #0
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3843 || n = 3860)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3844 || n = 3865)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3850 || n = 3867)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3851)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D6A2'
	ldx #214
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3852)
	; 10 bytes
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3855)
	; 9 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	delay_n 7
	dex
	Jne *-6
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
.elseif(n = 3861)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 3863)
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 14
	dex
	Jne *-6
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3876 || n = 3878 || n = 3882 || n = 3883 || n = 3892 || n = 3893 || n = 3895 || n = 3899 || n = 3900 || n = 3902)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3869)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3870)
	; 10 bytes
	bit <$2C ;hides 'bit $D7A2'
	ldx #215
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3871)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3872)
	; 9 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3873 || n = 3889 || n = 3905)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3874 || n = 3875 || n = 3879 || n = 3881 || n = 3890 || n = 3891 || n = 3898)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3877 || n = 3894)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3880 || n = 3897)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3884 || n = 3901)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3885)
	; 10 bytes
	bit <$2C ;hides 'bit $B9A2'
	ldx #185
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3886)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3887)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 3888)
	; 9 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3896)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3903)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0CA2'
	ldx #12
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3904)
	; 9 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3906)
	; 10 bytes
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	delay_n 9
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
; 3907 to 3945
.if(n = 3907 || n = 3925)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif((n > 3907 && n < 3911) || n = 3912 || n = 3916 || n = 3929 || n = 3933 || n = 3940 || n = 3944)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3911 || n = 3928 || n = 3945)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3913 || n = 3917 || n = 3922 || n = 3923 || n = 3930 || n = 3932 || n = 3938 || n = 3939)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3914 || n = 3931)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3915 || n = 3934)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3918)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3919)
	; 9 bytes
	ldx #<$2C ;hides 'bit $F5A2'
	ldx #245
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3920)
	; 9 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3921 || n = 3937)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3924)
	; 10 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3926)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-7
.elseif(n = 3927)
	; 10 bytes
	bit <$2C ;hides 'bit $BBA2'
	ldx #187
	jsr @nearby_rts
	dex
	Jne *-7
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
.elseif(n = 3941)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3942)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	rol $26
	ror $26
	inx
	Jne *-7
.elseif(n = 3943)
	; 9 bytes
	ldx #24 ;hides 'clc'
	rol $26
	ror $26
	inx
	Jne *-6
.endif
.else
; 3946 to 3984
.if(n = 3946 || n = 3950 || (n > 3955 && n < 3959) || n = 3963 || n = 3973 || n = 3975 || n = 3976 || n = 3980)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3947 || n = 3954 || n = 3955 || n = 3964 || n = 3966 || n = 3971 || n = 3981)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3948 || n = 3965 || n = 3982)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3949 || n = 3970)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
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
.elseif(n = 3953 || n = 3969)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3959)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	delay_n 7
	inx
	Jne *-7
.elseif(n = 3960)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	nop
	rol $26
	ror $26
	inx
	Jne *-7
.elseif(n = 3961)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3962 || n = 3979)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.elseif(n = 3972)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3974)
	; 10 bytes
	php
	delay_n (n-7)
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
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	rol $26
	ror $26
	inx
	Jne *-7
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
.else
; 3985 to 4062
.if((n .mod 16 = 1) && ((n > 3984 && n < 4050)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3986 || n = 3987 || n = 3993 || n = 3998 || n = 4002 || n = 4003 || n = 4018 || n = 4019 || n = 4034 || n = 4035 || n = 4050)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3988 || n = 3992 || n = 3994 || (n > 4003 && n < 4009) || (n > 4021 && n < 4029) || n = 4036 || (n > 4037 && n < 4041) || (n > 4041 && n < 4047) || n = 4055 || n = 4056 || n = 4058 || n = 4060 || n = 4062)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3995)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3997 || n = 4051)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3999)
	; 9 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4000)
	; 9 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4009 || n = 4057)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4011)
	; 10 bytes
	bit <$2C ;hides 'bit $BFA2'
	ldx #191
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4012 || n = 4054)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4014)
	; 10 bytes
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4015)
	; 9 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4016)
	; 9 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4020 || n = 4037)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4021 || n = 4041 || n = 4061)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
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
.elseif(n = 4052)
	; 10 bytes
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4053)
	; 10 bytes
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	jsr @nearby_rts
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
.elseif(n = 4066 || n = 4082 || n = 4083 || n = 4088 || n = 4099 || n = 4100)
	; 9..10 bytes
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
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4070 || n = 4072 || (n > 4075 && n < 4079) || (n > 4088 && n < 4094))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4071)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4073)
	; 10 bytes
	ldx #<$2C ;hides 'bit $C2A2'
	ldx #194
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4074)
	; 10 bytes
	bit <$2C ;hides 'bit $C2A2'
	ldx #194
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4075)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
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
.elseif(n = 4084 || n = 4101)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4085)
	; 10 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4094)
	; 10 bytes
	ldx #<$2C ;hides 'bit $C3A2'
	ldx #195
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4095)
	; 9 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 7
	dex
	Jne *-6
.elseif(n = 4096)
	; 9 bytes
	bit <$2C ;hides 'bit $00A2'
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
.endif
.else
; 4102 to 4140
.if(n = 4102 || n = 4107 || n = 4117 || n = 4119 || n = 4126 || n = 4134 || n = 4136)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4103)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4104)
	; 10 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4105 || n = 4124)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4106 || n = 4118 || n = 4135)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4108 || n = 4109 || (n > 4110 && n < 4115) || n = 4120 || n = 4125 || n = 4127 || n = 4128 || n = 4130 || n = 4131 || n = 4133)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4110)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4115 || n = 4132)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4116)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
.elseif(n = 4121)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E5A2'
	ldx #229
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4122)
	; 10 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4123)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4129)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4137)
	; 10 bytes
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4138)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4139)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E6A2'
	ldx #230
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4140)
	; 10 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 9
	inx
	Jne *-7
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141 || n = 4159 || n = 4177 || n = 4195 || n = 4213)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4142 || n = 4144 || (n > 4145 && n < 4149) || n = 4150 || n = 4154 || n = 4155 || n = 4160 || n = 4163 || n = 4165 || n = 4167 || n = 4171 || n = 4172 || n = 4174 || n = 4182 || n = 4184 || n = 4188 || n = 4189 || n = 4191 || n = 4192 || (n > 4195 && n < 4199) || n = 4205 || n = 4206 || (n > 4207 && n < 4211) || n = 4214 || n = 4216 || n = 4218)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4143 || n = 4162 || n = 4181)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4145 || n = 4151 || n = 4153 || n = 4164 || n = 4168 || n = 4170 || n = 4185 || n = 4187 || n = 4202 || n = 4204)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4152 || n = 4169 || n = 4186 || n = 4203)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4156 || n = 4173 || n = 4190 || n = 4207)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4157)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
.elseif(n = 4158)
	; 10 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4161)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 11
	inx
	Jne *-7
.elseif(n = 4175)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 4176)
	; 10 bytes
	bit <$2C ;hides 'bit $E8A2'
	ldx #232
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4178)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $EAEA ;hides two 'nop's
	delay_n 7
	inx
	Jne *-7
.elseif(n = 4179)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4180 || n = 4201)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
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
.elseif(n = 4212)
	; 10 bytes
	bit <$2C ;hides 'bit $EAA2'
	ldx #234
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4215)
	; 9 bytes
	ldx #248 ;hides 'sed'
	rol $26
	ror $26
	dex
	Jne *-6
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
.elseif(n = 4220 || n = 4237 || n = 4254 || n = 4271 || n = 4288)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4221 || n = 4236 || n = 4240 || n = 4253 || n = 4255 || n = 4259 || n = 4270 || n = 4272 || n = 4278 || n = 4287 || n = 4289)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4222 || n = 4243 || n = 4264)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4223 || (n > 4225 && n < 4229) || n = 4232 || n = 4233 || n = 4235 || n = 4239 || (n > 4243 && n < 4247) || n = 4250 || n = 4252 || n = 4256 || n = 4260 || n = 4269 || n = 4274 || n = 4277 || n = 4279 || n = 4280 || n = 4282 || n = 4286 || n = 4290 || n = 4291 || n = 4293 || n = 4294 || n = 4296)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4224 || n = 4241 || n = 4258 || n = 4275 || n = 4292)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4225 || n = 4273)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4229)
	; 10 bytes
	ldx #<$2C ;hides 'bit $EBA2'
	ldx #235
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4230)
	; 10 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4231 || n = 4249 || n = 4267)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4234 || n = 4251 || n = 4268 || n = 4285)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4242)
	; 10 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4247)
	; 10 bytes
	ldx #<$2C ;hides 'bit $ECA2'
	ldx #236
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4248)
	; 10 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4261 || n = 4281)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4262)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-6
.elseif(n = 4263)
	; 10 bytes
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4265)
	; 10 bytes
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4266)
	; 10 bytes
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4283)
	; 10 bytes
	ldx #<$2C ;hides 'bit $EEA2'
	ldx #238
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4284)
	; 10 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	delay_n 9
	inx
	Jne *-7
.endif
.else
; 4297 to 4375
.if(n = 4297 || n = 4304 || n = 4316 || n = 4321 || n = 4323 || n = 4335 || n = 4338 || n = 4340 || n = 4354 || n = 4355 || n = 4373)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n > 4297 && n < 4301) || n = 4307 || n = 4308 || (n > 4309 && n < 4314) || n = 4315 || n = 4317 || n = 4318 || n = 4324 || (n > 4327 && n < 4333) || n = 4334 || n = 4342 || n = 4344 || (n > 4348 && n < 4352) || n = 4358 || n = 4359 || (n > 4361 && n < 4367) || n = 4370 || n = 4372)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4301)
	; 10 bytes
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4302 || n = 4319 || n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4303 || n = 4339 || n = 4357 || n = 4375)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4305 || n = 4322)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4306 || n = 4327 || n = 4348 || n = 4369)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4309 || n = 4326 || n = 4343 || n = 4360)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4320)
	; 10 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4325)
	; 10 bytes
	ldx #<$2C ;hides 'bit $CEA2'
	ldx #206
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4337)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4341 || n = 4361)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4345)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4346)
	; 10 bytes
	ldx #<$2C ;hides 'bit $CFA2'
	ldx #207
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4347)
	; 10 bytes
	bit <$2C ;hides 'bit $CFA2'
	ldx #207
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4353)
	; 8 bytes
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4356)
	; 10 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 9
	inx
	Jne *-7
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
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
; 4376 to 4453
.if((n > 4375 && n < 4380) || (n > 4382 && n < 4388) || (n > 4393 && n < 4399) || (n > 4402 && n < 4407) || n = 4408 || n = 4412 || n = 4413 || n = 4415 || n = 4416 || n = 4418 || n = 4419 || (n > 4422 && n < 4427) || (n > 4432 && n < 4441) || (n > 4441 && n < 4445) || n = 4450)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4380)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4381 || n = 4401 || n = 4421)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4382 || n = 4392 || n = 4402 || n = 4422 || n = 4430 || n = 4449)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4388)
	; 10 bytes
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
.elseif(n = 4389)
	; 10 bytes
	bit <$2C ;hides 'bit $D1A2'
	ldx #209
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4390 || n = 4409 || n = 4428 || n = 4447)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4391)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0CA2'
	ldx #12
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4393 || n = 4411 || n = 4429)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4399)
	; 10 bytes
	ldx #56 ;hides 'sec'
	bit $24
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4400)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4407)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4410)
	; 10 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4414 || n = 4452)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4417 || n = 4441)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4420)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4427)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4431)
	; 10 bytes
	bit <$2C ;hides 'bit $D3A2'
	ldx #211
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4432 || n = 4453)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
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
.elseif(n = 4448)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4451)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D4A2'
	ldx #212
	jsr @nearby_rts
	dex
	Jne *-7
.endif
.else
; 4454 to 4531
.if((n > 4453 && n < 4461) || n = 4462 || n = 4467 || (n > 4468 && n < 4472) || (n > 4474 && n < 4481) || n = 4484 || n = 4486 || n = 4488 || (n > 4489 && n < 4493) || (n > 4495 && n < 4499) || n = 4502 || n = 4503 || n = 4505 || (n > 4506 && n < 4513) || n = 4520 || n = 4522 || n = 4524 || (n > 4525 && n < 4532))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4461 || n = 4521)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4463)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4464)
	; 10 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4465)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 17
	Jne *-7
.elseif(n = 4466 || n = 4485 || n = 4504 || n = 4523)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4468 || n = 4487 || n = 4506 || n = 4525)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4472)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D5A2'
	ldx #213
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4473)
	; 10 bytes
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4474 || n = 4495 || n = 4516)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4481)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4482)
	; 10 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4483 || n = 4501 || n = 4519)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4489 || n = 4513)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4493)
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
.elseif(n = 4499)
	; 10 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4500)
	; 10 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4514)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	jsr @nearby_rts
	dex
	Jne *-7
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
.elseif(n = 4518)
	; 10 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 9
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if((n > 4531 && n < 4535) || (n > 4537 && n < 4541) || n = 4543 || (n > 4544 && n < 4553) || n = 4559 || n = 4560 || n = 4562 || (n > 4563 && n < 4571) || (n > 4573 && n < 4577) || n = 4583 || n = 4584 || (n > 4585 && n < 4589) || (n > 4591 && n < 4598) || (n > 4601 && n < 4607))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4535)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 4536)
	; 10 bytes
	bit <$2C ;hides 'bit $04A2'
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
.elseif(n = 4541 || n = 4581)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4542 || n = 4561 || n = 4580 || n = 4599)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4544 || n = 4563 || n = 4582 || n = 4601)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4553)
	; 10 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4554)
	; 10 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4556)
	; 10 bytes
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4557)
	; 10 bytes
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4558 || n = 4579 || n = 4600)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4571)
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
.elseif(n = 4577)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DAA2'
	ldx #218
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4578)
	; 10 bytes
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4585)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4589)
	; 10 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4590)
	; 10 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4598)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4607)
	; 10 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4608)
	; 10 bytes
	bit <$2C ;hides 'bit $00A2'
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
.endif
.else
; 4610 to 4687
.if((n > 4609 && n < 4618) || (n > 4621 && n < 4633) || (n > 4633 && n < 4637) || n = 4638 || (n > 4642 && n < 4656) || n = 4659 || n = 4660 || (n > 4663 && n < 4668) || (n > 4668 && n < 4675) || n = 4676 || n = 4678 || n = 4680 || (n > 4684 && n < 4688))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4618 || n = 4637 || n = 4656 || n = 4675)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4619)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
.elseif(n = 4620 || n = 4639 || n = 4658 || n = 4677)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4621 || n = 4642 || n = 4663 || n = 4684)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4633 || n = 4657 || n = 4681)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4640)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DDA2'
	ldx #221
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4641 || n = 4661)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4662)
	; 10 bytes
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4668)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-7
.elseif(n = 4679)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4682)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4683)
	; 10 bytes
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	jsr @nearby_rts
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
; 4688 to 4765
.if((n > 4687 && n < 4694) || n = 4695 || (n > 4696 && n < 4701) || n = 4702 || (n > 4705 && n < 4711) || n = 4712 || n = 4716 || n = 4717 || n = 4719 || n = 4720 || n = 4722 || n = 4723 || n = 4727 || n = 4728 || n = 4730 || n = 4731 || n = 4733 || (n > 4734 && n < 4741) || (n > 4741 && n < 4745) || (n > 4747 && n < 4751) || n = 4752 || (n > 4753 && n < 4761) || (n > 4761 && n < 4766))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4694 || n = 4713 || n = 4732 || n = 4751)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4696 || n = 4715 || n = 4734 || n = 4753)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4701 || n = 4721 || n = 4741 || n = 4761)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4703)
	; 10 bytes
	ldx #<$2C ;hides 'bit $E0A2'
	ldx #224
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4704)
	; 10 bytes
	bit <$2C ;hides 'bit $E0A2'
	ldx #224
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4705 || n = 4726 || n = 4747)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4711)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4714)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4718)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4724)
	; 10 bytes
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4725)
	; 10 bytes
	bit <$2C ;hides 'bit $E1A2'
	ldx #225
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4729)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
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
.endif
.else
; 4766 to 4843
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
.elseif(n = 4768 || n = 4810 || n = 4831)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4769 || n = 4771 || (n > 4772 && n < 4777) || (n > 4777 && n < 4781) || n = 4782 || (n > 4783 && n < 4787) || n = 4790 || (n > 4791 && n < 4801) || (n > 4801 && n < 4808) || (n > 4810 && n < 4818) || n = 4819 || n = 4820 || (n > 4821 && n < 4825) || n = 4826 || n = 4828 || (n > 4831 && n < 4835) || (n > 4835 && n < 4839) || n = 4842 || n = 4843)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4770 || n = 4789 || n = 4808 || n = 4827)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4772 || n = 4791 || n = 4829)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4777 || n = 4801 || n = 4825)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4781 || n = 4821 || n = 4841)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4783)
	; 10 bytes
	ldx #184 ;hides 'clv'
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-7
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
.elseif(n = 4809)
	; 10 bytes
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4818)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 4830)
	; 10 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4835)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	inx
	Jne *-7
.elseif(n = 4839)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4840)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 16
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
; 4844 to 4921
.if(n = 4844 || n = 4845 || n = 4847 || (n > 4852 && n < 4861) || n = 4863 || n = 4864 || n = 4866 || (n > 4867 && n < 4871) || (n > 4873 && n < 4881) || (n > 4881 && n < 4892) || n = 4895 || n = 4896 || (n > 4897 && n < 4901) || (n > 4901 && n < 4913) || (n > 4915 && n < 4921))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4846)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4848 || n = 4867)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4849 || n = 4897 || n = 4921)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4850)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
.elseif(n = 4851)
	; 10 bytes
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4852 || n = 4873 || n = 4894 || n = 4915)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4861 || n = 4881 || n = 4901)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4862)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 14
	inx
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
.elseif(n = 4872)
	; 10 bytes
	bit <$2C ;hides 'bit $E8A2'
	ldx #232
	jsr @nearby_rts
	dex
	Jne *-7
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
.endif
.else
; 4922 to 5000
.if((n > 4921 && n < 4934) || (n > 4936 && n < 4941) || (n > 4941 && n < 4945) || (n > 4945 && n < 4955) || n = 4958 || n = 4960 || (n > 4961 && n < 4967) || n = 4968 || (n > 4969 && n < 4976) || n = 4979 || n = 4980 || (n > 4981 && n < 4993) || (n > 4993 && n < 4997) || n = 5000)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4934)
	; 10 bytes
	ldx #<$2C ;hides 'bit $EBA2'
	ldx #235
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4935)
	; 10 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4936 || n = 4957 || n = 4978 || n = 4999)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4941 || n = 4961 || n = 4981)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4945 || n = 4969 || n = 4993)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4955)
	; 10 bytes
	ldx #<$2C ;hides 'bit $ECA2'
	ldx #236
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4956)
	; 10 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4959)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4967)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 4976)
	; 10 bytes
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4977)
	; 10 bytes
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	jsr @nearby_rts
	dex
	Jne *-7
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
; 5001 to 5156
.if(n = 5001 || n = 5021 || n = 5061 || n = 5081 || n = 5101)
	; 10 bytes
	ldx #(n-1)/20
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif((n > 5001 && n < 5017) || (n > 5021 && n < 5039) || (n > 5041 && n < 5060) || n = 5063 || n = 5064 || (n > 5065 && n < 5081) || (n > 5083 && n < 5089) || (n > 5089 && n < 5101) || (n > 5104 && n < 5113) || (n > 5113 && n < 5121) || n = 5122 || (n > 5125 && n < 5137) || (n > 5137 && n < 5144) || (n > 5147 && n < 5157))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5017 || n = 5065 || n = 5089 || n = 5113 || n = 5137)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
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
.elseif(n = 5020 || n = 5041 || n = 5062 || n = 5083 || n = 5104 || n = 5125 || n = 5146)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5039)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F0A2'
	ldx #240
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5040)
	; 10 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5060)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5082)
	; 10 bytes
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5102)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F3A2'
	ldx #243
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5103)
	; 10 bytes
	ldx #24 ;hides 'clc'
	bit $24
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5121)
	; 10 bytes
	ldx #0
	bit $24
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5123)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0CA2'
	ldx #12
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5124)
	; 10 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5144)
	; 10 bytes
	ldx #<$2C ;hides 'bit $F5A2'
	ldx #245
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5145)
	; 10 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5147)
	; 10 bytes
	ldx #234 ;hides 'nop'
	bit $24
	jsr @nearby_rts
	dex
	Jne *-7
.endif
.else
; 5157 to 5312
.if((n > 5156 && n < 5161) || (n > 5161 && n < 5165) || (n > 5167 && n < 5185) || (n > 5188 && n < 5199) || (n > 5199 && n < 5207) || (n > 5209 && n < 5228) || n = 5231 || n = 5232 || (n > 5233 && n < 5249) || (n > 5251 && n < 5257) || (n > 5257 && n < 5270) || (n > 5272 && n < 5277) || (n > 5277 && n < 5281) || (n > 5281 && n < 5291) || (n > 5293 && n < 5305) || (n > 5305 && n < 5312))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5161 || n = 5185 || n = 5233 || n = 5257 || n = 5281 || n = 5305)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
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
.elseif(n = 5167 || n = 5188 || n = 5209 || n = 5230 || n = 5251 || n = 5272 || n = 5293)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5186)
	; 10 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5187)
	; 10 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5199)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts
	inx
	Jne *-7
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
.elseif(n = 5249)
	; 10 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5250)
	; 10 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5270)
	; 10 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5271)
	; 10 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5277)
	; 10 bytes
	ldx #202 ;hides 'dex'
	.repeat 3
	 php
	 plp
	.endrepeat
	Jne *-7
.elseif(n = 5291)
	; 10 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5292)
	; 10 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5312)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 14
	dex
	dex
	Jne *-7
.endif
.endif
.else
; 5313 to 5625
.if(n = 5313)
	; 10 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5314 || n = 5335 || n = 5356)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif((n > 5314 && n < 5329) || (n > 5329 && n < 5333) || (n > 5335 && n < 5353) || (n > 5356 && n < 5375) || (n > 5377 && n < 5381) || (n > 5381 && n < 5401) || (n > 5401 && n < 5425) || (n > 5425 && n < 5449) || (n > 5449 && n < 5455) || (n > 5455 && n < 5473) || (n > 5473 && n < 5497) || (n > 5497 && n < 5521) || (n > 5521 && n < 5545) || (n > 5545 && n < 5569) || (n > 5569 && n < 5593) || (n > 5593 && n < 5615) || n = 5616 || (n > 5617 && n < 5626))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5329 || n = 5353 || n = 5401 || n = 5425 || n = 5449 || n = 5473 || n = 5497 || n = 5521 || n = 5545 || n = 5569 || n = 5593 || n = 5617)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5333)
	; 10 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5334)
	; 10 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	jsr @nearby_rts
	inx
	Jne *-7
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
.elseif(n = 5375)
	; 10 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5376)
	; 10 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	jsr @nearby_rts
	inx
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
.elseif(n = 5455)
	; 10 bytes
	ldx #248 ;hides 'sed'
	bit $24
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5615)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-7
.endif
.endif
.else
; 5626 to 6250
.if((n > 5625 && n < 5641) || (n > 5641 && n < 5665) || (n > 5665 && n < 5689) || (n > 5689 && n < 5703) || (n > 5703 && n < 5713) || (n > 5713 && n < 5737) || (n > 5737 && n < 5761) || (n > 5761 && n < 5785) || (n > 5785 && n < 5809) || (n > 5809 && n < 5833) || (n > 5833 && n < 5857) || (n > 5857 && n < 5881) || (n > 5881 && n < 5886) || (n > 5886 && n < 5905) || (n > 5905 && n < 5929) || (n > 5929 && n < 5953) || (n > 5953 && n < 5977) || (n > 5977 && n < 6001) || (n > 6001 && n < 6025) || (n > 6025 && n < 6031) || (n > 6031 && n < 6049) || (n > 6049 && n < 6073) || (n > 6073 && n < 6083) || (n > 6083 && n < 6097) || (n > 6097 && n < 6121) || (n > 6121 && n < 6145) || (n > 6145 && n < 6251))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5641 || n = 5665 || n = 5689 || n = 5713 || n = 5737 || n = 5761 || n = 5785 || n = 5809 || n = 5833 || n = 5857 || n = 5881 || n = 5905 || n = 5929 || n = 5953 || n = 5977 || n = 6001 || n = 6025 || n = 6049 || n = 6073 || n = 6097 || n = 6121)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5703)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5886)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
.elseif(n = 6031)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 6083)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 6145)
	; 10 bytes
	ldx #0
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6447) || (n > 6447 && n < 7501))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6447)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-7
.endif
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 10001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.endif

.endif
	rest
.endmacro
