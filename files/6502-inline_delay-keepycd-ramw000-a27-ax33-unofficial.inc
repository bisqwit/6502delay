; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, Z+N, and V
; Will preserve Y, S, C, D, and I (interrupt-disable flag)
;
; WARNING: May write garbage into $00-$FF.
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
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 9
; - 4 bytes or less when n <= 17
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 1299
; - 8 bytes or less when n <= 1345
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=3, avg=4.0, max=5
; - When 21 <= n <= 30: min=5, avg=5.8, max=7
; - When 31 <= n <= 50: min=5, avg=6.2, max=7
; - When 51 <= n <= 75: min=5, avg=6.3, max=7
; - When 76 <= n <= 100: min=5, avg=6.3, max=7
; - When 101 <= n <= 200: min=4, avg=6.2, max=7
; - When 201 <= n <= 300: min=5, avg=6.2, max=7
; - When 301 <= n <= 500: min=4, avg=6.2, max=7
; - When 501 <= n <= 750: min=5, avg=6.2, max=7
; - When 751 <= n <= 1000: min=5, avg=6.2, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.9, max=9
; - When 2001 <= n <= 3000: min=6, avg=7.9, max=9
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
; 1 to 78
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	bit $24
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 23 || n = 28 || n = 33 || n = 38 || n = 48 || n = 58 || n = 63 || n = 68 || n = 73)
	; 2..6 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 6)
	; 2 bytes
	inc $F6,X
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 9 || n = 11 || n = 19 || n = 22)
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
.elseif(n = 13)
	; 4 bytes
	php
	inc $F6,X
	plp
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 24)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 20 || n = 27 || n = 30)
	; 5..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 21 || n = 26 || n = 36 || n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 25 || n = 35)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 29 || n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 31 || n = 41 || n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 32)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 34)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 37 || n = 39 || n = 42 || n = 44 || n = 45 || n = 47 || n = 49 || n = 52 || n = 54 || n = 55 || n = 59 || n = 60 || n = 62 || n = 65 || n = 67 || n = 69 || n = 70 || n = 72 || n = 74 || n = 75 || n = 77)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 40)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	dex
	Jne *-3
.elseif(n = 43 || n = 57)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 53)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
.endif
.else
; 79 to 156
.if(n = 79 || n = 80 || n = 82 || n = 84 || n = 87 || n = 89 || n = 90 || n = 94 || n = 95 || n = 97 || n = 100 || n = 102 || n = 104 || n = 105 || n = 109 || n = 110 || n = 112 || n = 114 || n = 115 || n = 117 || n = 119 || n = 122 || n = 132 || n = 135 || n = 137 || n = 139 || n = 140 || n = 142 || n = 144 || n = 145 || n = 147 || n = 149 || n = 150 || n = 152 || n = 154)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 81 || n = 96 || n = 121 || n = 131 || n = 146 || n = 151 || n = 156)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 5 = 3) && ((n > 82 && n < 109) || n = 118 || n = 123 || (n > 132 && n < 144)))
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
.elseif(n = 128)
	; 6 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 129)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 130)
	; 6 bytes
	delay_n (n-6)
	inc @zptemp,x
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
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
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
	ldx #90 ;hides 'nop'
	inx
	inx
	Jpl *-3
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
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 188)
	; 5 bytes
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jpl *-2
.elseif(n = 199)
	; 6 bytes
	ldx #232 ;hides 'inx'
	sta @zptemp
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
.if(n = 235 || n = 237 || n = 240 || n = 244 || n = 245 || n = 247 || n = 249 || n = 250 || n = 252 || n = 254 || n = 255 || n = 257 || n = 259 || n = 262 || n = 264 || n = 270 || n = 272 || n = 275 || n = 277 || n = 279 || n = 282 || n = 284 || n = 285 || n = 287 || n = 289 || n = 290)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 236 || n = 246 || n = 261 || n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 238 || n = 243 || n = 248 || n = 258 || n = 263 || n = 268 || n = 273 || n = 278 || n = 283 || n = 293 || n = 297 || n = 298 || n = 303 || n = 308)
	; 6..7 bytes
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
	ldx #26 ;hides 'nop'
	nop
	dex
	Jpl *-3
.elseif((n .mod 7 = 1) && ((n > 252 && n < 275) || (n > 294 && n < 310)))
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
.elseif(n = 288)
	; 6 bytes
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
.elseif(n = 292)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
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
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 304)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 305 || n = 310)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 307)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 312)
	; 6 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	dex
	Jpl *-3
.endif
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 318 || n = 328 || n = 332 || n = 333 || n = 338 || n = 343 || n = 348 || n = 353 || n = 363 || n = 367 || n = 368 || n = 373 || n = 378 || n = 388)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314)
	; 6 bytes
	ldx #58 ;hides 'nop'
	inx
	inx
	Jpl *-3
.elseif(n = 315 || n = 320 || n = 345 || n = 350)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 387)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 322 || n = 347 || n = 357 || n = 387)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 349 || n = 354 || n = 359 || n = 364 || n = 369 || n = 374 || n = 382 || n = 389)
	; 6..7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 321 || n = 326 || n = 336 || n = 356)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 323 || n = 330 || n = 344 || n = 365)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 325 || n = 335 || n = 340 || n = 355 || n = 360 || n = 370 || n = 375 || n = 380 || n = 390)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 327 || n = 342 || n = 362)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 337 || n = 358 || n = 372)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 352)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 377)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	inx
	Jpl *-3
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 383)
	; 6 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 384)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 385)
	; 6 bytes
	inc $F6,X
	delay_n (n-6)
.endif
.else
; 391 to 468
.if(n = 391 || n = 396 || n = 401 || n = 411 || n = 426 || n = 441 || n = 451 || n = 456 || n = 461)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 392 || n = 432 || n = 447 || n = 462)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 393 || n = 449)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 4) && (n = 394 || n = 399 || n = 409 || (n > 418 && n < 445) || n = 454 || n = 459))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 395)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jpl *-3
.elseif(n = 397)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 398)
	; 5 bytes
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
.elseif(n = 400 || n = 407 || n = 414 || n = 428 || n = 435 || n = 442 || n = 463)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
.elseif(n = 403 || n = 408 || n = 413 || n = 418 || n = 423 || n = 433 || n = 437 || n = 438 || n = 443 || n = 448 || n = 453 || n = 458 || n = 468)
	; 6..7 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 406 || n = 416 || n = 421 || n = 431 || n = 436 || n = 446 || n = 466)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 410 || n = 415 || n = 425 || n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
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
.elseif(n = 420 || n = 430 || n = 440 || n = 445 || n = 450 || n = 465)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 422 || n = 427 || n = 452)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 457)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 464)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inx
	Jpl *-3
.elseif(n = 467)
	; 6 bytes
	ldx #24 ;hides 'clc'
	inx
	inx
	Jpl *-3
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if((n .mod 5 = 4) && ((n > 468 && n < 480) || (n > 498 && n < 515) || (n > 523 && n < 540)))
	; 7 bytes
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
	ldx #69 ;hides 'eor zp'
	nop
	inx
	Jpl *-3
.elseif(n = 473 || n = 478 || n = 482 || n = 483 || n = 488 || n = 493 || n = 508 || n = 513 || n = 518 || n = 523 || n = 528 || n = 538 || n = 542 || n = 543)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 475 || n = 490 || n = 500 || n = 520)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
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
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jpl *-3
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
.elseif(n = 492 || n = 502 || n = 522)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 494)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 495 || n = 510 || n = 515 || n = 525 || n = 535 || n = 545)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 497)
	; 7 bytes
	ldx #(n-9)/8
	bit $24
	dex
	Jpl *-3
.elseif(n = 503)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
.elseif(n = 507)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
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
	ldx #138 ;hides 'txa'
	inx
	inx
	Jne *-3
.elseif(n = 532)
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
.elseif(n = 544)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
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
.elseif(n = 548 || n = 553 || n = 558 || n = 562 || n = 563 || n = 573 || n = 578 || n = 583 || n = 588 || n = 593 || n = 598 || n = 608 || n = 612 || n = 613 || n = 618 || n = 623)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && (n = 549 || (n > 558 && n < 585) || (n > 593 && n < 620)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 550 || n = 555 || n = 565 || n = 590 || n = 600 || n = 605)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 551 || n = 566 || n = 586 || n = 591 || n = 601 || n = 621)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 552)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	nop
	dex
	Jne *-3
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
	ldx #69 ;hides 'eor zp'
	nop
	dex
	Jpl *-3
.elseif(n = 567 || n = 587)
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
.elseif(n = 570 || n = 580 || n = 585 || n = 595 || n = 615 || n = 625)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 572 || n = 597)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 577)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 592)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	dex
	Jpl *-4
.elseif(n = 602)
	; 6 bytes
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 607)
	; 6 bytes
	ldx #202 ;hides 'dex'
	sta @zptemp
	Jmi *-3
