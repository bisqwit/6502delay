; delay_n, delays n cycles with small code
; Will clobber X, Z+N, and D
; Will preserve A, Y, S, C, V, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 5
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 1289
; - 8 bytes or less when n <= 1303
; - 9 bytes or less when n <= 2269
; -10 bytes or less when n <= 2617
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.6, max=4
; - When 11 <= n <= 20: min=3, avg=4.1, max=5
; - When 21 <= n <= 30: min=5, avg=5.8, max=7
; - When 31 <= n <= 50: min=5, avg=6.3, max=7
; - When 51 <= n <= 75: min=5, avg=6.2, max=7
; - When 76 <= n <= 100: min=5, avg=6.3, max=7
; - When 101 <= n <= 200: min=4, avg=6.2, max=7
; - When 201 <= n <= 300: min=5, avg=6.2, max=7
; - When 301 <= n <= 500: min=4, avg=6.3, max=7
; - When 501 <= n <= 750: min=5, avg=6.3, max=7
; - When 751 <= n <= 1000: min=5, avg=6.3, max=7
; - When 1001 <= n <= 2000: min=4, avg=7.1, max=9
; - When 2001 <= n <= 3000: min=6, avg=8.4, max=11
; - When 3001 <= n <= 5000: min=7, avg=9.6, max=11
; - When 5001 <= n <= 7500: min=8, avg=10.8, max=11
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
	ldx $A6
.elseif(n = 4 || n = 6 || (n > 15 && n < 19) || n = 23 || n = 25 || n = 28 || n = 33 || n = 38)
	; 2..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || n = 13 || n = 19 || n = 22)
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
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 20 || n = 27 || n = 30)
	; 5..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 21 || n = 26 || n = 36)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 29)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 31)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 32)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 34 || n = 39)
	; 7 bytes
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
.if(n = 40 || n = 48 || n = 52 || n = 53 || n = 58 || n = 60 || n = 63 || n = 65 || n = 68 || n = 70 || n = 73 || n = 75)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 41 || n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 42 || n = 72)
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
.elseif(n = 44 || n = 49 || n = 54 || n = 59 || n = 69 || n = 74)
	; 7 bytes
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
.elseif(n = 47 || n = 67 || n = 77)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.endif
.endif
.else
; 79 to 156
.if((n .mod 5 = 4) && ((n > 78 && n < 95) || (n > 103 && n < 120) || (n > 138 && n < 155)))
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
.elseif(n = 83 || n = 87 || n = 88 || n = 90 || n = 93 || n = 95 || n = 98 || n = 100 || n = 103 || n = 105 || n = 108 || n = 110 || n = 118 || n = 122 || n = 123 || n = 125 || n = 128 || n = 133 || n = 135 || n = 138 || n = 140 || n = 143 || n = 145)
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
.elseif(n = 129)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 137)
	; 7 bytes
	ldx #(n-9)/8
	sta @zptemp
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
.if(n = 157 || n = 158 || n = 163 || n = 165 || n = 168 || n = 170 || n = 173 || n = 175 || n = 178 || n = 180 || n = 188 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 203 || n = 205 || n = 208 || n = 210 || n = 213 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233 || n = 235 || n = 238 || n = 240 || n = 243 || n = 245 || n = 248 || n = 250 || n = 258 || n = 263 || n = 268 || n = 270 || n = 273 || n = 275 || n = 278 || n = 280 || n = 283 || n = 285 || n = 293 || n = 297 || n = 298 || n = 300 || n = 303 || n = 305 || n = 308 || n = 310)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 177 || n = 184 || n = 189 || n = 194 || n = 209 || n = 214 || n = 219 || n = 229 || n = 234 || n = 244 || n = 249 || n = 254 || n = 259 || n = 264 || n = 269 || n = 282 || n = 284 || n = 289 || n = 294 || n = 304)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 160 || n = 272)
	; 7 bytes
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
.elseif(n = 182 || n = 187 || n = 202 || n = 212 || n = 222 || n = 237 || n = 242 || n = 252 || n = 292 || n = 312)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 185 || n = 220 || n = 255 || n = 290)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 199)
	; 6 bytes
	ldx #232 ;hides 'inx'
	sta @zptemp
	Jne *-3
.elseif(n = 207 || n = 247 || n = 287)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.elseif(n = 257)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
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
.if(n = 313 || n = 315 || n = 318 || n = 320 || n = 328 || n = 332 || n = 333 || n = 335 || n = 338 || n = 340 || n = 343 || n = 345 || n = 348 || n = 350 || n = 353 || n = 355 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 380 || n = 383 || n = 385 || n = 388 || n = 390 || n = 402 || n = 403 || n = 408 || n = 410 || n = 413 || n = 415 || n = 418 || n = 420 || n = 423 || n = 425 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 448 || n = 450 || n = 453 || n = 455 || n = 458 || n = 460 || n = 468)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314 || n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 349 || n = 354 || n = 364 || n = 369 || n = 374 || n = 382 || n = 389 || n = 394 || n = 399 || n = 409 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459 || n = 464)
	; 6..7 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 322 || n = 347 || n = 357 || n = 387 || n = 392 || n = 432 || n = 447 || n = 462)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
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
.elseif(n = 327 || n = 342 || n = 362 || n = 412 || n = 422 || n = 427 || n = 452)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.elseif(n = 377)
	; 7 bytes
	ldx #(n-9)/8
	sta @zptemp
	dex
	Jpl *-3
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 384)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
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
.if((n .mod 5 = 4) && ((n > 468 && n < 480) || (n > 488 && n < 515) || (n > 523 && n < 550) || (n > 558 && n < 585) || (n > 593 && n < 620)))
	; 7 bytes
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
.elseif(n = 472 || n = 473 || n = 475 || n = 478 || n = 480 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 528 || n = 530 || n = 535 || n = 538 || n = 542 || n = 543 || n = 545 || n = 548 || n = 553 || n = 555 || n = 558 || n = 560 || n = 563 || n = 565 || n = 570 || n = 573 || n = 578 || n = 580 || n = 583 || n = 585 || n = 588 || n = 590 || n = 593 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623 || n = 625)
	; 6..7 bytes
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
.elseif(n = 482 || n = 492 || n = 502 || n = 522 || n = 552 || n = 562 || n = 567 || n = 587 || n = 602)
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
.elseif(n = 487 || n = 577)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 497)
	; 7 bytes
	ldx #(n-9)/8
	sta @zptemp
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
.elseif(n = 527 || n = 557 || n = 572 || n = 592 || n = 597)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 532 || n = 550 || n = 595)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 537)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 607)
	; 6 bytes
	ldx #202 ;hides 'dex'
	sta @zptemp
	Jmi *-3
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
; 626 to 937
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 937)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 5 = 2) && (n = 627 || n = 632 || n = 642 || n = 662 || n = 672 || n = 677 || n = 692 || n = 702 || n = 707 || n = 732 || n = 742 || n = 747 || (n > 761 && n < 773) || n = 782 || n = 807 || n = 812 || n = 837 || n = 842 || n = 852 || n = 867 || n = 872 || n = 882 || n = 887 || (n > 901 && n < 923)))
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 628 || n = 630 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 665 || n = 668 || n = 670 || n = 675 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 693 || n = 695 || n = 698 || n = 700 || n = 703 || n = 705 || n = 713 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 752 || n = 753 || n = 758 || n = 760 || n = 763 || n = 765 || n = 768 || n = 770 || n = 773 || n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 810 || n = 818 || n = 822 || n = 823 || n = 825 || n = 828 || n = 830 || n = 833 || n = 835 || n = 838 || n = 840 || n = 843 || n = 845 || n = 853 || n = 857 || n = 858 || n = 860 || n = 863 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 903 || n = 905 || n = 908 || n = 910 || n = 913 || n = 915 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
	; 6..7 bytes
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
.elseif((n .mod 5 = 4) && ((n > 633 && n < 655) || (n > 663 && n < 690) || (n > 698 && n < 725) || (n > 738 && n < 760) || (n > 768 && n < 795) || (n > 803 && n < 830) || (n > 838 && n < 865) || (n > 873 && n < 895) || (n > 908 && n < 935)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 637)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && (n = 638 || n = 645 || (n > 672 && n < 695) || (n > 707 && n < 723) || (n > 742 && n < 765) || n = 778 || n = 785 || n = 799 || n = 813 || n = 834 || (n > 847 && n < 863) || n = 883 || n = 904))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 640 || n = 730 || n = 775 || n = 802 || n = 847 || n = 937)
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
.elseif(n = 652 || n = 659 || n = 729 || n = 792 || n = 820 || n = 827 || n = 869 || n = 890 || n = 897 || n = 918 || n = 925 || n = 932)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657 || n = 737 || n = 777)
	; 7 bytes
	ldx #(n-9)/8
	sta @zptemp
	dex
	Jpl *-3
.elseif(n = 667 || n = 685 || n = 712 || n = 865 || n = 892)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 697 || n = 817 || n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 710 || n = 745 || n = 780 || n = 815 || n = 850 || n = 885 || n = 920)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 727)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
.elseif(n = 748)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
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
.elseif(n = 832)
	; 7 bytes
	pha
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
	pla
