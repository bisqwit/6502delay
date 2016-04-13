; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Y, Z+N, C, and V
; Will preserve A, S, D, and I (interrupt-disable flag)
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).
;
; Requires that a label @nearby_rts_14cyc is defined,
; that points _somewhere_ that contains a dummy 2-cycle
; opcode, such as NOP or LDA #1, followed by a $60 byte (RTS).
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
; - 5 bytes or less when n <= 19
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 291
; - 8 bytes or less when n <= 1301
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.5, max=6
; - When 21 <= n <= 30: min=5, avg=6.1, max=7
; - When 31 <= n <= 50: min=4, avg=6.0, max=7
; - When 51 <= n <= 75: min=5, avg=6.1, max=7
; - When 76 <= n <= 100: min=5, avg=6.2, max=7
; - When 101 <= n <= 200: min=4, avg=6.2, max=7
; - When 201 <= n <= 300: min=4, avg=6.2, max=8
; - When 301 <= n <= 500: min=4, avg=6.3, max=8
; - When 501 <= n <= 750: min=4, avg=6.2, max=8
; - When 751 <= n <= 1000: min=5, avg=6.3, max=8
; - When 1001 <= n <= 2000: min=4, avg=7.0, max=9
; - When 2001 <= n <= 3000: min=6, avg=8.2, max=9
; - When 3001 <= n <= 5000: min=7, avg=8.8, max=9
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
.elseif((n > 3 && n < 7) || n = 16 || n = 18 || n = 20 || n = 23 || n = 25 || n = 28 || n = 30 || n = 33 || n = 38)
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
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15 || n = 24)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
.elseif(n = 34)
	; 5 bytes
	ldy #136 ;hides 'dey'
	dey
	Jmi *-2
.elseif(n = 35)
	; 6 bytes
	ldy #136 ;hides 'dey'
	dey
	dey
	Jmi *-3
.elseif(n = 37 || n = 39)
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.endif
.else
; 40 to 78
.if(n = 40 || n = 42 || n = 45 || n = 47 || n = 54 || n = 55 || n = 59 || n = 60 || n = 65 || n = 67 || n = 70 || n = 72 || n = 74 || n = 75 || n = 77)
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 41 || n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 43 || n = 57)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 44)
	; 6 bytes
	ldy #136 ;hides 'dey'
	nop
	dey
	Jmi *-3
.elseif(n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif((n .mod 5 = 3) && ((n > 47 && n < 74)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 49)
	; 4 bytes
	ldy #136 ;hides 'dey'
	Jmi *-1
.elseif(n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 52)
	; 6 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 62)
	; 5 bytes
	ldx #136 ;hides 'dey'
	dex
	Jmi *-2
.elseif(n = 69)
	; 5 bytes
	ldy #136 ;hides 'dey'
	nop
	Jmi *-2
.endif
.endif
.else
; 79 to 156
.if(n = 79)
	; 6 bytes
	ldy #136 ;hides 'dey'
	nop $04
	Jmi *-3
.elseif(n = 80)
	; 6 bytes
	ldx #136 ;hides 'dey'
	nop
	dex
	Jmi *-3
.elseif(n = 81 || n = 96 || n = 121 || n = 131 || n = 146 || n = 151 || n = 156)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 82 || n = 84 || n = 87 || n = 90 || n = 94 || n = 95 || n = 97 || n = 100 || n = 102 || n = 104 || n = 105 || n = 109 || n = 110 || n = 112 || n = 114 || n = 115 || n = 117 || n = 122 || n = 129 || n = 130 || n = 132 || n = 135 || n = 137 || n = 139 || n = 140 || n = 142 || n = 144 || n = 145 || n = 147 || n = 149 || n = 150 || n = 152 || n = 154)
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
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
.elseif(n = 89)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 4
	Jmi *-3
.elseif(n = 92 || n = 120 || n = 127 || n = 148 || n = 155)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 107)
	; 6 bytes
	ldx #24 ;hides 'clc'
	dex
	dex
	Jne *-3
.elseif(n = 119)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	Jmi *-3
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
.if(n = 157 || n = 159 || n = 160 || n = 164 || n = 165 || n = 172 || n = 175 || n = 177 || n = 180 || n = 182 || n = 184 || n = 185 || n = 187 || n = 189 || n = 192 || n = 194 || n = 195 || n = 200 || n = 202 || n = 205 || n = 207 || n = 209 || n = 210 || n = 212 || n = 214 || n = 217 || n = 219 || n = 220 || n = 222 || n = 227 || n = 229 || n = 230 || n = 234)
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif((n .mod 5 = 3) && ((n > 157 && n < 179) || (n > 192 && n < 214) || (n > 222 && n < 234)))
	; 6 bytes
	delay_n (n-2)
	nop
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
	ldy #200 ;hides 'iny'
	iny
	iny
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
.elseif(n = 179)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 188)
	; 5 bytes
	ldx #26 ;hides 'nop'
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
.endif
.else
; 235 to 312
.if(n = 235 || n = 237 || n = 240 || n = 244 || n = 245 || n = 247 || n = 249 || n = 250 || n = 252 || n = 254 || n = 255 || n = 257 || n = 259 || n = 262 || n = 264 || n = 270 || n = 272 || n = 275 || n = 277 || n = 279 || n = 282 || n = 285 || n = 289 || n = 290)
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 236 || n = 246 || n = 261 || n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 238 || n = 243 || n = 248 || n = 258 || n = 263 || n = 268 || n = 273 || n = 278 || n = 283 || n = 292 || n = 293 || n = 297 || n = 298 || n = 303 || n = 305 || n = 308 || n = 310)
	; 6..8 bytes
	delay_n (n-2)
	nop
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
	ldy #202 ;hides 'dex'
	iny
	iny
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 252 && n < 275) || (n > 287 && n < 310)))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 265)
	; 5 bytes
	ldx #202 ;hides 'dex'
	dex
	Jmi *-2
.elseif(n = 269)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	dey
	dey
	Jmi *-3
.elseif(n = 280)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 284)
	; 4 bytes
	ldy #200 ;hides 'iny'
	Jmi *-1
.elseif(n = 287)
	; 6 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 294)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 299)
	; 6 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
.elseif(n = 300)
	; 5 bytes
	ldy #170 ;hides 'tax'
	dey
	Jmi *-2
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
.elseif(n = 312)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	nop
	dey
	Jmi *-3
.endif
.endif
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 315 || n = 317 || n = 318 || n = 322 || n = 327 || n = 328 || n = 333 || n = 335 || n = 338 || n = 340 || n = 342 || n = 343 || n = 345 || n = 348 || n = 350 || n = 353 || n = 355 || n = 357 || n = 362 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 380 || n = 383 || n = 385 || n = 387 || n = 388 || n = 390 || n = 392 || n = 402 || n = 403 || n = 408 || n = 410 || n = 413 || n = 415 || n = 418 || n = 420 || n = 422 || n = 423 || n = 425 || n = 427 || n = 432 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 447 || n = 448 || n = 450 || n = 453 || n = 458 || n = 460 || n = 462 || n = 468)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 314)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	iny
	iny
	Jne *-3
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 347 || n = 349 || n = 354 || n = 359 || n = 364 || n = 369 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 409 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 452 || n = 454 || n = 459)
	; 6..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 320)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jmi *-3
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
.elseif(n = 332)
	; 6 bytes
	ldx #200 ;hides 'iny'
	dex
	dex
	Jmi *-3
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
	; 5 bytes
	ldy #202 ;hides 'dex'
	iny
	Jne *-2
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
	ldy #186 ;hides 'tsx'
	dey
	Jmi *-2
.elseif(n = 417)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
.elseif(n = 455)
	; 6 bytes
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
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
.if(n = 469 || n = 474 || n = 479 || n = 499 || n = 504 || n = 509 || n = 514 || n = 522 || n = 529 || n = 534)
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
.elseif(n = 473 || n = 475 || n = 478 || n = 482 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 502 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 528 || n = 535 || n = 538 || n = 542 || n = 543 || n = 545)
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
	ldy #186 ;hides 'tsx'
	iny
	Jne *-2
.elseif(n = 492 || n = 527)
	; 7 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 494)
	; 6 bytes
	ldy #182 ;hides 'ldx zp,y'
	nop
	dey
	Jmi *-3
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
	; 5 bytes
	ldx #200 ;hides 'iny'
	dex
	Jmi *-2