.elseif(n = 620)
	; 6 bytes
	ldx #138 ;hides 'txa'
	dex
	dex
	Jne *-3
.elseif(n = 622)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
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
.elseif(n = 627 || n = 632 || n = 642 || n = 662 || n = 672 || n = 677 || n = 702)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 628 || n = 633 || n = 643 || n = 648 || n = 653 || n = 658 || n = 663 || n = 668 || n = 678 || n = 682 || n = 683 || n = 693 || n = 698 || n = 703)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 5 bytes
	ldx #90 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 630 || n = 640 || n = 650)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif((n .mod 5 = 4) && ((n > 633 && n < 655) || n = 664 || n = 669 || (n > 678 && n < 690) || n = 699))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 635 || n = 655 || n = 660 || n = 670 || n = 675 || n = 685 || n = 690 || n = 695 || n = 700)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
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
	bit $24
	dex
	Jpl *-3
.elseif(n = 665)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 667)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 674)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
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
.endif
.else
; 704 to 781
.if((n .mod 5 = 4) && ((n > 703 && n < 725) || n = 739 || (n > 748 && n < 760) || (n > 768 && n < 780)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 5 = 0) && (n = 705 || n = 710 || (n > 724 && n < 746) || (n > 759 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && ((n > 705 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 707 || n = 732 || n = 742 || n = 747 || n = 762 || n = 767 || n = 772)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 7 = 1) && ((n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 712)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 713)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
.elseif(n = 717 || n = 718 || n = 723 || n = 728 || n = 733 || n = 738 || n = 753 || n = 758 || n = 763 || n = 768 || n = 773)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 720)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	inx
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
.elseif(n = 734)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 737 || n = 777)
	; 7 bytes
	ldx #(n-9)/8
	bit $24
	dex
	Jpl *-3
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
.elseif(n = 752)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
.elseif(n = 755)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	dex
	Jne *-3
.endif
.endif
.else
; 782 to 937
.if(n = 782)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 783 || n = 787 || n = 788 || n = 793 || n = 798 || n = 803 || n = 808 || n = 818 || n = 822 || n = 823 || n = 828 || n = 833 || n = 838 || n = 843 || n = 857 || n = 858 || n = 863 || n = 868 || n = 873 || n = 878 || n = 888 || n = 893 || n = 898 || n = 903 || n = 908 || n = 913 || n = 923 || n = 927 || n = 928 || n = 933)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 795) || n = 804 || (n > 813 && n < 830) || (n > 838 && n < 865) || (n > 873 && n < 895) || (n > 908 && n < 935)))
	; 7 bytes
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
.elseif((n .mod 5 = 0) && (n = 790 || n = 795 || (n > 804 && n < 816) || (n > 829 && n < 851) || (n > 864 && n < 886) || (n > 894 && n < 921) || n = 930))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
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
	ldx #100 ;hides 'nop zp'
	nop
	dex
	Jne *-3
.elseif(n = 802 || n = 847)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 807 || n = 812 || n = 837 || n = 842 || n = 852 || n = 867 || n = 872 || n = 882 || n = 887 || n = 902 || n = 907 || n = 912 || n = 922)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 809)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 817 || n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 825)
	; 5 bytes
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
.elseif(n = 832)
	; 7 bytes
	pha
	delay_n (n-7)
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
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 943 || n = 948 || n = 958 || n = 963 || n = 968 || n = 973 || n = 978 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1003 || n = 1008 || n = 1013 || n = 1028 || n = 1032 || n = 1033 || n = 1038 || n = 1043 || n = 1048 || n = 1053 || n = 1067 || n = 1068 || n = 1073 || n = 1078 || n = 1083 || n = 1088)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || n = 1093))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 0) && ((n > 939 && n < 956) || n = 970 || n = 975 || n = 985 || n = 990 || (n > 999 && n < 1016) || n = 1025 || (n > 1034 && n < 1061) || (n > 1074 && n < 1091)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1092)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942 || n = 947 || n = 952 || n = 957 || n = 987 || n = 1012 || n = 1020 || n = 1022 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1082 || n = 1087 || n = 1092)
	; 6..7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1024 || n = 1029 || n = 1039 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1089)
	; 6..7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 962)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jmi *-3
.elseif(n = 965)
	; 5 bytes
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
.elseif(n = 972)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 977)
	; 7 bytes
	ldx #(n-9)/8
	bit $24
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
.elseif(n = 984)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jpl *-3
.elseif(n = 992)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
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
.elseif(n = 1018)
	; 6 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 1019)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
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
.elseif(n = 1063)
	; 5 bytes
	ldx #152 ;hides 'tya'
	dex
	Jne *-2
.elseif(n = 1070)
	; 7 bytes
	php
	lax #4
	jsr delay_256a_x_33_clocks
	plp
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && ((n > 1093 && n < 1110) || n = 1119 || (n > 1128 && n < 1145) || (n > 1153 && n < 1180) || (n > 1188 && n < 1215) || (n > 1223 && n < 1250)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 5 = 0) && (n = 1095 || (n > 1104 && n < 1131) || (n > 1139 && n < 1166) || (n > 1174 && n < 1201) || (n > 1209 && n < 1236) || n = 1245))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
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
.elseif(n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1113 || n = 1118 || n = 1123 || n = 1133 || n = 1137 || n = 1138 || n = 1143 || n = 1148 || n = 1153 || n = 1158 || n = 1168 || n = 1172 || n = 1173 || n = 1178 || n = 1183 || n = 1188 || n = 1193 || n = 1203 || n = 1208 || n = 1213 || n = 1218 || n = 1223 || n = 1228 || n = 1238 || n = 1242 || n = 1243 || n = 1248)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 2) && (n = 1112 || (n > 1121 && n < 1133) || (n > 1146 && n < 1158) || n = 1167 || n = 1182 || (n > 1191 && n < 1203) || (n > 1221 && n < 1233)))
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.elseif(n = 1162)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 1187)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	inx
	Jpl *-4
.elseif(n = 1217)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
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
.if((n >= 1251) && (n < 1290))
; 1251 to 1289
.if((n .mod 5 = 1) && ((n > 1250 && n < 1277)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1252)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1253 || n = 1258 || n = 1263 || n = 1273 || n = 1277 || n = 1278 || n = 1283)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1290)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1271) || n = 1280))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 1257)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1259)
	; 6 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1262 || n = 1267 || n = 1272)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 5 = 4) && ((n > 1263 && n < 1285)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1281)
	; 5 bytes
	ldx #0
	dex
	Jne *-1
.elseif(n = 1285)
	; 7 bytes
	tya
	ldy #0
	dey
	Jne *-1
	tay
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
.elseif(n = 1288)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 1290 to 1328
.if(n = 1290)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1291)
	; 7 bytes
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
.elseif(n = 1292)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1293 || n = 1302 || n = 1318)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1294)
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
.elseif((n .mod 7 = 1) && ((n > 1295 && n < 1325)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1297 || n = 1306 || n = 1315)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1298 || n = 1305 || n = 1308 || n = 1314 || n = 1319 || n = 1322 || n = 1326)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1299)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1300 || n = 1307)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
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
.elseif(n = 1312)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1313)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1316)
	; 8 bytes
	tya
	ldx #164 ;hides 'ldy zp'
	nop
	dex
	Jne *-3
	tay
.elseif(n = 1320)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	nop
	dex
	Jne *-3
.elseif(n = 1321)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1323)
	; 7 bytes
	bit <$2C ;hides 'bit $6DA2'
	ldx #109
	inx
	Jne *-4
.elseif(n = 1325)
	; 7 bytes
	ldx #26 ;hides 'nop'
	inc $F6,X
	inx
	Jpl *-4
.elseif(n = 1327)
	; 7 bytes
	php
	lax #5
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1328)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
	Jne *-3
.endif
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
.elseif(n = 1330 || n = 1340 || n = 1344 || n = 1347 || n = 1354 || n = 1361)
	; 7..8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1335 || n = 1356 || n = 1363)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1336)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1339)
	; 7 bytes
	ldx #122 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 1341)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1343)
	; 7 bytes
	ldx #121
	inc $F6,X
	dex
	Jpl *-3
.elseif(n = 1346)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 1364)
	; 8 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	inx
	Jpl *-5
.elseif(n = 1365)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1367)
	; 6 bytes
	ldx #152 ;hides 'tya'
	nop
	dex
	Jne *-3
.endif
.else
; 1368 to 1406
.if((n .mod 7 = 3) && ((n > 1367 && n < 1390)))
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1369 || n = 1378 || n = 1396 || n = 1405)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1370 || n = 1384)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1371 || n = 1391)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1372 || n = 1400)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.elseif(n = 1377)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1379)
	; 7 bytes
	ldx #138 ;hides 'txa'
	lda $A5
	dex
	Jne *-4
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
	; 7 bytes
	bit <$2C ;hides 'bit $9AA2'
	ldx #154
	dex
	Jne *-4