.elseif(n = 899)
	; 6 bytes
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
.elseif(n = 935)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
.endif
.else
; 938 to 1250
.if(n = 938 || n = 940 || n = 943 || n = 945 || n = 948 || n = 950 || n = 958 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 980 || n = 983 || n = 985 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1063 || n = 1067 || n = 1068 || n = 1070 || n = 1073 || n = 1075 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1088 || n = 1090 || n = 1098 || n = 1102 || n = 1103 || n = 1105 || n = 1108 || n = 1110 || n = 1113 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1133 || n = 1137 || n = 1138 || n = 1140 || n = 1143 || n = 1145 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1158 || n = 1160 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1183 || n = 1185 || n = 1188 || n = 1190 || n = 1193 || n = 1195 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1225 || n = 1228 || n = 1230 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248 || n = 1250)
	; 6..7 bytes
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
.elseif(n = 942 || n = 947 || n = 952 || n = 957 || n = 987 || n = 992 || n = 1012 || n = 1020 || n = 1022 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1082 || n = 1087 || n = 1092 || n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1152 || n = 1157 || n = 1167 || n = 1182 || n = 1192 || n = 1197 || n = 1202 || n = 1222 || n = 1227 || n = 1232)
	; 6..7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1089 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1124 || n = 1129 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1209 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244 || n = 1249)
	; 6..7 bytes
	delay_n (n-3)
	ldx $A6
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
	sta @zptemp
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
.elseif(n = 1019)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
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
.elseif(n = 1097 || n = 1217)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
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
.elseif(n = 1187)
	; 7 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
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
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1263 || n = 1265 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1290 || n = 1291 || n = 1293 || n = 1298 || n = 1300 || n = 1305 || n = 1307 || n = 1308 || n = 1312 || n = 1314 || n = 1319 || n = 1323 || n = 1326 || n = 1328)
	; 6..8 bytes
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
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1292 || n = 1299 || n = 1318 || n = 1320 || n = 1327)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1287 || n = 1295 || n = 1302 || n = 1316)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.elseif(n = 1288 || n = 1304)
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
.elseif(n = 1301 || n = 1311)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
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
.elseif(n = 1322)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1325)
	; 9 bytes
	delay_n 124
	delay_n (n-124)
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1337 || n = 1353 || n = 1377 || n = 1385)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 1330)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jmi *-4
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
.elseif(n = 1334 || n = 1348 || n = 1355 || n = 1362 || n = 1379 || n = 1390 || n = 1397 || n = 1402)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1335 || n = 1398)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1336 || n = 1350 || n = 1364 || n = 1392 || n = 1404)
	; 7..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1339 || n = 1340 || n = 1344 || n = 1346 || n = 1347 || n = 1349 || n = 1354 || n = 1356 || n = 1361 || n = 1368 || n = 1370 || n = 1375 || n = 1382 || n = 1384 || n = 1389 || n = 1395 || n = 1403)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1341 || n = 1371 || n = 1391)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1357)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 14 = 0) && ((n > 1357 && n < 1401)))
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1363)
	; 7 bytes
	ldx #248 ;hides 'sed'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1365)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1367)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
	Jpl *-5
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
.elseif(n = 1381 || n = 1393)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1383 || n = 1406)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1388)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.endif
.endif
.else
; 1407 to 1562
.if(n = 1407 || n = 1410 || n = 1412 || n = 1413 || n = 1416 || n = 1417 || n = 1419 || n = 1424 || n = 1431 || n = 1434 || n = 1435 || n = 1438 || n = 1440 || n = 1445 || n = 1447 || n = 1452 || n = 1454 || n = 1463 || n = 1466 || n = 1470 || n = 1473 || n = 1475 || n = 1479 || n = 1480 || n = 1482 || n = 1483 || n = 1487 || n = 1493 || n = 1494 || n = 1496 || n = 1501 || n = 1503 || n = 1508 || n = 1510 || n = 1515 || n = 1529 || n = 1533 || n = 1535 || n = 1536 || n = 1538 || n = 1539 || n = 1542 || n = 1543 || n = 1547 || n = 1550 || n = 1552 || n = 1557 || n = 1559 || n = 1560)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1409 || n = 1425 || n = 1433 || n = 1449 || n = 1481 || n = 1505 || n = 1521 || n = 1545 || n = 1553)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 1411 || n = 1421 || n = 1451 || n = 1461 || n = 1491 || n = 1551)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442) || (n > 1458 && n < 1505) || (n > 1521 && n < 1559)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1418 || n = 1439 || n = 1444 || n = 1446 || n = 1458 || n = 1460 || n = 1467 || n = 1474 || n = 1488 || n = 1500 || n = 1502 || n = 1509 || n = 1514 || n = 1523 || n = 1530 || n = 1544)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1426 || n = 1428 || n = 1456 || n = 1484 || n = 1498 || n = 1512 || n = 1517 || n = 1526)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1427 || n = 1430 || n = 1518 || n = 1556)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1437)
	; 9 bytes
	ldx #(n-7)/10
	inc @zptemp
	dex
	Jne *-3
	inc @zptemp,x
.elseif(n = 1442)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1448 || n = 1462 || n = 1469 || n = 1476 || n = 1490 || n = 1516 || n = 1532 || n = 1546)
	; 7..8 bytes
	delay_n (n-5)
	inc @zptemp
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
.elseif(n = 1472 || n = 1528)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1489 || n = 1525 || n = 1537 || n = 1561)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1497)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jmi *-4
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
.elseif(n = 1519)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
.elseif(n = 1524)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1554)
	; 8 bytes
	pha
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1574 || n = 1596)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1564 || n = 1566 || n = 1571 || n = 1578 || n = 1580 || n = 1592 || n = 1595 || n = 1599 || n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1615 || n = 1619 || n = 1620 || n = 1622 || n = 1627 || n = 1634 || n = 1636 || n = 1641 || n = 1647 || n = 1655 || n = 1659 || n = 1662 || n = 1664 || n = 1668 || n = 1669 || n = 1676 || n = 1678 || n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1692 || n = 1697 || n = 1699 || n = 1703 || n = 1704 || n = 1706 || n = 1707 || n = 1715 || n = 1718)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1570 || n = 1572 || n = 1579 || n = 1586 || n = 1600 || n = 1607 || n = 1614 || n = 1626 || n = 1635 || n = 1640 || n = 1656 || n = 1663 || n = 1670 || n = 1677 || n = 1698 || n = 1712)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1584 && n < 1622) || (n > 1647 && n < 1694) || n = 1711))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1568 || n = 1582 || n = 1589 || n = 1624 || n = 1629 || n = 1638 || n = 1643 || n = 1652 || n = 1680 || n = 1687 || n = 1694 || n = 1708)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1717)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1573 || n = 1609 || n = 1633 || n = 1645 || n = 1705 || n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1575)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1577 || n = 1593 || n = 1601 || n = 1617 || n = 1649 || n = 1665 || n = 1673 || n = 1689 || n = 1713)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 1581 || n = 1591 || n = 1631 || n = 1651 || n = 1661 || n = 1671 || n = 1691 || n = 1701)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1584 || n = 1696)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1587 || n = 1598 || n = 1650)
	; 8..9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1588 || n = 1602 || n = 1616 || n = 1628 || n = 1642 || n = 1658 || n = 1672 || n = 1679 || n = 1700 || n = 1714)
	; 7..8 bytes
	delay_n (n-5)
	inc @zptemp
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
.elseif(n = 1630 || n = 1644)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1654)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1710)
	; 9 bytes
	ldx #213
	sta @zptemp
	dex
	Jne *-3
	inc @zptemp
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1726 || n = 1733 || n = 1754 || n = 1768 || n = 1780 || n = 1782 || n = 1796)
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
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 1722 || n = 1725 || n = 1727 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1748 || n = 1749 || n = 1753 || n = 1755 || n = 1760 || n = 1762 || n = 1763 || n = 1767 || n = 1773 || n = 1781 || n = 1787 || n = 1788 || n = 1790 || n = 1795)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
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
.elseif(n = 1728 || n = 1750 || n = 1794)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1731 || n = 1771 || n = 1791)
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
.elseif(n = 1736 || n = 1757 || n = 1764 || n = 1778)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1740 || n = 1766 || n = 1770 || n = 1784)
	; 7..9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1741 || n = 1777 || n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1752)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1759)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1775)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1776)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1793)
	; 6 bytes
	ldx #0
	nop
	dex
	Jne *-2
.endif
.else
; 1797 to 1875
.if(n = 1797 || n = 1803 || n = 1812 || n = 1814 || n = 1815 || n = 1830 || n = 1843 || n = 1845 || n = 1848 || n = 1850 || n = 1853 || n = 1859 || n = 1866 || n = 1867 || n = 1869 || n = 1875)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1798 || n = 1804 || n = 1870)
	; 8..9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1800 || n = 1806 || n = 1808 || n = 1862)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 9 = 1) && ((n > 1800 && n < 1874)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1802 || n = 1822 || n = 1829 || n = 1836 || n = 1852 || n = 1858 || n = 1874)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 11 = 1) && ((n > 1804 && n < 1839) || n = 1860))
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
.elseif(n = 1809 || n = 1817 || n = 1833 || n = 1841 || n = 1857 || n = 1865)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 1811 || n = 1821 || n = 1831 || n = 1851 || n = 1871)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1813 || n = 1825 || n = 1849 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1818)
	; 9 bytes
	ldx #234 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jmi *-6
.elseif(n = 1820 || n = 1854)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
.elseif(n = 1832)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1834 || n = 1847)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 1835 || n = 1863)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
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
.elseif(n = 1840 || n = 1856 || n = 1872)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1842)
	; 8 bytes
	pha
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 1844)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 1868)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
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
.elseif(n = 1878 || n = 1880 || n = 1892 || n = 1898 || n = 1908 || n = 1916 || n = 1925 || n = 1934 || n = 1940 || n = 1950 || n = 1952)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1879 || n = 1883 || n = 1884 || n = 1886 || n = 1887 || n = 1899 || n = 1902 || n = 1903 || n = 1907 || n = 1917 || n = 1920 || n = 1935 || n = 1938 || n = 1939 || n = 1947)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1881 || n = 1889 || n = 1905 || n = 1913 || n = 1929 || n = 1937 || n = 1953)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1946)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1885 || n = 1897 || n = 1921 || n = 1933)
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
.elseif(n = 1890 || n = 1910)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif((n .mod 11 = 1) && ((n > 1892 && n < 1927) || n = 1948))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1894 || n = 1924 || n = 1930 || n = 1946)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1895)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 16 = 8) && ((n > 1895 && n < 1945)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1901 || n = 1911 || n = 1931 || n = 1941 || n = 1951)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1914)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
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
.elseif(n = 1932)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
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
.elseif(n = 1949)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.endif
.else
; 1954 to 2031
.if((n .mod 9 = 1) && ((n > 1953 && n < 2027)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1955 || n = 1956 || n = 1965 || n = 1973 || n = 1974 || n = 1979 || n = 1983 || n = 1987 || n = 1995 || n = 2007 || n = 2010 || n = 2013 || n = 2019 || n = 2027 || n = 2028)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1957 || n = 1969 || n = 1993 || n = 2005 || n = 2029)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1958 || n = 1996)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 1959 || n = 1970 || n = 1992 || n = 2003 || n = 2014)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1960 || n = 1976 || n = 1984 || n = 2000 || n = 2016)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1961 || n = 1977 || n = 1985 || n = 2001 || n = 2009 || n = 2025)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 1962 || n = 1982 || n = 2022)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1964 || n = 1978 || n = 1997 || n = 1998 || n = 2006 || n = 2012 || n = 2018)
	; 9 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1971 || n = 1991 || n = 2011 || n = 2021 || n = 2031)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1975 || n = 1989)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1980 || n = 2002 || n = 2004 || n = 2020)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1994)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2024)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 2030)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
; 2032 to 2109
.if(n = 2032)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
.elseif(n = 2033)
	; 7 bytes
	ldx #(n-1)/8
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 2034 || n = 2072)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2108)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 11 = 1) && ((n > 2035 && n < 2070) || n = 2102))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2037 || n = 2039 || n = 2043 || n = 2046 || n = 2055 || n = 2057 || n = 2063 || n = 2064 || n = 2066 || n = 2067 || n = 2073 || n = 2079 || n = 2082 || n = 2083 || n = 2093 || n = 2095 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2038 || n = 2052 || n = 2054 || n = 2074 || n = 2090 || n = 2108)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2040 || n = 2048)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2041 || n = 2065 || n = 2077 || n = 2101)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2042 || n = 2060 || n = 2085 || n = 2088 || n = 2094 || n = 2096)
	; 8..9 bytes
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
	sta @zptemp
	dex
	Jne *-3