.elseif(n = 517)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 524)
	; 5 bytes
	ldy #202 ;hides 'dex'
	dey
	Jmi *-2
.elseif(n = 530)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jmi *-3
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
.elseif(n = 539)
	; 6 bytes
	ldx #136 ;hides 'dey'
	inx
	inx
	Jne *-3
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
.elseif(n = 548 || n = 553 || n = 555 || n = 558 || n = 562 || n = 563 || n = 565 || n = 570 || n = 572 || n = 573 || n = 578 || n = 580 || n = 583 || n = 585 || n = 587 || n = 588 || n = 590 || n = 593 || n = 597 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 618 || n = 620 || n = 623 || n = 625)
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
.elseif(n = 615)
	; 5 bytes
	ldx #168 ;hides 'tay'
	inx
	Jne *-2
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 702)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 627 || n = 634 || n = 639 || n = 644 || n = 649 || n = 654 || n = 662 || n = 664 || n = 669 || n = 679 || n = 689 || n = 699)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 628 || n = 630 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 668 || n = 670 || n = 675 || n = 677 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 693 || n = 695 || n = 698 || n = 700 || n = 702 || n = 703)
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
.elseif(n = 638 || n = 645 || n = 673 || n = 680 || n = 687 || n = 694)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 640)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 642)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 652 || n = 659)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657)
	; 7 bytes
	ldx #(n-9)/8
	cmp $C5
	dex
	Jpl *-3
.elseif(n = 665)
	; 6 bytes
	ldy #182 ;hides 'ldx zp,y'
	nop
	iny
	Jne *-3
.elseif(n = 667 || n = 685)
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
	ldy #202 ;hides 'dex'
	nop
	dey
	Jmi *-3
.elseif(n = 684)
	; 4 bytes
	ldy #136 ;hides 'dey'
	Jne *-1
.elseif(n = 692)
	; 6 bytes
	ldx #180 ;hides 'ldy zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 697)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.endif
.else
; 704 to 781
.if(n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 732 || n = 739 || n = 749 || n = 754 || n = 759 || n = 767 || n = 769 || n = 774 || n = 779)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 705 || n = 707 || n = 717 || n = 718 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 742 || n = 752 || n = 753 || n = 758 || n = 760 || n = 762 || n = 763 || n = 765 || n = 768 || n = 770 || n = 772 || n = 773)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 705 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 7 = 1) && ((n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 710 || n = 745 || n = 780)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 712)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	iny
	Jne *-3
.elseif(n = 713)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
.elseif(n = 720)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	nop
	iny
	Jne *-3
.elseif(n = 727)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
.elseif(n = 729)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 730 || n = 775)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 734)
	; 5 bytes
	ldy #232 ;hides 'inx'
	dey
	Jmi *-2
.elseif(n = 737 || n = 777)
	; 7 bytes
	ldx #(n-9)/8
	cmp $C5
	dex
	Jpl *-3
.elseif(n = 744)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
.elseif(n = 747)
	; 7 bytes
	ldx #136 ;hides 'dey'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 748)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
.elseif(n = 755)
	; 6 bytes
	ldx #168 ;hides 'tay'
	dex
	dex
	Jne *-3
.endif
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
.elseif(n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 818 || n = 822 || n = 823 || n = 825 || n = 828 || n = 830 || n = 833 || n = 835 || n = 838 || n = 840 || n = 843 || n = 845 || n = 852 || n = 857 || n = 858)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 784 || n = 789 || n = 794 || n = 804 || n = 814 || n = 819 || n = 824 || n = 829 || n = 837 || n = 844 || n = 849 || n = 854 || n = 859)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 810)
	; 7 bytes
	tay
	lax #3
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 812)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	iny
	Jne *-4
.elseif(n = 815 || n = 850)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 817)
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
.elseif(n = 839)
	; 5 bytes
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
.elseif(n = 842)
	; 7 bytes
	delay_n (n-3)
	ldy $A4
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
.elseif(n = 863 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 882 || n = 887 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 902 || n = 903 || n = 905 || n = 908 || n = 910 || n = 913 || n = 915 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 864 || n = 872 || n = 874 || n = 879 || n = 884 || n = 889 || n = 894 || n = 907 || n = 909 || n = 914 || n = 919 || n = 924 || n = 929 || n = 934)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 869 || n = 890 || n = 897 || n = 918 || n = 925 || n = 932)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
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
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 940 || n = 943 || n = 945 || n = 948 || n = 950 || n = 952 || n = 957 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 983 || n = 985 || n = 987 || n = 992 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1020 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1042 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1062 || n = 1063 || n = 1068 || n = 1073 || n = 1075 || n = 1077 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1087 || n = 1088 || n = 1090 || n = 1092)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || n = 1093))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1092)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 944)
	; 6 bytes
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
.elseif(n = 947)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1012 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1039 || n = 1047 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1089)
	; 6..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 955 || n = 990 || n = 1025 || n = 1060)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 958)
	; 5 bytes
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
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
	ldy #186 ;hides 'tsx'
	nop
	dey
	dey
	Jne *-4
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
.elseif(n = 1057)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1067)
	; 7 bytes
	tay
	lax #4
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 1070)
	; 6 bytes
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
.elseif(n = 1082)
	; 7 bytes
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	Jmi *-4
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1129 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1174 || n = 1179 || n = 1194 || n = 1199 || n = 1204 || n = 1209 || n = 1214 || n = 1222 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244 || n = 1249)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1095)
	; 6 bytes
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
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
.elseif(n = 1098 || n = 1102 || n = 1103 || n = 1105 || n = 1108 || n = 1110 || n = 1112 || n = 1113 || n = 1115 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1127 || n = 1132 || n = 1133 || n = 1137 || n = 1138 || n = 1140 || n = 1143 || n = 1145 || n = 1147 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1157 || n = 1158 || n = 1160 || n = 1167 || n = 1168 || n = 1172 || n = 1173 || n = 1178 || n = 1180 || n = 1183 || n = 1185 || n = 1188 || n = 1190 || n = 1193 || n = 1195 || n = 1197 || n = 1202 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1225 || n = 1227 || n = 1228 || n = 1230 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
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
.elseif(n = 1130 || n = 1165 || n = 1200 || n = 1235)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1175)
	; 5 bytes
	ldx #168 ;hides 'tay'
	dex
	Jne *-2
.elseif(n = 1182)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1187)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jpl *-4
.elseif(n = 1189)
	; 5 bytes
	ldy #170 ;hides 'tax'
	dey
	Jne *-2
.elseif(n = 1192)
	; 7 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 1217)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1232)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
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
.elseif(n = 1252 || n = 1297 || n = 1306 || n = 1315)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1262 || n = 1263 || n = 1265 || n = 1267 || n = 1272 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1291 || n = 1293 || n = 1298 || n = 1300 || n = 1305 || n = 1307 || n = 1311 || n = 1314 || n = 1316 || n = 1319 || n = 1322 || n = 1323 || n = 1326)
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
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1259)
	; 6 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290 || n = 1292 || n = 1299)
	; 7..8 bytes
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
	; 8 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1301)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
.elseif(n = 1302 || n = 1318)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1304)
	; 6 bytes
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1308)
	; 7 bytes
	ldy #136 ;hides 'dey'
	nop
	iny
	iny
	Jne *-4
.elseif(n = 1309)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1312)
	; 6 bytes
	ldx #164 ;hides 'ldy zp'
	nop
	dex
	Jne *-3
.elseif(n = 1320)
	; 6 bytes
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1321)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1325)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1327)
	; 7 bytes
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1328)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	nop
	dey
	Jne *-3
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1337 || n = 1353 || n = 1377)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1330 || n = 1340 || n = 1344 || n = 1347 || n = 1349 || n = 1354 || n = 1356 || n = 1361 || n = 1363 || n = 1368 || n = 1370 || n = 1371 || n = 1375 || n = 1382 || n = 1384 || n = 1389 || n = 1391 || n = 1395 || n = 1403)
	; 7..8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1334 || n = 1348 || n = 1355 || n = 1362 || n = 1379 || n = 1390 || n = 1397 || n = 1402 || n = 1404)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1335 || n = 1398)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1336)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