.elseif(n = 1388)
	; 7 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1390)
	; 7 bytes
	ldx #234 ;hides 'nop'
	inc $F6,X
	dex
	Jmi *-4
.elseif(n = 1395)
	; 7 bytes
	bit <$2C ;hides 'bit $9BA2'
	ldx #155
	dex
	Jne *-4
.elseif(n = 1397 || n = 1402)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1398)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.elseif(n = 1403)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
.elseif(n = 1404)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1410 || n = 1416 || n = 1417 || n = 1431 || n = 1434 || n = 1435 || n = 1438 || n = 1445)
	; 7..8 bytes
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
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1418)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1419)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1424)
	; 7 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 1426 || n = 1428)
	; 7..8 bytes
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
.elseif(n = 1437)
	; 8 bytes
	ldx #130 ;hides 'nop #imm'
	inc $EA,X ;hides 'nop'
	nop
	dex
	Jne *-5
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1444)
	; 8 bytes
	ldx #110
	nop
	inc $F6,X
	dex
	Jpl *-4
.endif
.else
; 1446 to 1484
.if(n = 1446 || n = 1460 || n = 1474)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1447 || n = 1454 || n = 1482)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
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
.elseif(n = 1451 || n = 1461)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1452 || n = 1466 || n = 1470 || n = 1473 || n = 1479 || n = 1480 || n = 1483)
	; 7..8 bytes
	delay_n (n-2)
	nop
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
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 1487 || n = 1490 || n = 1494 || n = 1498 || n = 1500 || n = 1501 || n = 1508 || n = 1514 || n = 1515)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1489)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1491 || n = 1511)
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
	ldx #69 ;hides 'eor zp'
	nop
	inx
	Jne *-3
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
.elseif(n = 1504)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1505 || n = 1521)
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
.elseif(n = 1509 || n = 1516 || n = 1523)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1510)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1512)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1517)
	; 7 bytes
	ldx #138 ;hides 'txa'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1518)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1519)
	; 7 bytes
	ldx #152 ;hides 'tya'
	lda $A5
	dex
	Jne *-4
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1526 || n = 1535)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1528)
	; 7 bytes
	ldx #169 ;hides 'lda #imm'
	inc $EA,X ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 1529)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	inx
	Jne *-3
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
.elseif(n = 1533)
	; 7 bytes
	ldx #138 ;hides 'txa'
	inc $F6,X
	inx
	Jne *-4
.elseif(n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1557)
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
	ldx #85 ;hides 'eor zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1539)
	; 7 bytes
	bit <$2C ;hides 'bit $ABA2'
	ldx #171
	dex
	Jne *-4
.elseif(n = 1544)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1545 || n = 1553)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1546)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1547)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1554)
	; 8 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1556)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1559)
	; 7 bytes
	ldx #173 ;hides 'lda abs'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 1560)
	; 7 bytes
	ldx #173 ;hides 'lda abs'
	inc $F6,X
	dex
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1564 || n = 1571 || n = 1578 || n = 1586 || n = 1592 || n = 1595 || n = 1599)
	; 7..8 bytes
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
.elseif(n = 1572 || n = 1579 || n = 1600)
	; 8 bytes
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
.elseif(n = 1580 || n = 1587)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1581 || n = 1591)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1584)
	; 7 bytes
	php
	lax #6
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1588)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1598)
	; 7 bytes
	ldx #250 ;hides 'nop'
	inc $F6,X
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
.elseif(n = 1605 || n = 1606 || n = 1613 || n = 1620 || n = 1634)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1607)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1608 || n = 1622 || n = 1636)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
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
.elseif(n = 1614)
	; 7 bytes
	ldx #77 ;hides 'eor abs'
	inc $F6,X
	inx
	Jne *-4
.elseif(n = 1615 || n = 1624 || n = 1629)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1616 || n = 1630)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1617)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1619)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1623)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
.elseif(n = 1626 || n = 1635 || n = 1640)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1627)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
.elseif(n = 1628)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1631)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1633)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
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
.if(n = 1641)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1642 || n = 1672 || n = 1679)
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
	ldx #73 ;hides 'eor #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1647 && n < 1676)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1649 || n = 1665 || n = 1673)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1650)
	; 7 bytes
	ldx #165 ;hides 'lda zp'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1651)
	; 7 bytes
	ldx #138 ;hides 'txa'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 1652 || n = 1662 || n = 1668 || n = 1669 || n = 1676)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1654)
	; 7 bytes
	ldx #73 ;hides 'eor #imm'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
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
.elseif(n = 1658 || n = 1663 || n = 1670 || n = 1677)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1659)
	; 7 bytes
	ldx #90 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 1661)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1664)
	; 7 bytes
	ldx #<$2C ;hides 'bit $B9A2'
	ldx #185
	dex
	Jne *-4
.elseif(n = 1671)
	; 7 bytes
	ldx #152 ;hides 'tya'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1678)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.endif
.else
; 1680 to 1718
.if(n = 1680 || n = 1687 || n = 1694 || n = 1708)
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
.elseif(n = 1683 || n = 1686 || n = 1690 || n = 1697 || n = 1703 || n = 1704 || n = 1715 || n = 1718)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1693 || n = 1711)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1685 || n = 1699)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1689 || n = 1713)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
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
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1698 || n = 1712)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1700)
	; 7 bytes
	ldx #<$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jne *-4
.elseif(n = 1705 || n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1706)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
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
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
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
.elseif(n = 1722 || n = 1725 || n = 1732 || n = 1739 || n = 1743 || n = 1746 || n = 1753 || n = 1754)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1752)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724)
	; 8 bytes
	ldx #26 ;hides 'nop'
	inc $F6,X
	inx
	inx
	Jne *-5
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
	php
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	plp
.elseif(n = 1731)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1734 || n = 1748)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1735)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
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
.elseif(n = 1750)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
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
.elseif(n = 1762 || n = 1767 || n = 1770 || n = 1787 || n = 1788 || n = 1795)
	; 7..8 bytes
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
.elseif(n = 1776)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1777 || n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1780)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 1784)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1796)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
.if((n >= 1797) && (n < 1816))
; 1797 to 1815
.if(n = 1797)
	; 8 bytes
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
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
.elseif(n = 1801 || n = 1810)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1802)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1803)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1804)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	lda $A5
	dex
	Jne *-5
.elseif(n = 1805)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
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
.elseif(n = 1809)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1811)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1812 || n = 1815)
	; 8 bytes
	delay_n (n-2)
	nop
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
.endif
.else
; 1816 to 1835
.if(n = 1816)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1817)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1818)
	; 7 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	dex
	Jne *-4
.elseif(n = 1819 || n = 1828)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1820 || n = 1829)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1821 || n = 1831)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1822)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1823)
	; 7 bytes
	ldx #152 ;hides 'tya'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 1824)
	; 8 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1825)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 1830)
	; 8 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 1832)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1833)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1834)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 1835)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
.endif
.endif
.else
.if((n >= 1836) && (n < 1856))
; 1836 to 1855
.if(n = 1836)
	; 7 bytes
	bit <$2C ;hides 'bit $CCA2'
	ldx #204
	dex
	Jne *-4
.elseif(n = 1837 || n = 1846 || n = 1855)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1838)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1839)
	; 7 bytes
	ldx #184 ;hides 'clv'
	bit $24
	dex
	Jne *-4
.elseif(n = 1840)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1841)
	; 7 bytes
	php
	lax #7
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1842)
	; 8 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1843 || n = 1848)
	; 8 bytes
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
.elseif(n = 1847)
	; 8 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1849)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1850)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1851)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1852)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.endif
.else
; 1856 to 1875
.if(n = 1856 || n = 1859 || n = 1866 || n = 1867 || n = 1874)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1857 || n = 1865)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1858)
	; 8 bytes
	ldx #<$2C ;hides 'bit $A9A2'
	ldx #169
	nop
	dex
	Jne *-5
.elseif(n = 1860)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
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
.elseif(n = 1864 || n = 1873)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1868)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1869)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1870)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 1871)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
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
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
.if((n >= 1876) && (n < 1915))
; 1876 to 1914
.if(n = 1876 || n = 1878 || n = 1894 || n = 1896)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1877)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1879)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1880)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	delay_n 4
	inx
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
.elseif(n = 1883 || n = 1884 || n = 1895 || n = 1903 || n = 1906)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1885 || n = 1897)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1886 || n = 1912)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 1887)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1888)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
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
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
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
	ldx #45 ;hides 'and abs'
	inc $F6,X
	inx
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
	inc $F6,X
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
.elseif(n = 1920 || n = 1928 || n = 1938 || n = 1939 || n = 1946 || n = 1947)
	; 8 bytes
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
.elseif(n = 1930 || n = 1949)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1931)
	; 7 bytes
	ldx #138 ;hides 'txa'
	pla
	pha
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
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1940)
	; 8 bytes
	ldx #194 ;hides 'nop #imm'
	nop $FF44 ;hides 'nop zp'
	dex
	Jne *-5
