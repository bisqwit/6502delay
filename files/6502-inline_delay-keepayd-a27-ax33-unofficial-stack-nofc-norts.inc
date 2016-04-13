; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, S, Z+N, C, and V
; Will preserve A, Y, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 14
; - 5 bytes or less when n <= 17
; - 6 bytes or less when n <= 26
; - 7 bytes or less when n <= 1301
; - 8 bytes or less when n <= 1945
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=4, avg=4.9, max=6
; - When 21 <= n <= 30: min=5, avg=6.0, max=7
; - When 31 <= n <= 50: min=5, avg=6.2, max=7
; - When 51 <= n <= 75: min=5, avg=6.1, max=7
; - When 76 <= n <= 100: min=5, avg=6.1, max=7
; - When 101 <= n <= 200: min=4, avg=6.0, max=7
; - When 201 <= n <= 300: min=5, avg=6.1, max=7
; - When 301 <= n <= 500: min=4, avg=6.0, max=7
; - When 501 <= n <= 750: min=5, avg=6.0, max=7
; - When 751 <= n <= 1000: min=5, avg=6.1, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.7, max=9
; - When 2001 <= n <= 3000: min=6, avg=7.8, max=9
; - When 3001 <= n <= 5000: min=7, avg=8.9, max=9
; - When 5001 <= n <= 7500: min=7, avg=9.0, max=9
; - When 7501 <= n <= 10000: min=7, avg=9.0, max=9
; - When 10001 <= n <= 20000: min=7, avg=9.0, max=9
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
	; 1 bytes
	pha
.elseif(n = 4 || n = 5 || n = 8 || n = 23 || n = 28 || n = 30 || n = 33 || n = 38 || n = 48 || n = 53 || n = 58 || n = 63 || n = 68 || n = 73)
	; 2..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 6)
	; 2 bytes
	delay_n (n-3)
	pha
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif((n > 8 && n < 12) || n = 13)
	; 3..4 bytes
	delay_n (n-7)
	delay_n 7
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
	.repeat 5
	 pha
	.endrepeat
.elseif((n > 15 && n < 19) || n = 20)
	; 5..6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 19)
	; 6 bytes
	php
	delay_n 12
	plp
.elseif(n = 21 || n = 26 || n = 36 || n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 22 || n = 29 || n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 24 || n = 34 || n = 39 || n = 44 || n = 54 || n = 59 || n = 69 || n = 74)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 25)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 27)
	; 7 bytes
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
.elseif(n = 31 || n = 41 || n = 61 || n = 71 || n = 76)
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
.elseif(n = 35)
	; 7 bytes
	tax
	lda #2
	jsr delay_a_27_clocks
	txa
.elseif(n = 37 || n = 40 || n = 42 || n = 45 || n = 47 || n = 52 || n = 55 || n = 60 || n = 62 || n = 67 || n = 70 || n = 72 || n = 75 || n = 77)
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 43 || n = 57)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 49 || n = 65)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.endif
.else
; 79 to 156
.if(n = 79 || n = 84 || n = 94 || n = 104 || n = 109 || n = 114 || n = 119 || n = 127 || n = 139 || n = 144 || n = 149 || n = 154)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 80 || n = 82 || n = 87 || n = 90 || n = 95 || n = 100 || n = 102 || n = 110 || n = 112 || n = 115 || n = 117 || n = 122 || n = 132 || n = 135 || n = 140 || n = 142 || n = 147 || n = 150 || n = 152)
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 81 || n = 96 || n = 121 || n = 131 || n = 146 || n = 151 || n = 156)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 5 = 3) && ((n > 82 && n < 109) || (n > 117 && n < 144)))
	; 6 bytes
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
.elseif(n = 89 || n = 105 || n = 137 || n = 145)
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
.elseif(n = 97 || n = 129)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 107)
	; 6 bytes
	ldx #24 ;hides 'clc'
	dex
	dex
	Jne *-3
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
.elseif(n = 130)
	; 6 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 153)
	; 5 bytes
	ldx #234 ;hides 'nop'
	inx
	Jne *-2
.endif
.endif
.else
; 157 to 312
.if(n = 157 || n = 160 || n = 165 || n = 172 || n = 175 || n = 180 || n = 182 || n = 187 || n = 192 || n = 195 || n = 200 || n = 205 || n = 207 || n = 210 || n = 212 || n = 220 || n = 222 || n = 227 || n = 230 || n = 235 || n = 237 || n = 240 || n = 245 || n = 247 || n = 250 || n = 252 || n = 255 || n = 262 || n = 270 || n = 272 || n = 275 || n = 277 || n = 282 || n = 285 || n = 287 || n = 290)
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 158 || n = 163 || n = 168 || n = 173 || n = 178 || n = 193 || n = 198 || n = 203 || n = 208 || n = 213 || n = 223 || n = 228 || n = 233 || n = 238 || n = 243 || n = 248 || n = 258 || n = 263 || n = 268 || n = 273 || n = 278 || n = 283 || n = 293 || n = 298 || n = 300 || n = 303 || n = 308 || n = 310)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 184 || n = 189 || n = 194 || n = 202 || n = 214 || n = 219 || n = 229 || n = 234 || n = 244 || n = 254 || n = 259 || n = 264 || n = 279 || n = 284 || n = 294)
	; 6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 177 || n = 185 || n = 217 || n = 289 || n = 297 || n = 305)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 179)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
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
.elseif(n = 209 || n = 249 || n = 257)
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
.elseif(n = 280)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 292 || n = 312)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 307)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
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
.elseif(n = 317 || n = 322 || n = 327 || n = 342 || n = 347 || n = 357 || n = 362 || n = 392 || n = 422 || n = 427 || n = 432 || n = 447 || n = 452 || n = 462)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 319 || n = 324 || n = 334 || n = 339 || n = 349 || n = 354 || n = 359 || n = 364 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 415 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459)
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
.elseif(n = 329 || n = 345 || n = 369 || n = 417 || n = 457 || n = 465)
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
.elseif(n = 385 || n = 409 || n = 425)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 387)
	; 7 bytes
	tax
	pla
	txa
	delay_n (n-8)
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
.if(n = 469 || n = 474 || n = 479 || n = 492 || n = 494 || n = 499 || n = 504 || n = 509 || n = 514 || n = 522 || n = 524 || n = 534 || n = 539)
	; 6..7 bytes
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
	ldx #197 ;hides 'cmp zp'
	nop
	inx
	Jne *-3
.elseif(n = 473 || n = 475 || n = 478 || n = 482 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 528 || n = 535 || n = 538 || n = 542 || n = 543)
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
.elseif(n = 502 || n = 527)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
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
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 544)
	; 6 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jmi *-3
