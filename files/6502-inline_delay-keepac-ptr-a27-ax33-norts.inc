; delay_n, delays n cycles with small code
; Will clobber X, Y, Z+N, D, and V
; Will preserve A, S, C, and I (interrupt-disable flag)
;
; Uses JMP (unrelocatable code).

; Requires a zeropage two-byte variable @ptrtemp, that points
; into a memory address that can be safely read and overwritten
; with random data.
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
; - 5 bytes or less when n <= 14
; - 6 bytes or less when n <= 19
; - 7 bytes or less when n <= 1291
; - 8 bytes or less when n <= 1345
; - 9 bytes or less when n <= 2317
; -10 bytes or less when n <= 3117
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=4, avg=5.3, max=7
; - When 21 <= n <= 30: min=5, avg=6.2, max=7
; - When 31 <= n <= 50: min=4, avg=6.0, max=7
; - When 51 <= n <= 75: min=5, avg=6.0, max=7
; - When 76 <= n <= 100: min=5, avg=6.2, max=7
; - When 101 <= n <= 200: min=4, avg=6.2, max=7
; - When 201 <= n <= 300: min=4, avg=6.2, max=7
; - When 301 <= n <= 500: min=4, avg=6.2, max=7
; - When 501 <= n <= 750: min=4, avg=6.2, max=7
; - When 751 <= n <= 1000: min=5, avg=6.3, max=7
; - When 1001 <= n <= 2000: min=4, avg=7.0, max=9
; - When 2001 <= n <= 3000: min=6, avg=8.3, max=10
; - When 3001 <= n <= 5000: min=7, avg=9.8, max=11
; - When 5001 <= n <= 7500: min=9, avg=10.4, max=11
; - When 7501 <= n <= 10000: min=9, avg=10.4, max=11
; - When 10001 <= n <= 20000: min=9, avg=10.5, max=11
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
.elseif(n = 9 || n = 11 || n = 13 || n = 15)
	; 3..6 bytes
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
.elseif(n = 24 || n = 39)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 32)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 34)
	; 5 bytes
	ldy #136 ;hides 'dey'
	dey
	Jmi *-2
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
.if(n = 40 || n = 48 || n = 53 || n = 58 || n = 60 || n = 63 || n = 65 || n = 68 || n = 70 || n = 73 || n = 75)
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
	sta (@ptrtemp,x)
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
	sta (@ptrtemp,x)
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
.elseif(n = 54 || n = 59 || n = 74)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 55)
	; 5 bytes
	ldx #248 ;hides 'sed'
	inx
	Jne *-2
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
.if((n .mod 5 = 4) && (n = 79 || n = 84 || n = 94 || (n > 103 && n < 115) || n = 129 || (n > 138 && n < 155)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 97)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 102 || n = 130 || n = 152)
	; 6..7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	sta (@ptrtemp,x)
.elseif(n = 115 || n = 150)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 119)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	Jmi *-3
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
.if(n = 157 || n = 158 || n = 163 || n = 165 || n = 168 || n = 173 || n = 175 || n = 178 || n = 180 || n = 188 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 203 || n = 205 || n = 208 || n = 210 || n = 213 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 177 || n = 184 || n = 189 || n = 194 || n = 199 || n = 209 || n = 214 || n = 219 || n = 229 || n = 234)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
	ldy #200 ;hides 'iny'
	iny
	iny
	Jne *-3
.elseif(n = 172 || n = 217)
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
.elseif(n = 179)
	; 6 bytes
	ldx #216 ;hides 'cld'
	inx
	inx
	Jne *-3
.elseif(n = 182 || n = 187 || n = 202 || n = 212 || n = 222)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 185 || n = 220)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 207)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.if(n = 235 || n = 238 || n = 240 || n = 243 || n = 245 || n = 248 || n = 250 || n = 258 || n = 263 || n = 268 || n = 270 || n = 273 || n = 275 || n = 278 || n = 282 || n = 283 || n = 285 || n = 293 || n = 297 || n = 298 || n = 303 || n = 305 || n = 308 || n = 310)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 236 || n = 246 || n = 261 || n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 237 || n = 252 || n = 292)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.elseif((n .mod 5 = 4) && ((n > 243 && n < 265) || n = 289 || n = 294))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 247)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif((n .mod 7 = 1) && ((n > 252 && n < 275) || (n > 287 && n < 310)))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 255)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 257)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
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
	ldy #186 ;hides 'tsx'
	dey
	dey
	Jmi *-3
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
.elseif(n = 284)
	; 4 bytes
	ldy #200 ;hides 'iny'
	Jmi *-1
.elseif(n = 287)
	; 6 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 290)
	; 6 bytes
	ldy #200 ;hides 'iny'
	Jne *-1
	sta (@ptrtemp),y
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
.if(n = 313 || n = 315 || n = 318 || n = 322 || n = 328 || n = 333 || n = 335 || n = 338 || n = 340 || n = 343 || n = 345 || n = 348 || n = 350 || n = 353 || n = 355 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 380 || n = 383 || n = 385 || n = 388 || n = 390 || n = 402 || n = 403 || n = 408 || n = 410 || n = 413 || n = 415 || n = 418 || n = 420 || n = 423 || n = 425 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 448 || n = 450 || n = 453 || n = 455 || n = 458 || n = 460 || n = 468)
	; 6..7 bytes
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
.elseif(n = 317 || n = 347 || n = 357 || n = 387 || n = 392 || n = 432 || n = 447 || n = 462)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 349 || n = 354 || n = 364 || n = 369 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 409 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459 || n = 464)
	; 6..7 bytes
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
.elseif(n = 327 || n = 342 || n = 362 || n = 422 || n = 427 || n = 452)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 359)
	; 6 bytes
	ldx #216 ;hides 'cld'
	nop
	inx
	Jne *-3
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
	ldx #168 ;hides 'tay'
	inx
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
	; 7 bytes
	php
	delay_n (n-7)
	plp
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
.if((n >= 469) && (n < 547))
; 469 to 546
.if((n .mod 5 = 4) && ((n > 468 && n < 480) || (n > 498 && n < 515) || n = 529 || n = 534 || n = 544))
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
.elseif(n = 472 || n = 473 || n = 475 || n = 478 || n = 480 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 528 || n = 535 || n = 538 || n = 542 || n = 543 || n = 545)
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
.elseif(n = 482 || n = 502 || n = 522)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
	bit $24
	dex
	Jne *-3
.elseif(n = 539)
	; 6 bytes
	ldx #136 ;hides 'dey'
	inx
	inx
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
.elseif(n = 548 || n = 553 || n = 555 || n = 558 || n = 560 || n = 563 || n = 565 || n = 570 || n = 573 || n = 578 || n = 580 || n = 583 || n = 585 || n = 588 || n = 590 || n = 593 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 618 || n = 620 || n = 623 || n = 625)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && (n = 549 || (n > 558 && n < 585) || (n > 593 && n < 620)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 552 || n = 562 || n = 567 || n = 587 || n = 602)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 556 || n = 561 || n = 571 || n = 576 || n = 581 || n = 596 || n = 606 || n = 611 || n = 616)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 557 || n = 572 || n = 592 || n = 597)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 607)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jmp *+3
	Jmi *-4
.elseif(n = 615)
	; 5 bytes
	ldx #168 ;hides 'tay'
	inx
	Jne *-2
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
	sta (@ptrtemp,x)
.elseif(n = 628 || n = 630 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 668 || n = 670 || n = 675 || n = 678 || n = 682 || n = 683 || n = 688 || n = 693 || n = 695 || n = 698 || n = 700 || n = 703)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	nop
	iny
	Jne *-3
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif((n .mod 5 = 4) && ((n > 633 && n < 655) || n = 664 || n = 669 || n = 679 || n = 689 || n = 699))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 690)
	; 6 bytes
	delay_n (n-6)
	sta (@ptrtemp),y
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
.if((n .mod 5 = 4) && ((n > 703 && n < 725) || n = 739 || (n > 748 && n < 760) || (n > 768 && n < 780)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 705 || n = 713 || n = 717 || n = 718 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 752 || n = 753 || n = 758 || n = 760 || n = 763 || n = 765 || n = 768 || n = 770 || n = 773)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 705 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 707 || n = 732 || n = 742 || n = 762 || n = 767 || n = 772)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	bit $24
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
; 782 to 937
.if((n .mod 5 = 2) && (n = 782 || n = 807 || n = 837 || n = 852 || n = 867 || n = 872 || n = 882 || n = 887 || (n > 901 && n < 923)))
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 810 || n = 818 || n = 822 || n = 823 || n = 825 || n = 828 || n = 830 || n = 833 || n = 835 || n = 838 || n = 840 || n = 843 || n = 845 || n = 853 || n = 857 || n = 858 || n = 860 || n = 863 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 903 || n = 905 || n = 908 || n = 910 || n = 913 || n = 915 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 795) || (n > 803 && n < 830) || (n > 843 && n < 865) || (n > 873 && n < 895) || (n > 908 && n < 935)))
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
.elseif(n = 812)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	iny
	Jne *-4
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
.elseif(n = 839)
	; 5 bytes
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
.elseif(n = 842)
	; 7 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 865 || n = 892)
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
.elseif(n = 935)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 940 || n = 943 || n = 945 || n = 948 || n = 950 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 980 || n = 983 || n = 985 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1063 || n = 1067 || n = 1068 || n = 1073 || n = 1075 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1088 || n = 1090)
	; 6..7 bytes
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
.elseif(n = 942 || n = 947 || n = 952 || n = 957 || n = 987 || n = 992 || n = 1012 || n = 1020 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1082 || n = 1087 || n = 1092)
	; 6..7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 944)
	; 6 bytes
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
.elseif(n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1089)
	; 6..7 bytes
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
	bit $24
	dex
	Jpl *-3
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
.elseif(n = 1070)
	; 6 bytes
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && ((n > 1093 && n < 1110) || (n > 1118 && n < 1145) || (n > 1153 && n < 1180) || (n > 1193 && n < 1215) || (n > 1223 && n < 1250)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1095 || n = 1130 || n = 1165 || n = 1200 || n = 1235)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 1) && ((n > 1095 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1097 || n = 1217)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1098 || n = 1102 || n = 1103 || n = 1105 || n = 1108 || n = 1110 || n = 1113 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1133 || n = 1137 || n = 1138 || n = 1140 || n = 1143 || n = 1145 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1158 || n = 1160 || n = 1168 || n = 1172 || n = 1173 || n = 1178 || n = 1180 || n = 1183 || n = 1185 || n = 1188 || n = 1190 || n = 1193 || n = 1195 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1225 || n = 1228 || n = 1230 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248 || n = 1250)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1152 || n = 1157 || n = 1167 || n = 1197 || n = 1202 || n = 1222 || n = 1227)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	ldx #216 ;hides 'cld'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1189)
	; 5 bytes
	ldy #170 ;hides 'tax'
	dey
	Jne *-2
.elseif(n = 1192)
	; 7 bytes
	delay_n (n-3)
	ldy $A4
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
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1263 || n = 1265 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1291 || n = 1298 || n = 1300 || n = 1305 || n = 1311 || n = 1314 || n = 1316 || n = 1319 || n = 1322 || n = 1326)
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
	bit $24
	dex
	Jne *-3
.elseif(n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290 || n = 1292 || n = 1299 || n = 1327)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1293 || n = 1302 || n = 1318)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
.elseif(n = 1304)
	; 6 bytes
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1307)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp),y
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
.elseif(n = 1323)
	; 7 bytes
	bit <$2C ;hides 'bit $6DA2'
	ldx #109
	inx
	Jne *-4
.elseif(n = 1325)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1328)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	nop
	dey
	Jne *-3
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
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
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
	ldx #121
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
.elseif(n = 1372 || n = 1400)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 1379 || n = 1390 || n = 1397 || n = 1402)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
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
.if(n = 1407 || n = 1410 || n = 1417 || n = 1424 || n = 1431 || n = 1434 || n = 1435 || n = 1438 || n = 1445)
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
	; 5 bytes
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1416)
	; 7 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 1419)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp),y
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
.elseif(n = 1426 || n = 1428)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1439)
	; 7 bytes
	ldx #<$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
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
.endif
.else
; 1446 to 1484
.if(n = 1446 || n = 1460 || n = 1474)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1447 || n = 1452 || n = 1454 || n = 1466 || n = 1470 || n = 1473 || n = 1479 || n = 1480 || n = 1482 || n = 1483)
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
.elseif(n = 1456 || n = 1463)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1458)
	; 7 bytes
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	dey
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
.elseif(n = 1467)
	; 7 bytes
	bit <$2C ;hides 'bit $A3A2'
	ldx #163
	dex
	Jne *-4
.elseif(n = 1469)
	; 7 bytes
	ldy #232 ;hides 'inx'
	delay_n 7
	dey
	Jmi *-4
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
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if((n .mod 7 = 1) && ((n > 1484 && n < 1521)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1486 || n = 1495 || n = 1504 || n = 1522)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1487 || n = 1490 || n = 1491 || n = 1494 || n = 1496 || n = 1501 || n = 1508 || n = 1510 || n = 1515)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488 || n = 1509 || n = 1514 || n = 1516 || n = 1523)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1489)
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
.elseif(n = 1498 || n = 1517)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1500)
	; 8 bytes
	tax
	ldy #69 ;hides 'eor zp'
	nop
	iny
	Jne *-3
	txa
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
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1525 || n = 1537 || n = 1561)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1526)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1528)
	; 8 bytes
	ldx #<$2C ;hides 'bit $75A2'
	ldx #117
	nop
	inx
	Jne *-5
.elseif(n = 1529)
	; 6 bytes
	ldy #170 ;hides 'tax'
	nop
	dey
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
	; 8 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 1533 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1552 || n = 1557 || n = 1559 || n = 1560)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1535)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp),y
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
.elseif(n = 1544 || n = 1551)
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
	ldy #136 ;hides 'dey'
	delay_n 4
	iny
	iny
	Jne *-5
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
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1596)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1564 || n = 1571 || n = 1578 || n = 1580 || n = 1588 || n = 1592 || n = 1595 || n = 1599 || n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1615 || n = 1620 || n = 1622 || n = 1627 || n = 1634 || n = 1636)
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
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1584 && n < 1622)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1568 || n = 1582 || n = 1616 || n = 1624 || n = 1629)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1640)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1570 || n = 1572 || n = 1579 || n = 1586 || n = 1600 || n = 1607 || n = 1614 || n = 1626 || n = 1635 || n = 1640)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1573 || n = 1609 || n = 1633)
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
.elseif(n = 1577 || n = 1593 || n = 1601 || n = 1617)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1581 || n = 1631)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
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
.elseif(n = 1589)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1591)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1602)
	; 7 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	dex
	Jne *-4
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
.elseif(n = 1628)
	; 6 bytes
	ldx #180 ;hides 'ldy zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1630)
	; 7 bytes
	php
	delay_n (n-7)
	plp
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
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641 || n = 1652 || n = 1662 || n = 1668 || n = 1669 || n = 1671 || n = 1672 || n = 1676 || n = 1678)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1642 || n = 1658 || n = 1663 || n = 1677)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1643)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
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
.elseif((n .mod 9 = 1) && ((n > 1647 && n < 1676)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1649 || n = 1665)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
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
.elseif(n = 1659)
	; 7 bytes
	ldy #166 ;hides 'ldx zp'
	ldx $EA ;hides 'nop'
	dey
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
.endif
.else
; 1680 to 1718
.if(n = 1680 || n = 1696)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1697 || n = 1703 || n = 1704 || n = 1706 || n = 1707 || n = 1714 || n = 1715 || n = 1718)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1693 || n = 1711)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1687 || n = 1694 || n = 1708)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1689 || n = 1713)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
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
.elseif(n = 1698 || n = 1712)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1699)
	; 7 bytes
	ldy #170 ;hides 'tax'
	ldx $A6
	dey
	Jne *-4
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
.elseif(n = 1705 || n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1710)
	; 7 bytes
	bit <$2C ;hides 'bit $BEA2'
	ldx #190
	dex
	Jne *-4
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
.elseif(n = 1722 || n = 1725 || n = 1731 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1748 || n = 1749 || n = 1753 || n = 1754)
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
.elseif(n = 1757)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
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
.elseif(n = 1762 || n = 1767 || n = 1770 || n = 1781 || n = 1784 || n = 1787 || n = 1788 || n = 1795)
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
	sta (@ptrtemp,x)
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
.elseif(n = 1780)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 8
	Jne *-5
.elseif(n = 1782)
	; 7 bytes
	bit <$2C ;hides 'bit $C6A2'
	ldx #198
	dex
	Jne *-4
.elseif(n = 1790)
	; 6 bytes
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
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
.elseif(n = 1796)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797 || n = 1798 || n = 1803 || n = 1812 || n = 1815 || n = 1821 || n = 1830)
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
	ldx #164 ;hides 'ldy zp'
	nop
	ldy $A4
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
.elseif(n = 1809 || n = 1833)
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
.elseif(n = 1817)
	; 6 bytes
	ldy #202 ;hides 'dex'
	nop
	dey
	Jne *-3
.elseif(n = 1818)
	; 7 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	dex
	Jne *-4
.elseif(n = 1820)
	; 8 bytes
	delay_n (n-3)
	ldy $A4
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
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 1843 || n = 1848 || n = 1850 || n = 1851 || n = 1856 || n = 1866 || n = 1867 || n = 1874 || n = 1875)
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
.elseif(n = 1859)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	ldx $A6
	dey
	Jne *-4
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
	; 7 bytes
	ldy #170 ;hides 'tax'
	delay_n 4
	dey
	Jne *-4
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
.elseif(n = 1878 || n = 1888 || n = 1896 || n = 1912)
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
	; 7 bytes
	ldx #136 ;hides 'dey'
	delay_n 7
	dex
	Jne *-4
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
	ldy #136 ;hides 'dey'
	delay_n 9
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
.elseif(n = 1920 || n = 1928 || n = 1938 || n = 1939 || n = 1947)
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 1949)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
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
.elseif(n = 1955 || n = 1956 || n = 1964 || n = 1965 || n = 1974 || n = 1982 || n = 1983 || n = 1987)
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
.elseif(n = 1960 || n = 1984)
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
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	ldy $EAEA ;hides two 'nop's
	dex
	Jne *-5
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
.elseif(n = 1973)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
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
.elseif(n = 1995 || n = 2002 || n = 2010 || n = 2018 || n = 2027 || n = 2028)
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
.elseif(n = 2000)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
.elseif(n = 2001 || n = 2009 || n = 2025)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
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
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	nop
	dey
	Jne *-4
.elseif(n = 2013)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
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
.elseif(n = 2022)
	; 9 bytes
	delay_n (n-3)
	ldy $A4
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
	delay_n (n-7)
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
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032 || n = 2040)
	; 9 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 2035 || n = 2044)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2036 || n = 2047)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2037 || n = 2046)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2038)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2039)
	; 8 bytes
	ldy #170 ;hides 'tax'
	nop
	ldx $A6
	dey
	Jne *-5
.elseif(n = 2041)
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
.elseif(n = 2045)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 4
	dey
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
.elseif(n = 2050)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.endif
.else
; 2051 to 2070
.if(n = 2051)
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
.elseif(n = 2053 || n = 2062)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2054)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-5
.elseif(n = 2055 || n = 2063 || n = 2064 || n = 2066 || n = 2067)
	; 8..9 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2058)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2059)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
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
.elseif(n = 2065)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 2076 || n = 2085 || n = 2094)
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
.elseif(n = 2084 || n = 2093)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 2095)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	dey
	dey
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
.elseif(n = 2153 || n = 2154 || n = 2162 || n = 2163 || n = 2165 || n = 2172 || n = 2174 || n = 2175 || n = 2180 || n = 2181)
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
.elseif(n = 2158)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $A4A4 ;hides 'ldy zp'
	dex
	Jne *-5
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
.elseif(n = 2164)
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
.elseif(n = 2182)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $A6A6 ;hides 'ldx zp'
	dey
	Jne *-5
.elseif(n = 2183)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	nop
	ldx $A6
	dey
	Jne *-5
.elseif(n = 2184)
	; 9 bytes
	tax
	ldy #37 ;hides 'and zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	txa
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
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
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
.elseif(n = 2228 || n = 2238 || n = 2239 || n = 2248)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2229 || n = 2235 || n = 2243 || n = 2244 || n = 2246 || n = 2247 || n = 2252 || n = 2253 || n = 2262)
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
	; 8 bytes
	ldx #160 ;hides 'ldy #imm'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2237)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 2264)
	; 8 bytes
	ldy #162 ;hides 'ldx #imm'
	ldx $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
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
.elseif(n = 2306 || n = 2307 || n = 2327 || n = 2331 || n = 2334 || n = 2335 || n = 2340 || n = 2343)
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 2323)
	; 8 bytes
	tsx
	ldy #24 ;hides 'clc'
	pha
	iny
	Jne *-3
	txs
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
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $EAEA ;hides two 'nop's
	dex
	Jne *-5
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
	bit $24
	dex
	Jne *-4
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
.elseif(n = 2345 || n = 2350 || n = 2357 || n = 2368 || n = 2373)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2346 || n = 2348 || n = 2358 || n = 2360 || n = 2362 || n = 2372 || n = 2374 || n = 2382)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2347)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2349)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2351)
	; 7 bytes
	ldx #168 ;hides 'tay'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 2352)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	inx ;first loop only
	delay_n 4
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
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2359 || n = 2370)
	; 9 bytes
	tsx
	ldy #(n-5)/11
	pha
	pha
	dey
	Jne *-3
	txs
.elseif(n = 2361 || n = 2371)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2363)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EAEA ;hides two 'nop's
	dey
	Jne *-5
.elseif(n = 2364)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	nop
	dey
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
.elseif(n = 2380)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2390 || n = 2391 || n = 2394 || n = 2403 || n = 2405 || n = 2412 || n = 2415)
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
	ldy #200 ;hides 'iny'
	bit $24
	dey
	dey
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
.elseif(n = 2407)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 2417)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	delay_n 6
	dey
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
.if(n = 2422 || n = 2427 || n = 2429 || n = 2434 || n = 2439)
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
	tsx
	ldy #36 ;hides 'bit zp'
	nop
	pha
	iny
	Jne *-4
	txs
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
	tsx
	ldy #(n-5)/11
	pha
	pha
	dey
	Jne *-3
	txs
.elseif(n = 2442 || n = 2444 || n = 2450 || n = 2456)
	; 8..10 bytes
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
.elseif(n = 2455)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2457)
	; 9 bytes
	bit <$2C ;hides 'bit $BDA2'
	ldx #189
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2458)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2459)
	; 9 bytes
	ldx #<$2C ;hides 'bit $CDA2'
	ldx #205
	bit $24
	dex
	Jne *-6
.elseif(n = 2460)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	delay_n 9
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
.elseif(n = 2462 || n = 2463 || n = 2467 || n = 2475 || n = 2477 || n = 2478 || n = 2489 || n = 2495 || n = 2499)
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2466 || n = 2500)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2468 || n = 2480 || n = 2492 || n = 2494)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2469)
	; 9 bytes
	tsx
	ldy #(n-5)/11
	pha
	pha
	dey
	Jne *-3
	txs
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
.elseif(n = 2483)
	; 8 bytes
	tsx
	ldy #248 ;hides 'sed'
	pha
	dey
	Jne *-3
	txs
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
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	delay_n 9
	dey
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
.elseif(n = 2502)
	; 9 bytes
	tsx
	ldy #(n-5)/11
	pha
	pha
	dey
	Jne *-3
	txs
.elseif(n = 2503 || n = 2511 || n = 2513 || n = 2522 || n = 2523 || n = 2525)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2516 || n = 2528 || n = 2538)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2505)
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
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
.elseif(n = 2514)
	; 9 bytes
	tax
	ldy #5 ;hides 'ora zp'
	delay_n 4
	iny
	Jne *-4
	txa
.elseif(n = 2515)
	; 9 bytes
	tsx
	ldy #251
	nop
	pha
	dey
	Jne *-3
	txs
.elseif(n = 2517 || n = 2534)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 2539)
	; 9 bytes
	ldx #46
	delay_n 49
	dex
	Jpl *-5
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
.elseif(n = 2544 || n = 2547 || n = 2555 || n = 2559 || n = 2565 || n = 2571)
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
	; 9 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	ldx $A6
	dey
	Jne *-6
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
.elseif(n = 2558 || n = 2570 || n = 2576)
	; 9..10 bytes
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
.if(n = 2579 || n = 2583 || n = 2595 || n = 2598 || n = 2607 || n = 2609 || n = 2610)
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
.elseif(n = 2587)
	; 9 bytes
	tax
	ldy #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	txa
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
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	delay_n 7
	dey
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
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
.elseif(n = 2603)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 7
	dey
	Jne *-4
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
.elseif(n = 2621 || n = 2631 || n = 2632 || n = 2635 || n = 2643 || n = 2645 || n = 2649 || n = 2654 || n = 2655)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2623 || n = 2642)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2624 || n = 2636 || n = 2637 || n = 2646 || n = 2648)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2625)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 6
	dey
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
.elseif(n = 2634)
	; 9 bytes
	tsx
	ldy #(n-5)/11
	pha
	pha
	dey
	Jne *-3
	txs
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
.elseif(n = 2659)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2660 || n = 2682)
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
.elseif(n = 2670)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
	tsx
	ldy #(n-5)/11
	pha
	pha
	dey
	Jne *-3
	txs
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
.elseif(n = 2687)
	; 8 bytes
	ldx #168 ;hides 'tay'
	delay_n 9
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
.elseif(n = 2697 || n = 2698 || n = 2702 || n = 2709 || n = 2715 || n = 2733)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2699)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2700 || n = 2711 || n = 2722)
	; 9 bytes
	tsx
	ldy #(n-5)/11
	pha
	pha
	dey
	Jne *-3
	txs
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 2708 || n = 2720 || n = 2732 || n = 2734)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2710)
	; 9 bytes
	ldy #21
	delay_n 124
	dey
	Jne *-5
.elseif(n = 2712)
	; 9 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	bit $24
	inx
	Jne *-6
.elseif(n = 2714)
	; 9 bytes
	ldx #180 ;hides 'ldy zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-6
.elseif(n = 2716)
	; 8 bytes
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	nop
	inx
	Jne *-5
.elseif(n = 2719)
	; 8 bytes
	ldy #170 ;hides 'tax'
	delay_n 9
	dey
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
.elseif(n = 2743 || n = 2767)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2744 || n = 2756 || n = 2758 || n = 2768)
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 2766)
	; 9 bytes
	tsx
	ldy #(n-5)/11
	pha
	pha
	dey
	Jne *-3
	txs
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
.if(n = 2774 || n = 2775 || n = 2777 || n = 2786 || n = 2803 || n = 2808 || n = 2811)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2776 || n = 2791)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2778 || n = 2780 || n = 2792)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2779)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 2810)
	; 9 bytes
	tsx
	ldy #(n-5)/11
	pha
	pha
	dey
	Jne *-3
	txs
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2818 || n = 2819 || n = 2823 || n = 2825 || n = 2826 || n = 2831 || n = 2835 || n = 2837 || n = 2838 || n = 2842 || n = 2847 || n = 2849)
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
.elseif(n = 2827)
	; 7 bytes
	ldy #202 ;hides 'dex'
	delay_n 7
	dey
	Jne *-4
.elseif(n = 2828 || n = 2834 || n = 2840 || n = 2848 || n = 2850)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2830)
	; 9 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 2832)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2836 || n = 2851)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2839)
	; 9 bytes
	ldy #21
	delay_n 124
	dey
	Jpl *-5
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
.elseif(n = 2853 || n = 2854 || n = 2858 || n = 2859 || n = 2862 || n = 2883 || n = 2890)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2855)
	; 9 bytes
	ldx #168 ;hides 'tay'
	rol $26
	ror $26
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
.elseif(n = 2860 || n = 2872 || n = 2884)
	; 9 bytes
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
	ldx #180 ;hides 'ldy zp,x'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2875)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2877)
	; 10 bytes
	tsx
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	pha
	iny
	Jne *-5
	txs
.elseif(n = 2879)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 2880)
	; 9 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	bit $24
	dex
	Jne *-6
.elseif(n = 2882)
	; 10 bytes
	delay_n (n-3)
	ldy $A4
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 2894 || n = 2895 || n = 2897 || n = 2901 || n = 2902 || n = 2907 || n = 2909 || n = 2910 || n = 2914 || n = 2915 || n = 2919 || n = 2921 || n = 2922)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2896 || n = 2926)
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
.elseif(n = 2900 || n = 2912 || n = 2924)
	; 9 bytes
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
.elseif(n = 2906)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
.elseif(n = 2911)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	delay_n 9
	dey
	Jne *-5
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
.elseif(n = 2923)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 2931 || n = 2933 || n = 2943 || n = 2945 || n = 2954 || n = 2958 || n = 2966 || n = 2967)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2934 || n = 2936 || n = 2948 || n = 2960 || n = 2962)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2935 || n = 2947)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2946)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2949)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-7
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
.elseif(n = 2970 || n = 2973 || n = 2974 || n = 2979 || n = 2981 || n = 2985 || n = 2991 || n = 2994 || n = 2998 || n = 3003 || n = 3005 || n = 3006)
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
.elseif(n = 2995 || n = 3007)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 3019)
	; 9 bytes
	tsx
	ldy #24 ;hides 'clc'
	pha
	pha
	iny
	Jne *-4
	txs
.elseif(n = 3021)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.elseif(n = 3027 || n = 3029 || n = 3033 || n = 3034 || n = 3038)
	; 8..10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3045)
	; 9 bytes
	tsx
	ldy #234 ;hides 'nop'
	pha
	pha
	dey
	Jne *-4
	txs
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
.elseif(n = 3050 || n = 3051 || n = 3055 || n = 3062 || n = 3063 || n = 3065 || n = 3070 || n = 3075 || n = 3077 || (n > 3082 && n < 3086))
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
.elseif(n = 3058)
	; 9 bytes
	ldy #3
	delay_n 1014
	dey
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.endif
.else
; 3086 to 3125
.if(n = 3086)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #66
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 3087)
	; 9 bytes
	ldy #248 ;hides 'sed'
	delay_n 379
	iny
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
.elseif(n = 3092)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
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
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3115)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3118)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 3124)
	; 10 bytes
	ldx #(n-1042)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 3128 || n = 3142)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3129 || n = 3146 || n = 3163)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3130 || n = 3144)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3131 || n = 3138 || n = 3139 || n = 3141 || n = 3148 || n = 3149 || n = 3157 || n = 3162)
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
	ldx #57
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 3134 || n = 3147 || n = 3160)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3135)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 49
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
.elseif(n = 3143)
	; 9 bytes
	ldy #24 ;hides 'clc'
	delay_n 124
	dey
	Jne *-6
.elseif(n = 3145)
	; 9 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3150)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 18
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
.elseif(n = 3156)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.elseif(n = 3161)
	; 9 bytes
	ldy #186 ;hides 'tsx'
	rol $26
	ror $26
	dey
	Jne *-6
.elseif(n = 3164)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 626
	dey
	Jmi *-7
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
.elseif(n = 3167 || n = 3175 || n = 3176 || n = 3182 || n = 3188 || n = 3189 || n = 3195 || n = 3203)
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
.elseif(n = 3170)
	; 10 bytes
	tsx
	ldy #(n-5)/15
	pha
	delay_n 7
	dey
	Jne *-4
	txs
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
	tsx
	ldy #244
	nop
	pha
	pha
	dey
	Jne *-4
	txs
.elseif(n = 3178)
	; 10 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3180 || n = 3197)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
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
	; 9 bytes
	ldx #<$2C ;hides 'bit $F5A2'
	ldx #245
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3187)
	; 9 bytes
	ldx #58
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 3190)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3191)
	; 9 bytes
	ldx #168 ;hides 'tay'
	delay_n 12
	dex
	Jne *-6
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
	ldx #(n-32)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.elseif(n = 3208)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3209 || n = 3213 || n = 3218 || n = 3219 || n = 3237 || n = 3240)
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
.elseif(n = 3214)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 3222)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 284
	inx
	inx
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
.elseif(n = 3227)
	; 9 bytes
	tsx
	ldy #248 ;hides 'sed'
	pha
	pha
	dey
	Jne *-4
	txs
.elseif(n = 3229)
	; 9 bytes
	ldy #170 ;hides 'tax'
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3231)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 9
	dey
	Jne *-5
.elseif(n = 3232)
	; 9 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3234)
	; 9 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
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
.if((n >= 3243) && (n < 3262))
; 3243 to 3261
.if(n = 3243 || n = 3258)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3244)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3245)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3246)
	; 10 bytes
	tax
	ldy #53 ;hides 'and zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	iny
	Jne *-5
	txa
.elseif(n = 3247)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3248)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3249)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3250)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3251)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3252)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3253)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3254)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3255)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #27
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 3256)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3257)
	; 9 bytes
	ldx #180 ;hides 'ldy zp,x'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3259)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3260)
	; 10 bytes
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3261)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 3262 to 3281
.if(n = 3262)
	; 9 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3263 || n = 3277)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3264)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3265)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3266 || n = 3267 || n = 3272 || n = 3273 || n = 3279)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3268)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3269)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3270)
	; 9 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
.elseif(n = 3271)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3274)
	; 9 bytes
	ldy #24 ;hides 'clc'
	delay_n 124
	dey
	Jpl *-6
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
.elseif(n = 3278)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3284 || n = 3292 || n = 3298)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3283 || n = 3287 || n = 3293 || n = 3304 || n = 3317 || n = 3318)
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
.elseif(n = 3286 || n = 3301 || n = 3316)
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
.elseif(n = 3299)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
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
.elseif(n = 3306)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 3320)
	; 10 bytes
	ldy #166 ;hides 'ldx zp'
	nop
	delay_n 12
	dey
	Jne *-7
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3323 || n = 3330 || n = 3335 || n = 3337 || n = 3348 || n = 3349 || n = 3351 || n = 3352 || n = 3357)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3322 || n = 3340 || n = 3354)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 3325)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 3334)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3336)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3338)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3339)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 661
	dey
	Jmi *-7
.elseif(n = 3341 || n = 3342 || n = 3358)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3353)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 71
	dey
	Jmi *-7
.elseif(n = 3355)
	; 9 bytes
	ldy #26
	delay_n 124
	dey
	Jne *-5
.elseif(n = 3356)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
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
.elseif(n = 3362 || n = 3363 || n = 3365 || n = 3366 || n = 3369 || n = 3378 || n = 3386 || n = 3395)
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3368 || n = 3382 || n = 3396)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 3369 && n < 3373))
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
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
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
	ldx #(n-32)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3381)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
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
	ldx #248 ;hides 'sed'
	ldy #4
	dey
	Jne *-1
	dex
	Jmi *-7
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
.elseif(n = 3394)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3397)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
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
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 3443 || n = 3447 || n = 3449 || n = 3453 || n = 3454 || n = 3461 || n = 3463 || n = 3468 || n = 3475)
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
.elseif(n = 3446)
	; 9 bytes
	ldx #4
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 3448 || n = 3458 || n = 3474 || n = 3476)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 3460 || n = 3467)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3464)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
.elseif(n = 3465)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3470)
	; 10 bytes
	tsx
	ldy #(n-5)/15
	pha
	delay_n 7
	dey
	Jne *-4
	txs
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
.if(n = 3477)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3478 || n = 3480 || n = 3494 || n = 3508)
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
.elseif(n = 3482 || n = 3502 || n = 3512)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3483 || n = 3498 || n = 3499 || n = 3507 || n = 3513)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3484)
	; 9 bytes
	ldy #27
	delay_n 124
	dey
	Jne *-5
.elseif(n = 3485)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	inx
	Jne *-7
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
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 12
	dex
	Jne *-6
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
.elseif(n = 3506)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3509)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3510)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
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
.elseif(n = 3517 || n = 3523 || n = 3528 || n = 3531 || n = 3539 || n = 3545 || n = 3547)
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3521 || n = 3553)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3522 || n = 3550)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3524)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3525)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3526 || n = 3541)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3527)
	; 9 bytes
	ldx #168 ;hides 'tay'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 3529 || n = 3543)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3530)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #4
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 3533)
	; 9 bytes
	ldy #186 ;hides 'tsx'
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3534 || n = 3548)
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
.elseif(n = 3538)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 3544)
	; 10 bytes
	ldx #(n-1182)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3549)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 3560 || n = 3574)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3562 || n = 3572 || n = 3576 || n = 3580 || n = 3593)
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
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 3564 || n = 3578)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3565)
	; 9 bytes
	ldx #65
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 3566)
	; 10 bytes
	ldx #(n-32)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jpl *-6
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
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
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
	; 9 bytes
	ldy #200 ;hides 'iny'
	delay_n 9
	dey
	dey
	Jne *-6
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3591)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
.if((n >= 3594) && (n < 3613))
; 3594 to 3612
.if(n = 3594 || n = 3602 || n = 3603 || n = 3607)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3596 || n = 3612)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3597)
	; 10 bytes
	ldx #(n-32)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3598)
	; 10 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	delay_n 9
	dex
	Jne *-7
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
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3604)
	; 10 bytes
	ldx #(n-1202)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3605)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3606)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3608)
	; 10 bytes
	ldx #164 ;hides 'ldy zp'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 3609)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3610)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3611)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.endif
.else
; 3613 to 3632
.if(n = 3613)
	; 9 bytes
	ldy #27
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 3614)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 716
	dey
	Jmi *-7
.elseif(n = 3615)
	; 9 bytes
	ldx #<$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3616 || n = 3631)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3617)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3618 || n = 3624)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3619)
	; 9 bytes
	ldx #67
	delay_n 49
	dex
	Jne *-5
.elseif(n = 3620)
	; 10 bytes
	tsx
	ldy #(n-5)/15
	pha
	delay_n 7
	dey
	Jne *-4
	txs
.elseif(n = 3621)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3622)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3623)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #88
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 3625)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3626)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #79
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 3627)
	; 10 bytes
	ldx #48
	delay_n 69
	dex
	Jpl *-6
.elseif(n = 3628)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3629)
	; 10 bytes
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 6
	inx
	Jne *-7
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
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3635)
	; 9 bytes
	ldy #202 ;hides 'dex'
	tax
	pla
	pha
	txa
	dey
	Jne *-6
.elseif(n = 3636)
	; 10 bytes
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3637)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3638)
	; 10 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
.elseif(n = 3639)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3640 || n = 3643)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3641 || n = 3651)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3642)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3644)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	dex
	Jmi *-7
.elseif(n = 3645)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3646)
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
.elseif(n = 3650)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 3652 to 3671
.if(n = 3652)
	; 10 bytes
	ldy #166 ;hides 'ldx zp'
	delay_n 16
	dey
	Jne *-7
.elseif(n = 3653)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
.elseif(n = 3654)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3655)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3656)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3657)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 55
	dey
	Jmi *-7
.elseif(n = 3658 || n = 3666 || n = 3667 || n = 3670)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3659)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3660)
	; 10 bytes
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3661)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3662)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 3668)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3669)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3671)
	; 9 bytes
	ldx #216 ;hides 'cld'
	rol $26
	ror $26
	dex
	Jne *-6
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
.elseif(n = 3673 || n = 3690 || n = 3707)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3674)
	; 10 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 14
	dey
	Jne *-7
.elseif(n = 3675 || n = 3678 || n = 3682 || n = 3683 || n = 3689 || n = 3692 || n = 3693 || n = 3699 || n = 3708 || n = 3709)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3676 || n = 3691 || n = 3706)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3677 || n = 3684 || n = 3686 || n = 3688)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 3685 || n = 3705)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3687)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3694)
	; 10 bytes
	ldx #(n-1232)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 3698)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3700)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3701)
	; 10 bytes
	ldx #50
	delay_n 69
	dex
	Jne *-6
.elseif(n = 3702)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3703)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #91
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 3704)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3710)
	; 10 bytes
	tsx
	ldy #(n-5)/15
	pha
	delay_n 7
	dey
	Jne *-4
	txs
.endif
.else
.if((n >= 3711) && (n < 3731))
; 3711 to 3730
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
.elseif(n = 3713 || n = 3729)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3714)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3715 || n = 3726 || n = 3730)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3716)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 56
	dey
	Jmi *-7
.elseif(n = 3717)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3718)
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
.elseif(n = 3721)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3722)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3723)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3724)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3725)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3727)
	; 9 bytes
	ldx #68
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 3728)
	; 9 bytes
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 7
	inx
	Jne *-6
.endif
.else
; 3731 to 3750
.if(n = 3731 || n = 3737 || n = 3738 || n = 3746 || n = 3747)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3732)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3733)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
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
.elseif(n = 3736)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
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
.elseif(n = 3741)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3742)
	; 9 bytes
	ldy #28
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 3743)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3744)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3745)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3748)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3749)
	; 10 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3750)
	; 10 bytes
	php
	delay_n (n-7)
	plp
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
.if(n = 3751 || n = 3766)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3752)
	; 10 bytes
	ldx #(n-32)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3753 || n = 3762 || n = 3765 || n = 3768)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3754)
	; 10 bytes
	ldx #(n-1252)/1251
	ldy #249
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3755)
	; 10 bytes
	tsx
	ldy #(n-5)/15
	pha
	delay_n 7
	dey
	Jne *-4
	txs
.elseif(n = 3756)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3757)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3758)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 3761)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3763)
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
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 3770 to 3789
.if(n = 3770)
	; 10 bytes
	tsx
	ldy #(n-5)/15
	pha
	delay_n 7
	dey
	Jne *-4
	txs
.elseif(n = 3771)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
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
.elseif(n = 3773)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3774)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 748
	dey
	Jmi *-7
.elseif(n = 3775)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 3778 || n = 3779 || n = 3783 || n = 3784)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3780)
	; 10 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3781)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3782)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3785)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3786)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 3788)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3789)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 751
	dey
	Jmi *-7
.endif
.endif
.else
.if((n >= 3790) && (n < 3809))
; 3790 to 3808
.if(n = 3790)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3791)
	; 9 bytes
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3792)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3793)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3794 || n = 3795 || n = 3798 || n = 3803)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3796)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3797)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 3799)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 12
	inx
	Jne *-6
.elseif(n = 3800)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 3801)
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
.elseif(n = 3804)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3805)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3806)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
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
.if(n = 3809)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3810)
	; 10 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3811 || n = 3826)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3812)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #16
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 3813 || n = 3822 || n = 3827 || n = 3828)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3814)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3815)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #8
	dey
	Jpl *-1
	inx
	Jpl *-7
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
.elseif(n = 3818)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3819)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3820)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3821)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 3825)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
.if((n >= 3829) && (n < 3848))
; 3829 to 3847
.if(n = 3829)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3830)
	; 10 bytes
	tsx
	ldy #(n-5)/15
	pha
	delay_n 7
	dey
	Jne *-4
	txs
.elseif(n = 3831)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3832 || n = 3836 || n = 3838 || n = 3846)
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
.elseif(n = 3835)
	; 9 bytes
	ldx #71
	delay_n 49
	dex
	Jne *-5
.elseif(n = 3837)
	; 9 bytes
	ldy #202 ;hides 'dex'
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3839)
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
.elseif(n = 3842)
	; 10 bytes
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	delay_n 12
	dey
	Jne *-7
.elseif(n = 3843)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3844)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3845)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3847)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 3848 to 3867
.if(n = 3848 || n = 3850 || n = 3854 || n = 3866 || n = 3867)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3849)
	; 10 bytes
	ldx #(n-1)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 3851)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #84
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 3852)
	; 10 bytes
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3853)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
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
.elseif(n = 3858)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3859 || n = 3862)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3860)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3861)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3863)
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 3864)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 766
	dey
	Jmi *-7
.elseif(n = 3865)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.endif
.endif
.else
; 3868 to 3906
.if(n = 3868)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #8
	dey
	Jpl *-1
	dex
	Jmi *-7
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
	ldy #29
	delay_n 124
	dey
	Jpl *-5
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
.elseif(n = 3874 || n = 3875 || n = 3879 || n = 3890 || n = 3891 || n = 3898)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3876)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3877 || n = 3894)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3878 || (n > 3881 && n < 3885) || n = 3892 || n = 3897 || n = 3899 || n = 3900 || n = 3902)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3880)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3881)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3885)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 3893)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3895)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3896)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3901)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
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
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
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
.elseif(n = 3908 || n = 3910 || n = 3912 || n = 3918 || n = 3932 || n = 3933 || n = 3944)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3909)
	; 10 bytes
	ldy #170 ;hides 'tax'
	ldx #2
	dex
	Jpl *-1
	dey
	Jne *-7
.elseif(n = 3911 || n = 3928 || n = 3945)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3913 || n = 3917 || n = 3922 || n = 3923 || n = 3930 || n = 3939)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3914)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3915 || n = 3934)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3916)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
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
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 3929)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3931)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
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
.elseif(n = 3938)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3940)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 3941)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 66
	iny
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
.if(n = 3946 || n = 3948 || n = 3950 || n = 3958 || n = 3975 || n = 3976 || n = 3980 || n = 3982)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3947 || n = 3954 || n = 3955 || n = 3964 || n = 3971 || n = 3981)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3956)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3957)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3959)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #4
	dey
	Jpl *-1
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
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3963)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3965)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 3966)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
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
.elseif(n = 3973)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
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
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985 || n = 4001 || n = 4017)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3986 || n = 3987 || n = 3993 || n = 3998 || n = 4002 || n = 4003 || n = 4018 || n = 4019)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3988 || n = 3990 || n = 4004 || n = 4007 || n = 4008 || n = 4020 || n = 4022)
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
	ldy #232 ;hides 'inx'
	delay_n 31
	dey
	Jmi *-7
.elseif(n = 3991 || n = 4010)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3992)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #4
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 3994)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 3995)
	; 10 bytes
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3996 || n = 4013)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3997)
	; 9 bytes
	ldx #73
	delay_n 49
	dex
	Jpl *-5
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
.elseif(n = 4005)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4006)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4009)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4011)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4012)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4014)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 796
	dey
	Jmi *-7
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
.elseif(n = 4021)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4023)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #98
	dey
	Jpl *-1
	inx
	Jne *-7
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4028 || (n > 4035 && n < 4039) || n = 4040 || n = 4042 || n = 4046 || n = 4052 || n = 4055 || n = 4058)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4025 || n = 4045)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4026)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4027)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4029 || n = 4048)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4030 || n = 4047)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 4033 || n = 4049)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4034 || n = 4035 || n = 4050 || n = 4053)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4039)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 801
	dey
	Jmi *-7
.elseif(n = 4041)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4043)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4044)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4051)
	; 9 bytes
	ldx #74
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4054)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4056)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4057)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4059)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 8
	Jne *-7
.elseif(n = 4060)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4061)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4062)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
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
	; 9 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4064 || n = 4081)
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
.elseif(n = 4066)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4067)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4068)
	; 10 bytes
	ldx #(n-582)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4069)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4070 || n = 4072 || n = 4073)
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
	ldx #55
	delay_n 69
	dex
	Jne *-6
.elseif(n = 4074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 4075)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4076)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #88
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 4077)
	; 9 bytes
	ldy #4
	delay_n 1014
	dey
	Jne *-5
.elseif(n = 4078)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.endif
.else
; 4082 to 4101
.if(n = 4082 || n = 4083 || n = 4088 || n = 4099 || n = 4100)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4084 || n = 4092 || n = 4094)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4085)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4086)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4087)
	; 9 bytes
	ldx #200 ;hides 'iny'
	delay_n 49
	dex
	Jmi *-6
.elseif(n = 4089)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4090)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4091)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1014
	dey
	dey
	Jmi *-7
.elseif(n = 4093)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
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
.elseif(n = 4098)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4101)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
; 4102 to 4140
.if(n = 4102 || n = 4110 || n = 4112 || n = 4113 || n = 4120 || n = 4128 || n = 4130 || n = 4133 || n = 4136)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4103)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 12
	dex
	Jne *-6
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
.elseif(n = 4106 || n = 4131)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4107 || n = 4117 || n = 4126 || n = 4134)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4108)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4109)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4111)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4114)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 816
	dey
	Jmi *-7
.elseif(n = 4115 || n = 4132)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4116)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
.elseif(n = 4118)
	; 10 bytes
	ldx #22
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 4119)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4121)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.elseif(n = 4125)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4127)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 41
	iny
	Jne *-7
.elseif(n = 4129)
	; 9 bytes
	ldy #31
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 4135)
	; 9 bytes
	ldx #6
	delay_n 684
	dex
	Jne *-5
.elseif(n = 4137)
	; 10 bytes
	ldx #(n-377)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4138)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4139)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 821
	dey
	Jmi *-7
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
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4177)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4142)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4143 || n = 4162)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4144 || n = 4146 || n = 4148 || n = 4152 || n = 4154 || n = 4163 || n = 4167 || (n > 4169 && n < 4173))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4145 || n = 4151 || n = 4164 || n = 4168)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4147)
	; 10 bytes
	ldx #(n-1)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4149 || n = 4166)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4150)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4153)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4155)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4156)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 4159)
	; 9 bytes
	ldx #76
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4160)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 4161)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 11
	inx
	Jne *-7
.elseif(n = 4165)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4169)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4173)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4174)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
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
	ldy #200 ;hides 'iny'
	delay_n 12
	dey
	dey
	Jne *-7
.elseif(n = 4179)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 9
	inx
	Jne *-7
.endif
.else
.if((n >= 4180) && (n < 4199))
; 4180 to 4198
.if(n = 4180)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4181)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4182 || n = 4184 || n = 4188 || n = 4191 || (n > 4195 && n < 4199))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4183)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4185)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4186)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4187)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4189)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 831
	dey
	Jmi *-7
.elseif(n = 4190)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4192)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4193)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4194)
	; 10 bytes
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4195)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.endif
.else
; 4199 to 4218
.if(n = 4199)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 4200 || n = 4217)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4201)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4202)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4203)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4204 || n = 4218)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4205)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4206)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4207)
	; 10 bytes
	ldx #(n-1)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4208)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4209)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4210)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.elseif(n = 4213)
	; 9 bytes
	ldx #77
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4214)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 836
	dey
	Jmi *-7
.elseif(n = 4215)
	; 9 bytes
	ldx #248 ;hides 'sed'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 4216)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
.if((n >= 4219) && (n < 4258))
; 4219 to 4257
.if(n = 4219 || n = 4238 || n = 4257)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4220 || n = 4226 || n = 4228 || n = 4232 || n = 4242 || n = 4244 || n = 4246 || n = 4250 || n = 4254)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4221 || n = 4227 || n = 4236 || n = 4240 || n = 4253)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4222 || n = 4243)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4223)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #7
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 4224)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4225)
	; 9 bytes
	ldy #10
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 4229)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4230)
	; 10 bytes
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4231 || n = 4249)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4233)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4234 || n = 4251)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4235)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4237)
	; 10 bytes
	ldx #(n-1)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4239)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4241)
	; 9 bytes
	ldy #202 ;hides 'dex'
	delay_n 14
	dey
	Jne *-6
.elseif(n = 4245)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4247)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4248)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4252)
	; 10 bytes
	ldx #108
	delay_n 34
	dex
	Jpl *-6
.elseif(n = 4255)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 69
	dex
	Jne *-7
.elseif(n = 4256)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 4258 to 4296
.if(n = 4258)
	; 9 bytes
	ldy #32
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 4259 || n = 4260 || n = 4269 || n = 4270 || n = 4278 || n = 4287)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4261)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4262)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-6
.elseif(n = 4263)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 34
	inx
	Jpl *-7
.elseif(n = 4264)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4265 || n = 4291)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4266)
	; 10 bytes
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4267)
	; 9 bytes
	ldx #78
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4268 || n = 4285)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4271)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4272 || n = 4274 || n = 4275 || n = 4280 || n = 4282 || n = 4286 || n = 4288 || n = 4290 || n = 4292 || n = 4296)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4273)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4276 || n = 4295)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4277)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	ldx #2
	dex
	Jpl *-1
	dey
	Jne *-7
.elseif(n = 4279)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4281)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 4289)
	; 10 bytes
	ldx #(n-1)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4293)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4294)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if(n = 4297 || n = 4304 || n = 4316 || n = 4323 || n = 4335)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 4297 && n < 4301) || (n > 4306 && n < 4310) || n = 4312 || n = 4315 || n = 4318 || n = 4322 || n = 4326 || n = 4328 || n = 4334)
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
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4302 || n = 4319)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4303)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4305)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4306 || n = 4327)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4310)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4311)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4313)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4314 || n = 4333)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4317)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4320)
	; 10 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4321)
	; 9 bytes
	ldx #79
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4324)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4325)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4329)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4330)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4331)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4332)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 4336 to 4375
.if(n = 4336)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4337)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4338 || n = 4354 || n = 4355 || n = 4373)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4339 || n = 4357)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4340 || n = 4342 || n = 4350 || n = 4351 || (n > 4357 && n < 4361) || n = 4362 || n = 4363 || n = 4368 || n = 4370)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4341 || n = 4372)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4343)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4344)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4345)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4346)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #95
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 4347)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4348 || n = 4369)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4349)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 51
	dey
	Jmi *-7
.elseif(n = 4352 || n = 4371)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4353)
	; 9 bytes
	ldx #0
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4356)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4361)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4364)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 866
	dey
	Jmi *-7
.elseif(n = 4365)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4366)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4367)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 4374)
	; 10 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4375)
	; 9 bytes
	ldx #81
	delay_n 49
	dex
	Jne *-5
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
.if(n = 4376 || n = 4378 || n = 4379 || n = 4382 || n = 4384 || (n > 4395 && n < 4400) || n = 4402 || n = 4404 || n = 4413)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4377 || n = 4389 || n = 4392)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4380)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4381)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4383)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4385 || n = 4405)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4386)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 34
	dex
	Jmi *-7
.elseif(n = 4387)
	; 9 bytes
	ldy #33
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 4388)
	; 10 bytes
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
.elseif(n = 4390 || n = 4409)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4391)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 4393 || n = 4411)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4394)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4395)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4400)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4401)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4403)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4406)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4407)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 12
	inx
	Jne *-6
.elseif(n = 4408)
	; 10 bytes
	ldx #113
	delay_n 34
	dex
	Jne *-6
.elseif(n = 4410)
	; 10 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4412)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4414)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 876
	dey
	Jmi *-7
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4416 || n = 4419 || n = 4422 || n = 4424 || n = 4426 || n = 4433 || n = 4435 || n = 4437 || n = 4440 || (n > 4441 && n < 4445) || n = 4448 || (n > 4449 && n < 4453))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4417)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4418)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4420)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4421)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4423)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4425)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4427)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #19
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 4428 || n = 4447)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4429)
	; 9 bytes
	ldx #81
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4430 || n = 4449)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4431)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4432 || n = 4453)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4434)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4436)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #96
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 4438)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4439)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 881
	dey
	Jmi *-7
.elseif(n = 4441)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4445)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4446)
	; 10 bytes
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	delay_n 9
	dex
	Jne *-7
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4455)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4456)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif((n > 4456 && n < 4460) || n = 4462 || n = 4469 || n = 4472 || n = 4475 || n = 4479 || n = 4480 || n = 4484 || (n > 4489 && n < 4493))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4460)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4461)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4463)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4464)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 886
	dey
	Jmi *-7
.elseif(n = 4465)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 17
	Jne *-7
.elseif(n = 4466 || n = 4485)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4467)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4468 || n = 4487)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4470)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4471)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #6
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 4473)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4474)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4476)
	; 10 bytes
	ldx #24
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 4477)
	; 10 bytes
	ldx #(n-1)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4478)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.elseif(n = 4483)
	; 9 bytes
	ldx #83
	delay_n 49
	dex
	Jne *-5
.elseif(n = 4486)
	; 10 bytes
	ldx #114
	delay_n 34
	dex
	Jpl *-6
.elseif(n = 4488)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4489)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
.if((n >= 4493) && (n < 4512))
; 4493 to 4511
.if(n = 4493 || n = 4494 || n = 4498 || n = 4502 || n = 4508)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4495)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4496)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4497)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
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
.elseif(n = 4501)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4503)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #111
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 4504)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4505)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4506)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4507)
	; 10 bytes
	ldx #(n-1)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4509)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4510)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4511)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 4512 to 4531
.if(n = 4512 || n = 4520 || n = 4528 || n = 4530)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4513)
	; 10 bytes
	ldx #(n-97)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4514)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 36
	dey
	Jmi *-7
.elseif(n = 4515)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4516)
	; 9 bytes
	ldy #35
	delay_n 124
	dey
	Jne *-5
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
.elseif(n = 4519)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4521)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4522)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4523)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4524)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-7
.elseif(n = 4525)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4526)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4527)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4529)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4531)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if(n = 4532 || n = 4534 || n = 4538 || n = 4541 || n = 4547 || n = 4548 || n = 4550 || n = 4560 || n = 4562 || n = 4566 || n = 4568 || n = 4570)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4533)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.elseif(n = 4537)
	; 9 bytes
	ldx #84
	delay_n 49
	dex
	Jne *-5
.elseif(n = 4539 || n = 4544 || n = 4563)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4540)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4542 || n = 4561)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4543)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #112
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 4545)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4546)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4549)
	; 10 bytes
	ldx #2
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 4551)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4552)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.elseif(n = 4555)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4556)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4557)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 46
	iny
	Jne *-7
.elseif(n = 4558)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4559)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4564)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4565)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4567)
	; 10 bytes
	ldx #(n-1)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4569)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
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
.elseif(n = 4573)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 4575)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #39
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 4576)
	; 10 bytes
	ldy #200 ;hides 'iny'
	delay_n 14
	dey
	dey
	Jne *-7
.elseif(n = 4577 || n = 4603)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4578 || n = 4584 || n = 4587 || n = 4588 || n = 4592 || (n > 4593 && n < 4597) || n = 4602 || n = 4604)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4579 || n = 4600)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4580 || n = 4599)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4581 || n = 4606)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4582 || n = 4601)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4583)
	; 10 bytes
	ldx #29
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif(n = 4585)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4586)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
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
.elseif(n = 4591)
	; 9 bytes
	ldx #84
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4593)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4597)
	; 10 bytes
	ldx #(n-1)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4598)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #10
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 4605)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
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
	; 9 bytes
	ldy #12
	delay_n 379
	dey
	Jne *-5
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4612 || n = 4613 || n = 4615 || n = 4617 || n = 4622 || n = 4626 || n = 4630 || n = 4636 || n = 4638 || n = 4640 || n = 4644 || n = 4646 || n = 4648)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4611)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4614)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 916
	dey
	Jmi *-7
.elseif(n = 4616)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4618 || n = 4637)
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
.elseif(n = 4620 || n = 4639)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4621 || n = 4642)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4623)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #113
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 4624)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4625)
	; 9 bytes
	ldx #16
	delay_n 284
	dex
	Jne *-5
.elseif(n = 4627)
	; 10 bytes
	ldx #(n-1)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4628)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4629)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4631)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4632)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4633)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4634)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4635)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4641)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4643)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4645)
	; 9 bytes
	ldx #86
	delay_n 49
	dex
	Jne *-5
.elseif(n = 4647)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 4649 to 4687
.if(n = 4649)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4650 || n = 4652 || n = 4653 || n = 4659 || n = 4660 || n = 4662 || n = 4667 || n = 4669 || n = 4671 || n = 4672 || n = 4674 || n = 4676 || n = 4678 || n = 4680 || n = 4683 || n = 4686)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4651 || n = 4682)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4654)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4655 || n = 4681)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4656 || n = 4675)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4657)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4658 || n = 4677)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4661)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4663 || n = 4684)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4664)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 926
	dey
	Jmi *-7
.elseif(n = 4665)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4666)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4668)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-7
.elseif(n = 4670)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4673)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4679)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4685)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4687)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if(n = 4688 || n = 4691 || n = 4692 || n = 4695 || n = 4697 || n = 4700 || n = 4702 || n = 4704 || (n > 4707 && n < 4711) || n = 4712 || n = 4718 || n = 4722 || n = 4723)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4689)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4690)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4693)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4694 || n = 4713)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4696 || n = 4701 || n = 4715)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4698)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4699)
	; 9 bytes
	ldx #87
	delay_n 49
	dex
	Jne *-5
.elseif(n = 4703)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 4705 || n = 4726)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4706)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4707)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4711)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4714)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 936
	dey
	Jmi *-7
.elseif(n = 4716)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4717)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4719)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #21
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 4720)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4721)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4724)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 56
	dey
	Jmi *-7
.elseif(n = 4725)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 4727 to 4765
.if(n = 4727 || n = 4728 || n = 4736 || n = 4738 || n = 4740 || n = 4746 || n = 4749 || n = 4750 || n = 4754 || n = 4760 || n = 4762 || n = 4763)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4729)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4730)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 4731)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4732 || n = 4751)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4733 || n = 4759)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4734 || n = 4755)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4735)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4737)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4739)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4741)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4742)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4743)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #117
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 4744)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4745 || n = 4765)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4747)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4748)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4752)
	; 10 bytes
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 124
	dey
	Jne *-7
.elseif(n = 4753)
	; 9 bytes
	ldx #87
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4756)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4757)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4758)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4761)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4764)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 946
	dey
	Jmi *-7
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
; 4766 to 4804
.if(n = 4766 || n = 4767 || n = 4773 || n = 4779 || n = 4782 || n = 4784 || n = 4786 || n = 4788 || n = 4790 || (n > 4791 && n < 4795) || n = 4799 || n = 4800 || n = 4802 || n = 4804)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4768)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4769)
	; 10 bytes
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4770 || n = 4789)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4771)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4772 || n = 4791)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4774)
	; 9 bytes
	ldy #37
	delay_n 124
	dey
	Jne *-5
.elseif(n = 4775)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4776)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4777)
	; 10 bytes
	ldx #(n-1)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4778)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4780)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4781)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4783)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #117
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 4785)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4787)
	; 10 bytes
	ldy #56 ;hides 'sec'
	delay_n 124
	iny
	iny
	Jpl *-7
.elseif(n = 4795)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4796)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #105
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 4797)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4798)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4801)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4803)
	; 10 bytes
	ldx #(n-687)/686
	ldy #136
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
.if((n >= 4805) && (n < 4824))
; 4805 to 4823
.if(n = 4805)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4806)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4807)
	; 9 bytes
	ldx #89
	delay_n 49
	dex
	Jne *-5
.elseif(n = 4808)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4809)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4810)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4811)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4812 || n = 4822)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4813)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4814)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 956
	dey
	Jmi *-7
.elseif(n = 4815)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4816)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4817)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4818)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 4819)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4820)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4821)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4823)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #119
	dey
	Jne *-1
	inx
	Jne *-7
.endif
.else
; 4824 to 4843
.if(n = 4824)
	; 9 bytes
	ldx #6
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 4825)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4826 || n = 4829)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4827)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4828 || n = 4830 || n = 4832 || n = 4836 || n = 4842 || n = 4843)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4831)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4833)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4834)
	; 10 bytes
	ldx #26
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 4835)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	inx
	Jne *-7
.elseif(n = 4837)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4838)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4839)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4840)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 16
	inx
	Jne *-7
.elseif(n = 4841)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4847 || n = 4851 || n = 4853 || n = 4855 || n = 4857 || n = 4859 || n = 4860 || n = 4866 || n = 4869 || n = 4872 || n = 4874 || n = 4875 || n = 4878 || n = 4882)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4845)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
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
.elseif(n = 4849)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4850)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 12
	dex
	dex
	Jne *-7
.elseif(n = 4852 || n = 4873)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4854)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4856)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4858)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 106
	dey
	Jmi *-7
.elseif(n = 4861)
	; 9 bytes
	ldx #89
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4862)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 14
	inx
	Jne *-7
.elseif(n = 4863)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4864)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 966
	dey
	Jmi *-7
.elseif(n = 4865)
	; 9 bytes
	ldx #0
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4868)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4870)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4871)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 4876)
	; 10 bytes
	ldx #125
	delay_n 34
	dex
	Jne *-6
.elseif(n = 4877)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4879)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4880)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4881)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
.if((n >= 4883) && (n < 4902))
; 4883 to 4901
.if(n = 4883 || n = 4887 || n = 4890 || n = 4892 || n = 4893 || n = 4898)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4884)
	; 10 bytes
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 124
	dey
	Jpl *-7
.elseif(n = 4885)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4886)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #107
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 4888)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4889)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4891)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4894)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4895)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #12
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 4896)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 76
	dey
	Jmi *-7
.elseif(n = 4897)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4899)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4900)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4901)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 4902 to 4921
.if(n = 4902 || n = 4904 || n = 4909 || n = 4910 || n = 4912 || n = 4918 || n = 4920)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4903)
	; 9 bytes
	ldy #37
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 4905)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4906)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4907)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4908)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4911)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4913)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 4914)
	; 9 bytes
	ldx #16
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 4915)
	; 9 bytes
	ldx #90
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4916 || n = 4917)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4919)
	; 10 bytes
	ldx #136 ;hides 'dey'
	delay_n 34
	inx
	Jne *-7
.elseif(n = 4921)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
; 4922 to 4960
.if(n = 4922 || n = 4924 || n = 4926 || n = 4928 || n = 4932 || n = 4934 || n = 4935 || n = 4937 || n = 4938 || n = 4940 || n = 4944 || n = 4946 || n = 4947 || n = 4952 || n = 4953 || n = 4955 || n = 4958)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4923)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4925)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4927)
	; 9 bytes
	ldx #168 ;hides 'tay'
	delay_n 49
	inx
	Jne *-6
.elseif(n = 4929)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4930)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4931 || n = 4956)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4933)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4936 || n = 4957)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4939)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 981
	dey
	Jmi *-7
.elseif(n = 4941)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4942)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4943)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4945)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4948)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4949)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 4950)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4951)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4954)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4959)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4960)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
.if((n >= 4961) && (n < 4981))
; 4961 to 4980
.if(n = 4961)
	; 10 bytes
	ldx #(n-497)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4962)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4963)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #12
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 4964)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 986
	dey
	Jmi *-7
.elseif(n = 4965)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4966)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4967)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 4968 || n = 4970 || n = 4972 || n = 4974 || n = 4975 || n = 4980)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4969)
	; 9 bytes
	ldx #91
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 4971)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4973)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 4976)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #109
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 4977)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4978)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4979)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 4981 to 5000
.if(n = 4981)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4982 || n = 4986 || n = 4988 || n = 4990 || n = 4994 || n = 4997 || n = 4998 || n = 5000)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4983)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 49
	dex
	Jmi *-6
.elseif(n = 4984)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4985)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4987)
	; 10 bytes
	ldx #(n-1)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4989)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 4991)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #7
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 4992)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 4993)
	; 9 bytes
	ldy #13
	delay_n 379
	dey
	Jne *-5
.elseif(n = 4995)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4996)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 4999)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
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
; 5001 to 5039
.if(n = 5001 || n = 5002 || n = 5004 || n = 5007 || n = 5008 || n = 5010 || n = 5012 || n = 5018 || n = 5022 || n = 5024 || (n > 5026 && n < 5031) || (n > 5034 && n < 5039))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5003)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5005)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5006)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5009)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5011)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5013)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5014)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 996
	dey
	Jmi *-7
.elseif(n = 5015)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5016)
	; 10 bytes
	ldy #166 ;hides 'ldx zp'
	dey ;first loop only
	delay_n 124
	dey
	Jmi *-7
.elseif(n = 5017)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5019)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5020)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5021)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5023)
	; 9 bytes
	ldx #93
	delay_n 49
	dex
	Jne *-5
.elseif(n = 5025)
	; 10 bytes
	ldx #3
	ldy #(n-25)/20
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5026)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5031)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5032)
	; 9 bytes
	ldy #39
	delay_n 124
	dey
	Jne *-5
.elseif(n = 5033)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5034)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5039)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 41
	dey
	Jmi *-7
.endif
.else
.if((n >= 5040) && (n < 5059))
; 5040 to 5058
.if((n .mod 6 = 0) && ((n > 5039 && n < 5059)))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5041)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5042)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5043)
	; 10 bytes
	ldy #24 ;hides 'clc'
	delay_n 379
	dey
	dey
	Jpl *-7
.elseif(n = 5044)
	; 10 bytes
	ldx #(n-42)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5045)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5047)
	; 10 bytes
	ldx #(n-1)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5048)
	; 10 bytes
	ldx #(n-722)/721
	ldy #143
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5049)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5050)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5051)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5053)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5054)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5055)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5056)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5057)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 5059 to 5078
.if(n = 5059)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5060 || n = 5067 || n = 5068 || n = 5070 || n = 5076 || n = 5078)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5061)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5062)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5063)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #124
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5064)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5065)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5066)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #111
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 5069)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5071)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5072)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5073)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 111
	dey
	Jmi *-7
.elseif(n = 5074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 5075)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5077)
	; 9 bytes
	ldx #94
	delay_n 49
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5098)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5080 || n = 5082 || (n > 5085 && n < 5089) || (n > 5089 && n < 5093) || n = 5094 || n = 5100 || n = 5102 || n = 5108 || n = 5112 || n = 5115 || n = 5117)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5081 || n = 5106)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5083)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5084)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5085 || n = 5116)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5089)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5093)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5095)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5096)
	; 9 bytes
	ldy #4
	delay_n 1014
	dey
	Jpl *-5
.elseif(n = 5097)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5099)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 61
	dey
	Jmi *-7
.elseif(n = 5101)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5103)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #9
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5104)
	; 9 bytes
	ldy #136 ;hides 'dey'
	delay_n 1014
	dey
	Jmi *-6
.elseif(n = 5105)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5107)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5109)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 66
	iny
	Jne *-7
.elseif(n = 5110)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5111)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5113)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5114)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1016
	dey
	Jmi *-7
.endif
.else
; 5118 to 5156
.if(n = 5118)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5119)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #23
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5120 || n = 5122 || n = 5124 || n = 5128 || n = 5130 || n = 5132 || n = 5134 || n = 5136 || n = 5138 || n = 5141 || n = 5142 || n = 5144 || n = 5154 || n = 5155)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5121)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5123 || n = 5149)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5125 || n = 5146)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5126)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5127)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5129)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5131)
	; 9 bytes
	ldx #94
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5133)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5135)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5137)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5139)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1021
	dey
	Jmi *-7
.elseif(n = 5140)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5143)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #126
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5145)
	; 10 bytes
	ldx #<((n-1287)/1286)
	ldy #>((n-1287)/1286)
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5147)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5148)
	; 10 bytes
	ldy #166 ;hides 'ldx zp'
	nop
	delay_n 124
	dey
	Jmi *-7
.elseif(n = 5150)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5151)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5152)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5153)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5156)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
; 5157 to 5195
.if(n = 5157 || n = 5160 || n = 5162 || n = 5165 || n = 5166 || n = 5168 || (n > 5169 && n < 5173) || n = 5174 || n = 5176 || n = 5177 || n = 5179 || n = 5180 || n = 5190 || n = 5194)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5158)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5159)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #6
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5161)
	; 9 bytes
	ldy #39
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 5163 || n = 5187)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5164)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1026
	dey
	Jmi *-7
.elseif(n = 5167 || n = 5188)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5169)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5173)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5175)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5178)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5181)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5182)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5183)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #127
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5184)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5185)
	; 9 bytes
	ldx #95
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5186)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5189)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1031
	dey
	Jmi *-7
.elseif(n = 5191)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5192)
	; 10 bytes
	ldx #29
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 5193)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5195)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 5196 to 5234
.if(n = 5196 || n = 5198 || n = 5200 || n = 5210 || n = 5212 || n = 5213 || n = 5216 || n = 5219 || n = 5220 || n = 5222 || (n > 5223 && n < 5227) || n = 5228 || n = 5229 || n = 5232 || n = 5234)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5197)
	; 10 bytes
	ldx #(n-1)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5199)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5201 || n = 5227)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5202)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #6
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5203)
	; 9 bytes
	ldx #17
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 5204)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5205)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5206 || n = 5231)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5207)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 5208)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5209 || n = 5230)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5211)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5214)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5215)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5217)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5218)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5221)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5223)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5233)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
; 5235 to 5273
.if(n = 5235)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #45
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5236 || n = 5238 || n = 5242 || n = 5248 || n = 5252 || n = 5259 || n = 5260 || n = 5262 || (n > 5266 && n < 5271) || n = 5273)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5237)
	; 10 bytes
	ldx #(n-477)/476
	ldy #93
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5239)
	; 9 bytes
	ldx #96
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5240 || n = 5271)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5241)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5243)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #26
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 5244)
	; 10 bytes
	ldx #164 ;hides 'ldy zp'
	nop
	delay_n 49
	inx
	Jne *-7
.elseif(n = 5245)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5246)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #114
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5247)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5249)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5250)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5251 || n = 5272)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5253)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5254)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5255)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5256)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5257)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5258)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5261)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5263)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #130
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5264)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1046
	dey
	Jmi *-7
.elseif(n = 5265)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5266)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 5274 to 5312
.if(n = 5274 || n = 5278 || (n > 5281 && n < 5286) || n = 5294 || n = 5296 || n = 5298 || n = 5299 || n = 5304 || n = 5307 || n = 5309 || n = 5310)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5275)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5276)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5277)
	; 10 bytes
	ldx #202 ;hides 'dex'
	.repeat 3
	 php
	 plp
	.endrepeat
	Jne *-7
.elseif(n = 5279 || n = 5305)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5280)
	; 10 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 124
	dey
	Jmi *-7
.elseif(n = 5281)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5286)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5287)
	; 10 bytes
	ldx #(n-1)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5288)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5289)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5290)
	; 9 bytes
	ldy #40
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 5291)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5292)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5293)
	; 9 bytes
	ldx #97
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5295)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5297)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5300)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 5301)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5302)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5303)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5306)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5308)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5311)
	; 10 bytes
	ldx #(n-532)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jpl *-6
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
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5314 || n = 5335)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5315 || n = 5316 || n = 5318 || n = 5322 || n = 5323 || n = 5327 || n = 5330 || n = 5332 || n = 5334 || n = 5338 || n = 5342 || n = 5346 || n = 5348 || n = 5350)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5317)
	; 10 bytes
	ldx #(n-1)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #25
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5320)
	; 10 bytes
	ldx #(n-592)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5321)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 5325)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5326)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5328)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5329)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5331)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5333)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5336)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #117
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 5337)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5339)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1061
	dey
	Jmi *-7
.elseif(n = 5340)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5341)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5343)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #132
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5344)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5345)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #46
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5347)
	; 9 bytes
	ldx #98
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5349)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5351)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-7
.endif
.else
; 5352 to 5390
.if(n = 5352 || n = 5355 || n = 5358 || n = 5360 || n = 5362 || (n > 5365 && n < 5369) || (n > 5373 && n < 5377) || n = 5378 || n = 5380 || n = 5382 || (n > 5383 && n < 5389) || n = 5390)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5353)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5354)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5356)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5357 || n = 5383)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5359)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5361)
	; 10 bytes
	ldx #(n-537)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5363)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5364)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5365)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5369)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5370)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5371)
	; 10 bytes
	ldx #30
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 5372)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5373)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5377)
	; 9 bytes
	ldy #13
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 5379)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5381)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 16
	dex
	Jne *-7
.elseif(n = 5389)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1071
	dey
	Jmi *-7
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
; 5391 to 5429
.if((n > 5390 && n < 5395) || n = 5396 || n = 5399 || n = 5400 || n = 5404 || n = 5408 || n = 5412 || n = 5415 || n = 5418 || n = 5420 || n = 5424 || n = 5427)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5395 || n = 5426)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5397)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5398)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5401)
	; 9 bytes
	ldx #100
	delay_n 49
	dex
	Jne *-5
.elseif(n = 5402)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5403)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5405)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5406)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5407)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5409)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5410)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5411)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5413)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5414)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1076
	dey
	Jmi *-7
.elseif(n = 5416)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5417)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 56
	iny
	Jne *-7
.elseif(n = 5419)
	; 9 bytes
	ldy #42
	delay_n 124
	dey
	Jne *-5
.elseif(n = 5421)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5422)
	; 10 bytes
	ldx #139
	delay_n 34
	dex
	Jne *-6
.elseif(n = 5423)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #134
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5425)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5428)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5429)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 5430 to 5468
.if(n = 5430)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5431)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5432 || n = 5434 || n = 5436 || n = 5438 || (n > 5441 && n < 5446) || n = 5447 || n = 5448 || n = 5450 || n = 5451 || n = 5460 || n = 5462 || n = 5465 || n = 5466)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5433)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5435 || n = 5461)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5437)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5439)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1081
	dey
	Jmi *-7
.elseif(n = 5440)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5441)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5446)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5449)
	; 10 bytes
	ldx #(n-1)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5452)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #25
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 5453)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5454)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5455)
	; 9 bytes
	ldx #100
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5456)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5457)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5458)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5459)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 71
	iny
	Jne *-7
.elseif(n = 5463)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #135
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5464)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1086
	dey
	Jmi *-7
.elseif(n = 5467)
	; 10 bytes
	ldx #(n-1)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5468)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
.if((n >= 5469) && (n < 5508))
; 5469 to 5507
.if(n = 5469 || n = 5470 || n = 5472 || n = 5476 || n = 5478 || n = 5480 || (n > 5482 && n < 5486) || n = 5490 || n = 5493 || n = 5496 || n = 5498 || n = 5499 || n = 5501 || n = 5502 || n = 5504 || n = 5507)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5471)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 5473)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5474)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5475)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5477)
	; 10 bytes
	ldx #73
	delay_n 69
	dex
	Jpl *-6
.elseif(n = 5479)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5481 || n = 5506)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5482)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5486)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 86
	dey
	Jmi *-7
.elseif(n = 5487)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5488)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5489)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5491)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5492)
	; 9 bytes
	ldx #19
	delay_n 284
	dex
	Jne *-5
.elseif(n = 5494)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5495)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5497)
	; 10 bytes
	ldx #(n-1)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5500)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5503)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5505)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
.if((n >= 5508) && (n < 5527))
; 5508 to 5526
.if(n = 5508 || n = 5510 || n = 5518 || n = 5520 || n = 5522 || n = 5524 || n = 5525)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5509)
	; 9 bytes
	ldx #101
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5511)
	; 10 bytes
	ldx #(n-552)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5512)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5513)
	; 9 bytes
	ldx #8
	delay_n 684
	dex
	Jne *-5
.elseif(n = 5514)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1096
	dey
	Jmi *-7
.elseif(n = 5515)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5516)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #121
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 5517)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
	dex
	Jmi *-7
.elseif(n = 5519)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5521)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5523)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5526)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 5527 to 5546
.if(n = 5527)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 684
	inx
	Jne *-6
.elseif(n = 5528 || n = 5532 || n = 5534 || n = 5535 || n = 5541 || n = 5542 || n = 5546)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5529)
	; 10 bytes
	ldx #(n-1)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5530)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5531)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5533)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5536)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5537)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5538)
	; 10 bytes
	ldx #(n-792)/791
	ldy #157
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5539)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5540)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5543)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5544)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5545)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 5547) && (n < 5586))
; 5547 to 5585
.if(n = 5547)
	; 10 bytes
	ldx #200 ;hides 'iny'
	delay_n 69
	dex
	Jmi *-7
.elseif(n = 5548)
	; 9 bytes
	ldy #43
	delay_n 124
	dey
	Jne *-5
.elseif(n = 5549)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5550 || n = 5581)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5551)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5552)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif((n > 5552 && n < 5556) || n = 5558 || n = 5559 || n = 5562 || n = 5568 || (n > 5569 && n < 5573) || n = 5576 || n = 5579 || n = 5580 || n = 5582 || n = 5584)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5556)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5557)
	; 10 bytes
	ldx #(n-1)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5560)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5561)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5563)
	; 9 bytes
	ldx #102
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5564)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 46
	dey
	Jmi *-7
.elseif(n = 5565)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5566)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5567)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5569)
	; 10 bytes
	ldx #(n-97)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5573)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 5575)
	; 10 bytes
	ldx #136 ;hides 'dey'
	delay_n 34
	dex
	Jne *-7
.elseif(n = 5577)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5578)
	; 10 bytes
	ldx #143
	delay_n 34
	dex
	Jne *-6
.elseif(n = 5583)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #138
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5585)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #18
	dey
	Jne *-1
	dex
	Jpl *-7
.endif
.else
.if((n >= 5586) && (n < 5606))
; 5586 to 5605
.if(n = 5586 || n = 5592 || (n > 5593 && n < 5599) || n = 5600 || n = 5602 || n = 5603 || n = 5605)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5587)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5588)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5589)
	; 10 bytes
	ldx #10
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 5590)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5591)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5593)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5599)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5601)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5604)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 5606 to 5625
.if(n = 5606)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5607 || n = 5616 || n = 5620 || n = 5624)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5608)
	; 10 bytes
	ldx #(n-802)/801
	ldy #159
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5609)
	; 10 bytes
	ldx #(n-1)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5610)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5611)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5612)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5613)
	; 10 bytes
	ldx #(n-47)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5614)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1116
	dey
	Jmi *-7
.elseif(n = 5615)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5617)
	; 9 bytes
	ldx #104
	delay_n 49
	dex
	Jne *-5
.elseif(n = 5618)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5619)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5621)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5622)
	; 10 bytes
	ldx #(n-512)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5623)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #27
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5625)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.if(n = 5626 || n = 5627 || n = 5630 || n = 5637 || n = 5638 || n = 5640 || n = 5642)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5628)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5629)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5631)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5632)
	; 9 bytes
	ldy #170 ;hides 'tax'
	delay_n 124
	dey
	Jmi *-6
.elseif(n = 5633)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5634)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5635)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5636)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5639)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1121
	dey
	Jmi *-7
.elseif(n = 5641)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5643)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5644)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 5645 to 5664
.if(n = 5645)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif((n .mod 2 = 0) && ((n > 5645 && n < 5655) || n = 5658))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5647)
	; 10 bytes
	ldx #(n-1)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5649)
	; 10 bytes
	ldx #(n-1)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5651)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5653)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5655)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 21
	dey
	Jne *-7
.elseif(n = 5656)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5657)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5659)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5660)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	dex
	Jmi *-7
.elseif(n = 5661)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5662)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5663)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #140
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5664)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1126
	dey
	Jmi *-7
.endif
.endif
.else
; 5665 to 5703
.if(n = 5665)
	; 10 bytes
	ldx #(n-97)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif((n > 5665 && n < 5669) || n = 5672 || n = 5676 || (n > 5681 && n < 5685) || n = 5686 || n = 5687 || n = 5690 || n = 5691 || n = 5694 || n = 5702)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5669 || n = 5695)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5670)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #8
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5671)
	; 9 bytes
	ldx #104
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5673 || n = 5679)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5674)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5675)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #49
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5677)
	; 9 bytes
	ldy #44
	delay_n 124
	dey
	Jne *-5
.elseif(n = 5678)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5680)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5681)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5685)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5688)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5689)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5692)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5693)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #13
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5696)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #124
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5697)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5698)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 5700)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5701)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5703)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 16
	dex
	Jne *-7
.endif
.endif
.else
.if((n >= 5704) && (n < 5743))
; 5704 to 5742
.if(n = 5704 || (n > 5707 && n < 5711) || n = 5712 || n = 5715 || n = 5717 || (n > 5721 && n < 5725) || n = 5726 || n = 5728 || n = 5730 || n = 5733 || n = 5738 || n = 5740 || n = 5742)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5705 || n = 5736)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5706)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5707)
	; 10 bytes
	ldx #(n-1)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5711)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5713)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5714)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1136
	dey
	Jmi *-7
.elseif(n = 5716)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5718)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 126
	dey
	Jmi *-7
.elseif(n = 5719)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #27
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5720)
	; 10 bytes
	ldx #(n-1)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5721)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5725)
	; 9 bytes
	ldx #105
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5727)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5729)
	; 10 bytes
	ldx #(n-1)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5731)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5732)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5734)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5735)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5737)
	; 10 bytes
	ldx #(n-1)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5739)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5741)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 5743 to 5781
.if(n = 5743)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif((n > 5743 && n < 5747) || n = 5749 || n = 5750 || n = 5753 || n = 5754 || n = 5757 || n = 5760 || n = 5762 || n = 5766 || n = 5768 || n = 5771 || n = 5772 || n = 5775 || n = 5778 || n = 5780)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5747 || n = 5773)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5748)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5751)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5752)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5755)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5756)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5758)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5759)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #7
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5761)
	; 9 bytes
	ldy #15
	delay_n 379
	dey
	Jne *-5
.elseif(n = 5763)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5764)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5765)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5767)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5769)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5770)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5774)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5776)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5777)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5779)
	; 9 bytes
	ldx #106
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5781)
	; 9 bytes
	ldx #19
	delay_n 284
	dex
	Jpl *-5
.endif
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
.if((n >= 5782) && (n < 5821))
; 5782 to 5820
.if(n = 5782)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5783)
	; 10 bytes
	ldx #(n-827)/826
	ldy #164
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5784 || n = 5788 || n = 5790 || n = 5792 || n = 5794 || n = 5795 || n = 5800 || (n > 5801 && n < 5806) || n = 5810 || n = 5816 || n = 5817 || n = 5819 || n = 5820)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5785)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5786)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #126
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5787)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5789)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1151
	dey
	Jmi *-7
.elseif(n = 5791)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5793)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5796)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5797)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5798)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5799)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5801)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5806)
	; 9 bytes
	ldy #45
	delay_n 124
	dey
	Jne *-5
.elseif(n = 5807)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #7
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5808)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5809)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5811)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5812)
	; 10 bytes
	ldx #149
	delay_n 34
	dex
	Jne *-6
.elseif(n = 5813)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #28
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5814)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1156
	dey
	Jmi *-7
.elseif(n = 5815)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5818)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 5821 to 5859
.if(n = 5821)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5822 || n = 5826 || n = 5828 || n = 5832 || n = 5834 || (n > 5835 && n < 5839) || n = 5840 || (n > 5843 && n < 5847) || n = 5848 || n = 5850 || n = 5852 || n = 5854 || n = 5855 || n = 5858)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5823)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 49
	inx
	Jpl *-6
.elseif(n = 5824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 5825 || n = 5851)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5827)
	; 10 bytes
	ldx #(n-1)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5829)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5830)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5831)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5833)
	; 9 bytes
	ldx #107
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5835)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5839)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1161
	dey
	Jmi *-7
.elseif(n = 5841)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5842)
	; 10 bytes
	ldx #(n-532)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5843)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5847)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 5849)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5853)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5856)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5857)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5859)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860 || n = 5891)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5861)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5862)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #27
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 5863)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #145
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5864)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5865 || n = 5867 || n = 5868 || (n > 5871 && n < 5875) || (n > 5877 && n < 5881) || n = 5882 || n = 5885 || n = 5886 || n = 5892 || n = 5898)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5866)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5869)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5870)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 5871)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	inx
	Jne *-7
.elseif(n = 5875)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5876)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #128
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 5877)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5881)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5883)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5884)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5887)
	; 9 bytes
	ldx #108
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5888)
	; 10 bytes
	ldx #(n-842)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5889)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5890)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5893)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5894)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5895)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5896)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5897)
	; 10 bytes
	ldx #(n-537)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 5899 to 5937
.if(n = 5899 || n = 5900 || n = 5904 || n = 5907 || n = 5912 || n = 5913 || n = 5915 || n = 5920 || (n > 5923 && n < 5929) || n = 5932 || n = 5934 || n = 5936)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5901)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5902)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5903 || n = 5929)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5905)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5906 || n = 5931)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5908)
	; 10 bytes
	ldx #33
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 5909)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5910)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5911)
	; 10 bytes
	ldx #(n-592)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5914)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5916)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5917)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5918)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5919)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #27
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 5921)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5922)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5923)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5930)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5933)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 131
	dey
	Jmi *-7
.elseif(n = 5935)
	; 9 bytes
	ldy #45
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 5937)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
.if((n >= 5938) && (n < 5977))
; 5938 to 5976
.if(n = 5938 || n = 5940 || n = 5942 || n = 5944 || n = 5945 || n = 5948 || n = 5954 || n = 5957 || n = 5962 || n = 5967 || (n > 5969 && n < 5974) || n = 5976)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5939)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1181
	dey
	Jmi *-7
.elseif(n = 5941)
	; 9 bytes
	ldx #110
	delay_n 49
	dex
	Jne *-5
.elseif(n = 5943)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5946)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5947)
	; 10 bytes
	ldx #(n-1)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5949)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 5950)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5951)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #47
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 5952)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5953)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5955)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5956)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5958)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5959)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5960)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5961)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5963)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5964)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1186
	dey
	Jmi *-7
.elseif(n = 5965)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5966)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #131
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 5968)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5969)
	; 10 bytes
	ldx #(n-1)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5974)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5975)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 5977 to 6015
.if(n = 5977 || n = 6013)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5978 || n = 5980 || n = 5982 || n = 5988 || n = 5990 || n = 5992 || n = 5996 || (n > 5997 && n < 6001) || n = 6002 || n = 6008 || n = 6010 || n = 6012)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5979)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5981 || n = 6007)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 5983)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #12
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 5984 || n = 6015)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5985)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5986)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5987)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 5989)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1191
	dey
	Jmi *-7
.elseif(n = 5991)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 49
	dex
	Jmi *-6
.elseif(n = 5993)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 5994)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 5995)
	; 9 bytes
	ldx #110
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 5997)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6001)
	; 10 bytes
	ldx #100
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif(n = 6003)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #29
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 6004)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6005)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6006)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6009)
	; 10 bytes
	ldx #(n-1)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6011)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6014)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1196
	dey
	Jmi *-7
.endif
.endif
.else
.if((n >= 6016) && (n < 6055))
; 6016 to 6054
.if(n = 6016)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6017)
	; 10 bytes
	ldx #(n-1)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6018 || n = 6020 || n = 6022 || n = 6024 || n = 6029 || n = 6030 || n = 6032 || n = 6034 || n = 6038 || n = 6042 || n = 6044 || n = 6045 || n = 6048 || n = 6050 || n = 6052 || n = 6054)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6019)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6021)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6023)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #149
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6025)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6026)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6027)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6028)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6031)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6033)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6035)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6036)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6037)
	; 10 bytes
	ldx #(n-1)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6039)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1201
	dey
	Jmi *-7
.elseif(n = 6040)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6041)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6043)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6046)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6047)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 6049)
	; 9 bytes
	ldx #111
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 6051)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6053)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 6055 to 6093
.if(n = 6055 || n = 6057 || n = 6060 || n = 6068 || n = 6075 || n = 6078 || n = 6080 || (n > 6081 && n < 6085) || n = 6086 || n = 6088 || n = 6090 || n = 6092 || n = 6093)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6056)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6058)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #15
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6059 || n = 6085)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6061)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6062)
	; 10 bytes
	ldx #(n-552)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6063)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6064)
	; 9 bytes
	ldy #46
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 6065)
	; 10 bytes
	ldx #3
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 6066 || n = 6072)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6067)
	; 10 bytes
	ldx #(n-1)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6069)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6070)
	; 9 bytes
	ldx #20
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 6071)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 6073)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 6076)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6077)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6079)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jmi *-7
.elseif(n = 6081)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6087)
	; 10 bytes
	ldx #34
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 6089)
	; 10 bytes
	ldx #(n-1)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6091)
	; 10 bytes
	ldx #(n-1)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if((n > 6093 && n < 6097) || n = 6099 || n = 6102 || n = 6104 || n = 6109 || n = 6116 || n = 6118 || n = 6122 || n = 6123 || n = 6125 || n = 6126 || n = 6128 || n = 6132)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6097)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6098)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6100)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6101)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6103)
	; 9 bytes
	ldx #113
	delay_n 49
	dex
	Jne *-5
.elseif(n = 6105)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6106)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6107)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6108)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6110)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6111)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6112)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6113)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6114)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1216
	dey
	Jmi *-7
.elseif(n = 6115)
	; 9 bytes
	ldy #6
	delay_n 1014
	dey
	Jne *-5
.elseif(n = 6117)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6119)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6120)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6121)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6124)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6127)
	; 10 bytes
	ldx #(n-1)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6129)
	; 10 bytes
	ldx #(n-1)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6130)
	; 10 bytes
	ldx #(n-682)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6131)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 6133 to 6171
.if(n = 6133)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif((n > 6133 && n < 6137) || (n > 6139 && n < 6143) || n = 6144 || n = 6150 || n = 6153 || n = 6154 || n = 6160 || n = 6167 || n = 6171)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6137 || n = 6163)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6138)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6139 || n = 6170)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6143)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6145)
	; 9 bytes
	ldy #15
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 6146)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #135
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6147)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6148)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 136
	dey
	Jmi *-7
.elseif(n = 6149)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6151)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6152)
	; 10 bytes
	ldx #168 ;hides 'tay'
	delay_n 284
	dex
	dex
	Jmi *-7
.elseif(n = 6155)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6156)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6157)
	; 9 bytes
	ldx #113
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 6158)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6159)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 81
	iny
	Jne *-7
.elseif(n = 6161)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6162)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6164)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1226
	dey
	Jmi *-7
.elseif(n = 6165)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6166)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6168)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6169)
	; 10 bytes
	ldx #(n-1)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 6172) && (n < 6211))
; 6172 to 6210
.if(n = 6172)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6173 || n = 6174 || n = 6176 || n = 6179 || n = 6180 || n = 6182 || (n > 6183 && n < 6187) || n = 6188 || n = 6190 || n = 6194 || (n > 6195 && n < 6199) || n = 6200 || n = 6207 || n = 6208 || n = 6210)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6175)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6177)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6178)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6181)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6183)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6187)
	; 10 bytes
	ldx #(n-1)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6189)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6191)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #49
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 6192)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6193)
	; 9 bytes
	ldy #47
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 6195)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6199)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 6201)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6202)
	; 9 bytes
	ldx #8
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 6203)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6204)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6205)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6206)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6209)
	; 10 bytes
	ldx #(n-1)/776
	ldy #154
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 6211 to 6250
.if(n = 6211)
	; 9 bytes
	ldx #114
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 6212 || n = 6216 || n = 6219 || n = 6222 || n = 6226 || n = 6228 || n = 6230 || n = 6235 || n = 6237 || n = 6240 || (n > 6241 && n < 6245) || n = 6246 || n = 6248 || n = 6250)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6213)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6214)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1236
	dey
	Jmi *-7
.elseif(n = 6215 || n = 6241)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6217)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6218)
	; 9 bytes
	ldx #136 ;hides 'dey'
	delay_n 684
	dex
	Jmi *-6
.elseif(n = 6220)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6221)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6223)
	; 10 bytes
	ldx #(n-52)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6224)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 76
	dey
	Jmi *-7
.elseif(n = 6225)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #55
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6227)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6229)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6231)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6232)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6233)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6234)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6236)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #137
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6238)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6239)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1241
	dey
	Jmi *-7
.elseif(n = 6245)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6247)
	; 10 bytes
	ldx #(n-1)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6249)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
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
.if(n = 6251 || n = 6252 || (n > 6257 && n < 6261) || n = 6262 || (n > 6267 && n < 6271) || n = 6275 || n = 6278 || n = 6283 || (n > 6284 && n < 6289))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6253)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6254)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6255)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6256 || n = 6281)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6257)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6261)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6263)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6264)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1246
	dey
	Jmi *-7
.elseif(n = 6265)
	; 9 bytes
	ldx #115
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 6266)
	; 10 bytes
	ldx #34
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 6267)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6271)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6272)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #28
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 6273)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6274)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6276)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6277)
	; 10 bytes
	ldx #(n-1)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6279)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6280)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6282)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6284)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6289)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
.if((n >= 6290) && (n < 6309))
; 6290 to 6308
.if(n = 6290)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6291)
	; 10 bytes
	ldx #84
	delay_n 69
	dex
	Jpl *-6
.elseif(n = 6292 || (n > 6294 && n < 6301) || n = 6302 || n = 6305)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6293)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6294)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6301)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6303)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6304)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6306)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6307)
	; 10 bytes
	ldx #(n-1)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6308)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 6309 to 6328
.if(n = 6309)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6310)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6311)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6312 || n = 6317 || n = 6318 || n = 6323)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6313)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6314)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1256
	dey
	Jmi *-7
.elseif(n = 6315)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6316)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6319)
	; 9 bytes
	ldx #117
	delay_n 49
	dex
	Jne *-5
.elseif(n = 6320)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6321)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6322)
	; 9 bytes
	ldy #49
	delay_n 124
	dey
	Jne *-5
.elseif(n = 6324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 6325)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6326)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #139
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6327)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 6328)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if(n = 6329)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6330 || (n > 6331 && n < 6335) || n = 6338 || (n > 6339 && n < 6343) || n = 6344 || n = 6347 || n = 6348 || (n > 6349 && n < 6355) || n = 6357 || n = 6360 || n = 6362 || n = 6366)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6331)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6335)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #15
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 6336)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6337)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6339)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1261
	dey
	Jmi *-7
.elseif(n = 6343)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6345)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6346)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6349)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6355)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6356)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6358)
	; 10 bytes
	ldx #163
	delay_n 34
	dex
	Jne *-6
.elseif(n = 6359)
	; 9 bytes
	ldx #21
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 6361)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6363)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 141
	dey
	Jmi *-7
.elseif(n = 6364)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6365)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6367)
	; 10 bytes
	ldx #(n-1)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 6368 to 6406
.if(n = 6368 || n = 6370 || n = 6372 || n = 6374 || n = 6377 || n = 6379 || n = 6380 || n = 6382 || n = 6386 || n = 6388 || n = 6390 || n = 6394 || n = 6396 || n = 6398 || n = 6402 || n = 6404 || n = 6405)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6369)
	; 10 bytes
	ldx #(n-1)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6371 || n = 6397)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6373)
	; 9 bytes
	ldx #118
	delay_n 49
	dex
	Jne *-5
.elseif(n = 6375)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6376)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6378)
	; 10 bytes
	ldx #(n-912)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6381)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6383)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 6384)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6385)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6387)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6389)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1271
	dey
	Jmi *-7
.elseif(n = 6391)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6392)
	; 10 bytes
	ldx #(n-582)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6393)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6395)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6399)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	inx
	Jne *-7
.elseif(n = 6400)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6401)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 284
	inx
	Jne *-6
.elseif(n = 6403)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6406)
	; 10 bytes
	ldx #4
	ldy #(n-31)/25
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
; 6407 to 6445
.if(n = 6407 || n = 6408 || n = 6415 || n = 6419 || n = 6422 || n = 6424 || n = 6426 || n = 6428 || n = 6430 || n = 6432 || n = 6434 || n = 6435 || n = 6437 || (n > 6441 && n < 6445))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6409 || n = 6445)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6410)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6411)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6412)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #8
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 6413)
	; 10 bytes
	ldx #(n-917)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6414)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6416)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #141
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6417)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6418)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6420)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6421)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6423)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6425)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6427)
	; 9 bytes
	ldx #118
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 6429)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6431)
	; 10 bytes
	ldx #<((n-1287)/1286)
	ldy #>((n-1287)/1286)
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6433)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6436)
	; 10 bytes
	ldx #165
	delay_n 34
	dex
	Jne *-6
.elseif(n = 6438)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6439)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6440)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 6441)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 6446 to 6484
.if(n = 6446 || n = 6450 || n = 6452 || n = 6454 || n = 6456 || n = 6458 || n = 6460 || n = 6468 || (n > 6469 && n < 6475) || n = 6476 || n = 6482 || n = 6484)
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
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6448)
	; 10 bytes
	ldx #(n-922)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6449 || n = 6475)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6451)
	; 9 bytes
	ldy #49
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 6453)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6455)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6457)
	; 10 bytes
	ldx #(n-1)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6459)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 31
	dey
	Jne *-7
.elseif(n = 6461)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6462)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6463)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #160
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6464)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6465)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6466)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6467)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6469)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6477)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #29
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 6478)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6479)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6480)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6481)
	; 9 bytes
	ldx #120
	delay_n 49
	dex
	Jne *-5
.elseif(n = 6483)
	; 10 bytes
	ldx #(n-927)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 6485) && (n < 6524))
; 6485 to 6523
.if(n = 6485 || n = 6486 || n = 6488 || n = 6491 || n = 6492 || n = 6498 || n = 6500 || n = 6504 || n = 6507 || n = 6510 || n = 6512 || n = 6515 || n = 6516 || n = 6522 || n = 6523)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6487)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6489)
	; 10 bytes
	ldx #(n-1)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6490)
	; 10 bytes
	ldx #(n-722)/721
	ldy #143
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6493)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6494)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6495)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6496)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #13
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6497)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6499)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6501)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6502)
	; 10 bytes
	ldx #(n-592)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6503)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #161
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6505)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6506)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #143
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6508)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6509)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 86
	iny
	Jne *-7
.elseif(n = 6511)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6513)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6514)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6517)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6518)
	; 10 bytes
	ldx #(n-932)/931
	ldy #185
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6519)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #31
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6520)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6521)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 6524 to 6562
.if(n = 6524 || n = 6530 || n = 6534 || n = 6536 || (n > 6537 && n < 6541) || (n > 6543 && n < 6547) || n = 6548 || n = 6549 || n = 6552 || n = 6554 || n = 6556 || n = 6558 || n = 6560)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6525)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6526)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6527 || n = 6553)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6528)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6529)
	; 9 bytes
	ldy #16
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 6531)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6532)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6533)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6535)
	; 9 bytes
	ldx #120
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 6537)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6541)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6542)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6543)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #162
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6547)
	; 10 bytes
	ldx #(n-1)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6550)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6551)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6555)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6557)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6559)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jmi *-7
.elseif(n = 6561)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6562)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
.if((n >= 6563) && (n < 6602))
; 6563 to 6601
.if(n = 6563)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6564 || n = 6565 || n = 6567 || n = 6568 || n = 6570 || n = 6572 || n = 6576 || n = 6581 || (n > 6583 && n < 6587) || n = 6590 || n = 6594 || n = 6597 || n = 6598 || n = 6600)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6566)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6569)
	; 10 bytes
	ldx #(n-1)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6571)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6573)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 6575)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6577)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6578)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 146
	dey
	Jmi *-7
.elseif(n = 6579)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6580)
	; 9 bytes
	ldy #51
	delay_n 124
	dey
	Jne *-5
.elseif(n = 6582 || n = 6591)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6583)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #163
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6587)
	; 10 bytes
	ldx #89
	delay_n 69
	dex
	Jne *-6
.elseif(n = 6588)
	; 10 bytes
	ldx #(n-942)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6589)
	; 9 bytes
	ldx #121
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 6592)
	; 10 bytes
	ldx #169
	delay_n 34
	dex
	Jne *-6
.elseif(n = 6593)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6595)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6596)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #145
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #4
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 6601)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 6602 to 6640
.if(n = 6602)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6603 || n = 6606 || n = 6608 || n = 6610 || n = 6615 || (n > 6616 && n < 6621) || n = 6622 || n = 6626 || n = 6627 || n = 6629 || n = 6630 || n = 6632 || n = 6636 || (n > 6637 && n < 6641))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6604 || n = 6635)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6605 || n = 6631)
	; 10 bytes
	ldx #(n-1)/26
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6607)
	; 10 bytes
	ldx #(n-1)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6609)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6611)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6612)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6613)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6614)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 56
	dey
	Jmi *-7
.elseif(n = 6616)
	; 10 bytes
	ldx #(n-1)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6621)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6623)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6624)
	; 10 bytes
	ldx #36
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 6625)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6628)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6633)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
	dex
	Jmi *-7
.elseif(n = 6634)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6637)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 6641) && (n < 6680))
; 6641 to 6679
.if(n = 6641)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6642 || n = 6646 || n = 6647 || (n > 6651 && n < 6656) || n = 6659 || n = 6660 || n = 6662 || n = 6664 || n = 6668 || n = 6669 || n = 6672 || n = 6674 || (n > 6675 && n < 6679))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6643)
	; 9 bytes
	ldx #122
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 6644)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6645)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6648)
	; 9 bytes
	ldx #22
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 6649)
	; 10 bytes
	ldx #(n-1)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6650)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6651)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6656)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6657)
	; 10 bytes
	ldx #0
	ldy #4
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6658)
	; 10 bytes
	ldx #(n-952)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6661)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6663)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #165
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6665)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6666)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6667)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6670)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6671)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6673)
	; 10 bytes
	ldx #(n-1)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6675)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6679)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 6680 to 6718
.if(n = 6680 || n = 6681 || n = 6683 || n = 6685 || n = 6688 || n = 6690 || n = 6692 || n = 6696 || n = 6701 || n = 6702 || (n > 6703 && n < 6707) || n = 6708 || n = 6710 || (n > 6711 && n < 6715) || n = 6716 || n = 6718)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6682)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6684)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6686)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6687)
	; 10 bytes
	ldx #168 ;hides 'tay'
	delay_n 69
	inx
	Jne *-7
.elseif(n = 6689)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6691)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6693)
	; 10 bytes
	ldx #(n-957)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6694)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6695)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #16
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 6697)
	; 9 bytes
	ldx #124
	delay_n 49
	dex
	Jne *-5
.elseif(n = 6698)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 6700)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6703)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #166
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6707)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 71
	iny
	Jne *-7
.elseif(n = 6709)
	; 9 bytes
	ldy #52
	delay_n 124
	dey
	Jne *-5
.elseif(n = 6711)
	; 10 bytes
	ldx #(n-672)/671
	ldy #133
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6715)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6717)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 6719) && (n < 6797))
.if((n >= 6719) && (n < 6758))
; 6719 to 6757
.if(n = 6719)
	; 9 bytes
	ldx #136 ;hides 'dey'
	delay_n 49
	inx
	Jne *-6
.elseif(n = 6720 || n = 6723 || n = 6726 || n = 6730 || n = 6732 || n = 6734 || n = 6736 || n = 6738 || n = 6739 || n = 6741 || n = 6744 || n = 6747 || n = 6750 || n = 6752 || (n > 6753 && n < 6757))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6721)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6722)
	; 10 bytes
	ldx #(n-612)/611
	ldy #121
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6724)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6725)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6727)
	; 10 bytes
	ldx #(n-1)/1121
	ldy #223
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6728)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6729)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6731)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #148
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6733)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6735)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6737)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6740)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #10
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 6742)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6743)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #167
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6745)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6746)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6748)
	; 10 bytes
	ldx #173
	delay_n 34
	dex
	Jne *-6
.elseif(n = 6749)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6751)
	; 9 bytes
	ldx #125
	delay_n 49
	dex
	Jne *-5
.elseif(n = 6753)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6757)
	; 10 bytes
	ldx #(n-1)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
.if((n >= 6758) && (n < 6777))
; 6758 to 6776
.if(n = 6758 || n = 6762 || n = 6764 || n = 6767 || n = 6770 || n = 6771 || n = 6773)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6759)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6760)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6761)
	; 10 bytes
	ldx #(n-677)/676
	ldy #134
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6763)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6765)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6766)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6768)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6769)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6772)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6774)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6775)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 49
	dex
	Jmi *-6
.elseif(n = 6776)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 6777 to 6796
.if(n = 6777)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6778)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6779 || n = 6780 || n = 6782 || n = 6786 || n = 6789 || n = 6792)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6781)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6783)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #168
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6784)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6785)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6787)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6788)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #16
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 6790)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6791)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 6793)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6794)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	dex
	Jmi *-7
.elseif(n = 6795)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6796)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 6797) && (n < 6836))
; 6797 to 6835
.if(n = 6797 || n = 6800 || n = 6802 || n = 6804 || n = 6806 || n = 6808 || n = 6810 || n = 6812 || (n > 6813 && n < 6817) || n = 6819 || n = 6820 || n = 6822 || n = 6825 || (n > 6826 && n < 6832) || n = 6834)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6798)
	; 10 bytes
	ldx #(n-972)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6799)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6801)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6803)
	; 10 bytes
	ldx #37
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 6805)
	; 9 bytes
	ldx #125
	delay_n 49
	dex
	Jpl *-5
.elseif(n = 6807)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6809)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6811)
	; 10 bytes
	ldx #(n-682)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6813)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6817)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6818)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6821)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6823)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #169
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 6826)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6832)
	; 10 bytes
	ldx #(n-622)/621
	ldy #122
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6833)
	; 10 bytes
	ldx #(n-57)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6835)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 6836 to 6875
.if(n = 6836 || n = 6837 || n = 6842 || n = 6844 || n = 6846 || n = 6851 || n = 6854 || (n > 6855 && n < 6859) || n = 6862 || n = 6864 || n = 6867 || (n > 6869 && n < 6875))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6838)
	; 9 bytes
	ldy #52
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 6839)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6840)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6841)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6843)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6845)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6847)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6848)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6849)
	; 10 bytes
	ldx #(n-1)/856
	ldy #170
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6850)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6852)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6853)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6855)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6859)
	; 9 bytes
	ldx #127
	delay_n 49
	dex
	Jne *-5
.elseif(n = 6860)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6861)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6863)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6865)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6866)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #151
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6868)
	; 10 bytes
	ldx #(n-982)/981
	ldy #195
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6869)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6875)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
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
.if((n >= 6876) && (n < 6915))
; 6876 to 6914
.if(n = 6876 || n = 6878 || n = 6880 || n = 6882 || n = 6884 || n = 6890 || n = 6892 || n = 6896 || n = 6899 || n = 6900 || n = 6902 || n = 6905 || n = 6906 || n = 6908 || n = 6909 || n = 6912)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6877)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6879)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6881)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6883 || n = 6914)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6885)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #61
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6886)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6887)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6888)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6889)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6891)
	; 9 bytes
	ldx #9
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 6893)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6894)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6895)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6897)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6898)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6901)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6903)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6904)
	; 10 bytes
	ldx #(n-532)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6907)
	; 10 bytes
	ldx #(n-1)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6910)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6911)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6913)
	; 9 bytes
	ldy #18
	delay_n 379
	dey
	Jne *-5
.endif
.else
; 6915 to 6953
.if(n = 6915 || n = 6939)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6916)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6917)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6918 || n = 6920 || n = 6922 || n = 6923 || (n > 6924 && n < 6929) || (n > 6931 && n < 6937) || n = 6946 || n = 6948 || n = 6950 || n = 6951)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6919)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #32
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 6921)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6924)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6929)
	; 10 bytes
	ldx #(n-1)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6930)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6931)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6937)
	; 9 bytes
	ldx #24
	delay_n 284
	dex
	Jne *-5
.elseif(n = 6938)
	; 10 bytes
	ldx #(n-992)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6940)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6941)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #13
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 6942)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6943)
	; 10 bytes
	ldx #178
	delay_n 34
	dex
	Jne *-6
.elseif(n = 6944)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6945)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6947)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6949)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6952)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6953)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 6954) && (n < 6993))
; 6954 to 6992
.if(n = 6954 || n = 6958 || n = 6960 || n = 6962 || n = 6963 || n = 6965 || n = 6966 || n = 6968 || n = 6972 || n = 6975 || (n > 6977 && n < 6982) || n = 6984 || n = 6986 || n = 6987 || n = 6989 || n = 6992)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6955)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6956)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #153
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 6957)
	; 10 bytes
	ldx #94
	delay_n 69
	dex
	Jne *-6
.elseif(n = 6959)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6961)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6964)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6967)
	; 9 bytes
	ldy #54
	delay_n 124
	dey
	Jne *-5
.elseif(n = 6969)
	; 10 bytes
	ldx #(n-1)/871
	ldy #173
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6970)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6971)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6973)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 6974)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 86
	dey
	Jmi *-7
.elseif(n = 6976)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6977)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6982)
	; 10 bytes
	ldx #38
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 6983)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 284
	dex
	Jne *-6
.elseif(n = 6985)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 6988)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6990)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6991)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 6993 to 7031
.if(n = 6993)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 6994)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 6995)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #62
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 6996 || (n > 6997 && n < 7001) || (n > 7001 && n < 7005) || n = 7006 || n = 7013 || n = 7014 || n = 7018 || n = 7020 || n = 7022 || n = 7024 || n = 7025 || n = 7028)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6997)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7001)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7005)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7007)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7008)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7009)
	; 10 bytes
	ldx #(n-97)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7010)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 7011)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7012)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7015)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7016)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7017)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #9
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 7019)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7021)
	; 9 bytes
	ldx #130
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7023)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7026)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7027)
	; 10 bytes
	ldx #(n-1)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7029)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7030)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7031)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
.if((n >= 7032) && (n < 7071))
; 7032 to 7070
.if(n = 7032 || n = 7035 || n = 7037 || n = 7040 || n = 7042 || n = 7044 || n = 7045 || n = 7054 || n = 7056 || n = 7058 || n = 7059 || n = 7062 || n = 7064 || n = 7065 || n = 7070)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7033)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7034)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7036)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7038 || n = 7069)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7039)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7041)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7043)
	; 10 bytes
	ldx #(n-1007)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7046)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #155
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7047)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7048)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7049)
	; 10 bytes
	ldx #(n-1)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7050)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7051)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7052)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7053)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7055)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #18
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 7057)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7060)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7061)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7063)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7066)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7067)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 31
	dey
	Jne *-7
.elseif(n = 7068)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 7071 to 7109
.if(n = 7071)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7072 || n = 7076 || n = 7079 || n = 7080 || (n > 7081 && n < 7085) || n = 7086 || n = 7088 || n = 7095 || n = 7097 || n = 7102 || n = 7104 || n = 7106 || n = 7108 || n = 7109)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7073)
	; 9 bytes
	ldy #202 ;hides 'dex'
	delay_n 124
	iny
	Jne *-6
.elseif(n = 7074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 7075)
	; 9 bytes
	ldx #131
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7077)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7078)
	; 10 bytes
	ldx #(n-1012)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7081)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7085)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7087)
	; 10 bytes
	ldx #(n-1)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7089)
	; 10 bytes
	ldx #(n-1)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7090)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7091)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #156
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7092)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #32
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 7093)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7094)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7096)
	; 9 bytes
	ldy #55
	delay_n 124
	dey
	Jne *-5
.elseif(n = 7098)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7099)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7100)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7101)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7103)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7105)
	; 10 bytes
	ldx #(n-97)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7107)
	; 10 bytes
	ldx #(n-647)/646
	ldy #127
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if(n = 7110 || n = 7112 || (n > 7113 && n < 7117) || n = 7118 || (n > 7121 && n < 7126) || n = 7128 || n = 7130 || n = 7132 || n = 7142 || n = 7144 || n = 7146)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7111)
	; 10 bytes
	ldx #(n-712)/711
	ldy #141
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7113)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7117)
	; 10 bytes
	ldx #(n-1)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7119)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #33
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 7120)
	; 10 bytes
	ldx #(n-792)/791
	ldy #157
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7121)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7126)
	; 10 bytes
	ldy #4
	delay_n 1420
	dey
	Jpl *-6
.elseif(n = 7127)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 7129)
	; 9 bytes
	ldx #132
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7131)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7133)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #24 ;hides 'clc'
	dey
	Jne *-2
	dex
	Jmi *-7
.elseif(n = 7134)
	; 9 bytes
	ldy #6
	delay_n 1014
	dey
	Jpl *-5
.elseif(n = 7135)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7136)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7137)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7138)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7139)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7140)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7141)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7143)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #35
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 7145)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7147)
	; 10 bytes
	ldx #(n-1)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7148)
	; 10 bytes
	ldx #(n-1022)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 7149 to 7187
.if(n = 7149)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #216 ;hides 'cld'
	iny
	Jne *-2
	dex
	Jpl *-7
.elseif(n = 7150 || n = 7152 || n = 7156 || n = 7157 || n = 7159 || n = 7160 || n = 7163 || n = 7166 || n = 7168 || n = 7170 || n = 7171 || n = 7174 || n = 7178 || n = 7180 || n = 7182 || n = 7184 || n = 7186 || n = 7187)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7151)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7153)
	; 10 bytes
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7154)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7155)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7158)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7161)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7162)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7164)
	; 10 bytes
	ldx #(n-552)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7165)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7167)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 7169)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7172)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7173)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7175)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7176)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7177)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7179)
	; 10 bytes
	ldx #96
	delay_n 69
	dex
	Jpl *-6
.elseif(n = 7181)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 126
	iny
	Jne *-7
.elseif(n = 7183)
	; 9 bytes
	ldx #133
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7185)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
.if((n >= 7188) && (n < 7266))
.if((n >= 7188) && (n < 7227))
; 7188 to 7226
.if(n = 7188 || n = 7189 || n = 7194 || (n > 7195 && n < 7199) || n = 7200 || (n > 7201 && n < 7206) || n = 7208 || n = 7212 || n = 7214 || n = 7220 || n = 7222)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7190)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7191)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 49
	dex
	dex
	Jne *-7
.elseif(n = 7192)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7193 || n = 7224)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7195)
	; 10 bytes
	ldx #(n-67)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7199)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7201)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7206)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7207)
	; 10 bytes
	ldx #(n-1)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7209)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7210)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7211)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7213)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7215)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #63
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 7216)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7217)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7218)
	; 10 bytes
	ldx #(n-1032)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7219)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7221)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7223)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7225)
	; 9 bytes
	ldy #55
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 7226)
	; 9 bytes
	ldx #24
	delay_n 284
	dex
	Jpl *-5
.endif
.else
; 7227 to 7265
.if(n = 7227 || n = 7228 || n = 7239)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7229)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7230 || n = 7232 || (n > 7233 && n < 7237) || n = 7238 || n = 7242 || n = 7244 || n = 7245 || n = 7247 || n = 7248 || (n > 7249 && n < 7253) || n = 7254 || n = 7257 || n = 7262 || n = 7264 || n = 7265)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7231)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7233)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7237)
	; 9 bytes
	ldx #134
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7240)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7241)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7243)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7246)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7249)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7253)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7255)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7256)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 116
	dey
	Jmi *-7
.elseif(n = 7258)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7259)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7260)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7261)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7263)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #180
	dey
	Jne *-1
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 7266) && (n < 7305))
; 7266 to 7304
.if(n = 7266 || n = 7268 || n = 7270 || (n > 7276 && n < 7280) || n = 7282 || n = 7284 || n = 7285 || n = 7287 || n = 7290 || n = 7292 || n = 7296 || n = 7298 || n = 7302 || n = 7304)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7267)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7269)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7271)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 7272)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7273)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7274)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 284
	dex
	Jpl *-6
.elseif(n = 7275)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #12
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7276)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7280)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7281)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7283)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7286)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7288)
	; 10 bytes
	ldx #(n-1042)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7289)
	; 10 bytes
	ldx #(n-1)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7291)
	; 9 bytes
	ldx #135
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7293)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7294)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7295)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7297)
	; 9 bytes
	ldy #18
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 7299)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7300)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7301)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7303)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #15
	dey
	Jne *-1
	inx
	Jne *-7
.endif
.else
; 7305 to 7343
.if(n = 7305)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7306 || n = 7308 || n = 7310 || n = 7312 || n = 7313 || n = 7320 || n = 7322 || n = 7326 || (n > 7329 && n < 7333) || n = 7334 || n = 7336 || n = 7338 || n = 7339 || n = 7341 || n = 7342)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7307)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7309)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7311)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7314)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7315)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7316)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #161
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7317)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7318)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #35
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7321)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7323)
	; 10 bytes
	ldx #(n-1047)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 7325)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #64
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 7327)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7328)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7329)
	; 10 bytes
	ldx #(n-1)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7333)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7335)
	; 9 bytes
	ldy #56 ;hides 'sec'
	delay_n 124
	dey
	Jne *-6
.elseif(n = 7337)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7340)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7343)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #182
	dey
	Jne *-1
	inx
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 7344) && (n < 7422))
.if((n >= 7344) && (n < 7383))
; 7344 to 7382
.if(n = 7344 || n = 7346 || (n > 7349 && n < 7354) || n = 7355 || n = 7360 || (n > 7361 && n < 7365) || n = 7367 || n = 7368 || n = 7370 || n = 7375 || n = 7376 || n = 7378 || n = 7380)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7345)
	; 9 bytes
	ldx #136
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7347 || n = 7356)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7348 || n = 7379)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7349)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 91
	dey
	Jmi *-7
.elseif(n = 7354)
	; 9 bytes
	ldy #57
	delay_n 124
	dey
	Jne *-5
.elseif(n = 7357)
	; 10 bytes
	ldx #(n-1)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7358)
	; 10 bytes
	ldx #(n-1052)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7359)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7361)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7365)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7366)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7369)
	; 10 bytes
	ldx #(n-1)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7371)
	; 10 bytes
	ldy #200 ;hides 'iny'
	delay_n 379
	iny
	iny
	Jne *-7
.elseif(n = 7372)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7373)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7374)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7377)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7381)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7382)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 7383 to 7421
.if(n = 7383)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #183
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7384)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7385)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7386)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7387)
	; 10 bytes
	ldx #(n-1)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7388 || n = 7389 || n = 7392 || n = 7394 || n = 7395 || n = 7398 || (n > 7401 && n < 7406) || n = 7408 || n = 7412 || n = 7414 || n = 7416 || n = 7418 || n = 7419)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7390)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7391)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #60
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 7393)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7396)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7397)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7399)
	; 9 bytes
	ldx #137
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7400)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7401)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7406)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #163
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7407)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7409)
	; 10 bytes
	ldx #(n-1)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7410)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7411)
	; 10 bytes
	ldx #(n-742)/741
	ldy #147
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7413)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7415)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #18
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 7417)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7420)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jmi *-7
.elseif(n = 7421)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 7422) && (n < 7461))
; 7422 to 7460
.if(n = 7422)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7423)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #184
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7424)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7425)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7426 || n = 7429 || (n > 7430 && n < 7435) || n = 7436 || n = 7440 || n = 7442 || (n > 7443 && n < 7447) || n = 7448 || n = 7452 || n = 7454 || n = 7458 || n = 7460)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7427)
	; 10 bytes
	ldx #46
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 7428)
	; 10 bytes
	ldx #(n-1062)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7430)
	; 10 bytes
	ldx #(n-1)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7435)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7437)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7438)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7439)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
	inx
	Jne *-7
.elseif(n = 7441)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7443)
	; 10 bytes
	ldx #(n-62)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7447)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7449)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7450)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7451)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 131
	iny
	Jne *-7
.elseif(n = 7453)
	; 9 bytes
	ldx #138
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7455)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7456)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7457)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7459)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 7461 to 7500
.if(n = 7461)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7462 || n = 7464 || n = 7467 || n = 7469 || n = 7470 || n = 7473 || n = 7476 || n = 7478 || n = 7479 || n = 7482 || n = 7484 || (n > 7485 && n < 7489) || n = 7490 || n = 7491 || (n > 7492 && n < 7496) || n = 7497 || n = 7500)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7463)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7465)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7466)
	; 9 bytes
	ldy #56 ;hides 'sec'
	delay_n 124
	dey
	Jpl *-6
.elseif(n = 7468)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7471)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7472)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7474)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7475)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7477)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7480)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7481)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7483)
	; 9 bytes
	ldy #57
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 7485)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7489)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7492)
	; 10 bytes
	ldx #(n-682)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7496)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #165
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7498)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7499)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
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
	; 10 bytes
	ldx #125
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif(n = 7502)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #35
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7503 || n = 7534)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7504)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7505)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7506 || n = 7508 || n = 7510 || n = 7512 || n = 7514 || n = 7520 || n = 7522 || n = 7524 || n = 7530 || n = 7532 || n = 7535 || n = 7536 || n = 7538 || n = 7539)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7507)
	; 9 bytes
	ldx #139
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7509 || n = 7517)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7511)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7513)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7515)
	; 9 bytes
	ldx #26
	delay_n 284
	dex
	Jne *-5
.elseif(n = 7516)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7518)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #18
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 7519)
	; 10 bytes
	ldx #41
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 7521)
	; 10 bytes
	ldx #(n-377)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7523)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #37
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 7525)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7526)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7527)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7528)
	; 10 bytes
	ldx #193
	delay_n 34
	dex
	Jne *-6
.elseif(n = 7529)
	; 10 bytes
	ldx #(n-1)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7531)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7533)
	; 10 bytes
	ldx #(n-1077)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7537)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 7540 to 7578
.if(n = 7540 || n = 7542 || n = 7544 || n = 7546 || n = 7548 || n = 7550 || n = 7552 || (n > 7554 && n < 7559) || n = 7560 || n = 7562 || n = 7564 || n = 7566 || n = 7571 || n = 7572 || n = 7577 || n = 7578)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7541)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7543)
	; 10 bytes
	ldx #184 ;hides 'clv'
	delay_n 34
	dex
	Jne *-7
.elseif(n = 7545)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7547)
	; 10 bytes
	ldx #(n-687)/686
	ldy #136
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7549)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7551)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7553)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7554)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7559)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #10
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 7561)
	; 9 bytes
	ldx #140
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7563)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7565)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7567)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7568)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7569)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7570)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7573)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 7575)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7576)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 7579) && (n < 7618))
.if((n >= 7579) && (n < 7598))
; 7579 to 7597
.if(n = 7579)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7580)
	; 9 bytes
	ldx #10
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 7581)
	; 10 bytes
	ldx #4
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 7582)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7583)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7584 || n = 7587 || n = 7590 || n = 7592 || n = 7594 || n = 7595)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7585)
	; 10 bytes
	ldx #(n-97)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7586)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7588)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7589)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7591)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7593)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7596)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7597)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 7598 to 7617
.if(n = 7598)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7599)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7600)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7601)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7602)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7603)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7604 || n = 7605 || n = 7607 || n = 7608 || n = 7610 || n = 7616)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7606)
	; 10 bytes
	ldx #195
	delay_n 34
	dex
	Jne *-6
.elseif(n = 7609)
	; 10 bytes
	ldx #(n-1)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7611)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7612)
	; 9 bytes
	ldy #58
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 7613)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7614)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7615)
	; 9 bytes
	ldx #141
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7617)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
; 7618 to 7656
.if(n = 7618 || n = 7625 || (n > 7627 && n < 7631) || n = 7632 || (n > 7633 && n < 7637) || n = 7640 || (n > 7641 && n < 7645) || (n > 7645 && n < 7649) || n = 7650 || n = 7652 || n = 7654 || n = 7656)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7619)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7620)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7621)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7622)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 684
	inx
	inx
	Jne *-7
.elseif(n = 7623)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7624)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7626)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7627)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7631)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 7633)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7637)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7638)
	; 10 bytes
	ldx #(n-1092)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7639)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7641)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7645)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7649)
	; 10 bytes
	ldx #(n-1)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7651)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7653)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 171
	dey
	Jmi *-7
.elseif(n = 7655)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
; 7657 to 7695
.if(n = 7657)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7658 || n = 7689)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7659 || n = 7662 || n = 7665 || n = 7667 || n = 7670 || n = 7672 || n = 7674 || (n > 7677 && n < 7681) || n = 7682 || n = 7686 || n = 7688 || (n > 7689 && n < 7693) || n = 7694 || n = 7695)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7660)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7661)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7663)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #190
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7664)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 66
	dey
	Jmi *-7
.elseif(n = 7666)
	; 10 bytes
	ldx #(n-1)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7668)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7669)
	; 9 bytes
	ldx #142
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7671)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7673)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7675)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 31
	dey
	Jne *-7
.elseif(n = 7676)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #169
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7677)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7681)
	; 9 bytes
	ldy #19
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 7683)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7684)
	; 10 bytes
	ldx #(n-592)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7685)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7687)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7693)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
.if((n >= 7696) && (n < 7715))
; 7696 to 7714
.if(n = 7696)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7697)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7698)
	; 10 bytes
	ldx #42
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 7699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #60
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 7700 || n = 7701 || n = 7704 || n = 7706 || n = 7710 || n = 7714)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7702)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7703)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #191
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7705)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7707)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #35
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 7708)
	; 10 bytes
	ldx #(n-1102)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7709)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7711)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7712)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7713)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 7715 to 7734
.if(n = 7715)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7716 || n = 7718 || n = 7722 || n = 7726 || (n > 7730 && n < 7735))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7717)
	; 10 bytes
	ldx #<((n-1)/1286)
	ldy #>((n-1)/1286)
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7719)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #37
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7720)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7721)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7723)
	; 9 bytes
	ldx #143
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7724)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 96
	dey
	Jmi *-7
.elseif(n = 7725)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7727)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 7728)
	; 9 bytes
	ldy #186 ;hides 'tsx'
	delay_n 124
	dey
	Jmi *-6
.elseif(n = 7729)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7730)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 7735) && (n < 7774))
; 7735 to 7773
.if(n = 7735 || n = 7737 || n = 7738 || n = 7742 || n = 7744 || n = 7746 || n = 7747 || n = 7752 || n = 7754 || n = 7755 || n = 7757 || n = 7758 || n = 7760 || n = 7763 || n = 7764 || n = 7768 || n = 7770 || n = 7772)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7736)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7739)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7740)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7741)
	; 9 bytes
	ldy #60
	delay_n 124
	dey
	Jne *-5
.elseif(n = 7743)
	; 10 bytes
	ldx #(n-1107)/1106
	ldy #220
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7745)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7748)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7749)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7750)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7751)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7753)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7756)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7759)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7761)
	; 10 bytes
	ldx #(n-777)/776
	ldy #154
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7762)
	; 10 bytes
	ldx #199
	delay_n 34
	dex
	Jne *-6
.elseif(n = 7765)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #69
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7766)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #171
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7767)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7769)
	; 10 bytes
	ldx #(n-1)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7771)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7773)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 7774 to 7812
.if(n = 7774 || n = 7780 || n = 7781 || (n > 7785 && n < 7789) || n = 7790 || n = 7793 || n = 7794 || (n > 7795 && n < 7801) || n = 7802 || n = 7805 || n = 7807 || n = 7812)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7775)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7776)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7777)
	; 9 bytes
	ldx #144
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7778)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7779 || n = 7806)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7782)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7783)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #193
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7784)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7785)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7789)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7791)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7792)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7795)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7801)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7803)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7804)
	; 9 bytes
	ldx #26
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 7808)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7809)
	; 10 bytes
	ldx #8
	ldy #194
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7810)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7811)
	; 10 bytes
	ldx #(n-782)/781
	ldy #155
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
.if((n >= 7813) && (n < 7852))
; 7813 to 7851
.if(n = 7813 || n = 7844)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7814)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7815 || (n > 7816 && n < 7821) || n = 7826 || n = 7828 || n = 7830 || (n > 7833 && n < 7838) || n = 7839 || n = 7842 || n = 7843 || n = 7847 || n = 7850 || n = 7851)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7816)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7821)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7822)
	; 10 bytes
	ldx #(n-712)/711
	ldy #141
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7823)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #194
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 7825)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7827)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7829)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7831)
	; 9 bytes
	ldx #145
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7832)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7833)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7838)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7840)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7841)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7845)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7846)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 126
	dey
	Jmi *-7
.elseif(n = 7848)
	; 10 bytes
	ldx #(n-1122)/1121
	ldy #223
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7849)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
.if((n >= 7852) && (n < 7871))
; 7852 to 7870
.if(n = 7852 || n = 7854 || n = 7859 || n = 7862 || n = 7864)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7853)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7855)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7856)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #173
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7857)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7858)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7860)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7861)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7863)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #195
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7865)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 7866)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7867)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7868)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 176
	dey
	Jmi *-7
.elseif(n = 7869)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7870)
	; 9 bytes
	ldy #61
	delay_n 124
	dey
	Jne *-5
.endif
.else
; 7871 to 7890
.if(n = 7871)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 7872)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7873)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7874 || n = 7876 || n = 7878 || n = 7880 || n = 7881 || n = 7884 || n = 7886 || n = 7888)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7875)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7877)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7879)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7882)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7883)
	; 10 bytes
	ldx #(n-1127)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7885)
	; 9 bytes
	ldx #146
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7887)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 7889)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7890)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if(n = 7891)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif((n > 7891 && n < 7897) || (n > 7897 && n < 7901) || n = 7902 || n = 7904 || n = 7907 || n = 7908 || n = 7910 || (n > 7914 && n < 7918) || n = 7920 || n = 7922 || n = 7924 || (n > 7925 && n < 7929))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7897)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7901)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7903)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #39
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 7905)
	; 10 bytes
	ldx #(n-1)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7906)
	; 10 bytes
	ldx #(n-1)/31
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7909)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 106
	iny
	Jne *-7
.elseif(n = 7911)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7912)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #37
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7913)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7914)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7918)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7919)
	; 10 bytes
	ldx #106
	delay_n 69
	dex
	Jpl *-6
.elseif(n = 7921)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7923)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7925)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7929)
	; 10 bytes
	ldx #(n-1)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 7930 to 7968
.if(n = 7930)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7931)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7932)
	; 10 bytes
	ldx #(n-722)/721
	ldy #143
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7933)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
.elseif((n > 7933 && n < 7937) || n = 7938 || n = 7940 || n = 7942 || n = 7947 || n = 7948 || n = 7950 || n = 7952 || n = 7956 || n = 7958 || n = 7960 || (n > 7961 && n < 7965) || n = 7967 || n = 7968)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7937)
	; 10 bytes
	ldx #0
	ldy #4
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7939)
	; 9 bytes
	ldx #147
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7941)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7943)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7944)
	; 10 bytes
	ldx #(n-612)/611
	ldy #121
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7945)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7946)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #175
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 7949)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 7951)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7953)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7954)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7955)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7957)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7959)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7961)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7965)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7966)
	; 10 bytes
	ldx #(n-1)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 7969) && (n < 8047))
.if((n >= 7969) && (n < 8008))
; 7969 to 8007
.if(n = 7969)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif((n > 7969 && n < 7975) || (n > 7975 && n < 7980) || n = 7982 || n = 7984 || n = 7986 || n = 7989 || n = 7990 || n = 7994 || n = 7998 || n = 8003 || n = 8006)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7975)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7980)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7981)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 7983)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #198
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 7985)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #70
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 7987)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 7988)
	; 10 bytes
	ldx #(n-1142)/1141
	ldy #227
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7991)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 7992)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 7993)
	; 9 bytes
	ldx #148
	delay_n 49
	dex
	Jne *-5
.elseif(n = 7995)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7996)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 7997)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 36
	dey
	Jne *-7
.elseif(n = 7999)
	; 9 bytes
	ldy #62
	delay_n 124
	dey
	Jne *-5
.elseif(n = 8000)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8001)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8002)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8004)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8005)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8007)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-7
.endif
.else
; 8008 to 8046
.if(n = 8008)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8009)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8010 || (n > 8011 && n < 8017) || n = 8018 || n = 8022 || n = 8027 || n = 8028 || n = 8030 || n = 8032 || n = 8034 || n = 8039 || n = 8040 || (n > 8042 && n < 8047))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8011)
	; 10 bytes
	ldx #(n-802)/801
	ldy #159
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8017)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8019)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8020)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8021)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8023)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8024)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8025)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8026)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8029)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8031)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8033)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8035)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8036)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #177
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8037)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8038)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8041)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8042)
	; 10 bytes
	ldx #(n-732)/731
	ldy #145
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 8047) && (n < 8086))
; 8047 to 8085
.if(n = 8047)
	; 9 bytes
	ldx #149
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8048 || n = 8050 || n = 8052 || n = 8054 || n = 8055 || n = 8060 || n = 8062 || n = 8064 || n = 8066 || (n > 8067 && n < 8074) || (n > 8074 && n < 8078) || n = 8079 || n = 8080 || n = 8082 || n = 8084)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8049)
	; 10 bytes
	ldx #(n-1)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8051)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8053)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8056)
	; 10 bytes
	ldx #45
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 8057)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8058)
	; 10 bytes
	ldx #(n-1152)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8059)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8061)
	; 10 bytes
	ldx #(n-807)/806
	ldy #160
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8063)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 8065)
	; 9 bytes
	ldy #21
	delay_n 379
	dey
	Jne *-5
.elseif(n = 8067)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8074)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8078)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8081)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8083)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 181
	dey
	Jmi *-7
.elseif(n = 8085)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 8086 to 8125
.if(n = 8086)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8087)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8088 || n = 8090 || n = 8091 || n = 8094 || n = 8096 || n = 8098 || n = 8102 || n = 8105 || n = 8106 || n = 8109 || n = 8115 || n = 8118 || n = 8120 || n = 8122 || n = 8124 || n = 8125)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8089)
	; 10 bytes
	ldx #(n-1)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8092)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8093)
	; 9 bytes
	ldx #27
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 8095)
	; 10 bytes
	ldx #(n-1)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8097)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8099)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 101
	dey
	Jmi *-7
.elseif(n = 8100)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8101)
	; 9 bytes
	ldx #150
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8103)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8104)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8107)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8108)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8110)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8111)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8112)
	; 10 bytes
	ldy #248 ;hides 'sed'
	delay_n 124
	dey
	dey
	Jmi *-7
.elseif(n = 8113)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8114)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8116)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8117)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #37
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 8119)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8121)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8123)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
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
.if(n = 8126)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #179
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8127 || n = 8132 || n = 8134 || n = 8136 || n = 8138 || n = 8140 || n = 8142 || n = 8144 || n = 8145 || n = 8147 || n = 8148 || n = 8154 || n = 8156 || n = 8158 || n = 8162 || n = 8164)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8128)
	; 9 bytes
	ldy #63
	delay_n 124
	dey
	Jne *-5
.elseif(n = 8129)
	; 10 bytes
	ldx #(n-1)/1016
	ldy #202
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8130 || n = 8157)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8131)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8133)
	; 10 bytes
	ldx #(n-77)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8135)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8137)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8139)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8141)
	; 10 bytes
	ldx #4
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 8143)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8146)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8149)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8150)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8151)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8152)
	; 10 bytes
	ldx #(n-742)/741
	ldy #147
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8153)
	; 9 bytes
	ldy #7
	delay_n 1014
	dey
	Jpl *-5
.elseif(n = 8155)
	; 9 bytes
	ldx #151
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8159)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 41
	dey
	Jne *-7
.elseif(n = 8160)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8161)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8163)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 8165 to 8203
.if(n = 8165 || n = 8168 || n = 8170 || n = 8172 || n = 8174 || n = 8176 || n = 8180 || n = 8181 || n = 8184 || (n > 8185 && n < 8189) || n = 8192 || n = 8194 || n = 8196 || n = 8197 || n = 8202)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8166)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8167)
	; 9 bytes
	ldy #248 ;hides 'sed'
	delay_n 1014
	iny
	Jne *-6
.elseif(n = 8169)
	; 10 bytes
	ldx #(n-1)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8171)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8173)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8175)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8177)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8178)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8179)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8182)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8183)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #16
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 8185)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8189)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8190)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8191)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8193)
	; 10 bytes
	ldx #>(n+48)
	ldy #<(n+48)
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8195)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8198)
	; 10 bytes
	ldx #(n-1172)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8199)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 8200)
	; 10 bytes
	ldx #(n-912)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8201)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8203)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 284
	dex
	dex
	Jne *-7
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
; 8204 to 8242
.if(n = 8204)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8205)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8206 || n = 8208 || n = 8210 || n = 8213 || n = 8218 || n = 8220 || n = 8222 || (n > 8223 && n < 8227) || n = 8228 || n = 8232 || n = 8234 || (n > 8237 && n < 8241))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8207)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8209)
	; 9 bytes
	ldx #152
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8211)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8212)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8214)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8215)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8216)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #181
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8217)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8219)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 1637
	dey
	Jmi *-7
.elseif(n = 8221)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8223)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #204
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8227)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #12
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8229)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8230)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8231)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 8233)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8235)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8236)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8237)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8241)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8242)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 8243 to 8281
.if(n = 8243 || n = 8244 || n = 8246 || n = 8250 || n = 8252 || n = 8253 || n = 8255 || n = 8256 || n = 8258 || n = 8260 || n = 8264 || n = 8267 || n = 8270 || n = 8272 || n = 8274 || (n > 8276 && n < 8281))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8245 || n = 8281)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8247)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8248)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #21
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8249)
	; 10 bytes
	ldx #(n-1)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8251)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8254)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8257)
	; 9 bytes
	ldy #63
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 8259)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 111
	iny
	Jne *-7
.elseif(n = 8261)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8262)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8263)
	; 9 bytes
	ldx #153
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8265 || n = 8271)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8266)
	; 10 bytes
	ldx #(n-1)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8268)
	; 10 bytes
	ldx #(n-1182)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8269)
	; 9 bytes
	ldx #12
	delay_n 684
	dex
	Jne *-5
.elseif(n = 8273)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8275)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8276)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 8282) && (n < 8360))
.if((n >= 8282) && (n < 8321))
; 8282 to 8320
.if(n = 8282)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8283)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8284)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8285)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8286 || n = 8288 || n = 8291 || n = 8292 || n = 8294 || n = 8296 || n = 8300 || n = 8302 || n = 8307 || n = 8309 || n = 8310 || n = 8312 || n = 8313 || n = 8316 || n = 8318)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8287)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 8289)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8290)
	; 10 bytes
	ldx #(n-922)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8293)
	; 10 bytes
	ldx #(n-1)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8295)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8297)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8298)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 186
	dey
	Jmi *-7
.elseif(n = 8299)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8301)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8303)
	; 10 bytes
	ldx #(n-1187)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8304)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8305)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8306)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8308)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8311)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8314)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8315)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #73
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 8317)
	; 9 bytes
	ldx #154
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8319)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8320)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 8321 to 8359
.if(n = 8321)
	; 10 bytes
	ldx #(n-417)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8322)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #39
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8323)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8324)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8325 || n = 8328 || (n > 8329 && n < 8334) || n = 8336 || n = 8339 || n = 8340 || n = 8342 || n = 8346 || n = 8349 || n = 8352 || (n > 8353 && n < 8358) || n = 8359)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8326)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8327)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8329)
	; 10 bytes
	ldx #(n-1)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8334)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8335)
	; 10 bytes
	ldx #(n-927)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8337)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8338)
	; 10 bytes
	ldx #(n-1192)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8341)
	; 10 bytes
	ldx #(n-1)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8343)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8344)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8345)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8347)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8348)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8350)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8351)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 8353)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8358)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 8360) && (n < 8399))
; 8360 to 8398
.if(n = 8360 || n = 8362 || n = 8364 || (n > 8365 && n < 8369) || n = 8370 || n = 8374 || (n > 8375 && n < 8379) || n = 8381 || n = 8387 || (n > 8389 && n < 8396) || n = 8397)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8361)
	; 10 bytes
	ldx #9
	ldy #166
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8363)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8365)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8369)
	; 10 bytes
	ldx #(n-1)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8371)
	; 9 bytes
	ldx #155
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8372)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8373)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8375)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8379)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8380)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8382)
	; 9 bytes
	ldx #29
	delay_n 284
	dex
	Jne *-5
.elseif(n = 8383)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8384)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8385)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8386)
	; 9 bytes
	ldy #64
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 8388)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8389)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8396)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #185
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8398)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
.if((n >= 8399) && (n < 8418))
; 8399 to 8417
.if(n = 8399)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8400 || n = 8403 || n = 8404 || n = 8407 || n = 8410 || n = 8412)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8401)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8402)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8405)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8406)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8408)
	; 10 bytes
	ldx #(n-1202)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8409)
	; 10 bytes
	ldx #(n-1)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8411)
	; 10 bytes
	ldx #(n-842)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8413)
	; 10 bytes
	ldx #(n-1)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8414)
	; 10 bytes
	ldx #46
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 8415)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8416)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8417)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 8418 to 8437
.if((n .mod 2 = 0) && (n = 8418 || n = 8420 || (n > 8423 && n < 8435)))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8419)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8421)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8422)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8423)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #209
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8425)
	; 9 bytes
	ldx #156
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8427)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8429)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8431)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8433)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8435)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 8436)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8437)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
; 8438 to 8476
.if(n = 8438 || n = 8439 || n = 8442 || (n > 8443 && n < 8447) || n = 8448 || (n > 8451 && n < 8457) || (n > 8457 && n < 8461) || n = 8462 || n = 8466 || n = 8468 || n = 8472 || n = 8476)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8440)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8441)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8443)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8447)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8449)
	; 9 bytes
	ldy #21
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 8450)
	; 10 bytes
	ldx #(n-72)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8451)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8457)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8461)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8463)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #210
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8464)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8465)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8467)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8469)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8470)
	; 10 bytes
	ldx #(n-942)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8471)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8473)
	; 10 bytes
	ldx #(n-1)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8474)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 106
	dey
	Jmi *-7
.elseif(n = 8475)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 8477 to 8515
.if(n = 8477)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8478)
	; 10 bytes
	ldx #(n-1212)/1211
	ldy #241
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8479)
	; 9 bytes
	ldx #157
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8480)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8481)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8482)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8483 || n = 8484 || n = 8487 || n = 8490 || n = 8492 || n = 8494 || (n > 8497 && n < 8503) || n = 8504 || n = 8505 || n = 8507 || n = 8509 || n = 8510 || n = 8512 || n = 8514)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8485)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8486)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #187
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8488)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8489)
	; 10 bytes
	ldx #(n-1)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8491)
	; 9 bytes
	ldy #234 ;hides 'nop'
	delay_n 379
	iny
	Jne *-6
.elseif(n = 8493)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8495)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #21
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 8496)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 284
	dex
	dex
	Jpl *-7
.elseif(n = 8497)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8503)
	; 10 bytes
	ldx #218
	delay_n 34
	dex
	Jne *-6
.elseif(n = 8506)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8508)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8511)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8513)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8515)
	; 9 bytes
	ldy #65
	delay_n 124
	dey
	Jpl *-5
.endif
.endif
.else
.if((n >= 8516) && (n < 8555))
; 8516 to 8554
.if(n = 8516)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8517)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8518)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8519)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #40
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 8520 || (n > 8521 && n < 8526) || n = 8528 || n = 8530 || n = 8532 || n = 8536 || (n > 8537 && n < 8541) || n = 8544 || n = 8546 || n = 8547 || n = 8549 || n = 8550 || (n > 8551 && n < 8555))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8521)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8526)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8527)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8529)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8531)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 151
	iny
	Jne *-7
.elseif(n = 8533)
	; 9 bytes
	ldx #158
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8534)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8535)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8537)
	; 10 bytes
	ldx #(n-777)/776
	ldy #154
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8541)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8542)
	; 10 bytes
	ldx #219
	delay_n 34
	dex
	Jne *-6
.elseif(n = 8543)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #212
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8545)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8548)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8551)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 8555 to 8593
.if(n = 8555)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8556 || n = 8558 || n = 8559 || (n > 8561 && n < 8566) || n = 8568 || (n > 8570 && n < 8574) || n = 8575 || (n > 8577 && n < 8581) || n = 8582 || n = 8584 || n = 8588 || n = 8590)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8557)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8560)
	; 10 bytes
	ldx #(n-952)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8561)
	; 10 bytes
	ldx #(n-857)/856
	ldy #170
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8566)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8567)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 8569)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8570)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 8576)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #189
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8577)
	; 10 bytes
	ldx #(n-1)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8581)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8583)
	; 10 bytes
	ldx #(n-1227)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8585)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8586)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8587)
	; 9 bytes
	ldx #159
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8589)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8591)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 8592)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8593)
	; 10 bytes
	ldx #(n-1)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
.if((n >= 8594) && (n < 8633))
; 8594 to 8632
.if(n = 8594)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8595 || n = 8597 || n = 8598 || n = 8600 || n = 8604 || n = 8606 || n = 8610 || n = 8612 || n = 8616 || n = 8617 || n = 8619 || n = 8622 || n = 8624 || (n > 8625 && n < 8631))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8596)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #6
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 8601)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8602)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8603)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8605)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8607)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8608)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8609)
	; 10 bytes
	ldx #(n-1)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8611)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8613)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #21
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 8614)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8615)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8618)
	; 10 bytes
	ldx #(n-1232)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8620)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8621)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8623)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #18
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8625)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8631)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #15
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 8632)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 8633 to 8671
.if(n = 8633)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif((n > 8633 && n < 8637) || n = 8638 || n = 8640 || n = 8642 || n = 8648 || (n > 8653 && n < 8657) || n = 8658 || n = 8660 || n = 8662 || n = 8664 || n = 8667 || n = 8669 || n = 8670)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8637)
	; 10 bytes
	ldx #16
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 8639)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 153
	iny
	Jne *-7
.elseif(n = 8641)
	; 9 bytes
	ldx #160
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8643)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8644)
	; 9 bytes
	ldy #66
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 8645)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #76
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 8646)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8647)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8649)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8650)
	; 10 bytes
	ldx #(n-962)/961
	ldy #191
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8651)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8652)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8653)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8657)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8659)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8661)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8663)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8665)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8666)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #191
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8668)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8671)
	; 9 bytes
	ldx #30
	delay_n 284
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 8672) && (n < 8711))
; 8672 to 8710
.if(n = 8672 || (n > 8673 && n < 8677) || (n > 8677 && n < 8681) || n = 8682 || n = 8683 || n = 8686 || n = 8690 || n = 8692 || n = 8694 || n = 8696 || n = 8700 || n = 8704 || (n > 8705 && n < 8709) || n = 8710)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8673)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8677)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8681)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8684)
	; 10 bytes
	ldx #136 ;hides 'dey'
	delay_n 958
	dex
	Jmi *-7
.elseif(n = 8685)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 36
	dey
	Jne *-7
.elseif(n = 8687)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8688)
	; 10 bytes
	ldx #(n-1242)/1241
	ldy #247
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8689)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8691)
	; 10 bytes
	ldx #55
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif(n = 8693)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8695)
	; 9 bytes
	ldx #161
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8697)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8698)
	; 10 bytes
	ldx #223
	delay_n 34
	dex
	Jne *-6
.elseif(n = 8699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 8701)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8702)
	; 10 bytes
	ldx #(n-792)/791
	ldy #157
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8703)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #216
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8705)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8709)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 8711 to 8750
.if(n = 8711)
	; 10 bytes
	ldx #(n-872)/871
	ldy #173
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8712)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8713)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8714)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 76
	dey
	Jmi *-7
.elseif(n = 8715 || n = 8717 || n = 8718 || (n > 8724 && n < 8728) || n = 8735 || n = 8736 || n = 8738 || n = 8739 || n = 8742 || (n > 8743 && n < 8749) || n = 8750)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8716)
	; 10 bytes
	ldx #(n-1)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8719)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #42
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8720)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 8721)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8722)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8723)
	; 10 bytes
	ldx #(n-1247)/1246
	ldy #248
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8724)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8728)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 196
	dey
	Jmi *-7
.elseif(n = 8729)
	; 10 bytes
	ldx #(n-1)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8730)
	; 10 bytes
	ldx #(n-1)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8731)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8732)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #40
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 8733)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8734)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8737)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8740)
	; 10 bytes
	ldx #(n-972)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8741)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8743)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8749)
	; 9 bytes
	ldx #162
	delay_n 49
	dex
	Jne *-5
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
.if(n = 8751)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 8751 && n < 8755) || n = 8762 || n = 8763 || (n > 8765 && n < 8769) || n = 8771 || n = 8774 || n = 8777 || (n > 8779 && n < 8783) || n = 8784)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8755)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #77
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 8756)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #193
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8757)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8758)
	; 10 bytes
	ldx #(n-1252)/1251
	ldy #249
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8759)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8760)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8761)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8764)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8765)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8769)
	; 10 bytes
	ldx #(n-1)/1096
	ldy #218
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8770)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8772)
	; 10 bytes
	ldx #48
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 8773)
	; 9 bytes
	ldy #68
	delay_n 124
	dey
	Jne *-5
.elseif(n = 8775)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8776)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8778)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8779)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8783)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #218
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8785)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8786)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8787)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8788)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8789)
	; 10 bytes
	ldx #(n-677)/676
	ldy #134
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 8790 to 8828
.if(n = 8790 || n = 8792 || (n > 8793 && n < 8799) || n = 8800 || n = 8802 || n = 8804 || n = 8806 || n = 8808 || n = 8810 || n = 8813 || n = 8814 || (n > 8817 && n < 8821) || n = 8822 || (n > 8824 && n < 8828))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8791)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8793)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8799)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8801)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8803)
	; 9 bytes
	ldx #163
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8805)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8807)
	; 10 bytes
	ldx #118
	delay_n 69
	dex
	Jpl *-6
.elseif(n = 8809)
	; 10 bytes
	ldx #(n-1)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8811)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8812)
	; 10 bytes
	ldx #(n-802)/801
	ldy #159
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8815)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8816)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8817)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8821)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8823)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8824)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8828)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 8829) && (n < 8868))
.if((n >= 8829) && (n < 8848))
; 8829 to 8847
.if(n = 8829 || (n > 8835 && n < 8839) || n = 8840 || n = 8843 || n = 8844)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8830)
	; 10 bytes
	ldx #(n-982)/981
	ldy #195
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8831)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 8832)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #13
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8833)
	; 9 bytes
	ldy #22
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 8834)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8835)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8839)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8841)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8842)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8845)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8846)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8847)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-7
.endif
.else
; 8848 to 8867
.if(n = 8848 || (n > 8849 && n < 8853) || n = 8860 || n = 8862 || n = 8864)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8849)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8853)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #45
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8854)
	; 10 bytes
	ldx #(n-682)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8855)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8856)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8857)
	; 9 bytes
	ldx #164
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8858)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8859)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8861)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8863)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8865)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #79
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8866)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8867)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
; 8868 to 8906
.if(n = 8868 || n = 8869 || n = 8871 || n = 8872 || n = 8874 || n = 8877 || n = 8878 || n = 8882 || n = 8883 || (n > 8884 && n < 8889) || n = 8890 || n = 8892 || (n > 8893 && n < 8897) || n = 8899 || n = 8900 || n = 8906)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8870)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8873)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 49
	dex
	dex
	Jne *-7
.elseif(n = 8875)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8876)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8879)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8880)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #15
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8881)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8884)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8889)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8891)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 8893)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8897)
	; 10 bytes
	ldx #(n-1)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8898)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8901)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8902)
	; 9 bytes
	ldy #69
	delay_n 124
	dey
	Jne *-5
.elseif(n = 8903)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 8904)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8905)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 8907) && (n < 8985))
.if((n >= 8907) && (n < 8946))
; 8907 to 8945
.if(n = 8907)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8908 || n = 8912 || (n > 8913 && n < 8918) || n = 8923 || n = 8924 || n = 8928 || n = 8930 || n = 8931 || n = 8934 || n = 8935 || n = 8940 || n = 8942)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8909)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8910)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8911)
	; 9 bytes
	ldx #165
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8913)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8918)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8919)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8920)
	; 10 bytes
	ldx #(n-992)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8921)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8922)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8925)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8926)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8927)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 41
	dey
	Jne *-7
.elseif(n = 8929)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8932)
	; 10 bytes
	ldx #229
	delay_n 34
	dex
	Jne *-6
.elseif(n = 8933)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8936)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #197
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8937)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #42
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8938)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8939)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8941)
	; 10 bytes
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8943)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8944)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8945)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 8946 to 8984
.if(n = 8946 || n = 8948 || n = 8952 || n = 8954 || n = 8956 || n = 8957 || n = 8963 || n = 8964 || n = 8966 || n = 8970 || (n > 8971 && n < 8975) || n = 8976 || n = 8979 || n = 8982)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8947)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8949)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 8950)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8951)
	; 10 bytes
	ldx #50
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 8953)
	; 10 bytes
	ldx #(n-1)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8955)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8958)
	; 9 bytes
	ldx #12
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 8959)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 121
	iny
	Jne *-7
.elseif(n = 8960)
	; 9 bytes
	ldx #31
	delay_n 284
	dex
	Jne *-5
.elseif(n = 8961)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8962 || n = 8967)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8965)
	; 9 bytes
	ldx #166
	delay_n 49
	dex
	Jne *-5
.elseif(n = 8968)
	; 10 bytes
	ldx #(n-1282)/1281
	ldy #255
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8969)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8971)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8975)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8977)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8978)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8980)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8981)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #198
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 8983)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #223
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 8984)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 8985) && (n < 9024))
; 8985 to 9023
.if((n > 8984 && n < 8989) || n = 8991 || n = 8995 || n = 8996 || (n > 8997 && n < 9001) || n = 9002 || n = 9004 || n = 9006 || n = 9012 || n = 9014 || n = 9015)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8989)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 8990)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 8992)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8993)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 8994)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 8997)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9001)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9003)
	; 10 bytes
	ldx #<((n-1287)/1286)
	ldy #>((n-1287)/1286)
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9005)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 9007)
	; 10 bytes
	ldx #56
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 9008)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 684
	dex
	dex
	Jpl *-7
.elseif(n = 9009)
	; 10 bytes
	ldx #(n-1)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9010)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9011)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9013)
	; 10 bytes
	ldx #(n-1)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9016)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9017)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9018)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9019)
	; 9 bytes
	ldx #167
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9020)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9021)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9022)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9023)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #224
	dey
	Jne *-1
	inx
	Jne *-7
.endif
.else
; 9024 to 9062
.if(n = 9024 || n = 9027 || n = 9030 || (n > 9033 && n < 9037) || n = 9038 || n = 9039 || n = 9042 || n = 9044 || n = 9047 || n = 9048 || n = 9050 || n = 9054 || n = 9056 || (n > 9057 && n < 9061))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9025)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9026)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9028)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9029)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9031)
	; 9 bytes
	ldy #69
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 9032)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9033)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9037)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9040)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9041)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9043)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9045)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9046)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9049)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9051)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9052)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9053)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9055)
	; 10 bytes
	ldx #(n-1007)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9057)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9061)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9062)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
.if((n >= 9063) && (n < 9102))
; 9063 to 9101
.if(n = 9063)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9064)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9065)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif((n > 9065 && n < 9071) || n = 9072 || n = 9077 || n = 9078 || (n > 9079 && n < 9085) || n = 9086 || (n > 9091 && n < 9097) || n = 9098 || n = 9099)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9071)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #73
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 9073)
	; 9 bytes
	ldx #168
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9074)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9075)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9076)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9079)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9085)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #81
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9087)
	; 10 bytes
	ldx #(n-827)/826
	ldy #164
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9088)
	; 10 bytes
	ldx #233
	delay_n 34
	dex
	Jne *-6
.elseif(n = 9089)
	; 10 bytes
	ldx #8
	ldy #226
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9090)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9091)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9097)
	; 10 bytes
	ldx #6
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	dex
	Jne *-6
.elseif(n = 9100)
	; 10 bytes
	ldx #(n-1012)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9101)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 9102 to 9140
.if(n = 9102 || (n > 9103 && n < 9109) || n = 9110 || n = 9112 || n = 9118 || n = 9120 || (n > 9121 && n < 9127) || n = 9128 || n = 9132 || n = 9134 || n = 9135 || n = 9138 || n = 9139)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9103)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9109)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9111)
	; 10 bytes
	ldx #(n-912)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9113)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9114)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9115)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9116)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #201
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9117)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9119)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-7
.elseif(n = 9121)
	; 10 bytes
	ldx #(n-97)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9127)
	; 9 bytes
	ldx #169
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9129)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9130)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9131)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9133)
	; 10 bytes
	ldx #(n-1)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9136)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9137)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9140)
	; 10 bytes
	ldx #(n-1)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if(n = 9141 || (n > 9145 && n < 9151) || n = 9152 || n = 9156 || n = 9163 || n = 9164 || n = 9167 || n = 9168 || n = 9170 || n = 9173 || n = 9176)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9142)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9143)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #227
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9144)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9145)
	; 10 bytes
	ldx #(n-1)/36
	ldy #6
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9151)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9153)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9154)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9155)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9157)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9158)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 206
	dey
	Jmi *-7
.elseif(n = 9159)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9160)
	; 9 bytes
	ldy #70
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 9161)
	; 10 bytes
	ldx #(n-917)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9162 || n = 9174)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9165)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9166)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9169)
	; 9 bytes
	ldy #186 ;hides 'tsx'
	delay_n 124
	iny
	Jne *-6
.elseif(n = 9171)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9172)
	; 9 bytes
	ldy #8
	delay_n 1014
	dey
	Jpl *-5
.elseif(n = 9175)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9177)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9178)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9179)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 9180 to 9218
.if(n = 9180 || n = 9182 || n = 9184 || (n > 9185 && n < 9190) || n = 9194 || n = 9196 || n = 9200 || n = 9202 || n = 9204 || n = 9207 || n = 9210 || n = 9216 || n = 9218)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9181)
	; 9 bytes
	ldx #170
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9183)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9185)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9190)
	; 10 bytes
	ldx #(n-1022)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9191)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 9192)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9193)
	; 10 bytes
	ldx #(n-1)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9195)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #82
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9197)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9198)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9199)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9201)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9203)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9205)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9206)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #203
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9208)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9209)
	; 10 bytes
	ldx #(n-1)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9211)
	; 10 bytes
	ldx #(n-922)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9212)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9213)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9214)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9215)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9217)
	; 9 bytes
	ldy #23
	delay_n 379
	dey
	Jpl *-5
.endif
.endif
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
; 9219 to 9257
.if(n = 9219 || n = 9237 || n = 9251)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9220 || n = 9222 || n = 9225 || (n > 9226 && n < 9231) || n = 9234 || n = 9236 || n = 9238 || n = 9240 || n = 9242 || n = 9243 || n = 9245 || n = 9246 || n = 9248 || n = 9250 || n = 9254 || n = 9256)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9221)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9223)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9224)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9226)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9231)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9232)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9233)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #46
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 9235)
	; 9 bytes
	ldx #171
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9239)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 81
	dey
	Jmi *-7
.elseif(n = 9241)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9244)
	; 10 bytes
	ldx #(n-712)/711
	ldy #141
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9247)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9249)
	; 9 bytes
	ldx #32
	delay_n 284
	dex
	Jne *-5
.elseif(n = 9252)
	; 10 bytes
	ldx #(n-842)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9253)
	; 10 bytes
	ldx #(n-1)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9255)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9257)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 9258 to 9296
.if((n > 9257 && n < 9261) || n = 9264 || n = 9266 || n = 9268 || n = 9272 || n = 9275 || (n > 9276 && n < 9280) || n = 9282 || n = 9284 || n = 9286 || n = 9292 || n = 9294 || n = 9295)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9261)
	; 10 bytes
	ldx #(n-927)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9262)
	; 10 bytes
	ldx #(n-442)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9263)
	; 9 bytes
	ldy #24 ;hides 'clc'
	delay_n 379
	dey
	Jne *-6
.elseif(n = 9265)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9267)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9269)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9270)
	; 10 bytes
	ldx #22
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 9271)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9273)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9274)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9276)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9280)
	; 10 bytes
	ldx #(n-1032)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9281)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9283)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9285)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9287)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 101
	iny
	Jne *-7
.elseif(n = 9288)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9289)
	; 9 bytes
	ldy #72
	delay_n 124
	dey
	Jne *-5
.elseif(n = 9290)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 9291)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9293)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9296)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #205
	dey
	Jne *-1
	dex
	Jmi *-7
.endif
.endif
.else
.if((n >= 9297) && (n < 9336))
; 9297 to 9335
.if(n = 9297)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif((n > 9297 && n < 9301) || n = 9304 || n = 9306 || n = 9312 || n = 9314 || n = 9315 || n = 9317 || n = 9320 || (n > 9325 && n < 9329) || n = 9333 || n = 9335)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9301)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9302)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9303)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #231
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9305)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #82
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 9307)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9308)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9309)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9310)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9311)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9313)
	; 10 bytes
	ldx #(n-97)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9316)
	; 10 bytes
	ldx #(n-82)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9318)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #45
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9321)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9322)
	; 10 bytes
	ldx #239
	delay_n 34
	dex
	Jne *-6
.elseif(n = 9323)
	; 10 bytes
	ldx #58
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 9324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 9325)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9329)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9330)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9331)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9332)
	; 10 bytes
	ldx #(n-1)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9334)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 9336 to 9375
.if((n > 9335 && n < 9339) || n = 9340 || n = 9342 || n = 9344 || n = 9346 || (n > 9349 && n < 9353) || (n > 9353 && n < 9359) || (n > 9363 && n < 9367) || n = 9371 || n = 9372)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9339)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9341)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 166
	iny
	Jne *-7
.elseif(n = 9343)
	; 9 bytes
	ldx #173
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9345)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9347)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #44
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9348)
	; 10 bytes
	ldx #164 ;hides 'ldy zp'
	nop
	delay_n 49
	dex
	Jne *-7
.elseif(n = 9349)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9353)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9359)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #13
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 9360)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9361)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9362)
	; 10 bytes
	ldx #(n-852)/851
	ldy #169
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9363)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9367)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9368)
	; 10 bytes
	ldx #(n-552)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9369)
	; 10 bytes
	ldx #(n-1)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9370)
	; 10 bytes
	ldx #(n-1042)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9373)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9374)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9375)
	; 10 bytes
	ldx #(n-87)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.if(n = 9376 || n = 9378 || (n > 9379 && n < 9383) || n = 9384 || n = 9388 || n = 9389 || n = 9392 || n = 9393 || n = 9396 || n = 9398 || (n > 9401 && n < 9405) || n = 9408 || n = 9410 || n = 9412 || n = 9414)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9377)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9379)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9383)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #233
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9385)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9386)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #207
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9387)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9390)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9391)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9394)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9395)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9397)
	; 9 bytes
	ldx #174
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9399)
	; 10 bytes
	ldx #127
	delay_n 69
	dex
	Jne *-6
.elseif(n = 9400)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9401)
	; 10 bytes
	ldx #(n-377)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9405)
	; 10 bytes
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	delay_n 49
	dex
	Jne *-7
.elseif(n = 9406)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9407)
	; 9 bytes
	ldx #168 ;hides 'tay'
	delay_n 49
	dex
	Jne *-6
.elseif(n = 9409)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9411)
	; 10 bytes
	ldx #(n-942)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9413)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 9415 to 9453
.if(n = 9415)
	; 10 bytes
	ldx #(n-1047)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9416 || n = 9419 || n = 9422 || n = 9424 || (n > 9425 && n < 9431) || n = 9432 || n = 9434 || n = 9438 || n = 9440 || n = 9442 || n = 9443 || (n > 9444 && n < 9448) || n = 9450 || n = 9452)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9417)
	; 10 bytes
	ldx #(n-857)/856
	ldy #170
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9418)
	; 9 bytes
	ldy #72
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 9420)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9421)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9423)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #47
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 9425)
	; 10 bytes
	ldx #(n-1)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9431)
	; 9 bytes
	ldy #56 ;hides 'sec'
	delay_n 124
	iny
	Jpl *-6
.elseif(n = 9433)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9435)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9436)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9437)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #13
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 9439)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9441)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9444)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9448)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9449)
	; 10 bytes
	ldx #(n-1)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9451)
	; 9 bytes
	ldx #175
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9453)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 9454) && (n < 9493))
; 9454 to 9492
.if(n = 9454)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9455)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9456)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9457)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9458 || n = 9459 || n = 9462 || n = 9464 || n = 9467 || n = 9468 || n = 9470 || n = 9471 || n = 9474 || n = 9475 || n = 9479 || n = 9480 || n = 9482 || n = 9484 || n = 9485 || n = 9490 || n = 9492)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9460)
	; 10 bytes
	ldx #(n-1052)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9461)
	; 10 bytes
	ldx #9
	ldy #188
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9463)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9465)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9466)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9469)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9472)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9473)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9476)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #209
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9477)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9478)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9481)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9483)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9486)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9487)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9488)
	; 10 bytes
	ldx #53
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 9489)
	; 10 bytes
	ldx #(n-1)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9491)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 9493 to 9531
.if(n = 9493)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9494 || (n > 9496 && n < 9500) || n = 9502 || n = 9506 || (n > 9507 && n < 9511) || n = 9512 || n = 9516 || n = 9518 || n = 9522 || n = 9524 || n = 9528 || n = 9530 || n = 9531)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9495)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9496)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9500)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9501)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9503)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #19
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 9504)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9505)
	; 9 bytes
	ldx #176
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9507)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9511)
	; 10 bytes
	ldx #(n-952)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9513)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9514)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9515)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9517)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9519)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #45
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 9520)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9521)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9523)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9525)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #84
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 9526)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9527)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9529)
	; 10 bytes
	ldx #(n-1)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 9532) && (n < 9610))
.if((n >= 9532) && (n < 9571))
; 9532 to 9570
.if((n > 9531 && n < 9535) || n = 9536 || n = 9542 || (n > 9543 && n < 9547) || n = 9548 || n = 9557 || n = 9558 || n = 9562 || n = 9563 || n = 9565 || n = 9567 || n = 9568 || n = 9570)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9535)
	; 10 bytes
	ldx #(n-1)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9537)
	; 10 bytes
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9538)
	; 9 bytes
	ldx #33
	delay_n 284
	dex
	Jne *-5
.elseif(n = 9539)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9540)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9541)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9543)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #237
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9547)
	; 9 bytes
	ldy #73
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 9549)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9550)
	; 10 bytes
	ldx #(n-1062)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9551)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9552)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #45
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9553)
	; 10 bytes
	ldx #(n-1)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9554)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9555)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9556)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9559)
	; 9 bytes
	ldx #177
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9560)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9561)
	; 10 bytes
	ldx #(n-957)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9564)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9566)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #211
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9569)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 9571 to 9609
.if(n = 9571)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9572 || n = 9573 || n = 9576 || (n > 9578 && n < 9582) || n = 9584 || n = 9585 || n = 9587 || (n > 9589 && n < 9593) || n = 9597 || n = 9598 || n = 9602 || (n > 9605 && n < 9609))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 9575)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #25
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 9577)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9578)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9582)
	; 10 bytes
	ldx #(n-872)/871
	ldy #173
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9583)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #238
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9586)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9588)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 216
	dey
	Jmi *-7
.elseif(n = 9589)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9593)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9594)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9595)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9596)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #7
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 9600)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9601)
	; 9 bytes
	ldy #25
	delay_n 379
	dey
	Jne *-5
.elseif(n = 9603)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9604)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9605)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9609)
	; 10 bytes
	ldx #(n-1)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 9610) && (n < 9649))
; 9610 to 9648
.if(n = 9610 || n = 9612 || n = 9614 || (n > 9617 && n < 9621) || n = 9622 || n = 9624 || (n > 9625 && n < 9629) || n = 9630 || n = 9632 || n = 9638 || (n > 9642 && n < 9647) || n = 9648)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9611)
	; 10 bytes
	ldx #(n-962)/961
	ldy #191
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9613)
	; 9 bytes
	ldx #178
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9615)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9616)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9617)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9621)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9623)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9625)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9629)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9631)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9633)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9634)
	; 10 bytes
	ldx #(n-742)/741
	ldy #147
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9635)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #85
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 9636)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9637)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9639)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9640)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9641)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9642)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9647)
	; 9 bytes
	ldx #14
	delay_n 684
	dex
	Jne *-5
.endif
.else
; 9649 to 9687
.if(n = 9649)
	; 9 bytes
	ldy #24 ;hides 'clc'
	delay_n 379
	dey
	Jpl *-6
.elseif(n = 9650 || n = 9652 || n = 9654 || n = 9655 || n = 9660 || n = 9662 || n = 9664 || n = 9666 || n = 9668 || n = 9670 || n = 9672 || n = 9674 || n = 9679 || n = 9680 || n = 9686)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9651)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9653)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9656)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #213
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9657)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9658)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9659)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 131
	iny
	Jne *-7
.elseif(n = 9661)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9663)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #240
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9665)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9667)
	; 9 bytes
	ldx #179
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9669 || n = 9678)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9671)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #16
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 9673)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9675)
	; 10 bytes
	ldx #(n-1)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9676)
	; 9 bytes
	ldy #74
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 9677)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9681)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9682)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9683)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9684)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9685)
	; 10 bytes
	ldx #(n-1077)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9687)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-7
.endif
.endif
.endif
.endif
.else
.if((n >= 9688) && (n < 9844))
.if((n >= 9688) && (n < 9766))
.if((n >= 9688) && (n < 9727))
; 9688 to 9726
.if(n = 9688 || n = 9690 || n = 9693 || n = 9694 || n = 9696 || n = 9698 || n = 9702 || (n > 9703 && n < 9707) || n = 9709 || (n > 9712 && n < 9717) || n = 9720 || n = 9722 || n = 9724 || n = 9726)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9689)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9691)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9692)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9695)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9697)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9699)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9700)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9701)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #214
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9703)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9707)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9708)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9710)
	; 10 bytes
	ldx #(n-1)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9711)
	; 10 bytes
	ldx #(n-972)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9712)
	; 10 bytes
	ldx #249
	delay_n 34
	dex
	Jne *-6
.elseif(n = 9717)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 106
	iny
	Jne *-7
.elseif(n = 9718)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9719)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9721)
	; 9 bytes
	ldx #180
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9723)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9725)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 9727 to 9765
.if(n = 9727 || n = 9731 || n = 9732 || (n > 9733 && n < 9738) || n = 9740 || n = 9741 || n = 9744 || n = 9749 || n = 9750 || n = 9752 || (n > 9753 && n < 9757) || n = 9758 || n = 9760 || n = 9762 || n = 9763 || n = 9765)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9728)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9729)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9730)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9733)
	; 10 bytes
	ldx #(n-1)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9738)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9739)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9742)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9743)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 9745)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9746)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #215
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9747)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9748)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9751)
	; 10 bytes
	ldx #250
	delay_n 34
	dex
	Jne *-6
.elseif(n = 9753)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9757)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9759)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9761)
	; 10 bytes
	ldx #(n-977)/976
	ldy #194
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9764)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 9766) && (n < 9805))
; 9766 to 9804
.if(n = 9766)
	; 10 bytes
	ldx #(n-1)/651
	ldy #128
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9767 || n = 9768 || (n > 9769 && n < 9773) || n = 9774 || (n > 9777 && n < 9781) || n = 9782 || (n > 9783 && n < 9787) || n = 9788 || n = 9789 || n = 9792 || n = 9794 || n = 9796 || n = 9804)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9769)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9773)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 174
	iny
	Jne *-7
.elseif(n = 9775)
	; 9 bytes
	ldx #181
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9776)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9777)
	; 10 bytes
	ldx #(n-1)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9781)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9783)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9787)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9790)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9791)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #79
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 9793)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9795)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9797)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9798)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9799)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9800)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9801)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9802)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9803)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 221
	dey
	Jmi *-7
.endif
.else
; 9805 to 9843
.if(n = 9805)
	; 9 bytes
	ldy #76
	delay_n 124
	dey
	Jne *-5
.elseif(n = 9806 || n = 9808 || n = 9810 || n = 9812 || n = 9814 || (n > 9816 && n < 9820) || n = 9825 || n = 9828 || n = 9830 || (n > 9831 && n < 9835) || (n > 9836 && n < 9841) || n = 9842)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9807 || n = 9831)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9809)
	; 10 bytes
	ldx #(n-1)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9811)
	; 10 bytes
	ldx #(n-982)/981
	ldy #195
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9813)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9815)
	; 10 bytes
	ldx #(n-1)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9816)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9820)
	; 10 bytes
	ldx #(n-1092)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9821)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9822)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9823)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #244
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9824)
	; 9 bytes
	ldy #202 ;hides 'dex'
	delay_n 124
	dey
	Jmi *-6
.elseif(n = 9826)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9827)
	; 9 bytes
	ldx #34
	delay_n 284
	dex
	Jne *-5
.elseif(n = 9829)
	; 9 bytes
	ldx #182
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9835)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9836)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9841)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9843)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
.if((n >= 9844) && (n < 9883))
; 9844 to 9882
.if(n = 9844)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9845)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9846)
	; 10 bytes
	ldx #55
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 9847 || n = 9848 || n = 9851 || n = 9852 || n = 9854 || n = 9856 || n = 9858 || n = 9862 || n = 9864 || n = 9866 || n = 9867 || n = 9869 || (n > 9871 && n < 9878) || n = 9879)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9849)
	; 10 bytes
	ldx #(n-1)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9850)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9853)
	; 10 bytes
	ldx #(n-1)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9855)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #87
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 9857)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9859)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9860)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 9861)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9863)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #245
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9865)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9868)
	; 10 bytes
	ldx #253
	delay_n 34
	dex
	Jne *-6
.elseif(n = 9870)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9871)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9878)
	; 10 bytes
	ldx #(n-582)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9880)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9881)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9882)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 9883 to 9921
.if(n = 9883)
	; 9 bytes
	ldx #183
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9884 || (n > 9885 && n < 9889) || n = 9893 || (n > 9895 && n < 9899) || n = 9902 || (n > 9903 && n < 9907) || n = 9908 || n = 9909 || n = 9915)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9885)
	; 10 bytes
	ldx #(n-1)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9889)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9890)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9891)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9892)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9894)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9895)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9899)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9900)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9901)
	; 10 bytes
	ldx #(n-397)/396
	ldy #77
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9903)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #246
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9907)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9910)
	; 10 bytes
	ldx #(n-1102)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9911)
	; 10 bytes
	ldx #(n-992)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9912)
	; 10 bytes
	ldx #10
	ldy #179
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9913)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9914)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9916)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9917)
	; 10 bytes
	ldx #134
	delay_n 69
	dex
	Jne *-6
.elseif(n = 9918)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9919)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9920)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9921)
	; 10 bytes
	ldx #(n-497)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 9922) && (n < 9961))
; 9922 to 9960
.if(n = 9922 || n = 9924 || n = 9925 || n = 9928 || n = 9930 || n = 9932 || n = 9938 || n = 9940 || n = 9942 || n = 9947 || n = 9948 || n = 9951 || n = 9954 || n = 9958 || n = 9960)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9923)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9926)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #219
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9927)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 9929)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9931)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9933)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9934)
	; 9 bytes
	ldy #76
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 9935)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #26
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 9936 || n = 9939)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9937)
	; 9 bytes
	ldx #184
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9941)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9943)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #21
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 9944)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9945)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9946)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9949)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #77
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 9950)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #16
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 9952)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9953)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9955)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9956)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9957)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 9959)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 9961 to 10000
.if(n = 9961)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9962)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #46
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 9963)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9964)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9965)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9966 || n = 9968 || n = 9970 || n = 9972 || (n > 9977 && n < 9983) || n = 9984 || n = 9986 || (n > 9987 && n < 9991) || n = 9992 || n = 9994 || n = 9996 || n = 9998 || n = 9999)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9967)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 9969)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9971)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #220
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9973)
	; 10 bytes
	ldx #(n-1)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9974)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 126
	dey
	Jmi *-7
.elseif(n = 9975)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #6
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 9976)
	; 10 bytes
	ldy #6
	delay_n 1420
	dey
	Jpl *-6
.elseif(n = 9977)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9983)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 9985)
	; 9 bytes
	ldy #26
	delay_n 379
	dey
	Jne *-5
.elseif(n = 9987)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9991)
	; 9 bytes
	ldx #185
	delay_n 49
	dex
	Jne *-5
.elseif(n = 9993)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #51
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 9995)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 9997)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10000)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.if((n > 10000 && n < 10005) || (n > 10005 && n < 10009) || n = 10010 || (n > 10012 && n < 10016) || n = 10019 || n = 10020 || n = 10027 || n = 10030 || n = 10032 || n = 10034 || n = 10036 || n = 10038 || n = 10039)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10005)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10009)
	; 10 bytes
	ldx #(n-1)/1251
	ldy #249
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10011)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10012)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10016)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #221
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10017)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10018)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10021)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10022)
	; 10 bytes
	ldx #(n-912)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10023)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #249
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 10024)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10025)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10026)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10028)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10029)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10031)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #82
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 10033)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10035)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10037)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 10040 to 10078
.if(n = 10040 || n = 10043 || n = 10050 || (n > 10051 && n < 10056) || (n > 10056 && n < 10061) || n = 10062 || n = 10064 || (n > 10066 && n < 10071) || n = 10072 || n = 10078)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10041)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10042)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #15
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10044)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10045)
	; 9 bytes
	ldx #186
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10046)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10047)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10048)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10049)
	; 10 bytes
	ldx #(n-1)/1256
	ldy #250
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10051)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10056)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #21
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10061)
	; 10 bytes
	ldx #(n-1007)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10063)
	; 9 bytes
	ldy #77
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 10065)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10066)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10071)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10073)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #26
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 10075)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10076)
	; 10 bytes
	ldx #24
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 10077)
	; 10 bytes
	ldx #(n-917)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 10079) && (n < 10118))
; 10079 to 10117
.if(n = 10079 || n = 10080 || n = 10082 || (n > 10083 && n < 10087) || n = 10091 || n = 10092 || n = 10094 || n = 10096 || n = 10098 || n = 10100 || n = 10104 || n = 10108 || n = 10110 || n = 10112 || n = 10114 || n = 10115)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10081)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10083)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10087)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10088)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10089)
	; 10 bytes
	ldx #8
	ldy #251
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10090)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10093)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10095)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10097)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10099)
	; 9 bytes
	ldx #187
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10101)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10102)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10103)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #251
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 10105)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10106)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #223
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10107)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10109)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10111)
	; 10 bytes
	ldx #(n-1012)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10113)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10116)
	; 9 bytes
	ldx #34
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 10117)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
.if((n >= 10118) && (n < 10137))
; 10118 to 10136
.if(n = 10118)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10119)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #49
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 10120 || n = 10122 || n = 10125 || n = 10130 || n = 10131 || n = 10134 || n = 10136)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10121)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10123)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10124)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10126)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10127)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10128)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10129)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10132)
	; 10 bytes
	ldx #(n-922)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10133)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10135)
	; 10 bytes
	ldx #(n-1127)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 10137 to 10156
.if(n = 10137)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10138 || n = 10140 || n = 10142 || n = 10146 || n = 10148 || n = 10152 || n = 10156)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10139)
	; 10 bytes
	ldx #137
	delay_n 69
	dex
	Jne *-6
.elseif(n = 10141)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10143)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10144)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10145)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 10147)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 111
	iny
	Jne *-7
.elseif(n = 10149)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10150)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10151)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10153)
	; 9 bytes
	ldx #188
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10154)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10155)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.endif
.else
.if((n >= 10157) && (n < 10235))
.if((n >= 10157) && (n < 10196))
; 10157 to 10195
.if((n > 10156 && n < 10161) || (n > 10161 && n < 10165) || n = 10166 || n = 10168 || n = 10170 || (n > 10171 && n < 10177) || n = 10178 || n = 10179 || n = 10181 || n = 10182 || n = 10184 || n = 10190 || n = 10195)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10161)
	; 10 bytes
	ldx #(n-1017)/1016
	ldy #202
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10165)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10167)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10169)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10171)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10177)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10180)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10183)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #52
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10185)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10186)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10187)
	; 10 bytes
	ldx #(n-927)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10188)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10189)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10191)
	; 9 bytes
	ldy #9
	delay_n 1014
	dey
	Jpl *-5
.elseif(n = 10192)
	; 9 bytes
	ldy #79
	delay_n 124
	dey
	Jne *-5
.elseif(n = 10193)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10194)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.else
; 10196 to 10234
.if(n = 10196)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #225
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10197 || n = 10198 || n = 10200 || n = 10203 || n = 10205 || n = 10208 || n = 10212 || n = 10214 || n = 10215 || n = 10217 || n = 10222 || n = 10224 || (n > 10225 && n < 10231) || n = 10232 || n = 10234)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10199)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 10201)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10202)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10204)
	; 10 bytes
	ldx #56
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 10206)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 166
	dey
	Jmi *-7
.elseif(n = 10207)
	; 9 bytes
	ldx #189
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10209)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10210)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10211)
	; 10 bytes
	ldx #(n-1022)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10213)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10216)
	; 10 bytes
	ldx #(n-1)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10218)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10219)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10220)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 284
	dex
	Jne *-7
.elseif(n = 10221)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10223)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #254
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 10225)
	; 10 bytes
	ldx #(n-1)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10231)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10233)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 231
	dey
	Jmi *-7
.endif
.endif
.else
.if((n >= 10235) && (n < 10274))
; 10235 to 10273
.if(n = 10235)
	; 10 bytes
	ldx #(n-1)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif((n > 10235 && n < 10241) || (n > 10242 && n < 10247) || n = 10248 || n = 10250 || (n > 10253 && n < 10257) || n = 10258 || n = 10260 || (n > 10265 && n < 10269) || n = 10272)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10241)
	; 10 bytes
	ldx #>(n+48)
	ldy #<(n+48)
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10242)
	; 10 bytes
	ldx #(n-932)/931
	ldy #185
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10247)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 10249)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10251)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10252)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10253)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10257)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10259)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10261)
	; 9 bytes
	ldx #190
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10262)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10263)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10264)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10265)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 167
	dey
	Jmi *-7
.elseif(n = 10269)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10270)
	; 10 bytes
	ldx #(n-1142)/1141
	ldy #227
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10271)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 10273)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 10274 to 10312
.if(n = 10274 || n = 10275 || n = 10277 || n = 10278 || n = 10281 || n = 10282 || n = 10285 || n = 10290 || n = 10298 || n = 10300 || n = 10302 || n = 10304 || (n > 10305 && n < 10310))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10276)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10279)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10280)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10283)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10284)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10286)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10287)
	; 10 bytes
	ldx #139
	delay_n 69
	dex
	Jne *-6
.elseif(n = 10288)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10289)
	; 10 bytes
	ldx #<((n-1)/1286)
	ldy #>((n-1)/1286)
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10291)
	; 10 bytes
	ldx #15
	ldy #136
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10292)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10293)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10294)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10295)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #27
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 10296)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10297)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10299)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10301)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10303)
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 49
	dex
	Jne *-6
.elseif(n = 10305)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10310)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10311)
	; 10 bytes
	ldx #(n-1032)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10312)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
.if((n >= 10313) && (n < 10391))
.if((n >= 10313) && (n < 10352))
; 10313 to 10351
.if(n = 10313 || n = 10314 || n = 10320 || n = 10322 || (n > 10325 && n < 10331) || n = 10332 || n = 10334 || n = 10339 || (n > 10340 && n < 10345) || (n > 10345 && n < 10349))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10315)
	; 9 bytes
	ldx #191
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10316)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-7
.elseif(n = 10317 || n = 10323 || n = 10338)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10318)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #49
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 10321)
	; 9 bytes
	ldy #79
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 10324)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10325)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10331)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #228
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10333)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10335)
	; 10 bytes
	ldx #136 ;hides 'dey'
	delay_n 69
	dex
	Jne *-7
.elseif(n = 10336)
	; 9 bytes
	ldx #15
	delay_n 684
	dex
	Jne *-5
.elseif(n = 10337)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10340)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10345)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10349)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10350)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10351)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 10352 to 10390
.if(n = 10352)
	; 10 bytes
	ldx #(n-942)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10353 || (n > 10354 && n < 10359) || n = 10362 || n = 10365 || n = 10368 || n = 10370 || (n > 10376 && n < 10381) || n = 10382 || n = 10384 || n = 10386 || n = 10387 || n = 10390)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10354)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10359)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 141
	iny
	Jne *-7
.elseif(n = 10360)
	; 10 bytes
	ldx #(n-1152)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10361)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10363)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10364)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10366)
	; 10 bytes
	ldx #(n-1)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10367)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10369)
	; 9 bytes
	ldy #27
	delay_n 379
	dey
	Jne *-5
.elseif(n = 10371)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10372)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #49
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10373)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #52
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 10374)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10375)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10376)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #229
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10381)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10383)
	; 10 bytes
	ldx #57
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 10385)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10388)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10389)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 10391) && (n < 10430))
; 10391 to 10429
.if(n = 10391)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 10392 || n = 10394 || n = 10395 || n = 10399 || n = 10400 || n = 10402 || n = 10403 || n = 10406 || (n > 10407 && n < 10411) || n = 10412 || n = 10416 || n = 10418 || n = 10419 || n = 10422 || n = 10424 || n = 10426 || n = 10428)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10393)
	; 10 bytes
	ldx #(n-1)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10396)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10397)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10398)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10401)
	; 10 bytes
	ldx #(n-417)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10404)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10405)
	; 9 bytes
	ldx #35
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 10407)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10411)
	; 10 bytes
	ldx #(n-1042)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10413)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10414)
	; 10 bytes
	ldx #(n-802)/801
	ldy #159
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10415)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10417)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10420)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10421)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10423)
	; 9 bytes
	ldx #193
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10425)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10427)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10429)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 10430 to 10468
.if(n = 10430)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 10431 || n = 10434 || n = 10436 || n = 10437 || n = 10440 || n = 10442 || n = 10444 || n = 10446 || n = 10447 || n = 10451 || n = 10454 || n = 10455 || n = 10457 || n = 10458 || n = 10460 || n = 10463 || n = 10464 || n = 10467 || n = 10468)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10432)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10433)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10435)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10438)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10439)
	; 10 bytes
	ldx #180 ;hides 'ldy zp,x'
	nop
	delay_n 49
	dex
	Jne *-7
.elseif(n = 10441)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10443)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10445)
	; 10 bytes
	ldx #(n-1)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10448)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 236
	dey
	Jmi *-7
.elseif(n = 10449)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #82
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 10450)
	; 9 bytes
	ldy #81
	delay_n 124
	dey
	Jne *-5
.elseif(n = 10452)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10453)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10456)
	; 10 bytes
	ldx #(n-1)/41
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10459)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10461)
	; 10 bytes
	ldx #(n-1047)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10462)
	; 10 bytes
	ldx #(n-952)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10465)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10466)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 10469) && (n < 10547))
.if((n >= 10469) && (n < 10508))
; 10469 to 10507
.if(n = 10469 || n = 10471 || n = 10472 || n = 10474 || n = 10476 || n = 10478 || n = 10480 || (n > 10481 && n < 10485) || (n > 10485 && n < 10489) || (n > 10489 && n < 10493) || n = 10494 || n = 10496 || n = 10498 || n = 10500 || (n > 10501 && n < 10505))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10470)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10473)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10475)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 379
	iny
	iny
	Jne *-7
.elseif(n = 10477)
	; 9 bytes
	ldx #194
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10479)
	; 10 bytes
	ldx #(n-807)/806
	ldy #160
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10481)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10485)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #18
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10489)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10493)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10495)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10497)
	; 10 bytes
	ldx #0
	ldy #6
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10499)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10501)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 171
	dey
	Jmi *-7
.elseif(n = 10505)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10506)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10507)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 10508 to 10546
.if(n = 10508 || n = 10510 || n = 10514 || n = 10518 || n = 10520 || (n > 10522 && n < 10526) || n = 10530 || n = 10532 || n = 10534 || n = 10537 || n = 10538 || n = 10541 || n = 10542 || n = 10545)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10509)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10511)
	; 10 bytes
	ldx #(n-1052)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10512)
	; 10 bytes
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	delay_n 284
	dex
	Jmi *-7
.elseif(n = 10513)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10515)
	; 10 bytes
	ldx #(n-1)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10516)
	; 10 bytes
	ldx #(n-1)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10517)
	; 10 bytes
	ldx #(n-957)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10519)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #51
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 10521)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10522)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10526)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10527)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 10528)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10529)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10531)
	; 9 bytes
	ldx #195
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10533)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10535)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10536)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10539)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 55
	dey
	Jne *-7
.elseif(n = 10540)
	; 10 bytes
	ldx #(n-1172)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10543)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10544)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10546)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 10547) && (n < 10586))
.if((n >= 10547) && (n < 10566))
; 10547 to 10565
.if(n = 10547)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 284
	inx
	inx
	Jpl *-7
.elseif(n = 10548 || n = 10550 || n = 10552 || n = 10553 || n = 10555 || n = 10560 || n = 10564)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10549)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10551)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10554)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10556)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #233
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10557)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10558)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10559)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #15
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 10561)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10562)
	; 10 bytes
	ldx #58
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 10563)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #54
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10565)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 10566 to 10585
.if((n > 10565 && n < 10570) || n = 10578 || n = 10582 || n = 10584)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10570)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10571)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10572)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10573)
	; 10 bytes
	ldx #(n-1)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 10575)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10576)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10577)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10579)
	; 9 bytes
	ldy #82
	delay_n 124
	dey
	Jne *-5
.elseif(n = 10580)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10581)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10583)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10585)
	; 9 bytes
	ldx #196
	delay_n 49
	dex
	Jne *-5
.endif
.endif
.else
; 10586 to 10625
.if(n = 10586 || (n > 10587 && n < 10591) || n = 10592 || (n > 10594 && n < 10599) || n = 10600 || n = 10602 || n = 10608 || n = 10610 || n = 10614 || (n > 10615 && n < 10621) || n = 10622 || n = 10624)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10587)
	; 10 bytes
	ldx #67
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif(n = 10591)
	; 10 bytes
	ldx #(n-1)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10593)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10594)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #8
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 10601)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10603)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10604)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10605)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10606)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10607)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 10609)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10611)
	; 10 bytes
	ldx #(n-1062)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10612)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10613)
	; 10 bytes
	ldx #6
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 10615)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10621)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10623)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10625)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
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
.if(n = 10626 || n = 10628 || n = 10629 || n = 10632 || (n > 10633 && n < 10639) || n = 10640 || n = 10642 || n = 10644 || n = 10645 || n = 10650 || n = 10652 || n = 10653 || n = 10656 || n = 10658 || n = 10662 || n = 10664)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10627)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10630)
	; 10 bytes
	ldx #(n-1182)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10631)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #86
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 10633)
	; 10 bytes
	ldx #(n-1)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10639)
	; 9 bytes
	ldx #197
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10641)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10643)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10646)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #235
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10647)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #15
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 10648)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10649)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10651)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10654)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10655)
	; 10 bytes
	ldx #(n-1)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10657)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10659)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10660)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10661)
	; 10 bytes
	ldx #(n-1067)/1066
	ldy #212
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10663)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 241
	dey
	Jmi *-7
.endif
.else
; 10665 to 10703
.if(n = 10665)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10666)
	; 10 bytes
	ldx #(n-1)/711
	ldy #141
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10667 || n = 10668 || n = 10670 || n = 10672 || n = 10677 || (n > 10682 && n < 10687) || (n > 10687 && n < 10691) || n = 10692 || n = 10698 || n = 10700 || n = 10701 || n = 10703)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10669)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10671)
	; 10 bytes
	ldx #184 ;hides 'clv'
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 10673)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10674)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10675)
	; 10 bytes
	ldx #(n-1187)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10676)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10678)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 174
	dey
	Jmi *-7
.elseif(n = 10679)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10680)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10681)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10682)
	; 10 bytes
	ldx #(n-972)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10687)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10691)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 191
	iny
	Jne *-7
.elseif(n = 10693)
	; 9 bytes
	ldx #198
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10694)
	; 9 bytes
	ldx #36
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 10695 || n = 10696)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10697)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 10702)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 10704) && (n < 10743))
.if((n >= 10704) && (n < 10723))
; 10704 to 10722
.if((n .mod 2 = 0) && (n = 10704 || n = 10706 || (n > 10711 && n < 10719)))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10705)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10707)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10708)
	; 9 bytes
	ldy #82
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 10709)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 56
	dey
	Jne *-7
.elseif(n = 10710)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10711)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10713)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10715)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 10717)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10719)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10720)
	; 10 bytes
	ldx #(n-1192)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10721)
	; 10 bytes
	ldx #(n-537)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10722)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 10723 to 10742
.if(n = 10723 || n = 10726 || n = 10730 || n = 10733 || n = 10734 || n = 10738 || n = 10740 || n = 10742)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10724)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 136
	dey
	Jmi *-7
.elseif(n = 10725)
	; 10 bytes
	ldx #(n-1)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10727)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10728)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10729)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10731)
	; 10 bytes
	ldx #145
	delay_n 69
	dex
	Jne *-6
.elseif(n = 10732)
	; 10 bytes
	ldx #(n-512)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10735)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #96
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 10736)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #237
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10737)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10739)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10741)
	; 10 bytes
	ldx #(n-1)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
; 10743 to 10781
.if(n = 10743)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10744 || n = 10746 || n = 10748 || n = 10750 || n = 10752 || n = 10754 || (n > 10755 && n < 10759) || n = 10763 || n = 10764 || n = 10766 || n = 10767 || n = 10769 || (n > 10770 && n < 10774) || n = 10775 || (n > 10776 && n < 10781))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10745)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10747)
	; 9 bytes
	ldx #199
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10749)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10751)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10753)
	; 9 bytes
	ldy #27
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 10755)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10759)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10760)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10761)
	; 10 bytes
	ldx #(n-1077)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10762)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10765)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10768)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10770)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10774)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10776)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10781)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 10782) && (n < 10860))
.if((n >= 10782) && (n < 10821))
.if((n >= 10782) && (n < 10801))
; 10782 to 10800
.if(n = 10782)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #51
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif((n > 10782 && n < 10787) || n = 10790 || n = 10797 || n = 10799 || n = 10800)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10787)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 51
	dey
	Jne *-7
.elseif(n = 10788)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10789)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10791)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10792)
	; 10 bytes
	ldx #(n-982)/981
	ldy #195
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10793)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10794)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10795)
	; 10 bytes
	ldx #(n-1)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10796)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 176
	dey
	Jmi *-7
.elseif(n = 10798)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 10801 to 10820
.if(n = 10801)
	; 9 bytes
	ldx #200
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10802 || n = 10806 || n = 10812 || n = 10815 || n = 10818 || n = 10820)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10803)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10804)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10805)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10807)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 10808)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10809)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10810)
	; 10 bytes
	ldx #(n-1202)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10811)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10813)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10814)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 96
	dey
	Jmi *-7
.elseif(n = 10816)
	; 10 bytes
	ldx #(n-1)/721
	ldy #143
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10817)
	; 10 bytes
	ldx #(n-1)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10819)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
; 10821 to 10859
.if(n = 10821)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10822 || n = 10827 || n = 10829 || n = 10832 || n = 10833 || n = 10835 || n = 10836 || n = 10838 || (n > 10841 && n < 10845) || n = 10848 || (n > 10850 && n < 10855) || n = 10856)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10823)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #22
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 10824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 10825)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10826)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #239
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10828)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10830)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10831)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10834)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10837)
	; 9 bytes
	ldy #84
	delay_n 124
	dey
	Jne *-5
.elseif(n = 10839)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10840)
	; 10 bytes
	ldx #200 ;hides 'iny'
	delay_n 284
	dex
	dex
	Jmi *-7
.elseif(n = 10841)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10845)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #96
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 10846)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10847)
	; 10 bytes
	ldx #10
	ldy #196
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10849)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10850)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10855)
	; 9 bytes
	ldx #201
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10857)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10858)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10859)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 10860) && (n < 10899))
; 10860 to 10898
.if(n = 10860 || n = 10862 || n = 10866 || n = 10868 || n = 10870 || n = 10872 || (n > 10874 && n < 10878) || n = 10880 || (n > 10882 && n < 10886) || n = 10887 || n = 10889 || n = 10890 || (n > 10891 && n < 10897))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10861)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10863)
	; 10 bytes
	ldy #56 ;hides 'sec'
	delay_n 379
	dey
	dey
	Jne *-7
.elseif(n = 10864)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10865)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10867)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #216 ;hides 'cld'
	iny
	Jne *-2
	dex
	Jmi *-7
.elseif(n = 10869)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10871)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #88
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 10873)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10874)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10878)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 246
	dey
	Jmi *-7
.elseif(n = 10879)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10881)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10882)
	; 10 bytes
	ldx #(n-1)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10886)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10888)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10891)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10897)
	; 10 bytes
	ldx #(n-1)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10898)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 10899 to 10937
.if(n = 10899 || n = 10904 || n = 10906 || n = 10908 || n = 10910 || n = 10912 || n = 10914 || n = 10917 || n = 10918 || n = 10922 || (n > 10924 && n < 10933) || n = 10937)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10900)
	; 10 bytes
	ldx #(n-1212)/1211
	ldy #241
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10901)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10902)
	; 10 bytes
	ldx #(n-992)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10903)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10905)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10907)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10909)
	; 9 bytes
	ldx #202
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10911)
	; 10 bytes
	ldx #(n-1092)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10913)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10915)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10916)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #241
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10919)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10920)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10921)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10923)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10924)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10933)
	; 10 bytes
	ldx #(n-1)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10934)
	; 10 bytes
	ldx #(n-842)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10935)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10936)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if((n > 10937 && n < 10942) || n = 10944 || n = 10948 || n = 10950 || n = 10952 || n = 10954 || n = 10956 || n = 10958 || n = 10959 || n = 10962 || n = 10964 || n = 10967 || (n > 10968 && n < 10973) || n = 10974 || n = 10976)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10942)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10943)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #55
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 10945)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10946)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #22
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 10947)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10949)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10951)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10953)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10955)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #98
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 10957)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10960)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10961)
	; 10 bytes
	ldx #(n-1097)/1096
	ldy #218
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10963)
	; 9 bytes
	ldx #203
	delay_n 49
	dex
	Jne *-5
.elseif(n = 10965 || n = 10968)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10966)
	; 9 bytes
	ldy #84
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 10973)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 10975)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 10977 to 11015
.if(n = 10977)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif((n > 10977 && n < 10981) || n = 10982 || n = 10984 || n = 10986 || n = 10988 || n = 10989 || n = 10992 || n = 10994 || (n > 10995 && n < 10999) || (n > 11000 && n < 11005) || n = 11008 || n = 11013 || n = 11014)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10981)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10983)
	; 9 bytes
	ldx #37
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 10985)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10987)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #52
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 10990)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 10991)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #90
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 10993)
	; 10 bytes
	ldx #(n-1)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 10995)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 10999)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11000)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 11005)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11006)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11007)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 121
	iny
	Jne *-7
.elseif(n = 11009)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11010)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11011)
	; 10 bytes
	ldx #(n-1102)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11012)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11015)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #28
	dey
	Jpl *-1
	inx
	Jpl *-7
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
; 11016 to 11054
.if(n = 11016 || n = 11018 || n = 11022 || (n > 11027 && n < 11033) || n = 11034 || n = 11036 || n = 11038 || n = 11039 || n = 11043 || n = 11044 || n = 11046 || n = 11048 || n = 11049 || n = 11052 || n = 11054)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11017)
	; 9 bytes
	ldx #204
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11019 || n = 11027)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11020)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #18
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 11021)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11023)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11024)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11025)
	; 9 bytes
	ldx #16
	delay_n 684
	dex
	Jne *-5
.elseif(n = 11026)
	; 10 bytes
	ldx #(n-442)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11033)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11035)
	; 10 bytes
	ldx #(n-1227)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11037)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11040)
	; 10 bytes
	ldx #(n-1)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11041)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11042)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11045)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11047)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11050)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11051)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11053)
	; 10 bytes
	ldx #(n-1)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 11055 to 11093
.if(n = 11055 || n = 11058 || n = 11060 || n = 11062 || n = 11063 || n = 11066 || n = 11069 || n = 11070 || n = 11072 || n = 11076 || n = 11078 || (n > 11080 && n < 11085) || n = 11086 || n = 11088 || n = 11090 || n = 11092)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11056)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11057)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 284
	dex
	Jmi *-6
.elseif(n = 11059)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11061)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11064)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11065)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11067)
	; 10 bytes
	ldx #(n-1007)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11068)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11071)
	; 9 bytes
	ldx #205
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11073)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #86
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 11075)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11077)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11079)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11080)
	; 10 bytes
	ldx #(n-1232)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11085)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11087)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11089)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11091)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 181
	dey
	Jmi *-7
.elseif(n = 11093)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 11094) && (n < 11172))
.if((n >= 11094) && (n < 11133))
; 11094 to 11132
.if(n = 11094 || n = 11100 || n = 11102 || (n > 11103 && n < 11110) || n = 11114 || n = 11115 || n = 11117 || n = 11118 || n = 11120 || n = 11124 || n = 11126 || n = 11128 || n = 11130 || n = 11132)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11095)
	; 9 bytes
	ldy #85
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 11096)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #245
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 11097)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11098)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11099)
	; 10 bytes
	ldx #61
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 11101)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11103)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11110)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11111)
	; 10 bytes
	ldx #9
	ldy #221
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11112)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11113)
	; 10 bytes
	ldx #(n-1)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11116)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11119)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11121)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11122)
	; 10 bytes
	ldx #(n-1012)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11123)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11125)
	; 9 bytes
	ldx #206
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11127)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11129)
	; 10 bytes
	ldx #(n-857)/856
	ldy #170
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11131)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 11133 to 11171
.if(n = 11133)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11134 || (n > 11141 && n < 11145) || n = 11146 || n = 11147 || n = 11149 || n = 11150 || n = 11154 || (n > 11155 && n < 11159) || n = 11160 || n = 11162 || n = 11163 || (n > 11164 && n < 11168) || n = 11169 || n = 11171)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11135)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11136)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11137)
	; 9 bytes
	ldy #28
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 11138)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11139)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11140)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11141)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #246
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 11145)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11148)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11151)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11152)
	; 10 bytes
	ldx #(n-532)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11153)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11155)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11159)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #16
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 11161)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11164)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11168)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #28
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 11170)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 11172) && (n < 11211))
; 11172 to 11210
.if(n = 11172 || n = 11174 || n = 11176 || n = 11178 || n = 11180 || n = 11182 || n = 11183 || (n > 11186 && n < 11191) || n = 11193 || n = 11196 || n = 11198 || n = 11200 || (n > 11201 && n < 11207) || n = 11208 || n = 11209)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11173)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11175)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11177)
	; 10 bytes
	ldx #(n-1017)/1016
	ldy #202
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11179)
	; 9 bytes
	ldx #207
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11181)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11184)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11185)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11186)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #247
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 11191)
	; 10 bytes
	ldx #(n-1)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11192)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11194)
	; 10 bytes
	ldx #(n-92)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11195)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11197)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11199)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 49
	inx
	Jne *-6
.elseif(n = 11201)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11207)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11210)
	; 9 bytes
	ldy #10
	delay_n 1014
	dey
	Jpl *-5
.endif
.else
; 11211 to 11250
.if(n = 11211)
	; 10 bytes
	ldx #(n-1122)/1121
	ldy #223
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11212)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11213 || n = 11214 || n = 11216 || n = 11218 || n = 11220 || n = 11222 || n = 11227 || n = 11228 || n = 11234 || n = 11239 || n = 11240 || n = 11242 || (n > 11243 && n < 11247) || n = 11248 || n = 11250)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11215)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11217)
	; 10 bytes
	ldx #(n-1)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11219)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11221)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11223)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11224)
	; 9 bytes
	ldy #86
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 11225)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11226 || n = 11235)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11229)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11230)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11231)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 11232)
	; 10 bytes
	ldx #(n-1022)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11233)
	; 9 bytes
	ldx #208
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11236)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11237)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11238)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11241)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11243)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11247)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	inx
	Jne *-7
.elseif(n = 11249)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
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
	; 10 bytes
	ldy #56 ;hides 'sec'
	delay_n 379
	dey
	dey
	Jpl *-7
.elseif(n = 11252)
	; 10 bytes
	ldy #234 ;hides 'nop'
	delay_n 1014
	iny
	iny
	Jne *-7
.elseif(n = 11253)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11254)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11255)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11256 || n = 11258 || n = 11262 || n = 11264 || n = 11268)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11257)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11259)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11260)
	; 10 bytes
	ldx #(n-1252)/1251
	ldy #249
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11261)
	; 10 bytes
	ldx #(n-1127)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11263)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #23
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 11265)
	; 9 bytes
	ldy #170 ;hides 'tax'
	delay_n 124
	iny
	Jne *-6
.elseif(n = 11266)
	; 10 bytes
	ldx #(n-1)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11267)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11269)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 11270 to 11289
.if(n = 11270)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11271)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11272)
	; 9 bytes
	ldx #38
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 11273 || n = 11275 || n = 11277 || n = 11279 || n = 11280 || (n > 11281 && n < 11285) || n = 11288)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11274)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11276)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11278)
	; 10 bytes
	ldx #62
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 11281)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11285)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11286)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11287)
	; 9 bytes
	ldx #209
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11289)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
; 11290 to 11328
.if(n = 11290)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11291 || n = 11292 || n = 11295 || (n > 11297 && n < 11301) || n = 11302 || n = 11303 || n = 11306 || n = 11307 || n = 11309 || n = 11310 || n = 11312 || (n > 11313 && n < 11317) || n = 11318 || n = 11322 || n = 11327)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11293)
	; 10 bytes
	ldx #(n-1)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11294)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11296)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11297)
	; 10 bytes
	ldx #(n-1)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11301)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11304)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 124
	dey
	dey
	Jne *-7
.elseif(n = 11305)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11308)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 256
	dey
	Jmi *-7
.elseif(n = 11311)
	; 10 bytes
	ldx #(n-1132)/1131
	ldy #225
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11313)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11317)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #55
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 11320)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11321)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11323)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11324)
	; 10 bytes
	ldx #(n-872)/871
	ldy #173
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11325)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11326)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11328)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 11329) && (n < 11368))
; 11329 to 11367
.if(n = 11329)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11330 || (n > 11331 && n < 11337) || n = 11338 || (n > 11343 && n < 11347) || n = 11348 || n = 11349 || n = 11352 || n = 11354 || (n > 11355 && n < 11359) || n = 11360 || n = 11364 || n = 11365)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11331)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11337)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11339)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 101
	dey
	Jmi *-7
.elseif(n = 11340)
	; 10 bytes
	ldx #(n-1)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11341)
	; 9 bytes
	ldx #210
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11342)
	; 10 bytes
	ldx #(n-1032)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11343)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11347)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11350)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11351)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 11353)
	; 9 bytes
	ldy #87
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 11355)
	; 10 bytes
	ldx #(n-1)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11359)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11361)
	; 10 bytes
	ldx #9
	ldy #226
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11362)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11363)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11366)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #251
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 11367)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-7
.endif
.else
; 11368 to 11406
.if(n = 11368)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11369)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif((n > 11369 && n < 11374) || (n > 11377 && n < 11383) || n = 11384 || n = 11385 || n = 11387 || n = 11388 || n = 11390 || n = 11392 || n = 11394 || n = 11396 || n = 11399 || n = 11400 || n = 11402 || n = 11403 || n = 11405 || n = 11406)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11374)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11375)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11376)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11377)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11383)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11386)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 186
	dey
	Jmi *-7
.elseif(n = 11389)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11391)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #23
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 11393)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11395)
	; 9 bytes
	ldx #211
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11397)
	; 10 bytes
	ldx #10
	ldy #206
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11398)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11401)
	; 10 bytes
	ldx #(n-457)/456
	ldy #90
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11404)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
.if((n >= 11407) && (n < 11446))
; 11407 to 11445
.if(n = 11407)
	; 10 bytes
	ldx #184 ;hides 'clv'
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
	dex
	Jne *-7
.elseif(n = 11408)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11409)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11410 || n = 11412 || n = 11417 || n = 11418 || n = 11424 || (n > 11425 && n < 11431) || n = 11433 || n = 11435 || n = 11436 || n = 11438 || (n > 11441 && n < 11446))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11411)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11413)
	; 10 bytes
	ldx #(n-1)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11414)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11415)
	; 10 bytes
	ldy #248 ;hides 'sed'
	delay_n 1420
	iny
	Jne *-7
.elseif(n = 11416)
	; 10 bytes
	ldx #(n-1)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11419)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11420)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11421)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11422)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11423)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 11425)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11431)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11432)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11434)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11437)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 126
	iny
	Jne *-7
.elseif(n = 11439)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11440)
	; 10 bytes
	ldx #(n-1272)/1271
	ldy #253
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11441)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 11446 to 11484
.if(n = 11446 || n = 11448 || n = 11450 || n = 11453 || n = 11459 || n = 11460 || n = 11462 || (n > 11463 && n < 11467) || n = 11468 || n = 11470 || n = 11472 || n = 11475 || (n > 11477 && n < 11481) || n = 11483)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11447)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 11449)
	; 9 bytes
	ldx #212
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11451 || n = 11484)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11452)
	; 10 bytes
	ldx #(n-1042)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11454)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11455)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11456)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #253
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 11457)
	; 10 bytes
	ldx #(n-1)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11458)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11461)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11463)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11467)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11469)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11471)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #93
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 11473)
	; 10 bytes
	ldx #(n-1)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11474)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 146
	dey
	Jmi *-7
.elseif(n = 11476)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11477)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11481)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11482)
	; 9 bytes
	ldy #88
	delay_n 124
	dey
	Jpl *-5
.endif
.endif
.else
.if((n >= 11485) && (n < 11524))
; 11485 to 11523
.if(n = 11485)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif((n > 11485 && n < 11491) || n = 11492 || n = 11494 || (n > 11496 && n < 11501) || n = 11504 || n = 11506 || n = 11510 || n = 11512 || n = 11514 || (n > 11515 && n < 11519) || n = 11520)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11491)
	; 10 bytes
	ldx #(n-1)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11493)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11495)
	; 10 bytes
	ldx #(n-1)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11496)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11501)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11502)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11503)
	; 9 bytes
	ldx #213
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11505)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #102
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 11507)
	; 10 bytes
	ldx #(n-1047)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11508)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11509)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11511)
	; 10 bytes
	ldx #(n-1152)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11513)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #58
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 11515)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11519)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11521)
	; 9 bytes
	ldy #29
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 11522)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11523)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 261
	dey
	Jmi *-7
.endif
.else
; 11524 to 11562
.if(n = 11524)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11525)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11526)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11527)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11528 || n = 11532 || n = 11534 || n = 11536 || n = 11538 || (n > 11539 && n < 11545) || (n > 11547 && n < 11551) || n = 11552 || n = 11554 || n = 11556 || n = 11560)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11529)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11530)
	; 10 bytes
	ldx #(n-1282)/1281
	ldy #255
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11531)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 55
	dey
	Jne *-7
.elseif(n = 11533)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11535)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11537)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11539)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11545)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11546)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #255
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 11547)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11551)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11553)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11555)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11557)
	; 9 bytes
	ldx #214
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11558)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11559)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 61
	dey
	Jne *-7
.elseif(n = 11561)
	; 9 bytes
	ldx #40
	delay_n 284
	dex
	Jne *-5
.elseif(n = 11562)
	; 10 bytes
	ldx #(n-1052)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jpl *-6
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
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11564 || n = 11567 || n = 11568 || n = 11571 || n = 11576 || n = 11577 || n = 11579 || n = 11580 || n = 11582 || n = 11583 || (n > 11586 && n < 11591) || n = 11592 || n = 11594 || n = 11596 || n = 11598 || n = 11600)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11565)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11566)
	; 10 bytes
	ldx #(n-1)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11569)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11570)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 11572)
	; 10 bytes
	ldx #(n-552)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11573)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11574)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11575)
	; 10 bytes
	ldx #<((n-1287)/1286)
	ldy #>((n-1287)/1286)
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11578)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11581)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11584)
	; 10 bytes
	ldx #(n-1)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11585)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11586)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11591)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11593)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11595)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11597)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #9
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 11601)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 11602 to 11640
.if(n = 11602)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #55
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif((n > 11602 && n < 11606) || (n > 11606 && n < 11610) || n = 11612 || n = 11614 || n = 11618 || n = 11620 || (n > 11621 && n < 11628) || (n > 11630 && n < 11635) || n = 11637 || n = 11640)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11606)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11610)
	; 10 bytes
	ldx #(n-1)/611
	ldy #121
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11611)
	; 9 bytes
	ldy #90
	delay_n 124
	dey
	Jne *-5
.elseif(n = 11613)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11615)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #104
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 11616)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11617)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11619)
	; 10 bytes
	ldx #157
	delay_n 69
	dex
	Jne *-6
.elseif(n = 11621)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11628)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11629)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11630)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11635)
	; 10 bytes
	ldx #(n-1)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11636)
	; 10 bytes
	ldx #65
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 11638)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11639)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 284
	inx
	Jne *-6
.endif
.endif
.else
.if((n >= 11641) && (n < 11680))
; 11641 to 11679
.if(n = 11641)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif((n > 11641 && n < 11645) || n = 11646 || n = 11648 || n = 11650 || n = 11655 || n = 11662 || n = 11664 || n = 11666 || (n > 11667 && n < 11672) || (n > 11672 && n < 11677) || n = 11678)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11645)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11647)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 11649)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11651)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11652)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11653)
	; 10 bytes
	ldx #(n-1)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11654)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11656)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11657)
	; 10 bytes
	ldx #(n-1)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11658)
	; 9 bytes
	ldy #216 ;hides 'cld'
	delay_n 124
	dey
	Jmi *-6
.elseif(n = 11659)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11660)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11661)
	; 10 bytes
	ldx #9
	ldy #232
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11663)
	; 10 bytes
	ldx #(n-687)/686
	ldy #136
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11665)
	; 9 bytes
	ldx #216
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11667)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11672)
	; 10 bytes
	ldx #(n-1062)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11677)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11679)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jmi *-7
.endif
.else
; 11680 to 11718
.if(n = 11680)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11681)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11682 || n = 11684 || n = 11686 || n = 11687 || (n > 11688 && n < 11693) || n = 11694 || n = 11696 || n = 11698 || n = 11700 || n = 11702 || (n > 11705 && n < 11711) || n = 11712 || n = 11718)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11683)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11685)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11688)
	; 10 bytes
	ldx #29
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 11693)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11695)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 265
	dey
	Jmi *-7
.elseif(n = 11697)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 11701)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11703)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #25
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 11704)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11705)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11711)
	; 10 bytes
	ldx #(n-1172)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11713)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11714)
	; 9 bytes
	ldx #16
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 11715)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 51
	dey
	Jne *-7
.elseif(n = 11716)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11717)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 11719) && (n < 11797))
.if((n >= 11719) && (n < 11758))
; 11719 to 11757
.if(n = 11719)
	; 9 bytes
	ldx #217
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11720 || (n > 11721 && n < 11725) || n = 11726 || (n > 11727 && n < 11731) || (n > 11731 && n < 11735) || n = 11736 || n = 11739 || n = 11741 || n = 11744 || n = 11745 || n = 11747 || n = 11750 || n = 11752 || (n > 11754 && n < 11758))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11721)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11725)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #105
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 11727)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11731)
	; 10 bytes
	ldx #(n-1)/46
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11735)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #31
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 11737)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11738)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11740)
	; 9 bytes
	ldy #91
	delay_n 124
	dey
	Jne *-5
.elseif(n = 11742)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11743)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11746)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11748)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11749)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11751)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #21
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 11753)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11754)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 11758 to 11796
.if(n = 11758 || n = 11760 || (n > 11761 && n < 11765) || (n > 11767 && n < 11771) || n = 11772 || n = 11778 || n = 11780 || n = 11784 || n = 11786 || n = 11788 || n = 11790 || n = 11792 || (n > 11793 && n < 11797))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11759)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #18
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 11761)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11765)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11766)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11767)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11771)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 211
	iny
	Jne *-7
.elseif(n = 11773)
	; 9 bytes
	ldx #218
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11774)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11775)
	; 10 bytes
	ldx #(n-1)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11776)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11777)
	; 10 bytes
	ldx #0
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11779)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11781)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11782)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11783)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11785)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11787)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11789)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11791)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11793)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 11797) && (n < 11836))
; 11797 to 11835
.if(n = 11797 || n = 11799 || n = 11802 || (n > 11803 && n < 11807) || n = 11808 || n = 11813 || n = 11814 || n = 11816 || n = 11820 || n = 11822 || n = 11823 || n = 11825 || n = 11826 || n = 11828 || n = 11830 || n = 11832 || n = 11834)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11798)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11800)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11801)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11803)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11807)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #55
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 11809)
	; 10 bytes
	ldx #(n-1)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11810)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11811)
	; 10 bytes
	ldx #(n-1182)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11812)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11815)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11817)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11818)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11819)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11821)
	; 10 bytes
	ldx #(n-592)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 11827)
	; 9 bytes
	ldx #219
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11829)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11831)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11833)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11835)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 11836 to 11875
.if(n = 11836)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #25
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 11837)
	; 10 bytes
	ldx #(n-1077)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif((n > 11837 && n < 11841) || n = 11842 || (n > 11845 && n < 11849) || n = 11853 || n = 11854 || n = 11856 || n = 11862 || n = 11863 || n = 11865 || n = 11868 || n = 11870 || n = 11872 || n = 11874 || n = 11875)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11841)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11843)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11844)
	; 10 bytes
	ldx #(n-912)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11845)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11849)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 151
	dey
	Jmi *-7
.elseif(n = 11850)
	; 9 bytes
	ldx #40
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 11851)
	; 10 bytes
	ldx #74
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 11852 || n = 11871)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11855)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 11857)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11858)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11859)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11860)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11861)
	; 10 bytes
	ldx #(n-1187)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11864)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 106
	dey
	Jmi *-7
.elseif(n = 11866)
	; 10 bytes
	ldx #(n-1)/791
	ldy #157
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11867)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 131
	iny
	Jne *-7
.elseif(n = 11869)
	; 9 bytes
	ldy #91
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 11873)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
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
.if(n = 11876 || n = 11877 || n = 11879 || n = 11882 || n = 11885 || n = 11886 || (n > 11887 && n < 11892) || n = 11894 || n = 11897 || n = 11900 || n = 11902 || n = 11904 || n = 11906 || n = 11910 || n = 11912 || n = 11913)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11878)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11880)
	; 10 bytes
	ldy #166 ;hides 'ldx zp'
	nop
	delay_n 124
	iny
	Jne *-7
.elseif(n = 11881)
	; 9 bytes
	ldx #220
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11883 || n = 11907)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11884)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11887)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11892)
	; 10 bytes
	ldx #10
	ldy #215
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11893)
	; 10 bytes
	ldx #(n-1)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11895)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11896)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11898)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #31
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 11899)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11901)
	; 10 bytes
	ldx #(n-477)/476
	ldy #93
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11903)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11905)
	; 9 bytes
	ldy #31
	delay_n 379
	dey
	Jne *-5
.elseif(n = 11908)
	; 10 bytes
	ldx #(n-442)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11909)
	; 10 bytes
	ldx #(n-917)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11911)
	; 10 bytes
	ldx #(n-1192)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11914)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 11915 to 11953
.if(n = 11915)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11916 || n = 11923 || n = 11924 || n = 11926 || n = 11928 || n = 11931 || n = 11934 || (n > 11937 && n < 11941) || (n > 11941 && n < 11945) || n = 11946 || n = 11948 || n = 11950 || n = 11952)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11917)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11918)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11919)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11920)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11921)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11922)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11925)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11927)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 11929)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11930)
	; 9 bytes
	ldx #168 ;hides 'tay'
	delay_n 284
	dex
	Jmi *-6
.elseif(n = 11932)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11933)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11935)
	; 9 bytes
	ldx #221
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11936)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11937)
	; 10 bytes
	ldx #(n-1)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11941)
	; 10 bytes
	ldx #(n-1)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11945)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #107
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 11947)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11949)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11951)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #98
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 11953)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 11954) && (n < 11993))
; 11954 to 11992
.if(n = 11954 || n = 11956 || (n > 11957 && n < 11961) || (n > 11961 && n < 11967) || n = 11968 || n = 11970 || n = 11972 || n = 11975 || n = 11977 || n = 11978 || (n > 11980 && n < 11984) || n = 11988)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11955)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11957)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11961)
	; 10 bytes
	ldx #9
	ldy #238
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11967)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11969)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11971)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11973)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11974)
	; 10 bytes
	ldx #(n-922)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 11976)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 196
	dey
	Jmi *-7
.elseif(n = 11979)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11980)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11984)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 11985)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11986)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11987)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 11989)
	; 9 bytes
	ldx #222
	delay_n 49
	dex
	Jne *-5
.elseif(n = 11990)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 11991)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11992)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 11993 to 12031
.if(n = 11993 || (n > 11994 && n < 11998) || (n > 12003 && n < 12007) || n = 12008 || n = 12010 || n = 12014 || n = 12015 || n = 12019 || (n > 12021 && n < 12025) || n = 12027 || n = 12029 || n = 12030)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11994)
	; 10 bytes
	ldx #67
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 11998)
	; 9 bytes
	ldy #93
	delay_n 124
	dey
	Jne *-5
.elseif(n = 11999)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 153
	dey
	Jmi *-7
.elseif(n = 12000)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12001)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12002)
	; 10 bytes
	ldx #(n-1092)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12003)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12007)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12009)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12011)
	; 10 bytes
	ldx #(n-1202)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12012)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #57
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 12013)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12016)
	; 10 bytes
	ldx #15
	ldy #159
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12017)
	; 10 bytes
	ldx #(n-1)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12018)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12020)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12021)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12025)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12026)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12028)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12031)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
; 12032 to 12070
.if(n = 12032 || n = 12035 || n = 12036 || n = 12038 || n = 12040 || n = 12042 || n = 12044 || (n > 12045 && n < 12049) || n = 12050 || n = 12052 || n = 12054 || n = 12056 || (n > 12057 && n < 12061) || n = 12062 || n = 12064 || (n > 12065 && n < 12069))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12033)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12034)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12037)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12039)
	; 10 bytes
	ldx #(n-927)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12041)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12043)
	; 9 bytes
	ldx #223
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12045)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12049)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12051)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12053)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12055)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12057)
	; 10 bytes
	ldx #(n-1097)/1096
	ldy #218
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12061)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12063)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12065)
	; 10 bytes
	ldx #(n-1)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12069)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12070)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 12071 to 12109
.if(n = 12071)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12072 || n = 12075 || n = 12077 || n = 12078 || n = 12080 || n = 12082 || n = 12086 || n = 12087 || n = 12089 || (n > 12091 && n < 12095) || n = 12096 || n = 12098 || n = 12102 || n = 12103 || (n > 12105 && n < 12109))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12073)
	; 10 bytes
	ldx #(n-1)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 12076)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12079)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12081)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12083)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #62
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 12084)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12085)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12088)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12090)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #21
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 12091)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12095)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 49
	dex
	Jne *-6
.elseif(n = 12097)
	; 9 bytes
	ldx #224
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12099)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12100)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12101)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12104)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12105)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12109)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 166
	iny
	Jne *-7
.endif
.endif
.else
.if((n >= 12110) && (n < 12149))
; 12110 to 12148
.if(n = 12110 || (n > 12112 && n < 12117) || n = 12118 || n = 12120 || n = 12124 || n = 12126 || n = 12131 || n = 12132 || (n > 12133 && n < 12137) || n = 12138 || (n > 12145 && n < 12149))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12111)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12112)
	; 10 bytes
	ldx #(n-1102)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12117)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12119)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12121)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12122)
	; 10 bytes
	ldx #(n-1)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12123)
	; 10 bytes
	ldx #(n-552)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12125)
	; 10 bytes
	ldx #(n-1)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12127)
	; 9 bytes
	ldy #93
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 12128)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12129 || n = 12141)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12130)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12133)
	; 10 bytes
	ldx #(n-1)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12137)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12139)
	; 9 bytes
	ldx #41
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 12140)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12142)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12143)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #26
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 12144)
	; 10 bytes
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 124
	iny
	Jpl *-7
.elseif(n = 12145)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 12149 to 12187
.if(n = 12149 || n = 12150 || n = 12152 || n = 12154 || (n > 12156 && n < 12161) || (n > 12161 && n < 12165) || (n > 12169 && n < 12173) || (n > 12173 && n < 12177) || n = 12182 || n = 12183 || (n > 12184 && n < 12188))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12151)
	; 9 bytes
	ldx #225
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12153)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12155)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12156)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12161)
	; 10 bytes
	ldx #9
	ldy #242
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12165)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #109
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 12166)
	; 10 bytes
	ldx #(n-1)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12167)
	; 10 bytes
	ldx #(n-1107)/1106
	ldy #220
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12168)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 276
	dey
	Jmi *-7
.elseif(n = 12169)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12173)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12177)
	; 10 bytes
	ldx #(n-1)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12178)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12179)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 167
	iny
	Jne *-7
.elseif(n = 12180)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12181)
	; 10 bytes
	ldx #(n-1)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12184)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
.if((n >= 12188) && (n < 12227))
; 12188 to 12226
.if(n = 12188 || n = 12189 || n = 12192 || n = 12194 || n = 12197 || n = 12198 || n = 12200 || n = 12203 || n = 12204 || n = 12206 || n = 12208 || n = 12212 || n = 12215 || n = 12216 || (n > 12217 && n < 12221) || n = 12226)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12190)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12191)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12193)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12195)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12196)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12199)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12201)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12202)
	; 10 bytes
	ldx #(n-582)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12205)
	; 9 bytes
	ldx #226
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12207)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #42
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 12209)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12210)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12211)
	; 10 bytes
	ldx #9
	ldy #243
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12213)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12214)
	; 10 bytes
	ldx #(n-532)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12217)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #57
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 12221)
	; 10 bytes
	ldx #(n-612)/611
	ldy #121
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12222)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12223)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12224)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 156
	dey
	Jmi *-7
.elseif(n = 12225)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 12227 to 12265
.if(n = 12227 || n = 12228 || n = 12230 || n = 12236 || n = 12238 || n = 12240 || n = 12242 || n = 12243 || n = 12245 || n = 12246 || (n > 12248 && n < 12253) || n = 12254 || n = 12255 || n = 12260 || n = 12264)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12229)
	; 9 bytes
	ldy #12
	delay_n 1014
	dey
	Jne *-5
.elseif(n = 12231)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12232)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12233)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12234)
	; 10 bytes
	ldx #(n-942)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12235)
	; 10 bytes
	ldy #184 ;hides 'clv'
	delay_n 124
	dey
	dey
	Jne *-7
.elseif(n = 12237)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12239)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12241)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12244)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12247)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12248)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12253)
	; 10 bytes
	ldx #(n-1)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12256)
	; 9 bytes
	ldy #95
	delay_n 124
	dey
	Jne *-5
.elseif(n = 12257)
	; 10 bytes
	ldx #(n-1)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12258)
	; 10 bytes
	ldx #(n-722)/721
	ldy #143
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12259)
	; 9 bytes
	ldx #227
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12261)
	; 10 bytes
	ldx #(n-1227)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12262)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12263)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 12265)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 12266) && (n < 12305))
; 12266 to 12304
.if(n = 12266 || n = 12267 || n = 12269 || n = 12270 || n = 12272 || n = 12274 || (n > 12277 && n < 12281) || n = 12282 || n = 12283 || n = 12287 || n = 12288 || n = 12290 || n = 12293 || n = 12294 || n = 12296 || n = 12298 || (n > 12301 && n < 12305))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12268)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12271)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #21
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 12273)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12275)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12276)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12277)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12281)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #26
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 12284)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12285)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12286)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12289)
	; 9 bytes
	ldy #31
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 12291)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12292)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12295)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #8
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 12297)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12299)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12300)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12301)
	; 10 bytes
	ldx #(n-1)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 12305 to 12343
.if(n = 12305 || n = 12306 || n = 12308 || n = 12310 || n = 12312 || n = 12314 || n = 12317 || n = 12318 || n = 12320 || (n > 12325 && n < 12329) || n = 12330 || n = 12333 || n = 12334 || n = 12336 || (n > 12337 && n < 12342))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12307)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12309)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12311)
	; 10 bytes
	ldx #(n-1232)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12313)
	; 9 bytes
	ldx #228
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12315)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12316)
	; 10 bytes
	ldx #(n-1)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #60
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 12321)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12322)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12323)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 12325)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12329)
	; 10 bytes
	ldx #(n-537)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12331)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12332)
	; 10 bytes
	ldx #(n-1122)/1121
	ldy #223
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12335)
	; 10 bytes
	ldx #(n-1)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12337)
	; 10 bytes
	ldx #(n-1)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12342)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12343)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 12344) && (n < 12422))
.if((n >= 12344) && (n < 12383))
; 12344 to 12382
.if((n > 12343 && n < 12349) || n = 12350 || n = 12351 || n = 12354 || (n > 12355 && n < 12359) || n = 12360 || n = 12362 || n = 12363 || n = 12366 || n = 12371 || n = 12372 || (n > 12373 && n < 12377) || (n > 12377 && n < 12382))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12349)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12352)
	; 10 bytes
	ldx #68
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 12353)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12355)
	; 10 bytes
	ldx #(n-1)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12359)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #18
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 12361)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12364)
	; 10 bytes
	ldx #(n-952)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12365)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12367)
	; 9 bytes
	ldx #229
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12368)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 124
	dey
	dey
	Jne *-7
.elseif(n = 12369)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12370)
	; 10 bytes
	ldx #(n-1)/651
	ldy #128
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12373)
	; 10 bytes
	ldx #(n-1)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12377)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12382)
	; 10 bytes
	ldx #202 ;hides 'dex'
	php
	delay_n 49
	plp
	Jne *-7
.endif
.else
; 12383 to 12421
.if(n = 12383)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12384)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12385)
	; 9 bytes
	ldy #96
	delay_n 124
	dey
	Jne *-5
.elseif(n = 12386 || n = 12388 || n = 12390 || n = 12392 || n = 12393 || (n > 12394 && n < 12398) || n = 12400 || n = 12402 || n = 12404 || (n > 12405 && n < 12409) || n = 12410 || n = 12414 || n = 12415 || (n > 12417 && n < 12421))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12387)
	; 10 bytes
	ldx #(n-1127)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12389)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12391)
	; 10 bytes
	ldx #15
	ldy #164
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12394)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12398)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12399)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
	inx
	Jne *-7
.elseif(n = 12401)
	; 10 bytes
	ldx #(n-497)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12403)
	; 9 bytes
	ldx #17
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 12405)
	; 10 bytes
	ldx #(n-1)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12409)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12411)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12412)
	; 10 bytes
	ldx #(n-592)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12413)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12416)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12417)
	; 10 bytes
	ldx #(n-1)/776
	ldy #154
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12421)
	; 9 bytes
	ldx #230
	delay_n 49
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 12422) && (n < 12461))
; 12422 to 12460
.if(n = 12422)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #58
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 12423 || n = 12430)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12424)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12425)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 12426)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12427 || (n > 12433 && n < 12439) || n = 12440 || n = 12443 || n = 12447 || n = 12452 || n = 12454 || n = 12456 || n = 12458 || n = 12460)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12428)
	; 9 bytes
	ldx #42
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 12429)
	; 10 bytes
	ldx #(n-957)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12431)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12432)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12433)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12439)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12441)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12442)
	; 10 bytes
	ldx #(n-1132)/1131
	ldy #225
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12444)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12445)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12446)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12448)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12449)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #98
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 12450)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12451)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12453)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12455)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #32
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 12457)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12459)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 171
	iny
	Jne *-7
.endif
.else
; 12461 to 12500
.if(n = 12461)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12462)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #18
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 12463)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12464)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12465)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12466)
	; 10 bytes
	ldx #(n-1)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12467)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12468)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12469 || n = 12470 || (n > 12471 && n < 12475) || n = 12476 || (n > 12477 && n < 12481) || n = 12482 || n = 12485 || n = 12486 || n = 12488 || n = 12491 || n = 12492 || n = 12498 || n = 12499)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12471)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12475)
	; 9 bytes
	ldx #231
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12477)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12481)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12483)
	; 10 bytes
	ldx #78
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 12484)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12487)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 12489)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12490)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12493)
	; 10 bytes
	ldx #(n-1)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12494)
	; 10 bytes
	ldx #(n-962)/961
	ldy #191
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12495)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #112
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 12496)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12497)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12500)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
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
.if(n = 12501 || n = 12502 || n = 12504 || n = 12508 || n = 12510 || n = 12512 || n = 12515 || n = 12518 || n = 12522 || n = 12524 || n = 12528 || n = 12530 || n = 12532 || (n > 12534 && n < 12540))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12503)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12505)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12506)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12507)
	; 10 bytes
	ldx #(n-1)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12509)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12511)
	; 10 bytes
	ldx #(n-1252)/1251
	ldy #249
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12513)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12514)
	; 9 bytes
	ldy #96
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 12516)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12517)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12519)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #61
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 12520)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12521)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12523)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 55
	dey
	Jne *-7
.elseif(n = 12525)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12526)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12527)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #24 ;hides 'clc'
	dey
	Jne *-2
	inx
	Jpl *-7
.elseif(n = 12529)
	; 9 bytes
	ldx #232
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12531)
	; 10 bytes
	ldx #69
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 12533)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12534)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 12540 to 12578
.if(n = 12540)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 49
	inx
	Jne *-7
.elseif(n = 12541)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12542 || n = 12543 || n = 12546 || n = 12548 || n = 12549 || n = 12554 || n = 12555 || n = 12558 || n = 12562 || n = 12564 || n = 12565 || (n > 12568 && n < 12574) || n = 12575 || n = 12576 || n = 12578)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12544)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12545)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12547)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12550)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12551)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12552)
	; 10 bytes
	ldx #(n-1142)/1141
	ldy #227
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12553)
	; 10 bytes
	ldx #(n-1)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12556)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12557)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12559)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12560)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12561)
	; 10 bytes
	ldx #(n-1257)/1256
	ldy #250
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12563)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12566)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 206
	dey
	Jmi *-7
.elseif(n = 12567)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12568)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12574)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12577)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 12579) && (n < 12618))
; 12579 to 12617
.if(n = 12579)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12580 || n = 12582 || n = 12584 || n = 12586 || (n > 12587 && n < 12597) || n = 12600 || (n > 12601 && n < 12605) || n = 12606 || (n > 12607 && n < 12611) || n = 12612 || n = 12614)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12581)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 226
	iny
	Jne *-7
.elseif(n = 12583)
	; 9 bytes
	ldx #233
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12585)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12587)
	; 10 bytes
	ldx #(n-1)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12597)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 49
	inx
	Jne *-7
.elseif(n = 12598)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #10
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 12601)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12605)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #112
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 12607)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12611)
	; 10 bytes
	ldx #(n-1262)/1261
	ldy #251
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12613)
	; 10 bytes
	ldx #(n-1)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12615)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12616)
	; 10 bytes
	ldx #(n-1)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12617)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 12618 to 12656
.if(n = 12618)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12619)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12620 || n = 12623 || n = 12630 || (n > 12631 && n < 12636) || n = 12638 || n = 12640 || n = 12642 || n = 12644 || n = 12648 || n = 12652 || n = 12654 || n = 12656)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12621)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12622)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12624)
	; 10 bytes
	ldx #(n-972)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12625)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12626)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12627)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #60
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 12628)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12629)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12631)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12636)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12637)
	; 9 bytes
	ldx #234
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12639)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12641)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12643)
	; 9 bytes
	ldy #98
	delay_n 124
	dey
	Jne *-5
.elseif(n = 12645)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12646)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12647)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 61
	dey
	Jne *-7
.elseif(n = 12649)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12650)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12651)
	; 10 bytes
	ldx #(n-507)/506
	ldy #100
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12653)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #64
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 12655)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
.if((n >= 12657) && (n < 12696))
; 12657 to 12695
.if(n = 12657)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif((n > 12657 && n < 12661) || n = 12663 || (n > 12664 && n < 12669) || n = 12670 || n = 12672 || (n > 12675 && n < 12683) || n = 12684 || (n > 12685 && n < 12689) || n = 12690 || n = 12692 || n = 12694 || n = 12695)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12661)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12662)
	; 10 bytes
	ldx #(n-1152)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12664)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12669)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 174
	iny
	Jne *-7
.elseif(n = 12671)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12673)
	; 9 bytes
	ldy #32
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 12674)
	; 10 bytes
	ldx #(n-552)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12675)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12683)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12685)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12689)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12691)
	; 9 bytes
	ldx #235
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12693)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 12696 to 12734
.if(n = 12696 || n = 12698 || n = 12702 || n = 12704 || n = 12705 || n = 12707 || n = 12712 || n = 12714 || n = 12716 || (n > 12721 && n < 12725) || n = 12728 || (n > 12729 && n < 12733) || n = 12734)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12697)
	; 10 bytes
	ldx #(n-277)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #100
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 12700)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12701)
	; 10 bytes
	ldx #24
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 12703)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12706)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12708)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12709)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12710)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12711)
	; 10 bytes
	ldx #(n-1272)/1271
	ldy #253
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12713)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12715)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12717)
	; 9 bytes
	ldx #43
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 12718)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12719)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12720)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12721)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12725)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 56
	dey
	Jne *-7
.elseif(n = 12726)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #27
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 12727)
	; 10 bytes
	ldx #(n-102)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12729)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12733)
	; 10 bytes
	ldx #(n-1)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 12735) && (n < 12774))
; 12735 to 12773
.if(n = 12735 || n = 12736 || n = 12740 || n = 12743 || n = 12744 || n = 12746 || n = 12748 || n = 12753 || (n > 12755 && n < 12761) || (n > 12761 && n < 12766) || n = 12770 || n = 12771)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12737)
	; 10 bytes
	ldx #(n-1)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12738)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12739)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12741)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12742)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12745)
	; 9 bytes
	ldx #236
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12747)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12749)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	dex
	Jpl *-7
.elseif(n = 12750)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12751)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12752)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12754)
	; 10 bytes
	ldx #(n-982)/981
	ldy #195
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12755)
	; 10 bytes
	ldx #(n-1)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12761)
	; 10 bytes
	ldx #9
	ldy #254
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12766)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12767)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12768)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12769)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12772)
	; 9 bytes
	ldy #98
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 12773)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 12774 to 12812
.if(n = 12774)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12775 || (n > 12776 && n < 12781) || n = 12782 || n = 12784 || (n > 12785 && n < 12789) || n = 12792 || (n > 12793 && n < 12798) || n = 12800 || n = 12804 || (n > 12805 && n < 12809) || n = 12810 || n = 12812)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12776)
	; 10 bytes
	ldx #(n-512)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12781)
	; 10 bytes
	ldx #(n-1)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12783)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12785)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12789)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12790)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12791)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #22
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 12793)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12798)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12799)
	; 9 bytes
	ldx #237
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12801)
	; 10 bytes
	ldx #>(n+48)
	ldy #<(n+48)
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12802)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12803)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12805)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12809)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 176
	iny
	Jne *-7
.elseif(n = 12811)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 12813) && (n < 12969))
.if((n >= 12813) && (n < 12891))
.if((n >= 12813) && (n < 12852))
; 12813 to 12851
.if(n = 12813)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 291
	dey
	Jmi *-7
.elseif(n = 12814 || n = 12816 || n = 12818 || n = 12820 || n = 12822 || n = 12823 || (n > 12828 && n < 12832) || n = 12834 || n = 12837 || n = 12838 || n = 12840 || n = 12842 || n = 12844 || (n > 12845 && n < 12849) || n = 12850)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12815)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12817)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12819)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12821)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #101
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 12825)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12826)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12827)
	; 10 bytes
	ldx #(n-107)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12828)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12832)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12833)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12835)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12836)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12839)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12841)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12843)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #65
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 12845)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12849)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12851)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 231
	iny
	Jne *-7
.endif
.else
; 12852 to 12890
.if(n = 12852)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12853)
	; 9 bytes
	ldx #238
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12854 || (n > 12855 && n < 12861) || (n > 12861 && n < 12865) || (n > 12865 && n < 12871) || (n > 12872 && n < 12877) || n = 12878 || n = 12879 || (n > 12884 && n < 12889) || n = 12890)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12855)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12861)
	; 10 bytes
	ldx #<((n-1287)/1286)
	ldy #>((n-1287)/1286)
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12865)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12871)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12872)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12877)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12880)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12881)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12882)
	; 10 bytes
	ldx #(n-1172)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12883)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12884)
	; 10 bytes
	ldx #(n-992)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12889)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 12891) && (n < 12930))
; 12891 to 12929
.if(n = 12891)
	; 10 bytes
	ldx #168 ;hides 'tay'
	delay_n 284
	inx
	inx
	Jne *-7
.elseif(n = 12892 || n = 12894 || n = 12896 || (n > 12897 && n < 12901) || n = 12902 || n = 12905 || n = 12906 || n = 12908 || n = 12912 || n = 12915 || n = 12917 || n = 12918 || n = 12920 || n = 12922 || (n > 12923 && n < 12927) || n = 12928)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12893)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12895)
	; 10 bytes
	ldx #(n-1)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12897)
	; 10 bytes
	ldx #(n-1)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12901)
	; 9 bytes
	ldy #100
	delay_n 124
	dey
	Jne *-5
.elseif(n = 12903)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12904)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12907)
	; 9 bytes
	ldx #239
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12909)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12910)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12911)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #106
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 12913)
	; 10 bytes
	ldx #(n-1)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12914)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 116
	dey
	Jmi *-7
.elseif(n = 12916)
	; 10 bytes
	ldx #(n-1)/861
	ldy #171
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12919)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #63
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 12921)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12923)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12927)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12929)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 12930 to 12968
.if(n = 12930 || n = 12932 || n = 12934 || n = 12936 || n = 12939 || (n > 12940 && n < 12944) || (n > 12944 && n < 12948) || n = 12950 || (n > 12951 && n < 12955) || n = 12956 || n = 12958 || n = 12960 || n = 12962 || n = 12964 || (n > 12965 && n < 12969))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12931)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12933)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12935)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12937)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12938)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12940)
	; 10 bytes
	ldx #(n-1)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12944)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12948)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12949)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12951)
	; 10 bytes
	ldx #175
	delay_n 69
	dex
	Jne *-6
.elseif(n = 12955)
	; 10 bytes
	ldx #(n-1)/51
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12957)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12959)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12961)
	; 9 bytes
	ldx #240
	delay_n 49
	dex
	Jne *-5
.elseif(n = 12963)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12965)
	; 10 bytes
	ldx #(n-1)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 12969) && (n < 13047))
.if((n >= 12969) && (n < 13008))
; 12969 to 13007
.if(n = 12969 || n = 12971 || n = 12972 || n = 12975 || n = 12976 || n = 12978 || n = 12980 || (n > 12981 && n < 12985) || n = 12990 || n = 12996 || (n > 12997 && n < 13003) || n = 13004 || n = 13005 || n = 13007)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12970)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12973)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12974)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 166
	dey
	Jmi *-7
.elseif(n = 12977)
	; 10 bytes
	ldx #(n-1)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12979)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12981)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12985)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 12986)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 12987)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12988)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12989)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12991)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 49
	inx
	Jne *-6
.elseif(n = 12992)
	; 10 bytes
	ldx #(n-1182)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 12993)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12994)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12995)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 12997)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13003)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13006)
	; 9 bytes
	ldx #45
	delay_n 284
	dex
	Jne *-5
.endif
.else
; 13008 to 13046
.if(n = 13008 || n = 13017 || n = 13032)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13009)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif((n > 13009 && n < 13014) || n = 13016 || n = 13018 || n = 13020 || n = 13022 || n = 13024 || n = 13029 || n = 13034 || n = 13036 || n = 13038 || n = 13043 || n = 13044 || n = 13046)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13014)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13015)
	; 9 bytes
	ldx #241
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13019)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13021)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13023)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13025)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13026)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13027)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13028)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 296
	dey
	Jmi *-7
.elseif(n = 13030)
	; 9 bytes
	ldy #101
	delay_n 124
	dey
	Jne *-5
.elseif(n = 13031)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 13033)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13035)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13037)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #62
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 13039)
	; 10 bytes
	ldx #(n-1)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13040)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13041)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13042)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13045)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #117
	dey
	Jne *-1
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 13047) && (n < 13086))
; 13047 to 13085
.if(n = 13047)
	; 10 bytes
	ldx #(n-1187)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13048 || n = 13050 || n = 13054 || n = 13056 || n = 13058 || (n > 13059 && n < 13063) || n = 13064 || n = 13070 || n = 13072 || (n > 13074 && n < 13079) || n = 13082 || n = 13084 || n = 13085)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13049)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13051)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13052)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13053)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13055)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13057)
	; 9 bytes
	ldy #33
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 13059 || n = 13071)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13063)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13065)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13066)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13067)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #19
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 13068)
	; 10 bytes
	ldx #72
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 13069)
	; 9 bytes
	ldx #242
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13073)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 13079)
	; 10 bytes
	ldx #(n-1007)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13080)
	; 10 bytes
	ldx #(n-1)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13081)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13083)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 13086 to 13125
.if((n > 13085 && n < 13092) || n = 13095 || n = 13097 || n = 13098 || n = 13100 || n = 13104 || n = 13106 || n = 13107 || (n > 13109 && n < 13113) || n = 13117 || n = 13118 || n = 13120 || n = 13122 || n = 13124)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13092)
	; 9 bytes
	ldx #19
	delay_n 684
	dex
	Jne *-5
.elseif(n = 13093)
	; 10 bytes
	ldx #(n-1)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13094)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13096)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13099)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13101)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13102)
	; 10 bytes
	ldx #(n-1192)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13103)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 49
	dex
	Jne *-6
.elseif(n = 13105)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13108)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13109)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13113)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13114)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13115)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13116)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13119)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #63
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 13121)
	; 10 bytes
	ldx #(n-657)/656
	ldy #130
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13123)
	; 9 bytes
	ldx #243
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13125)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
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
.if((n > 13125 && n < 13130) || n = 13132 || n = 13139 || n = 13140 || n = 13142 || n = 13146 || n = 13148 || n = 13150 || n = 13152 || n = 13154 || n = 13158 || n = 13163 || n = 13164)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13130)
	; 10 bytes
	ldx #(n-1)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13131)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13133)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13134)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13135)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13136)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13137)
	; 10 bytes
	ldx #(n-1)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13138)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13141)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13143)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #8
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 13144)
	; 10 bytes
	ldx #(n-1012)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13145)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13147)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13149)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13151)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13153)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13155)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #117
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 13156)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 216
	dey
	Jmi *-7
.elseif(n = 13157)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13159)
	; 9 bytes
	ldy #102
	delay_n 124
	dey
	Jne *-5
.elseif(n = 13160)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #22
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 13161)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13162)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 13165 to 13203
.if((n > 13164 && n < 13168) || n = 13169 || n = 13170 || n = 13172 || n = 13174 || n = 13176 || n = 13178 || (n > 13180 && n < 13184) || (n > 13185 && n < 13189) || n = 13191 || n = 13192 || n = 13194 || n = 13195 || n = 13197 || n = 13198 || n = 13200 || n = 13202)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13168)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13171)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #27
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 13173)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13175)
	; 10 bytes
	ldx #(n-1)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13177)
	; 9 bytes
	ldx #244
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13179)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13180)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13184)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 284
	dex
	dex
	Jmi *-7
.elseif(n = 13185)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13189)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13190)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13193)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13196)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13199)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 13201)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13203)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 13204) && (n < 13243))
; 13204 to 13242
.if(n = 13204)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13205 || n = 13206 || n = 13211 || n = 13215 || (n > 13217 && n < 13221) || n = 13222 || n = 13224 || (n > 13225 && n < 13231) || (n > 13234 && n < 13238) || n = 13239 || n = 13240)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13207)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13208)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13209)
	; 10 bytes
	ldx #(n-1017)/1016
	ldy #202
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13210)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13212)
	; 10 bytes
	ldx #(n-1202)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13213)
	; 10 bytes
	ldx #(n-1)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13214)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13216)
	; 10 bytes
	ldx #(n-1)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13217)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13221)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13223)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13225)
	; 10 bytes
	ldx #(n-1)/456
	ldy #90
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13231)
	; 9 bytes
	ldx #245
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13232)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13233)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13234)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13238)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13241)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13242)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #63
	dey
	Jne *-1
	dex
	Jmi *-7
.endif
.else
; 13243 to 13281
.if(n = 13243)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 301
	dey
	Jmi *-7
.elseif(n = 13244 || n = 13246 || n = 13253 || n = 13254 || n = 13257 || n = 13260 || (n > 13261 && n < 13265) || n = 13266 || (n > 13267 && n < 13271) || n = 13272 || n = 13275 || n = 13277 || n = 13279)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13245)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13247)
	; 10 bytes
	ldx #73
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 13248)
	; 9 bytes
	ldy #13
	delay_n 1014
	dey
	Jne *-5
.elseif(n = 13249)
	; 10 bytes
	ldx #(n-1)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13250)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13251)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13252)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13255)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13256)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13258)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13259)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 71
	dey
	Jne *-7
.elseif(n = 13261)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13265)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #119
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 13267)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13271)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #109
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 13273)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13274)
	; 10 bytes
	ldx #(n-1022)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13276)
	; 10 bytes
	ldx #(n-532)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13278)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13280)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13281)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 13282) && (n < 13360))
.if((n >= 13282) && (n < 13321))
; 13282 to 13320
.if((n > 13281 && n < 13285) || n = 13286 || n = 13292 || n = 13293 || n = 13296 || n = 13302 || n = 13303 || (n > 13304 && n < 13311) || n = 13314 || (n > 13315 && n < 13319))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13285)
	; 9 bytes
	ldx #246
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13287)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13288)
	; 9 bytes
	ldy #102
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 13289)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13290)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13291)
	; 10 bytes
	ldx #(n-1)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13294)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13295)
	; 9 bytes
	ldx #45
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 13297)
	; 10 bytes
	ldx #(n-1)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13298)
	; 10 bytes
	ldy #24 ;hides 'clc'
	delay_n 1014
	dey
	dey
	Jpl *-7
.elseif(n = 13299)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13300)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13301)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13304)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13311)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13312)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13313)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13315)
	; 10 bytes
	ldx #(n-1)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #64
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 13320)
	; 10 bytes
	ldx #(n-1)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 13321 to 13359
.if(n = 13321)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13322)
	; 10 bytes
	ldx #(n-1212)/1211
	ldy #241
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13323 || n = 13325 || n = 13326 || n = 13328 || n = 13330 || n = 13332 || n = 13334 || n = 13337 || n = 13338 || n = 13340 || (n > 13341 && n < 13345) || (n > 13345 && n < 13349) || (n > 13349 && n < 13357) || n = 13359)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #105
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 13327)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 13329)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13331)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13333)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13335)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13336)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13339)
	; 9 bytes
	ldx #247
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13341)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13345)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13349)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13357)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13358)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 13360) && (n < 13399))
; 13360 to 13398
.if(n = 13360)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13361 || n = 13365 || n = 13368 || n = 13369 || n = 13372 || n = 13373 || n = 13376 || n = 13379 || n = 13380 || (n > 13381 && n < 13385) || n = 13386 || (n > 13387 && n < 13391) || n = 13392 || n = 13394 || n = 13398)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13362)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13363)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13364)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13366)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13367)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13370)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13371)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13374)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13375)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #120
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 13377)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13378)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13381)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13385)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13387)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13391)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13393)
	; 9 bytes
	ldx #248
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13395)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13396)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13397)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 49
	dex
	dex
	Jne *-7
.endif
.else
; 13399 to 13437
.if(n = 13399)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13400 || n = 13402 || n = 13403 || (n > 13404 && n < 13413) || n = 13414 || n = 13416 || n = 13418 || (n > 13421 && n < 13426) || n = 13427 || n = 13428 || n = 13430 || (n > 13434 && n < 13438))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13401)
	; 10 bytes
	ldx #(n-537)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13404)
	; 10 bytes
	ldx #(n-1032)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13413)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #69
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 13415)
	; 10 bytes
	ldx #(n-1)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13417)
	; 9 bytes
	ldy #104
	delay_n 124
	dey
	Jne *-5
.elseif(n = 13419)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13420)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13421)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13426)
	; 10 bytes
	ldx #74
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 13429)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13431)
	; 10 bytes
	ldx #84
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 13432)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13433)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13434)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
.if((n >= 13438) && (n < 13477))
; 13438 to 13476
.if(n = 13438 || n = 13442 || (n > 13443 && n < 13447) || n = 13450 || n = 13452 || n = 13454 || n = 13456 || (n > 13458 && n < 13462) || n = 13464 || n = 13466 || n = 13467 || (n > 13469 && n < 13473) || (n > 13473 && n < 13477))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13439)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 121
	dey
	Jmi *-7
.elseif(n = 13440)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13441)
	; 9 bytes
	ldy #35
	delay_n 379
	dey
	Jne *-5
.elseif(n = 13443)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13447)
	; 9 bytes
	ldx #249
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13448)
	; 10 bytes
	ldx #(n-792)/791
	ldy #157
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13449)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #106
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 13451)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 221
	dey
	Jmi *-7
.elseif(n = 13453)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13455)
	; 10 bytes
	ldx #(n-1)/961
	ldy #191
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13457)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13458)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 306
	dey
	Jmi *-7
.elseif(n = 13462)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13463)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13465)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13468)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13469)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13473)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 13477 to 13515
.if(n = 13477 || n = 13478 || n = 13480 || n = 13481 || n = 13484 || n = 13486 || n = 13488 || n = 13490 || n = 13492 || n = 13493 || n = 13496 || (n > 13497 && n < 13501) || n = 13502 || n = 13504 || (n > 13505 && n < 13509) || n = 13512 || n = 13514)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13479)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13482)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13483)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13485)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #121
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 13487)
	; 10 bytes
	ldx #(n-1227)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13489)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13491)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13494)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13495)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13497)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13501)
	; 9 bytes
	ldx #250
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13503)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13505)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13509)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 186
	iny
	Jne *-7
.elseif(n = 13510)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13511)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13513)
	; 10 bytes
	ldx #(n-1)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13515)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 13516) && (n < 13555))
; 13516 to 13554
.if(n = 13516)
	; 10 bytes
	ldx #(n-1)/901
	ldy #179
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13517)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13518 || n = 13520 || (n > 13521 && n < 13525) || n = 13527 || n = 13530 || n = 13532 || n = 13536 || (n > 13537 && n < 13542) || n = 13544 || n = 13549 || n = 13550 || n = 13552 || n = 13554)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13519)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13521)
	; 10 bytes
	ldx #(n-677)/676
	ldy #134
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13525)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13526)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13528)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13529)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13531)
	; 10 bytes
	ldx #(n-1)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13533)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13534)
	; 10 bytes
	ldx #(n-1042)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13535)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13537)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13542)
	; 10 bytes
	ldx #(n-1232)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13543)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13545)
	; 10 bytes
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 379
	dey
	Jne *-7
.elseif(n = 13546)
	; 9 bytes
	ldy #105
	delay_n 124
	dey
	Jne *-5
.elseif(n = 13547)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13548)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13551)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13553)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 13555 to 13593
.if(n = 13555)
	; 9 bytes
	ldx #251
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13556 || n = 13558 || n = 13563 || n = 13566 || n = 13568 || n = 13570 || n = 13575 || n = 13578 || (n > 13579 && n < 13584) || n = 13588 || n = 13590 || n = 13592 || n = 13593)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13557 || n = 13586)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13559)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #21
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 13560)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13561)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13562)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13564)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13565)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 13567)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13569)
	; 10 bytes
	ldx #>(n+48)
	ldy #<(n+48)
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13571)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 13572)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13573)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 13576)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13577)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 66
	dey
	Jne *-7
.elseif(n = 13579)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13584)
	; 9 bytes
	ldx #46
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 13585)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13587)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 151
	iny
	Jne *-7
.elseif(n = 13589)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13591)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 13594) && (n < 13672))
.if((n >= 13594) && (n < 13633))
; 13594 to 13632
.if(n = 13594)
	; 10 bytes
	ldx #(n-592)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13595)
	; 10 bytes
	ldx #(n-1)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13596 || n = 13598 || n = 13600 || n = 13602 || n = 13606 || n = 13608 || n = 13610 || n = 13612 || n = 13614 || n = 13615 || n = 13619 || n = 13620 || n = 13622 || n = 13624 || (n > 13625 && n < 13630) || n = 13632)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13597)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13599)
	; 10 bytes
	ldx #(n-1047)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13601)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13603)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #69
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 13604)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13605)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13607)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 13609)
	; 9 bytes
	ldx #252
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13611)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13613)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13616)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #28
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 13617)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13618)
	; 10 bytes
	ldx #(n-802)/801
	ldy #159
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13621)
	; 10 bytes
	ldx #(n-682)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13623)
	; 9 bytes
	ldy #24 ;hides 'clc'
	delay_n 124
	iny
	Jpl *-6
.elseif(n = 13625)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13630)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13631)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-7
.endif
.else
; 13633 to 13671
.if(n = 13633)
	; 10 bytes
	ldx #(n-1)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13634 || (n > 13636 && n < 13641) || (n > 13641 && n < 13645) || (n > 13645 && n < 13651) || n = 13653 || (n > 13654 && n < 13660) || n = 13662 || n = 13667 || n = 13668 || n = 13670 || n = 13671)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13635)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13636)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13641)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13645)
	; 10 bytes
	ldx #8
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 13651)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13652)
	; 10 bytes
	ldx #(n-1242)/1241
	ldy #247
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13654)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13660)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13661)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 246
	iny
	Jne *-7
.elseif(n = 13663)
	; 9 bytes
	ldx #253
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13664)
	; 10 bytes
	ldx #(n-1052)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13665)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13666)
	; 10 bytes
	ldx #(n-1)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13669)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 13672) && (n < 13711))
; 13672 to 13710
.if(n = 13672)
	; 10 bytes
	ldx #(n-1)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13673)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 311
	dey
	Jmi *-7
.elseif(n = 13674)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13675)
	; 9 bytes
	ldy #106
	delay_n 124
	dey
	Jne *-5
.elseif(n = 13676 || n = 13679 || n = 13680 || (n > 13681 && n < 13685) || n = 13687 || n = 13688 || n = 13692 || n = 13694 || n = 13696 || n = 13698 || n = 13701 || n = 13702 || n = 13706 || n = 13708 || n = 13710)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13677)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13678)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13681)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13685)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13686)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13689)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13690)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13691)
	; 10 bytes
	ldx #185
	delay_n 69
	dex
	Jne *-6
.elseif(n = 13693)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13695)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #23
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 13697)
	; 10 bytes
	ldx #(n-1)/856
	ldy #170
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13699)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13700)
	; 10 bytes
	ldx #(n-1)/721
	ldy #143
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13703)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13704)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13705)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13707)
	; 10 bytes
	ldx #(n-1247)/1246
	ldy #248
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13709)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 13711 to 13750
.if((n > 13710 && n < 13716) || n = 13718 || n = 13720 || n = 13722 || n = 13727 || (n > 13729 && n < 13735) || n = 13736 || n = 13739 || n = 13740 || (n > 13742 && n < 13746) || n = 13748 || n = 13750)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13716)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13717)
	; 9 bytes
	ldx #254
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13719)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13721)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13723)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #35
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 13724)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 176
	dey
	Jmi *-7
.elseif(n = 13725)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13726)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13728)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13729)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13735)
	; 10 bytes
	ldx #(n-127)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13737)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13738)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13741)
	; 10 bytes
	ldx #(n-1)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13742)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13746)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 226
	dey
	Jmi *-7
.elseif(n = 13747)
	; 10 bytes
	ldx #87
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif(n = 13749)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.if(n = 13751)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 13752 || n = 13755 || n = 13758 || n = 13760 || n = 13763 || n = 13764 || (n > 13765 && n < 13770) || n = 13772 || n = 13774 || n = 13778 || n = 13780 || n = 13782 || n = 13785 || n = 13786)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13753)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13754)
	; 9 bytes
	ldy #232 ;hides 'inx'
	delay_n 124
	dey
	Jmi *-6
.elseif(n = 13756)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13757)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13759)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 153
	iny
	Jne *-7
.elseif(n = 13761)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13762)
	; 10 bytes
	ldx #(n-1252)/1251
	ldy #249
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13765)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13770)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13771)
	; 9 bytes
	ldx #255
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13773)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13775)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13776)
	; 10 bytes
	ldx #(n-552)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13777)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13779)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13781)
	; 9 bytes
	ldx #19
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 13783)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13784)
	; 10 bytes
	ldx #76
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 13787)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13788)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13789)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 13790 to 13828
.if((n > 13789 && n < 13793) || n = 13796 || n = 13797 || n = 13799 || n = 13800 || n = 13802 || (n > 13807 && n < 13813) || n = 13814 || n = 13818 || n = 13820 || n = 13823 || n = 13826 || n = 13828)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13793)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13794)
	; 10 bytes
	ldx #(n-1062)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13795)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13798)
	; 10 bytes
	ldx #(n-512)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13801)
	; 10 bytes
	ldx #(n-1)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13803)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13804)
	; 9 bytes
	ldy #107
	delay_n 124
	dey
	Jne *-5
.elseif(n = 13805)
	; 10 bytes
	ldx #(n-1)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13806)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13807)
	; 10 bytes
	ldx #(n-1)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13813)
	; 10 bytes
	ldx #(n-1)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13815)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #123
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 13816)
	; 10 bytes
	ldx #(n-1)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13817)
	; 10 bytes
	ldx #(n-1257)/1256
	ldy #250
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13819)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13821)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13822)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13824)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13825)
	; 9 bytes
	ldx #0
	delay_n 49
	dex
	Jne *-5
.elseif(n = 13827)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 13829) && (n < 13868))
; 13829 to 13867
.if(n = 13829 || n = 13830 || n = 13832 || n = 13835 || n = 13836 || n = 13840 || (n > 13841 && n < 13846) || n = 13849 || n = 13851 || (n > 13852 && n < 13857) || n = 13858 || n = 13860 || (n > 13861 && n < 13868))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13831)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13833)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13834)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13837)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	inx
	Jne *-7
.elseif(n = 13838)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13839)
	; 10 bytes
	ldx #187
	delay_n 69
	dex
	Jne *-6
.elseif(n = 13841)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13846)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13847)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13848)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13850)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 13852)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13857)
	; 10 bytes
	ldx #(n-1)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13859)
	; 10 bytes
	ldx #(n-1067)/1066
	ldy #212
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13861)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 13868 to 13906
.if(n = 13868 || n = 13869 || n = 13874 || n = 13877 || n = 13878 || n = 13880 || n = 13881 || n = 13884 || n = 13886 || n = 13892 || n = 13896 || (n > 13897 && n < 13901) || n = 13902 || n = 13904 || n = 13906)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13870)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13871)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13872)
	; 10 bytes
	ldx #(n-1262)/1261
	ldy #251
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13873)
	; 9 bytes
	ldx #48
	delay_n 284
	dex
	Jne *-5
.elseif(n = 13875)
	; 10 bytes
	ldx #(n-1)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13876)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13879)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13882)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13883)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13885)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13887)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 49
	dex
	Jne *-6
.elseif(n = 13888)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 316
	dey
	Jmi *-7
.elseif(n = 13889)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13890)
	; 10 bytes
	ldx #19
	ldy #145
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13891)
	; 10 bytes
	ldx #(n-1)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13893)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13894)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13895)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #37
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 13897)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13901)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13903)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13905)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 13907) && (n < 13985))
.if((n >= 13907) && (n < 13946))
; 13907 to 13945
.if(n = 13907 || n = 13908 || (n > 13909 && n < 13913) || n = 13914 || n = 13915 || n = 13918 || n = 13920 || n = 13922 || n = 13923 || n = 13926 || (n > 13927 && n < 13931) || n = 13934 || n = 13936 || (n > 13939 && n < 13943))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13909)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13913)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13916)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13917)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13919)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #68
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 13921)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13924)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13925)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13927)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13931)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 251
	iny
	Jne *-7
.elseif(n = 13932)
	; 10 bytes
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 379
	dey
	Jne *-7
.elseif(n = 13933)
	; 9 bytes
	ldy #108
	delay_n 124
	dey
	Jne *-5
.elseif(n = 13935)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13937)
	; 10 bytes
	ldx #(n-1)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13938)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13939)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13943)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 13944)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13945)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 13946 to 13984
.if((n > 13945 && n < 13949) || n = 13952 || (n > 13953 && n < 13958) || n = 13959 || n = 13961 || n = 13962 || n = 13965 || n = 13968 || (n > 13970 && n < 13975) || (n > 13975 && n < 13979) || n = 13980 || n = 13984)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13949)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 13950)
	; 10 bytes
	ldx #(n-1)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13951)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13953)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13958)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13960)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13963)
	; 10 bytes
	ldx #77
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 13964)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 126
	dey
	Jmi *-7
.elseif(n = 13966)
	; 10 bytes
	ldx #15
	ldy #185
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13967)
	; 10 bytes
	ldy #56 ;hides 'sec'
	delay_n 379
	iny
	iny
	Jpl *-7
.elseif(n = 13969)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13970)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13975)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13979)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13981)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13982)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 13983)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #72
	dey
	Jne *-1
	dex
	Jmi *-7
.endif
.endif
.else
.if((n >= 13985) && (n < 14024))
; 13985 to 14023
.if(n = 13985)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 13986 || n = 13990 || n = 13992 || n = 13994 || n = 13995 || (n > 13996 && n < 14001) || (n > 14001 && n < 14007) || (n > 14009 && n < 14013) || n = 14020 || n = 14022)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13987)
	; 10 bytes
	ldx #(n-112)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13988)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13989)
	; 10 bytes
	ldx #(n-1077)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 13991)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #114
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 13993)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 13996)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14001)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14007)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 14008)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14009)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14013)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14014)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14015)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14016)
	; 9 bytes
	ldy #234 ;hides 'nop'
	delay_n 124
	dey
	Jmi *-6
.elseif(n = 14017)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14018)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14019)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14021)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14023)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 14024 to 14062
.if(n = 14024 || n = 14025 || n = 14027 || n = 14028 || n = 14030 || (n > 14031 && n < 14035) || n = 14038 || n = 14039 || n = 14042 || (n > 14044 && n < 14051) || n = 14052 || n = 14055 || n = 14056 || n = 14058 || n = 14060)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14026)
	; 10 bytes
	ldx #24
	ldy #111
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14029)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14031)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14035)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #126
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 14036)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14037)
	; 10 bytes
	ldx #(n-117)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14040)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14041)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14043)
	; 10 bytes
	ldx #(n-827)/826
	ldy #164
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14044)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14051)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14053)
	; 10 bytes
	ldx #(n-1)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14054)
	; 10 bytes
	ldx #(n-612)/611
	ldy #121
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14057)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14059)
	; 10 bytes
	ldx #(n-1)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14061)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14062)
	; 9 bytes
	ldy #109
	delay_n 124
	dey
	Jne *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 14063) && (n < 14219))
.if((n >= 14063) && (n < 14141))
.if((n >= 14063) && (n < 14102))
; 14063 to 14101
.if(n = 14063)
	; 10 bytes
	ldx #89
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif(n = 14064)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14065)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14066 || n = 14069 || n = 14070 || n = 14072 || n = 14073 || n = 14075 || n = 14076 || n = 14078 || (n > 14081 && n < 14085) || n = 14086 || n = 14087 || (n > 14088 && n < 14092) || n = 14094 || n = 14096 || n = 14100)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14067)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14068)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14071)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #111
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 14077)
	; 10 bytes
	ldx #(n-1)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14079)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	inx
	Jne *-7
.elseif(n = 14080)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14081)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14085)
	; 10 bytes
	ldx #(n-1)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14088)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #37
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 14092)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14093)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14095)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14097)
	; 10 bytes
	ldx #(n-1)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14098)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14099)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14101)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 14102 to 14140
.if(n = 14102 || n = 14104 || n = 14107 || n = 14108 || n = 14110 || n = 14112 || n = 14114 || n = 14115 || n = 14117 || n = 14118 || n = 14120 || (n > 14121 && n < 14125) || n = 14126 || n = 14127 || n = 14129 || (n > 14131 && n < 14135) || n = 14136 || n = 14139 || n = 14140)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14103)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14105)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14106)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14109)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 76
	dey
	Jne *-7
.elseif(n = 14111)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14113)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14116)
	; 10 bytes
	ldx #(n-1)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14119)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14121)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14125)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14128)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14130)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14131)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14135)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14137)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14138)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 14141) && (n < 14180))
; 14141 to 14179
.if(n = 14141)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14142)
	; 10 bytes
	ldx #78
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 14143 || n = 14144 || n = 14146 || n = 14150 || n = 14152 || n = 14154 || n = 14156 || n = 14157 || n = 14160 || n = 14165 || n = 14166 || n = 14168 || (n > 14169 && n < 14173) || n = 14174 || n = 14178 || n = 14179)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14145)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14147)
	; 10 bytes
	ldx #<((n-1287)/1286)
	ldy #>((n-1287)/1286)
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14148)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14149)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14151)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14153)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14155)
	; 10 bytes
	ldx #(n-1)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14158)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14159)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #21
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 14161)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14162)
	; 9 bytes
	ldx #48
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 14163)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14164)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14167)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 14169)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14173)
	; 10 bytes
	ldx #(n-1)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14175)
	; 10 bytes
	ldx #(n-1)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14176)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14177)
	; 10 bytes
	ldx #(n-1)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 14180 to 14218
.if((n > 14179 && n < 14184) || n = 14186 || (n > 14187 && n < 14191) || n = 14192 || n = 14196 || n = 14198 || n = 14200 || n = 14202 || n = 14204 || (n > 14205 && n < 14209) || n = 14210 || n = 14212 || (n > 14215 && n < 14219))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14184)
	; 10 bytes
	ldx #(n-1092)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14185)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14187)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14191)
	; 9 bytes
	ldy #110
	delay_n 124
	dey
	Jne *-5
.elseif(n = 14193)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14194)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14195)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14197)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14199)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 14201)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14203)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14205)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14209)
	; 9 bytes
	ldy #37
	delay_n 379
	dey
	Jne *-5
.elseif(n = 14211)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14213)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14214)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14215)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
.if((n >= 14219) && (n < 14258))
; 14219 to 14257
.if(n = 14219 || n = 14222 || n = 14223 || (n > 14225 && n < 14230) || n = 14232 || (n > 14234 && n < 14239) || (n > 14242 && n < 14249) || n = 14250 || (n > 14251 && n < 14255) || n = 14256)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14220)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14221)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14224)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14225)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14230)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #25
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 14231)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 14233)
	; 10 bytes
	ldx #(n-1)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14234)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14239)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14240)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14241)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14242)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14249)
	; 10 bytes
	ldx #(n-1097)/1096
	ldy #218
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14251)
	; 10 bytes
	ldy #9
	delay_n 1420
	dey
	Jpl *-6
.elseif(n = 14255)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #37
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 14257)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 14258 to 14296
.if(n = 14258 || n = 14259 || (n > 14260 && n < 14266) || n = 14268 || n = 14271 || n = 14274 || n = 14278 || n = 14282 || (n > 14284 && n < 14288) || n = 14290 || n = 14291 || n = 14294 || n = 14296)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14260)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14266)
	; 10 bytes
	ldx #(n-1)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14267)
	; 9 bytes
	ldy #13
	delay_n 1014
	dey
	Jpl *-5
.elseif(n = 14269)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14270)
	; 10 bytes
	ldx #(n-1)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14272)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14273)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14275)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14276)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14277)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14279)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14280)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14281)
	; 10 bytes
	ldx #(n-1)/56
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14283)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14284)
	; 10 bytes
	ldx #(n-622)/621
	ldy #122
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14288)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14289)
	; 10 bytes
	ldx #(n-377)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14292)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14293)
	; 10 bytes
	ldx #(n-1)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14295)
	; 10 bytes
	ldx #(n-1)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 14297) && (n < 14336))
; 14297 to 14335
.if(n = 14297 || n = 14299 || n = 14303 || n = 14304 || n = 14306 || n = 14310 || n = 14312 || n = 14313 || n = 14316 || n = 14317 || n = 14325 || n = 14328 || n = 14331 || (n > 14332 && n < 14336))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14298)
	; 10 bytes
	ldx #(n-842)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14300)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14301)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14302)
	; 10 bytes
	ldx #(n-682)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14305)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14307)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14308)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14309)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14311)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14314)
	; 10 bytes
	ldx #(n-1102)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14315)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14318)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 326
	dey
	Jmi *-7
.elseif(n = 14319)
	; 10 bytes
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 379
	dey
	Jpl *-7
.elseif(n = 14320)
	; 9 bytes
	ldy #111
	delay_n 124
	dey
	Jne *-5
.elseif(n = 14321)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14322)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14323)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 14326)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14327)
	; 10 bytes
	ldx #(n-1)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14329)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14330)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14332)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 14336 to 14375
.if(n = 14336)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14337)
	; 10 bytes
	ldx #0
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14338)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14339 || n = 14340 || n = 14342 || n = 14344 || n = 14347 || n = 14348 || n = 14350 || n = 14352 || n = 14354 || n = 14355 || (n > 14357 && n < 14363) || (n > 14366 && n < 14376))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14341)
	; 10 bytes
	ldx #(n-1)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14343)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14345)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14346)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14349)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14351)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #26
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 14353)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14356)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14357)
	; 10 bytes
	ldx #194
	delay_n 69
	dex
	Jne *-6
.elseif(n = 14363)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14364)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14365)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14366)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
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
.if((n > 14375 && n < 14379) || (n > 14379 && n < 14383) || n = 14384 || n = 14387 || n = 14388 || n = 14390 || n = 14391 || (n > 14392 && n < 14397) || n = 14398 || (n > 14401 && n < 14407) || (n > 14407 && n < 14411) || n = 14412)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14379)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14383)
	; 10 bytes
	ldx #(n-307)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14385)
	; 10 bytes
	ldx #(n-1)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14386)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14389)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14392)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14397)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14399)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14400)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14401)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14407)
	; 10 bytes
	ldx #(n-687)/686
	ldy #136
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14411)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14413)
	; 10 bytes
	ldx #(n-1)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14414)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 14415 to 14453
.if(n = 14415 || n = 14418 || n = 14423 || n = 14424 || (n > 14425 && n < 14431) || n = 14432 || n = 14434 || n = 14436 || (n > 14437 && n < 14441) || n = 14442 || n = 14445 || n = 14448 || n = 14450 || n = 14452)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14416)
	; 10 bytes
	ldx #(n-1)/961
	ldy #191
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14417)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14419)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14420)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 14421)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14422)
	; 10 bytes
	ldx #22
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	dex
	Jpl *-6
.elseif(n = 14425)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14431)
	; 10 bytes
	ldx #(n-1)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14433)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14435)
	; 10 bytes
	ldx #(n-1)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14437)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14441)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14443)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14444)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14446)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14447)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14449)
	; 9 bytes
	ldy #112
	delay_n 124
	dey
	Jne *-5
.elseif(n = 14451)
	; 9 bytes
	ldx #50
	delay_n 284
	dex
	Jne *-5
.elseif(n = 14453)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #37
	dey
	Jpl *-1
	dex
	Jmi *-7
.endif
.endif
.else
.if((n >= 14454) && (n < 14493))
; 14454 to 14492
.if(n = 14454 || n = 14456 || n = 14459 || (n > 14461 && n < 14465) || n = 14466 || n = 14467 || n = 14469 || (n > 14475 && n < 14479) || n = 14480 || (n > 14481 && n < 14485) || (n > 14485 && n < 14489) || n = 14490 || n = 14492)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14455)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14457)
	; 10 bytes
	ldx #(n-1)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14458)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14460)
	; 10 bytes
	ldx #(n-1)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14461)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14465)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14468)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14470)
	; 9 bytes
	ldx #20
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 14471)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 261
	iny
	Jne *-7
.elseif(n = 14472)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #69
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 14473)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14474)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 186
	dey
	Jmi *-7
.elseif(n = 14475)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #130
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 14479)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14481)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14485)
	; 10 bytes
	ldx #(n-1)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14489)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 131
	dey
	Jmi *-7
.elseif(n = 14491)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 14493 to 14531
.if((n > 14492 && n < 14497) || n = 14498 || n = 14499 || (n > 14501 && n < 14505) || n = 14508 || n = 14510 || n = 14511 || n = 14513 || n = 14516 || n = 14518 || n = 14520 || n = 14522 || n = 14525 || (n > 14526 && n < 14530) || n = 14531)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14497)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14500)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14501)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14505)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14506)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #31
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 14507)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 71
	dey
	Jne *-7
.elseif(n = 14509)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14512)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14514)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14515)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14517)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14519)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14521)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14523)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14524)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14526)
	; 10 bytes
	ldx #(n-582)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14530)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
; 14532 to 14570
.if(n = 14532 || n = 14534 || (n > 14537 && n < 14542) || n = 14544 || (n > 14545 && n < 14551) || n = 14557 || n = 14558 || (n > 14561 && n < 14565) || (n > 14566 && n < 14571))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14533)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14535)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14536)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14537)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14542)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14543)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14545)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14551)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14552)
	; 10 bytes
	ldx #168 ;hides 'tay'
	delay_n 684
	dex
	dex
	Jmi *-7
.elseif(n = 14553)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14554)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14555)
	; 10 bytes
	ldx #(n-1)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14556)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14559)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14560)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14561)
	; 10 bytes
	ldx #(n-417)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14565)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14566)
	; 10 bytes
	ldx #(n-1)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 14571 to 14609
.if((n > 14570 && n < 14574) || n = 14576 || (n > 14581 && n < 14585) || n = 14586 || n = 14588 || n = 14590 || n = 14592 || n = 14594 || n = 14596 || n = 14598 || n = 14600 || (n > 14602 && n < 14608))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14574)
	; 10 bytes
	ldx #(n-1122)/1121
	ldy #223
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14575)
	; 10 bytes
	ldx #(n-1)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14577)
	; 10 bytes
	ldx #(n-1)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14578)
	; 9 bytes
	ldy #112
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 14579)
	; 10 bytes
	ldx #197
	delay_n 69
	dex
	Jne *-6
.elseif(n = 14580)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14581)
	; 10 bytes
	ldx #(n-1)/486
	ldy #96
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14585)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #131
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 14587)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14589)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14591)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #120
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 14593)
	; 9 bytes
	ldy #37
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 14595)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14597)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14599)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14601)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14602)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14608)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14609)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 14610) && (n < 14649))
; 14610 to 14648
.if((n > 14609 && n < 14613) || n = 14614 || n = 14616 || n = 14619 || n = 14620 || (n > 14621 && n < 14627) || n = 14628 || n = 14630 || n = 14632 || n = 14634 || n = 14636 || n = 14637 || n = 14640 || n = 14643 || n = 14644 || n = 14647)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14613)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14615)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #10
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 14617)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14618)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14621)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14627)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14629)
	; 10 bytes
	ldx #(n-1)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14631)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14633)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14635)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14638)
	; 10 bytes
	ldx #16
	ldy #171
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14639)
	; 10 bytes
	ldx #(n-1127)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14641)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14642)
	; 10 bytes
	ldx #(n-122)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14645)
	; 10 bytes
	ldx #(n-1)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14646)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14648)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 14649 to 14687
.if(n = 14649 || n = 14654 || n = 14656 || (n > 14657 && n < 14662) || n = 14663 || (n > 14665 && n < 14669) || n = 14670 || n = 14672 || n = 14676 || n = 14678 || (n > 14679 && n < 14687))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14650)
	; 10 bytes
	ldx #(n-1)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14651)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14652)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14653)
	; 10 bytes
	ldx #(n-397)/396
	ldy #77
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14655)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14657)
	; 10 bytes
	ldx #(n-1)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14662)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14664)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14665)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14669)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14671)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14673)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14674)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14675)
	; 10 bytes
	ldx #29
	ldy #100
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14677)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #69
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 14679)
	; 10 bytes
	ldx #81
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 14687)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-7
.endif
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
.if((n >= 14688) && (n < 14766))
.if((n >= 14688) && (n < 14727))
; 14688 to 14726
.if(n = 14688 || n = 14690 || (n > 14691 && n < 14695) || n = 14696 || n = 14697 || n = 14700 || n = 14703 || n = 14710 || n = 14712 || n = 14714 || n = 14718 || n = 14720 || (n > 14723 && n < 14727))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14689)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14691)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14695)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14698)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 14701)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14702)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14704)
	; 10 bytes
	ldx #(n-1132)/1131
	ldy #225
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14705)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 14706)
	; 10 bytes
	ldy #166 ;hides 'ldx zp'
	dey ;first loop only
	delay_n 379
	dey
	Jmi *-7
.elseif(n = 14707)
	; 9 bytes
	ldy #113
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 14708)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14709)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14711)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #121
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 14713)
	; 10 bytes
	ldx #(n-1)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14715)
	; 10 bytes
	ldx #(n-1)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14716)
	; 10 bytes
	ldx #15
	ldy #195
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14717)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14719)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14721)
	; 10 bytes
	ldx #(n-737)/736
	ldy #146
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14722)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14723)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 14727 to 14765
.if(n = 14727)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 14728 || n = 14730 || n = 14732 || n = 14734 || n = 14738 || n = 14739 || n = 14746 || n = 14749 || n = 14751 || n = 14752 || (n > 14753 && n < 14757) || n = 14758 || n = 14760 || n = 14762)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14729)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14731)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14733)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14735)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14736)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14737)
	; 10 bytes
	ldx #(n-1)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14740)
	; 9 bytes
	ldx #51
	delay_n 284
	dex
	Jne *-5
.elseif(n = 14741)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 14742)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14743)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14744)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14745)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14747)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14748)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 336
	dey
	Jmi *-7
.elseif(n = 14750)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14753)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14757)
	; 10 bytes
	ldx #(n-1)/476
	ldy #93
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14759)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #22
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 14761)
	; 10 bytes
	ldx #(n-1)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14763)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14764)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14765)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #26
	dey
	Jne *-1
	dex
	Jmi *-7
.endif
.endif
.else
.if((n >= 14766) && (n < 14805))
; 14766 to 14804
.if(n = 14766 || n = 14768 || n = 14770 || n = 14772 || n = 14774 || n = 14777 || n = 14778 || (n > 14779 && n < 14783) || n = 14784 || (n > 14785 && n < 14791) || n = 14792 || n = 14794 || n = 14796 || n = 14798 || n = 14800 || n = 14803)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14767)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14769)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14771)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14773)
	; 10 bytes
	ldx #(n-1)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14775)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14776)
	; 10 bytes
	ldx #(n-592)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14779)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14783)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 14785)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14791)
	; 10 bytes
	ldx #15
	ldy #196
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14793)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14795)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	dex
	Jmi *-7
.elseif(n = 14797)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14799)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14801)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14802)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14804)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 14805 to 14843
.if(n = 14805)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #133
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 14806)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14807 || n = 14809 || n = 14811 || n = 14812 || (n > 14813 && n < 14817) || n = 14819 || n = 14822 || n = 14823 || n = 14826 || (n > 14827 && n < 14831) || n = 14832 || n = 14835 || n = 14837 || n = 14839)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14808)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14810)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14813)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14817)
	; 10 bytes
	ldx #(n-1)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14818)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #39
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 14820)
	; 10 bytes
	ldx #(n-1)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14821)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14824)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14825)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14827)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14831)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #121
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 14833)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14834)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14836)
	; 9 bytes
	ldy #114
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 14838)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14840)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14841)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14842)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14843)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 14844) && (n < 14922))
.if((n >= 14844) && (n < 14883))
; 14844 to 14882
.if((n > 14843 && n < 14847) || n = 14850 || n = 14852 || n = 14854 || n = 14856 || n = 14857 || (n > 14860 && n < 14866) || n = 14867 || n = 14868 || n = 14870 || n = 14872 || n = 14874 || n = 14876 || n = 14880)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14847)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14848)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14849)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14851)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14853)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14855)
	; 10 bytes
	ldx #(n-1)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14858)
	; 10 bytes
	ldx #83
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 14859)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14860)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14866)
	; 10 bytes
	ldx #(n-1)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14869)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14871)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #27
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 14873)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14875)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14877)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 166
	iny
	Jne *-7
.elseif(n = 14878)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14879)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14881)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14882)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #70
	dey
	Jpl *-1
	dex
	Jmi *-7
.endif
.else
; 14883 to 14921
.if(n = 14883 || (n > 14885 && n < 14893) || n = 14895 || n = 14898 || n = 14900 || (n > 14901 && n < 14905) || n = 14907 || n = 14908 || n = 14910 || n = 14914 || n = 14916 || n = 14918 || n = 14920)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14884)
	; 10 bytes
	ldx #(n-1)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14885)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14893)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14894)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14896)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14897)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14899)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14901)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14905)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14906)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14909)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 206
	iny
	Jne *-7
.elseif(n = 14911)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14912)
	; 10 bytes
	ldx #(n-1)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14913)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14915)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #134
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 14917)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14919)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #72
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 14921)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 14922) && (n < 14961))
; 14922 to 14960
.if((n > 14921 && n < 14925) || n = 14927 || n = 14928 || n = 14930 || n = 14931 || n = 14934 || (n > 14935 && n < 14941) || n = 14942 || n = 14944 || n = 14945 || n = 14948 || n = 14952 || (n > 14953 && n < 14959) || n = 14960)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14925)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14926)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 246
	dey
	Jmi *-7
.elseif(n = 14929)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14932)
	; 10 bytes
	ldx #(n-712)/711
	ldy #141
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14933)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #76
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 14935)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14941)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14943)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14946)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14947)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14949)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14950)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14951)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #122
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 14953)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14959)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 14961 to 15000
.if(n = 14961)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 14962 || n = 14966 || (n > 14967 && n < 14973) || n = 14976 || n = 14980 || n = 14981 || n = 14984 || n = 14985 || (n > 14986 && n < 14990) || (n > 14990 && n < 14995) || n = 14996 || (n > 14997 && n < 15001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14963)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 341
	dey
	Jmi *-7
.elseif(n = 14964)
	; 10 bytes
	ldx #(n-1152)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14965)
	; 9 bytes
	ldy #115
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 14967 || n = 14979)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14973)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14974)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14975)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #39
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 14977)
	; 9 bytes
	ldy #39
	delay_n 379
	dey
	Jne *-5
.elseif(n = 14978)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14982)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 14983)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14986)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 14990)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 14995)
	; 10 bytes
	ldx #(n-307)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 14997)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
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
	; 10 bytes
	ldx #250
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif((n > 15001 && n < 15005) || n = 15006 || n = 15008 || n = 15010 || n = 15012 || n = 15015 || (n > 15016 && n < 15021) || n = 15022 || n = 15027 || n = 15028 || (n > 15031 && n < 15037) || n = 15038)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15005)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15007)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15009)
	; 10 bytes
	ldx #(n-537)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15011)
	; 10 bytes
	ldx #94
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 15013)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15014)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 136
	dey
	Jmi *-7
.elseif(n = 15016)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15021)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15023)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15024)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15025)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15026)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15029)
	; 9 bytes
	ldx #51
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 15030)
	; 10 bytes
	ldx #(n-1)/791
	ldy #157
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15031)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15037)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15039)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 15040 to 15078
.if(n = 15040 || (n > 15041 && n < 15049) || n = 15052 || n = 15054 || n = 15056 || (n > 15057 && n < 15061) || n = 15062 || n = 15064 || n = 15069 || n = 15070 || n = 15072 || n = 15075 || n = 15077 || n = 15078)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15041)
	; 10 bytes
	ldx #(n-1)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15049)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15050)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15051)
	; 10 bytes
	ldx #(n-1)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15053)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15055)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15057)
	; 10 bytes
	ldx #(n-1)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15061)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15063)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15065)
	; 10 bytes
	ldx #(n-1)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15066)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15067)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15068)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15071)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 15073)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 15076)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 15079) && (n < 15118))
; 15079 to 15117
.if(n = 15079 || n = 15080 || (n > 15081 && n < 15086) || n = 15092 || (n > 15097 && n < 15101) || (n > 15101 && n < 15105) || n = 15106 || n = 15108 || n = 15109 || (n > 15110 && n < 15115) || n = 15116 || n = 15117)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15081)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15086)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15087)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #72
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 15088)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15089)
	; 10 bytes
	ldx #(n-657)/656
	ldy #130
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15090)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15091)
	; 10 bytes
	ldx #(n-1)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15093)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15094)
	; 9 bytes
	ldy #117
	delay_n 124
	dey
	Jne *-5
.elseif(n = 15095)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	dex
	Jne *-7
.elseif(n = 15096)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15097)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15101)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15105)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15107)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15110)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15115)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 15118 to 15156
.if(n = 15118 || n = 15120 || n = 15122 || n = 15127 || n = 15128 || (n > 15129 && n < 15133) || n = 15134 || n = 15136 || n = 15138 || n = 15140 || n = 15141 || n = 15144 || n = 15146 || n = 15147 || n = 15150 || (n > 15151 && n < 15157))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15119)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #73
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 15121)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15123)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #77
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 15124)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15125)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15126)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15129)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15133)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15135)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15137)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15139)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15142)
	; 10 bytes
	ldx #(n-722)/721
	ldy #143
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15143)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15145)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15148)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15149)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15151)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 15157) && (n < 15235))
.if((n >= 15157) && (n < 15196))
; 15157 to 15195
.if(n = 15157)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15158 || n = 15160 || n = 15162 || n = 15164 || n = 15167 || n = 15168 || n = 15170 || (n > 15171 && n < 15175) || n = 15176 || n = 15179 || n = 15180 || n = 15182 || (n > 15185 && n < 15190) || n = 15192 || n = 15194)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15159)
	; 9 bytes
	ldx #21
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 15161)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15163)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15165)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15166)
	; 10 bytes
	ldx #(n-1)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15169)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15171)
	; 10 bytes
	ldx #205
	delay_n 69
	dex
	Jne *-6
.elseif(n = 15175)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15177)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15178)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 346
	dey
	Jmi *-7
.elseif(n = 15181)
	; 10 bytes
	ldx #(n-1)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15183)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #39
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 15184)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15185)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15190)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15191)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 15193)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15195)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 15196 to 15234
.if(n = 15196 || n = 15200 || n = 15202 || n = 15206 || n = 15207 || (n > 15208 && n < 15212) || n = 15213 || n = 15215 || n = 15218 || n = 15219 || n = 15226 || n = 15228 || n = 15230 || n = 15232 || n = 15234)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15197)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15198)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15199)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #120
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 15201)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 684
	inx
	Jne *-6
.elseif(n = 15203)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15204)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15205)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15208)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15212)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15214)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15216)
	; 10 bytes
	ldx #84
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 15217)
	; 10 bytes
	ldx #(n-1)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15220)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15221)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15222)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15223)
	; 9 bytes
	ldy #117
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 15224)
	; 10 bytes
	ldx #(n-1172)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15225)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15227)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15229)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15231)
	; 10 bytes
	ldy #56 ;hides 'sec'
	delay_n 265
	dey
	Jne *-7
.elseif(n = 15233)
	; 10 bytes
	ldx #(n-1)/476
	ldy #93
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 15235) && (n < 15274))
; 15235 to 15273
.if(n = 15235)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 284
	inx
	inx
	Jpl *-7
.elseif(n = 15236 || n = 15238 || (n > 15241 && n < 15245) || n = 15246 || n = 15248 || n = 15249 || n = 15254 || (n > 15255 && n < 15259) || n = 15260 || (n > 15261 && n < 15265) || (n > 15266 && n < 15271) || n = 15272)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15237)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15239)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15240)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15241)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15245)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15247)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15250)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15251)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15252)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15253)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15255)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15259)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 211
	iny
	Jne *-7
.elseif(n = 15261)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15265)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15266)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15271)
	; 10 bytes
	ldx #184 ;hides 'clv'
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 15273)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 15274 to 15312
.if(n = 15274 || (n > 15277 && n < 15281) || n = 15282 || n = 15284 || n = 15285 || n = 15290 || n = 15291 || (n > 15292 && n < 15296) || n = 15298 || n = 15299 || (n > 15301 && n < 15307) || n = 15308)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15275)
	; 10 bytes
	ldx #(n-1)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15276)
	; 10 bytes
	ldx #(n-612)/611
	ldy #121
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15277)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15281)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15283)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15286)
	; 9 bytes
	ldy #15
	delay_n 1014
	dey
	Jne *-5
.elseif(n = 15287)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 15288)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15289)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15292)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #72
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 15296)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15297)
	; 10 bytes
	ldx #(n-1)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15300)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #27
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 15301)
	; 10 bytes
	ldx #(n-1)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15307)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 171
	iny
	Jne *-7
.elseif(n = 15309)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15310)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15311)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #126
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 15312)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
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
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15314 || n = 15317 || n = 15322 || n = 15325 || n = 15326 || n = 15329 || n = 15330 || (n > 15331 && n < 15335) || n = 15336 || (n > 15337 && n < 15341) || (n > 15341 && n < 15345) || (n > 15346 && n < 15352))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15315)
	; 10 bytes
	ldx #19
	ldy #160
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15316)
	; 10 bytes
	ldx #(n-1)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15318)
	; 9 bytes
	ldx #53
	delay_n 284
	dex
	Jne *-5
.elseif(n = 15319)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15320)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15321)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15323)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #121
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 15327)
	; 10 bytes
	ldx #96
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 15328)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15331)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15335)
	; 10 bytes
	ldx #(n-1)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15337)
	; 10 bytes
	ldx #(n-1)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15341)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15345)
	; 10 bytes
	ldx #(n-1)/1096
	ldy #218
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15346)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 15352 to 15390
.if(n = 15352)
	; 9 bytes
	ldy #119
	delay_n 124
	dey
	Jne *-5
.elseif(n = 15353)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15354)
	; 10 bytes
	ldx #(n-1182)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15355)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #138
	dey
	Jne *-1
	inx
	Jne *-7
.elseif((n > 15355 && n < 15359) || n = 15360 || n = 15362 || (n > 15363 && n < 15367) || n = 15372 || n = 15374 || n = 15376 || n = 15378 || (n > 15380 && n < 15385) || n = 15387 || n = 15388 || n = 15390)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15359)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #23
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 15361)
	; 9 bytes
	ldy #39
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 15363)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15367)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15368)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15369)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15370)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15371)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15373)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15375)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
	dex
	Jne *-7
.elseif(n = 15377)
	; 10 bytes
	ldx #(n-1)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15379)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15380)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15385)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15386)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15389)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	dex
	Jmi *-7
.endif
.endif
.else
.if((n >= 15391) && (n < 15430))
; 15391 to 15429
.if(n = 15391)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15392 || n = 15394 || (n > 15396 && n < 15400) || n = 15402 || (n > 15404 && n < 15408) || (n > 15410 && n < 15415) || n = 15416 || n = 15420 || n = 15422 || n = 15426 || n = 15428)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15393)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15395)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15396)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #32
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 15400)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15401)
	; 10 bytes
	ldx #24
	ldy #121
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15403)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15404)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15408)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15409)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15410)
	; 10 bytes
	ldx #(n-1)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15415)
	; 10 bytes
	ldx #(n-1)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15417)
	; 10 bytes
	ldx #(n-377)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15418)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15419)
	; 10 bytes
	ldx #(n-1187)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15421)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15423)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15424)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15425)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15427)
	; 10 bytes
	ldy #24 ;hides 'clc'
	delay_n 124
	iny
	iny
	Jne *-7
.elseif(n = 15429)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 15430 to 15468
.if(n = 15430)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15431)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 15432 || n = 15435 || n = 15438 || n = 15440 || n = 15442 || n = 15444 || n = 15446 || n = 15447 || n = 15450 || n = 15452 || n = 15453 || n = 15455 || n = 15458 || (n > 15459 && n < 15465) || n = 15468)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15433)
	; 10 bytes
	ldx #<((n-1)/1286)
	ldy #>((n-1)/1286)
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15434)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15436)
	; 10 bytes
	ldx #(n-442)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15437)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 76
	dey
	Jne *-7
.elseif(n = 15439)
	; 9 bytes
	ldy #216 ;hides 'cld'
	delay_n 379
	iny
	Jne *-6
.elseif(n = 15441)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15443)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 279
	iny
	Jne *-7
.elseif(n = 15445)
	; 10 bytes
	ldx #(n-157)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15448)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15449)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #121
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 15451)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15454)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15456)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15457)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15459)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15465)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #139
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 15466)
	; 10 bytes
	ldx #(n-1)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15467)
	; 10 bytes
	ldx #209
	delay_n 69
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 15469) && (n < 15547))
.if((n >= 15469) && (n < 15508))
; 15469 to 15507
.if(n = 15469 || n = 15470 || (n > 15472 && n < 15477) || n = 15482 || n = 15486 || (n > 15489 && n < 15495) || n = 15496 || n = 15498 || (n > 15499 && n < 15503) || n = 15504 || n = 15506 || n = 15507)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15471)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15472)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15477)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15478)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15479)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15480)
	; 10 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 379
	dey
	Jmi *-7
.elseif(n = 15481)
	; 9 bytes
	ldy #120
	delay_n 124
	dey
	Jne *-5
.elseif(n = 15483)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15484)
	; 10 bytes
	ldx #(n-1192)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15485)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15487)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #23
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 15488)
	; 10 bytes
	ldx #(n-912)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15489)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15495)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15497)
	; 10 bytes
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15499)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15503)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #79
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 15505)
	; 10 bytes
	ldx #(n-1)/456
	ldy #90
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 15508 to 15546
.if((n > 15507 && n < 15511) || (n > 15511 && n < 15516) || n = 15518 || n = 15520 || n = 15522 || n = 15523 || n = 15525 || n = 15527 || (n > 15528 && n < 15532) || (n > 15533 && n < 15537) || n = 15538 || n = 15540 || n = 15542 || n = 15543 || n = 15546)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15511)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15516)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 256
	dey
	Jmi *-7
.elseif(n = 15517)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15519)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #76
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 15521)
	; 10 bytes
	ldx #(n-777)/776
	ldy #154
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15524)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15526)
	; 10 bytes
	ldx #(n-622)/621
	ldy #122
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15528)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 284
	inx
	inx
	Jne *-7
.elseif(n = 15532)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15533)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15537)
	; 10 bytes
	ldx #(n-1)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15539)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 141
	dey
	Jmi *-7
.elseif(n = 15541)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15544)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15545)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if(n = 15547 || n = 15552 || n = 15557 || n = 15559 || n = 15561 || n = 15564 || n = 15566 || n = 15568 || n = 15570 || n = 15572 || n = 15576 || n = 15579 || n = 15580 || n = 15582 || n = 15584 || n = 15585)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15548)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #40
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 15549)
	; 10 bytes
	ldx #(n-677)/676
	ldy #134
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15550)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15551)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15553)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15554)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15555)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15556)
	; 10 bytes
	ldx #(n-1)/61
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15558)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15560)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 15562)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15563)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15565)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 174
	iny
	Jne *-7
.elseif(n = 15567)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15569)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15571)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15573)
	; 10 bytes
	ldx #(n-917)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15574)
	; 10 bytes
	ldx #87
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 15575)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #140
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 15577)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15578)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15581)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15583)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 15586 to 15625
.if((n > 15585 && n < 15589) || (n > 15590 && n < 15595) || (n > 15595 && n < 15599) || (n > 15601 && n < 15605) || n = 15606 || n = 15613 || n = 15620 || n = 15624)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15589)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15590)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15595)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #13
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 15600)
	; 10 bytes
	ldx #(n-1)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15601)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15605)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15607)
	; 9 bytes
	ldx #54
	delay_n 284
	dex
	Jne *-5
.elseif(n = 15608)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 356
	dey
	Jmi *-7
.elseif(n = 15609)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 216
	iny
	Jne *-7
.elseif(n = 15610)
	; 9 bytes
	ldy #121
	delay_n 124
	dey
	Jne *-5
.elseif(n = 15611)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15612)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15614)
	; 10 bytes
	ldx #(n-1202)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15615)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15616)
	; 10 bytes
	ldx #(n-1)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15617)
	; 10 bytes
	ldx #0
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15618)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15619)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15621)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15622)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15623)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15625)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
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
.if(n = 15626 || (n > 15627 && n < 15631) || n = 15632 || n = 15633 || (n > 15634 && n < 15640) || n = 15642 || (n > 15643 && n < 15649) || n = 15650 || n = 15654 || n = 15655 || n = 15659 || n = 15660 || n = 15662)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15627)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15631)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15634)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15640)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15641)
	; 10 bytes
	ldx #(n-1)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15643)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15649)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15651)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15652)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15653)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15656)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15657)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15658)
	; 10 bytes
	ldx #(n-922)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15661)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15663)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15664)
	; 10 bytes
	ldx #(n-682)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 15665 to 15703
.if(n = 15665)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15666)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15667)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif((n > 15667 && n < 15671) || (n > 15671 && n < 15675) || n = 15678 || n = 15680 || (n > 15681 && n < 15685) || n = 15686 || n = 15694 || n = 15696 || n = 15698 || n = 15700 || n = 15701 || n = 15703)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15671)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #128
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 15675)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15676)
	; 10 bytes
	ldy #10
	delay_n 1420
	dey
	Jpl *-6
.elseif(n = 15677)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	dex
	Jmi *-7
.elseif(n = 15679)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15681)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15685)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #141
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 15687)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15688)
	; 10 bytes
	ldx #(n-582)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15689)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15690)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15691)
	; 10 bytes
	ldx #(n-1)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15692)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15693)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #81
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 15695)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15697)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 15702)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #74
	dey
	Jpl *-1
	dex
	Jmi *-7
.endif
.endif
.else
.if((n >= 15704) && (n < 15743))
; 15704 to 15742
.if(n = 15704 || (n > 15705 && n < 15709) || (n > 15709 && n < 15713) || n = 15714 || n = 15716 || n = 15717 || n = 15720 || n = 15722 || n = 15723 || (n > 15724 && n < 15730) || n = 15732 || (n > 15733 && n < 15737) || n = 15738 || n = 15740 || n = 15742)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15705)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15709)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15713)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15715)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15718)
	; 10 bytes
	ldx #38
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 15719)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15721)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15724)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	dex
	Jpl *-7
.elseif(n = 15730)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15731)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15733)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15737)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15739)
	; 9 bytes
	ldy #121
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 15741)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.else
; 15743 to 15781
.if(n = 15743)
	; 10 bytes
	ldx #(n-927)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15744)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15745)
	; 9 bytes
	ldy #40
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 15746 || n = 15750 || n = 15752 || n = 15754 || n = 15756 || (n > 15757 && n < 15763) || n = 15764 || n = 15768 || n = 15770 || n = 15773 || n = 15774 || n = 15778 || n = 15780)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15747)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15748)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15749)
	; 10 bytes
	ldx #31
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 15751)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15753)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15755)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 71
	dey
	Jne *-7
.elseif(n = 15757)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15763)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15765)
	; 10 bytes
	ldx #(n-1)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15766)
	; 10 bytes
	ldx #(n-1)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15767)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 15769)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15771)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15772)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15775)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15776)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15777)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15779)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15781)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 15782) && (n < 15860))
.if((n >= 15782) && (n < 15821))
; 15782 to 15820
.if((n > 15781 && n < 15786) || (n > 15786 && n < 15790) || n = 15792 || n = 15794 || (n > 15796 && n < 15801) || (n > 15801 && n < 15809) || n = 15810 || n = 15812 || (n > 15814 && n < 15821))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15786)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15790)
	; 10 bytes
	ldx #(n-1)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15791)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #130
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 15793)
	; 10 bytes
	ldx #(n-337)/336
	ldy #65
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15795)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15796)
	; 10 bytes
	ldx #(n-1)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15801)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15809)
	; 10 bytes
	ldx #(n-417)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15811)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15813)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15814)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 15821 to 15859
.if(n = 15821)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15822 || (n > 15828 && n < 15834) || n = 15836 || (n > 15837 && n < 15841) || n = 15843 || n = 15844 || n = 15846 || n = 15851 || n = 15853 || n = 15855)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15823)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 15825)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15826)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15827)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15828)
	; 10 bytes
	ldx #(n-932)/931
	ldy #185
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15834)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15835)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15837)
	; 10 bytes
	ldx #214
	delay_n 69
	dex
	Jne *-6
.elseif(n = 15841)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15842)
	; 10 bytes
	ldx #(n-1)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15845)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15847)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 15848)
	; 9 bytes
	ldx #22
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 15849)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15850)
	; 9 bytes
	ldy #248 ;hides 'sed'
	delay_n 124
	dey
	Jmi *-6
.elseif(n = 15852)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15854)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15856)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15857)
	; 10 bytes
	ldx #(n-1)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15858)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15859)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 15860) && (n < 15899))
; 15860 to 15898
.if(n = 15860 || n = 15861 || n = 15864 || n = 15866 || n = 15867 || n = 15869 || n = 15871 || n = 15872 || n = 15875 || n = 15876 || n = 15878 || n = 15879 || n = 15881 || n = 15882 || n = 15884 || n = 15886 || n = 15888 || (n > 15889 && n < 15893) || n = 15895 || n = 15897)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15862)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15863)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15865)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15868)
	; 9 bytes
	ldy #122
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 15870 || n = 15898)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15873)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15874)
	; 10 bytes
	ldx #(n-1)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15877)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15880)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15883)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #82
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 15885)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15887)
	; 10 bytes
	ldx #(n-1)/611
	ldy #121
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15889)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15893)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15894)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15896)
	; 9 bytes
	ldx #55
	delay_n 284
	dex
	Jne *-5
.endif
.else
; 15899 to 15937
.if(n = 15899 || n = 15900 || n = 15902 || n = 15903 || n = 15906 || (n > 15907 && n < 15911) || n = 15912 || n = 15914 || n = 15918 || n = 15920 || (n > 15922 && n < 15926) || n = 15928 || n = 15930 || n = 15933 || n = 15935 || n = 15936)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15901)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15904)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15905)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15907)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15911)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15913)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15915)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15916)
	; 10 bytes
	ldx #(n-1)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15917)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15919)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #77
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 15921)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15922)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15926)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15927)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	inx
	Jne *-7
.elseif(n = 15929)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15931)
	; 10 bytes
	ldx #(n-1)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15932)
	; 10 bytes
	ldx #89
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 15934)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15937)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 15938) && (n < 16094))
.if((n >= 15938) && (n < 16016))
.if((n >= 15938) && (n < 15977))
; 15938 to 15976
.if(n = 15938 || (n > 15940 && n < 15948) || (n > 15949 && n < 15954) || (n > 15954 && n < 15958) || n = 15960 || (n > 15961 && n < 15973))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15939)
	; 10 bytes
	ldx #(n-1227)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15940)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15948)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15949)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15954)
	; 10 bytes
	ldx #(n-1)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15958)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15959)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15961)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15973)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15974)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 206
	dey
	Jmi *-7
.elseif(n = 15975)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15976)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 15977 to 16015
.if((n > 15976 && n < 15980) || n = 15981 || n = 15984 || (n > 15985 && n < 15989) || (n > 15991 && n < 15997) || (n > 15999 && n < 16003) || n = 16005 || n = 16008 || (n > 16009 && n < 16015))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15980)
	; 10 bytes
	ldx #(n-1)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 15982)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15983)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15985)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15989)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15990)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 15991)
	; 10 bytes
	ldx #(n-1)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 15997)
	; 9 bytes
	ldy #123
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 15998)
	; 10 bytes
	ldx #(n-942)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 15999)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16003)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16004)
	; 10 bytes
	ldx #(n-1232)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16006)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16007)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16009)
	; 10 bytes
	ldx #(n-277)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16015)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 16016) && (n < 16055))
; 16016 to 16054
.if(n = 16016 || n = 16019 || n = 16020 || (n > 16021 && n < 16026) || n = 16027 || n = 16029 || (n > 16033 && n < 16037) || n = 16040 || (n > 16041 && n < 16045) || n = 16046 || n = 16048 || n = 16050 || (n > 16051 && n < 16055))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16017)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16018)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16021)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16026)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16028)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16030)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16031)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #132
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 16032)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16033)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16037)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16038)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 366
	dey
	Jmi *-7
.elseif(n = 16039)
	; 10 bytes
	ldx #(n-1)/486
	ldy #96
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16041)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16045)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16047)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16049)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16051)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 16055 to 16093
.if(n = 16055)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #43
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 16056 || n = 16058 || n = 16062 || n = 16063 || n = 16067 || n = 16068 || (n > 16069 && n < 16073) || (n > 16075 && n < 16081) || n = 16082 || (n > 16083 && n < 16087) || (n > 16087 && n < 16091))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16057)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16059)
	; 10 bytes
	ldx #217
	delay_n 69
	dex
	Jne *-6
.elseif(n = 16060)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16061)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16064)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 146
	dey
	Jmi *-7
.elseif(n = 16065)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16066)
	; 10 bytes
	ldx #15
	ldy #213
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16069)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16073)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 16075)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16081)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16083)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16087)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16091)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 291
	iny
	Jne *-7
.elseif(n = 16092)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #25
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 16093)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 16094) && (n < 16172))
.if((n >= 16094) && (n < 16133))
.if((n >= 16094) && (n < 16113))
; 16094 to 16112
.if(n = 16094)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16095 || (n > 16097 && n < 16101) || n = 16102 || n = 16104 || n = 16107 || n = 16109)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16096)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16097)
	; 10 bytes
	ldx #(n-1)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16101)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16103)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16105)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16106)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 266
	dey
	Jmi *-7
.elseif(n = 16108)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16110)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16111)
	; 10 bytes
	ldx #89
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 16112)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #76
	dey
	Jpl *-1
	dex
	Jmi *-7
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16114)
	; 10 bytes
	ldx #(n-132)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16115)
	; 10 bytes
	ldx #(n-1)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16116 || n = 16122 || n = 16123 || n = 16132)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16117)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16118)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16119)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #79
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 16120)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16121)
	; 10 bytes
	ldx #(n-807)/806
	ldy #160
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16124)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16125)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16126)
	; 9 bytes
	ldy #124
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 16127)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 16128 || n = 16131)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16129)
	; 9 bytes
	ldy #42
	delay_n 379
	dey
	Jne *-5
.elseif(n = 16130)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-7
.endif
.endif
.else
; 16133 to 16171
.if(n = 16133)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16134)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16135 || (n > 16136 && n < 16141) || (n > 16141 && n < 16147) || n = 16149 || n = 16150 || (n > 16152 && n < 16158) || n = 16159 || n = 16160 || (n > 16161 && n < 16167))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16136)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16141)
	; 10 bytes
	ldx #(n-1)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16147)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16148)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16151)
	; 10 bytes
	ldx #(n-647)/646
	ldy #127
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16152)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16158)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16161)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16167)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 181
	iny
	Jne *-7
.elseif(n = 16168)
	; 10 bytes
	ldx #(n-952)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16169)
	; 10 bytes
	ldx #(n-377)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16170)
	; 10 bytes
	ldx #19
	ldy #169
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16171)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 16172) && (n < 16211))
; 16172 to 16210
.if(n = 16172 || (n > 16173 && n < 16177) || (n > 16177 && n < 16183) || n = 16184 || n = 16186 || n = 16188 || n = 16190 || (n > 16193 && n < 16199) || n = 16200 || n = 16202 || n = 16203 || n = 16205 || n = 16206 || (n > 16207 && n < 16211))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16173)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16177)
	; 10 bytes
	ldx #(n-1)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16183)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16185)
	; 9 bytes
	ldx #56
	delay_n 284
	dex
	Jne *-5
.elseif(n = 16187)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16189)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16191)
	; 10 bytes
	ldx #184 ;hides 'clv'
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 16192)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16193)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16199)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16201)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16204)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16207)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 16211 to 16250
.if(n = 16211 || n = 16212 || n = 16214 || n = 16217 || n = 16218 || (n > 16221 && n < 16225) || n = 16226 || n = 16229 || n = 16230 || (n > 16231 && n < 16235) || n = 16238 || n = 16240 || (n > 16241 && n < 16245) || n = 16247 || n = 16249 || n = 16250)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16213)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16215)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16216)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16219)
	; 10 bytes
	ldx #(n-1)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16220)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16221)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16225)
	; 10 bytes
	ldx #(n-417)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16227)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16228)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16231)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16235)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #146
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 16236)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16237)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16239)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16241)
	; 10 bytes
	ldx #(n-1)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16245)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16246)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16248)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.if(n = 16251 || n = 16252 || n = 16254 || n = 16256 || n = 16258 || n = 16259 || (n > 16265 && n < 16269) || n = 16272 || n = 16274 || n = 16279 || n = 16280 || n = 16283 || n = 16284 || (n > 16286 && n < 16290))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16253)
	; 10 bytes
	ldx #(n-957)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16255)
	; 9 bytes
	ldy #126
	delay_n 124
	dey
	Jne *-5
.elseif(n = 16257)
	; 10 bytes
	ldx #(n-1)/1016
	ldy #202
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16260)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16261)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16262)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16263)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16264)
	; 10 bytes
	ldx #(n-1252)/1251
	ldy #249
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16265)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16269)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16270)
	; 10 bytes
	ldx #29
	ldy #111
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16271)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #134
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 16273)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16275)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16276)
	; 10 bytes
	ldx #24
	ldy #128
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16277)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16278)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #43
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 16281)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16282)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16285)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16286)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #35
	dey
	Jne *-1
	dex
	Jmi *-7
.endif
.else
; 16290 to 16328
.if(n = 16290)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16291)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif((n > 16291 && n < 16295) || n = 16296 || n = 16298 || n = 16300 || n = 16302 || n = 16304 || n = 16306 || n = 16308 || n = 16310 || (n > 16311 && n < 16316) || n = 16320 || n = 16322 || n = 16323 || (n > 16325 && n < 16329))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16295)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 284
	dex
	Jne *-6
.elseif(n = 16297)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16299)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16301)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16303)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16305)
	; 9 bytes
	ldy #15
	delay_n 1014
	dey
	Jpl *-5
.elseif(n = 16307)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16309)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16311)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16316)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16317)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #77
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 16318)
	; 10 bytes
	ldx #(n-442)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #79
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 16321)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16324)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #128
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 16325)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 16329) && (n < 16368))
; 16329 to 16367
.if(n = 16329)
	; 10 bytes
	ldx #(n-1257)/1256
	ldy #250
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16330 || (n > 16331 && n < 16336) || n = 16339 || n = 16340 || (n > 16341 && n < 16345) || (n > 16345 && n < 16349) || (n > 16349 && n < 16353) || n = 16356 || n = 16358 || n = 16359 || n = 16362 || n = 16364 || n = 16365)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16331)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16336)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16337)
	; 10 bytes
	ldx #(n-1)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16338)
	; 10 bytes
	ldx #(n-962)/961
	ldy #191
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16341)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16345)
	; 10 bytes
	ldx #(n-1)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16349)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 211
	dey
	Jmi *-7
.elseif(n = 16353)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16354)
	; 10 bytes
	ldx #(n-712)/711
	ldy #141
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16355)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16357)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16360)
	; 10 bytes
	ldx #(n-1)/861
	ldy #171
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16361)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 296
	iny
	Jne *-7
.elseif(n = 16363)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16366)
	; 10 bytes
	ldx #(n-1)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16367)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 81
	dey
	Jne *-7
.endif
.else
; 16368 to 16406
.if(n = 16368 || n = 16371 || (n > 16372 && n < 16376) || n = 16378 || n = 16380 || n = 16382 || n = 16383 || n = 16387 || n = 16388 || n = 16390 || n = 16392 || n = 16393 || (n > 16395 && n < 16399) || n = 16400 || (n > 16402 && n < 16407))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16369)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16370)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #28
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 16372)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16376)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16377)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16379)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16381)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16384)
	; 9 bytes
	ldy #126
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 16385)
	; 10 bytes
	ldx #>(n+48)
	ldy #<(n+48)
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16386)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16389)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16391)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #135
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 16394)
	; 10 bytes
	ldx #(n-1262)/1261
	ldy #251
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16395)
	; 10 bytes
	ldx #(n-1)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16399)
	; 10 bytes
	ldx #(n-912)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16401)
	; 10 bytes
	ldx #(n-657)/656
	ldy #130
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16402)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 16407) && (n < 16485))
.if((n >= 16407) && (n < 16446))
; 16407 to 16445
.if(n = 16407)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16408 || (n > 16409 && n < 16413) || n = 16414 || n = 16416 || n = 16418 || n = 16419 || n = 16422 || (n > 16423 && n < 16427) || n = 16428 || n = 16430 || n = 16432 || n = 16434 || (n > 16435 && n < 16441) || n = 16443 || n = 16445)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16409)
	; 10 bytes
	ldx #(n-587)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16413)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16415)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16417)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16420)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16421)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16423)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16427)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16429)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16431)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16433)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16435)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16441)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16442)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16444)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 16446 to 16484
.if((n > 16445 && n < 16449) || (n > 16449 && n < 16453) || n = 16454 || n = 16456 || n = 16458 || n = 16461 || n = 16466 || n = 16467 || n = 16470 || n = 16475 || (n > 16476 && n < 16481) || n = 16482)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16449)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #130
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 16453)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #84
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 16455)
	; 10 bytes
	ldx #(n-1)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16457)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16459)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16460)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16462)
	; 10 bytes
	ldx #(n-1)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16463)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 16464)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16465)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16468)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 376
	dey
	Jmi *-7
.elseif(n = 16469)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16471)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16472)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16473)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16474)
	; 9 bytes
	ldx #56
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 16476)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16481)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16483)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16484)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 16485) && (n < 16524))
; 16485 to 16523
.if(n = 16485 || n = 16487 || n = 16488 || n = 16490 || n = 16492 || (n > 16493 && n < 16497) || n = 16500 || n = 16502 || (n > 16503 && n < 16507) || n = 16509 || n = 16510 || n = 16512 || n = 16514 || n = 16517 || n = 16518 || n = 16520)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16486)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16489)
	; 10 bytes
	ldx #(n-917)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16491)
	; 10 bytes
	ldy #248 ;hides 'sed'
	delay_n 124
	dey
	dey
	Jne *-7
.elseif(n = 16493)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16497)
	; 10 bytes
	ldx #(n-1)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16498)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16499)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16501)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16503)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16507)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16508)
	; 10 bytes
	ldx #(n-972)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16511)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #136
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 16513)
	; 9 bytes
	ldy #43
	delay_n 379
	dey
	Jne *-5
.elseif(n = 16515)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16516)
	; 10 bytes
	ldx #(n-1)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16519)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #81
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 16521)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16522)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #79
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 16523)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 16524 to 16562
.if(n = 16524)
	; 10 bytes
	ldx #(n-1272)/1271
	ldy #253
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16525)
	; 10 bytes
	ldx #(n-1)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16526)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif((n > 16526 && n < 16531) || n = 16533 || n = 16536 || n = 16538 || n = 16541 || n = 16542 || (n > 16545 && n < 16549) || (n > 16551 && n < 16555) || n = 16557 || n = 16558 || n = 16562)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16531)
	; 10 bytes
	ldx #(n-1)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16532)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16534)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16535)
	; 10 bytes
	ldx #(n-1)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16537)
	; 9 bytes
	ldx #24
	delay_n 684
	dex
	Jne *-5
.elseif(n = 16539)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16540)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16543)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #35
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 16544)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16545)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16549)
	; 10 bytes
	ldx #(n-592)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16550)
	; 10 bytes
	ldx #(n-1)/871
	ldy #173
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16551)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16555)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16556)
	; 10 bytes
	ldx #(n-1)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16559)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #26
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 16560)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16561)
	; 10 bytes
	ldx #(n-1)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
.if((n >= 16563) && (n < 16602))
; 16563 to 16601
.if(n = 16563)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16564 || n = 16566 || n = 16568 || (n > 16569 && n < 16574) || n = 16575 || n = 16581 || n = 16582 || n = 16587 || n = 16590 || n = 16592 || (n > 16593 && n < 16597) || n = 16598 || n = 16600)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16565)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16567)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16569)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 16576)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16577)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16578)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16579)
	; 10 bytes
	ldx #(n-922)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16580)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16583)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 684
	dex
	Jne *-6
.elseif(n = 16584)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16585)
	; 10 bytes
	ldx #(n-1)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16586)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 284
	dex
	Jpl *-6
.elseif(n = 16588)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16589)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16591)
	; 10 bytes
	ldx #15
	ldy #220
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16593)
	; 10 bytes
	ldx #(n-137)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16597)
	; 9 bytes
	ldy #170 ;hides 'tax'
	delay_n 379
	dey
	Jmi *-6
.elseif(n = 16599)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16601)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 16602 to 16640
.if(n = 16602 || n = 16603 || n = 16608 || n = 16610 || n = 16613 || n = 16614 || n = 16616 || n = 16619 || n = 16620 || (n > 16621 && n < 16627) || n = 16628 || n = 16629 || n = 16632 || (n > 16633 && n < 16638) || n = 16640)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16604)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	dex
	Jmi *-7
.elseif(n = 16605)
	; 10 bytes
	ldx #(n-1)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16606)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16607)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16609)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16611)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16612)
	; 10 bytes
	ldx #(n-792)/791
	ldy #157
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16615)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16617)
	; 10 bytes
	ldx #(n-1)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16618)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16621)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16627)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16630)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16631)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 301
	iny
	Jne *-7
.elseif(n = 16633)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16638)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16639)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 16641) && (n < 16680))
; 16641 to 16679
.if(n = 16641)
	; 10 bytes
	ldx #>(n+48)
	ldy #<(n+48)
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16642)
	; 9 bytes
	ldy #128
	delay_n 124
	dey
	Jpl *-5
.elseif(n = 16643)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16644)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16645)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16646 || n = 16649 || n = 16650 || n = 16652 || n = 16655 || n = 16656 || n = 16658 || n = 16660 || (n > 16661 && n < 16666) || n = 16668 || n = 16671 || n = 16672 || n = 16674 || n = 16676 || n = 16679)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16647)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16648)
	; 10 bytes
	ldx #93
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 16651)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16653)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16654)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16657)
	; 10 bytes
	ldx #(n-1)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16659)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 91
	dey
	Jne *-7
.elseif(n = 16661)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16666)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16667)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16669)
	; 10 bytes
	ldx #(n-927)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16670)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16673)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16675)
	; 10 bytes
	ldx #(n-1)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16677)
	; 10 bytes
	ldx #10
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 16678)
	; 10 bytes
	ldx #(n-982)/981
	ldy #195
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 16680 to 16718
.if(n = 16680 || n = 16682 || (n > 16689 && n < 16693) || n = 16694 || n = 16698 || (n > 16700 && n < 16705) || (n > 16705 && n < 16710) || (n > 16710 && n < 16717) || n = 16718)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16681)
	; 10 bytes
	ldx #(n-1)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16683)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 381
	dey
	Jmi *-7
.elseif(n = 16684)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16685)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16686)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16687)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16688)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16689)
	; 10 bytes
	ldx #(n-597)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16693)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16695)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16696)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16697)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #26
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 16699)
	; 10 bytes
	ldx #(n-1)/66
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16700)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 16705)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16710)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16717)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 16719) && (n < 16797))
.if((n >= 16719) && (n < 16758))
; 16719 to 16757
.if(n = 16719)
	; 10 bytes
	ldx #<((n-1287)/1286)
	ldy #>((n-1287)/1286)
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16720 || n = 16722 || n = 16723 || n = 16726 || n = 16728 || n = 16730 || n = 16732 || (n > 16733 && n < 16737) || n = 16738 || n = 16739 || n = 16742 || n = 16744 || (n > 16745 && n < 16749) || n = 16750 || (n > 16751 && n < 16757))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16721)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16724)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 216
	dey
	Jmi *-7
.elseif(n = 16725)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16727)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 16729)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16731)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #35
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 16733)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16737)
	; 10 bytes
	ldx #(n-1)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16740)
	; 10 bytes
	ldx #(n-1)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16741)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16743)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16745)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16749)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16751)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #138
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 16757)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 16758 to 16796
.if(n = 16758 || n = 16760 || n = 16761 || n = 16764 || n = 16766 || n = 16770 || n = 16774 || n = 16778 || n = 16779 || (n > 16780 && n < 16785) || (n > 16785 && n < 16791) || (n > 16791 && n < 16795) || n = 16796)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16759)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16762)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16763)
	; 9 bytes
	ldx #57
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 16765)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16767)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16768)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16769)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16771)
	; 9 bytes
	ldy #130
	delay_n 124
	dey
	Jne *-5
.elseif(n = 16772)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16773)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16775)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #45
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 16776)
	; 10 bytes
	ldx #(n-672)/671
	ldy #133
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16777)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16780)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16785)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #151
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 16791)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16795)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 16797) && (n < 16836))
; 16797 to 16835
.if(n = 16797)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16798 || n = 16800 || n = 16802 || (n > 16803 && n < 16809) || (n > 16810 && n < 16814) || n = 16819 || n = 16823 || n = 16826 || n = 16830 || n = 16832)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16799)
	; 10 bytes
	ldx #227
	delay_n 69
	dex
	Jne *-6
.elseif(n = 16801)
	; 10 bytes
	ldx #(n-1)/336
	ldy #65
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16803)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16809)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16810)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16814)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16815)
	; 10 bytes
	ldx #(n-1)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16816)
	; 10 bytes
	ldx #(n-1)/1121
	ldy #223
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16817)
	; 10 bytes
	ldx #(n-1)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16818)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16820)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16821)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16822)
	; 10 bytes
	ldx #(n-802)/801
	ldy #159
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #133
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 16825)
	; 10 bytes
	ldx #(n-1)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16827)
	; 10 bytes
	ldx #94
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 16828)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16829)
	; 10 bytes
	ldx #(n-602)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16831)
	; 10 bytes
	ldx #(n-1)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16833)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16834)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16835)
	; 10 bytes
	ldx #(n-1)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 16836 to 16875
.if(n = 16836)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16837)
	; 10 bytes
	ldx #(n-277)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif((n > 16837 && n < 16841) || n = 16842 || (n > 16843 && n < 16847) || n = 16851 || n = 16854 || n = 16856 || (n > 16857 && n < 16861) || n = 16862 || n = 16866 || n = 16867 || n = 16869 || n = 16870 || n = 16872 || n = 16874)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16841)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16843)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16847)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 16848)
	; 10 bytes
	ldx #(n-992)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16849)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16850)
	; 10 bytes
	ldx #(n-1)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16852)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16853)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16855)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16857)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16861)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16863)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #12
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 16864)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16865)
	; 10 bytes
	ldx #(n-1)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16868)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16871)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16873)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16875)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.if((n > 16875 && n < 16879) || n = 16880 || n = 16883 || n = 16884 || (n > 16885 && n < 16891) || n = 16892 || n = 16894 || n = 16896 || n = 16904 || (n > 16907 && n < 16913) || n = 16914)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16879)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16881)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16882)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16885)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16891)
	; 10 bytes
	ldx #(n-1)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16893)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16895)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #152
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 16897)
	; 9 bytes
	ldy #44
	delay_n 379
	dey
	Jne *-5
.elseif(n = 16898)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 386
	dey
	Jmi *-7
.elseif(n = 16899)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16900)
	; 9 bytes
	ldy #131
	delay_n 124
	dey
	Jne *-5
.elseif(n = 16901)
	; 10 bytes
	ldx #(n-677)/676
	ldy #134
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16902)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16903)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16905)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #29
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 16906)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16907)
	; 10 bytes
	ldx #106
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 16913)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 16915 to 16953
.if((n > 16914 && n < 16919) || n = 16920 || (n > 16921 && n < 16927) || n = 16928 || n = 16931 || n = 16934 || n = 16936 || n = 16938 || n = 16940 || n = 16942 || n = 16944 || n = 16946 || n = 16948 || n = 16950 || n = 16952)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16919)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #82
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 16921)
	; 10 bytes
	ldx #(n-1)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16927)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16929)
	; 10 bytes
	ldx #(n-737)/736
	ldy #146
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16930)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16932)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #81
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 16933)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16935)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #13
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 16937)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16939)
	; 10 bytes
	ldx #(n-942)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16941)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16943)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16945)
	; 10 bytes
	ldx #(n-1)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16947)
	; 10 bytes
	ldx #229
	delay_n 69
	dex
	Jne *-6
.elseif(n = 16949)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16951)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16953)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 16954) && (n < 16993))
; 16954 to 16992
.if(n = 16954 || n = 16956 || (n > 16957 && n < 16961) || n = 16962 || n = 16964 || n = 16965 || (n > 16970 && n < 16975) || n = 16976 || (n > 16977 && n < 16981) || n = 16982 || (n > 16985 && n < 16991) || n = 16992)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16955)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16957)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16961)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16963)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16966)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16967)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 16968)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16969)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 16970)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16975)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16977)
	; 10 bytes
	ldx #(n-1)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16981)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16983)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #37
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 16984)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 16985)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 16991)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 281
	dey
	Jmi *-7
.endif
.else
; 16993 to 17031
.if(n = 16993)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 16994 || (n > 16995 && n < 17000) || (n > 17001 && n < 17005) || n = 17007 || n = 17010 || n = 17012 || n = 17013 || n = 17016 || n = 17017 || n = 17019 || n = 17020 || n = 17022 || n = 17024 || n = 17028 || n = 17030)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16995)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17000)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17001)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17005)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17006)
	; 10 bytes
	ldx #94
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 17008)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #45
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 17009)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 236
	iny
	Jne *-7
.elseif(n = 17011)
	; 10 bytes
	ldx #(n-487)/486
	ldy #96
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17014)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17015)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17018)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17021)
	; 10 bytes
	ldx #(n-852)/851
	ldy #169
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17023)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #87
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 17025)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17026)
	; 10 bytes
	ldx #(n-682)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17027)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 191
	iny
	Jne *-7
.elseif(n = 17029)
	; 9 bytes
	ldy #132
	delay_n 124
	dey
	Jne *-5
.elseif(n = 17031)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
.if((n >= 17032) && (n < 17071))
; 17032 to 17070
.if(n = 17032)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17033 || n = 17034 || n = 17036 || n = 17037 || n = 17039 || n = 17040 || n = 17042 || n = 17043 || (n > 17044 && n < 17051) || n = 17055 || n = 17056 || (n > 17058 && n < 17062) || n = 17063 || n = 17066 || n = 17068)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17035)
	; 10 bytes
	ldx #(n-1)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17038)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17041)
	; 10 bytes
	ldx #(n-1)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17044)
	; 10 bytes
	ldx #(n-742)/741
	ldy #147
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17051)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17052)
	; 9 bytes
	ldx #58
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 17053)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17054)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17057)
	; 10 bytes
	ldx #(n-417)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17058)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17062)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17064)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17065)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17067)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17069)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17070)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 17071 to 17109
.if(n = 17071 || n = 17072 || (n > 17074 && n < 17081) || n = 17084 || n = 17085 || n = 17087 || n = 17088 || n = 17090 || n = 17091 || n = 17093 || n = 17094 || (n > 17095 && n < 17099) || n = 17100 || n = 17102 || n = 17104 || n = 17105 || n = 17107 || n = 17108)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17073)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17074)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17081)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17082)
	; 10 bytes
	ldx #(n-1)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17083)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17086)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17089)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17092)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17095)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17099)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17101)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17103)
	; 10 bytes
	ldx #(n-1007)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17106)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17109)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 17110) && (n < 17149))
; 17110 to 17148
.if(n = 17110 || n = 17117 || n = 17118 || (n > 17121 && n < 17131) || (n > 17131 && n < 17135) || n = 17136 || n = 17138 || n = 17141 || (n > 17142 && n < 17146) || n = 17147 || n = 17148)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17111)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #141
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17112)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17113)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17114)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 156
	dey
	Jmi *-7
.elseif(n = 17115)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17116)
	; 10 bytes
	ldx #15
	ldy #227
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17119)
	; 10 bytes
	ldx #(n-952)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17120)
	; 10 bytes
	ldx #(n-1)/901
	ldy #179
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17121)
	; 10 bytes
	ldx #(n-857)/856
	ldy #170
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17131)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17135)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #45
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 17137)
	; 10 bytes
	ldx #(n-1)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17139)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17140)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17142)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17146)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 17149 to 17187
.if(n = 17149 || n = 17150 || n = 17152 || (n > 17153 && n < 17158) || n = 17163 || n = 17164 || (n > 17165 && n < 17169) || n = 17172 || n = 17174 || n = 17175 || (n > 17177 && n < 17183) || n = 17184)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17151)
	; 10 bytes
	ldx #(n-687)/686
	ldy #136
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17153)
	; 10 bytes
	ldx #>(n+48)
	ldy #<(n+48)
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17158)
	; 9 bytes
	ldy #133
	delay_n 124
	dey
	Jne *-5
.elseif(n = 17159)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17160)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17161)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17162)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17165)
	; 10 bytes
	ldx #(n-1)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17169)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17170)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17171)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17173)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17176)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #37
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 17177)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17183)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17185)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17186)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17187)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
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
	; 10 bytes
	ldx #(n-1012)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17189 || n = 17190 || n = 17192 || n = 17193 || n = 17195 || n = 17197 || n = 17198 || n = 17202 || n = 17204 || n = 17207 || n = 17208 || (n > 17209 && n < 17213) || n = 17214 || n = 17216 || (n > 17217 && n < 17221) || n = 17222 || n = 17224)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17191)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17194)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17196)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17199)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #136
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 17200)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17201)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17203)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17205)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17206)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17209)
	; 10 bytes
	ldx #(n-957)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17213)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17215)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17217)
	; 10 bytes
	ldx #(n-1)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17221)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17223)
	; 10 bytes
	ldx #108
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 17225)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #155
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 17226)
	; 9 bytes
	ldx #24
	delay_n 684
	dex
	Jpl *-5
.endif
.else
; 17227 to 17265
.if(n = 17227)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17228)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17229)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17230 || (n > 17231 && n < 17235) || n = 17236 || n = 17238 || n = 17240 || n = 17245 || n = 17246 || n = 17248 || (n > 17249 && n < 17253) || (n > 17254 && n < 17262) || n = 17264)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17231)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #142
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17235)
	; 10 bytes
	ldx #(n-1)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17237)
	; 10 bytes
	ldx #(n-1)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17239)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17241)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17242)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17243)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17244)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17247)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #60
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17249)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17253)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17254)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17262)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17263)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17265)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 17266) && (n < 17305))
; 17266 to 17304
.if(n = 17266)
	; 10 bytes
	ldx #(n-1)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jne *-6
.elseif((n > 17266 && n < 17270) || n = 17271 || n = 17275 || n = 17277 || n = 17278 || n = 17280 || n = 17282 || n = 17284 || n = 17288 || n = 17290 || n = 17292 || (n > 17293 && n < 17297) || n = 17298 || n = 17300)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17270)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 17272)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17273)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17274)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 684
	dex
	Jpl *-6
.elseif(n = 17276)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17279)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17281)
	; 9 bytes
	ldy #45
	delay_n 379
	dey
	Jne *-5
.elseif(n = 17283 || n = 17289)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17285)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17286)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 286
	dey
	Jmi *-7
.elseif(n = 17287)
	; 9 bytes
	ldy #134
	delay_n 124
	dey
	Jne *-5
.elseif(n = 17291)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17293)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17297)
	; 10 bytes
	ldx #(n-377)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17299)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17301)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17302)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #27
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 17303)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17304)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 17305 to 17343
.if(n = 17305)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17306 || n = 17307 || n = 17309 || n = 17311 || n = 17312 || (n > 17313 && n < 17317) || n = 17318 || n = 17322 || n = 17323 || n = 17327 || (n > 17330 && n < 17335) || (n > 17335 && n < 17341))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17308)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17310)
	; 10 bytes
	ldx #(n-1)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17313)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17317)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #84
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 17320)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17321)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17324)
	; 9 bytes
	ldy #16
	delay_n 1014
	dey
	Jpl *-5
.elseif(n = 17325)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17326)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17328)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 396
	dey
	Jmi *-7
.elseif(n = 17329)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17330)
	; 10 bytes
	ldx #42
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 17335)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17341)
	; 9 bytes
	ldx #60
	delay_n 284
	dex
	Jne *-5
.elseif(n = 17342)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #82
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 17343)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
; 17344 to 17382
.if(n = 17344)
	; 10 bytes
	ldx #(n-142)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17345)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17346 || (n > 17347 && n < 17351) || n = 17352 || (n > 17353 && n < 17358) || n = 17360 || n = 17362 || n = 17363 || n = 17365 || (n > 17366 && n < 17371) || n = 17372 || n = 17374 || n = 17376 || (n > 17377 && n < 17381))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17347)
	; 10 bytes
	ldx #(n-827)/826
	ldy #164
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17351)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #143
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17353)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17358)
	; 10 bytes
	ldx #(n-1022)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17359)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17361)
	; 10 bytes
	ldx #(n-497)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17364)
	; 10 bytes
	ldx #96
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 17366)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17371)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17373)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17375)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17377)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17381)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17382)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 17383 to 17421
.if(n = 17383 || n = 17384 || (n > 17385 && n < 17389) || n = 17390 || n = 17393 || n = 17394 || n = 17397 || n = 17398 || n = 17400 || n = 17402 || n = 17404 || n = 17406 || n = 17408 || (n > 17409 && n < 17414) || n = 17415 || n = 17419 || n = 17420)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17385)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17389)
	; 10 bytes
	ldx #(n-277)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17391)
	; 10 bytes
	ldx #235
	delay_n 69
	dex
	Jne *-6
.elseif(n = 17392)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17395)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17396)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17399)
	; 10 bytes
	ldx #232 ;hides 'inx'
	php
	delay_n 684
	plp
	Jne *-7
.elseif(n = 17401)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17403)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17405)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17407)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 17409)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17414)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 398
	dey
	Jmi *-7
.elseif(n = 17416)
	; 9 bytes
	ldy #135
	delay_n 124
	dey
	Jne *-5
.elseif(n = 17417)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17418)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17421)
	; 10 bytes
	ldx #(n-872)/871
	ldy #173
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 17422) && (n < 17461))
; 17422 to 17460
.if(n = 17422)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 124
	dey
	dey
	Jne *-7
.elseif(n = 17423)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17424)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17425)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17426 || n = 17427 || n = 17429 || (n > 17431 && n < 17438) || n = 17439 || n = 17442 || n = 17444 || n = 17446 || n = 17448 || n = 17450 || n = 17451 || (n > 17452 && n < 17457) || n = 17458)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17428)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17430)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17431)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17438)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17440)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #31
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 17441)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17443)
	; 10 bytes
	ldx #(n-307)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17445)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17447)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17449)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #138
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 17452)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17457)
	; 10 bytes
	ldx #(n-1)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17459)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17460)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 17461 to 17500
.if(n = 17461)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif((n > 17461 && n < 17465) || n = 17466 || (n > 17467 && n < 17471) || n = 17472 || (n > 17474 && n < 17479) || n = 17480 || (n > 17481 && n < 17485) || n = 17486 || n = 17487 || n = 17489 || n = 17490 || (n > 17491 && n < 17495) || n = 17496 || n = 17498 || n = 17499)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17465)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17467)
	; 10 bytes
	ldx #(n-1)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17471)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #31
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 17473)
	; 10 bytes
	ldx #(n-337)/336
	ldy #65
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17474)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17479)
	; 10 bytes
	ldx #(n-972)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17481)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17485)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17488)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17491)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17495)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #46
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 17497)
	; 10 bytes
	ldx #(n-1)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17500)
	; 10 bytes
	ldx #(n-1)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jne *-6
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
.if((n > 17500 && n < 17504) || (n > 17504 && n < 17509) || n = 17510 || n = 17512 || n = 17516 || n = 17518 || n = 17520 || n = 17522 || n = 17523 || n = 17525 || (n > 17529 && n < 17536))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17504)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17509)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 96
	dey
	Jne *-7
.elseif(n = 17511)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17513)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17514)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17515)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17517)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17519)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17521)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17524)
	; 10 bytes
	ldx #(n-1)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17526)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17527)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17528)
	; 10 bytes
	ldx #(n-1032)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17529)
	; 10 bytes
	ldx #(n-627)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17536)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17537)
	; 10 bytes
	ldx #(n-1)/1096
	ldy #218
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17538)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17539)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 17540 to 17578
.if((n > 17539 && n < 17543) || n = 17548 || n = 17549 || (n > 17551 && n < 17555) || n = 17556 || (n > 17558 && n < 17564) || n = 17565 || n = 17567 || n = 17568 || n = 17570 || n = 17572 || n = 17576)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17543)
	; 10 bytes
	ldx #97
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 17544)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17545)
	; 9 bytes
	ldy #136
	delay_n 124
	dey
	Jne *-5
.elseif(n = 17546)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17547)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17550)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17551)
	; 10 bytes
	ldx #(n-1)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17555)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17557)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17558)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17564)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17566)
	; 10 bytes
	ldx #(n-1)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17569)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17571)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17573)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17574)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 17575)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17577)
	; 10 bytes
	ldx #26
	ldy #134
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17578)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 17579) && (n < 17618))
; 17579 to 17617
.if(n = 17579)
	; 10 bytes
	ldx #136 ;hides 'dey'
	delay_n 284
	inx
	inx
	Jne *-7
.elseif(n = 17580 || (n > 17581 && n < 17585) || (n > 17585 && n < 17590) || n = 17592 || n = 17598 || n = 17600 || (n > 17601 && n < 17605) || (n > 17605 && n < 17609) || (n > 17609 && n < 17613))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17581)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17585)
	; 10 bytes
	ldx #(n-1)/1256
	ldy #250
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17590)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17591)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #145
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17593)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #91
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 17594)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17595)
	; 10 bytes
	ldx #(n-1)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17596)
	; 10 bytes
	ldx #(n-1)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17597)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #15
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 17601)
	; 10 bytes
	ldx #(n-1)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17605)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17609)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17613)
	; 10 bytes
	ldx #(n-477)/476
	ldy #93
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17614)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17615)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	dex
	Jne *-7
.elseif(n = 17616)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17617)
	; 10 bytes
	ldx #(n-1)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 17618 to 17656
.if(n = 17618 || n = 17620 || n = 17622 || n = 17624 || n = 17625 || n = 17627 || n = 17628 || (n > 17633 && n < 17639) || n = 17640 || (n > 17641 && n < 17645) || n = 17646 || n = 17648 || n = 17649 || (n > 17651 && n < 17655))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17619)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17621)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17623)
	; 10 bytes
	ldx #(n-802)/801
	ldy #159
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17626)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17629)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17630)
	; 9 bytes
	ldx #60
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 17631)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17632)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17633)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17639)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 161
	dey
	Jmi *-7
.elseif(n = 17641)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17645)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17647)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17650)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17651)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17655)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17656)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 17657) && (n < 17735))
.if((n >= 17657) && (n < 17696))
; 17657 to 17695
.if(n = 17657 || n = 17658 || n = 17660 || n = 17661 || n = 17663 || n = 17664 || n = 17666 || n = 17670 || n = 17675 || n = 17678 || n = 17679 || n = 17682 || n = 17684 || n = 17685 || n = 17688 || (n > 17690 && n < 17696))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17659)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17662)
	; 10 bytes
	ldx #(n-842)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17665)
	; 9 bytes
	ldy #45
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 17667)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17668)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17669)
	; 10 bytes
	ldx #(n-632)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17671)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17672)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17673)
	; 10 bytes
	ldx #(n-377)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17674)
	; 9 bytes
	ldy #137
	delay_n 124
	dey
	Jne *-5
.elseif(n = 17676)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17677)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17680)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17681)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17683)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17686)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17687)
	; 10 bytes
	ldx #239
	delay_n 69
	dex
	Jne *-6
.elseif(n = 17689)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17690)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 17696 to 17734
.if(n = 17696 || n = 17700 || n = 17705 || n = 17706 || n = 17708 || n = 17710 || n = 17712 || n = 17714 || n = 17718 || n = 17723 || n = 17724 || n = 17726 || n = 17727 || n = 17729 || n = 17730 || n = 17732)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17697)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17698)
	; 10 bytes
	ldx #(n-1042)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 17701)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17702)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17703)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17704)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17707)
	; 10 bytes
	ldx #(n-1)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17709)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17711)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17713)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17715)
	; 10 bytes
	ldx #(n-1)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17716)
	; 10 bytes
	ldx #(n-1)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17717)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17719)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #86
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 17720)
	; 10 bytes
	ldx #(n-1)/611
	ldy #121
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17721)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17722)
	; 10 bytes
	ldx #98
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 17725)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17728)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17731)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17733)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17734)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 17735) && (n < 17774))
; 17735 to 17773
.if((n > 17734 && n < 17738) || (n > 17740 && n < 17746) || n = 17747 || n = 17748 || n = 17750 || (n > 17752 && n < 17757) || n = 17762 || n = 17765 || n = 17766 || (n > 17767 && n < 17774))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17738)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #46
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 17739)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17740)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17746)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17749)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17751)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17752)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17757)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17758)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 406
	dey
	Jmi *-7
.elseif(n = 17759)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #27
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 17760)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17761)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17763)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17764)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17767)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 17774 to 17812
.if(n = 17774 || (n > 17777 && n < 17781) || n = 17782 || n = 17784 || n = 17786 || n = 17787 || n = 17789 || n = 17790 || n = 17792 || n = 17794 || n = 17796 || n = 17797 || n = 17799 || n = 17800 || n = 17802 || n = 17804 || n = 17806 || n = 17808 || (n > 17809 && n < 17813))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17775)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 81
	dey
	Jne *-7
.elseif(n = 17776)
	; 10 bytes
	ldx #(n-712)/711
	ldy #141
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17777)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17781)
	; 10 bytes
	ldx #34
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 17783)
	; 10 bytes
	ldx #(n-1047)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17785)
	; 10 bytes
	ldx #(n-457)/456
	ldy #90
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17788)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17791)
	; 10 bytes
	ldx #(n-1)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17793)
	; 10 bytes
	ldx #(n-1)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17795)
	; 10 bytes
	ldx #(n-1)/1271
	ldy #253
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17798)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17801)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17803)
	; 9 bytes
	ldy #138
	delay_n 124
	dey
	Jne *-5
.elseif(n = 17805)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17807)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17809)
	; 10 bytes
	ldx #(n-1)/336
	ldy #65
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
.if((n >= 17813) && (n < 17852))
; 17813 to 17851
.if(n = 17813)
	; 10 bytes
	ldx #(n-147)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif((n > 17813 && n < 17817) || n = 17818 || n = 17820 || (n > 17824 && n < 17831) || (n > 17831 && n < 17835) || n = 17836 || n = 17838 || n = 17842 || (n > 17843 && n < 17848) || n = 17850 || n = 17851)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17817)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17819)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17821)
	; 10 bytes
	ldx #(n-1)/396
	ldy #77
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17822)
	; 10 bytes
	ldx #(n-1)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17823)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #141
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 17831)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #147
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17835)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17837)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17839)
	; 10 bytes
	ldx #(n-992)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17840)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 17841)
	; 10 bytes
	ldx #(n-1)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17843)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17848)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17849)
	; 10 bytes
	ldx #(n-777)/776
	ldy #154
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 17852 to 17890
.if(n = 17852 || n = 17853 || n = 17856 || (n > 17857 && n < 17861) || n = 17862 || n = 17864 || n = 17867 || (n > 17868 && n < 17872) || n = 17873 || n = 17874 || n = 17879 || (n > 17881 && n < 17885) || n = 17888)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17854)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17855)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17857)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17861)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17863)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #39
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 17865)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17866)
	; 10 bytes
	ldx #(n-1)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17868)
	; 10 bytes
	ldx #(n-1052)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17872)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17875)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17876)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 296
	dey
	Jmi *-7
.elseif(n = 17877)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17878)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17880)
	; 10 bytes
	ldx #(n-1)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17881)
	; 10 bytes
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17885)
	; 10 bytes
	ldx #(n-1)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17886)
	; 10 bytes
	ldx #(n-512)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17887)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17889)
	; 10 bytes
	ldx #(n-417)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17890)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 17891) && (n < 17930))
; 17891 to 17929
.if(n = 17891 || n = 17892 || (n > 17893 && n < 17899) || n = 17900 || n = 17903 || n = 17904 || n = 17906 || (n > 17909 && n < 17915) || n = 17916 || n = 17918 || n = 17922 || (n > 17923 && n < 17927))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17893)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17899)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17901)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17902)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17905)
	; 10 bytes
	ldx #(n-1)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17907)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #27
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 17908)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17909)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17915)
	; 9 bytes
	ldx #26
	delay_n 684
	dex
	Jne *-5
.elseif(n = 17917)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17919)
	; 9 bytes
	ldx #61
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 17920)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17921)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17923)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17927)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17928)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17929)
	; 10 bytes
	ldx #(n-1)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 17930 to 17968
.if(n = 17930 || n = 17933 || (n > 17937 && n < 17941) || (n > 17941 && n < 17948) || n = 17950 || n = 17952 || n = 17954 || n = 17958 || (n > 17959 && n < 17964) || n = 17965 || n = 17966 || n = 17968)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17931)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17932)
	; 9 bytes
	ldy #139
	delay_n 124
	dey
	Jne *-5
.elseif(n = 17934)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17935)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17936)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17937)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17941)
	; 10 bytes
	ldx #(n-1)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17948)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17949)
	; 10 bytes
	ldx #(n-642)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17951)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 17953)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17955)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17956)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17957)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #85
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 17959)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17964)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17967)
	; 10 bytes
	ldx #(n-1)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 17969) && (n < 18047))
.if((n >= 17969) && (n < 18008))
; 17969 to 18007
.if(n = 17969 || n = 17971 || n = 17972 || n = 17974 || n = 17976 || n = 17978 || n = 17982 || n = 17984 || n = 17986 || (n > 17987 && n < 17991) || n = 17992 || n = 17994 || (n > 17995 && n < 18005) || n = 18006 || n = 18007)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17970)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17973)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 411
	dey
	Jmi *-7
.elseif(n = 17975)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17977)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17979)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17980)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17981)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 17983)
	; 10 bytes
	ldx #(n-487)/486
	ldy #96
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17985)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 17987)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 17991)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #32
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 17993)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 17995)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #162
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 18005)
	; 10 bytes
	ldx #<((n-1)/1286)
	ldy #>((n-1)/1286)
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 18008 to 18046
.if(n = 18008 || n = 18009 || n = 18011 || n = 18014 || n = 18015 || n = 18018 || n = 18020 || (n > 18021 && n < 18025) || (n > 18026 && n < 18030) || n = 18032 || n = 18034 || n = 18036 || n = 18040 || n = 18042 || n = 18044 || n = 18045)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18010)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18012)
	; 10 bytes
	ldx #(n-1)/581
	ldy #114
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18013)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18016)
	; 10 bytes
	ldx #(n-1)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18017)
	; 10 bytes
	ldx #(n-1)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18019)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18021)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18025)
	; 10 bytes
	ldx #(n-1)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18026)
	; 10 bytes
	ldx #(n-722)/721
	ldy #143
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18030)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18031)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18033)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18035)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18037)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18038)
	; 10 bytes
	ldx #(n-1062)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18039)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18041)
	; 10 bytes
	ldx #(n-1)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18043)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18046)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 18047) && (n < 18086))
; 18047 to 18085
.if(n = 18047 || n = 18048 || n = 18050 || (n > 18051 && n < 18055) || n = 18056 || n = 18058 || n = 18060 || n = 18062 || n = 18065 || (n > 18066 && n < 18070) || n = 18072 || n = 18075 || n = 18077 || n = 18078 || n = 18083 || n = 18084)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18049)
	; 9 bytes
	ldy #46
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 18051)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18055)
	; 10 bytes
	ldx #(n-307)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18057)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18059)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 251
	iny
	Jne *-7
.elseif(n = 18061)
	; 9 bytes
	ldy #140
	delay_n 124
	dey
	Jne *-5
.elseif(n = 18063)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18064)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18066)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #39
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 18070)
	; 10 bytes
	ldx #(n-1)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18071)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 18073)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18074)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18076)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18079)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18080)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18081)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18082)
	; 10 bytes
	ldx #(n-442)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18085)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 18086 to 18125
.if(n = 18086 || n = 18088 || n = 18090 || n = 18092 || n = 18093 || n = 18096 || (n > 18097 && n < 18101) || n = 18102 || n = 18107 || n = 18108 || n = 18111 || n = 18112 || (n > 18113 && n < 18118) || n = 18120 || n = 18122 || n = 18124)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18087)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 18089)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18091)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18094)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18095)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18097)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18101)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18103)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18104)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18105)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18106)
	; 10 bytes
	ldx #(n-1)/71
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18109)
	; 10 bytes
	ldx #(n-1007)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18110)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18113)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18118)
	; 10 bytes
	ldx #(n-672)/671
	ldy #133
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18119)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	dex
	Jne *-7
.elseif(n = 18121)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18123)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18125)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-7
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
.if(n = 18126 || n = 18128 || n = 18130 || (n > 18131 && n < 18136) || (n > 18136 && n < 18143) || n = 18144 || (n > 18147 && n < 18151) || (n > 18151 && n < 18155) || n = 18156 || (n > 18158 && n < 18162))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18127)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18129)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18131)
	; 10 bytes
	ldx #245
	delay_n 69
	dex
	Jne *-6
.elseif(n = 18136)
	; 10 bytes
	ldx #45
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 18143)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18145)
	; 10 bytes
	ldx #(n-1)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18146)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18147)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18151)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18155)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18157)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18158)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18162)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #86
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 18163)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #93
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 18164)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 166
	dey
	Jmi *-7
.endif
.else
; 18165 to 18203
.if(n = 18165)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18166)
	; 10 bytes
	ldx #15
	ldy #241
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18167)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18168)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18169 || n = 18170 || n = 18172 || n = 18174 || n = 18176 || n = 18178 || n = 18179 || n = 18182 || n = 18183 || n = 18185 || n = 18186 || n = 18189 || (n > 18194 && n < 18199) || n = 18200 || n = 18202 || n = 18203)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18171)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 301
	dey
	Jmi *-7
.elseif(n = 18173)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18175)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18177)
	; 10 bytes
	ldx #0
	ldy #13
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18180)
	; 10 bytes
	ldx #(n-372)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18181)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18184)
	; 10 bytes
	ldx #(n-1)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18187)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18188)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 416
	dey
	Jmi *-7
.elseif(n = 18190)
	; 9 bytes
	ldy #141
	delay_n 124
	dey
	Jne *-5
.elseif(n = 18191)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #150
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 18192)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18193)
	; 10 bytes
	ldx #12
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	dex
	Jne *-6
.elseif(n = 18194)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18199)
	; 10 bytes
	ldx #(n-1012)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18201)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 18204) && (n < 18243))
; 18204 to 18242
.if(n = 18204 || n = 18207 || n = 18209 || n = 18211 || n = 18212 || n = 18214 || n = 18216 || (n > 18217 && n < 18221) || n = 18222 || n = 18223 || n = 18228 || (n > 18229 && n < 18233) || n = 18234 || n = 18235 || (n > 18236 && n < 18241) || n = 18242)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18205)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18206)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18208)
	; 9 bytes
	ldx #62
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 18210)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18213)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18215)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #49
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 18217)
	; 10 bytes
	ldx #(n-277)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18221)
	; 10 bytes
	ldx #(n-912)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18224)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 236
	dey
	Jmi *-7
.elseif(n = 18225)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18226)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18227)
	; 10 bytes
	ldx #(n-1)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18229)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18233)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18236)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18241)
	; 10 bytes
	ldx #(n-1)/456
	ldy #90
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 18243 to 18281
.if((n > 18242 && n < 18250) || n = 18252 || (n > 18253 && n < 18257) || n = 18258 || (n > 18261 && n < 18265) || n = 18267 || n = 18268 || n = 18270 || n = 18274 || n = 18275 || n = 18277 || n = 18278 || n = 18280 || n = 18281)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18250)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18251)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18253)
	; 10 bytes
	ldx #(n-352)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18257)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18259)
	; 10 bytes
	ldx #101
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 18260)
	; 10 bytes
	ldx #(n-1)/961
	ldy #191
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18261)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18265)
	; 10 bytes
	ldx #(n-1)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18266)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18269)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 167
	dey
	Jmi *-7
.elseif(n = 18271)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18272)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18273)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18276)
	; 10 bytes
	ldx #(n-732)/731
	ldy #145
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18279)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
.if((n >= 18282) && (n < 18321))
; 18282 to 18320
.if(n = 18282)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18283)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif((n > 18283 && n < 18287) || n = 18288 || n = 18290 || n = 18291 || (n > 18293 && n < 18298) || n = 18299 || n = 18302 || n = 18304 || n = 18306 || n = 18308 || n = 18310 || n = 18312 || n = 18314 || n = 18315 || n = 18318 || n = 18320)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18287)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18289)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18292)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18293)
	; 10 bytes
	ldx #(n-1077)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18298)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18300)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18301)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18303)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #39
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 18305)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18307)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18309)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18311)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 18313)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18316)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18317)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18319)
	; 9 bytes
	ldy #142
	delay_n 124
	dey
	Jne *-5
.endif
.else
; 18321 to 18359
.if(n = 18321)
	; 10 bytes
	ldx #(n-917)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18322)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18323 || (n > 18325 && n < 18329) || n = 18330 || n = 18332 || n = 18333 || n = 18336 || n = 18338 || n = 18340 || n = 18342 || (n > 18345 && n < 18349) || n = 18351 || n = 18352 || n = 18354 || n = 18356)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18324)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18325)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #165
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 18329)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18331)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18334)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18335)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18337)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18339)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18341)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18343)
	; 9 bytes
	ldy #18
	delay_n 1014
	dey
	Jne *-5
.elseif(n = 18344)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18345)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18349)
	; 10 bytes
	ldx #(n-1)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18350)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18353)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18355)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18357)
	; 10 bytes
	ldx #(n-1)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18358)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18359)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 101
	dey
	Jne *-7
.endif
.endif
.else
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if(n = 18360 || (n > 18362 && n < 18367) || n = 18368 || (n > 18370 && n < 18378) || (n > 18379 && n < 18383) || n = 18384 || n = 18386 || n = 18387 || n = 18389 || n = 18390 || n = 18392 || n = 18394 || n = 18396 || n = 18398)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18361)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18362)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18367)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 18369)
	; 10 bytes
	ldx #(n-657)/656
	ldy #130
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18370)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18378)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18379)
	; 10 bytes
	ldx #(n-1022)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18383)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18385)
	; 10 bytes
	ldx #(n-1)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18388)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18391)
	; 10 bytes
	ldx #(n-1)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18393)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18395)
	; 10 bytes
	ldx #(n-1)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18397)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 18399 to 18437
.if(n = 18399 || n = 18400 || n = 18402 || n = 18404 || (n > 18405 && n < 18409) || n = 18413 || n = 18414 || n = 18416 || (n > 18417 && n < 18421) || n = 18422 || n = 18426 || n = 18428 || n = 18430 || n = 18432 || n = 18434 || n = 18436 || n = 18437)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18401)
	; 10 bytes
	ldx #(n-737)/736
	ldy #146
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18403)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 421
	dey
	Jmi *-7
.elseif(n = 18405)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18409)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18410)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 18411)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18412)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18415)
	; 10 bytes
	ldx #(n-187)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18417)
	; 10 bytes
	ldx #(n-1)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18421)
	; 10 bytes
	ldx #(n-922)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18423)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18424)
	; 10 bytes
	ldx #(n-802)/801
	ldy #159
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18425)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18427)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18429)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18431)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 18433)
	; 9 bytes
	ldy #47
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 18435)
	; 10 bytes
	delay_n (n-2)
	nop
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
	; 10 bytes
	ldx #102
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 18439 || n = 18440 || (n > 18441 && n < 18445) || n = 18446 || (n > 18451 && n < 18461) || n = 18462 || n = 18464 || n = 18465 || n = 18467 || n = 18470 || n = 18471 || n = 18473 || n = 18475 || n = 18476)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18441)
	; 10 bytes
	ldx #(n-1)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18445)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18447)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18448)
	; 9 bytes
	ldy #143
	delay_n 124
	dey
	Jne *-5
.elseif(n = 18449)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #146
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 18450)
	; 10 bytes
	ldx #(n-1)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18451)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18461)
	; 10 bytes
	ldx #65
	ldy #216 ;hides 'cld'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif(n = 18463)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18466)
	; 10 bytes
	ldx #(n-1)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18468)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #49
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 18469)
	; 10 bytes
	ldx #(n-487)/486
	ldy #96
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18472)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18474)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 18477 to 18515
.if(n = 18477)
	; 10 bytes
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif((n > 18477 && n < 18481) || n = 18482 || n = 18483 || n = 18485 || n = 18486 || (n > 18487 && n < 18491) || n = 18494 || n = 18495 || n = 18498 || n = 18500 || n = 18504 || (n > 18505 && n < 18509) || n = 18515)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18481)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18484)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18487)
	; 10 bytes
	ldx #(n-1)/711
	ldy #141
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18491)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18492)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18493)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18496)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18497)
	; 9 bytes
	ldx #64
	delay_n 284
	dex
	Jne *-5
.elseif(n = 18499)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18501)
	; 10 bytes
	ldx #250
	delay_n 69
	dex
	Jne *-6
.elseif(n = 18502)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18503)
	; 10 bytes
	ldx #(n-842)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18505)
	; 10 bytes
	ldx #(n-1)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18509)
	; 10 bytes
	ldx #(n-662)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18510)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #32
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 18511)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #33
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 18512)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #28
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 18513)
	; 10 bytes
	ldx #(n-357)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18514)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 18516) && (n < 18555))
; 18516 to 18554
.if(n = 18516)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18517 || n = 18518 || n = 18520 || n = 18522 || n = 18524 || n = 18527 || n = 18530 || n = 18531 || n = 18535 || n = 18536 || n = 18540 || n = 18542 || n = 18544 || n = 18546 || n = 18549 || (n > 18551 && n < 18555))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18519)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #91
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 18521)
	; 10 bytes
	ldx #(n-927)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18523)
	; 10 bytes
	ldx #(n-442)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18525)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18526)
	; 10 bytes
	ldx #(n-742)/741
	ldy #147
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18528)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18529)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18532)
	; 10 bytes
	ldx #(n-1)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18533)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18534)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18537)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18538)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18539)
	; 10 bytes
	ldx #(n-807)/806
	ldy #160
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18541)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18543)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18545)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18547)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18548)
	; 10 bytes
	ldx #(n-1092)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18550)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18551)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 18555 to 18593
.if((n > 18554 && n < 18558) || n = 18560 || (n > 18561 && n < 18565) || (n > 18565 && n < 18569) || n = 18570 || n = 18571 || n = 18573 || n = 18576 || n = 18578 || (n > 18579 && n < 18583) || n = 18584 || (n > 18586 && n < 18590) || n = 18592)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18558)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18559)
	; 10 bytes
	ldx #(n-1032)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18561)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18565)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18569)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18572)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #88
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 18574)
	; 10 bytes
	ldx #(n-152)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18575)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18577)
	; 9 bytes
	ldy #144
	delay_n 124
	dey
	Jne *-5
.elseif(n = 18579)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18583)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18585)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18586)
	; 10 bytes
	ldx #(n-532)/531
	ldy #105
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18590)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18591)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18593)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
.if((n >= 18594) && (n < 18633))
; 18594 to 18632
.if((n > 18593 && n < 18599) || n = 18600 || n = 18602 || n = 18605 || (n > 18607 && n < 18613) || n = 18614 || n = 18615 || n = 18620 || (n > 18621 && n < 18625) || (n > 18625 && n < 18631))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #16
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 18601)
	; 10 bytes
	ldx #(n-1)/186
	ldy #35
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18603)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18604)
	; 9 bytes
	ldx #26
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 18606)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18607)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18613)
	; 10 bytes
	ldx #(n-397)/396
	ldy #77
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18616)
	; 10 bytes
	ldx #15
	ldy #247
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18617)
	; 10 bytes
	ldx #(n-1)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18618)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 426
	dey
	Jmi *-7
.elseif(n = 18619)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18621)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18625)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18631)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18632)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 18633 to 18671
.if(n = 18633)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18634 || (n > 18635 && n < 18640) || n = 18642 || n = 18643 || n = 18646 || n = 18648 || n = 18650 || n = 18652 || n = 18653 || n = 18660 || n = 18662 || (n > 18663 && n < 18667) || n = 18668 || n = 18669)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18635)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18640)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18641)
	; 10 bytes
	ldx #(n-1)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18644)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18645)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18647)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 18649)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18651)
	; 10 bytes
	ldx #(n-747)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18654)
	; 10 bytes
	ldx #(n-812)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18655)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #168
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 18656)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18657)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18658)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18659)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18661)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18663)
	; 10 bytes
	ldx #(n-302)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18667)
	; 10 bytes
	ldx #(n-307)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18670)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18671)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #154
	dey
	Jne *-1
	dex
	Jne *-7
.endif
.endif
.else
.if((n >= 18672) && (n < 18711))
; 18672 to 18710
.if((n > 18671 && n < 18677) || n = 18678 || (n > 18679 && n < 18685) || n = 18687 || n = 18688 || n = 18690 || n = 18692 || n = 18693 || n = 18696 || n = 18698 || (n > 18700 && n < 18706) || n = 18707 || n = 18709 || n = 18710)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18677)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18679)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18685)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18686)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18689)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18691)
	; 10 bytes
	ldx #15
	ldy #248
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18694)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18695)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 18697)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18699)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #148
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 18700)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18706)
	; 9 bytes
	ldy #145
	delay_n 124
	dey
	Jne *-5
.elseif(n = 18708)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.else
; 18711 to 18750
.if(n = 18711 || (n > 18712 && n < 18718) || n = 18720 || n = 18722 || n = 18724 || (n > 18725 && n < 18733) || n = 18736 || n = 18738 || (n > 18739 && n < 18743) || n = 18744 || n = 18746 || n = 18748 || n = 18750)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18712)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18718)
	; 10 bytes
	ldx #(n-1102)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18719)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18721)
	; 10 bytes
	ldx #(n-1)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18723)
	; 10 bytes
	ldx #(n-507)/506
	ldy #100
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18725)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18733)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18734)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18735)
	; 10 bytes
	ldx #(n-1)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18737)
	; 10 bytes
	ldx #(n-1)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18739)
	; 10 bytes
	ldx #(n-1042)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18743)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #40
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 18745)
	; 10 bytes
	ldx #(n-427)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18747)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18749)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
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
.if(n = 18751 || n = 18752 || n = 18755 || n = 18758 || n = 18762 || n = 18764 || n = 18768 || (n > 18769 && n < 18773) || n = 18774 || n = 18778 || n = 18779 || (n > 18781 && n < 18785))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18753)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18754)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18756)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18757)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18759)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18760)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18761)
	; 10 bytes
	ldx #(n-537)/536
	ldy #106
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18763)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18765)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #169
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 18766)
	; 10 bytes
	ldx #(n-1)/1251
	ldy #249
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18767)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 18769)
	; 10 bytes
	ldx #(n-277)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18773)
	; 10 bytes
	ldx #(n-362)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18775)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18776)
	; 10 bytes
	ldx #(n-752)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18777)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #90
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 18780)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18781)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18785)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 86
	dey
	Jne *-7
.elseif(n = 18786)
	; 9 bytes
	ldx #65
	delay_n 284
	dex
	Jne *-5
.elseif(n = 18787)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18788)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18789)
	; 10 bytes
	ldx #(n-672)/671
	ldy #133
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 18790 to 18828
.if(n = 18790 || n = 18792 || n = 18794 || n = 18795 || n = 18798 || n = 18800 || n = 18802 || (n > 18803 && n < 18811) || (n > 18811 && n < 18817) || n = 18818 || n = 18820 || n = 18822 || n = 18827)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18791)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 341
	iny
	Jne *-7
.elseif(n = 18793)
	; 10 bytes
	ldx #(n-1)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18796)
	; 10 bytes
	ldx #104
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 18797)
	; 10 bytes
	ldx #36
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 18799)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18801)
	; 10 bytes
	ldx #(n-1)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18803)
	; 10 bytes
	ldx #(n-1107)/1106
	ldy #220
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18811)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18817)
	; 9 bytes
	ldy #49
	delay_n 379
	dey
	Jne *-5
.elseif(n = 18819)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18821)
	; 10 bytes
	ldx #(n-942)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18823)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	inx
	Jpl *-7
.elseif(n = 18824)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #149
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 18825)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18826)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18828)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 18829) && (n < 18868))
; 18829 to 18867
.if(n = 18829)
	; 10 bytes
	ldx #(n-1047)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18830)
	; 10 bytes
	ldx #(n-1)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18831 || n = 18832 || n = 18834 || n = 18836 || n = 18839 || n = 18840 || n = 18842 || n = 18843 || n = 18845 || n = 18846 || (n > 18849 && n < 18865) || n = 18866 || n = 18867)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18833)
	; 10 bytes
	ldx #(n-177)/176
	ldy #33
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18835)
	; 9 bytes
	ldy #146
	delay_n 124
	dey
	Jne *-5
.elseif(n = 18837)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18838)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18841)
	; 10 bytes
	ldx #(n-1)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18844)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18847)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 69
	dex
	Jne *-7
.elseif(n = 18848)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18849)
	; 10 bytes
	ldx #(n-497)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18865)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 18868 to 18906
.if(n = 18868)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18869)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18870 || n = 18872 || n = 18873 || n = 18878 || n = 18879 || n = 18882 || n = 18883 || n = 18885 || n = 18886 || (n > 18888 && n < 18895) || n = 18896 || (n > 18897 && n < 18901) || n = 18903 || n = 18904 || n = 18906)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18871)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18874)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18875)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #170
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 18876)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18877)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18880)
	; 10 bytes
	ldx #(n-1)/651
	ldy #128
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18881)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18884)
	; 10 bytes
	ldx #(n-822)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18887)
	; 10 bytes
	ldx #(n-1)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18888)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18895)
	; 10 bytes
	ldx #(n-1)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18897)
	; 10 bytes
	ldx #(n-1)/1181
	ldy #235
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18901)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18902)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18905)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
; 18907 to 18945
.if(n = 18907)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18908)
	; 10 bytes
	ldx #(n-512)/511
	ldy #101
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18909)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18910)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18911)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #156
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 18912 || n = 18914 || n = 18917 || n = 18918 || n = 18920 || n = 18924 || n = 18926 || n = 18930 || (n > 18931 && n < 18935) || n = 18938 || n = 18939 || n = 18941 || n = 18944)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18913)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18915)
	; 10 bytes
	ldx #(n-387)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18916)
	; 10 bytes
	ldx #(n-1)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18919)
	; 10 bytes
	ldx #(n-1052)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18921)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18922)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18923)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #98
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 18925)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18927)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #65
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 18928)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18929)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18931)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18935)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #51
	dey
	Jne *-1
	inx
	Jpl *-7
.elseif(n = 18936)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18937)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18940)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18942)
	; 10 bytes
	ldx #(n-1)/611
	ldy #121
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18943)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18945)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 18946 to 18984
.if(n = 18946)
	; 10 bytes
	ldx #(n-1)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18947 || n = 18948 || n = 18950 || n = 18952 || n = 18954 || n = 18957 || n = 18958 || n = 18960 || n = 18962 || n = 18963 || (n > 18966 && n < 18971) || n = 18976 || n = 18978 || n = 18979 || n = 18984)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18949)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #150
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 18951)
	; 10 bytes
	ldx #184 ;hides 'clv'
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 18953)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18955)
	; 10 bytes
	ldx #(n-1)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18956)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #40
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 18959)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #29
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 18961)
	; 10 bytes
	ldx #(n-1)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18964)
	; 9 bytes
	ldy #147
	delay_n 124
	dey
	Jne *-5
.elseif(n = 18965)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18966)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18971)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18972)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18973)
	; 10 bytes
	ldx #(n-307)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 18974)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 246
	dey
	Jmi *-7
.elseif(n = 18975)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18977)
	; 10 bytes
	ldx #(n-1)/1186
	ldy #236
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18980)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 18981)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18982)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18983)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-7
.endif
.endif
.else
.if((n >= 18985) && (n < 19024))
; 18985 to 19023
.if(n = 18985)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif((n > 18985 && n < 18989) || n = 18990 || (n > 18991 && n < 18996) || n = 18997 || n = 19000 || n = 19002 || n = 19003 || n = 19005 || n = 19008 || (n > 19009 && n < 19013) || (n > 19013 && n < 19020) || n = 19022 || n = 19023)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18989)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18991)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 18996)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 18998)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 18999)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19001)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19004)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 174
	dey
	Jmi *-7
.elseif(n = 19006)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19007)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19009)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19013)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19020)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19021)
	; 10 bytes
	ldx #(n-952)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 19024 to 19062
.if(n = 19024 || n = 19028 || n = 19030 || n = 19032 || n = 19034 || n = 19035 || n = 19037 || n = 19038 || (n > 19041 && n < 19045) || n = 19046 || n = 19047 || n = 19049 || n = 19050 || n = 19052 || n = 19055 || n = 19059 || n = 19060 || n = 19062)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19025)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19026)
	; 10 bytes
	ldx #(n-762)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19027)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19029)
	; 10 bytes
	ldx #67
	ldy #216 ;hides 'cld'
	iny
	Jne *-2
	dex
	Jne *-6
.elseif(n = 19031)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19033)
	; 10 bytes
	ldx #(n-367)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19036)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19039)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19040)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19041)
	; 10 bytes
	ldx #(n-1)/476
	ldy #93
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19045)
	; 10 bytes
	ldx #(n-277)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19048)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 436
	dey
	Jmi *-7
.elseif(n = 19051)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19053)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19054)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19056)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 316
	dey
	Jmi *-7
.elseif(n = 19057)
	; 10 bytes
	ldx #(n-1)/1191
	ldy #237
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19058)
	; 10 bytes
	ldx #(n-1122)/1121
	ldy #223
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19061)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 346
	iny
	Jne *-7
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
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19064 || n = 19065 || n = 19067 || n = 19068 || (n > 19069 && n < 19073) || n = 19076 || (n > 19077 && n < 19081) || n = 19082 || (n > 19083 && n < 19090) || n = 19092 || n = 19094 || n = 19098 || n = 19100)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19066)
	; 10 bytes
	ldx #(n-1)/1271
	ldy #253
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19069)
	; 10 bytes
	ldx #(n-682)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19073)
	; 10 bytes
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19074)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #151
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 19075)
	; 9 bytes
	ldx #65
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 19077)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19081)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19083)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19090)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19091)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19093)
	; 9 bytes
	ldy #148
	delay_n 124
	dey
	Jne *-5
.elseif(n = 19095)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #172
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19096)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19097)
	; 10 bytes
	ldx #(n-1)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19099)
	; 10 bytes
	ldx #(n-1062)/1061
	ldy #211
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19101)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 19102 to 19140
.if((n > 19101 && n < 19105) || n = 19106 || n = 19108 || n = 19110 || n = 19112 || n = 19116 || n = 19118 || n = 19120 || (n > 19121 && n < 19127) || (n > 19127 && n < 19132) || n = 19133 || n = 19135 || n = 19136 || (n > 19137 && n < 19141))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19105)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19107)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19109)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19111)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19113)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #98
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 19114)
	; 10 bytes
	ldx #(n-832)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19115)
	; 10 bytes
	ldx #(n-1)/1006
	ldy #200
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19117)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19119)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19121)
	; 10 bytes
	ldx #(n-957)/956
	ldy #190
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19127)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19132)
	; 10 bytes
	ldx #(n-912)/911
	ldy #181
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19134)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19137)
	; 10 bytes
	ldx #(n-417)/416
	ldy #82
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 19141) && (n < 19180))
; 19141 to 19179
.if(n = 19141)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19142 || (n > 19143 && n < 19150) || n = 19152 || n = 19155 || n = 19156 || n = 19158 || n = 19161 || n = 19162 || (n > 19163 && n < 19170) || n = 19172 || n = 19173 || n = 19175 || n = 19176)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19143)
	; 10 bytes
	ldx #(n-1127)/1126
	ldy #224
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19150)
	; 10 bytes
	ldx #(n-492)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19151)
	; 10 bytes
	ldx #(n-767)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19153)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19154)
	; 10 bytes
	ldx #106
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 19157)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 96
	dey
	Jne *-7
.elseif(n = 19159)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19160)
	; 10 bytes
	ldx #(n-392)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19163)
	; 10 bytes
	ldx #(n-1)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19170)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19171)
	; 10 bytes
	ldx #(n-1)/426
	ldy #84
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19174)
	; 10 bytes
	ldx #(n-1)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19177)
	; 10 bytes
	ldx #(n-1)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19178)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19179)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 19180 to 19218
.if((n > 19179 && n < 19183) || n = 19184 || n = 19186 || n = 19188 || n = 19190 || (n > 19191 && n < 19198) || n = 19200 || n = 19206 || n = 19208 || n = 19212 || n = 19213 || n = 19215 || n = 19218)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19183)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #42
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19185)
	; 10 bytes
	ldx #(n-437)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19187)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19189)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19191)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19198)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19199)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19201)
	; 9 bytes
	ldy #49
	delay_n 379
	dey
	Jpl *-5
.elseif(n = 19202)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19203)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19204)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19205)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #173
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19207)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19209)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19210)
	; 10 bytes
	ldx #(n-1)/1011
	ldy #201
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19211)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19214)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 176
	dey
	Jmi *-7
.elseif(n = 19216)
	; 10 bytes
	ldx #15
	ldy #255
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19217)
	; 10 bytes
	ldx #(n-1)/1201
	ldy #239
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 19219) && (n < 19297))
.if((n >= 19219) && (n < 19258))
; 19219 to 19257
.if(n = 19219 || n = 19220 || n = 19223 || n = 19226 || n = 19227 || n = 19230 || n = 19232 || n = 19234 || n = 19236 || n = 19238 || n = 19239 || n = 19243 || n = 19244 || (n > 19245 && n < 19249) || n = 19250 || n = 19251 || n = 19253 || n = 19254 || n = 19256)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19221)
	; 10 bytes
	ldx #(n-962)/961
	ldy #191
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19222)
	; 9 bytes
	ldy #149
	delay_n 124
	dey
	Jne *-5
.elseif(n = 19224)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19225)
	; 10 bytes
	ldx #(n-1)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19228)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19229)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19231)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19233)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19235)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19237)
	; 10 bytes
	ldx #(n-917)/916
	ldy #182
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19240)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19241)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19242)
	; 10 bytes
	ldx #(n-1)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19245)
	; 10 bytes
	ldx #(n-1)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19249)
	; 10 bytes
	ldx #(n-1)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19252)
	; 10 bytes
	ldx #31
	ldy #122
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19255)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #15
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19257)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 19258 to 19296
.if(n = 19258)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19259 || n = 19260 || n = 19262 || n = 19264 || n = 19266 || (n > 19267 && n < 19271) || n = 19272 || n = 19274 || n = 19275 || n = 19280 || n = 19282 || n = 19284 || n = 19285 || (n > 19286 && n < 19291) || n = 19294 || n = 19296)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19261)
	; 10 bytes
	ldx #(n-1)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19263)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 441
	dey
	Jmi *-7
.elseif(n = 19265)
	; 10 bytes
	ldx #(n-1)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19267)
	; 10 bytes
	ldx #26
	ldy #147
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19271)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #159
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 19273)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19276)
	; 10 bytes
	ldx #(n-772)/771
	ldy #153
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19277)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19278)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19279)
	; 10 bytes
	ldx #(n-307)/306
	ldy #60
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19281)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19283)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19286)
	; 10 bytes
	ldx #(n-552)/551
	ldy #109
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19291)
	; 10 bytes
	ldx #<((n-1)/1286)
	ldy #>((n-1)/1286)
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19292)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19293)
	; 9 bytes
	ldx #27
	delay_n 684
	dex
	Jpl *-5
.elseif(n = 19295)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #52
	dey
	Jne *-1
	inx
	Jpl *-7
.endif
.endif
.else
.if((n >= 19297) && (n < 19336))
; 19297 to 19335
.if(n = 19297)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif((n > 19297 && n < 19301) || n = 19302 || n = 19304 || (n > 19306 && n < 19312) || n = 19314 || n = 19316 || n = 19317 || n = 19320 || n = 19322 || n = 19323 || (n > 19324 && n < 19328) || n = 19330 || n = 19332 || n = 19334 || n = 19335)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19301)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19303)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #100
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 19305)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19306)
	; 10 bytes
	ldx #(n-1)/351
	ldy #69
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19312)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19313)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19315)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19318)
	; 10 bytes
	ldx #(n-412)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19319)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #95
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19321)
	; 10 bytes
	ldx #(n-277)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19324)
	; 10 bytes
	ldx #(n-1)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19328)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19329)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19331)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 351
	iny
	Jne *-7
.elseif(n = 19333)
	; 10 bytes
	ldx #(n-717)/716
	ldy #142
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 19336 to 19375
.if((n > 19335 && n < 19339) || n = 19340 || n = 19341 || (n > 19345 && n < 19349) || n = 19350 || n = 19352 || (n > 19353 && n < 19359) || (n > 19369 && n < 19373) || n = 19374 || n = 19375)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19339)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19342)
	; 10 bytes
	ldx #(n-922)/921
	ldy #183
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19343)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-7
.elseif(n = 19344)
	; 10 bytes
	ldx #(n-842)/841
	ldy #167
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19345)
	; 10 bytes
	ldx #(n-497)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19349)
	; 10 bytes
	ldx #(n-692)/691
	ldy #137
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19351)
	; 9 bytes
	ldy #150
	delay_n 124
	dey
	Jne *-5
.elseif(n = 19353)
	; 10 bytes
	ldx #(n-237)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19359)
	; 10 bytes
	ldx #248 ;hides 'sed'
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	dex
	Jmi *-7
.elseif(n = 19360)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19361)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19362)
	; 9 bytes
	ldy #18
	delay_n 1014
	dey
	Jpl *-5
.elseif(n = 19363)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19364)
	; 9 bytes
	ldx #67
	delay_n 284
	dex
	Jne *-5
.elseif(n = 19365)
	; 10 bytes
	ldx #(n-1)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19366)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19367)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19368)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19369)
	; 10 bytes
	ldx #(n-1077)/1076
	ldy #214
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19373)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
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
.if(n = 19376 || n = 19378 || n = 19380 || n = 19382 || (n > 19384 && n < 19389) || n = 19395 || n = 19402 || n = 19404 || n = 19406 || (n > 19407 && n < 19413) || n = 19414)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19377)
	; 10 bytes
	ldx #(n-1)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19379)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	dex
	Jmi *-7
.elseif(n = 19381)
	; 10 bytes
	ldx #(n-1)/76
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19383)
	; 10 bytes
	ldx #(n-882)/881
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19384)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19389)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19390)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19391)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #160
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 19392)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #93
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 19393)
	; 10 bytes
	ldx #(n-1)/96
	ldy #18
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19394)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19396)
	; 10 bytes
	ldx #(n-1)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19397)
	; 10 bytes
	ldx #(n-1)/746
	ldy #148
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19398)
	; 10 bytes
	ldx #(n-1142)/1141
	ldy #227
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19399)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19400)
	; 10 bytes
	ldx #(n-1)/1021
	ldy #203
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19401)
	; 10 bytes
	ldx #(n-777)/776
	ldy #154
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19403)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 684
	dex
	dex
	Jne *-7
.elseif(n = 19405)
	; 10 bytes
	ldx #(n-197)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19407)
	; 10 bytes
	ldx #(n-1)/626
	ldy #123
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19413)
	; 10 bytes
	ldx #(n-212)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 19415 to 19453
.if(n = 19415)
	; 10 bytes
	ldx #(n-1)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif((n > 19415 && n < 19419) || n = 19420 || n = 19422 || n = 19424 || (n > 19426 && n < 19432) || n = 19433 || n = 19434 || n = 19439 || n = 19440 || (n > 19441 && n < 19447) || n = 19448 || (n > 19449 && n < 19454))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19419)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19421)
	; 10 bytes
	ldx #(n-972)/971
	ldy #193
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19423)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19425)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #175
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19426)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19432)
	; 10 bytes
	ldx #(n-1)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19435)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19436)
	; 10 bytes
	ldy #200 ;hides 'iny'
	delay_n 1014
	iny
	iny
	Jne *-7
.elseif(n = 19437)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19438)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19441)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19447)
	; 10 bytes
	ldx #(n-927)/926
	ldy #184
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19449)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.endif
.else
.if((n >= 19454) && (n < 19493))
; 19454 to 19492
.if((n > 19453 && n < 19457) || n = 19458 || n = 19462 || n = 19464 || n = 19466 || n = 19467 || (n > 19468 && n < 19475) || n = 19476 || n = 19479 || n = 19481 || (n > 19483 && n < 19487) || n = 19488 || (n > 19489 && n < 19493))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19457)
	; 10 bytes
	ldx #0
	ldy #13
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19459)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19460)
	; 10 bytes
	ldx #29
	ldy #133
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19461)
	; 10 bytes
	ldx #(n-557)/556
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19463)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19465)
	; 10 bytes
	ldx #(n-1)/811
	ldy #161
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19468)
	; 10 bytes
	ldx #(n-722)/721
	ldy #143
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19475)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19477)
	; 10 bytes
	ldx #(n-542)/541
	ldy #107
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19478)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 446
	dey
	Jmi *-7
.elseif(n = 19480)
	; 9 bytes
	ldy #151
	delay_n 124
	dey
	Jne *-5
.elseif(n = 19482)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19483)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19487)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #68
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 19489)
	; 10 bytes
	ldx #(n-337)/336
	ldy #65
	dey
	Jpl *-1
	dex
	Jpl *-6
.endif
.else
; 19493 to 19531
.if(n = 19493)
	; 10 bytes
	ldx #(n-887)/886
	ldy #176
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19494)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19495)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19496 || n = 19497 || n = 19499 || n = 19500 || n = 19502 || n = 19503 || (n > 19505 && n < 19511) || (n > 19513 && n < 19517) || n = 19518 || n = 19524 || n = 19525 || n = 19528 || n = 19529)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19498)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19501)
	; 10 bytes
	ldx #(n-1)/156
	ldy #29
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19504)
	; 10 bytes
	ldx #(n-1)/591
	ldy #117
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19505)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19511)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #161
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 19512)
	; 10 bytes
	ldx #108
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
.elseif(n = 19513)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19517)
	; 10 bytes
	ldx #(n-477)/476
	ldy #93
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19519)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #96
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19520)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19521)
	; 10 bytes
	ldx #(n-977)/976
	ldy #194
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19522)
	; 10 bytes
	ldx #(n-1)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19523)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19526)
	; 10 bytes
	ldx #(n-782)/781
	ldy #155
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19527)
	; 10 bytes
	ldx #(n-1)/751
	ldy #149
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19530)
	; 10 bytes
	ldx #(n-332)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19531)
	; 10 bytes
	ldx #(n-1)/126
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 19532) && (n < 19610))
.if((n >= 19532) && (n < 19571))
; 19532 to 19570
.if((n > 19531 && n < 19535) || n = 19536 || n = 19538 || n = 19539 || (n > 19540 && n < 19549) || (n > 19553 && n < 19559) || n = 19560 || n = 19564 || n = 19567 || n = 19569 || n = 19570)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19535)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #176
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19537)
	; 10 bytes
	ldx #(n-297)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19540)
	; 10 bytes
	ldx #(n-502)/501
	ldy #98
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19549)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19550)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-7
.elseif(n = 19551)
	; 10 bytes
	ldx #(n-1)/391
	ldy #76
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19552)
	; 10 bytes
	ldx #(n-932)/931
	ldy #185
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19553)
	; 10 bytes
	ldx #(n-377)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19559)
	; 10 bytes
	ldx #136 ;hides 'dey'
	ldy #31
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19561)
	; 10 bytes
	ldx #(n-1)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19562)
	; 10 bytes
	ldx #(n-1)/631
	ldy #124
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19563)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #52
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 19565)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19566)
	; 10 bytes
	ldx #(n-1)/301
	ldy #58
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19568)
	; 10 bytes
	ldx #(n-1152)/1151
	ldy #229
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 19571 to 19609
.if(n = 19571)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19572 || n = 19575 || n = 19577 || n = 19578 || n = 19584 || n = 19586 || n = 19588 || n = 19591 || n = 19592 || n = 19596 || n = 19598 || n = 19602 || n = 19604 || n = 19606 || n = 19608)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19573)
	; 10 bytes
	ldx #(n-467)/466
	ldy #91
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19574)
	; 10 bytes
	ldx #(n-852)/851
	ldy #169
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19576)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19579)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19580)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #35
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 19581)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19582)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19583)
	; 10 bytes
	ldy #56 ;hides 'sec'
	delay_n 265
	iny
	Jpl *-7
.elseif(n = 19585)
	; 9 bytes
	ldy #51
	delay_n 379
	dey
	Jne *-5
.elseif(n = 19587)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19589)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19590)
	; 10 bytes
	ldx #(n-1)/1031
	ldy #205
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19593)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19594)
	; 10 bytes
	ldx #(n-312)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19595)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19597)
	; 10 bytes
	ldx #(n-1)/276
	ldy #54
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #18
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19600)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19601)
	; 10 bytes
	ldx #(n-1)/196
	ldy #37
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19603)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19605)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19607)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 221
	iny
	Jne *-7
.elseif(n = 19609)
	; 9 bytes
	ldy #152
	delay_n 124
	dey
	Jne *-5
.endif
.endif
.else
.if((n >= 19610) && (n < 19649))
; 19610 to 19648
.if(n = 19610 || (n > 19611 && n < 19617) || (n > 19617 && n < 19621) || n = 19622 || (n > 19625 && n < 19629) || n = 19630 || (n > 19631 && n < 19636) || n = 19637 || n = 19638 || n = 19640 || n = 19642 || n = 19644 || n = 19647)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19611)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19617)
	; 10 bytes
	ldx #(n-1)/1226
	ldy #244
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19621)
	; 10 bytes
	ldx #(n-1)/436
	ldy #85
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19623)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #43
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19624)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19625)
	; 10 bytes
	ldx #(n-447)/446
	ldy #87
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19629)
	; 10 bytes
	ldx #(n-702)/701
	ldy #139
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19631)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19636)
	; 10 bytes
	ldx #(n-232)/231
	ldy #45
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19639)
	; 10 bytes
	ldx #(n-1092)/1091
	ldy #217
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19641)
	; 10 bytes
	ldx #(n-1)/491
	ldy #96
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19643)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19645)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #177
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19646)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 326
	dey
	Jmi *-7
.elseif(n = 19648)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 19649 to 19687
.if(n = 19649 || n = 19652 || n = 19654 || n = 19656 || (n > 19657 && n < 19663) || n = 19665 || n = 19667 || n = 19668 || n = 19672 || n = 19673 || (n > 19674 && n < 19681) || n = 19682 || n = 19686)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19650)
	; 10 bytes
	ldx #(n-402)/401
	ldy #79
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19651)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19653)
	; 9 bytes
	ldx #67
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 19655)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #52
	dey
	Jpl *-1
	inx
	Jpl *-7
.elseif(n = 19657)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19663)
	; 10 bytes
	ldx #(n-1)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19664)
	; 10 bytes
	ldx #(n-1)/371
	ldy #72
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19666)
	; 10 bytes
	ldx #(n-172)/171
	ldy #32
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19669)
	; 10 bytes
	ldx #(n-1)/596
	ldy #117
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19670)
	; 10 bytes
	ldx #(n-1)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19671)
	; 10 bytes
	ldx #(n-282)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19674)
	; 10 bytes
	ldx #(n-192)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19681)
	; 10 bytes
	ldx #(n-247)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19683)
	; 10 bytes
	ldx #202 ;hides 'dex'
	ldy #102
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 19684)
	; 10 bytes
	ldx #(n-1)/81
	ldy #15
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19685)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19687)
	; 10 bytes
	ldx #(n-1)/386
	ldy #76
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 19688) && (n < 19844))
.if((n >= 19688) && (n < 19766))
.if((n >= 19688) && (n < 19727))
; 19688 to 19726
.if(n = 19688 || n = 19690 || n = 19692 || n = 19694 || n = 19698 || (n > 19699 && n < 19705) || (n > 19705 && n < 19709) || n = 19710 || n = 19712 || (n > 19713 && n < 19717) || n = 19718 || n = 19720 || n = 19725 || n = 19726)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19689)
	; 10 bytes
	ldx #(n-857)/856
	ldy #170
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19691)
	; 10 bytes
	ldx #110
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jne *-6
.elseif(n = 19693)
	; 10 bytes
	ldy #170 ;hides 'tax'
	delay_n 451
	dey
	Jmi *-7
.elseif(n = 19695)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19696)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19697)
	; 10 bytes
	ldx #(n-1)/1231
	ldy #245
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19699)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19705)
	; 10 bytes
	ldx #(n-1)/821
	ldy #163
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19709)
	; 10 bytes
	ldx #12
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 19711)
	; 10 bytes
	ldx #(n-1)/146
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19713)
	; 10 bytes
	ldx #(n-257)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19717)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19719)
	; 10 bytes
	ldx #216 ;hides 'cld'
	ldy #96
	dey
	Jpl *-1
	inx
	Jne *-7
.elseif(n = 19721)
	; 10 bytes
	ldx #(n-1)/116
	ldy #21
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19722)
	; 10 bytes
	ldx #(n-482)/481
	ldy #95
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19723)
	; 10 bytes
	ldx #(n-347)/346
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19724)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 19727 to 19765
.if(n = 19727 || n = 19728 || (n > 19730 && n < 19735) || n = 19736 || n = 19737 || (n > 19741 && n < 19748) || n = 19749 || (n > 19751 && n < 19755) || (n > 19755 && n < 19759) || n = 19760 || n = 19761 || n = 19764)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19729)
	; 10 bytes
	ldx #(n-1)/411
	ldy #81
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19730)
	; 10 bytes
	ldx #(n-182)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19735)
	; 10 bytes
	ldx #(n-287)/286
	ldy #55
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19738)
	; 9 bytes
	ldy #153
	delay_n 124
	dey
	Jne *-5
.elseif(n = 19739)
	; 10 bytes
	ldy #232 ;hides 'inx'
	delay_n 181
	dey
	Jmi *-7
.elseif(n = 19740)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19741)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19748)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19750)
	; 10 bytes
	ldx #(n-1)/681
	ldy #135
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19751)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #163
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 19755)
	; 10 bytes
	ldx #(n-167)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19759)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19762)
	; 10 bytes
	ldx #(n-942)/941
	ldy #187
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19763)
	; 10 bytes
	ldx #(n-242)/241
	ldy #46
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19765)
	; 10 bytes
	ldx #(n-1)/366
	ldy #72
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 19766) && (n < 19805))
; 19766 to 19804
.if(n = 19766 || n = 19768 || (n > 19769 && n < 19776) || n = 19778 || n = 19785 || n = 19786 || (n > 19789 && n < 19795) || n = 19796 || n = 19798 || n = 19800 || n = 19803)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19767)
	; 10 bytes
	ldx #56 ;hides 'sec'
	ldy #69
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 19769)
	; 10 bytes
	ldx #(n-707)/706
	ldy #140
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19776)
	; 10 bytes
	ldx #(n-792)/791
	ldy #157
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19777)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19779)
	; 10 bytes
	ldx #(n-342)/341
	ldy #66
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19780)
	; 10 bytes
	ldx #(n-1)/1041
	ldy #207
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19781)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19782)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19783)
	; 10 bytes
	ldx #(n-472)/471
	ldy #93
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19784)
	; 10 bytes
	ldx #(n-272)/271
	ldy #52
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19787)
	; 10 bytes
	ldx #(n-1)/761
	ldy #151
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19788)
	; 10 bytes
	ldx #(n-422)/421
	ldy #82
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19789)
	; 10 bytes
	ldx #(n-292)/291
	ldy #57
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19795)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 91
	dey
	Jne *-7
.elseif(n = 19797)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19799)
	; 10 bytes
	ldx #(n-522)/521
	ldy #102
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19801)
	; 10 bytes
	ldx #(n-1)/396
	ldy #77
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19802)
	; 10 bytes
	ldx #168 ;hides 'tay'
	ldy #95
	dey
	Jne *-1
	dex
	Jmi *-7
.elseif(n = 19804)
	; 10 bytes
	ldx #(n-162)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jpl *-6
.endif
.else
; 19805 to 19843
.if((n > 19804 && n < 19809) || n = 19810 || n = 19812 || (n > 19813 && n < 19819) || n = 19820 || n = 19822 || n = 19826 || n = 19828 || n = 19829 || n = 19832 || n = 19836 || n = 19838 || n = 19840 || n = 19842 || n = 19843)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19809)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 276
	iny
	Jne *-7
.elseif(n = 19811)
	; 10 bytes
	ldx #(n-567)/566
	ldy #112
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19813)
	; 10 bytes
	ldx #(n-382)/381
	ldy #74
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19819)
	; 10 bytes
	ldx #(n-1102)/1101
	ldy #219
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19821)
	; 10 bytes
	ldx #(n-992)/991
	ldy #197
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19823)
	; 10 bytes
	ldx #(n-1)/106
	ldy #19
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19824)
	; 10 bytes
	ldx #(n-462)/461
	ldy #91
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19825)
	; 10 bytes
	ldx #(n-1)/236
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19827)
	; 10 bytes
	ldx #(n-432)/431
	ldy #84
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19830)
	; 10 bytes
	ldx #(n-252)/251
	ldy #49
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19831)
	; 10 bytes
	ldx #(n-1)/661
	ldy #131
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19833)
	; 10 bytes
	ldx #(n-1)/296
	ldy #57
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19834)
	; 10 bytes
	ldx #(n-1)/601
	ldy #119
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19835)
	; 10 bytes
	ldx #(n-1)/211
	ldy #40
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19837)
	; 10 bytes
	ldx #(n-262)/261
	ldy #51
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19839)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19841)
	; 10 bytes
	ldx #(n-1)/496
	ldy #98
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
.if((n >= 19844) && (n < 19883))
; 19844 to 19882
.if(n = 19844 || (n > 19846 && n < 19856) || (n > 19857 && n < 19861) || (n > 19861 && n < 19865) || n = 19866 || n = 19868 || n = 19874 || (n > 19875 && n < 19881))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19845)
	; 10 bytes
	ldx #(n-452)/451
	ldy #88
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19846)
	; 10 bytes
	ldx #(n-1)/441
	ldy #86
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19856)
	; 10 bytes
	ldx #(n-1)/361
	ldy #70
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19857)
	; 10 bytes
	ldx #(n-1)/136
	ldy #26
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19861)
	; 10 bytes
	ldx #(n-1)/331
	ldy #64
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19865)
	; 10 bytes
	ldx #(n-1)/191
	ldy #37
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19867)
	; 9 bytes
	ldy #154
	delay_n 124
	dey
	Jne *-5
.elseif(n = 19869)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19870)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19871)
	; 10 bytes
	ldy #202 ;hides 'dex'
	delay_n 361
	iny
	Jne *-7
.elseif(n = 19872)
	; 10 bytes
	ldx #(n-1)/641
	ldy #126
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19873)
	; 10 bytes
	ldx #(n-217)/216
	ldy #42
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19875)
	; 10 bytes
	ldx #(n-1)/1046
	ldy #208
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19881)
	; 10 bytes
	ldx #69
	ldy #216 ;hides 'cld'
	iny
	Jne *-2
	dex
	Jpl *-6
.elseif(n = 19882)
	; 10 bytes
	ldx #(n-1)/141
	ldy #27
	dey
	Jne *-1
	dex
	Jne *-6
.endif
.else
; 19883 to 19921
.if((n > 19882 && n < 19887) || n = 19888 || n = 19890 || (n > 19891 && n < 19895) || n = 19896 || n = 19897 || n = 19899 || n = 19902 || n = 19904 || n = 19906 || n = 19907 || n = 19910 || n = 19912 || (n > 19913 && n < 19917) || n = 19918 || n = 19920)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19887)
	; 10 bytes
	ldx #(n-327)/326
	ldy #63
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19889)
	; 10 bytes
	ldx #(n-227)/226
	ldy #44
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19891)
	; 10 bytes
	ldx #(n-222)/221
	ldy #43
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19895)
	; 10 bytes
	ldx #(n-407)/406
	ldy #79
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19898)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19900)
	; 10 bytes
	ldx #(n-202)/201
	ldy #39
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19901)
	; 10 bytes
	ldx #(n-797)/796
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19903)
	; 10 bytes
	ldx #(n-322)/321
	ldy #63
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19905)
	; 10 bytes
	ldx #(n-1)/311
	ldy #61
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19908)
	; 10 bytes
	ldx #(n-1172)/1171
	ldy #233
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19909)
	; 10 bytes
	ldx #(n-317)/316
	ldy #62
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19911)
	; 10 bytes
	ldx #(n-1)/181
	ldy #35
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19913)
	; 10 bytes
	ldx #(n-1)/131
	ldy #25
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19917)
	; 10 bytes
	ldx #(n-1)/766
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19919)
	; 10 bytes
	ldx #(n-867)/866
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19921)
	; 10 bytes
	ldx #(n-1)/166
	ldy #31
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 19922) && (n < 19961))
; 19922 to 19960
.if(n = 19922 || n = 19924 || n = 19926 || n = 19931 || n = 19932 || (n > 19933 && n < 19937) || (n > 19937 && n < 19941) || n = 19943 || (n > 19945 && n < 19949) || n = 19950 || (n > 19953 && n < 19961))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19923)
	; 10 bytes
	ldx #136 ;hides 'dey'
	delay_n 284
	dex
	dex
	Jne *-7
.elseif(n = 19925)
	; 10 bytes
	ldx #(n-1)/586
	ldy #115
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19927)
	; 10 bytes
	ldx #(n-1)/246
	ldy #47
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19928)
	; 10 bytes
	ldx #200 ;hides 'iny'
	ldy #52
	dey
	Jpl *-1
	dex
	Jmi *-7
.elseif(n = 19929)
	; 10 bytes
	ldx #(n-1)/376
	ldy #73
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19930)
	; 10 bytes
	ldx #(n-1)/91
	ldy #16
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19933)
	; 10 bytes
	ldx #(n-1)/151
	ldy #28
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19937)
	; 10 bytes
	ldx #(n-1)/356
	ldy #69
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19941)
	; 10 bytes
	ldy #186 ;hides 'tsx'
	delay_n 331
	dey
	Jmi *-7
.elseif(n = 19942)
	; 9 bytes
	ldx #68
	delay_n 284
	dex
	Jpl *-5
.elseif(n = 19944)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19945)
	; 10 bytes
	ldx #(n-1)/831
	ldy #165
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19949)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #158
	dey
	Jne *-1
	dex
	Jpl *-7
.elseif(n = 19951)
	; 10 bytes
	ldx #(n-267)/266
	ldy #52
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19952)
	; 10 bytes
	ldx #(n-1)/281
	ldy #55
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19953)
	; 10 bytes
	ldx #(n-1)/86
	ldy #15
	dey
	Jpl *-1
	dex
	Jne *-6
.endif
.else
; 19961 to 20000
.if((n > 19960 && n < 19965) || n = 19967 || n = 19968 || n = 19973 || n = 19974 || (n > 19975 && n < 19981) || n = 19985 || n = 19987 || n = 19988 || n = 19990 || n = 19992 || n = 19994 || n = 19995 || n = 19997 || n = 20000)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19965)
	; 10 bytes
	ldx #(n-1)/161
	ldy #31
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19966)
	; 10 bytes
	ldx #(n-1)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
.elseif(n = 19969)
	; 9 bytes
	ldy #52
	delay_n 379
	dey
	Jne *-5
.elseif(n = 19970)
	; 10 bytes
	ldx #(n-1)/1051
	ldy #209
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19971 || n = 19984 || n = 19998)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19972)
	; 10 bytes
	ldx #(n-952)/951
	ldy #189
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19975)
	; 10 bytes
	ldx #234 ;hides 'nop'
	ldy #180
	dey
	Jne *-1
	inx
	Jne *-7
.elseif(n = 19981)
	; 10 bytes
	ldx #(n-1)/111
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
.elseif(n = 19982)
	; 9 bytes
	ldx #29
	delay_n 684
	dex
	Jne *-5
.elseif(n = 19983)
	; 10 bytes
	ldx #(n-207)/206
	ldy #39
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19986)
	; 10 bytes
	ldx #(n-572)/571
	ldy #112
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19989)
	; 10 bytes
	ldx #(n-527)/526
	ldy #104
	dey
	Jne *-1
	dex
	Jpl *-6
.elseif(n = 19991)
	; 10 bytes
	ldx #24 ;hides 'clc'
	ldy #165
	dey
	Jne *-1
	dex
	Jne *-7
.elseif(n = 19993)
	; 10 bytes
	ldx #(n-477)/476
	ldy #93
	dey
	Jpl *-1
	dex
	Jpl *-6
.elseif(n = 19996)
	; 9 bytes
	ldy #155
	delay_n 124
	dey
	Jne *-5
.elseif(n = 19999)
	; 10 bytes
	ldx #(n-1)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
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