.elseif(n = 1339)
	; 7 bytes
	ldx #122 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 1341)
	; 7 bytes
	ldx #122 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1346 || n = 1358 || n = 1372 || n = 1400)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1350)
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	delay_n 9
	iny
	Jne *-5
.elseif(n = 1357)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 1367)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dex
	dex
	Jne *-5
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
.elseif(n = 1383 || n = 1392 || n = 1406)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
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
.if(n = 1407 || n = 1410 || n = 1412 || n = 1417 || n = 1419 || n = 1424 || n = 1426 || n = 1431 || n = 1434 || n = 1435 || n = 1438 || n = 1445 || n = 1447 || n = 1452 || n = 1454 || n = 1466 || n = 1470 || n = 1473 || n = 1475 || n = 1479 || n = 1480 || n = 1482 || n = 1483)
	; 7..8 bytes
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
.elseif(n = 1411 || n = 1418 || n = 1439 || n = 1446 || n = 1460 || n = 1467 || n = 1474)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1413)
	; 5 bytes
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442) || (n > 1458 && n < 1478)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1416)
	; 7 bytes
	delay_n (n-3)
	ldy $A4
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
.elseif(n = 1427)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1428 || n = 1444 || n = 1448 || n = 1456 || n = 1462 || n = 1472 || n = 1476 || n = 1484)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1430)
	; 8 bytes
	ldx #n/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 1437)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1440)
	; 7 bytes
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
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
.elseif(n = 1458)
	; 7 bytes
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
.elseif(n = 1461)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1463)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 1469)
	; 7 bytes
	ldy #232 ;hides 'inx'
	delay_n 7
	dey
	Jmi *-4
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
.elseif(n = 1487 || n = 1491 || n = 1494 || n = 1498 || n = 1501 || n = 1503 || n = 1508 || n = 1510 || n = 1514 || n = 1515 || n = 1517)
	; 7..8 bytes
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
.elseif(n = 1490)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
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
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 7
	dex
	Jmi *-5
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
	; 6 bytes
	ldx #168 ;hides 'tay'
	nop
	dex
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
.elseif(n = 1526 || n = 1535)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 1529)
	; 6 bytes
	ldy #170 ;hides 'tax'
	nop
	dey
	Jne *-3
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
.elseif(n = 1533 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1552 || n = 1557)
	; 7..8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 4
	iny
	iny
	Jne *-5
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
.elseif(n = 1564 || n = 1566 || n = 1570 || n = 1571 || n = 1578 || n = 1580 || n = 1586 || n = 1588 || n = 1592 || n = 1595 || n = 1599 || n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1615 || n = 1620 || n = 1622 || n = 1627 || n = 1629 || n = 1634 || n = 1636)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1572 || n = 1579 || n = 1600 || n = 1607 || n = 1614 || n = 1626 || n = 1635 || n = 1640)
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
.elseif(n = 1581)
	; 7 bytes
	tay
	lax #6
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 1582)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.elseif(n = 1589)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1598)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jmi *-5
.elseif(n = 1602 || n = 1638)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 1619)
	; 6 bytes
	ldx #180 ;hides 'ldy zp,x'
	nop
	dex
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
.elseif(n = 1628)
	; 6 bytes
	ldx #180 ;hides 'ldy zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1631)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.endif
.else
; 1641 to 1718
.if(n = 1641 || n = 1647 || n = 1652 || n = 1662 || n = 1664 || n = 1668 || n = 1669 || n = 1671 || n = 1672 || n = 1676 || n = 1678 || n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1692 || n = 1697 || n = 1704 || n = 1706 || n = 1715 || n = 1718)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1642 || n = 1656 || n = 1658 || n = 1663 || n = 1677 || n = 1691 || n = 1698 || n = 1712)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1643)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
.elseif(n = 1644 || n = 1680)
	; 7..8 bytes
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
.elseif(n = 1649 || n = 1665 || n = 1689 || n = 1713)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1650)
	; 7 bytes
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1651)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1654)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 1655)
	; 6 bytes
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
.elseif(n = 1659)
	; 7 bytes
	ldy #166 ;hides 'ldx zp'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
.elseif(n = 1661 || n = 1701)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1670)
	; 7 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
.elseif(n = 1673)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jne *-3
.elseif(n = 1679)
	; 7 bytes
	ldx #168 ;hides 'tay'
	ldy $A4
	dex
	Jne *-4
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1687 || n = 1694 || n = 1696 || n = 1700 || n = 1703 || n = 1708 || n = 1710 || n = 1714)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1699)
	; 7 bytes
	ldy #170 ;hides 'tax'
	ldx $A6
	dey
	Jne *-4
.elseif(n = 1707)
	; 7 bytes
	ldx #122 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if((n .mod 7 = 4) && ((n > 1718 && n < 1741) || n = 1782 || n = 1796))
	; 8 bytes
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
.elseif(n = 1722 || n = 1725 || n = 1727 || n = 1731 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1748 || (n > 1752 && n < 1756) || (n > 1761 && n < 1765) || n = 1767 || n = 1770 || n = 1787 || n = 1788 || n = 1791 || n = 1795)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724 || n = 1736 || n = 1766 || n = 1778)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1728)
	; 7 bytes
	pha
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	pla
.elseif(n = 1735)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
.elseif(n = 1752)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1757 || n = 1773)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
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
	cmp $C5
	dex
	Jne *-4
.elseif(n = 1776)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1780)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 8
	Jne *-5
.elseif(n = 1781)
	; 6 bytes
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 1784)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1790)
	; 6 bytes
	ldy #200 ;hides 'iny'
	dey
	dey
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
.if(n = 1797 || n = 1803 || n = 1812 || n = 1815 || n = 1818 || n = 1820 || n = 1821 || n = 1830 || n = 1834)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1798 || n = 1808 || n = 1814 || n = 1822)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1800 || n = 1806 || n = 1823 || n = 1831)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 9 = 1) && ((n > 1800 && n < 1829)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1802 || n = 1829)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1804)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	nop
	ldy $A4
	dex
	Jne *-5
.elseif(n = 1805)
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
.elseif(n = 1809 || n = 1833)
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
.elseif(n = 1816)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1817)
	; 6 bytes
	ldy #202 ;hides 'dex'
	nop
	dey
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
.elseif(n = 1827)
	; 7 bytes
	ldy #203 ;hides 'sbx #imm'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
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
.endif
.else
; 1836 to 1875
.if(n = 1836 || n = 1840 || n = 1843 || n = 1848 || n = 1851 || n = 1866 || n = 1867)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1874)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1838)
	; 7 bytes
	tay
	lax #7
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 1839)
	; 7 bytes
	ldx #184 ;hides 'clv'
	bit $24
	dex
	Jne *-4
.elseif(n = 1841)
	; 7 bytes
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1842)
	; 8 bytes
	pha
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 1844)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1845 || n = 1853)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1847)
	; 7 bytes
	ldx #168 ;hides 'tay'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1849 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1850 || n = 1852 || n = 1856 || n = 1858 || n = 1862 || n = 1868 || n = 1874)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1854)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1857 || n = 1865)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1859)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	ldx $A6
	dey
	Jne *-4
.elseif(n = 1860 || n = 1871)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1863)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1869)
	; 7 bytes
	ldy #170 ;hides 'tax'
	delay_n 4
	dey
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
.if(n = 1876 || n = 1878 || n = 1886 || (n > 1893 && n < 1897) || n = 1906 || n = 1908 || n = 1912 || n = 1924 || n = 1930 || n = 1934 || n = 1940 || (n > 1941 && n < 1945) || n = 1946)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1877 || n = 1919)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
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
.elseif(n = 1883 || n = 1884 || n = 1887 || n = 1892 || n = 1899 || n = 1911 || n = 1920 || n = 1935 || n = 1938 || n = 1939 || n = 1947 || n = 1952)
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1903)
	; 7 bytes
	ldx #136 ;hides 'dey'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 1907)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1910 || n = 1928)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1914 || n = 1923 || n = 1932)
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
	; 7 bytes
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
.elseif(n = 1922)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 1925)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1949)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
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
.elseif(n = 1955 || n = 1956 || n = 1962 || n = 1965 || n = 1971 || n = 1974 || n = 1982 || n = 1983 || n = 1987)
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
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
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
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1966 || n = 1976 || n = 1984 || n = 1986)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1967)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	ldy $EA ;hides 'nop'
	nop
	dex
	Jne *-5
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
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1975 || n = 1989)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1977 || n = 1985)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1978)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 1979)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1980)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
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
.elseif(n = 1992)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
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
	; 8 bytes
	ldy #232 ;hides 'inx'
	jsr @nearby_rts
	dey
	Jmi *-5