.endif
.else
; 547 to 625
.if(n = 547 || n = 554 || n = 575 || n = 617)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 548 || n = 553 || n = 555 || n = 558 || n = 562 || n = 563 || n = 565 || n = 570 || n = 573 || n = 578 || n = 580 || n = 583 || n = 587 || n = 588 || n = 590 || n = 593 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 549 || n = 559 || n = 564 || n = 572 || n = 574 || n = 579 || n = 584 || n = 592 || n = 594 || n = 599 || n = 604 || n = 614 || n = 619)
	; 6..7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 567 || n = 597)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 627 || n = 628 || n = 630 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 668 || n = 670 || n = 675 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 693 || n = 695 || n = 698 || n = 700 || n = 703 || n = 707 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 747 || n = 752 || n = 753 || n = 758 || n = 760 || n = 763 || n = 765 || n = 768 || n = 770 || n = 773)
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
.elseif(n = 632 || n = 634 || n = 639 || n = 644 || n = 654 || n = 662 || n = 664 || n = 669 || n = 679 || n = 684 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 730 || n = 734 || n = 739 || n = 749 || n = 754 || n = 759 || n = 772 || n = 774 || n = 779)
	; 6..7 bytes
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
.elseif(n = 642 || n = 677 || n = 702 || n = 732 || n = 742 || n = 762 || n = 767)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 672)
	; 7 bytes
	ldx #(n-12)/11
	delay_n 6
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
.if((n >= 782) && (n < 860))
; 782 to 859
.if(n = 782)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.elseif(n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 810 || n = 818 || n = 822 || n = 823 || n = 828 || n = 830 || n = 833 || n = 838 || n = 840 || n = 843 || n = 845 || n = 858)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 795) || n = 804 || (n > 813 && n < 830) || n = 839 || n = 844 || n = 854 || n = 859))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 785 || n = 799 || n = 813 || n = 834 || n = 848 || n = 855)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 785 && n < 857)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 792 || n = 820 || n = 827)
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
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jmi *-3
.elseif(n = 802 || n = 847)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 807 || n = 812 || n = 842 || n = 852)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 809)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 815 || n = 850)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 817 || n = 825 || n = 849 || n = 857)
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
.elseif(n = 837)
	; 7 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-3
.elseif(n = 853)
	; 5 bytes
	ldx #122 ;hides 'nop'
	dex
	Jne *-2
.endif
.else
; 860 to 937
.if(n = 860)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
.elseif((n .mod 5 = 1) && ((n > 860 && n < 937)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 862 || n = 883 || n = 904)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 863 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 903 || n = 908 || n = 910 || n = 913 || n = 915 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && (n = 864 || (n > 873 && n < 885) || n = 894 || (n > 908 && n < 925) || n = 934))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 865 || n = 889 || n = 929)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 867)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 869 || n = 890 || n = 897 || n = 918 || n = 925 || n = 932)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 872 || n = 882 || n = 887 || n = 902 || n = 907)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 885 || n = 920)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
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
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 943 || n = 948 || n = 950 || n = 958 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1063 || n = 1067 || n = 1068 || n = 1073 || n = 1075 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1088 || n = 1090)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || n = 1093))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1092)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942 || n = 952 || n = 957 || n = 987 || n = 992 || n = 1012 || n = 1020 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1082 || n = 1087 || n = 1092)
	; 6..7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 945 || n = 1025 || n = 1049 || n = 1057 || n = 1089)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 947)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 955 || n = 990 || n = 1060)
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
.elseif(n = 982 || n = 1027)
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
.elseif(n = 1022)
	; 7 bytes
	tax
	pla
	txa
	delay_n (n-8)
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
.elseif(n = 1070)
	; 7 bytes
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && ((n > 1093 && n < 1110) || n = 1119 || (n > 1133 && n < 1145) || (n > 1153 && n < 1165) || n = 1174 || n = 1179 || (n > 1188 && n < 1205) || n = 1214 || (n > 1223 && n < 1245)))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1095 || n = 1130 || n = 1165 || n = 1200 || n = 1235)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 1) && ((n > 1095 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1097)
	; 6 bytes
	ldx #122 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1110 || n = 1113 || n = 1115 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1133 || n = 1138 || n = 1140 || n = 1143 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1158 || n = 1160 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1183 || n = 1188 || n = 1190 || n = 1193 || n = 1195 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1228 || n = 1230 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1105 || n = 1129 || n = 1137 || n = 1145 || n = 1169 || n = 1185 || n = 1209 || n = 1217 || n = 1225 || n = 1249)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 5 = 2) && (n = 1112 || (n > 1121 && n < 1133) || n = 1147 || n = 1157 || n = 1167 || n = 1182 || (n > 1191 && n < 1203) || (n > 1221 && n < 1233)))
	; 7 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 1152)
	; 7 bytes
	ldx #128 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1162)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 1187)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jpl *-4
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
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1263 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1291 || n = 1298 || n = 1307 || n = 1312 || n = 1314 || n = 1318 || n = 1319 || n = 1322 || n = 1323 || n = 1326 || n = 1328)
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290 || n = 1292 || n = 1300 || n = 1304 || n = 1308 || n = 1316 || n = 1320)
	; 6..8 bytes
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
.elseif(n = 1295)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	pha
	inx
	Jpl *-4
.elseif(n = 1299)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1301 || n = 1311)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1309)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1325)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 6
	inx
	Jpl *-4
.elseif(n = 1327)
	; 7 bytes
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
.endif
.else
; 1329 to 1406
.if((n .mod 8 = 1) && (n = 1329 || n = 1337 || (n > 1352 && n < 1378) || n = 1393))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1330 || n = 1334 || n = 1346 || n = 1348 || n = 1356 || n = 1362 || n = 1364 || n = 1372 || n = 1383 || n = 1388 || n = 1390 || n = 1397 || n = 1402 || n = 1404)
	; 6..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1402)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1332 || n = 1343)
	; 7 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-3
.elseif((n .mod 9 = 1) && ((n > 1332 && n < 1361) || n = 1378 || n = 1396 || n = 1405))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1335)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1336)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 1339)
	; 6 bytes
	ldx #122 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1340 || n = 1344 || n = 1347 || n = 1349 || n = 1350 || n = 1354 || n = 1355 || n = 1363 || n = 1368 || n = 1375 || n = 1379 || n = 1382 || n = 1384 || n = 1389 || n = 1395 || n = 1403)
	; 7..8 bytes
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
.elseif(n = 1358 || n = 1400)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1365 || n = 1398)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1367)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dex
	dex
	Jne *-5
.elseif(n = 1370)
	; 7 bytes
	ldx #137 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 1371 || n = 1391)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
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
	; 8 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	nop
	inx
	Jpl *-5
.elseif(n = 1392 || n = 1406)
	; 7 bytes
	php
	delay_n (n-7)
	plp
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
.if(n = 1407 || n = 1410 || n = 1416 || n = 1419 || n = 1424 || n = 1431 || n = 1434 || n = 1435 || n = 1437 || n = 1438 || n = 1440 || n = 1445 || n = 1447 || n = 1448 || n = 1452 || n = 1454 || n = 1462 || n = 1466 || n = 1467 || n = 1469 || n = 1470 || n = 1479 || n = 1480 || n = 1482 || n = 1483)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1479)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && ((n > 1408 && n < 1450) || (n > 1464 && n < 1482)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1411 || n = 1421 || n = 1451 || n = 1461)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1412 || n = 1418 || n = 1423 || n = 1428 || n = 1439 || n = 1444 || n = 1446 || n = 1458 || n = 1460 || n = 1474 || n = 1476 || n = 1484)
	; 6..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1413)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1414 || n = 1432 || n = 1459 || n = 1468 || n = 1477)
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
.elseif(n = 1426)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1427)
	; 7 bytes
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
.elseif(n = 1442 || n = 1475)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
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
	ldx #(n-16)/15
	delay_n 10
	dex
	Jpl *-4