.elseif(n = 1941 || n = 1951)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1942)
	; 7 bytes
	ldx #41 ;hides 'and #imm'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1943)
	; 7 bytes
	ldx #<$2C ;hides 'bit $D8A2'
	ldx #216
	dex
	Jne *-4
.elseif(n = 1944)
	; 7 bytes
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	dex
	Jne *-4
.elseif(n = 1950)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
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
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 1960 || n = 1976 || n = 1984 || n = 1986)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
	; 7 bytes
	ldx #152 ;hides 'tya'
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 1977 || n = 1985)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1978)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	lda $EAEA ;hides two 'nop's
	dex
	Jne *-5
.elseif(n = 1979)
	; 7 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
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
	ldx #181 ;hides 'lda zp,x'
	lda $EA ;hides 'nop'
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
.elseif(n = 1994 || n = 1995 || n = 2002 || n = 2004 || n = 2010 || n = 2012 || (n > 2017 && n < 2021) || n = 2027 || n = 2028)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1996)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
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
.elseif(n = 1999 || n = 2017 || n = 2026)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2000)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 2001 || n = 2009 || n = 2025)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 2003 || n = 2014)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
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
.elseif(n = 2008)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
.elseif(n = 2011 || n = 2021 || n = 2031)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2013)
	; 8 bytes
	ldx #73 ;hides 'eor #imm'
	lda $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 2015)
	; 7 bytes
	ldx #<$2C ;hides 'bit $E0A2'
	ldx #224
	dex
	Jne *-4
.elseif(n = 2016)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2022)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.elseif(n = 2030)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2037 || n = 2038 || n = 2046 || n = 2054 || n = 2055 || n = 2064 || n = 2067)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2039)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	lda $A5
	inx
	Jne *-5
.elseif(n = 2040 || n = 2048 || n = 2066)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 2049)
	; 7 bytes
	ldx #0
	bit $24
	dex
	Jne *-3
.elseif(n = 2050)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $A5A5 ;hides 'lda zp'
	inx
	Jne *-5
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
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	nop
	lda $A5
	inx
	Jne *-5
.elseif(n = 2060)
	; 7 bytes
	ldx #<$2C ;hides 'bit $E5A2'
	ldx #229
	dex
	Jne *-4
.elseif(n = 2063)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	lda $A5
	inx
	Jne *-5
.elseif(n = 2068)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	lda $A5
	inx
	Jne *-5
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
.if(n = 2071 || n = 2080 || n = 2089 || n = 2107)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2072 || n = 2073 || n = 2083 || n = 2093 || n = 2099 || n = 2100 || n = 2103 || n = 2104)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074 || n = 2092)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2075)
	; 8 bytes
	ldx #121
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
.elseif(n = 2082)
	; 8 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 2084)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2086)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2090 || n = 2108)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2095)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
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
.elseif(n = 2098)
	; 7 bytes
	php
	lax #8
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2102)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
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
.elseif(n = 2109)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
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
	ldx #21 ;hides 'ora zp,x'
	nop
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
.elseif(n = 2117 || n = 2126)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2118 || n = 2136 || n = 2139 || n = 2144 || n = 2145 || n = 2148)
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
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2124)
	; 7 bytes
	bit <$2C ;hides 'bit $ECA2'
	ldx #236
	dex
	Jne *-4
.elseif(n = 2127)
	; 7 bytes
	ldx #152 ;hides 'tya'
	pla
	pha
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
	delay_n (n-7)
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2140)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2151 || n = 2171 || n = 2181)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 2151 && n < 2180)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2153 || n = 2154 || n = 2162 || n = 2163 || n = 2172 || n = 2175 || n = 2182)
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2158)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 2159)
	; 7 bytes
	ldx #<$2C ;hides 'bit $F0A2'
	ldx #240
	dex
	Jne *-4
.elseif(n = 2160)
	; 7 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	dex
	Jne *-4
.elseif(n = 2164 || n = 2165 || n = 2174 || n = 2176)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2180)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2183)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	lda $A5
	dex
	Jne *-5
.elseif(n = 2184)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	nop
	inc $F6,X
	inx
	Jne *-5
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
.elseif(n = 2189)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
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
	ldx #73 ;hides 'eor #imm'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
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
	inc $F6,X
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
.elseif(n = 2218 || n = 2219)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2220)
	; 8 bytes
	php
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2222)
	; 7 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	inx
	Jne *-4
.elseif(n = 2223)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	inx
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
.elseif(n = 2229 || n = 2264)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2230)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2231)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2232)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	nop
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2232 && n < 2261)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2234 || n = 2256)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2235 || n = 2236 || n = 2244 || n = 2247 || n = 2252 || n = 2253 || n = 2262 || n = 2263)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2237)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
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
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	nop
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
	bit <$2C ;hides 'bit $06A2'
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
.elseif(n = 2270 || n = 2273 || n = 2279 || n = 2280 || n = 2288 || n = 2298)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2271 || n = 2291 || n = 2301)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2272 || n = 2274 || n = 2275 || n = 2282 || n = 2284 || n = 2290 || n = 2292)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
	inc $F6,X
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
	; 7 bytes
	ldx #26 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 2302)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
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
.elseif(n = 2306 || n = 2307 || n = 2313 || n = 2324 || n = 2335 || n = 2343)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2312 || n = 2314 || n = 2316 || n = 2318 || n = 2327 || n = 2337)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2311 || n = 2321 || n = 2331)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2315 || n = 2328)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
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
	bit $24
	inx
	Jne *-4
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
	inc $F6,X
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
.elseif(n = 2330)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2332)
	; 8 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
.elseif(n = 2334)
	; 8 bytes
	ldx #44 ;hides 'bit abs'
	inc @zptemp
	nop
	inx
	Jne *-5
.elseif(n = 2336)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
.elseif(n = 2338)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2339)
	; 7 bytes
	ldx #234 ;hides 'nop'
	bit $24
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
	ldx #(n-10)/11 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2366)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2345 || n = 2348 || n = 2349 || n = 2356 || n = 2358 || n = 2359 || n = 2369 || n = 2370 || n = 2374 || n = 2378 || n = 2382)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2350)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $EAEA ;hides two 'nop's
	dex
	Jne *-5
.elseif((n .mod 10 = 1) && ((n > 2350 && n < 2382)))
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2352)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	delay_n 4
	nop
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
.elseif(n = 2355)
	; 7 bytes
	php
	lax #9
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2360)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	inc $EA,X ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2362)
	; 8 bytes
	ldx #169 ;hides 'lda #imm'
	lda $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
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
.elseif(n = 2372)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	inc $EA,X ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 2375)
	; 7 bytes
	ldx #58 ;hides 'nop'
	inc @zptemp
	inx
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
	inc $F6,X
	dex
	Jne *-4
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2390 || n = 2393 || n = 2403 || n = 2412 || n = 2415)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2385 || n = 2392 || n = 2394 || n = 2396 || n = 2400 || n = 2402 || n = 2405 || n = 2416 || n = 2417)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2388 || n = 2410)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2389 || n = 2401 || n = 2413)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2391 || n = 2411 || n = 2421)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2395 || n = 2409)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2397)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2398)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	lda $A5
	inx
	Jne *-5
.elseif(n = 2399)
	; 7 bytes
	ldx #56 ;hides 'sec'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2404)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2406)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2407)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
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
.elseif(n = 2414)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2418)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
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
.if(n = 2422 || n = 2424 || n = 2426 || n = 2429 || n = 2436 || n = 2448 || n = 2450 || n = 2455)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2423)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2425 || n = 2437 || n = 2449)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2427 || n = 2433 || n = 2453 || n = 2456)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2428)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2434)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $A5A5 ;hides 'lda zp'
	inx
	Jne *-5
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2438)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2439)
	; 8 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 2440)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2442)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2444)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2445 || n = 2458)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2446)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	inx
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
.elseif(n = 2459)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	inx
	Jne *-5
.elseif(n = 2460)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	pla
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
.elseif(n = 2462 || n = 2468 || n = 2469 || n = 2472 || n = 2480 || n = 2482 || n = 2488 || n = 2492 || n = 2494 || n = 2495)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2463 || n = 2467 || n = 2475 || n = 2478 || n = 2483 || n = 2489)
	; 8 bytes
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2466)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2470)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $A5A5 ;hides 'lda zp'
	inx
	Jne *-5
.elseif(n = 2471 || n = 2481 || n = 2491)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2474)
	; 8 bytes
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	nop
	inx
	Jne *-5
.elseif(n = 2477)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 2479 || n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2484)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2486)
	; 8 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
.elseif(n = 2490)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	pla
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
	; 7 bytes
	ldx #250 ;hides 'nop'
	nop $04
	dex
	Jne *-4