.elseif(n = 1995 || n = 2002 || n = 2007 || n = 2010 || n = 2018 || n = 2027 || n = 2028)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1996)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 1997 || n = 2013)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1998 || n = 2006 || n = 2022 || n = 2030)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 9 = 1) && ((n > 1998 && n < 2027)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2000)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
.elseif(n = 2001 || n = 2009 || n = 2025)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 2003 || n = 2014)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2004)
	; 8 bytes
	tax
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	iny
	Jne *-3
	txa
.elseif(n = 2011 || n = 2021)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2012)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	nop
	dey
	Jne *-4
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
.elseif(n = 2019)
	; 7 bytes
	ldy #202 ;hides 'dex'
	ldx $A6
	dey
	Jne *-4
.elseif(n = 2020)
	; 8 bytes
	tax
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	iny
	Jne *-3
	txa
.elseif(n = 2023)
	; 7 bytes
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2024)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
.elseif(n = 2037 || n = 2043 || n = 2046 || n = 2051 || n = 2055 || n = 2064 || n = 2067)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2038 || n = 2040 || n = 2042 || n = 2048 || n = 2050 || n = 2052 || n = 2056 || n = 2060 || n = 2066 || n = 2070)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2039)
	; 8 bytes
	ldy #170 ;hides 'tax'
	nop
	ldx $A6
	dey
	Jne *-5
.elseif(n = 2041 || n = 2065)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2045)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 4
	dey
	Jne *-4
.elseif(n = 2049)
	; 7 bytes
	ldx #0
	cmp $C5
	dex
	Jne *-3
.elseif(n = 2054)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-5
.elseif(n = 2057)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	nop $04
	inx
	Jne *-5
.elseif(n = 2059)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
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
.if(n = 2071 || n = 2080 || n = 2089 || n = 2107)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2072)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2073 || n = 2079 || n = 2082 || n = 2097 || n = 2100 || n = 2103 || n = 2109)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074 || n = 2083 || n = 2084 || n = 2086 || n = 2088 || n = 2093 || n = 2096 || n = 2099 || n = 2104 || n = 2106)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2075 || n = 2092)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
.elseif(n = 2081 || n = 2090 || n = 2108)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2087)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
.elseif(n = 2091 || n = 2102)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2095)
	; 7 bytes
	tay
	lax #8
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2098)
	; 7 bytes
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2105)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
.endif
.endif
.else
; 2110 to 2187
.if(n = 2110)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2111 || n = 2131 || n = 2141)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2112)
	; 8 bytes
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
.elseif(n = 2117)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2118 || n = 2127 || n = 2136 || n = 2139 || n = 2145 || n = 2151 || n = 2154 || n = 2163 || n = 2172 || n = 2175 || n = 2181 || n = 2187)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2119 || n = 2120 || n = 2122 || n = 2128 || n = 2133 || n = 2138 || n = 2140 || n = 2142 || n = 2144 || n = 2150 || n = 2153 || n = 2155 || n = 2158 || n = 2162 || (n > 2163 && n < 2167) || n = 2169 || n = 2174 || n = 2176 || n = 2180 || n = 2182 || n = 2184)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2126 || n = 2160)
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
.elseif(n = 2132)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2147)
	; 8 bytes
	ldx #232 ;hides 'inx'
	cmp $C5
	inx
	inx
	Jne *-5
.elseif(n = 2148)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2156)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2159)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	nop
	ldy $A4
	dex
	Jne *-5
.elseif(n = 2167)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	cmp $C5
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
.elseif(n = 2183)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	nop
	ldx $A6
	dey
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
.elseif(n = 2189)
	; 8 bytes
	delay_n (n-3)
	nop $04
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
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2192 || n = 2198 || n = 2202 || n = 2208 || n = 2211 || n = 2217 || n = 2226)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2193)
	; 8 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 2201 || n = 2212 || n = 2223)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2203 || n = 2214 || n = 2216 || n = 2218 || n = 2219 || n = 2225)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2204)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 2205)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
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
.elseif(n = 2228)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2229 || n = 2230 || n = 2236 || n = 2237 || (n > 2262 && n < 2266))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 2235 || n = 2244 || n = 2247 || n = 2252 || n = 2253 || n = 2262)
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2246)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
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
.if(n = 2266 || (n > 2272 && n < 2276) || n = 2284 || n = 2302)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2271 || n = 2272 || n = 2282 || n = 2288 || n = 2292 || n = 2298)
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2290)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2291 || n = 2301)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
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
	; 7 bytes
	ldx #26 ;hides 'nop'
	nop $04
	inx
	Jne *-4
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
.elseif(n = 2306 || n = 2307 || n = 2332 || n = 2343)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif((n > 2307 && n < 2311) || n = 2312 || (n > 2313 && n < 2317) || n = 2318 || n = 2324 || n = 2327 || n = 2334 || n = 2335 || n = 2337)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2311 || n = 2322 || n = 2333)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2313)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.elseif(n = 2328)
	; 7 bytes
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
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
.elseif(n = 2336)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
.elseif(n = 2338)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	nop
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
.if((n .mod 11 = 1) && (n = 2344 || n = 2366 || (n > 2387 && n < 2422)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2345 || n = 2346 || (n > 2347 && n < 2351) || n = 2356 || (n > 2357 && n < 2361) || n = 2362 || (n > 2367 && n < 2371) || (n > 2371 && n < 2375) || n = 2380 || (n > 2381 && n < 2387) || n = 2390 || (n > 2391 && n < 2395) || n = 2396 || n = 2400 || n = 2402 || (n > 2403 && n < 2409) || n = 2412 || n = 2416 || n = 2418 || n = 2419)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2347 || n = 2398)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2351)
	; 7 bytes
	ldx #168 ;hides 'tay'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 2352)
	; 7 bytes
	tay
	lax #9
	jsr delay_256a_x_33_clocks
	tya
.elseif((n .mod 12 = 1) && ((n > 2352 && n < 2414)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2354 || n = 2357 || n = 2391 || n = 2403 || n = 2415)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2355)
	; 7 bytes
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2361 || n = 2371 || n = 2411)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
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
	delay_n 6
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 2366 && n < 2410)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
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
.elseif(n = 2378)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
.elseif(n = 2379)
	; 7 bytes
	ldy #170 ;hides 'tax'
	delay_n 7
	dey
	Jne *-4
.elseif(n = 2387)
	; 8 bytes
	ldy #200 ;hides 'iny'
	cmp $C5
	dey
	dey
	Jne *-5
.elseif(n = 2397)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2414)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2417)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	delay_n 6
	dey
	Jne *-5
.elseif(n = 2420)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	inx
	Jne *-5
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2424 || n = 2426 || (n > 2427 && n < 2431) || n = 2434 || n = 2436 || n = 2440 || n = 2444 || n = 2446 || n = 2448 || n = 2450 || n = 2453 || (n > 2454 && n < 2459) || n = 2462 || (n > 2466 && n < 2471) || n = 2472 || n = 2474 || n = 2477 || n = 2478 || n = 2480 || n = 2482 || n = 2484 || n = 2488 || n = 2489 || n = 2492 || (n > 2493 && n < 2497))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2423 || n = 2451 || n = 2479 || n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 12 = 1) && ((n > 2424 && n < 2498)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2427 || n = 2439 || n = 2463 || n = 2475)
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2438)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2442)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2445)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 2447)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 2452)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2459)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	inx
	Jne *-5
.elseif(n = 2460)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 2464)
	; 8 bytes
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2466 || n = 2483 || n = 2500)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2486)
	; 8 bytes
	ldx #n/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2490)
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	delay_n 9
	dey
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
; 2501 to 2578
.if(n = 2501 || n = 2541)
	; 8 bytes
	ldx #(n-1)/10
	nop
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2502 || n = 2504 || n = 2506 || n = 2510 || (n > 2511 && n < 2517) || n = 2518 || n = 2522 || (n > 2523 && n < 2528) || n = 2530 || n = 2532 || (n > 2535 && n < 2540) || n = 2544 || n = 2546 || n = 2548 || n = 2550 || n = 2552 || n = 2554 || n = 2556 || n = 2558 || n = 2560 || n = 2562 || n = 2565 || n = 2570 || n = 2572 || n = 2574 || n = 2576 || n = 2578)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2503)
	; 8 bytes
	tsx
	ldy #250 ;hides 'nop'
	pha
	dey
	Jne *-3
	txs