.elseif(n = 2050 || n = 2084)
	; 8..9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2051 || n = 2061 || n = 2081 || n = 2091)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
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
.elseif(n = 2068)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2070 || n = 2076 || n = 2092)
	; 8..9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2075)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
.elseif(n = 2097)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2099)
	; 9 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
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
	ldx #(n-6)/10
	inc @zptemp
	dex
	Jne *-3
	inc @zptemp
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2142)
	; 8..9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif((n .mod 10 = 1) && ((n > 2110 && n < 2152) || n = 2171 || n = 2181))
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2112 || n = 2114 || n = 2120 || n = 2132 || n = 2144 || n = 2150 || n = 2156 || n = 2158 || n = 2178 || n = 2180)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2113 || n = 2137 || n = 2149 || n = 2173 || n = 2185)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2115 || n = 2129)
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
.elseif(n = 2117)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2118 || n = 2123 || n = 2127 || n = 2133 || n = 2136 || n = 2138 || n = 2139 || n = 2145 || n = 2153 || n = 2154 || n = 2163 || n = 2165 || n = 2169 || n = 2172 || n = 2174 || n = 2175 || n = 2183 || n = 2187)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2119 || n = 2128 || n = 2140 || n = 2155 || n = 2162 || n = 2164 || n = 2182 || n = 2184)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2122)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 11 = 1) && ((n > 2123 && n < 2169)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2126 || n = 2160 || n = 2177)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
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
.elseif(n = 2147)
	; 8 bytes
	ldx #232 ;hides 'inx'
	sta @zptemp
	inx
	inx
	Jne *-5
.elseif(n = 2148)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2159)
	; 7 bytes
	ldx #216 ;hides 'cld'
	sta @zptemp
	dex
	Jne *-4
.elseif(n = 2166)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2167 || n = 2186)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2176)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if((n .mod 9 = 1) && ((n > 2187 && n < 2261)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2189 || n = 2193 || n = 2203 || n = 2208 || n = 2210 || n = 2214 || n = 2217 || n = 2219 || n = 2225 || n = 2226 || n = 2229 || n = 2235 || n = 2243 || n = 2244 || n = 2247 || n = 2253 || n = 2259 || n = 2262 || n = 2263)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2190 || n = 2212 || n = 2223 || n = 2234 || n = 2256)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2191 || n = 2201 || n = 2211 || n = 2241 || n = 2261)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2192 || n = 2195 || n = 2204 || n = 2216 || n = 2222 || (n > 2237 && n < 2241) || n = 2249 || n = 2252 || n = 2258 || n = 2264)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2194)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 2196)
	; 9 bytes
	pha
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
.elseif(n = 2198 || n = 2250)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2199)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2200 || n = 2202 || n = 2218 || n = 2254)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2205)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2207)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2209 || n = 2221 || n = 2245 || n = 2257)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2213 || n = 2227 || n = 2255)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2220 || n = 2236)
	; 8..9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2228)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.elseif(n = 2237)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2246)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
.elseif(n = 2248)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	inx
	Jne *-4
	plp
.elseif(n = 2265)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	inx
	Jne *-4
	plp
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
.elseif(n = 2267 || n = 2289 || n = 2300)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2268 || n = 2276 || n = 2285 || n = 2288 || n = 2294)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 9 = 1) && ((n > 2268 && n < 2297)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2270 || n = 2272 || n = 2290 || n = 2299 || n = 2304)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2271 || n = 2291 || n = 2301)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2273 || n = 2280 || n = 2282 || n = 2295 || n = 2298 || n = 2303)
	; 8..9 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2277)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2279)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
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
.elseif(n = 2286)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2292)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	inx
	Jne *-4
	plp
.elseif(n = 2302)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	inx
	Jne *-4
	plp
.endif
.else
; 2305 to 2343
.if(n = 2305)
	; 7 bytes
	ldx #0
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2306 || n = 2316)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2307 || n = 2309 || n = 2313 || n = 2323 || n = 2327 || n = 2335 || n = 2340 || n = 2343)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2314 || n = 2320 || n = 2332 || n = 2334 || n = 2342)
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
.elseif(n = 2311 || n = 2321 || n = 2331)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2312 || n = 2318 || n = 2324 || n = 2336)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2315 || n = 2328)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2317 || n = 2329 || n = 2341)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2319)
	; 7 bytes
	ldx #24 ;hides 'clc'
	sta @zptemp
	inx
	Jne *-4
.elseif(n = 2322 || n = 2333)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
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
.elseif(n = 2330)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2337)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2338)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2339)
	; 7 bytes
	ldx #234 ;hides 'nop'
	sta @zptemp
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if(n = 2344 || n = 2355 || n = 2366 || n = 2388 || n = 2410)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2345 || n = 2350 || n = 2363 || n = 2368 || n = 2373 || n = 2379 || n = 2383 || n = 2390 || n = 2393 || n = 2397 || n = 2400 || n = 2403 || n = 2405 || n = 2412 || n = 2415 || n = 2417)
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
.elseif(n = 2347 || n = 2364 || n = 2398)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2348 || n = 2358 || n = 2360 || n = 2362 || n = 2372 || n = 2382 || n = 2384 || n = 2396 || n = 2408 || n = 2418 || n = 2420)
	; 9..10 bytes
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
.elseif((n .mod 10 = 1) && ((n > 2350 && n < 2392) || n = 2411 || n = 2421))
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2352 || n = 2370)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif((n .mod 12 = 1) && ((n > 2352 && n < 2414)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2354 || n = 2380 || n = 2406 || n = 2419)
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
.elseif(n = 2357 || n = 2376 || n = 2414)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2359 || n = 2387)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2367 || n = 2395 || n = 2409)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2369)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2374 || n = 2378 || n = 2392 || n = 2394 || n = 2402 || n = 2404)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2375)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2385)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2399)
	; 7 bytes
	ldx #56 ;hides 'sec'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2407)
	; 9 bytes
	ldx #(n-7)/10
	inc @zptemp
	dex
	Jne *-3
	inc @zptemp,x
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2427 || n = 2429 || n = 2433 || n = 2434 || n = 2439 || n = 2453 || n = 2455 || n = 2459 || n = 2463 || n = 2467 || n = 2469 || n = 2470 || n = 2475 || n = 2477 || n = 2478 || n = 2483 || n = 2489 || n = 2495 || n = 2499)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423 || n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2424 || n = 2428 || n = 2438 || n = 2464 || n = 2474 || n = 2482 || n = 2488 || n = 2494)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 12 = 1) && ((n > 2424 && n < 2498)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2426 || n = 2430 || n = 2436 || n = 2440 || n = 2496)
	; 8..9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2431 || n = 2441 || n = 2451 || n = 2471 || n = 2481 || n = 2491)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif((n .mod 11 = 1) && ((n > 2431 && n < 2499)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2442 || n = 2444 || n = 2448 || n = 2456 || n = 2468 || n = 2472 || n = 2480 || n = 2492)
	; 8..10 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 2450)
	; 9 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2452)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2457)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2460)
	; 9 bytes
	ldx #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jmi *-6
.elseif(n = 2462)
	; 10 bytes
	ldx #106
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jpl *-6
.elseif(n = 2466 || n = 2500)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2479)
	; 7 bytes
	ldx #248 ;hides 'sed'
	sta @zptemp
	dex
	Jne *-4
.elseif(n = 2486)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2490)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
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
; 2501 to 2578
.if(n = 2501 || n = 2511 || n = 2531 || n = 2541)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2502 || n = 2546 || n = 2550 || n = 2556 || n = 2574 || n = 2578)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2503 || n = 2513 || n = 2515 || n = 2519 || n = 2522 || n = 2523 || n = 2525 || n = 2530 || n = 2532 || n = 2543 || n = 2544 || n = 2547 || n = 2555 || n = 2559 || n = 2565 || n = 2570 || n = 2571)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2508 || n = 2516 || n = 2518 || n = 2538 || n = 2552 || n = 2558 || n = 2576)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2505)
	; 8 bytes
	ldx #232 ;hides 'inx'
	inc @zptemp
	inx
	inx
	Jne *-5
.elseif(n = 2506 || n = 2536)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && (n = 2507 || (n > 2534 && n < 2578)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 12 = 1) && ((n > 2508 && n < 2570)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2510 || n = 2562)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2512 || n = 2514 || n = 2524 || n = 2548 || n = 2554 || n = 2560 || n = 2572)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2517 || n = 2534 || n = 2568)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && (n = 2520 || (n > 2541 && n < 2576)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2526)
	; 9 bytes
	ldx #(n-6)/10
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
.elseif(n = 2528 || n = 2566)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2529)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2537)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2539 || n = 2567)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2540)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 2551)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2561)
	; 7 bytes
	ldx #0
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2573)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.endif
.else
; 2579 to 2656
.if(n = 2579 || n = 2583 || n = 2592 || n = 2595 || n = 2606 || n = 2607 || n = 2609 || n = 2610 || n = 2613 || n = 2616 || n = 2628 || n = 2631 || n = 2632 || n = 2635 || n = 2643 || n = 2645 || (n > 2648 && n < 2652) || n = 2654 || n = 2655)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2580 || n = 2588 || n = 2598 || n = 2600 || n = 2612 || n = 2615 || n = 2618 || n = 2624 || n = 2637 || n = 2648)
	; 9..11 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 12 = 1) && ((n > 2580 && n < 2654)))
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
.elseif(n = 2584 || n = 2589 || n = 2594 || n = 2620 || n = 2644)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2585 || n = 2602 || n = 2636)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2585 && n < 2631) || n = 2652))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2587 || n = 2639)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2590)
	; 9 bytes
	pha
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
.elseif(n = 2591)
	; 7 bytes
	ldx #216 ;hides 'cld'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2596 || n = 2611 || n = 2626 || n = 2656)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2601 || n = 2621)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2603)
	; 9 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