.elseif(n = 2500)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
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
.elseif(n = 2502 || n = 2503 || n = 2512 || n = 2513 || n = 2522 || n = 2523)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2506 || (n > 2513 && n < 2517) || (n > 2523 && n < 2528) || n = 2532 || (n > 2536 && n < 2540))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2505)
	; 8 bytes
	ldx #232 ;hides 'inx'
	inc @zptemp
	inx
	inx
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
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2517)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	inc $EA,X ;hides 'nop'
	inx
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
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 2520)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2528)
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
.elseif(n = 2530)
	; 8 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
.elseif(n = 2534)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2536)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
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
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2542 || n = 2553 || n = 2564)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2543 || n = 2544 || n = 2547 || n = 2555 || n = 2559 || n = 2560 || n = 2566 || n = 2571)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2545 || n = 2557 || n = 2569)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2546 || n = 2548 || n = 2550 || n = 2554 || n = 2556 || n = 2565 || n = 2567 || n = 2570 || n = 2572 || n = 2576)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2549 || n = 2563 || n = 2577)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
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
.elseif(n = 2558)
	; 7 bytes
	ldx #<$9D ;hides 'sta $00A2,X'
	ldx #0
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
.elseif(n = 2578)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 2580 || n = 2587 || n = 2589 || n = 2590 || n = 2598 || n = 2600 || n = 2609 || n = 2611 || n = 2613)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 12 = 1) && ((n > 2580 && n < 2618)))
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
.elseif(n = 2583)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2584)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2585 || n = 2607 || n = 2610 || n = 2614)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2586 || n = 2597 || n = 2608)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2588 || n = 2601)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2591)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2592)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2594)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2595)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
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
	inc $F6,X
	inx
	Jne *-4
.elseif(n = 2602)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2603)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2604)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 2606)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2612)
	; 7 bytes
	php
	lax #10
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2615)
	; 7 bytes
	ldx #218 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2616)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	delay_n 4
	inx
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2620 || n = 2622 || n = 2624 || n = 2634 || n = 2644 || n = 2645 || n = 2648 || n = 2656)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2621 || n = 2631 || n = 2632 || n = 2643 || n = 2654 || n = 2655)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2623 || n = 2642)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2625)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 2626)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $EAEA ;hides two 'nop's
	inx
	Jne *-5
.elseif(n = 2627)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	nop
	inx
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
.elseif(n = 2635)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inc $EA,X ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 2636)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EAEA ;hides two 'nop's
	inx
	Jne *-5
.elseif(n = 2637)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 2638)
	; 8 bytes
	ldx #202 ;hides 'dex'
	.repeat 4
	 nop
	.endrepeat
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
	delay_n 4
	nop
	inx
	Jne *-5
.elseif(n = 2646)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inc $EA,X ;hides 'nop'
	nop
	inx
	Jne *-5
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
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if(n = 2657 || n = 2658 || n = 2660 || n = 2668 || n = 2669 || n = 2671 || n = 2678 || n = 2688 || n = 2690)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2659)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	inc $EA,X ;hides 'nop'
	nop
	inx
	Jne *-5
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
	inc $F6,X
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
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2667 || n = 2676 || n = 2679 || n = 2687 || n = 2693)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2670)
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
.elseif(n = 2673)
	; 8 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	nop
	inx
	Jne *-5
.elseif(n = 2680)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2681)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 2691)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	inc $EA,X ;hides 'nop'
	dex
	Jne *-4
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
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2697 || n = 2700 || n = 2702 || n = 2708 || n = 2711 || n = 2722 || n = 2723 || n = 2726 || (n > 2731 && n < 2735))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2698 || n = 2709 || n = 2715 || n = 2720 || n = 2727)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2699)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	inc $F6,X
	dex
	Jne *-5
.elseif(n = 2701 || n = 2713 || n = 2725)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 2710)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
.elseif(n = 2712)
	; 8 bytes
	ldx #194 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2714)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	inc $F6,X
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
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 2724)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	inc $EA,X ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 2728)
	; 8 bytes
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
.elseif(n = 2730)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 2736 || n = 2743 || n = 2747 || n = 2748 || n = 2752 || n = 2758 || n = 2765 || n = 2766 || n = 2769)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2739 || n = 2742 || n = 2753 || n = 2763 || n = 2764)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2740 || n = 2751 || n = 2762)
	; 7 bytes
	ldx #(n-1)/11
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2741)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2744 || n = 2757 || n = 2770)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
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
.elseif(n = 2750)
	; 8 bytes
	bit <$2C ;hides 'bit $06A2'
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
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2756)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.else
; 2774 to 2812
.if(n = 2774 || (n > 2775 && n < 2781) || n = 2788 || n = 2791 || n = 2798 || n = 2802 || n = 2803 || n = 2810 || n = 2812)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2775 || n = 2786 || n = 2808 || n = 2811)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2781)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
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
	inc $F6,X
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
.elseif(n = 2789)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2790)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 2792)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	lda $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
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
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2799)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2800)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
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
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2832 || n = 2851)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2814 || (n > 2823 && n < 2827) || n = 2828 || n = 2836 || n = 2837 || n = 2839 || n = 2842 || n = 2846 || n = 2849 || n = 2850)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
	; 7 bytes
	ldx #0
	inc $F6,X
	dex
	Jne *-3
.elseif(n = 2818)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $A5A5 ;hides 'lda zp'
	inx
	Jne *-5
.elseif(n = 2819)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	lda $A5
	inx
	Jne *-5
.elseif(n = 2820)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	pla
	pha
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
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2823 || n = 2835 || n = 2847)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2827)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2830)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	inx
	Jne *-5
.elseif(n = 2831)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	lda $A5
	inx
	Jne *-5
.elseif(n = 2834)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2838)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
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
.elseif(n = 2844)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.endif
.else
; 2852 to 2890
.if(n = 2852)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2853 || n = 2854 || n = 2856 || n = 2858 || n = 2862 || (n > 2863 && n < 2869) || n = 2872 || (n > 2874 && n < 2879) || n = 2880 || n = 2882 || n = 2884 || n = 2886 || n = 2888)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2855)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2857 || n = 2881)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2859 || n = 2871 || n = 2883)
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
.elseif(n = 2861 || n = 2874 || n = 2887)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2863)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
.elseif(n = 2869)
	; 7 bytes
	php
	lax #11
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2870 || n = 2889)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2873)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2879)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	pla
	pha
	dex
	Jne *-5
.elseif(n = 2885)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2890)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
.endif
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2908 || n = 2925 || n = 2942 || n = 2959)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2892 || n = 2894 || n = 2896 || n = 2898 || (n > 2901 && n < 2905) || n = 2906 || n = 2909 || n = 2910 || n = 2912 || n = 2914 || n = 2916 || n = 2918 || n = 2920 || (n > 2921 && n < 2925) || n = 2928 || n = 2930 || (n > 2931 && n < 2935) || (n > 2935 && n < 2939) || n = 2940 || n = 2944 || n = 2945 || (n > 2947 && n < 2952) || n = 2954 || (n > 2955 && n < 2959) || n = 2960 || n = 2963 || n = 2966 || n = 2968)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2897)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 2899 || n = 2913 || n = 2927 || n = 2955)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2900 || n = 2926 || n = 2939 || n = 2952)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2901 || n = 2921 || n = 2961)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2911)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	pla
	pha
	dex
	Jne *-5
.elseif(n = 2915)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	dex
	Jne *-5
.elseif(n = 2935)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
.elseif(n = 2946)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2947)
	; 8 bytes
	ldx #122 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 2962)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2964)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
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
.elseif((n > 2969 && n < 2976) || n = 2980 || n = 2982 || (n > 2984 && n < 2989) || n = 2990 || n = 2992 || (n > 2993 && n < 2997) || n = 2998 || n = 3005 || (n > 3006 && n < 3010) || n = 3014 || n = 3016 || n = 3018 || n = 3020 || n = 3023 || n = 3026 || n = 3028 || (n > 3030 && n < 3035) || n = 3036 || n = 3038 || n = 3042 || n = 3045 || n = 3046)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2976 || n = 2993)
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
.elseif((n .mod 13 = 1) && (n = 2978 || (n > 3003 && n < 3044)))
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 2979 || n = 2991 || n = 3003 || n = 3027)
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2999)
	; 7 bytes
	ldx #250 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 3000)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 3002)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 3006)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3010)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $EAEA ;hides two 'nop's
	inx
	Jne *-5
.elseif(n = 3012)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	delay_n 4
	nop
	inx
	Jne *-5
.elseif(n = 3015)
	; 7 bytes
	ldx #24 ;hides 'clc'
	inc $F6,X
	inx
	Jne *-4