.elseif(n = 2505)
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
.elseif((n .mod 14 = 1) && (n = 2507 || (n > 2534 && n < 2578)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2508)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dex
	Jne *-5
.elseif((n .mod 12 = 1) && ((n > 2508 && n < 2570)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2511 || n = 2523 || n = 2547 || n = 2559 || n = 2571)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2517 || n = 2534 || n = 2568)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2519)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
.elseif((n .mod 11 = 1) && ((n > 2519 && n < 2565)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2528 || n = 2566)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2529)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2540)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 2543)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	cmp $C5
	dex
	Jne *-5
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
.elseif(n = 2567)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2573)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2575)
	; 7 bytes
	ldx #184 ;hides 'clv'
	delay_n 7
	dex
	Jne *-4
.endif
.else
; 2579 to 2656
.if(n = 2579 || n = 2580 || (n > 2586 && n < 2591) || n = 2592 || n = 2598 || n = 2600 || n = 2601 || n = 2610 || n = 2613 || n = 2616 || n = 2618 || (n > 2619 && n < 2623) || n = 2624 || n = 2626 || n = 2627 || n = 2632 || n = 2634 || n = 2635 || n = 2637 || (n > 2643 && n < 2647) || n = 2648 || n = 2649 || n = 2654 || n = 2656)
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
.elseif(n = 2582)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
.elseif(n = 2583 || n = 2596 || n = 2607 || n = 2611 || n = 2614 || n = 2631 || n = 2643 || n = 2655)
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
.elseif(n = 2585 || n = 2636)
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
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2602)
	; 7 bytes
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	Jne *-4
.elseif(n = 2603)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 7
	dey
	Jne *-4
.elseif(n = 2604 || n = 2623 || n = 2642)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2606)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2609)
	; 7 bytes
	tay
	lax #10
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2612)
	; 7 bytes
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2615)
	; 8 bytes
	ldx #218 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
.elseif(n = 2625)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 6
	dey
	Jne *-5
.elseif(n = 2628)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
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
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if((n > 2656 && n < 2661) || n = 2662 || n = 2666 || n = 2668 || n = 2669 || n = 2671 || n = 2673 || n = 2676 || n = 2678 || n = 2681 || n = 2686 || n = 2688 || n = 2690 || n = 2692 || n = 2693 || n = 2695 || n = 2697 || n = 2698 || n = 2700 || n = 2702 || n = 2705 || n = 2706 || (n > 2707 && n < 2713) || n = 2714 || n = 2716 || n = 2720 || n = 2722 || n = 2723 || n = 2726 || n = 2728 || n = 2730 || (n > 2731 && n < 2735))
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
.elseif(n = 2667 || n = 2679 || n = 2691 || n = 2715 || n = 2727)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2670 || n = 2704 || n = 2721)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2672)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2680 || n = 2699)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 2687)
	; 8 bytes
	ldx #168 ;hides 'tay'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 2694)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2719)
	; 8 bytes
	ldy #170 ;hides 'tax'
	delay_n 9
	dey
	Jne *-5
.elseif(n = 2724)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 4
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
	delay_n 6
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
.elseif(n = 2739 || n = 2763)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2740 || n = 2751 || n = 2762)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2741 || n = 2743 || n = 2744 || n = 2746 || n = 2747 || n = 2752 || n = 2753 || n = 2757 || n = 2758 || (n > 2763 && n < 2767) || n = 2769 || n = 2770)
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
.elseif(n = 2745 || n = 2759)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
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
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
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
	delay_n 6
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
.if(n = 2774 || (n > 2775 && n < 2781) || n = 2786 || n = 2788 || (n > 2789 && n < 2793) || n = 2796 || n = 2798 || n = 2800 || n = 2802 || n = 2803 || n = 2808 || n = 2810 || n = 2812)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2775 || n = 2811)
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2789)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
; 2813 to 2968
.if(n = 2813 || n = 2832 || n = 2851 || n = 2870 || n = 2889 || n = 2946)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2814 || n = 2818 || n = 2822 || (n > 2823 && n < 2827) || n = 2828 || n = 2830 || n = 2834 || (n > 2835 && n < 2840) || n = 2842 || n = 2844 || n = 2846 || (n > 2847 && n < 2851) || (n > 2851 && n < 2855) || n = 2856 || n = 2858 || (n > 2859 && n < 2863) || n = 2864 || n = 2865 || n = 2867 || n = 2872 || n = 2873 || n = 2875 || n = 2877 || n = 2878 || n = 2880 || n = 2882 || n = 2884 || (n > 2885 && n < 2889) || n = 2890 || n = 2892 || n = 2894 || (n > 2895 && n < 2899) || (n > 2899 && n < 2905) || n = 2906 || n = 2909 || n = 2910 || n = 2912 || n = 2914 || n = 2916 || n = 2918 || (n > 2919 && n < 2925) || n = 2926 || n = 2928 || n = 2930 || (n > 2931 && n < 2939) || n = 2944 || n = 2945 || (n > 2946 && n < 2953) || n = 2954 || (n > 2955 && n < 2959) || (n > 2959 && n < 2965) || n = 2966 || n = 2968)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif((n .mod 14 = 1) && ((n > 2814 && n < 2844) || (n > 2884 && n < 2928) || n = 2955))
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
.elseif((n .mod 12 = 1) && ((n > 2820 && n < 2858) || (n > 2880 && n < 2966)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2823 || n = 2835 || n = 2847 || n = 2859 || n = 2868 || n = 2871 || n = 2883 || n = 2895 || n = 2907 || n = 2919 || n = 2931 || n = 2943 || n = 2967)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2827)
	; 7 bytes
	ldy #202 ;hides 'dex'
	delay_n 7
	dey
	Jne *-4
.elseif(n = 2831)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jne *-5
.elseif(n = 2840 || n = 2874 || n = 2891 || n = 2908 || n = 2942 || n = 2959)
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
.elseif(n = 2855)
	; 8 bytes
	ldx #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 2863)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
.elseif(n = 2866)
	; 7 bytes
	tay
	lax #11
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2869)
	; 7 bytes
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2876)
	; 8 bytes
	ldy #136 ;hides 'dey'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
.elseif(n = 2879)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 2911)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	delay_n 9
	dey
	Jne *-5
.elseif(n = 2915)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	dex
	Jne *-5
.elseif(n = 2925)
	; 8 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
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
.endif
.else
; 2969 to 3125
.if((n .mod 14 = 1) && ((n > 2968 && n < 3012) || (n > 3038 && n < 3110)))
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
.elseif((n > 2970 && n < 2975) || n = 2978 || (n > 2979 && n < 2983) || (n > 2984 && n < 2989) || n = 2990 || n = 2992 || (n > 2993 && n < 2997) || n = 2998 || n = 3000 || (n > 3003 && n < 3010) || n = 3012 || n = 3014 || (n > 3015 && n < 3022) || n = 3023 || n = 3024 || n = 3026 || (n > 3027 && n < 3037) || n = 3038 || n = 3040 || n = 3042 || n = 3043 || (n > 3044 && n < 3049) || n = 3050 || n = 3052 || (n > 3054 && n < 3060) || n = 3062 || n = 3064 || n = 3065 || (n > 3067 && n < 3073) || n = 3074 || n = 3076 || n = 3077 || (n > 3081 && n < 3095) || n = 3096 || n = 3097 || (n > 3098 && n < 3109) || n = 3110 || n = 3111 || (n > 3112 && n < 3117) || (n > 3117 && n < 3123) || n = 3124)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2975)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	delay_n 9
	dey
	Jne *-5
.elseif(n = 2976 || n = 2993 || n = 3010 || n = 3078 || n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif((n .mod 12 = 1) && ((n > 2976 && n < 3062)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2979 || n = 2991 || n = 3003 || n = 3027 || n = 3063 || n = 3075 || n = 3125)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2984 || n = 3022 || n = 3060 || n = 3079 || n = 3098 || n = 3117)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2999)
	; 8 bytes
	ldx #250 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