.elseif(n = 1463)
	; 7 bytes
	ldx #122 ;hides 'nop'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 1472)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.endif
.else
; 1485 to 1562
.if((n .mod 7 = 1) && ((n > 1484 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && (n = 1486 || n = 1495 || (n > 1521 && n < 1559)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1487 || n = 1490 || n = 1494 || n = 1498 || n = 1501 || n = 1503 || n = 1507 || n = 1508 || n = 1510 || n = 1514 || n = 1515 || n = 1517 || n = 1523 || n = 1533 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1557)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488 || n = 1500 || n = 1502 || n = 1509 || n = 1516 || n = 1524 || n = 1528 || n = 1544)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && ((n > 1488 && n < 1506) || (n > 1520 && n < 1562)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1491 || n = 1511 || n = 1551)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
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
.elseif(n = 1504)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	nop
	inx
	Jne *-3
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
.elseif(n = 1519 || n = 1530 || n = 1552)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1526 || n = 1535)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1532)
	; 7 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1546)
	; 8 bytes
	ldx #(n-16)/15
	delay_n 10
	dex
	Jpl *-4
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
	; 6 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jne *-3
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1574 || n = 1596 || n = 1607 || n = 1629)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1564 || n = 1566 || n = 1570 || n = 1571 || n = 1578 || n = 1579 || n = 1586 || n = 1587 || n = 1589 || n = 1592 || n = 1595 || n = 1599 || n = 1602 || n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1619 || n = 1620 || n = 1622 || n = 1627 || n = 1634 || n = 1635)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1572 || n = 1580 || n = 1588 || n = 1600 || n = 1612 || n = 1614 || n = 1626 || n = 1628 || n = 1636 || n = 1640)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1567 || n = 1594 || n = 1603 || n = 1621)
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
.elseif(n = 1573)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1575 || n = 1616 || n = 1630)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 8 = 1) && ((n > 1576 && n < 1602) || n = 1617 || n = 1633))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1581 || n = 1591 || n = 1631)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1582 || n = 1615 || n = 1638)
	; 7..8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1584)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1598)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jmi *-4
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
.if((n .mod 8 = 1) && ((n > 1640 && n < 1674) || (n > 1688 && n < 1714)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1642 || n = 1652 || n = 1656 || n = 1658 || n = 1663 || n = 1668 || n = 1670 || n = 1677 || n = 1692 || n = 1698 || n = 1700 || n = 1708 || n = 1712)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1643 || n = 1647 || n = 1654 || n = 1662 || n = 1664 || n = 1669 || n = 1672 || n = 1676 || n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1699 || n = 1704 || n = 1710 || n = 1715 || n = 1718)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1644)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1645 || n = 1717)
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
.elseif((n .mod 9 = 1) && (n = 1648 || (n > 1665 && n < 1694) || n = 1711))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1650)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1651 || n = 1661 || n = 1671 || n = 1691 || n = 1701)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1655)
	; 6 bytes
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
.elseif(n = 1659)
	; 6 bytes
	ldx #90 ;hides 'nop'
	pha
	inx
	Jne *-3
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
.elseif(n = 1680 || n = 1687 || n = 1694 || n = 1703)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1696)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
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
.elseif(n = 1714)
	; 7 bytes
	ldx #100 ;hides 'nop zp'
	nop $48 ;hides 'pha'
	inx
	Jne *-4
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1724 || n = 1726 || n = 1733 || n = 1740 || n = 1748 || n = 1764 || n = 1775 || n = 1780 || n = 1782 || n = 1784 || n = 1796)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1720 || n = 1738 || n = 1747 || n = 1756 || n = 1774 || n = 1783 || n = 1792)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 8 = 1) && (n = 1721 || n = 1729 || (n > 1744 && n < 1786)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1722 || n = 1725 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1754 || n = 1755 || n = 1757 || n = 1762 || n = 1763 || n = 1766 || n = 1767 || n = 1770 || n = 1787 || n = 1788 || n = 1790 || n = 1795)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1727)
	; 7 bytes
	ldx #100 ;hides 'nop zp'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 1728)
	; 7 bytes
	pha
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	pla
.elseif(n = 1731 || n = 1771 || n = 1791)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1735 || n = 1736 || n = 1778)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1741 || n = 1789)
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
.elseif(n = 1768)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1773 || n = 1776)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
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
; 1797 to 1875
.if(n = 1797 || n = 1798 || n = 1803 || n = 1814 || n = 1815 || n = 1818 || n = 1822 || n = 1830 || n = 1834 || n = 1840 || n = 1843 || n = 1845 || n = 1848 || n = 1854 || n = 1859 || n = 1862 || n = 1866 || n = 1867 || n = 1869)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1800 || n = 1806 || n = 1823)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 8 = 1) && ((n > 1800 && n < 1874)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1802 || n = 1804 || n = 1812 || n = 1820 || n = 1829 || n = 1836 || n = 1842 || n = 1847 || n = 1852 || n = 1858 || n = 1868 || n = 1874)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1805 || n = 1827 || n = 1838 || n = 1860)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1807)
	; 8 bytes
	ldx #128
	delay_n 9
	dex
	Jpl *-4
.elseif(n = 1808)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1809 && n < 1865)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1811 || n = 1821 || n = 1831 || n = 1851 || n = 1871)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1813 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1816)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1824)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	nop
	dex
	Jne *-3
.elseif(n = 1826)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
.elseif(n = 1832)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1835)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1839)
	; 6 bytes
	ldx #184 ;hides 'clv'
	pha
	dex
	Jne *-3
.elseif(n = 1844)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1850)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1853)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1856)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
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
	delay_n 4
	nop
	inx
	Jne *-5
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
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
; 1876 to 1953
.if(n = 1876 || n = 1884 || n = 1888 || n = 1892 || n = 1896 || n = 1908 || n = 1910 || n = 1919 || n = 1924 || n = 1928 || n = 1930 || n = 1932 || n = 1940 || n = 1944)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1877)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1878 || n = 1883 || n = 1887 || n = 1894 || n = 1899 || n = 1923 || n = 1934 || n = 1935 || n = 1938 || n = 1939 || n = 1942 || n = 1947 || n = 1949 || n = 1952)
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
	ldx #68 ;hides 'nop zp'
	delay_n 4
	inx
	Jne *-4
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1954)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1937)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1885 || n = 1933)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1886 || n = 1912)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1890)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 1892 && n < 1927) || n = 1948))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1895 || n = 1903 || n = 1922 || n = 1943)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1898)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1901 || n = 1911 || n = 1931 || n = 1941 || n = 1951)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
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
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1914)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1916)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1917)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1920)
	; 7 bytes
	ldx #192 ;hides 'cpy #imm'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 1925)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1946)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1950)
	; 7 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
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
.elseif(n = 1955 || n = 1958 || n = 1962 || n = 1965 || n = 1974 || n = 1983 || n = 1987)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1956 || n = 1982 || n = 1984 || n = 1988)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1957)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1959)
	; 7 bytes
	ldx #196 ;hides 'cpy zp'
	cmp $EA ;hides 'nop'
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
	; 7 bytes
	pha
	ldx #245
	pha
	dex
	Jne *-2