.elseif(n = 3019)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inc $EA,X ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 3024)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 3029)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3035)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3040)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
.elseif(n = 3041)
	; 7 bytes
	ldx #234 ;hides 'nop'
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 3044)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
.endif
.else
; 3047 to 3125
.if(n = 3047 || n = 3048 || n = 3050 || n = 3057 || n = 3064 || n = 3068 || n = 3071 || n = 3072 || n = 3077 || (n > 3082 && n < 3094) || n = 3096 || n = 3097 || n = 3099 || n = 3100 || (n > 3101 && n < 3108) || n = 3110 || n = 3111 || (n > 3112 && n < 3117) || (n > 3117 && n < 3121) || n = 3122 || n = 3124 || n = 3125)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3049 || n = 3061)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3051)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3052)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $EAEA ;hides two 'nop's
	inx
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 3052 && n < 3124)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3054)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	delay_n 4
	nop
	inx
	Jne *-5
.elseif(n = 3055)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
.elseif(n = 3056 || n = 3069 || n = 3082 || n = 3108 || n = 3121)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 3058)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	inc $07F6,X ;hides 'inc $07,X'
	inx
	Jne *-5
.elseif(n = 3059)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3060 || n = 3098 || n = 3117)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3062)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	inc $EA,X ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 3063 || n = 3075)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3065)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inc $07F6,X ;hides 'inc $07,X'
	inx
	Jne *-5
.elseif(n = 3066)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3070)
	; 8 bytes
	ldx #<$9D ;hides 'sta $00A2,X'
	ldx #0
	nop
	dex
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
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3076)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
.elseif(n = 3078 || n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3079)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inc $07F6,X ;hides 'inc $07,X'
	inx
	Jne *-5
.elseif(n = 3080)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	nop
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3094)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3101)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
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
	php
	lax #12
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3127 || (n > 3129 && n < 3134) || n = 3135 || (n > 3137 && n < 3141) || (n > 3141 && n < 3146) || (n > 3147 && n < 3151) || n = 3152 || n = 3154 || n = 3156 || n = 3157 || n = 3159 || n = 3162 || n = 3166 || n = 3168 || n = 3172 || (n > 3175 && n < 3179) || n = 3182 || n = 3183 || n = 3185 || (n > 3186 && n < 3191) || n = 3192 || n = 3195 || n = 3196 || n = 3198 || n = 3200 || n = 3202 || n = 3203)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3128)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3129 || n = 3146 || n = 3163 || n = 3180 || n = 3197)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif((n .mod 13 = 1) && ((n > 3133 && n < 3187)))
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 3136 || n = 3155 || n = 3174)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && ((n > 3136 && n < 3194)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3141 || n = 3161 || n = 3181 || n = 3201)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 3164)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	inc $F6,X
	dex
	Jne *-5
.elseif(n = 3167)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3169)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dex
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
.elseif(n = 3175)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
.elseif(n = 3184)
	; 8 bytes
	ldx #(n-12)/13 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3191)
	; 8 bytes
	ldx #152 ;hides 'tya'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3194)
	; 8 bytes
	ldx #(n-14)/15 ;hides 'nop zp,x'
	inx ;first loop only
	inc $F6,X
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
.if((n > 3203 && n < 3207) || (n > 3208 && n < 3212) || n = 3213 || (n > 3214 && n < 3219) || n = 3220 || (n > 3221 && n < 3225) || (n > 3225 && n < 3231) || n = 3232 || n = 3234 || n = 3236 || n = 3237 || n = 3239 || n = 3240 || (n > 3242 && n < 3247) || (n > 3251 && n < 3258) || n = 3259 || n = 3262 || (n > 3265 && n < 3269) || (n > 3270 && n < 3275) || n = 3278)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3207 || n = 3221 || n = 3235 || n = 3263 || n = 3277)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3208)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
.elseif((n .mod 13 = 1) && ((n > 3211 && n < 3265)))
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 3214 || n = 3248 || n = 3265)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3219)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3231)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3233)
	; 8 bytes
	ldx #232 ;hides 'inx'
	inc @zptemp
	dex
	dex
	Jne *-5
.elseif(n = 3241 || n = 3261 || n = 3281)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3242)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
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
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 3250)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3258)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3260)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 3269)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3270)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3276)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3279)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3280)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.endif
.endif
.else
; 3282 to 3437
.if(n = 3282 || n = 3299 || n = 3350 || n = 3367 || n = 3384 || n = 3401 || n = 3418 || n = 3435)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283 || n = 3284 || n = 3287 || (n > 3291 && n < 3295) || (n > 3295 && n < 3299) || n = 3302 || n = 3304 || n = 3306 || (n > 3307 && n < 3315) || n = 3318 || n = 3320 || (n > 3321 && n < 3326) || n = 3327 || n = 3328 || (n > 3329 && n < 3333) || (n > 3333 && n < 3341) || n = 3342 || n = 3344 || n = 3346 || n = 3348 || n = 3349 || (n > 3350 && n < 3361) || n = 3362 || n = 3363 || n = 3365 || n = 3366 || (n > 3367 && n < 3375) || (n > 3375 && n < 3381) || n = 3382 || n = 3387 || n = 3388 || n = 3390 || (n > 3391 && n < 3401) || (n > 3403 && n < 3407) || (n > 3407 && n < 3412) || n = 3413 || n = 3414 || n = 3416 || n = 3419 || n = 3420 || (n > 3421 && n < 3429) || n = 3430 || (n > 3431 && n < 3435) || n = 3436 || n = 3437)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3285)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3286)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 3288 || n = 3307 || n = 3326 || n = 3345 || n = 3364 || n = 3402 || n = 3421)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3289)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3290 || n = 3316)
	; 8 bytes
	ldx #(n-1)/13
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && ((n > 3290 && n < 3432)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3295)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3300)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3301 || n = 3321 || n = 3341 || n = 3381)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3303)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inc @zptemp
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
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3329)
	; 8 bytes
	ldx #0
	nop
	inc $F6,X
	dex
	Jne *-4
.elseif(n = 3343)
	; 8 bytes
	ldx #152 ;hides 'tya'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3383)
	; 7 bytes
	php
	lax #13
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3385)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3386)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3391)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3407)
	; 8 bytes
	ldx #(n-15)/16 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3412)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 3415)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 3429)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
; 3438 to 3593
.if(n = 3438 || n = 3439 || (n > 3441 && n < 3445) || (n > 3445 && n < 3449) || n = 3451 || n = 3453 || (n > 3454 && n < 3459) || n = 3460 || n = 3462 || n = 3463 || (n > 3464 && n < 3469) || (n > 3469 && n < 3473) || (n > 3473 && n < 3478) || n = 3480 || (n > 3481 && n < 3485) || (n > 3487 && n < 3495) || n = 3496 || n = 3498 || n = 3502 || (n > 3503 && n < 3509) || (n > 3510 && n < 3514) || (n > 3516 && n < 3520) || n = 3522 || (n > 3524 && n < 3528) || (n > 3529 && n < 3535) || n = 3536 || n = 3540 || n = 3542 || (n > 3543 && n < 3554) || n = 3555 || n = 3556 || (n > 3557 && n < 3561) || (n > 3561 && n < 3571) || n = 3572 || (n > 3573 && n < 3581) || (n > 3581 && n < 3585) || n = 3586 || n = 3587 || (n > 3588 && n < 3592) || n = 3593)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3440 || n = 3478 || n = 3497 || n = 3516 || n = 3535 || n = 3573 || n = 3592)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3441 || n = 3461 || n = 3481 || n = 3521 || n = 3541 || n = 3561 || n = 3581)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
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
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3450)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 3452 || n = 3469 || n = 3486 || n = 3503 || n = 3537 || n = 3554 || n = 3588)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3454)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3464)
	; 8 bytes
	ldx #232 ;hides 'inx'
	inc $F6,X
	dex
	dex
	Jne *-5
.elseif(n = 3479)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	inc $F6,X
	inx
	Jne *-5
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
.elseif(n = 3500)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3509)
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	inc $F6,X
	dex
	Jne *-5
.elseif(n = 3510)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
.elseif(n = 3514)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3520)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
.elseif(n = 3523)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	inc $07F6,X ;hides 'inc $07,X'
	inx
	Jne *-5
.elseif(n = 3524)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3528)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inc $F6,X
	inx
	Jne *-5
.elseif(n = 3538)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inc $07F6,X ;hides 'inc $07,X'
	inx
	Jne *-5