.elseif(n = 2604 || n = 2623 || n = 2642)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2614 || n = 2627 || n = 2640)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2622 || n = 2634 || n = 2646)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2625)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
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
.endif
.endif
.else
; 2657 to 2812
.if(n = 2657 || n = 2662 || n = 2667 || n = 2669 || n = 2676 || n = 2678 || n = 2679 || n = 2688 || n = 2691 || n = 2693 || n = 2697 || n = 2698 || n = 2700 || n = 2709 || n = 2711 || n = 2715 || n = 2719 || n = 2722 || n = 2723 || n = 2733 || n = 2735 || n = 2739 || n = 2742 || n = 2747 || n = 2748 || n = 2753 || n = 2758 || (n > 2762 && n < 2766) || n = 2774 || n = 2775 || n = 2777 || n = 2786 || n = 2793 || n = 2803 || n = 2808 || n = 2811)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2658 || n = 2682 || n = 2694 || n = 2706 || n = 2712 || n = 2750 || n = 2754 || n = 2760 || n = 2766 || n = 2778)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2659 || n = 2683 || n = 2695 || n = 2702 || n = 2743 || n = 2767 || n = 2779 || n = 2805)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2660 || n = 2672 || n = 2684 || n = 2708 || n = 2714 || n = 2720 || n = 2724 || n = 2732 || n = 2734 || n = 2736 || n = 2744 || n = 2768 || n = 2780 || n = 2790 || n = 2792 || n = 2804)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 14 = 1) && (n = 2661 || n = 2675 || (n > 2702 && n < 2760) || n = 2787 || n = 2801))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2662 && n < 2763) || (n > 2783 && n < 2807)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2664 || n = 2668 || n = 2728 || n = 2730 || n = 2752 || n = 2788 || n = 2798 || n = 2800 || n = 2802 || n = 2810 || n = 2812)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 12 = 1) && ((n > 2664 && n < 2810)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2666 || n = 2692 || n = 2705 || n = 2757 || n = 2770 || n = 2796)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2670 || n = 2687 || n = 2704 || n = 2721 || n = 2738 || n = 2755 || n = 2772 || n = 2789)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2671 || n = 2686 || n = 2716 || n = 2746 || n = 2776 || n = 2791)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2673 || n = 2769)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2680 || n = 2699 || n = 2756 || n = 2794)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2681 || n = 2741 || n = 2781)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2690)
	; 9 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
.elseif(n = 2710)
	; 9 bytes
	ldx #128
	nop
	jsr @nearby_rts_14cyc
	dex
	Jpl *-5
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
.elseif(n = 2771)
	; 8 bytes
	ldx #232 ;hides 'inx'
	sta @zptemp
	dex
	dex
	Jne *-5
.elseif(n = 2782)
	; 9 bytes
	ldx #248 ;hides 'sed'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jmi *-6
.elseif(n = 2783)
	; 7 bytes
	ldx #24 ;hides 'clc'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2799)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2807)
	; 7 bytes
	ldx #234 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
; 2813 to 2968
.if(n = 2813 || n = 2832 || n = 2851 || n = 2870 || n = 2889 || n = 2946)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2814 || n = 2816 || n = 2828 || n = 2848 || n = 2852 || n = 2858 || n = 2864 || n = 2876 || n = 2888 || n = 2892 || n = 2900 || n = 2906 || n = 2912 || n = 2924 || n = 2936 || n = 2948 || n = 2960 || n = 2966)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 14 = 1) && ((n > 2814 && n < 2844) || (n > 2870 && n < 2928) || n = 2955))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2817)
	; 8 bytes
	ldx #0
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2818 || n = 2819 || n = 2823 || n = 2825 || n = 2830 || n = 2831 || n = 2834 || n = 2835 || n = 2837 || n = 2842 || n = 2847 || n = 2849 || (n > 2852 && n < 2856) || n = 2859 || n = 2867 || n = 2868 || n = 2883 || n = 2890 || n = 2895 || n = 2897 || n = 2902 || n = 2903 || n = 2907 || n = 2909 || n = 2914 || n = 2915 || n = 2919 || n = 2928 || n = 2931 || n = 2933 || n = 2937 || n = 2938 || n = 2943 || n = 2945 || n = 2950 || n = 2951 || n = 2954 || n = 2963 || n = 2967)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2820 || n = 2824 || n = 2844 || n = 2860 || n = 2872 || n = 2884 || n = 2894 || n = 2916 || n = 2920 || n = 2930 || n = 2932 || n = 2944 || n = 2968)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 12 = 1) && ((n > 2820 && n < 2966)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2822 || n = 2887 || n = 2939 || n = 2952)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2826 || n = 2838 || n = 2846 || n = 2850 || n = 2862 || n = 2879 || n = 2886 || n = 2898 || n = 2904 || n = 2910 || n = 2922 || n = 2934 || n = 2958 || n = 2964)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2827 || n = 2839 || n = 2875 || n = 2877 || n = 2923 || n = 2935 || n = 2947)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2836 || n = 2866 || n = 2896 || n = 2911 || n = 2926 || n = 2956)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2840 || n = 2874 || n = 2891 || n = 2908 || n = 2925 || n = 2942 || n = 2959)
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
.elseif(n = 2856)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2861 || n = 2901 || n = 2921 || n = 2961)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
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
.elseif(n = 2873 || n = 2957)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2878 || n = 2962)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 2880)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
	plp
.elseif(n = 2882)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 2940)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2949)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n .mod 14 = 1) && ((n > 2968 && n < 3012) || n = 3039))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2970 || n = 2994 || n = 3006 || n = 3018 || n = 3042)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2971 || n = 2986 || n = 3016 || n = 3031 || n = 3046)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2972 || n = 2982 || n = 2990 || n = 2996 || n = 3008 || n = 3020 || n = 3030 || n = 3032)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2973 || n = 2974 || n = 2979 || n = 2985 || n = 2987 || n = 2988 || n = 2991 || n = 2998 || n = 3003 || n = 3005 || n = 3012 || n = 3024 || n = 3027 || n = 3029 || n = 3033 || n = 3034 || n = 3045)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2975)
	; 7 bytes
	ldx #248 ;hides 'sed'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2976 || n = 2993 || n = 3010)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif((n .mod 12 = 1) && ((n > 2976 && n < 3038)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2978 || n = 3004 || n = 3017 || n = 3043)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2980 || n = 2992 || n = 3000 || n = 3014 || n = 3026 || n = 3028 || n = 3040)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2981 || n = 3021)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2984 || n = 3022)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2995 || n = 3007 || n = 3019)
	; 9 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2999)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3002)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 3009)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3015)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3023)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3035)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3036)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3038)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3041)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3044)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
.endif
.else
; 3047 to 3125
.if(n = 3047 || n = 3052 || n = 3064 || n = 3104 || n = 3120)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3048 || n = 3056 || n = 3068 || n = 3074 || n = 3080 || n = 3096 || n = 3102 || n = 3116 || n = 3124)
	; 9..11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3049 || n = 3061)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3050 || n = 3087 || n = 3118)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3051 || n = 3055 || n = 3058 || n = 3062 || n = 3063 || n = 3065 || n = 3070 || n = 3071 || n = 3075 || n = 3077 || (n > 3082 && n < 3086) || n = 3090 || n = 3093 || n = 3097 || n = 3099 || n = 3100 || n = 3107 || n = 3110 || n = 3111 || n = 3113 || n = 3114 || n = 3119 || n = 3125)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 1) && ((n > 3052 && n < 3124)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3054 || n = 3066 || n = 3072 || n = 3086 || n = 3122)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3057 || n = 3089 || n = 3105)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3059 || n = 3103)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3060 || n = 3079 || n = 3098 || n = 3117)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3069 || n = 3082 || n = 3108)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3076 || n = 3091 || n = 3106)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3078 || n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3088)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3092 || n = 3094)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3101 || n = 3121)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3115)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if(n = 3126 || n = 3132 || n = 3140 || n = 3154 || n = 3164 || n = 3177 || n = 3202 || n = 3210 || n = 3215 || n = 3224 || n = 3234 || n = 3236 || n = 3244 || n = 3278)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3127 || n = 3131 || n = 3138 || n = 3139 || n = 3143 || (n > 3147 && n < 3151) || n = 3157 || n = 3159 || n = 3162 || n = 3167 || n = 3168 || n = 3171 || n = 3175 || n = 3176 || n = 3178 || n = 3182 || n = 3183 || n = 3188 || n = 3195 || n = 3198 || n = 3203 || n = 3209 || n = 3216 || n = 3218 || n = 3219 || n = 3227 || n = 3237 || n = 3240 || n = 3243 || n = 3245 || n = 3258 || n = 3266 || n = 3267 || n = 3273 || n = 3279)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3128 || n = 3142 || n = 3156 || n = 3170 || n = 3184 || n = 3204 || n = 3246 || n = 3252 || n = 3268 || n = 3274 || n = 3280)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3129 || n = 3146 || n = 3163 || n = 3180 || n = 3197 || n = 3214 || n = 3231 || n = 3248 || n = 3265)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3130 || n = 3172)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3133 || n = 3187 || n = 3205 || n = 3259)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif((n .mod 13 = 1) && ((n > 3133 && n < 3187) || (n > 3224 && n < 3265)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3135 || n = 3152 || n = 3220 || n = 3229 || n = 3253 || n = 3255)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3136 || n = 3155 || n = 3174 || n = 3212 || n = 3250 || n = 3269)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && ((n > 3136 && n < 3278)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3141 || n = 3161 || n = 3181 || n = 3201 || n = 3241 || n = 3261 || n = 3281)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3144 || n = 3158 || n = 3192 || n = 3200 || n = 3206 || n = 3208 || n = 3228 || n = 3230 || n = 3232 || n = 3242 || n = 3254 || n = 3270 || n = 3272 || n = 3276)
	; 9..11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3145)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 3153 || n = 3169 || n = 3185 || n = 3217)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3166 || n = 3196 || n = 3211 || n = 3226 || n = 3256 || n = 3271)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3189 || n = 3190 || n = 3194 || n = 3222 || n = 3260)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3191 || n = 3213 || n = 3257)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3223)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3233)
	; 8 bytes
	ldx #232 ;hides 'inx'
	inc @zptemp
	dex
	dex
	Jne *-5
.elseif(n = 3239)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3247)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3262)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.endif
.else
.if((n >= 3282) && (n < 3360))
; 3282 to 3359
.if(n = 3282 || n = 3299 || n = 3316 || n = 3350)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283 || n = 3284 || n = 3293 || n = 3295 || n = 3300 || n = 3309 || n = 3315 || n = 3318 || n = 3320 || n = 3328 || n = 3330 || n = 3335 || n = 3337 || n = 3342 || n = 3343 || n = 3348 || n = 3349 || n = 3351 || n = 3352)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285 || n = 3294 || n = 3308 || n = 3336 || n = 3344)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3286 || n = 3331 || n = 3346)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3287)
	; 10 bytes
	pha
	ldx #37 ;hides 'and zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	pla