.elseif(n = 3002)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 3015)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-5
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
.elseif(n = 3051)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3054)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 3066)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3080)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 3123)
	; 7 bytes
	tay
	lax #12
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if(n = 3126)
	; 7 bytes
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3127 || (n > 3129 && n < 3136) || (n > 3137 && n < 3146) || (n > 3146 && n < 3151) || n = 3152 || n = 3154 || n = 3156 || n = 3157 || (n > 3158 && n < 3163) || n = 3164 || n = 3166 || n = 3168 || n = 3169 || n = 3172 || n = 3173 || (n > 3174 && n < 3179) || n = 3181 || n = 3182 || (n > 3184 && n < 3191) || n = 3192 || (n > 3193 && n < 3197) || n = 3198 || (n > 3199 && n < 3207) || (n > 3207 && n < 3212) || n = 3213 || (n > 3214 && n < 3219) || n = 3220 || (n > 3221 && n < 3229) || n = 3230 || (n > 3231 && n < 3235) || (n > 3235 && n < 3247) || (n > 3250 && n < 3263) || n = 3264 || (n > 3265 && n < 3269) || (n > 3269 && n < 3275) || n = 3276 || n = 3278 || n = 3280 || n = 3281)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3128)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3129 || n = 3146 || n = 3163 || n = 3180 || n = 3197 || n = 3214 || n = 3248 || n = 3265)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.elseif(n = 3153)
	; 8 bytes
	ldx #90 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3158)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
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
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3183)
	; 8 bytes
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
.elseif(n = 3184)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3191)
	; 8 bytes
	ldx #168 ;hides 'tay'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 3199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3219)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3229)
	; 8 bytes
	ldy #170 ;hides 'tax'
	jsr @nearby_rts
	dey
	Jne *-5
.elseif(n = 3231)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 9
	dey
	Jne *-5
.elseif(n = 3247)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
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
.else
; 3282 to 3437
.if(n = 3282 || n = 3299 || n = 3316 || n = 3350 || n = 3367 || n = 3384 || n = 3401 || n = 3418)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283 || n = 3284 || n = 3286 || n = 3287 || n = 3289 || n = 3290 || (n > 3291 && n < 3299) || (n > 3300 && n < 3305) || n = 3306 || (n > 3307 && n < 3315) || n = 3317 || n = 3318 || (n > 3319 && n < 3326) || (n > 3326 && n < 3333) || (n > 3333 && n < 3345) || n = 3346 || n = 3348 || n = 3349 || (n > 3350 && n < 3361) || n = 3362 || n = 3363 || n = 3365 || n = 3366 || (n > 3367 && n < 3375) || (n > 3375 && n < 3380) || n = 3381 || (n > 3385 && n < 3389) || n = 3390 || (n > 3391 && n < 3401) || n = 3404 || n = 3406 || (n > 3407 && n < 3417) || n = 3419 || n = 3422 || (n > 3423 && n < 3431) || (n > 3431 && n < 3435) || n = 3436 || n = 3437)
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
.elseif(n = 3288 || n = 3307 || n = 3326 || n = 3345 || n = 3364 || n = 3402 || n = 3421)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && ((n > 3290 && n < 3432)))
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
.elseif(n = 3315)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3380)
	; 7 bytes
	tay
	lax #13
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3382 || n = 3385)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3383)
	; 7 bytes
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3391)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
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
.elseif(n = 3435)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.endif
.endif
.else
; 3438 to 3750
.if(n = 3438 || n = 3439 || (n > 3440 && n < 3445) || (n > 3445 && n < 3450) || n = 3451 || (n > 3452 && n < 3459) || (n > 3459 && n < 3469) || (n > 3469 && n < 3473) || (n > 3473 && n < 3478) || (n > 3478 && n < 3485) || (n > 3487 && n < 3495) || n = 3496 || n = 3498 || n = 3500 || n = 3502 || (n > 3503 && n < 3515) || (n > 3516 && n < 3520) || (n > 3520 && n < 3527) || n = 3528 || (n > 3529 && n < 3533) || n = 3534 || n = 3536 || (n > 3537 && n < 3543) || (n > 3543 && n < 3554) || n = 3555 || n = 3556 || (n > 3557 && n < 3569) || n = 3570 || n = 3572 || (n > 3573 && n < 3585) || n = 3586 || n = 3587 || (n > 3588 && n < 3592) || (n > 3592 && n < 3605) || (n > 3605 && n < 3611) || (n > 3611 && n < 3622) || (n > 3622 && n < 3630) || (n > 3630 && n < 3637) || n = 3638 || n = 3641 || (n > 3642 && n < 3649) || (n > 3649 && n < 3656) || (n > 3656 && n < 3668) || (n > 3668 && n < 3673) || (n > 3673 && n < 3679) || (n > 3679 && n < 3687) || n = 3688 || n = 3689 || (n > 3690 && n < 3695) || (n > 3695 && n < 3706) || (n > 3707 && n < 3711) || (n > 3711 && n < 3724) || (n > 3725 && n < 3741) || n = 3742 || (n > 3744 && n < 3751))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3440 || n = 3478 || n = 3497 || n = 3516 || n = 3535 || n = 3573 || n = 3592 || n = 3611 || n = 3630 || n = 3649 || n = 3668 || n = 3687 || n = 3706 || n = 3725 || n = 3744)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && ((n > 3444 && n < 3572)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3450)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 3452 || n = 3469 || n = 3486 || n = 3503 || n = 3520 || n = 3537 || n = 3554 || n = 3588 || n = 3605 || n = 3622 || n = 3656 || n = 3673 || n = 3690 || n = 3707 || n = 3724 || n = 3741)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3485)
	; 8 bytes
	ldx #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 3495)
	; 8 bytes
	ldx #184 ;hides 'clv'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 3499)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3527)
	; 8 bytes
	ldx #168 ;hides 'tay'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3533)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	jsr @nearby_rts
	dey
	Jne *-5
.elseif(n = 3569)
	; 8 bytes
	ldy #170 ;hides 'tax'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3637)
	; 7 bytes
	tay
	lax #14
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3639 || n = 3642)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3640)
	; 7 bytes
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
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
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
; 3751 to 4062
.if((n > 3750 && n < 3758) || n = 3760 || n = 3762 || (n > 3763 && n < 3775) || (n > 3775 && n < 3782) || (n > 3782 && n < 3791) || (n > 3792 && n < 3799) || n = 3800 || (n > 3801 && n < 3809) || (n > 3809 && n < 3820) || (n > 3820 && n < 3826) || (n > 3826 && n < 3837) || n = 3838 || (n > 3839 && n < 3843) || (n > 3843 && n < 3856) || n = 3857 || n = 3859 || n = 3861 || n = 3862 || (n > 3863 && n < 3877) || (n > 3877 && n < 3887) || (n > 3887 && n < 3894) || n = 3895 || n = 3898 || (n > 3899 && n < 3903) || n = 3904 || (n > 3905 && n < 3911) || (n > 3911 && n < 3915) || (n > 3915 && n < 3919) || (n > 3919 && n < 3928) || (n > 3928 && n < 3934) || (n > 3934 && n < 3945) || (n > 3945 && n < 3953) || (n > 3953 && n < 3962) || (n > 3962 && n < 3972) || (n > 3972 && n < 3979) || (n > 3979 && n < 3991) || (n > 3991 && n < 3996) || n = 3997 || n = 3998 || (n > 3999 && n < 4010) || n = 4011 || n = 4012 || (n > 4013 && n < 4029) || (n > 4030 && n < 4047) || (n > 4048 && n < 4063))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3758 || n = 3775 || n = 3792 || n = 3809 || n = 3826 || n = 3843 || n = 3860 || n = 3877 || n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3979 || n = 3996 || n = 4013 || n = 4030 || n = 4047)
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
.elseif(n = 3761)
	; 8 bytes
	ldx #58 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3763 || n = 3782 || n = 3801 || n = 3820 || n = 3839 || n = 3858 || n = 3915 || n = 3934 || n = 3953 || n = 3972 || n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3791)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3799)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3837)
	; 8 bytes
	ldy #202 ;hides 'dex'
	jsr @nearby_rts
	dey
	Jne *-5