.elseif(n = 3539)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inc $F6,X
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
.if((n > 3593 && n < 3601) || (n > 3601 && n < 3605) || (n > 3605 && n < 3611) || (n > 3611 && n < 3621) || (n > 3622 && n < 3630) || (n > 3630 && n < 3639) || n = 3643 || (n > 3644 && n < 3649) || n = 3650 || n = 3652 || n = 3654 || n = 3657 || n = 3658 || n = 3660 || (n > 3661 && n < 3668) || (n > 3668 && n < 3673) || (n > 3674 && n < 3679) || n = 3680 || (n > 3681 && n < 3687) || n = 3688 || n = 3689 || (n > 3690 && n < 3695) || (n > 3695 && n < 3701) || (n > 3701 && n < 3706) || (n > 3707 && n < 3711) || (n > 3711 && n < 3721) || n = 3722 || n = 3723 || (n > 3725 && n < 3741) || n = 3742 || (n > 3744 && n < 3749))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3601 || n = 3621 || n = 3641 || n = 3661 || n = 3681 || n = 3701 || n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3605 || n = 3622 || n = 3639 || n = 3656 || n = 3673 || n = 3690 || n = 3707 || n = 3724 || n = 3741)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3611 || n = 3630 || n = 3649 || n = 3668 || n = 3687 || n = 3706 || n = 3725 || n = 3744)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3640)
	; 7 bytes
	php
	lax #14
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3642)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3644)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	inc $F6,X
	dex
	Jne *-5
.elseif(n = 3651)
	; 8 bytes
	ldx #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 3653)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
.elseif(n = 3655)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
.elseif(n = 3659)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	inc $F6,X
	dex
	Jne *-5
.elseif(n = 3674)
	; 8 bytes
	ldx #(n-14)/15 ;hides 'nop zp,x'
	inx ;first loop only
	inc $F6,X
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
	inc $F6,X
	dex
	Jne *-5