.elseif(n = 3288 || n = 3307 || n = 3326 || n = 3345)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3289 || n = 3306 || n = 3312 || n = 3314 || n = 3354 || n = 3357)
	; 10 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3296 || n = 3304 || n = 3310 || n = 3324 || n = 3338 || n = 3355)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3297 || n = 3313)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3298 || n = 3340)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3301 || n = 3321 || n = 3341)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3302)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 3311 || n = 3325 || n = 3327 || n = 3332 || n = 3339 || n = 3353 || n = 3356)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3317 || n = 3358)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3322)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3323)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3329)
	; 9 bytes
	ldx #0
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3334)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 3359)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.endif
.else
; 3360 to 3437
.if(n = 3360 || n = 3362 || n = 3397 || n = 3415)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 14 = 1) && ((n > 3360 && n < 3432)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3363 || n = 3365 || n = 3366 || n = 3369 || n = 3371 || n = 3378 || n = 3385 || n = 3386 || n = 3388 || n = 3395 || n = 3404 || n = 3405 || n = 3407 || n = 3408 || n = 3413 || n = 3419 || n = 3420 || n = 3426 || n = 3429 || n = 3433 || n = 3437)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3364 || n = 3383 || n = 3402 || n = 3421)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3367 || n = 3384 || n = 3401 || n = 3418 || n = 3435)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3368 || n = 3374 || n = 3396 || n = 3410 || n = 3416 || n = 3432)
	; 10..11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3370 || n = 3387 || n = 3392 || n = 3434)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3372 || n = 3380 || n = 3394 || n = 3422)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3373 || n = 3390)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3376 || n = 3391 || n = 3406 || n = 3436)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif((n .mod 16 = 1) && ((n > 3376 && n < 3426)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3379)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3381)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3382 || n = 3424)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3398)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3399)
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
.elseif(n = 3411)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3412)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3414)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3423)
	; 9 bytes
	ldx #(n-3)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	nop
.elseif(n = 3427)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3428)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3430)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if(n = 3438 || n = 3462 || n = 3485 || n = 3494 || n = 3510 || n = 3512)
	; 10..11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3439 || n = 3493)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3440 || n = 3478 || n = 3497)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3441 || n = 3461 || n = 3481)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif((n > 3441 && n < 3445) || n = 3447 || n = 3449 || n = 3454 || n = 3463 || n = 3468 || n = 3475 || n = 3480 || n = 3482 || n = 3483 || n = 3488 || n = 3498 || n = 3499 || n = 3507 || n = 3513)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 1) && ((n > 3444 && n < 3516)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3446 || n = 3460 || n = 3476 || n = 3492 || n = 3506)
	; 9..10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3448 || n = 3453 || n = 3458 || n = 3472 || n = 3474 || n = 3484 || n = 3490 || n = 3500 || n = 3504)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3450)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 3451 || n = 3466 || n = 3496 || n = 3511)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3452 || n = 3469 || n = 3486 || n = 3503)
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
.elseif(n = 3456 || n = 3465 || n = 3467 || n = 3477)
	; 9..10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3457 || n = 3489 || n = 3505)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3464)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
.elseif(n = 3470 || n = 3495 || n = 3514)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3471)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3479)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3491)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3502)
	; 10 bytes
	php
	ldx #184 ;hides 'clv'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 3508)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3509)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	dex
	Jne *-6
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3535 || n = 3573 || n = 3592)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif((n > 3516 && n < 3520) || (n > 3521 && n < 3525) || n = 3528 || n = 3531 || n = 3533 || n = 3539 || n = 3545 || n = 3547 || n = 3552 || n = 3555 || n = 3558 || n = 3559 || n = 3563 || n = 3567 || n = 3575 || n = 3583 || n = 3587 || n = 3589 || n = 3590)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3520 || n = 3537 || n = 3554 || n = 3588)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3521 || n = 3541 || n = 3561 || n = 3581)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3525 || n = 3534 || n = 3542 || n = 3548 || n = 3562 || n = 3576 || n = 3593)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3526 || n = 3556 || n = 3586)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3527 || n = 3536 || n = 3544 || n = 3564 || n = 3568 || n = 3578 || n = 3580)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 14 = 1) && ((n > 3528 && n < 3572)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3530 || n = 3570 || n = 3572 || n = 3582)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3532 || n = 3538 || n = 3540 || n = 3546 || n = 3560 || n = 3574 || n = 3584)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3549 || n = 3577 || n = 3591)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3550)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3551)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 3553 || n = 3569)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3565)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3566)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3579)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.endif
.endif
.else
; 3594 to 3750
.if(n = 3594 || n = 3596 || n = 3603 || n = 3607 || n = 3618 || n = 3623 || n = 3624 || n = 3626 || n = 3632 || n = 3635 || n = 3636 || n = 3643 || n = 3648 || n = 3651 || n = 3657 || n = 3658 || n = 3660 || n = 3663 || n = 3667 || n = 3670 || n = 3672 || n = 3675 || n = 3677 || n = 3678 || n = 3683 || n = 3685 || n = 3689 || (n > 3691 && n < 3695) || n = 3699 || n = 3703 || n = 3705 || n = 3708 || n = 3709 || n = 3715 || n = 3720 || n = 3723 || (n > 3725 && n < 3729) || n = 3731 || n = 3738 || n = 3746 || n = 3747)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3612 || n = 3629 || n = 3638 || n = 3640 || n = 3680 || n = 3688 || n = 3702 || n = 3732 || n = 3750)
	; 10..11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3597 || n = 3606 || n = 3610 || n = 3627 || n = 3644 || n = 3654 || n = 3666 || n = 3686 || n = 3700 || n = 3712 || n = 3716 || n = 3748)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3598 || n = 3628 || n = 3645 || n = 3647 || n = 3662 || n = 3674 || n = 3679 || n = 3696 || n = 3717 || n = 3730 || n = 3749)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3599)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 3600 || n = 3602 || n = 3620 || n = 3650 || n = 3669 || n = 3682 || n = 3722 || (n > 3732 && n < 3736) || n = 3737 || n = 3740 || n = 3742)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3601 || n = 3621 || n = 3641 || n = 3661 || n = 3681 || n = 3701 || n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3604 || n = 3608 || n = 3614 || n = 3625 || n = 3642 || n = 3652 || n = 3659 || n = 3664 || n = 3684 || n = 3698 || n = 3710 || n = 3714)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3605 || n = 3622 || n = 3639 || n = 3656 || n = 3673 || n = 3690 || n = 3707 || n = 3724 || n = 3741)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3609)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3611 || n = 3630 || n = 3649 || n = 3668 || n = 3687 || n = 3706 || n = 3725 || n = 3744)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3613 || n = 3634 || n = 3655 || n = 3718 || n = 3739)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3615)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3616 || n = 3631 || n = 3646 || n = 3676 || n = 3691 || n = 3736)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif((n .mod 16 = 1) && (n = 3617 || n = 3633 || n = 3665 || (n > 3696 && n < 3746)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3619 || n = 3637)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3653)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
.elseif(n = 3671)
	; 9 bytes
	ldx #216 ;hides 'cld'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 3695)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 3704)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3711)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3719)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3743)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
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
.if(n = 3751 || n = 3766 || n = 3796 || n = 3811)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3752 || n = 3770 || n = 3788 || n = 3789 || n = 3800 || n = 3806 || n = 3808 || n = 3816 || n = 3818)
	; 10..11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3753 || n = 3760 || n = 3762 || n = 3765 || n = 3768 || n = 3779 || n = 3783 || n = 3784 || n = 3786 || n = 3794 || n = 3795 || n = 3798 || n = 3803 || n = 3813 || n = 3822 || n = 3827 || n = 3828)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3754)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 3755 || n = 3756 || n = 3759 || n = 3774 || n = 3790 || n = 3791 || n = 3810)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 3758 || n = 3775 || n = 3792 || n = 3809 || n = 3826)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3761 || n = 3781 || n = 3801 || n = 3821)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3763 || n = 3782 || n = 3820)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3764 || n = 3805 || n = 3815)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.elseif(n = 3771)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
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
.elseif(n = 3773 || n = 3819)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3776)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 3777 || n = 3793 || n = 3825)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3778 || n = 3812 || n = 3824)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3780 || n = 3797 || n = 3804 || n = 3814)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3785 || n = 3807)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
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
.elseif(n = 3799)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3802 || n = 3823)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3817)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.endif
.else
; 3829 to 3906
.if(n = 3829 || n = 3851 || n = 3895)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3830 || n = 3845 || n = 3847 || n = 3862 || n = 3864 || n = 3875 || n = 3879 || n = 3883 || n = 3885 || n = 3891 || n = 3898 || n = 3903)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3831 || n = 3848 || n = 3882 || n = 3899 || n = 3906)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3832 || n = 3849 || n = 3866 || n = 3900 || n = 3902)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3833 || n = 3850 || n = 3863 || n = 3867 || n = 3868 || n = 3884 || n = 3888)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3834 || (n > 3835 && n < 3839) || n = 3852 || n = 3854 || n = 3855 || n = 3869 || n = 3872 || n = 3874 || n = 3878 || n = 3890 || n = 3892 || n = 3893)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3835 || n = 3853 || n = 3871)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3839 || n = 3858 || n = 3896)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3840)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 3841 || n = 3861 || n = 3881 || n = 3901)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3842 || n = 3846 || n = 3859 || n = 3880 || n = 3897 || n = 3904)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3843 || n = 3860 || n = 3877 || n = 3894)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3844 || n = 3865 || n = 3886)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3856)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
.elseif((n .mod 16 = 1) && ((n > 3856 && n < 3906)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3870)
	; 10 bytes
	php
	ldx #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3876)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 3887)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 11
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907 || n = 3949 || n = 3970)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3908 || n = 3912 || n = 3918 || n = 3922 || n = 3935 || n = 3952 || n = 3960)
	; 10..11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3909 || n = 3913 || n = 3923 || n = 3930 || n = 3932 || n = 3936 || n = 3938 || n = 3947 || n = 3955 || n = 3957 || n = 3963 || n = 3964 || n = 3966 || n = 3971 || n = 3974 || n = 3976)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3929 || n = 3942 || n = 3954)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3979)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3914 || n = 3924 || n = 3931 || n = 3944 || n = 3948 || n = 3956 || n = 3965 || n = 3975 || n = 3982)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3915 || n = 3934 || n = 3953 || n = 3972)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3916 || n = 3920 || n = 3933 || n = 3946 || n = 3950 || n = 3958 || n = 3984)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3917 || n = 3939 || n = 3983)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3919)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3921 || n = 3941 || n = 3961 || n = 3981)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3925)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3926)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-7
.elseif(n = 3927)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3937 || n = 3969)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3940 || n = 3951 || n = 3959 || n = 3968 || n = 3973 || n = 3978)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3943)
	; 9 bytes
	ldx #24 ;hides 'clc'
	rol $26
	ror $26
	inx
	Jne *-6
.elseif(n = 3967)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3977)
	; 9 bytes
	ldx #234 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 3980)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.endif
.else
; 3985 to 4062
.if(n = 3985 || n = 4017 || n = 4033 || n = 4049)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3986 || n = 3988 || n = 3992 || n = 4008 || n = 4020 || n = 4028 || n = 4037 || n = 4040 || n = 4055)
	; 10..11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3987 || n = 3993 || n = 3995 || n = 3998 || n = 4000 || n = 4003 || n = 4007 || n = 4015 || n = 4019 || n = 4023 || n = 4025 || n = 4031 || n = 4032 || n = 4034 || n = 4035 || n = 4043 || n = 4045 || n = 4050 || n = 4056)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3989 || n = 4011 || n = 4042 || n = 4058 || n = 4060)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3990)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3994 || n = 4004 || n = 4016 || n = 4024 || n = 4044 || n = 4062)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
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
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 13
	inx
	Jne *-7