.elseif(n = 1966)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
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
	ldx #197 ;hides 'cmp zp'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1971 || n = 1991)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1973)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1975 || n = 1989)
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
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1979)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1980)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1992)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.endif
.else
; 1993 to 2031
.if((n .mod 8 = 1) && ((n > 1992 && n < 2026)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1994 || n = 1995 || n = 1997 || n = 1998 || n = 2006 || n = 2007 || n = 2018 || n = 2019 || n = 2022 || n = 2027 || n = 2030)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1996 || n = 2002 || n = 2004 || n = 2012 || n = 2020 || n = 2024 || n = 2028)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1999)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pha
	inx
	Jne *-3
.elseif(n = 2000)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
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
.elseif(n = 2008 || n = 2026)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2010)
	; 7 bytes
	ldx #201 ;hides 'cmp #imm'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2011 || n = 2021 || n = 2031)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2013)
	; 8 bytes
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
.elseif(n = 2016)
	; 7 bytes
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2023)
	; 7 bytes
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
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
.if(n = 2032 || n = 2050 || n = 2052)
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
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2037 || n = 2038 || n = 2043 || n = 2046 || n = 2051 || n = 2054 || n = 2055 || n = 2060 || n = 2064 || n = 2070)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2039)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2040 || n = 2048 || n = 2056)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2042)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
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
.elseif(n = 2057)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2061)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
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
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2080 || n = 2089 || n = 2107)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2072 || n = 2074 || n = 2084 || n = 2090 || n = 2092 || n = 2108)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2073 || n = 2082 || n = 2083 || n = 2093 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
	; 8 bytes
	delay_n (n-2)
	nop
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
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2081 || n = 2091)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
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
.elseif(n = 2088 || n = 2095 || n = 2096 || n = 2106)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2097)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	inx
	Jne *-5
.elseif(n = 2098)
	; 7 bytes
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2099)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	delay_n 6
	inx
	Jne *-5
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
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2117 || n = 2119 || n = 2126 || n = 2128 || n = 2138 || n = 2140 || n = 2144)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 10 = 1) && ((n > 2110 && n < 2142)))
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
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
.elseif(n = 2115 || n = 2129)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2115 && n < 2144)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2118 || n = 2127 || n = 2136 || n = 2139 || n = 2148)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2120 || n = 2133)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2122 || n = 2142)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2123)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 2124 || n = 2135 || n = 2146)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 2145)
	; 7 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	pha
	dex
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
.elseif(n = 2150 || n = 2160 || n = 2164 || n = 2174 || n = 2182 || n = 2184)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2151 || n = 2171 || n = 2181)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2152 || n = 2161 || n = 2170)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2153 || n = 2158 || n = 2163 || n = 2169 || n = 2172 || n = 2175 || n = 2180 || n = 2183 || n = 2187)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2154)
	; 7 bytes
	ldx #196 ;hides 'cpy zp'
	cmp $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2155)
	; 7 bytes
	ldx #196 ;hides 'cpy zp'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2156)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2157 || n = 2168)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2159)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2162)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2165)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2166)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2167)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2176)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2177)
	; 7 bytes
	ldx #58 ;hides 'nop'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2178)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	pha
	dex
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
.elseif(n = 2189 || n = 2194 || n = 2198 || n = 2218)
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
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2192 || n = 2202 || n = 2203 || n = 2208 || n = 2214 || n = 2217 || n = 2225 || n = 2226)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2193)
	; 8 bytes
	php
	delay_n (n-7)
	plp
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
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 2205 || n = 2216 || n = 2219)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2207)
	; 7 bytes
	ldx #184 ;hides 'clv'
	nop
	pha
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
.elseif(n = 2212 || n = 2223)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
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
.elseif((n > 2227 && n < 2231) || n = 2265)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2231 || n = 2261)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2232)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 2232 && n < 2261)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2234 || n = 2256)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2235 || n = 2246 || n = 2252 || n = 2264)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2236)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2237 || n = 2263)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2238)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2239)
	; 7 bytes
	ldx #224 ;hides 'cpx #imm'
	cmp $48 ;hides 'pha'
	dex
	Jne *-4
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
.elseif(n = 2244 || n = 2247 || n = 2253 || n = 2262)
	; 8 bytes
	delay_n (n-2)
	nop
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
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
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
.elseif(n = 2270)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2271 || n = 2291 || n = 2301)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2272 || n = 2273 || n = 2282 || n = 2288 || n = 2292 || n = 2298)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2274 || n = 2284 || n = 2302)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2275)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2279)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2280)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	delay_n 4
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
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2290)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
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
	; 6 bytes
	ldx #26 ;hides 'nop'
	pha
	inx
	Jne *-3
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
.elseif(n = 2306 || n = 2307 || n = 2313 || n = 2324 || n = 2332 || n = 2335 || n = 2343)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2314 || n = 2336)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2309 || n = 2310 || n = 2312 || n = 2316 || n = 2318 || n = 2337 || n = 2338)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2311 || n = 2321)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2315 || n = 2328)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2317 || n = 2329 || n = 2341)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 2322 || n = 2333)
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
	ldx #194 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	nop
	dex
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
.elseif(n = 2334)
	; 8 bytes
	pha
	ldx #233
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2339)
	; 6 bytes
	ldx #234 ;hides 'nop'
	pha
	dex
	Jne *-3
.elseif(n = 2340)
	; 8 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	delay_n 4
	dex
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
.if(n = 2344 || n = 2366 || n = 2388 || n = 2399 || n = 2410)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2345)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2346 || n = 2357 || n = 2368 || n = 2373 || n = 2379 || n = 2383 || n = 2390 || n = 2403 || n = 2412 || n = 2415)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347 || n = 2358 || n = 2369 || n = 2374 || n = 2378 || n = 2384 || n = 2392 || n = 2394 || n = 2400 || n = 2402 || n = 2404 || n = 2414)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n > 2347 && n < 2351) || n = 2359 || n = 2360 || n = 2362 || n = 2370 || n = 2372 || n = 2382 || n = 2385 || n = 2387 || n = 2396 || n = 2398 || n = 2408 || n = 2417)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 10 = 1) && ((n > 2350 && n < 2392) || n = 2411 || n = 2421))
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
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
.elseif(n = 2354 || n = 2380 || n = 2393 || n = 2406)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2355)
	; 7 bytes
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2356 || n = 2386 || n = 2416)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
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
	delay_n 4
	nop
	dex
	Jne *-5