.elseif(n = 3750)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
; 3751 to 4062
.if((n > 3750 && n < 3754) || (n > 3754 && n < 3758) || n = 3762 || n = 3764 || (n > 3765 && n < 3770) || (n > 3770 && n < 3775) || (n > 3775 && n < 3780) || (n > 3782 && n < 3791) || (n > 3792 && n < 3799) || n = 3800 || (n > 3801 && n < 3809) || (n > 3809 && n < 3820) || (n > 3821 && n < 3826) || (n > 3826 && n < 3839) || n = 3840 || n = 3842 || (n > 3843 && n < 3856) || n = 3857 || n = 3859 || n = 3862 || (n > 3863 && n < 3877) || (n > 3877 && n < 3881) || (n > 3881 && n < 3887) || (n > 3887 && n < 3894) || n = 3895 || n = 3900 || n = 3902 || (n > 3903 && n < 3911) || (n > 3911 && n < 3915) || (n > 3915 && n < 3919) || n = 3920 || (n > 3921 && n < 3928) || (n > 3928 && n < 3934) || (n > 3934 && n < 3941) || (n > 3941 && n < 3945) || (n > 3945 && n < 3953) || (n > 3953 && n < 3961) || (n > 3962 && n < 3972) || (n > 3972 && n < 3979) || n = 3980 || (n > 3981 && n < 3991) || (n > 3991 && n < 3996) || n = 3997 || n = 3998 || n = 4000 || (n > 4001 && n < 4010) || n = 4011 || n = 4012 || (n > 4013 && n < 4021) || (n > 4021 && n < 4029) || (n > 4030 && n < 4041) || (n > 4041 && n < 4047) || (n > 4048 && n < 4059) || n = 4060 || n = 4062)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3754)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3758 || n = 3792 || n = 3809 || n = 3826 || n = 3843 || n = 3860 || n = 3877 || n = 3894 || n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3979 || n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3759)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3760)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3761 || n = 3781 || n = 3801 || n = 3821 || n = 3841 || n = 3861 || n = 3881 || n = 3901 || n = 3921 || n = 3941 || n = 3961 || n = 3981 || n = 4001 || n = 4021 || n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3763 || n = 3782 || n = 3820 || n = 3839 || n = 3858 || n = 3896 || n = 3915 || n = 3934 || n = 3953 || n = 3972 || n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3765)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3770)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3775)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3780)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
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
.elseif(n = 3897)
	; 7 bytes
	php
	lax #15
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3898)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3899)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3903)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3919)
	; 8 bytes
	ldx #(n-15)/16 ;hides 'nop zp,x'
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
.elseif(n = 4059)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
.endif
.else
; 4063 to 4375
.if(n = 4063 || n = 4065 || n = 4066 || (n > 4067 && n < 4081) || (n > 4081 && n < 4086) || (n > 4086 && n < 4098) || n = 4099 || n = 4100 || (n > 4101 && n < 4105) || (n > 4105 && n < 4115) || (n > 4115 && n < 4121) || n = 4122 || n = 4123 || (n > 4124 && n < 4132) || (n > 4132 && n < 4141) || n = 4142 || (n > 4143 && n < 4149) || (n > 4149 && n < 4154) || n = 4155 || (n > 4157 && n < 4161) || (n > 4162 && n < 4166) || (n > 4166 && n < 4181) || n = 4182 || (n > 4183 && n < 4199) || (n > 4201 && n < 4217) || n = 4218 || n = 4220 || (n > 4221 && n < 4234) || (n > 4234 && n < 4238) || n = 4239 || n = 4240 || (n > 4241 && n < 4251) || (n > 4251 && n < 4257) || (n > 4257 && n < 4261) || (n > 4262 && n < 4268) || (n > 4268 && n < 4276) || (n > 4276 && n < 4281) || (n > 4281 && n < 4285) || (n > 4285 && n < 4295) || (n > 4295 && n < 4301) || (n > 4302 && n < 4314) || (n > 4314 && n < 4319) || n = 4320 || (n > 4321 && n < 4333) || n = 4334 || n = 4335 || (n > 4336 && n < 4341) || (n > 4341 && n < 4352) || n = 4354 || (n > 4355 && n < 4361) || (n > 4361 && n < 4369) || n = 4370 || (n > 4371 && n < 4376))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 4101 || n = 4121 || n = 4141 || n = 4161 || n = 4201 || n = 4221 || n = 4241 || n = 4261 || n = 4281 || n = 4301 || n = 4321 || n = 4341 || n = 4361)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4154)
	; 7 bytes
	php
	lax #16
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4156)
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4355)
	; 8 bytes
	ldx #58 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
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
.if((n > 4375 && n < 4381) || (n > 4381 && n < 4390) || (n > 4390 && n < 4399) || n = 4400 || (n > 4401 && n < 4407) || n = 4408 || n = 4410 || n = 4412 || (n > 4413 && n < 4421) || (n > 4421 && n < 4428) || (n > 4428 && n < 4441) || (n > 4441 && n < 4445) || n = 4446 || (n > 4447 && n < 4461) || (n > 4461 && n < 4465) || (n > 4466 && n < 4481) || (n > 4481 && n < 4485) || (n > 4485 && n < 4501) || n = 4502 || n = 4503 || (n > 4504 && n < 4521) || n = 4522 || (n > 4523 && n < 4541) || (n > 4542 && n < 4561) || (n > 4561 && n < 4577) || n = 4578 || n = 4579 || (n > 4581 && n < 4599) || n = 4600 || (n > 4601 && n < 4618) || n = 4619 || n = 4620 || (n > 4621 && n < 4637) || (n > 4637 && n < 4641) || (n > 4641 && n < 4656) || (n > 4656 && n < 4661) || (n > 4661 && n < 4668) || n = 4669 || (n > 4670 && n < 4675) || (n > 4675 && n < 4681) || (n > 4681 && n < 4694) || (n > 4694 && n < 4701) || (n > 4701 && n < 4713) || (n > 4713 && n < 4721) || (n > 4721 && n < 4732) || (n > 4732 && n < 4741) || (n > 4741 && n < 4749) || n = 4750 || (n > 4751 && n < 4761) || (n > 4761 && n < 4770) || (n > 4770 && n < 4781) || (n > 4781 && n < 4789) || (n > 4789 && n < 4795) || (n > 4795 && n < 4801) || (n > 4801 && n < 4808) || (n > 4808 && n < 4821) || (n > 4821 && n < 4827) || n = 4828 || (n > 4829 && n < 4841) || (n > 4841 && n < 4846) || (n > 4846 && n < 4861) || (n > 4861 && n < 4865) || (n > 4865 && n < 4871) || (n > 4871 && n < 4881) || (n > 4881 && n < 4901) || (n > 4901 && n < 4913) || (n > 4913 && n < 4921) || (n > 4921 && n < 4925) || n = 4926 || (n > 4927 && n < 4941) || (n > 4941 && n < 4961) || (n > 4961 && n < 4981) || (n > 4981 && n < 5001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4381 || n = 4401 || n = 4421 || n = 4441 || n = 4461 || n = 4481 || n = 4501 || n = 4521 || n = 4541 || n = 4581 || n = 4601 || n = 4621 || n = 4641 || n = 4661 || n = 4681 || n = 4701 || n = 4721 || n = 4741 || n = 4761 || n = 4781 || n = 4801 || n = 4821 || n = 4841 || n = 4861 || n = 4881 || n = 4901 || n = 4921 || n = 4941 || n = 4961 || n = 4981)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4390 || n = 4409 || n = 4428 || n = 4447 || n = 4466 || n = 4485 || n = 4504 || n = 4523 || n = 4542 || n = 4561 || n = 4580 || n = 4599 || n = 4618 || n = 4637 || n = 4656 || n = 4675 || n = 4694 || n = 4713 || n = 4732 || n = 4751 || n = 4770 || n = 4789 || n = 4808 || n = 4827 || n = 4846)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 4407)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4411)
	; 7 bytes
	php
	lax #17
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4413 || n = 4670 || n = 4927)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4445)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4465)
	; 8 bytes
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
.elseif(n = 4577)
	; 8 bytes
	ldx #218 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4668)
	; 7 bytes
	php
	lax #18
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4795)
	; 8 bytes
	ldx #218 ;hides 'nop'
	jsr @nearby_rts_15cyc
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
.elseif(n = 4925)
	; 7 bytes
	php
	lax #19
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
; 5001 to 7500
.if(n = 5001 || n = 5021 || n = 5041 || n = 5061 || n = 5081 || n = 5101)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif((n > 5001 && n < 5021) || (n > 5021 && n < 5041) || (n > 5041 && n < 5059) || n = 5060 || (n > 5061 && n < 5081) || (n > 5081 && n < 5101) || n = 5102 || (n > 5103 && n < 5121) || (n > 5121 && n < 5147) || (n > 5147 && n < 5182) || n = 5183 || (n > 5184 && n < 5249) || (n > 5249 && n < 5439) || n = 5440 || (n > 5441 && n < 5499) || (n > 5499 && n < 5696) || n = 5697 || (n > 5698 && n < 5953) || n = 5954 || (n > 5955 && n < 6210) || n = 6211 || (n > 6212 && n < 6467) || n = 6468 || (n > 6469 && n < 6724) || n = 6725 || (n > 6726 && n < 6981) || n = 6982 || (n > 6983 && n < 7238) || n = 7239 || (n > 7240 && n < 7495) || n = 7496 || (n > 7497 && n < 7501))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5059)
	; 8 bytes
	ldx #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
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
.elseif(n = 5182)
	; 7 bytes
	php
	lax #20
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5184 || n = 5441 || n = 5698 || n = 5955 || n = 6212 || n = 6469 || n = 6726 || n = 6983 || n = 7240 || n = 7497)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5249)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5439)
	; 7 bytes
	php
	lax #21
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5499)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5696)
	; 7 bytes
	php
	lax #22
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5953)
	; 7 bytes
	php
	lax #23
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6210)
	; 7 bytes
	php
	lax #24
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6467)
	; 7 bytes
	php
	lax #25
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6724)
	; 7 bytes
	php
	lax #26
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6981)
	; 7 bytes
	php
	lax #27
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7238)
	; 7 bytes
	php
	lax #28
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7495)
	; 7 bytes
	php
	lax #29
	jsr delay_256a_x_33_clocks
	plp
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7752) || n = 7753 || (n > 7754 && n < 8009) || n = 8010 || (n > 8011 && n < 8266) || n = 8267 || (n > 8268 && n < 8523) || n = 8524 || (n > 8525 && n < 8780) || n = 8781 || (n > 8782 && n < 9037) || n = 9038 || (n > 9039 && n < 9294) || n = 9295 || (n > 9296 && n < 9551) || n = 9552 || (n > 9553 && n < 9808) || n = 9809 || (n > 9810 && n < 10001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7752)
	; 7 bytes
	php
	lax #30
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7754 || n = 8011 || n = 8268 || n = 8525 || n = 8782 || n = 9039 || n = 9296 || n = 9553 || n = 9810)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8009)
	; 7 bytes
	php
	lax #31
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8266)
	; 7 bytes
	php
	lax #32
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8523)
	; 7 bytes
	php
	lax #33
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8780)
	; 7 bytes
	php
	lax #34
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9037)
	; 7 bytes
	php
	lax #35
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9294)
	; 7 bytes
	php
	lax #36
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9551)
	; 7 bytes
	php
	lax #37
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9808)
	; 7 bytes
	php
	lax #38
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10065) || n = 10066 || (n > 10067 && n < 10322) || n = 10323 || (n > 10324 && n < 10579) || n = 10580 || (n > 10581 && n < 10836) || n = 10837 || (n > 10838 && n < 11093) || n = 11094 || (n > 11095 && n < 11350) || n = 11351 || (n > 11352 && n < 11607) || n = 11608 || (n > 11609 && n < 11864) || n = 11865 || (n > 11866 && n < 12121) || n = 12122 || (n > 12123 && n < 12378) || n = 12379 || (n > 12380 && n < 12635) || n = 12636 || (n > 12637 && n < 12892) || n = 12893 || (n > 12894 && n < 13149) || n = 13150 || (n > 13151 && n < 13406) || n = 13407 || (n > 13408 && n < 13663) || n = 13664 || (n > 13665 && n < 13920) || n = 13921 || (n > 13922 && n < 14177) || n = 14178 || (n > 14179 && n < 14434) || n = 14435 || (n > 14436 && n < 14691) || n = 14692 || (n > 14693 && n < 14948) || n = 14949 || (n > 14950 && n < 15001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10065)
	; 7 bytes
	php
	lax #39
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10067 || n = 10324 || n = 10581 || n = 10838 || n = 11095 || n = 11352 || n = 11609 || n = 11866 || n = 12123 || n = 12380 || n = 12637 || n = 12894 || n = 13151 || n = 13408 || n = 13665 || n = 13922 || n = 14179 || n = 14436 || n = 14693 || n = 14950)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10322)
	; 7 bytes
	php
	lax #40
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10579)
	; 7 bytes
	php
	lax #41
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10836)
	; 7 bytes
	php
	lax #42
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11093)
	; 7 bytes
	php
	lax #43
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11350)
	; 7 bytes
	php
	lax #44
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11607)
	; 7 bytes
	php
	lax #45
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11864)
	; 7 bytes
	php
	lax #46
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12121)
	; 7 bytes
	php
	lax #47
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12378)
	; 7 bytes
	php
	lax #48
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12635)
	; 7 bytes
	php
	lax #49
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12892)
	; 7 bytes
	php
	lax #50
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13149)
	; 7 bytes
	php
	lax #51
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13406)
	; 7 bytes
	php
	lax #52
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13663)
	; 7 bytes
	php
	lax #53
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13920)
	; 7 bytes
	php
	lax #54
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14177)
	; 7 bytes
	php
	lax #55
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14434)
	; 7 bytes
	php
	lax #56
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14691)
	; 7 bytes
	php
	lax #57
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14948)
	; 7 bytes
	php
	lax #58
	jsr delay_256a_x_33_clocks
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15205) || n = 15206 || (n > 15207 && n < 15462) || n = 15463 || (n > 15464 && n < 15719) || n = 15720 || (n > 15721 && n < 15976) || n = 15977 || (n > 15978 && n < 16233) || n = 16234 || (n > 16235 && n < 16490) || n = 16491 || (n > 16492 && n < 16747) || n = 16748 || (n > 16749 && n < 17004) || n = 17005 || (n > 17006 && n < 17261) || n = 17262 || (n > 17263 && n < 17518) || n = 17519 || (n > 17520 && n < 17775) || n = 17776 || (n > 17777 && n < 18032) || n = 18033 || (n > 18034 && n < 18289) || n = 18290 || (n > 18291 && n < 18546) || n = 18547 || (n > 18548 && n < 18803) || n = 18804 || (n > 18805 && n < 19060) || n = 19061 || (n > 19062 && n < 19317) || n = 19318 || (n > 19319 && n < 19574) || n = 19575 || (n > 19576 && n < 19831) || n = 19832 || (n > 19833 && n < 20001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15205)
	; 7 bytes
	php
	lax #59
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15207 || n = 15464 || n = 15721 || n = 15978 || n = 16235 || n = 16492 || n = 16749 || n = 17006 || n = 17263 || n = 17520 || n = 17777 || n = 18034 || n = 18291 || n = 18548 || n = 18805 || n = 19062 || n = 19319 || n = 19576 || n = 19833)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15462)
	; 7 bytes
	php
	lax #60
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15719)
	; 7 bytes
	php
	lax #61
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15976)
	; 7 bytes
	php
	lax #62
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 16233)
	; 7 bytes
	php
	lax #63
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 16490)
	; 7 bytes
	php
	lax #64
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 16747)
	; 7 bytes
	php
	lax #65
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17004)
	; 7 bytes
	php
	lax #66
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17261)
	; 7 bytes
	php
	lax #67
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17518)
	; 7 bytes
	php
	lax #68
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17775)
	; 7 bytes
	php
	lax #69
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18032)
	; 7 bytes
	php
	lax #70
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18289)
	; 7 bytes
	php
	lax #71
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18546)
	; 7 bytes
	php
	lax #72
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18803)
	; 7 bytes
	php
	lax #73
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19060)
	; 7 bytes
	php
	lax #74
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19317)
	; 7 bytes
	php
	lax #75
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19574)
	; 7 bytes
	php
	lax #76
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19831)
	; 7 bytes
	php
	lax #77
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