.elseif(n = 4001 || n = 4021 || n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4002 || n = 4036 || n = 4053)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4005 || n = 4027)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4006 || n = 4018 || n = 4046 || n = 4052)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4009 || n = 4057)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4012 || n = 4054)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4014)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4022)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4026)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4038)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4039)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4059)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
; 4063 to 4218
.if(n = 4063 || n = 4066 || n = 4068 || n = 4069 || n = 4073 || n = 4077 || n = 4083 || n = 4085 || n = 4088 || n = 4090 || n = 4099 || n = 4100 || n = 4102 || n = 4107 || n = 4117 || n = 4119 || n = 4123 || n = 4125 || n = 4126 || n = 4134 || n = 4136 || n = 4140 || n = 4145 || n = 4147 || n = 4151 || n = 4153 || (n > 4162 && n < 4166) || n = 4168 || n = 4170 || n = 4182 || n = 4185 || n = 4187 || n = 4202 || n = 4204)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132 || n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217)
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
.elseif(n = 4067 || n = 4086 || n = 4105 || n = 4124 || n = 4143 || n = 4162)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4070 || n = 4092 || n = 4104 || n = 4109 || n = 4111 || n = 4130 || n = 4155 || n = 4167 || n = 4172 || n = 4189 || n = 4206)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4071 || n = 4093 || n = 4137 || n = 4159 || n = 4203)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4072 || n = 4095 || n = 4210)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4074 || n = 4110 || n = 4112 || n = 4122 || n = 4128 || n = 4131 || n = 4139 || n = 4148 || n = 4150 || n = 4156 || n = 4173 || n = 4190 || n = 4207 || n = 4208 || n = 4218)
	; 10..11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4075 || n = 4096 || n = 4138 || n = 4180)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4076 || n = 4176)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 4078)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4079 || n = 4080 || n = 4082 || n = 4094 || n = 4113 || n = 4114 || n = 4133 || n = 4142 || n = 4158 || n = 4160 || n = 4174 || n = 4178 || n = 4179 || (n > 4190 && n < 4195) || (n > 4195 && n < 4199) || n = 4209 || n = 4212 || n = 4214 || n = 4216)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4084 || n = 4089 || n = 4106 || n = 4118 || n = 4127 || n = 4135 || n = 4144 || n = 4146 || n = 4152 || n = 4169 || n = 4184 || n = 4186)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4087 || n = 4177 || n = 4195 || n = 4213)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4091 || n = 4108 || n = 4120 || n = 4154 || n = 4171 || n = 4188 || n = 4205)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4097)
	; 9 bytes
	ldx #0
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4101 || n = 4121 || n = 4141 || n = 4161 || n = 4181 || n = 4201)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4103)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4116)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	inx
	inx
	Jne *-6
.elseif(n = 4129)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4157)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
.elseif(n = 4175)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 4199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4211)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4215)
	; 9 bytes
	ldx #248 ;hides 'sed'
	rol $26
	ror $26
	dex
	Jne *-6
.endif
.else
; 4219 to 4375
.if(n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295 || n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4220 || n = 4237 || n = 4244 || n = 4254 || n = 4260 || n = 4271 || n = 4279 || n = 4284 || n = 4288 || n = 4298 || n = 4305 || n = 4317 || n = 4322 || n = 4344 || n = 4356 || n = 4364 || n = 4374)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4221 || n = 4241 || n = 4261 || n = 4281 || n = 4301 || n = 4321 || n = 4341 || n = 4361)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4222 || n = 4243 || n = 4264 || n = 4306 || n = 4327 || n = 4348 || n = 4369)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4223 || n = 4227 || n = 4236 || n = 4240 || n = 4242 || n = 4253 || n = 4255 || n = 4259 || n = 4263 || n = 4265 || n = 4266 || n = 4270 || n = 4272 || n = 4278 || n = 4280 || n = 4283 || n = 4287 || n = 4289 || n = 4293 || n = 4297 || n = 4299 || n = 4303 || n = 4304 || n = 4315 || n = 4316 || n = 4318 || n = 4323 || n = 4329 || n = 4335 || n = 4337 || n = 4338 || n = 4340 || n = 4343 || n = 4350 || n = 4354 || n = 4355 || n = 4363 || n = 4365 || n = 4373)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4224 || n = 4226 || n = 4228 || n = 4245 || n = 4248 || n = 4258 || n = 4275 || n = 4292 || n = 4309 || n = 4326 || n = 4328 || n = 4360 || n = 4368)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4225 || n = 4247 || n = 4269 || n = 4291 || n = 4313 || n = 4357)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4229 || n = 4230 || n = 4232 || n = 4235 || n = 4250 || n = 4252 || n = 4277 || n = 4296 || (n > 4309 && n < 4313) || (n > 4329 && n < 4333) || n = 4334 || n = 4349 || n = 4362 || n = 4370 || n = 4372)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4231 || n = 4249 || n = 4267 || n = 4339 || n = 4375)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4233)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4234 || n = 4251 || n = 4268 || n = 4285 || n = 4302 || n = 4319 || n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4239 || n = 4246 || n = 4256 || n = 4286 || n = 4290 || n = 4300 || n = 4307 || n = 4324 || n = 4346 || n = 4358 || n = 4366)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4262)
	; 8 bytes
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
.elseif(n = 4273 || n = 4345)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4274 || n = 4282 || n = 4308 || n = 4320 || n = 4325 || n = 4342 || n = 4347 || n = 4359 || n = 4367)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4294)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4351)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 4353)
	; 8 bytes
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4386 || n = 4412 || n = 4433 || n = 4446 || n = 4450 || n = 4471 || n = 4490)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4377 || n = 4387 || n = 4396 || n = 4413 || n = 4415 || n = 4427 || n = 4451 || n = 4529)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4378 || n = 4397 || n = 4406 || n = 4408 || n = 4414 || n = 4416 || n = 4435 || n = 4452 || n = 4454 || n = 4472 || n = 4473 || n = 4488 || n = 4492 || n = 4508 || n = 4528 || n = 4530)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4379 || n = 4423 || n = 4467 || n = 4489 || n = 4511)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4380 || n = 4382 || n = 4389 || n = 4391 || n = 4398 || n = 4400 || (n > 4417 && n < 4421) || n = 4422 || (n > 4435 && n < 4440) || n = 4442 || (n > 4456 && n < 4461) || n = 4462 || n = 4475 || (n > 4476 && n < 4481) || n = 4482 || n = 4493 || n = 4494 || n = 4496 || n = 4498 || n = 4500 || n = 4502 || n = 4512 || n = 4514 || n = 4515 || n = 4517 || n = 4520 || n = 4522 || n = 4531)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4381 || n = 4401 || n = 4421 || n = 4441 || n = 4461 || n = 4481 || n = 4501 || n = 4521)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4383 || n = 4385 || n = 4392 || n = 4394 || n = 4403 || n = 4405 || n = 4430 || n = 4434 || n = 4443 || n = 4449 || n = 4455 || n = 4468 || n = 4470 || n = 4476 || n = 4483 || n = 4487 || n = 4491 || n = 4497 || n = 4503 || n = 4505 || n = 4506 || n = 4518 || n = 4525 || n = 4527)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4384 || n = 4404 || n = 4410 || n = 4424 || n = 4431 || n = 4444 || n = 4448 || n = 4464 || n = 4469 || n = 4484 || n = 4507 || n = 4524 || n = 4526)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4388)
	; 10 bytes
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
.elseif(n = 4390 || n = 4409 || n = 4428 || n = 4447 || n = 4466 || n = 4485 || n = 4504 || n = 4523)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4393 || n = 4429 || n = 4519)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4395 || n = 4499)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 4402 || n = 4456 || n = 4510)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4407)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4411 || n = 4432 || n = 4453 || n = 4474 || n = 4495 || n = 4516)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4417 || n = 4513)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4425)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4426)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 4440 || n = 4486 || n = 4509)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4445)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4463)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4465)
	; 8 bytes
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if(n = 4532 || n = 4578)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4533 || n = 4555 || n = 4577)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4534 || n = 4536 || n = 4550 || (n > 4551 && n < 4555) || n = 4556 || n = 4559 || n = 4562 || (n > 4568 && n < 4573) || n = 4574 || n = 4575 || n = 4589 || n = 4590 || (n > 4591 && n < 4599))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4535)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4537 || n = 4558 || n = 4579 || n = 4600)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4538 || n = 4545 || n = 4602 || n = 4604)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4539 || n = 4543 || n = 4544 || n = 4546 || n = 4557 || n = 4560 || n = 4565 || (n > 4581 && n < 4585) || n = 4603 || n = 4605)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4540)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4541 || n = 4581 || n = 4601)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4542 || n = 4561 || n = 4580 || n = 4599)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4547 || n = 4586 || n = 4607)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4548 || n = 4549 || n = 4587 || n = 4588 || n = 4606 || n = 4608)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4551)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4563)
	; 9 bytes
	ldx #(n-3)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	nop
.elseif(n = 4564)
	; 10 bytes
	ldx #228
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx $A6
.elseif(n = 4566)
	; 10 bytes
	ldx #228
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp
.elseif(n = 4567)
	; 10 bytes
	ldx #228
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp,x
.elseif(n = 4568)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4573 || n = 4591)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4576)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 4585)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4609)
	; 10 bytes
	ldx #0
	delay_n 13
	dex
	Jne *-6
.endif
.else
; 4610 to 4687
.if((n > 4609 && n < 4618) || (n > 4629 && n < 4633) || (n > 4633 && n < 4637) || n = 4638 || n = 4649 || n = 4650 || n = 4652 || n = 4654 || n = 4669 || n = 4671 || n = 4673 || n = 4674)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4618 || n = 4637 || n = 4656 || n = 4675)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4619)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
.elseif(n = 4620 || n = 4622 || n = 4623 || n = 4639 || n = 4658 || n = 4660 || n = 4663 || n = 4667 || n = 4677 || n = 4683 || n = 4685 || n = 4686)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4621 || n = 4641 || n = 4661 || n = 4681)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4624 || n = 4640 || n = 4659 || n = 4664 || n = 4678)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4625 || n = 4628 || n = 4644 || n = 4648 || n = 4682)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4626 || n = 4651 || n = 4676)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 4627 || n = 4662)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4629 || n = 4655)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4633 || n = 4657)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4642 || n = 4684)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4643 || n = 4665 || n = 4687)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4645)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4646 || n = 4666 || n = 4680)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4647 || n = 4670)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4653)
	; 10 bytes
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	inx
	inx
	Jne *-7
.elseif(n = 4668)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
.elseif(n = 4672)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4679)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 13
	dex
	Jne *-7