.elseif(n = 3856)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
.elseif(n = 3863)
	; 8 bytes
	ldx #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3887)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3894)
	; 7 bytes
	tay
	lax #15
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3896 || n = 3899)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3897)
	; 7 bytes
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3903)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3905)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
.elseif(n = 3919)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3999)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.endif
.else
; 4063 to 4375
.if(n = 4063 || n = 4065 || n = 4066 || (n > 4067 && n < 4081) || (n > 4081 && n < 4086) || (n > 4086 && n < 4098) || (n > 4098 && n < 4105) || (n > 4105 && n < 4115) || (n > 4115 && n < 4124) || (n > 4124 && n < 4132) || (n > 4132 && n < 4141) || n = 4142 || (n > 4143 && n < 4149) || n = 4150 || n = 4152 || n = 4155 || (n > 4157 && n < 4162) || (n > 4162 && n < 4166) || (n > 4166 && n < 4181) || n = 4182 || (n > 4183 && n < 4199) || (n > 4200 && n < 4217) || n = 4218 || (n > 4219 && n < 4234) || (n > 4234 && n < 4238) || n = 4239 || n = 4240 || (n > 4241 && n < 4251) || (n > 4251 && n < 4257) || (n > 4257 && n < 4262) || (n > 4262 && n < 4268) || (n > 4268 && n < 4276) || (n > 4276 && n < 4285) || (n > 4285 && n < 4295) || (n > 4295 && n < 4302) || (n > 4302 && n < 4314) || (n > 4314 && n < 4319) || (n > 4319 && n < 4333) || n = 4334 || n = 4335 || (n > 4336 && n < 4352) || (n > 4353 && n < 4369) || n = 4370 || (n > 4371 && n < 4376))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132 || n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217 || n = 4234 || n = 4251 || n = 4268 || n = 4285 || n = 4302 || n = 4319 || n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4067 || n = 4086 || n = 4105 || n = 4124 || n = 4143 || n = 4162 || n = 4181 || n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295 || n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4141)
	; 8 bytes
	ldx #218 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4151)
	; 7 bytes
	tay
	lax #16
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4153 || n = 4156)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4154)
	; 7 bytes
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4157)
	; 8 bytes
	ldx #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4241)
	; 8 bytes
	ldy #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
.elseif(n = 4262)
	; 8 bytes
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
.elseif(n = 4353)
	; 8 bytes
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4369)
	; 8 bytes
	ldx #26 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.endif