.elseif(n = 2367 || n = 2395)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2375)
	; 7 bytes
	ldx #58 ;hides 'nop'
	nop
	pha
	inx
	Jne *-4
.elseif(n = 2376)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2397)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2405)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 2407)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	delay_n 4
	pha
	inx
	Jne *-5
.elseif(n = 2409)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2418)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2419)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2420)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	nop
	pha
	inx
	Jne *-4
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2427 || n = 2433 || n = 2434 || n = 2439 || n = 2453 || n = 2456 || n = 2463 || n = 2467 || n = 2475 || n = 2478 || n = 2483 || n = 2489 || n = 2500)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423 || n = 2479 || n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2424 || n = 2428 || n = 2438 || n = 2440 || n = 2444 || n = 2448 || n = 2450 || n = 2452 || n = 2457 || n = 2462 || n = 2468 || n = 2474 || n = 2488 || n = 2490 || n = 2494)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 1) && ((n > 2424 && n < 2498)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2426 || n = 2429 || n = 2430 || n = 2436 || n = 2455 || n = 2460 || n = 2466 || n = 2469 || n = 2470 || n = 2472 || n = 2477 || n = 2480 || n = 2482 || n = 2492)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2431 || n = 2441 || n = 2451 || n = 2471 || n = 2481 || n = 2491)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif((n .mod 11 = 1) && ((n > 2431 && n < 2499)))
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
.elseif(n = 2442)
	; 8 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 2458 || n = 2484)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2459)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2464)
	; 8 bytes
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2486)
	; 8 bytes
	ldx #n/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2495)
	; 8 bytes
	ldx #192 ;hides 'cpy #imm'
	cmp $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2496)
	; 8 bytes
	ldx #192 ;hides 'cpy #imm'
	nop $48,X ;hides 'pha'
	pha
	dex
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
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2502 || n = 2504 || n = 2512 || n = 2514 || n = 2524 || n = 2532 || n = 2534)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2503 || n = 2513 || n = 2522 || n = 2523)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2505)
	; 8 bytes
	ldx #232 ;hides 'inx'
	nop
	pha
	inx
	inx
	Jne *-5
.elseif(n = 2506)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	bit $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2507)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2508)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2509 || n = 2521 || n = 2533)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2510 || n = 2536)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif((n > 2514 && n < 2519) || (n > 2524 && n < 2529) || n = 2530 || (n > 2536 && n < 2540))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2519)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2520)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2529)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2535)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
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
	nop
	pha
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
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2544 || n = 2547 || n = 2559 || n = 2571)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2545 || n = 2557 || n = 2569)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2546 || n = 2548 || n = 2556 || n = 2558 || n = 2560 || n = 2570 || n = 2572 || n = 2576)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2549 || n = 2563 || n = 2577)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2550 || n = 2552 || n = 2565 || n = 2568)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2551)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2554)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2555)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2561)
	; 7 bytes
	ldx #0
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2562)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2566)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2567)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	pha
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
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	pha
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
	ldx #234
	delay_n 6
	dex
	Jne *-3
.endif
.endif
.else
; 2579 to 2656
.if(n = 2579)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2580 || n = 2582 || n = 2585 || n = 2590 || n = 2592 || n = 2603 || n = 2604 || n = 2616 || (n > 2622 && n < 2626) || (n > 2633 && n < 2638) || n = 2642 || n = 2645 || n = 2648 || n = 2649)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 12 = 1) && ((n > 2580 && n < 2654)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2583 || n = 2596 || n = 2607 || n = 2610 || n = 2614 || n = 2621 || n = 2631 || n = 2632 || n = 2643 || n = 2654 || n = 2655)
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
.elseif((n .mod 11 = 1) && ((n > 2585 && n < 2631) || n = 2652))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2587 || n = 2589 || n = 2598 || n = 2600 || n = 2602 || n = 2609 || n = 2618 || n = 2620 || n = 2622 || n = 2644)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2588 || n = 2601 || n = 2627)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2591 || n = 2633 || n = 2647)
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
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-4
.elseif(n = 2606)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2611 || n = 2626 || n = 2656)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2612)
	; 7 bytes
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2613)
	; 8 bytes
	ldx #201 ;hides 'cmp #imm'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2615)
	; 7 bytes
	ldx #218 ;hides 'nop'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2628)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
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
	delay_n 4
	nop
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if((n > 2656 && n < 2661) || n = 2662 || n = 2669 || n = 2670 || n = 2673 || n = 2678 || n = 2681 || n = 2690 || n = 2693 || n = 2695 || n = 2700 || n = 2702 || n = 2706 || n = 2708 || n = 2714 || n = 2719 || n = 2722 || n = 2723 || n = 2726 || n = 2733 || n = 2734)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2661 || n = 2675 || n = 2703 || n = 2717 || n = 2731)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2662 && n < 2730)))
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
.elseif((n .mod 12 = 1) && ((n > 2664 && n < 2726)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2666 || n = 2692 || n = 2705)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2667 || n = 2676 || n = 2679 || n = 2687 || n = 2691 || n = 2698 || n = 2709 || n = 2715 || n = 2720 || n = 2727)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2668 || n = 2680 || n = 2688 || n = 2697 || n = 2699 || n = 2704 || n = 2710 || n = 2721 || n = 2728 || n = 2732)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2671 || n = 2686 || n = 2716)
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
.elseif(n = 2711)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	nop
	dex
	Jne *-5
.elseif(n = 2712)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	pha
	dex
	Jne *-5
.elseif(n = 2724)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2730)
	; 8 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2736)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	delay_n 4
	nop
	dex
	Jne *-5
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
.elseif(n = 2739 || n = 2753 || n = 2763 || n = 2764)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2740 || n = 2751 || n = 2762)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2741 || n = 2747 || n = 2766 || n = 2769)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2742)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2743 || n = 2752 || n = 2765)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2744 || n = 2757 || n = 2770)
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
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2748)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2750)
	; 8 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	nop
	inx
	Jne *-5
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
.elseif(n = 2756)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2758)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 2759)
	; 7 bytes
	ldx #26 ;hides 'nop'
	nop
	pha
	inx
	Jne *-4
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
	delay_n 4
	nop
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
.if(n = 2774)
	; 8 bytes
	pha
	ldx #232 ;hides 'inx'
	pha
	dex
	dex
	Jne *-4