.endif
.endif
.endif
.else
; 4688 to 5000
.if(n = 4688 || n = 4708 || n = 4718 || n = 4720 || n = 4728 || n = 4739 || n = 4748 || n = 4758 || n = 4788 || n = 4796 || n = 4815 || n = 4828 || n = 4834 || n = 4853 || n = 4868 || n = 4872 || n = 4878 || n = 4888 || n = 4908 || n = 4920 || n = 4928 || n = 4948 || n = 4968 || n = 4988)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4689 || n = 4696 || n = 4698 || n = 4703 || n = 4715 || n = 4723 || n = 4725 || n = 4734 || n = 4736 || n = 4743 || n = 4745 || n = 4749 || n = 4763 || n = 4765 || n = 4772 || n = 4774 || n = 4783 || n = 4785 || n = 4791 || n = 4793 || n = 4803 || n = 4805 || n = 4823 || n = 4829 || n = 4833 || n = 4843 || n = 4845 || n = 4848 || n = 4867 || n = 4869 || n = 4875 || n = 4883 || n = 4887 || n = 4896 || n = 4903 || n = 4905 || n = 4909 || n = 4917 || n = 4923 || n = 4925 || n = 4931 || n = 4938 || n = 4943 || n = 4953 || n = 4963 || n = 4965 || n = 4975 || n = 4980 || n = 4983 || n = 4985 || n = 4997)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n > 4689 && n < 4693) || n = 4695 || n = 4702 || n = 4710 || n = 4712 || n = 4722 || n = 4730 || n = 4740 || n = 4742 || n = 4750 || n = 4752 || n = 4760 || n = 4769 || n = 4771 || n = 4778 || n = 4779 || n = 4782 || n = 4790 || n = 4798 || n = 4800 || n = 4802 || n = 4809 || (n > 4815 && n < 4819) || n = 4820 || n = 4822 || n = 4835 || n = 4836 || (n > 4837 && n < 4841) || n = 4842 || (n > 4854 && n < 4861) || n = 4862 || n = 4879 || n = 4880 || n = 4882 || (n > 4889 && n < 4894) || n = 4895 || n = 4898 || n = 4899 || n = 4902 || (n > 4909 && n < 4913) || n = 4914 || n = 4922 || n = 4930 || (n > 4931 && n < 4936) || n = 4937 || n = 4939 || n = 4940 || n = 4949 || n = 4950 || n = 4952 || (n > 4953 && n < 4957) || n = 4958 || n = 4960 || n = 4962 || (n > 4969 && n < 4973) || n = 4974 || n = 4977 || n = 4979 || n = 4982 || (n > 4988 && n < 4992) || n = 4994 || n = 4998 || n = 5000)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4693 || n = 4762 || n = 4854 || n = 4877 || n = 4900 || n = 4946 || n = 4992)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4694 || n = 4713 || n = 4732 || n = 4751 || n = 4770 || n = 4789 || n = 4808 || n = 4827 || n = 4846)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4697 || n = 4704 || n = 4714 || n = 4724 || n = 4735 || n = 4744 || n = 4754 || n = 4764 || n = 4773 || n = 4784 || n = 4792 || n = 4804 || n = 4811 || n = 4824 || n = 4830 || n = 4844 || n = 4864 || n = 4874 || n = 4884 || n = 4904 || n = 4916 || n = 4924 || n = 4944 || n = 4964 || n = 4984)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4699 || n = 4780 || n = 4807 || n = 4942 || n = 4996)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4700 || n = 4717 || n = 4719 || n = 4727 || n = 4738 || n = 4757 || n = 4767 || n = 4787 || n = 4795 || n = 4814 || n = 4847 || n = 4919 || n = 4927 || n = 4947 || n = 4987)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4701 || n = 4721 || n = 4741 || n = 4761 || n = 4781 || n = 4801 || n = 4821 || n = 4841 || n = 4861 || n = 4881 || n = 4901 || n = 4921 || n = 4941 || n = 4961 || n = 4981)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4705 || n = 4726 || n = 4747 || n = 4768 || n = 4810 || n = 4831 || n = 4852 || n = 4873 || n = 4894 || n = 4915 || n = 4936 || n = 4957 || n = 4978 || n = 4999)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4706 || n = 4716 || n = 4737 || n = 4746 || n = 4756 || n = 4766 || n = 4786 || n = 4794 || n = 4806 || n = 4813 || n = 4832 || n = 4851 || n = 4866 || n = 4870 || n = 4876 || n = 4886 || n = 4906 || n = 4918 || n = 4966 || n = 4986)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4707 || n = 4733 || n = 4759 || n = 4837 || n = 4889)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4709 || n = 4731 || n = 4753 || n = 4775 || n = 4797 || n = 4819 || n = 4863 || n = 4885 || n = 4907 || n = 4929 || n = 4951 || n = 4973 || n = 4995)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4711)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4729 || n = 4777 || n = 4825 || n = 4849 || n = 4897 || n = 4945 || n = 4969 || n = 4993)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4755 || n = 4812)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4776 || n = 4826 || n = 4926 || n = 4976)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 4799)
	; 9 bytes
	ldx #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.elseif(n = 4850)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
.elseif(n = 4865)
	; 8 bytes
	ldx #0
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4871)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4913)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4959)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4967)
	; 9 bytes
	ldx #216 ;hides 'cld'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
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
; 5001 to 5312
.if(n = 5001 || n = 5021 || n = 5041 || n = 5061 || n = 5081 || n = 5101)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 5002 || (n > 5008 && n < 5015) || n = 5016 || n = 5018 || (n > 5028 && n < 5038) || n = 5040 || n = 5042 || n = 5049 || (n > 5051 && n < 5061) || n = 5069 || n = 5070 || n = 5072 || n = 5073 || n = 5075 || (n > 5077 && n < 5081) || n = 5082 || (n > 5089 && n < 5097) || (n > 5097 && n < 5101) || n = 5102 || n = 5111 || n = 5112 || (n > 5113 && n < 5121) || n = 5122 || (n > 5131 && n < 5137) || (n > 5137 && n < 5146) || (n > 5154 && n < 5158) || n = 5159 || n = 5160 || (n > 5161 && n < 5167) || n = 5168 || n = 5170 || n = 5172 || n = 5174 || (n > 5176 && n < 5183) || n = 5184 || n = 5186 || n = 5187 || n = 5189 || n = 5191 || n = 5192 || n = 5194 || (n > 5195 && n < 5199) || n = 5200 || (n > 5201 && n < 5207) || n = 5208 || n = 5216 || (n > 5217 && n < 5222) || (n > 5222 && n < 5226) || n = 5228 || n = 5229 || n = 5231 || (n > 5233 && n < 5237) || n = 5238 || (n > 5239 && n < 5245) || (n > 5245 && n < 5251) || n = 5252 || (n > 5253 && n < 5257) || n = 5258 || n = 5260 || (n > 5261 && n < 5266) || n = 5267 || (n > 5268 && n < 5272) || n = 5273 || n = 5275 || n = 5278 || n = 5280 || n = 5282 || (n > 5283 && n < 5291) || n = 5292 || n = 5294 || (n > 5295 && n < 5301) || n = 5302 || n = 5304 || (n > 5305 && n < 5312))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5003 || n = 5005 || n = 5019 || n = 5022 || n = 5023 || n = 5025 || n = 5043 || n = 5045 || n = 5063 || n = 5106 || n = 5123 || n = 5129 || n = 5148 || n = 5151 || n = 5169 || n = 5173 || n = 5190 || n = 5195 || n = 5211 || n = 5217 || n = 5232 || n = 5261 || n = 5274 || n = 5283 || n = 5295)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5004 || n = 5024 || n = 5044 || n = 5064 || n = 5124 || n = 5150 || n = 5210)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5006 || n = 5046 || n = 5066 || n = 5086 || n = 5126 || n = 5152)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5007 || n = 5027 || n = 5047 || n = 5067 || n = 5087 || n = 5109 || n = 5213)
	; 10 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 5008 || n = 5028 || n = 5048 || n = 5068 || n = 5088 || n = 5108 || n = 5110 || n = 5128 || n = 5154 || n = 5214)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5015 || n = 5038 || n = 5084 || n = 5107 || n = 5130 || n = 5153 || n = 5176 || n = 5222 || n = 5245 || n = 5268 || n = 5291)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5017 || n = 5039 || n = 5083 || n = 5105 || n = 5127 || n = 5149 || n = 5171 || n = 5193 || n = 5215 || n = 5237 || n = 5259 || n = 5281 || n = 5303)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5020 || n = 5062 || n = 5104 || n = 5125 || n = 5146 || n = 5167 || n = 5188 || n = 5209 || n = 5230 || n = 5251 || n = 5272 || n = 5293)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5026 || n = 5051 || n = 5076 || n = 5226 || n = 5276 || n = 5301)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 5050 || n = 5077 || n = 5131 || n = 5158 || n = 5212 || n = 5239 || n = 5266)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5065 || n = 5089 || n = 5113 || n = 5137 || n = 5161 || n = 5185 || n = 5233 || n = 5257 || n = 5305)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5071 || n = 5097 || n = 5175 || n = 5201 || n = 5227 || n = 5253 || n = 5279)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5074)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	jsr @nearby_rts_14cyc
	Jne *-7
.elseif(n = 5085)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5103)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 5121)
	; 8 bytes
	ldx #0
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 5147)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5183)
	; 9 bytes
	ldx #216 ;hides 'cld'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5199)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5207)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5277)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
.elseif(n = 5312)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
.endif
.else
; 5313 to 5625
.if(n = 5313 || n = 5315 || (n > 5316 && n < 5320) || (n > 5320 && n < 5325) || n = 5328 || n = 5330 || (n > 5331 && n < 5335) || n = 5336 || (n > 5337 && n < 5347) || n = 5348 || n = 5350 || n = 5352 || n = 5354 || n = 5355 || n = 5359 || (n > 5360 && n < 5369) || n = 5370 || n = 5372 || n = 5373 || n = 5375 || n = 5378 || n = 5380 || n = 5382 || (n > 5383 && n < 5391) || n = 5392 || (n > 5393 && n < 5399) || n = 5400 || (n > 5401 && n < 5406) || n = 5407 || n = 5408 || (n > 5409 && n < 5413) || n = 5414 || (n > 5415 && n < 5425) || n = 5427 || (n > 5429 && n < 5435) || n = 5436 || (n > 5437 && n < 5449) || n = 5450 || n = 5453 || n = 5454 || n = 5456 || (n > 5462 && n < 5473) || n = 5474 || n = 5477 || n = 5478 || (n > 5482 && n < 5487) || (n > 5487 && n < 5497) || n = 5499 || n = 5500 || n = 5502 || (n > 5503 && n < 5509) || (n > 5509 && n < 5513) || (n > 5513 && n < 5521) || n = 5522 || n = 5524 || (n > 5526 && n < 5536) || n = 5537 || n = 5538 || (n > 5539 && n < 5543) || n = 5546 || (n > 5547 && n < 5551) || (n > 5551 && n < 5563) || n = 5564 || n = 5566 || n = 5568 || (n > 5569 && n < 5576) || (n > 5576 && n < 5589) || n = 5592 || (n > 5593 && n < 5599) || n = 5600 || (n > 5601 && n < 5611) || n = 5612 || n = 5614 || n = 5616 || (n > 5617 && n < 5626))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5314 || n = 5335 || n = 5356)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5316 || n = 5327 || n = 5349 || n = 5358 || n = 5371 || n = 5379 || n = 5393 || n = 5415 || n = 5437 || n = 5459 || n = 5481 || n = 5503 || n = 5525 || n = 5547)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5320 || n = 5374 || n = 5428 || n = 5482 || n = 5509 || n = 5536 || n = 5563 || n = 5590)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5325 || n = 5347 || n = 5369 || n = 5391 || n = 5413 || n = 5435 || n = 5457 || n = 5479 || n = 5501 || n = 5523 || n = 5545 || n = 5567 || n = 5589 || n = 5611)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5326 || n = 5351 || n = 5376 || n = 5426 || n = 5451 || n = 5476 || n = 5526 || n = 5551 || n = 5576 || n = 5601)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif(n = 5329 || n = 5353 || n = 5401 || n = 5425 || n = 5449 || n = 5473 || n = 5497 || n = 5521 || n = 5569 || n = 5593 || n = 5617)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5331 || n = 5357 || n = 5409 || n = 5461 || n = 5487 || n = 5513 || n = 5539 || n = 5565 || n = 5591)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5337 || n = 5360 || n = 5383 || n = 5406 || n = 5429 || n = 5452 || n = 5475 || n = 5498 || n = 5544 || n = 5613)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5377)
	; 9 bytes
	ldx #0
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5381)
	; 9 bytes
	ldx #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5399)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5455)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5458)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5460)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5462)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5480)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