.endif
.else
; 4376 to 5000
.if((n > 4375 && n < 4390) || (n > 4390 && n < 4407) || n = 4412 || (n > 4413 && n < 4428) || (n > 4428 && n < 4445) || n = 4446 || (n > 4447 && n < 4466) || (n > 4466 && n < 4485) || (n > 4485 && n < 4504) || (n > 4504 && n < 4523) || (n > 4523 && n < 4542) || (n > 4542 && n < 4561) || (n > 4561 && n < 4577) || n = 4578 || n = 4579 || (n > 4580 && n < 4599) || (n > 4599 && n < 4618) || (n > 4618 && n < 4637) || (n > 4637 && n < 4656) || (n > 4656 && n < 4665) || n = 4666 || n = 4669 || (n > 4670 && n < 4675) || (n > 4675 && n < 4694) || (n > 4694 && n < 4713) || (n > 4713 && n < 4732) || (n > 4732 && n < 4749) || n = 4750 || (n > 4751 && n < 4770) || (n > 4770 && n < 4789) || (n > 4789 && n < 4808) || (n > 4808 && n < 4827) || n = 4828 || (n > 4829 && n < 4846) || (n > 4846 && n < 4865) || (n > 4865 && n < 4871) || (n > 4871 && n < 4913) || (n > 4913 && n < 4922) || n = 4923 || n = 4926 || (n > 4927 && n < 5001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4390 || n = 4409 || n = 4428 || n = 4447 || n = 4466 || n = 4485 || n = 4504 || n = 4523 || n = 4542 || n = 4561 || n = 4580 || n = 4599 || n = 4618 || n = 4637 || n = 4656 || n = 4675 || n = 4694 || n = 4713 || n = 4732 || n = 4751 || n = 4770 || n = 4789 || n = 4808 || n = 4827 || n = 4846)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4407)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4408)
	; 7 bytes
	tay
	lax #17
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4410 || n = 4413 || n = 4667 || n = 4670 || n = 4924 || n = 4927)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4411)
	; 7 bytes
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4445)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4577)
	; 8 bytes
	ldx #218 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4665)
	; 7 bytes
	tay
	lax #18
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4668)
	; 7 bytes
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4829)
	; 8 bytes
	ldx #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
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
.elseif(n = 4922)
	; 7 bytes
	tay
	lax #19
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4925)
	; 7 bytes
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
; 5001 to 7500
.if((n > 5000 && n < 5179) || n = 5180 || n = 5183 || (n > 5184 && n < 5249) || (n > 5249 && n < 5436) || n = 5437 || n = 5440 || (n > 5441 && n < 5693) || n = 5694 || n = 5697 || (n > 5698 && n < 5950) || n = 5951 || n = 5954 || (n > 5955 && n < 6207) || n = 6208 || n = 6211 || (n > 6212 && n < 6464) || n = 6465 || n = 6468 || (n > 6469 && n < 6721) || n = 6722 || n = 6725 || (n > 6726 && n < 6978) || n = 6979 || n = 6982 || (n > 6983 && n < 7235) || n = 7236 || n = 7239 || (n > 7240 && n < 7492) || n = 7493 || n = 7496 || (n > 7497 && n < 7501))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5179)
	; 7 bytes
	tay
	lax #20
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5181 || n = 5184 || n = 5438 || n = 5441 || n = 5695 || n = 5698 || n = 5952 || n = 5955 || n = 6209 || n = 6212 || n = 6466 || n = 6469 || n = 6723 || n = 6726 || n = 6980 || n = 6983 || n = 7237 || n = 7240 || n = 7494 || n = 7497)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5182)
	; 7 bytes
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5249)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5436)
	; 7 bytes
	tay
	lax #21
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5439)
	; 7 bytes
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5693)
	; 7 bytes
	tay
	lax #22
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5696)
	; 7 bytes
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5950)
	; 7 bytes
	tay
	lax #23
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5953)
	; 7 bytes
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6207)
	; 7 bytes
	tay
	lax #24
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6210)
	; 7 bytes
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6464)
	; 7 bytes
	tay
	lax #25
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6467)
	; 7 bytes
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6721)
	; 7 bytes
	tay
	lax #26
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6724)
	; 7 bytes
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6978)
	; 7 bytes
	tay
	lax #27
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6981)
	; 7 bytes
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7235)
	; 7 bytes
	tay
	lax #28
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 7238)
	; 7 bytes
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7492)
	; 7 bytes
	tay
	lax #29
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 7495)
	; 7 bytes
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7749) || n = 7750 || n = 7753 || (n > 7754 && n < 8006) || n = 8007 || n = 8010 || (n > 8011 && n < 8263) || n = 8264 || n = 8267 || (n > 8268 && n < 8520) || n = 8521 || n = 8524 || (n > 8525 && n < 8777) || n = 8778 || n = 8781 || (n > 8782 && n < 9034) || n = 9035 || n = 9038 || (n > 9039 && n < 9291) || n = 9292 || n = 9295 || (n > 9296 && n < 9548) || n = 9549 || n = 9552 || (n > 9553 && n < 9805) || n = 9806 || n = 9809 || (n > 9810 && n < 10001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7749)
	; 7 bytes
	tay
	lax #30
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 7751 || n = 7754 || n = 8008 || n = 8011 || n = 8265 || n = 8268 || n = 8522 || n = 8525 || n = 8779 || n = 8782 || n = 9036 || n = 9039 || n = 9293 || n = 9296 || n = 9550 || n = 9553 || n = 9807 || n = 9810)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 7752)
	; 7 bytes
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8006)
	; 7 bytes
	tay
	lax #31
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8009)
	; 7 bytes
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8263)
	; 7 bytes
	tay
	lax #32
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8266)
	; 7 bytes
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8520)
	; 7 bytes
	tay
	lax #33
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8523)
	; 7 bytes
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8777)
	; 7 bytes
	tay
	lax #34
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8780)
	; 7 bytes
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9034)
	; 7 bytes
	tay
	lax #35
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9037)
	; 7 bytes
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9291)
	; 7 bytes
	tay
	lax #36
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9294)
	; 7 bytes
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9548)
	; 7 bytes
	tay
	lax #37
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9551)
	; 7 bytes
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9805)
	; 7 bytes
	tay
	lax #38
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9808)
	; 7 bytes
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
; 10001 to 12500
.if((n > 10000 && n < 10062) || n = 10063 || n = 10066 || (n > 10067 && n < 10319) || n = 10320 || n = 10323 || (n > 10324 && n < 10576) || n = 10577 || n = 10580 || (n > 10581 && n < 10833) || n = 10834 || n = 10837 || (n > 10838 && n < 11090) || n = 11091 || n = 11094 || (n > 11095 && n < 11347) || n = 11348 || n = 11351 || (n > 11352 && n < 11604) || n = 11605 || n = 11608 || (n > 11609 && n < 11861) || n = 11862 || n = 11865 || (n > 11866 && n < 12118) || n = 12119 || n = 12122 || (n > 12123 && n < 12375) || n = 12376 || n = 12379 || (n > 12380 && n < 12501))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10062)
	; 7 bytes
	tay
	lax #39
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10064 || n = 10067 || n = 10321 || n = 10324 || n = 10578 || n = 10581 || n = 10835 || n = 10838 || n = 11092 || n = 11095 || n = 11349 || n = 11352 || n = 11606 || n = 11609 || n = 11863 || n = 11866 || n = 12120 || n = 12123 || n = 12377 || n = 12380)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10065)
	; 7 bytes
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10319)
	; 7 bytes
	tay
	lax #40
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10322)
	; 7 bytes
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10576)
	; 7 bytes
	tay
	lax #41
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10579)
	; 7 bytes
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10833)
	; 7 bytes
	tay
	lax #42
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10836)
	; 7 bytes
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11090)
	; 7 bytes
	tay
	lax #43
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11093)
	; 7 bytes
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11347)
	; 7 bytes
	tay
	lax #44
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11350)
	; 7 bytes
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11604)
	; 7 bytes
	tay
	lax #45
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11607)
	; 7 bytes
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11861)
	; 7 bytes
	tay
	lax #46
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11864)
	; 7 bytes
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12118)
	; 7 bytes
	tay
	lax #47
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12121)
	; 7 bytes
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12375)
	; 7 bytes
	tay
	lax #48
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12378)
	; 7 bytes
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
.endif
.else
; 12501 to 15000
.if((n > 12500 && n < 12632) || n = 12633 || n = 12636 || (n > 12637 && n < 12889) || n = 12890 || n = 12893 || (n > 12894 && n < 13146) || n = 13147 || n = 13150 || (n > 13151 && n < 13403) || n = 13404 || n = 13407 || (n > 13408 && n < 13660) || n = 13661 || n = 13664 || (n > 13665 && n < 13917) || n = 13918 || n = 13921 || (n > 13922 && n < 14174) || n = 14175 || n = 14178 || (n > 14179 && n < 14431) || n = 14432 || n = 14435 || (n > 14436 && n < 14688) || n = 14689 || n = 14692 || (n > 14693 && n < 14945) || n = 14946 || n = 14949 || (n > 14950 && n < 15001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12632)
	; 7 bytes
	tay
	lax #49
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12634 || n = 12637 || n = 12891 || n = 12894 || n = 13148 || n = 13151 || n = 13405 || n = 13408 || n = 13662 || n = 13665 || n = 13919 || n = 13922 || n = 14176 || n = 14179 || n = 14433 || n = 14436 || n = 14690 || n = 14693 || n = 14947 || n = 14950)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 12635)
	; 7 bytes
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12889)
	; 7 bytes
	tay
	lax #50
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12892)
	; 7 bytes
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13146)
	; 7 bytes
	tay
	lax #51
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13149)
	; 7 bytes
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13403)
	; 7 bytes
	tay
	lax #52
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13406)
	; 7 bytes
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13660)
	; 7 bytes
	tay
	lax #53
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13663)
	; 7 bytes
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13917)
	; 7 bytes
	tay
	lax #54
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13920)
	; 7 bytes
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14174)
	; 7 bytes
	tay
	lax #55
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14177)
	; 7 bytes
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14431)
	; 7 bytes
	tay
	lax #56
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14434)
	; 7 bytes
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14688)
	; 7 bytes
	tay
	lax #57
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14691)
	; 7 bytes
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14945)
	; 7 bytes
	tay
	lax #58
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14948)
	; 7 bytes
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
; 15001 to 17500
.if((n > 15000 && n < 15202) || n = 15203 || n = 15206 || (n > 15207 && n < 15459) || n = 15460 || n = 15463 || (n > 15464 && n < 15716) || n = 15717 || n = 15720 || (n > 15721 && n < 15973) || n = 15974 || n = 15977 || (n > 15978 && n < 16230) || n = 16231 || n = 16234 || (n > 16235 && n < 16487) || n = 16488 || n = 16491 || (n > 16492 && n < 16744) || n = 16745 || n = 16748 || (n > 16749 && n < 17001) || n = 17002 || n = 17005 || (n > 17006 && n < 17258) || n = 17259 || n = 17262 || (n > 17263 && n < 17501))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15202)
	; 7 bytes
	tay
	lax #59
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15204 || n = 15207 || n = 15461 || n = 15464 || n = 15718 || n = 15721 || n = 15975 || n = 15978 || n = 16232 || n = 16235 || n = 16489 || n = 16492 || n = 16746 || n = 16749 || n = 17003 || n = 17006 || n = 17260 || n = 17263)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15205)
	; 7 bytes
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15459)
	; 7 bytes
	tay
	lax #60
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15462)
	; 7 bytes
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15716)
	; 7 bytes
	tay
	lax #61
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15719)
	; 7 bytes
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15973)
	; 7 bytes
	tay
	lax #62
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15976)
	; 7 bytes
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16230)
	; 7 bytes
	tay
	lax #63
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 16233)
	; 7 bytes
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16487)
	; 7 bytes
	tay
	lax #64
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 16490)
	; 7 bytes
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16744)
	; 7 bytes
	tay
	lax #65
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 16747)
	; 7 bytes
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17001)
	; 7 bytes
	tay
	lax #66
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17004)
	; 7 bytes
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17258)
	; 7 bytes
	tay
	lax #67
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17261)
	; 7 bytes
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
.endif
.else
; 17501 to 20000
.if((n > 17500 && n < 17515) || n = 17516 || n = 17519 || (n > 17520 && n < 17772) || n = 17773 || n = 17776 || (n > 17777 && n < 18029) || n = 18030 || n = 18033 || (n > 18034 && n < 18286) || n = 18287 || n = 18290 || (n > 18291 && n < 18543) || n = 18544 || n = 18547 || (n > 18548 && n < 18800) || n = 18801 || n = 18804 || (n > 18805 && n < 19057) || n = 19058 || n = 19061 || (n > 19062 && n < 19314) || n = 19315 || n = 19318 || (n > 19319 && n < 19571) || n = 19572 || n = 19575 || (n > 19576 && n < 19828) || n = 19829 || n = 19832 || (n > 19833 && n < 20001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17515)
	; 7 bytes
	tay
	lax #68
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17517 || n = 17520 || n = 17774 || n = 17777 || n = 18031 || n = 18034 || n = 18288 || n = 18291 || n = 18545 || n = 18548 || n = 18802 || n = 18805 || n = 19059 || n = 19062 || n = 19316 || n = 19319 || n = 19573 || n = 19576 || n = 19830 || n = 19833)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 17518)
	; 7 bytes
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17772)
	; 7 bytes
	tay
	lax #69
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17775)
	; 7 bytes
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18029)
	; 7 bytes
	tay
	lax #70
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18032)
	; 7 bytes
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18286)
	; 7 bytes
	tay
	lax #71
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18289)
	; 7 bytes
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18543)
	; 7 bytes
	tay
	lax #72
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18546)
	; 7 bytes
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18800)
	; 7 bytes
	tay
	lax #73
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18803)
	; 7 bytes
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19057)
	; 7 bytes
	tay
	lax #74
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19060)
	; 7 bytes
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19314)
	; 7 bytes
	tay
	lax #75
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19317)
	; 7 bytes
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19571)
	; 7 bytes
	tay
	lax #76
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19574)
	; 7 bytes
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19828)
	; 7 bytes
	tay
	lax #77
	jsr delay_256a_x_33_clocks
	tya
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

.endif
	rest
.endmacro