.elseif(n = 2775 || n = 2786 || n = 2808 || n = 2811)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2776 || n = 2791)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n > 2776 && n < 2781) || n = 2789 || n = 2790 || n = 2792 || n = 2803)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
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
	; 7 bytes
	ldx #24 ;hides 'clc'
	nop
	pha
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
.elseif(n = 2788 || n = 2798 || n = 2800 || n = 2802 || n = 2810 || n = 2812)
	; 8 bytes
	pha
	delay_n (n-3)
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
	; 7 bytes
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
; 2813 to 2890
.if(n = 2813 || n = 2814 || n = 2818 || (n > 2824 && n < 2829) || (n > 2836 && n < 2841) || n = 2842 || n = 2849 || n = 2850 || n = 2852 || n = 2856 || n = 2862 || n = 2864 || n = 2865 || n = 2867 || n = 2868 || n = 2870 || (n > 2874 && n < 2881) || n = 2882 || n = 2886 || (n > 2887 && n < 2891))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2815 || n = 2885)
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
.elseif((n .mod 12 = 1) && ((n > 2820 && n < 2858) || n = 2881))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 13 = 1) && (n = 2822 || (n > 2847 && n < 2888)))
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif((n .mod 12 = 3) && ((n > 2822 && n < 2884)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2824 || n = 2832 || n = 2834 || n = 2844 || n = 2846 || n = 2860 || n = 2872 || n = 2884)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2836 || n = 2851 || n = 2866)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2841)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
.elseif(n = 2843)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
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
	delay_n 4
	pha
	inx
	Jne *-5
.elseif(n = 2858)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 2863)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
.elseif(n = 2869)
	; 7 bytes
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2873)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	pha
	inx
	Jne *-5
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2892 || n = 2894 || n = 2897 || n = 2898 || (n > 2900 && n < 2905) || n = 2906 || n = 2909 || n = 2910 || n = 2914 || n = 2916 || (n > 2920 && n < 2925) || (n > 2932 && n < 2937) || (n > 2944 && n < 2951) || n = 2954 || (n > 2956 && n < 2962))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 12 = 1) && ((n > 2892 && n < 2966)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 12 = 3) && ((n > 2894 && n < 2944) || n = 2967))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2896 || n = 2911 || n = 2956)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2899 || n = 2913 || n = 2955)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2900 || n = 2952)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2908 || n = 2918 || n = 2920 || n = 2930 || n = 2932 || n = 2942 || n = 2944 || n = 2968)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2912)
	; 8 bytes
	ldx #224 ;hides 'cpx #imm'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2915)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	dex
	Jne *-4
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
	ldx #12 ;hides 'nop abs'
	nop $0
	pha
	inx
	Jne *-5
.elseif(n = 2937)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2938)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2939)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2940)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 2951)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	nop
	pha
	dex
	Jne *-5
.elseif(n = 2962)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	bit $48 ;hides 'pha'
	nop
	dex
	Jne *-5
.elseif(n = 2963)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	pha
	dex
	Jne *-5
.elseif(n = 2964)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	delay_n 4
	pha
	dex
	Jne *-5
.elseif(n = 2966)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	dex
	Jne *-5
.endif
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
.elseif(n = 2970)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 2971 || n = 2986 || n = 3016 || n = 3031 || n = 3046)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n > 2971 && n < 2977) || n = 2982 || n = 2984 || n = 2985 || n = 2990 || n = 2993 || n = 2994 || n = 2996 || n = 2998 || (n > 3004 && n < 3011) || n = 3014 || (n > 3018 && n < 3023) || n = 3026 || n = 3029 || (n > 3031 && n < 3035) || n = 3038 || n = 3042 || n = 3045)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 12 = 1) && ((n > 2976 && n < 3038)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 13 = 1) && (n = 2978 || (n > 3003 && n < 3044)))
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2979 || n = 2991 || n = 3003 || n = 3027)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2980 || n = 2992 || n = 3018 || n = 3028 || n = 3040)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2981)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	pha
	dex
	Jne *-5
.elseif(n = 2987)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	pha
	inx
	Jne *-5
.elseif(n = 2988)
	; 8 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	pha
	inx
	Jne *-5
.elseif(n = 2995)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	nop
	pha
	dex
	Jne *-5
.elseif(n = 2999)
	; 7 bytes
	ldx #250 ;hides 'nop'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 3000)
	; 8 bytes
	cmp <$CD ;hides 'cmp $06A2'
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
.elseif(n = 3012)
	; 8 bytes
	cmp <$CD ;hides 'cmp $05A2'
	ldx #5
	pha
	inx
	Jne *-5
.elseif(n = 3015)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-4
.elseif(n = 3023)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $04A2'
	ldx #4
	pha
	inx
	Jne *-5
.elseif(n = 3024)
	; 8 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	pha
	inx
	Jne *-5
.elseif(n = 3035)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $03A2'
	ldx #3
	pha
	inx
	Jne *-5
.elseif(n = 3036)
	; 8 bytes
	cmp <$CD ;hides 'cmp $03A2'
	ldx #3
	pha
	inx
	Jne *-5
.elseif(n = 3041)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3044)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
.endif
.else
; 3047 to 3125
.if(n = 3047 || n = 3052 || n = 3064 || n = 3076)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3048)
	; 8 bytes
	cmp <$CD ;hides 'cmp $02A2'
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
.elseif(n = 3050 || n = 3055 || n = 3062 || n = 3065 || n = 3068 || n = 3070 || n = 3077 || (n > 3082 && n < 3091) || n = 3092 || n = 3093 || (n > 3095 && n < 3106) || n = 3107 || (n > 3109 && n < 3121) || n = 3122 || n = 3124 || n = 3125)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3051)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && ((n > 3052 && n < 3124)))
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
.elseif(n = 3056 || n = 3069 || n = 3082 || n = 3108 || n = 3121)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3057)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
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
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	pha
	inx
	Jne *-5
.elseif(n = 3060)
	; 8 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	pha
	inx
	Jne *-5
.elseif(n = 3063 || n = 3075)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3066)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 3071)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $00A2'
	ldx #0
	pha
	inx
	Jne *-5
.elseif(n = 3072)
	; 8 bytes
	cmp <$CD ;hides 'cmp $00A2'
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
.elseif(n = 3078)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	pha
	inx
	Jne *-5
.elseif(n = 3079)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	nop $48,X ;hides 'pha'
	pha
	inx
	Jne *-5
.elseif(n = 3080)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 3091 || n = 3106)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3094)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126)
	; 7 bytes
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3127)
	; 8 bytes
	ldx #184 ;hides 'clv'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3128)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3129)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n > 3129 && n < 3134) || n = 3135 || (n > 3137 && n < 3147) || (n > 3147 && n < 3151) || (n > 3151 && n < 3158) || n = 3159 || (n > 3160 && n < 3164) || n = 3168 || n = 3169 || n = 3172 || (n > 3173 && n < 3177) || n = 3180 || n = 3182 || n = 3183 || n = 3185 || (n > 3186 && n < 3190) || n = 3195 || n = 3197 || n = 3198 || (n > 3199 && n < 3204))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 13 = 1) && ((n > 3133 && n < 3187)))
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3136 || n = 3166 || n = 3181 || n = 3196)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && ((n > 3136 && n < 3194)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3158)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3164)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
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
	delay_n 4
	nop
	dex
	Jne *-5