.elseif(n = 5543)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
.elseif(n = 5599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5615)
	; 9 bytes
	ldx #234 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.endif
.endif
.else
; 5626 to 6250
.if(n = 5626 || n = 5651 || n = 5676 || n = 5701 || n = 5726 || n = 5776 || n = 5801 || n = 5826 || n = 5876 || n = 5901 || n = 5926 || n = 5976 || n = 6026 || n = 6051 || n = 6101 || n = 6126 || n = 6151 || n = 6176 || n = 6201 || n = 6226)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif((n > 5626 && n < 5633) || n = 5634 || (n > 5636 && n < 5641) || n = 5642 || (n > 5644 && n < 5651) || (n > 5651 && n < 5659) || (n > 5659 && n < 5665) || (n > 5665 && n < 5669) || n = 5670 || (n > 5671 && n < 5676) || (n > 5676 && n < 5682) || (n > 5683 && n < 5689) || (n > 5689 && n < 5695) || n = 5696 || n = 5697 || n = 5699 || n = 5700 || n = 5702 || n = 5704 || (n > 5705 && n < 5713) || (n > 5713 && n < 5721) || (n > 5721 && n < 5725) || n = 5727 || (n > 5728 && n < 5737) || (n > 5737 && n < 5747) || (n > 5747 && n < 5751) || (n > 5752 && n < 5761) || (n > 5761 && n < 5773) || n = 5775 || n = 5777 || n = 5778 || (n > 5779 && n < 5785) || (n > 5785 && n < 5797) || n = 5798 || n = 5800 || (n > 5801 && n < 5806) || n = 5807 || n = 5808 || (n > 5809 && n < 5820) || (n > 5820 && n < 5825) || (n > 5826 && n < 5831) || n = 5832 || (n > 5833 && n < 5843) || (n > 5843 && n < 5849) || n = 5850 || (n > 5851 && n < 5857) || n = 5858 || n = 5859 || (n > 5860 && n < 5866) || (n > 5866 && n < 5876) || (n > 5877 && n < 5881) || (n > 5881 && n < 5886) || n = 5888 || (n > 5889 && n < 5901) || n = 5902 || n = 5904 || (n > 5905 && n < 5914) || (n > 5914 && n < 5926) || n = 5927 || n = 5928 || (n > 5929 && n < 5941) || (n > 5941 && n < 5951) || n = 5952 || n = 5954 || (n > 5955 && n < 5968) || (n > 5968 && n < 5976) || (n > 5977 && n < 5981) || (n > 5981 && n < 5995) || (n > 5995 && n < 6001) || (n > 6001 && n < 6005) || n = 6006 || (n > 6007 && n < 6022) || n = 6023 || n = 6024 || (n > 6026 && n < 6031) || n = 6032 || (n > 6033 && n < 6047) || n = 6048 || n = 6050 || (n > 6051 && n < 6059) || (n > 6059 && n < 6073) || n = 6074 || n = 6075 || (n > 6076 && n < 6083) || n = 6084 || (n > 6085 && n < 6097) || (n > 6097 && n < 6101) || n = 6102 || (n > 6103 && n < 6111) || (n > 6111 && n < 6121) || (n > 6121 && n < 6126) || (n > 6126 && n < 6130) || (n > 6130 && n < 6137) || (n > 6137 && n < 6145) || (n > 6145 && n < 6151) || (n > 6151 && n < 6157) || (n > 6157 && n < 6163) || (n > 6163 && n < 6176) || (n > 6176 && n < 6184) || (n > 6184 && n < 6189) || (n > 6189 && n < 6199) || n = 6200 || (n > 6201 && n < 6211) || (n > 6211 && n < 6215) || (n > 6215 && n < 6226) || (n > 6226 && n < 6238) || n = 6239 || n = 6240 || (n > 6241 && n < 6251))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5633)
	; 9 bytes
	ldx #0
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5635)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5636 || n = 5659 || n = 5682 || n = 5705 || n = 5728 || n = 5751 || n = 5774 || n = 5797 || n = 5820 || n = 5843 || n = 5866)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5641 || n = 5665 || n = 5689 || n = 5713 || n = 5737 || n = 5761 || n = 5785 || n = 5809 || n = 5833 || n = 5857 || n = 5881 || n = 5905 || n = 5929 || n = 5953 || n = 5977 || n = 6001 || n = 6025 || n = 6049 || n = 6073 || n = 6097 || n = 6121)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5643 || n = 5669 || n = 5695 || n = 5721 || n = 5747 || n = 5773 || n = 5825 || n = 5851 || n = 5877 || n = 5903 || n = 5955 || n = 5981 || n = 6007 || n = 6033 || n = 6059 || n = 6085 || n = 6111 || n = 6137 || n = 6163 || n = 6189 || n = 6215 || n = 6241)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5644 || n = 5671 || n = 5698 || n = 5725 || n = 5752 || n = 5779 || n = 5806 || n = 5860 || n = 5887 || n = 5914 || n = 5941 || n = 5968 || n = 5995 || n = 6022 || n = 6076 || n = 6103 || n = 6130 || n = 6157 || n = 6184 || n = 6211 || n = 6238)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5683)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 9
	jsr @nearby_rts_14cyc
	Jne *-7
.elseif(n = 5703)
	; 9 bytes
	ldx #248 ;hides 'sed'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5799)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5831)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 20
	dex
	Jne *-7
.elseif(n = 5849)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5886)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
.elseif(n = 5889)
	; 10 bytes
	ldx #0
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5951)
	; 9 bytes
	ldx #248 ;hides 'sed'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 6005)
	; 10 bytes
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-7
.elseif(n = 6031)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 6047)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
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
.elseif(n = 6199)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.endif
.endif
.else
; 6251 to 7500
.if(n = 6251 || n = 6276 || n = 6301 || n = 6326 || n = 6351 || n = 6376)
	; 10 bytes
	ldx #(n-1)/25
	delay_n 20
	dex
	Jne *-6
.elseif((n > 6251 && n < 6263) || n = 6264 || n = 6266 || (n > 6267 && n < 6276) || (n > 6276 && n < 6292) || (n > 6293 && n < 6301) || (n > 6301 && n < 6317) || n = 6318 || (n > 6319 && n < 6326) || (n > 6326 && n < 6345) || (n > 6346 && n < 6351) || (n > 6351 && n < 6371) || n = 6372 || n = 6374 || n = 6375 || (n > 6376 && n < 6397) || n = 6398 || n = 6399 || (n > 6401 && n < 6423) || (n > 6423 && n < 6427) || (n > 6427 && n < 6447) || n = 6448 || (n > 6449 && n < 6454) || (n > 6454 && n < 6475) || (n > 6475 && n < 6481) || (n > 6481 && n < 6495) || (n > 6495 && n < 6501) || (n > 6501 && n < 6508) || (n > 6508 && n < 6527) || (n > 6527 && n < 6535) || (n > 6535 && n < 6551) || n = 6552 || (n > 6553 && n < 6562) || (n > 6562 && n < 6579) || (n > 6579 && n < 6589) || (n > 6589 && n < 6605) || (n > 6605 && n < 6616) || (n > 6616 && n < 6631) || (n > 6631 && n < 6643) || (n > 6643 && n < 6657) || (n > 6657 && n < 6670) || (n > 6670 && n < 6695) || n = 6696 || (n > 6698 && n < 6724) || n = 6725 || n = 6726 || (n > 6727 && n < 6751) || (n > 6751 && n < 6778) || (n > 6778 && n < 6785) || (n > 6785 && n < 6805) || (n > 6805 && n < 6832) || (n > 6832 && n < 6859) || (n > 6859 && n < 6886) || (n > 6886 && n < 6901) || (n > 6901 && n < 6913) || (n > 6913 && n < 6943) || (n > 6943 && n < 7104) || (n > 7104 && n < 7191) || (n > 7191 && n < 7501))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6263)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 6265 || n = 6319 || n = 6346 || n = 6373 || n = 6400 || n = 6427 || n = 6454 || n = 6481 || n = 6508 || n = 6535 || n = 6562 || n = 6589 || n = 6616 || n = 6643 || n = 6670 || n = 6697 || n = 6724 || n = 6751 || n = 6778 || n = 6805 || n = 6832 || n = 6859 || n = 6886)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 6267 || n = 6293 || n = 6345 || n = 6371 || n = 6397 || n = 6423 || n = 6449 || n = 6475 || n = 6501 || n = 6527 || n = 6553 || n = 6579 || n = 6605 || n = 6631)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 6292)
	; 10 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	jsr @nearby_rts_14cyc
	Jne *-7
.elseif(n = 6317)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 20
	dex
	Jne *-7
.elseif(n = 6401)
	; 10 bytes
	ldx #0
	delay_n 20
	dex
	Jne *-6
.elseif(n = 6447)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 6495)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-7
.elseif(n = 6551)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 6657)
	; 10 bytes
	ldx #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 6695)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 20
	dex
	Jne *-7
.elseif(n = 6698)
	; 10 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_14cyc
	Jne *-7
.elseif(n = 6727)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6785)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 6901)
	; 10 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	Jne *-7
.elseif(n = 6913)
	; 10 bytes
	ldx #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 6943)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 7104)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 30
	Jne *-7
.elseif(n = 7191)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_15cyc
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