.elseif(n = 3177)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3178)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3184)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3190)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	bit $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3191)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3192)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3194)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
.endif
.else
; 3204 to 3281
.if(n = 3204 || n = 3205 || n = 3208 || n = 3210 || (n > 3212 && n < 3219) || n = 3220 || n = 3223 || n = 3224 || (n > 3226 && n < 3233) || n = 3234 || n = 3236 || n = 3237 || n = 3239 || n = 3240 || (n > 3241 && n < 3247) || n = 3248 || (n > 3252 && n < 3256) || (n > 3256 && n < 3263) || (n > 3264 && n < 3269) || n = 3270 || (n > 3271 && n < 3275) || n = 3276 || n = 3280 || n = 3281)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3206)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3207 || n = 3221 || n = 3235 || n = 3263 || n = 3277)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3209)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif((n .mod 15 = 1) && ((n > 3210 && n < 3272)))
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n .mod 13 = 1) && ((n > 3211 && n < 3265)))
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3219)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3222 || n = 3250 || n = 3252 || n = 3278)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3233)
	; 8 bytes
	ldx #232 ;hides 'inx'
	nop
	pha
	dex
	dex
	Jne *-5
.elseif(n = 3247)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3249)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3269)
	; 8 bytes
	ldx #218 ;hides 'nop'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
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
.if((n > 3281 && n < 3285) || n = 3287 || n = 3288 || (n > 3291 && n < 3300) || n = 3304 || (n > 3305 && n < 3315) || n = 3318 || (n > 3319 && n < 3329) || n = 3330 || n = 3332 || (n > 3333 && n < 3346) || (n > 3347 && n < 3360))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3285)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3286 || n = 3301 || n = 3331 || n = 3346)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3289)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	pha
	inx
	Jne *-5
.elseif(n = 3290 || n = 3303 || n = 3316)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && ((n > 3290 && n < 3348)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3300)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3302)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 3315)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3317)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	pha
	inx
	Jne *-5
.elseif(n = 3329)
	; 8 bytes
	ldx #0
	nop
	delay_n 6
	dex
	Jne *-4
.endif
.else
; 3360 to 3437
.if(n = 3360 || (n > 3361 && n < 3365) || (n > 3365 && n < 3375) || (n > 3376 && n < 3383) || n = 3384 || n = 3387 || n = 3388 || n = 3390 || (n > 3391 && n < 3399) || n = 3400 || n = 3402 || n = 3404 || (n > 3407 && n < 3413) || n = 3416 || n = 3418 || n = 3419 || n = 3422 || (n > 3423 && n < 3429) || n = 3430 || (n > 3431 && n < 3435) || n = 3437)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 14 = 1) && ((n > 3360 && n < 3432)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3365)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3376 || n = 3406 || n = 3421 || n = 3436)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3383)
	; 7 bytes
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3385)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3386)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3391)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3401)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	pha
	dex
	Jne *-5
.elseif(n = 3405)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3407)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 9
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
	delay_n 4
	pha
	dex
	Jne *-5
.elseif(n = 3420)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3423)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3429)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	pha
	dex
	Jne *-5
.elseif(n = 3435)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
; 3438 to 3593
.if((n > 3437 && n < 3445) || (n > 3445 && n < 3449) || (n > 3451 && n < 3459) || (n > 3459 && n < 3464) || n = 3465 || (n > 3466 && n < 3473) || (n > 3473 && n < 3479) || n = 3480 || (n > 3481 && n < 3487) || (n > 3487 && n < 3496) || n = 3497 || n = 3498 || n = 3500 || (n > 3502 && n < 3509) || n = 3510 || (n > 3511 && n < 3515) || (n > 3515 && n < 3524) || n = 3525 || n = 3527 || n = 3528 || (n > 3529 && n < 3537) || n = 3540 || n = 3542 || (n > 3543 && n < 3554) || n = 3555 || (n > 3557 && n < 3571) || (n > 3571 && n < 3585) || (n > 3586 && n < 3594))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 14 = 1) && ((n > 3444 && n < 3572)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3449)
	; 8 bytes
	ldx #26 ;hides 'nop'
	nop
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3450)
	; 8 bytes
	ldx #202 ;hides 'dex'
	.repeat 4
	 pha
	.endrepeat
	Jne *-5
.elseif((n .mod 15 = 1) && ((n > 3450 && n < 3557) || n = 3586))
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3464)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-5
.elseif(n = 3479)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3499)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3502)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3509)
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3524)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3537)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	inx
	Jne *-5
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
.elseif(n = 3554)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.endif
.else
; 3594 to 3750
.if((n > 3593 && n < 3601) || (n > 3601 && n < 3616) || (n > 3616 && n < 3631) || (n > 3631 && n < 3640) || n = 3641 || n = 3645 || (n > 3646 && n < 3653) || (n > 3653 && n < 3657) || n = 3660 || (n > 3661 && n < 3674) || n = 3675 || n = 3677 || n = 3678 || (n > 3679 && n < 3691) || (n > 3691 && n < 3695) || (n > 3695 && n < 3705) || (n > 3706 && n < 3711) || (n > 3711 && n < 3721) || (n > 3721 && n < 3736) || (n > 3736 && n < 3743) || (n > 3743 && n < 3749) || n = 3750)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 15 = 1) && ((n > 3600 && n < 3737)))
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3640)
	; 7 bytes
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3642)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3643)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3644)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3653)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
.elseif(n = 3657)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3658)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3659)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3674)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3679)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3695)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 3705)
	; 8 bytes
	ldx #218 ;hides 'nop'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3711)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
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
.endif
.endif
.endif
.endif
.else
; 3751 to 5000
.if((n .mod 15 = 1) && ((n > 3750 && n < 3827)))
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n > 3751 && n < 3759) || (n > 3759 && n < 3766) || (n > 3766 && n < 3775) || (n > 3775 && n < 3781) || (n > 3781 && n < 3791) || (n > 3791 && n < 3796) || (n > 3796 && n < 3811) || (n > 3811 && n < 3826) || (n > 3826 && n < 3841) || (n > 3841 && n < 3856) || (n > 3856 && n < 3887) || (n > 3887 && n < 3897) || n = 3898 || n = 3901 || n = 3902 || (n > 3903 && n < 3909) || (n > 3909 && n < 3919) || (n > 3919 && n < 3943) || (n > 3943 && n < 3977) || (n > 3977 && n < 3999) || (n > 3999 && n < 4154) || n = 4155 || (n > 4157 && n < 4249) || (n > 4249 && n < 4411) || n = 4412 || (n > 4414 && n < 4668) || n = 4669 || (n > 4671 && n < 4925) || n = 4926 || (n > 4928 && n < 5001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3759)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3775)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3791)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3841)
	; 8 bytes
	ldx #0
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3856)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
.elseif(n = 3887)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3897)
	; 7 bytes
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3899 || n = 4156 || n = 4413 || n = 4670 || n = 4927)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3900 || n = 4157 || n = 4414 || n = 4671 || n = 4928)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3903)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3909)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3919)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3943)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3977)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3999)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 4154)
	; 7 bytes
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4249)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 4411)
	; 7 bytes
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4668)
	; 7 bytes
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4925)
	; 7 bytes
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5182) || n = 5183 || (n > 5185 && n < 5439) || n = 5440 || (n > 5442 && n < 5696) || n = 5697 || (n > 5699 && n < 5953) || n = 5954 || (n > 5956 && n < 6210) || n = 6211 || (n > 6213 && n < 6467) || n = 6468 || (n > 6470 && n < 6724) || n = 6725 || (n > 6727 && n < 6981) || n = 6982 || (n > 6984 && n < 7238) || n = 7239 || (n > 7241 && n < 7495) || n = 7496 || (n > 7498 && n < 7752) || n = 7753 || (n > 7755 && n < 8009) || n = 8010 || (n > 8012 && n < 8266) || n = 8267 || (n > 8269 && n < 8523) || n = 8524 || (n > 8526 && n < 8780) || n = 8781 || (n > 8783 && n < 9037) || n = 9038 || (n > 9040 && n < 9294) || n = 9295 || (n > 9297 && n < 9551) || n = 9552 || (n > 9554 && n < 9808) || n = 9809 || (n > 9811 && n < 10001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5182)
	; 7 bytes
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5184 || n = 5441 || n = 5698 || n = 5955 || n = 6212 || n = 6469 || n = 6726 || n = 6983 || n = 7240 || n = 7497 || n = 7754 || n = 8011 || n = 8268 || n = 8525 || n = 8782 || n = 9039 || n = 9296 || n = 9553 || n = 9810)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5185 || n = 5442 || n = 5699 || n = 5956 || n = 6213 || n = 6470 || n = 6727 || n = 6984 || n = 7241 || n = 7498 || n = 7755 || n = 8012 || n = 8269 || n = 8526 || n = 8783 || n = 9040 || n = 9297 || n = 9554 || n = 9811)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5439)
	; 7 bytes
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5696)
	; 7 bytes
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5953)
	; 7 bytes
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6210)
	; 7 bytes
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6467)
	; 7 bytes
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6724)
	; 7 bytes
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6981)
	; 7 bytes
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7238)
	; 7 bytes
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7495)
	; 7 bytes
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7752)
	; 7 bytes
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8009)
	; 7 bytes
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8266)
	; 7 bytes
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8523)
	; 7 bytes
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8780)
	; 7 bytes
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9037)
	; 7 bytes
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9294)
	; 7 bytes
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9551)
	; 7 bytes
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9808)
	; 7 bytes
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10065) || n = 10066 || (n > 10068 && n < 10322) || n = 10323 || (n > 10325 && n < 10579) || n = 10580 || (n > 10582 && n < 10836) || n = 10837 || (n > 10839 && n < 11093) || n = 11094 || (n > 11096 && n < 11350) || n = 11351 || (n > 11353 && n < 11607) || n = 11608 || (n > 11610 && n < 11864) || n = 11865 || (n > 11867 && n < 12121) || n = 12122 || (n > 12124 && n < 12378) || n = 12379 || (n > 12381 && n < 12635) || n = 12636 || (n > 12638 && n < 12892) || n = 12893 || (n > 12895 && n < 13149) || n = 13150 || (n > 13152 && n < 13406) || n = 13407 || (n > 13409 && n < 13663) || n = 13664 || (n > 13666 && n < 13920) || n = 13921 || (n > 13923 && n < 14177) || n = 14178 || (n > 14180 && n < 14434) || n = 14435 || (n > 14437 && n < 14691) || n = 14692 || (n > 14694 && n < 14948) || n = 14949 || (n > 14951 && n < 15001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10065)
	; 7 bytes
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10067 || n = 10324 || n = 10581 || n = 10838 || n = 11095 || n = 11352 || n = 11609 || n = 11866 || n = 12123 || n = 12380 || n = 12637 || n = 12894 || n = 13151 || n = 13408 || n = 13665 || n = 13922 || n = 14179 || n = 14436 || n = 14693 || n = 14950)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10068 || n = 10325 || n = 10582 || n = 10839 || n = 11096 || n = 11353 || n = 11610 || n = 11867 || n = 12124 || n = 12381 || n = 12638 || n = 12895 || n = 13152 || n = 13409 || n = 13666 || n = 13923 || n = 14180 || n = 14437 || n = 14694 || n = 14951)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10322)
	; 7 bytes
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10579)
	; 7 bytes
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10836)
	; 7 bytes
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11093)
	; 7 bytes
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11350)
	; 7 bytes
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11607)
	; 7 bytes
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11864)
	; 7 bytes
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12121)
	; 7 bytes
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12378)
	; 7 bytes
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12635)
	; 7 bytes
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12892)
	; 7 bytes
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13149)
	; 7 bytes
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13406)
	; 7 bytes
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13663)
	; 7 bytes
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13920)
	; 7 bytes
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14177)
	; 7 bytes
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14434)
	; 7 bytes
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14691)
	; 7 bytes
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14948)
	; 7 bytes
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15205) || n = 15206 || (n > 15208 && n < 15462) || n = 15463 || (n > 15465 && n < 15719) || n = 15720 || (n > 15722 && n < 15976) || n = 15977 || (n > 15979 && n < 16233) || n = 16234 || (n > 16236 && n < 16490) || n = 16491 || (n > 16493 && n < 16747) || n = 16748 || (n > 16750 && n < 17004) || n = 17005 || (n > 17007 && n < 17261) || n = 17262 || (n > 17264 && n < 17518) || n = 17519 || (n > 17521 && n < 17775) || n = 17776 || (n > 17778 && n < 18032) || n = 18033 || (n > 18035 && n < 18289) || n = 18290 || (n > 18292 && n < 18546) || n = 18547 || (n > 18549 && n < 18803) || n = 18804 || (n > 18806 && n < 19060) || n = 19061 || (n > 19063 && n < 19317) || n = 19318 || (n > 19320 && n < 19574) || n = 19575 || (n > 19577 && n < 19831) || n = 19832 || (n > 19834 && n < 20001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15205)
	; 7 bytes
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15207 || n = 15464 || n = 15721 || n = 15978 || n = 16235 || n = 16492 || n = 16749 || n = 17006 || n = 17263 || n = 17520 || n = 17777 || n = 18034 || n = 18291 || n = 18548 || n = 18805 || n = 19062 || n = 19319 || n = 19576 || n = 19833)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15208 || n = 15465 || n = 15722 || n = 15979 || n = 16236 || n = 16493 || n = 16750 || n = 17007 || n = 17264 || n = 17521 || n = 17778 || n = 18035 || n = 18292 || n = 18549 || n = 18806 || n = 19063 || n = 19320 || n = 19577 || n = 19834)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15462)
	; 7 bytes
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15719)
	; 7 bytes
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15976)
	; 7 bytes
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16233)
	; 7 bytes
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16490)
	; 7 bytes
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16747)
	; 7 bytes
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17004)
	; 7 bytes
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17261)
	; 7 bytes
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17518)
	; 7 bytes
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17775)
	; 7 bytes
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18032)
	; 7 bytes
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18289)
	; 7 bytes
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18546)
	; 7 bytes
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18803)
	; 7 bytes
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19060)
	; 7 bytes
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19317)
	; 7 bytes
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19574)
	; 7 bytes
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19831)
	; 7 bytes
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.endif

.endif
	rest
.endmacro
