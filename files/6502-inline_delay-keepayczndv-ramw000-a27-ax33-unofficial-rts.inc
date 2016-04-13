; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X
; Will preserve A, Y, S, and P
;
; WARNING: May write garbage into $00-$FF.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

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
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 14
; - 5 bytes or less when n <= 16
; - 6 bytes or less when n <= 21
; - 7 bytes or less when n <= 26
; - 8 bytes or less when n <= 31
; - 9 bytes or less when n <= 1306
; -10 bytes or less when n <= 1324
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.8, max=6
; - When 21 <= n <= 30: min=6, avg=7.1, max=8
; - When 31 <= n <= 50: min=7, avg=8.2, max=9
; - When 51 <= n <= 75: min=7, avg=8.2, max=9
; - When 76 <= n <= 100: min=7, avg=8.3, max=9
; - When 101 <= n <= 200: min=6, avg=8.2, max=9
; - When 201 <= n <= 300: min=7, avg=8.2, max=9
; - When 301 <= n <= 500: min=6, avg=8.2, max=9
; - When 501 <= n <= 750: min=7, avg=8.2, max=9
; - When 751 <= n <= 1000: min=7, avg=8.3, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.9, max=11
; - When 2001 <= n <= 3000: min=8, avg=9.9, max=11
; - When 3001 <= n <= 5000: min=9, avg=10.9, max=11
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
	sta @zptemp
.elseif(n = 4 || n = 6 || n = 15 || n = 22 || n = 26 || n = 30 || n = 35 || n = 37)
	; 2..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 3 bytes
	delay_n (n-3)
	sta @zptemp
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || n = 19 || n = 31)
	; 3..8 bytes
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
.elseif(n = 14 || n = 21)
	; 4..6 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 16 || n = 18)
	; 5..6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 17)
	; 6 bytes
	php
	rol $26
	ror $26
	plp
.elseif(n = 20)
	; 6 bytes
	php
	pha
	inc $F6,X
	pla
	plp
.elseif(n = 23)
	; 7 bytes
	nop
	delay_n (n-2)
.elseif(n = 24 || n = 25)
	; 6..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 8 bytes
	php
	inc $F6,X
	.repeat 2
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 28 || n = 33)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 29 || n = 36)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 32)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 34)
	; 9 bytes
	php
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	plp
.elseif(n = 38)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 39)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.endif
.else
; 40 to 78
.if((n .mod 5 = 0) && (n = 40 || n = 45 || (n > 54 && n < 76)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 41)
	; 9 bytes
	php
	tax
	lda #1
	jsr delay_a_27_clocks
	txa
	plp
.elseif(n = 42)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 43 || n = 53 || n = 58 || n = 63 || n = 73)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 44 || n = 46 || n = 47 || n = 49 || n = 51 || n = 52 || n = 54 || n = 56 || n = 59 || n = 61 || n = 66 || n = 67 || n = 69 || n = 72 || n = 74 || n = 76 || n = 77)
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 48 || n = 68 || n = 78)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 50 || n = 64)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 57 || n = 71)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 62)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	inx
	Jne *-2
	plp
.endif
.endif
.else
; 79 to 156
.if(n = 79 || n = 81 || n = 82 || n = 84 || n = 86 || n = 87 || n = 89 || n = 91 || n = 94 || n = 96 || n = 97 || n = 101 || n = 102 || n = 104 || n = 107 || n = 109 || n = 111 || n = 112 || n = 116 || n = 117 || n = 119 || n = 121 || n = 122 || n = 124 || n = 126 || n = 129 || n = 139 || n = 142 || n = 144 || n = 146 || n = 147 || n = 149 || n = 151 || n = 152 || n = 154 || n = 156)
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif((n .mod 5 = 0) && (n = 80 || (n > 89 && n < 116) || (n > 124 && n < 151)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 83 || n = 88 || n = 103 || n = 128 || n = 138 || n = 153)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 85 || n = 99 || n = 127 || n = 134 || n = 155)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 92 || n = 106 || n = 120 || n = 141)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 93 || n = 98 || n = 108 || n = 113 || n = 118 || n = 123 || n = 133 || n = 143 || n = 148)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 114)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 131)
	; 6 bytes
	php
	ldx #232 ;hides 'inx'
	Jne *-1
	plp
.elseif(n = 132)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	dex
	dex
	Jpl *-3
	plp
.elseif(n = 136)
	; 8 bytes
	php
	inc @zptemp
	ldx #232 ;hides 'inx'
	Jne *-1
	plp
.elseif(n = 137)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	Jne *-1
	inc @zptemp,x
	plp
.endif
.endif
.else
; 157 to 312
.if(n = 157 || n = 159 || n = 161 || n = 164 || n = 166 || n = 167 || n = 171 || n = 172 || n = 179 || n = 182 || n = 184 || n = 187 || n = 189 || n = 191 || n = 192 || n = 194 || n = 196 || n = 199 || n = 201 || n = 202 || n = 207 || n = 209 || n = 212 || n = 214 || n = 216 || n = 217 || n = 219 || n = 221 || n = 224 || n = 226 || n = 227 || n = 229 || n = 234 || n = 236 || n = 237 || n = 241 || n = 242 || n = 244 || n = 247 || n = 251 || n = 252 || n = 254 || n = 256 || n = 257 || n = 259 || n = 261 || n = 262 || n = 264 || n = 266 || n = 269 || n = 271 || n = 277 || n = 279 || n = 282 || n = 284 || n = 289 || n = 291 || n = 292 || n = 294 || n = 296 || n = 297)
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif((n .mod 5 = 3) && ((n > 157 && n < 179) || n = 208 || n = 233 || n = 248 || n = 258 || n = 263 || (n > 272 && n < 289) || n = 308))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 160)
	; 7 bytes
	php
	ldx #234 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif((n .mod 7 = 1) && (n = 162 || n = 169 || n = 197 || n = 204 || (n > 259 && n < 282) || (n > 294 && n < 310)))
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 165 || n = 170 || n = 175 || n = 180 || n = 185 || n = 200 || n = 205 || n = 210 || n = 215 || n = 220 || n = 230 || n = 235 || n = 240 || n = 245 || n = 250 || n = 255 || n = 265 || n = 270 || n = 275 || n = 280 || n = 285 || n = 290 || n = 300 || n = 305 || n = 307 || n = 310)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 174)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	dex
	Jne *-2
	plp
.elseif(n = 176 || n = 190 || n = 211 || n = 225 || n = 232 || n = 239 || n = 246)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 177)
	; 8 bytes
	php
	ldx #90 ;hides 'nop'
	inx
	inx
	Jpl *-3
	plp
.elseif(n = 181)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	dex
	Jpl *-2
	plp
.elseif((n .mod 5 = 3) && ((n > 182 && n < 204) || (n > 212 && n < 229) || n = 238 || n = 243 || n = 253 || n = 268 || (n > 292 && n < 304)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 186)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 195)
	; 7 bytes
	php
	ldx #(n-13)/7 ;hides 'nop'
	dex
	Jpl *-2
	plp
.elseif(n = 206)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	sta @zptemp
	Jne *-3
	plp
.elseif(n = 222)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 231)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	dex
	Jpl *-3
	plp
.elseif(n = 249)
	; 8 bytes
	php
	ldx #(n-15)/9 ;hides 'nop'
	nop
	dex
	Jpl *-3
	plp
.elseif(n = 272)
	; 7 bytes
	php
	ldx #202 ;hides 'dex'
	dex
	Jmi *-2
	plp
.elseif(n = 276)
	; 8 bytes
	php
	ldx #58 ;hides 'nop'
	dex
	dex
	Jpl *-3
	plp
.elseif(n = 286)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	inx
	Jne *-2
	plp
.elseif(n = 287)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 299)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 301)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 304 || n = 312)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 306)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 311)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	dex
	Jpl *-3
	plp
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if((n .mod 5 = 3) && ((n > 312 && n < 324) || n = 338 || (n > 347 && n < 359) || (n > 367 && n < 389)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 314)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 315 || n = 317 || n = 320 || n = 322 || n = 325 || n = 327 || n = 335 || n = 339 || n = 340 || n = 342 || n = 345 || n = 347 || n = 350 || n = 355 || n = 357 || n = 360 || n = 362 || n = 370 || n = 374 || n = 375 || n = 377 || n = 380 || n = 382 || n = 385 || n = 387 || n = 390)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 316 || n = 330 || n = 337 || n = 351 || n = 372)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 319 || n = 324 || n = 329 || n = 334 || n = 349 || n = 354 || n = 364 || n = 369)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 321)
	; 8 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	inx
	Jpl *-3
	plp
.elseif(n = 326 || n = 341 || n = 356 || n = 361 || n = 371 || n = 381)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 328 || n = 333 || n = 343 || n = 363)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 331 || n = 346)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 332 || n = 367)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 336 || n = 352 || n = 376)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 344 || n = 365 || n = 379)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 359)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 366)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 384)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 386)
	; 6 bytes
	php
	ldx #202 ;hides 'dex'
	Jmi *-1
	plp
.elseif(n = 389)
	; 8 bytes
	php
	ldx $A6
	ldx #202 ;hides 'dex'
	Jmi *-1
	plp
.endif
.else
; 391 to 468
.if(n = 391)
	; 8 bytes
	php
	inc @zptemp
	ldx #202 ;hides 'dex'
	Jmi *-1
	plp
.elseif(n = 392)
	; 8 bytes
	php
	inc $F6,X
	ldx #202 ;hides 'dex'
	Jmi *-1
	plp
.elseif((n .mod 5 = 3) && ((n > 392 && n < 409) || n = 418 || n = 433 || n = 448 || (n > 457 && n < 469)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 394 || n = 399 || n = 429 || n = 434 || n = 439 || n = 454 || n = 459)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 395 || n = 397 || n = 409 || n = 410 || n = 415 || n = 417 || n = 420 || n = 422 || n = 425 || n = 427 || n = 430 || n = 440 || n = 444 || n = 445 || n = 447 || n = 450 || n = 452 || n = 455 || n = 457 || n = 460 || n = 462 || n = 465 || n = 467)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 396 || n = 401 || n = 406 || n = 436 || n = 451 || n = 461 || n = 466)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 400 || n = 456)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 402)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 404)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 405)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
	plp
.elseif(n = 407 || n = 414 || n = 421 || n = 435 || n = 442 || n = 449)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 411)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	dex
	Jmi *-3
	plp
.elseif(n = 412)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 413 || n = 423 || n = 428 || n = 438 || n = 443 || n = 453)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 416 || n = 432)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 419)
	; 7 bytes
	php
	ldx #(n-13)/7 ;hides 'nop'
	dex
	Jpl *-2
	plp
.elseif(n = 424)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	plp
.elseif(n = 426 || n = 431 || n = 441 || n = 446)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 437)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 464)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 499 || n = 509 || n = 529 || n = 534)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 470 || n = 484 || n = 491 || n = 512 || n = 519 || n = 526 || n = 540)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 471)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 472 || n = 504 || n = 536)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 473 || n = 478 || n = 508 || n = 528)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 474)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	inx
	Jpl *-3
	plp
.elseif(n = 475 || n = 480 || n = 482 || n = 485 || n = 489 || n = 490 || n = 495 || n = 497 || n = 500 || n = 502 || n = 514 || n = 515 || n = 517 || n = 520 || n = 522 || n = 525 || n = 527 || n = 530 || n = 532 || n = 535 || n = 542 || n = 545)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 476 || n = 481 || n = 511 || n = 531)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 477 || n = 505)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 479)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	inx
	Jne *-3
	plp
.elseif((n .mod 5 = 3) && ((n > 482 && n < 504) || (n > 512 && n < 524) || (n > 532 && n < 544)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 486 || n = 501 || n = 506 || n = 516 || n = 521 || n = 541 || n = 546)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 487)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 492)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
	plp
.elseif(n = 494)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 496)
	; 7 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	Jpl *-2
	plp
.elseif(n = 507)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 510)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
	plp
.elseif(n = 524)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 537)
	; 8 bytes
	php
	ldx #(n-15)/9 ;hides 'nop'
	nop
	dex
	Jpl *-3
	plp
.elseif(n = 539)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 544)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.endif
.else
; 547 to 625
.if(n = 547 || n = 554 || n = 561 || n = 582 || n = 624)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 548 || n = 553 || n = 563 || n = 568 || n = 578 || n = 583 || n = 588 || n = 603 || n = 613 || n = 618 || n = 623)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 549 || n = 550 || n = 555 || n = 560 || n = 562 || n = 565 || n = 569 || n = 570 || n = 577 || n = 580 || n = 585 || n = 587 || n = 590 || n = 595 || n = 597 || n = 600 || n = 605 || n = 607 || n = 612 || n = 615 || n = 619 || n = 620 || n = 622 || n = 625)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 551)
	; 8 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 552 || n = 592 || n = 616)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 556 || n = 566 || n = 571 || n = 581 || n = 586 || n = 591 || n = 606 || n = 621)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 557 || n = 602)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 558 || n = 573 || n = 593 || n = 598 || n = 608)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 559)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 564)
	; 8 bytes
	php
	ldx #250 ;hides 'nop'
	dex
	dex
	Jmi *-3
	plp
.elseif(n = 567)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 572)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 574 || n = 579 || n = 594 || n = 599 || n = 604)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 575 || n = 589 || n = 596 || n = 610 || n = 617)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 576)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 584)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 601 || n = 611)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 609)
	; 8 bytes
	php
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 614)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	sta @zptemp
	Jmi *-3
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
; 626 to 781
.if(n = 626 || n = 641 || n = 656)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 627 || n = 630 || n = 635 || n = 637 || n = 640 || n = 642 || n = 650 || n = 655 || n = 657 || n = 660 || n = 662 || n = 665 || n = 667 || n = 670 || n = 675 || n = 677 || n = 682 || n = 685 || n = 689 || n = 690 || n = 695 || n = 697 || n = 700 || n = 702 || n = 705 || n = 707 || n = 710 || n = 724 || n = 725 || n = 727 || n = 730 || n = 732 || n = 735 || n = 740 || n = 742 || n = 745 || n = 747 || n = 759 || n = 760 || n = 765 || n = 767 || n = 770 || n = 772 || n = 775 || n = 777 || n = 780)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && (n = 628 || n = 633 || n = 643 || n = 648 || (n > 657 && n < 779)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 629)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
	plp
.elseif(n = 631 || n = 659 || n = 666 || n = 736)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 632 || n = 664 || n = 712 || n = 744 || n = 776)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 634 || n = 639 || n = 649 || n = 669 || n = 679 || n = 684 || n = 709 || n = 714 || n = 739 || n = 749 || n = 754 || n = 769 || n = 774 || n = 779)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 636)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 638 || n = 653)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 644)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && (n = 645 || n = 652 || (n > 679 && n < 702) || (n > 714 && n < 730) || (n > 749 && n < 772)))
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif((n .mod 5 = 1) && (n = 646 || n = 651 || n = 661 || n = 671 || n = 676 || n = 686 || n = 691 || (n > 705 && n < 732) || n = 741 || n = 746 || (n > 755 && n < 767) || n = 781))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 647 || n = 737)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 654)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 672 || n = 696 || n = 704 || n = 752)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 674 || n = 692 || n = 719)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 681)
	; 8 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 699)
	; 8 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 717)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 720)
	; 7 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
	plp
.elseif(n = 734)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
	plp
.elseif(n = 751)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 755)
	; 7 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
	plp
.elseif(n = 762)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
	plp
.endif
.else
; 782 to 937
.if(n = 782 || n = 809 || n = 854)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif((n .mod 5 = 3) && ((n > 782 && n < 934)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 784 || n = 824 || n = 832 || n = 856 || n = 864 || n = 896 || n = 936)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 785 || n = 792 || n = 806 || n = 820 || n = 841 || n = 855 || n = 862 || n = 869 || n = 890 || n = 911)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif((n .mod 5 = 1) && ((n > 785 && n < 802) || n = 811 || (n > 820 && n < 837) || n = 846 || n = 851 || (n > 860 && n < 872) || (n > 880 && n < 892) || n = 901 || (n > 915 && n < 932)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 787 || n = 822 || n = 857 || n = 892 || n = 927)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 789)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 790 || n = 794 || n = 795 || n = 797 || n = 800 || n = 802 || n = 805 || n = 810 || n = 815 || n = 817 || n = 825 || n = 829 || n = 830 || n = 835 || n = 837 || n = 840 || n = 842 || n = 845 || n = 847 || n = 850 || n = 852 || n = 865 || n = 870 || n = 875 || n = 877 || n = 880 || n = 882 || n = 885 || n = 887 || n = 895 || n = 900 || n = 902 || n = 905 || n = 907 || n = 910 || n = 915 || n = 917 || n = 920 || n = 922 || n = 930 || n = 934 || n = 935 || n = 937)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 799 || n = 827 || n = 834 || n = 876 || n = 897 || n = 904 || n = 925 || n = 932)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 804)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 7
	dex
	Jpl *-4
	plp
.elseif(n = 807)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 812)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 814 || n = 819 || n = 844 || n = 849 || n = 859 || n = 874 || n = 879 || n = 889 || n = 894 || n = 909 || n = 914 || n = 919 || n = 929)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 816)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 839)
	; 9 bytes
	php
	pha
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
	pla
	plp
.elseif(n = 860)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 867)
	; 7 bytes
	php
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
	plp
.elseif(n = 872 || n = 899)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 884)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 906)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 912)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 924)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	nop
	inx
	Jpl *-3
	plp
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if((n .mod 5 = 3) && ((n > 937 && n < 1094)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 7 = 1) && (n = 939 || n = 946 || (n > 959 && n < 982) || (n > 994 && n < 1017) || (n > 1029 && n < 1052) || (n > 1064 && n < 1087)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 940 || n = 945 || n = 947 || n = 950 || n = 955 || n = 957 || n = 965 || n = 970 || n = 972 || n = 975 || n = 977 || n = 980 || n = 982 || n = 985 || n = 990 || n = 1000 || n = 1005 || n = 1007 || n = 1010 || n = 1012 || n = 1015 || n = 1017 || n = 1020 || n = 1022 || n = 1025 || n = 1035 || n = 1039 || n = 1040 || n = 1042 || n = 1045 || n = 1047 || n = 1050 || n = 1052 || n = 1055 || n = 1057 || n = 1060 || n = 1062 || n = 1070 || n = 1074 || n = 1075 || n = 1080 || n = 1082 || n = 1085 || n = 1087 || n = 1090 || n = 1092)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && (n = 941 || (n > 950 && n < 972) || (n > 985 && n < 1012) || n = 1031 || n = 1036 || n = 1046 || (n > 1060 && n < 1082) || n = 1091))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 942)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 944)
	; 7 bytes
	php
	ldx #122 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif(n = 949 || n = 954 || n = 959 || n = 964 || n = 994 || n = 999 || n = 1019 || n = 1029 || n = 1049 || n = 1054 || n = 1069 || n = 1084 || n = 1089)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 952 || n = 1032 || n = 1056 || n = 1064)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 962 || n = 997 || n = 1067)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 969)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 976 || n = 984 || n = 992)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 979)
	; 9 bytes
	php
	pha
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 987)
	; 8 bytes
	php
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 989 || n = 1034)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 1004)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1014)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jpl *-4
	plp
.elseif(n = 1021)
	; 6 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	plp
.elseif(n = 1024)
	; 8 bytes
	php
	ldx $A6
	ldx #202 ;hides 'dex'
	Jne *-1
	plp
.elseif(n = 1026)
	; 8 bytes
	php
	inc @zptemp
	ldx #202 ;hides 'dex'
	Jne *-1
	plp
.elseif(n = 1027)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 1041)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 1059)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1077)
	; 9 bytes
	php
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && (n = 1094 || n = 1099 || n = 1119 || (n > 1128 && n < 1140) || (n > 1153 && n < 1165) || n = 1174 || n = 1189 || (n > 1198 && n < 1210) || (n > 1228 && n < 1240)))
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 1095 || n = 1097 || n = 1105 || n = 1109 || n = 1110 || n = 1115 || n = 1117 || n = 1120 || n = 1125 || n = 1127 || n = 1130 || n = 1132 || n = 1140 || n = 1145 || n = 1147 || n = 1150 || n = 1155 || n = 1157 || n = 1160 || n = 1162 || n = 1165 || n = 1167 || n = 1175 || n = 1179 || n = 1180 || n = 1182 || n = 1185 || n = 1187 || n = 1190 || n = 1195 || n = 1197 || n = 1200 || n = 1202 || n = 1210 || n = 1215 || n = 1217 || n = 1220 || n = 1222 || n = 1225 || n = 1227 || n = 1230 || n = 1232 || n = 1235 || n = 1237 || n = 1245 || n = 1249 || n = 1250)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1096 || n = 1112 || n = 1136 || n = 1144 || n = 1152 || n = 1176 || n = 1192 || n = 1216 || n = 1224)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 5 = 3) && ((n > 1097 && n < 1249)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1122) || (n > 1134 && n < 1157) || (n > 1169 && n < 1192) || (n > 1204 && n < 1227) || n = 1240 || n = 1247))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif((n .mod 5 = 1) && ((n > 1100 && n < 1117) || n = 1126 || (n > 1140 && n < 1152) || (n > 1160 && n < 1172) || n = 1181 || n = 1186 || (n > 1195 && n < 1212) || n = 1221 || (n > 1230 && n < 1247)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 1102 || n = 1137 || n = 1207 || n = 1242)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1104)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1122)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1124 || n = 1214)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1131)
	; 8 bytes
	php
	ldx #250 ;hides 'nop'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1169)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 1172 || n = 1244)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 1194)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	dex
	Jne *-4
	plp
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
.if((n .mod 5 = 1) && (n = 1251 || (n > 1270 && n < 1287)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 1252 || n = 1255 || n = 1260 || n = 1262 || n = 1265 || n = 1267 || n = 1270 || n = 1280 || n = 1284 || n = 1285 || n = 1287)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 1252 && n < 1284)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 1254 || n = 1261 || n = 1275 || n = 1282 || n = 1289)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1256 || n = 1264 || n = 1272)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1257)
	; 8 bytes
	php
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1259)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1266)
	; 8 bytes
	php
	ldx #116 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1269 || n = 1274 || n = 1279)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	inc @zptemp,x
	plp
.elseif(n = 1277)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1288)
	; 7 bytes
	php
	ldx #0
	dex
	Jne *-1
	plp
.endif
.else
; 1290 to 1328
.if(n = 1290 || n = 1292 || n = 1298 || n = 1305 || n = 1307 || n = 1314 || n = 1315 || n = 1319 || n = 1326)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1291)
	; 9 bytes
	php
	ldx $A6
	ldx #0
	dex
	Jne *-1
	plp
.elseif(n = 1293)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 1294)
	; 7 bytes
	php
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1295)
	; 9 bytes
	php
	pha
	ldx #0
	dex
	Jne *-1
	pla
	plp
.elseif((n .mod 7 = 1) && ((n > 1295 && n < 1325)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1297)
	; 9 bytes
	php
	ldx $A6
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1299)
	; 9 bytes
	php
	inc @zptemp
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1300)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1301)
	; 9 bytes
	php
	pha
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1302)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $90A2'
	ldx #144
	dex
	Jne *-4
	plp
.elseif(n = 1304 || n = 1313 || n = 1322)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1306)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 1308 || n = 1318)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1309 || n = 1323)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1311)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $91A2'
	ldx #145
	dex
	Jne *-4
	plp
.elseif(n = 1312 || n = 1320 || n = 1328)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1316)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 1321)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $92A2'
	ldx #146
	dex
	Jne *-4
	plp
.elseif(n = 1325)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1327)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.endif
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $93A2'
	ldx #147
	dex
	Jne *-4
	plp
.elseif(n = 1330)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $6DA2'
	ldx #109
	inx
	Jne *-4
	plp
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1367)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1332)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	inc $F6,X
	inx
	Jpl *-4
	plp
.elseif(n = 1333 || n = 1335 || n = 1347 || n = 1351 || n = 1354 || (n > 1360 && n < 1364))
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1334)
	; 9 bytes
	php
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1336 || n = 1344 || n = 1360)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1337)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jmi *-4
	plp
.elseif(n = 1339)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $6CA2'
	ldx #108
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1339 && n < 1368)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1341 || n = 1355)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1342)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1343)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-13)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1346)
	; 9 bytes
	php
	ldx #122 ;hides 'nop'
	nop $04
	inx
	Jne *-4
	plp
.elseif(n = 1348)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1350)
	; 9 bytes
	php
	ldx #121
	inc $F6,X
	dex
	Jpl *-3
	plp
.elseif(n = 1353)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1356)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $6AA2'
	ldx #106
	inx
	Jne *-4
	plp
.elseif(n = 1357)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $6AA2'
	ldx #106
	inx
	Jne *-4
	plp
.elseif(n = 1364)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1365)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $97A2'
	ldx #151
	dex
	Jne *-4
	plp
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1384 || n = 1400)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1369 || n = 1390 || n = 1404)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1370)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 1371)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	inx
	Jpl *-5
	plp
.elseif(n = 1372 || n = 1405)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1372 && n < 1402)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1374)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $98A2'
	ldx #152
	dex
	Jne *-4
	plp
.elseif(n = 1375 || n = 1377 || n = 1382 || n = 1386 || n = 1389 || n = 1391 || n = 1396)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1376 || n = 1385 || n = 1403)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1378 || n = 1398)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1379)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1381)
	; 9 bytes
	php
	ldx #250 ;hides 'nop'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 1383)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1388)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 1392)
	; 7 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif(n = 1393)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $9AA2'
	ldx #154
	dex
	Jne *-4
	plp
.elseif(n = 1395)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1397)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	inc $F6,X
	dex
	Jmi *-4
	plp
.elseif(n = 1399)
	; 9 bytes
	php
	pha
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
	pla
	plp
.elseif(n = 1402)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $9BA2'
	ldx #155
	dex
	Jne *-4
	plp
.elseif(n = 1406)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
	plp
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1435)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1444)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1409)
	; 9 bytes
	php
	ldx $A6
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
	plp
.elseif(n = 1410 || n = 1414 || n = 1417 || n = 1423 || n = 1426 || n = 1431 || n = 1438 || n = 1441 || n = 1442 || n = 1445)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1411)
	; 9 bytes
	php
	inc @zptemp
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
	plp
.elseif((n .mod 9 = 8) && ((n > 1411 && n < 1440)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1413)
	; 9 bytes
	php
	pha
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
	pla
	plp
.elseif((n .mod 8 = 0) && ((n > 1415 && n < 1441)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1418 || n = 1428)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1419)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $63A2'
	ldx #99
	inx
	Jne *-4
	plp
.elseif(n = 1420)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $63A2'
	ldx #99
	inx
	Jne *-4
	plp
.elseif(n = 1425)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1427)
	; 7 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	plp
.elseif(n = 1433)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1434)
	; 9 bytes
	php
	pha
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	pla
	plp
.elseif(n = 1437)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $61A2'
	ldx #97
	inx
	Jne *-4
	plp
.elseif(n = 1444)
	; 10 bytes
	php
	ldx #130 ;hides 'nop #imm'
	inc $EA,X ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.endif
.else
; 1446 to 1484
.if(n = 1446)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
	plp
.elseif(n = 1447)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
	plp
.elseif(n = 1448 || n = 1466 || n = 1475 || n = 1484)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1449)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1449 && n < 1479)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1451)
	; 10 bytes
	php
	ldx #110
	nop
	inc $F6,X
	dex
	Jpl *-4
	plp
.elseif(n = 1452 || n = 1454 || n = 1459 || n = 1461 || n = 1473 || n = 1477)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1453 || n = 1467 || n = 1481)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1455)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $A1A2'
	ldx #161
	dex
	Jne *-4
	plp
.elseif(n = 1456 || n = 1472 || n = 1480)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1458 || n = 1468)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1460)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 1462)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jpl *-4
	plp
.elseif(n = 1463)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1465)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $A2A2'
	ldx #162
	dex
	Jne *-4
	plp
.elseif(n = 1469 || n = 1476)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-13)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1470)
	; 9 bytes
	php
	ldx #(n-6)/12 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 1474)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $A3A2'
	ldx #163
	dex
	Jne *-4
	plp
.elseif(n = 1479)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1482)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $A4A2'
	ldx #164
	dex
	Jne *-4
	plp
.elseif(n = 1483)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $A4A2'
	ldx #164
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if((n .mod 7 = 1) && ((n > 1484 && n < 1521)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1486 || n = 1487 || n = 1489 || n = 1490 || n = 1494 || n = 1501 || n = 1505 || n = 1514 || n = 1515 || n = 1517 || n = 1522)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 0) && ((n > 1487 && n < 1513)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1491)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $A5A2'
	ldx #165
	dex
	Jne *-4
	plp
.elseif(n = 1493 || n = 1502)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1495 || n = 1516)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1497)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-13)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1498)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1500)
	; 8 bytes
	php
	ldx #90 ;hides 'nop'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1503)
	; 8 bytes
	php
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1507)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 7
	dex
	Jmi *-5
	plp
.elseif(n = 1508)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1509)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $A7A2'
	ldx #167
	dex
	Jne *-4
	plp
.elseif(n = 1510)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $A7A2'
	ldx #167
	dex
	Jne *-4
	plp
.elseif(n = 1511)
	; 8 bytes
	php
	ldx #68 ;hides 'nop zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1518)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1519)
	; 8 bytes
	php
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1521)
	; 9 bytes
	php
	ldx $A6
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1523)
	; 9 bytes
	php
	inc @zptemp
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1525)
	; 9 bytes
	php
	pha
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1526 || n = 1559)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif((n .mod 8 = 0) && ((n > 1527 && n < 1561)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 1528 && n < 1557)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1530 || n = 1540 || n = 1543 || n = 1549 || n = 1550 || n = 1557)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1531)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1532)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1533)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1535)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1537)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $AAA2'
	ldx #170
	dex
	Jne *-4
	plp
.elseif(n = 1539)
	; 9 bytes
	php
	pha
	ldx #(n-13)/7 ;hides 'nop'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1542)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 1545)
	; 8 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1546)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $ABA2'
	ldx #171
	dex
	Jne *-4
	plp
.elseif(n = 1551)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1553)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-13)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1554)
	; 8 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1558)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1561)
	; 10 bytes
	php
	pha
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563)
	; 8 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1564 || n = 1571 || n = 1577 || n = 1578 || (n > 1584 && n < 1588) || n = 1593 || n = 1595 || n = 1599)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1574 || n = 1592 || n = 1601)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1566)
	; 9 bytes
	php
	ldx #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 1567)
	; 8 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1568 || n = 1584 || n = 1600)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1598)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1570)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 1572)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $AEA2'
	ldx #174
	dex
	Jne *-4
	plp
.elseif(n = 1573)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $AEA2'
	ldx #174
	dex
	Jne *-4
	plp
.elseif(n = 1575)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1579)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	inc $F6,X
	dex
	Jmi *-4
	plp
.elseif(n = 1580)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1581)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $AFA2'
	ldx #175
	dex
	Jne *-4
	plp
.elseif(n = 1582)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $AFA2'
	ldx #175
	dex
	Jne *-4
	plp
.elseif(n = 1588 || n = 1598)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1589 || n = 1596)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1591)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1594)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 1602 to 1640
.if(n = 1602 || n = 1606 || n = 1612 || n = 1613 || n = 1615 || n = 1620 || n = 1627 || n = 1629 || n = 1634)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1603 || n = 1614)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1603 && n < 1640)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1605)
	; 9 bytes
	php
	ldx #250 ;hides 'nop'
	inc $F6,X
	dex
	Jmi *-4
	plp
.elseif(n = 1607)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1608 || n = 1624 || n = 1640)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1609)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1609 && n < 1638)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1616)
	; 7 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif(n = 1617)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 1621)
	; 9 bytes
	php
	inc @zptemp
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif(n = 1622)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1623)
	; 9 bytes
	php
	pha
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
	pla
	plp
.elseif(n = 1626)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $B4A2'
	ldx #180
	dex
	Jne *-4
	plp
.elseif(n = 1630)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
	plp
.elseif(n = 1631)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1633)
	; 9 bytes
	php
	ldx $A6
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
	plp
.elseif(n = 1635)
	; 9 bytes
	php
	inc @zptemp
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
	plp
.elseif(n = 1636)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1638)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if((n > 1640 && n < 1644) || n = 1669 || n = 1675)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1644)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1645)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B6A2'
	ldx #182
	dex
	Jne *-4
	plp
.elseif((n .mod 7 = 1) && ((n > 1645 && n < 1675)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1647)
	; 9 bytes
	php
	ldx $A6
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1648 || n = 1656 || n = 1672)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1649)
	; 9 bytes
	php
	inc @zptemp
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1650)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1651)
	; 9 bytes
	php
	pha
	ldx #(n-13)/7 ;hides 'nop'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1652 || n = 1676)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1654)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B7A2'
	ldx #183
	dex
	Jne *-4
	plp
.elseif(n = 1655 || n = 1664 || n = 1673)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1657)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1658 || n = 1668 || n = 1678)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1659)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1661)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1662)
	; 8 bytes
	php
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1663)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B8A2'
	ldx #184
	dex
	Jne *-4
	plp
.elseif(n = 1665)
	; 10 bytes
	php
	ldx $A6
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1666)
	; 9 bytes
	php
	ldx #90 ;hides 'nop'
	nop $04
	inx
	Jne *-4
	plp
.elseif(n = 1670 || n = 1677)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1671)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $B9A2'
	ldx #185
	dex
	Jne *-4
	plp
.elseif(n = 1679)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-13)/7
	nop
	dex
	Jne *-2
	plp
.endif
.else
; 1680 to 1718
.if(n = 1680 || n = 1696 || n = 1704 || n = 1712)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1680 && n < 1717)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1682 || n = 1691 || n = 1700 || n = 1718)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1683 || n = 1685 || n = 1690 || n = 1692 || n = 1693 || n = 1697 || n = 1706 || n = 1710 || n = 1711)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1705)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1686)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-13)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1687 || n = 1694 || n = 1701 || n = 1715)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
	plp
.elseif(n = 1689)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 1698 || n = 1708)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1699)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $BCA2'
	ldx #188
	dex
	Jne *-4
	plp
.elseif(n = 1703)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1707)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jne *-4
	plp
.elseif(n = 1713)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 1714)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 1717)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $BEA2'
	ldx #190
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719 || n = 1733 || n = 1740)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1720 || n = 1728 || n = 1752)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1721)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-13)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1722 || n = 1725 || n = 1729 || n = 1732 || n = 1739 || n = 1741 || n = 1746 || n = 1750 || n = 1753 || n = 1755)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1752)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1724 || n = 1748)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1726)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $BFA2'
	ldx #191
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1726 && n < 1755)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1731)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	inc $F6,X
	inx
	inx
	Jne *-5
	plp
.elseif(n = 1734)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C0A2'
	ldx #192
	dex
	Jne *-4
	plp
.elseif(n = 1735)
	; 9 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	pla
	plp
.elseif(n = 1738)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1742)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
	plp
.elseif(n = 1743)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	dex
	Jne *-4
	plp
.elseif(n = 1747)
	; 9 bytes
	php
	inc @zptemp
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
	plp
.elseif(n = 1749)
	; 9 bytes
	php
	pha
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 1756)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1757)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.endif
.else
; 1758 to 1796
.if((n .mod 7 = 1) && ((n > 1757 && n < 1794)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1759)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif((n .mod 8 = 0) && ((n > 1759 && n < 1793)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1761 || n = 1769 || n = 1774 || n = 1777 || n = 1794 || n = 1795)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1762)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C3A2'
	ldx #195
	dex
	Jne *-4
	plp
.elseif(n = 1763 || n = 1781 || n = 1790)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1764 || n = 1785)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	inc @zptemp,x
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
.elseif(n = 1767)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1770)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C4A2'
	ldx #196
	dex
	Jne *-4
	plp
.elseif(n = 1771)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C4A2'
	ldx #196
	dex
	Jne *-4
	plp
.elseif(n = 1773)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
	inc @zptemp,x
	plp
.elseif(n = 1775)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1778)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1780)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	dex
	Jne *-4
	plp
.elseif(n = 1782)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1783)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1787)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1788)
	; 8 bytes
	php
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1789)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C6A2'
	ldx #198
	dex
	Jne *-4
	plp
.elseif(n = 1791)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-13)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1796)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C7A2'
	ldx #199
	dex
	Jne *-4
	plp
.elseif(n = 1798 || n = 1818 || n = 1828)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 1798 && n < 1836)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1800)
	; 8 bytes
	php
	ldx #0
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1801 || n = 1812)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 1802 || n = 1804 || n = 1810 || n = 1819 || n = 1822 || n = 1825 || n = 1827)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1803)
	; 10 bytes
	php
	ldx $A6
	ldx #0
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1805)
	; 10 bytes
	php
	inc @zptemp
	ldx #0
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1806)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1807)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C8A2'
	ldx #200
	dex
	Jne *-4
	plp
.elseif(n = 1809)
	; 10 bytes
	php
	ldx $A6
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1811)
	; 10 bytes
	php
	inc @zptemp
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1813)
	; 10 bytes
	php
	pha
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 1814)
	; 10 bytes
	php
	ldx #128
	delay_n 9
	dex
	Jpl *-4
	plp
.elseif(n = 1815)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C9A2'
	ldx #201
	dex
	Jne *-4
	plp
.elseif(n = 1816 || n = 1824 || n = 1832)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1820)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1821)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $A5A2'
	ldx #165
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1823)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1829)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
	inc @zptemp,x
	plp
.elseif(n = 1830)
	; 10 bytes
	php
	pha
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
	pla
	plp
.elseif(n = 1831)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1833)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	plp
.elseif(n = 1834)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	dex
	Jne *-4
	plp
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 10 bytes
	php
	ldx $A6
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	plp
.elseif(n = 1837 || n = 1841 || n = 1850 || n = 1857 || n = 1863 || n = 1866 || n = 1873 || n = 1874)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1838 || n = 1858)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1839)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1840 || n = 1856 || n = 1864 || n = 1872)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1842)
	; 8 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1843)
	; 9 bytes
	php
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1843 && n < 1872)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1845)
	; 9 bytes
	php
	ldx #204 ;hides 'cpy abs'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 1846)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 1847)
	; 10 bytes
	php
	ldx #167
	inc $F6,X
	dex
	Jne *-3
	plp
	nop
.elseif(n = 1848)
	; 9 bytes
	php
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1849)
	; 10 bytes
	php
	pha
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
	plp
.elseif(n = 1851)
	; 8 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1852)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $CDA2'
	ldx #205
	dex
	Jne *-4
	plp
.elseif(n = 1854)
	; 9 bytes
	php
	ldx #205 ;hides 'cmp abs'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 1855)
	; 9 bytes
	php
	ldx #205 ;hides 'cmp abs'
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 1859 || n = 1875)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1860)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $CEA2'
	ldx #206
	dex
	Jne *-4
	plp
.elseif(n = 1861)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	dex
	Jne *-4
	plp
.elseif(n = 1865)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $A9A2'
	ldx #169
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1867)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 1868)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1869)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $CFA2'
	ldx #207
	dex
	Jne *-4
	plp
.elseif(n = 1870)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $CFA2'
	ldx #207
	dex
	Jne *-4
	plp
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
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $AAA2'
	ldx #170
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1877)
	; 9 bytes
	php
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 1878 || n = 1908)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1879)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D0A2'
	ldx #208
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1879 && n < 1908)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1881 || n = 1890 || n = 1891 || n = 1902 || n = 1910 || n = 1913)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1882)
	; 9 bytes
	php
	ldx #122 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 1883 || n = 1885 || n = 1901 || n = 1903)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1884)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 1886)
	; 9 bytes
	php
	ldx #68 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 1887)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $D1A2'
	ldx #209
	dex
	Jne *-4
	plp
.elseif((n .mod 8 = 0) && ((n > 1887 && n < 1913)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1892)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1893)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 1894)
	; 9 bytes
	php
	ldx #68 ;hides 'nop zp'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 1895)
	; 10 bytes
	php
	ldx #(n-25)/17
	jsr @nearby_rts
	dex
	Jpl *-4
	plp
.elseif(n = 1897)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D2A2'
	ldx #210
	dex
	Jne *-4
	plp
.elseif(n = 1899)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $ACA2'
	ldx #172
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1900 || n = 1911)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 1905)
	; 8 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1906)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D3A2'
	ldx #211
	dex
	Jne *-4
	plp
.elseif(n = 1909)
	; 9 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
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
.if((n >= 1915) && (n < 1934))
; 1915 to 1933
.if(n = 1915)
	; 9 bytes
	php
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	plp
.elseif(n = 1916 || n = 1925)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1917)
	; 9 bytes
	php
	ldx #44 ;hides 'bit abs'
	inc @zptemp
	inx
	Jne *-4
	plp
.elseif(n = 1918)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1919)
	; 10 bytes
	php
	inc @zptemp
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1920 || n = 1928)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1921)
	; 10 bytes
	php
	pha
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	pla
	plp
.elseif(n = 1922)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 1923)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1924)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
	dex
	Jne *-4
	plp
.elseif(n = 1926)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 1927 || n = 1930)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1929)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
	inc @zptemp,x
	plp
.elseif(n = 1931)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $AFA2'
	ldx #175
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1932)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1933)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	dex
	Jne *-4
	plp
.endif
.else
; 1934 to 1953
.if(n = 1934 || n = 1943 || n = 1952)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1935)
	; 10 bytes
	php
	ldx $A6
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1936 || n = 1944)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1937)
	; 10 bytes
	php
	inc @zptemp
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1938 || n = 1948)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1939)
	; 10 bytes
	php
	pha
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	pla
	plp
.elseif(n = 1940)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1941)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $D7A2'
	ldx #215
	dex
	Jne *-4
	plp
.elseif(n = 1942)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D7A2'
	ldx #215
	dex
	Jne *-4
	plp
.elseif(n = 1945 || n = 1946)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1947)
	; 10 bytes
	php
	ldx #194 ;hides 'nop #imm'
	nop $FF44 ;hides 'nop zp'
	dex
	Jne *-5
	plp
.elseif(n = 1949)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1950)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1951)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	dex
	Jne *-4
	plp
.elseif(n = 1953)
	; 10 bytes
	php
	ldx $A6
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
	plp
.endif
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if(n = 1954 || n = 1982)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 1955)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 1956)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
	inc @zptemp,x
	plp
.elseif(n = 1957)
	; 9 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1958 || n = 1978)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1959)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	dex
	Jne *-4
	plp
.elseif(n = 1960 || n = 1976 || n = 1984 || n = 1992)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 1960 && n < 1989)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1962 || n = 1963 || n = 1971 || n = 1972 || n = 1980 || n = 1981 || n = 1989 || n = 1990)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1964)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1965)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	nop
	dex
	Jne *-5
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
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1969)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	dex
	Jne *-4
	plp
.elseif(n = 1973)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-11)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1974)
	; 9 bytes
	php
	ldx #196 ;hides 'cpy zp'
	inc $EA,X ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 1975)
	; 9 bytes
	php
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
	plp
.elseif(n = 1977)
	; 9 bytes
	php
	ldx #197 ;hides 'cmp zp'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 1983 || n = 1985 || n = 1991)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1986)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $DCA2'
	ldx #220
	dex
	Jne *-4
	plp
.elseif(n = 1987)
	; 9 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
	plp
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 2003 || n = 2011 || n = 2027 || n = 2029)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1994 || n = 2001 || n = 2002 || n = 2007 || n = 2017 || n = 2020 || n = 2025 || n = 2026)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1995)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $DDA2'
	ldx #221
	dex
	Jne *-4
	plp
.elseif(n = 1996)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DDA2'
	ldx #221
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1996 && n < 2025)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1998 || n = 2018 || n = 2028)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 1999 || n = 2010 || n = 2021)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2000 || n = 2008 || n = 2016)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2004)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	dex
	Jne *-4
	plp
.elseif(n = 2005)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DEA2'
	ldx #222
	dex
	Jne *-4
	plp
.elseif(n = 2009)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $B6A2'
	ldx #182
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2012)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2013)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	dex
	Jne *-4
	plp
.elseif(n = 2014)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	dex
	Jne *-4
	plp
.elseif(n = 2019)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $B7A2'
	ldx #183
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2022)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $E0A2'
	ldx #224
	dex
	Jne *-4
	plp
.elseif(n = 2023)
	; 9 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2030)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2031)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	inx
	Jne *-4
	plp
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
.elseif((n .mod 9 = 8) && ((n > 2032 && n < 2070)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2034 || n = 2035 || n = 2044 || n = 2045 || n = 2053 || n = 2061 || n = 2062 || n = 2070)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2036)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2037 || n = 2046 || n = 2047 || n = 2055 || n = 2057)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2038 || n = 2058 || n = 2068)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2039)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
	plp
.elseif(n = 2041)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
	plp
.elseif(n = 2043 || n = 2054 || n = 2065)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2049)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
	plp
.elseif(n = 2050)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
	plp
.elseif(n = 2052 || n = 2066)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2056)
	; 9 bytes
	php
	ldx #0
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2059)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	dex
	Jne *-4
	plp
.elseif(n = 2063)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $BBA2'
	ldx #187
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2064)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $BBA2'
	ldx #187
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2067)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
	Jne *-4
	plp
.endif
.else
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
.if(n = 2071 || n = 2074 || n = 2079 || n = 2089)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2072 || n = 2084)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2073)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2075)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $BCA2'
	ldx #188
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2076)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2077)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inx
	Jne *-4
	plp
.elseif(n = 2078 || n = 2087)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2080)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2081)
	; 10 bytes
	php
	inc @zptemp
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2082)
	; 10 bytes
	php
	ldx #(n-25)/17
	jsr @nearby_rts
	dex
	Jpl *-4
	plp
.elseif(n = 2083)
	; 10 bytes
	php
	pha
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 2085)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 2086)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
	plp
.elseif(n = 2088)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.endif
.else
; 2090 to 2109
.if(n = 2090 || n = 2100 || n = 2106 || n = 2107)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2091)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	inc @zptemp,x
	plp
.elseif(n = 2092)
	; 10 bytes
	php
	pha
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2093)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2094)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2095)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	inx
	Jne *-4
	plp
.elseif(n = 2096)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2097)
	; 10 bytes
	php
	ldx $A6
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2098)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2099)
	; 10 bytes
	php
	inc @zptemp
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2101)
	; 10 bytes
	php
	pha
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 2102)
	; 10 bytes
	php
	ldx #116 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
	plp
.elseif(n = 2103)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $17A2'
	ldx #23
	inx
	Jne *-4
	plp
.elseif(n = 2104)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	inx
	Jne *-4
	plp
.elseif(n = 2105)
	; 9 bytes
	php
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2108)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2109)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.endif
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2111 || n = 2116 || n = 2124 || n = 2125 || n = 2135 || n = 2143 || n = 2146)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2112)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2113)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 2113 && n < 2142)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2115)
	; 10 bytes
	php
	ldx $A6
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2117)
	; 10 bytes
	php
	inc @zptemp
	ldx #(n-11)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif((n .mod 10 = 8) && ((n > 2117 && n < 2149)))
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2119)
	; 10 bytes
	php
	pha
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2120 || n = 2144)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2121)
	; 8 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2122)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	inx
	Jne *-4
	plp
.elseif(n = 2126)
	; 10 bytes
	php
	ldx #192 ;hides 'cpy #imm'
	inc $EA,X ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2127)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2129)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2130)
	; 8 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2131)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	inx
	Jne *-4
	plp
.elseif(n = 2133)
	; 9 bytes
	php
	ldx #236 ;hides 'cpx abs'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 2134)
	; 9 bytes
	php
	ldx #236 ;hides 'cpx abs'
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2136)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2137)
	; 10 bytes
	php
	pha
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
	plp
.elseif(n = 2139)
	; 8 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2140)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $EDA2'
	ldx #237
	dex
	Jne *-4
	plp
.elseif(n = 2142)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2145)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	inc @zptemp,x
	plp
.elseif(n = 2147)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.else
; 2149 to 2187
.if(n = 2149)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 2149 && n < 2187)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2151 || n = 2152 || n = 2155 || n = 2160 || n = 2161 || n = 2169 || n = 2170 || n = 2179 || n = 2182 || n = 2187)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2153 || n = 2164)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2154)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	bit $24
	inx
	inx
	Jne *-5
	plp
.elseif(n = 2156 || n = 2180)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2157)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	inx
	Jne *-4
	plp
.elseif(n = 2158 || n = 2178)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2162)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $C4A2'
	ldx #196
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2163)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2165 || n = 2171 || n = 2172 || n = 2181 || n = 2183)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2166)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2167)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	inx
	Jne *-4
	plp
.elseif(n = 2173)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $C5A2'
	ldx #197
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2174)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2175)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	dex
	Jne *-4
	plp
.elseif(n = 2176)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	inx
	Jne *-4
	plp
.elseif(n = 2184)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	plp
.elseif(n = 2185)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if((n .mod 10 = 8) && ((n > 2187 && n < 2219)))
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2189 || n = 2226)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2190 || n = 2196 || n = 2199 || n = 2200 || n = 2209 || n = 2210 || n = 2215 || n = 2223 || n = 2224)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2191)
	; 10 bytes
	php
	ldx #100 ;hides 'nop zp'
	nop
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 2192 || n = 2216)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2193)
	; 8 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 2194)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 2194 && n < 2223)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2197)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2201)
	; 10 bytes
	php
	ldx #(n-25)/17
	jsr @nearby_rts
	dex
	Jpl *-4
	plp
.elseif(n = 2202)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2203)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	inx
	Jne *-4
	plp
.elseif(n = 2205)
	; 9 bytes
	php
	ldx #12 ;hides 'nop abs'
	inc @zptemp
	inx
	Jne *-4
	plp
.elseif(n = 2206)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2207)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2211)
	; 8 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 2212)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	inx
	Jne *-4
	plp
.elseif(n = 2214)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 2217)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2219)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2220)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jne *-4
	plp
.elseif(n = 2221)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	inx
	Jne *-4
	plp
.elseif(n = 2225)
	; 10 bytes
	php
	ldx #201 ;hides 'cmp #imm'
	inc $EA,X ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.endif
.else
; 2227 to 2265
.if(n = 2227)
	; 10 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 2228 || n = 2252 || n = 2264)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2229)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	inx
	Jne *-4
	plp
.elseif(n = 2230)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $F7A2'
	ldx #247
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 2230 && n < 2259)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2232 || n = 2233 || n = 2242 || n = 2251 || n = 2254 || n = 2259 || n = 2260)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2234 || n = 2262)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2235)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2236 || n = 2237)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2238)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2239)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
	plp
.elseif(n = 2241 || n = 2263)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2243)
	; 10 bytes
	php
	inc @zptemp
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2244)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	inc @zptemp,x
	plp
.elseif(n = 2245)
	; 10 bytes
	php
	pha
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2246)
	; 10 bytes
	php
	ldx #116 ;hides 'nop zp,x'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 2247)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	inx
	Jne *-4
	plp
.elseif(n = 2248)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	inx
	Jne *-4
	plp
.elseif(n = 2250)
	; 9 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2253)
	; 10 bytes
	php
	ldx #204 ;hides 'cpy abs'
	inc @zptemp
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2255)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif(n = 2256)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2257)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	inx
	Jne *-4
	plp
.elseif(n = 2261)
	; 9 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop
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
.elseif((n .mod 9 = 8) && ((n > 2266 && n < 2304)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2268 || n = 2278 || n = 2298)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2269 || n = 2270 || n = 2279 || n = 2280 || n = 2289 || n = 2295 || n = 2299)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2271 || n = 2281 || n = 2282 || n = 2291)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2272)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2273)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
	plp
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
.elseif(n = 2283)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	inx
	Jne *-4
	plp
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
.elseif(n = 2287)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2288 || n = 2300)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2290 || n = 2304)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
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
.elseif(n = 2293)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	inx
	Jne *-4
	plp
.elseif(n = 2296)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2297)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2301)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
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
.if(n = 2305 || n = 2313 || n = 2314 || n = 2320 || n = 2331 || n = 2342)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2306)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	nop $04
	inx
	Jne *-4
	plp
.elseif(n = 2307 || n = 2329 || n = 2340)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2308 || n = 2318 || n = 2328)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2309 || n = 2322 || n = 2335)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2310)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	dex
	Jne *-4
	plp
.elseif(n = 2311)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	inx
	Jne *-4
	plp
.elseif(n = 2312)
	; 9 bytes
	php
	ldx #0
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2315 || n = 2319 || n = 2321 || n = 2323 || n = 2325 || n = 2334)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2316)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2317)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2324 || n = 2336)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2326)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	bit $24
	inx
	Jne *-4
	plp
.elseif(n = 2327)
	; 9 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2330)
	; 9 bytes
	php
	ldx #90 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 2332)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2333)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 4
	inx
	inx
	Jne *-5
	plp
.elseif(n = 2337)
	; 9 bytes
	php
	ldx #(n-5)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
	plp
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
	; 10 bytes
	php
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2341)
	; 10 bytes
	php
	ldx #44 ;hides 'bit abs'
	inc @zptemp
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2343)
	; 10 bytes
	php
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2345 || n = 2352 || n = 2355 || n = 2356 || n = 2363 || n = 2365 || n = 2367 || n = 2376 || n = 2377 || n = 2379 || n = 2381)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2346)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2347)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2348 || n = 2360 || n = 2372)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2349)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2350 || n = 2353 || n = 2364 || n = 2375 || n = 2380)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2351 || n = 2373)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2354)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2357)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inc $EA,X ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2358 || n = 2368 || n = 2378)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
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
.elseif(n = 2361)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $D6A2'
	ldx #214
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2362)
	; 9 bytes
	php
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2366)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	inc $EA,X ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2369)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 2370)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2371)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	delay_n 6
	dex
	Jne *-5
	plp
.elseif(n = 2374)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2382)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
	plp
.endif
.else
; 2383 to 2421
.if(n = 2383)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif((n .mod 12 = 8) && ((n > 2383 && n < 2421)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2385 || n = 2389 || n = 2391 || n = 2392 || n = 2399 || n = 2401 || n = 2403 || n = 2407 || n = 2409 || n = 2412 || n = 2421)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2386 || n = 2390 || n = 2397 || n = 2400 || n = 2410 || n = 2419)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2387 || n = 2413)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2388 || n = 2398 || n = 2418)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2393)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2394)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2395 || n = 2417)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2402 || n = 2416)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2404)
	; 9 bytes
	php
	ldx #(n-6)/11 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2405)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $DAA2'
	ldx #218
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2406)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	inc @zptemp
	inx
	Jne *-4
	plp
.elseif(n = 2411)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inx
	Jne *-5
	plp
.elseif(n = 2414)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	nop
	inc @zptemp
	inx
	Jne *-5
	plp
.elseif(n = 2415)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $DBA2'
	ldx #219
	nop
	dex
	Jne *-5
	plp
.endif
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2434 || n = 2440 || n = 2441 || n = 2446 || n = 2460 || n = 2463 || n = 2470 || n = 2474 || n = 2482 || n = 2485 || n = 2490 || n = 2496)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 2422 && n < 2426) || n = 2429 || n = 2431 || n = 2433 || n = 2435 || n = 2436 || n = 2443 || n = 2455 || n = 2457 || n = 2462 || n = 2467 || n = 2469 || (n > 2474 && n < 2478) || n = 2479 || n = 2484 || n = 2487 || n = 2489 || n = 2495 || n = 2497 || n = 2499)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2426)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	inx
	Jne *-5
	plp
.elseif(n = 2427)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	inx
	Jne *-5
	plp
.elseif((n .mod 10 = 8) && ((n > 2427 && n < 2459) || (n > 2477 && n < 2499)))
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2430 || n = 2500)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif((n .mod 12 = 8) && ((n > 2431 && n < 2493)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2437)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $DDA2'
	ldx #221
	nop
	dex
	Jne *-5
	plp
.elseif((n .mod 11 = 8) && ((n > 2438 && n < 2495)))
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2442)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 2445)
	; 10 bytes
	php
	ldx $A6
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 2447)
	; 10 bytes
	php
	inc @zptemp
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 2449)
	; 10 bytes
	php
	pha
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	pla
	plp
.elseif(n = 2451)
	; 10 bytes
	php
	ldx #68 ;hides 'nop zp'
	nop
	inc @zptemp
	inx
	Jne *-5
	plp
.elseif(n = 2452 || n = 2465 || n = 2491)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2453)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	inx
	Jne *-5
	plp
.elseif(n = 2454)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
	plp
.elseif(n = 2459)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2464)
	; 10 bytes
	php
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
	plp
.elseif(n = 2466)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	inx
	Jne *-5
	plp
.elseif(n = 2471)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2473)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2481)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2486)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2493)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
	plp
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
.if(n = 2501 || n = 2502 || n = 2509 || n = 2511 || n = 2513 || n = 2519 || (n > 2520 && n < 2524) || (n > 2530 && n < 2536) || n = 2539)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2503)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2504 || n = 2516 || n = 2528)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2505 || n = 2527)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2506)
	; 9 bytes
	php
	ldx #250 ;hides 'nop'
	nop $04
	dex
	Jne *-4
	plp
.elseif(n = 2507 || n = 2510 || n = 2520 || n = 2529 || n = 2530)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2508 || n = 2518 || n = 2538)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2512)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	inc @zptemp
	inx
	inx
	Jne *-5
	plp
.elseif(n = 2514)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2515)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2517)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2524)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2525)
	; 10 bytes
	php
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2526)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2536)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2537)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
	plp
.endif
.else
; 2540 to 2578
.if((n .mod 12 = 8) && ((n > 2539 && n < 2577)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2541 || n = 2575)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2542 || n = 2556 || n = 2570)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2543)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif((n > 2543 && n < 2547) || n = 2553 || n = 2557 || n = 2563 || n = 2572 || n = 2577)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2547)
	; 9 bytes
	php
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	plp
.elseif(n = 2548)
	; 9 bytes
	php
	ldx #(n-8)/10
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2549 || n = 2560 || n = 2571)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2550)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2551 || n = 2554 || n = 2566 || n = 2567 || n = 2573 || n = 2578)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2555)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	inc @zptemp
	dex
	Jne *-5
	plp
.elseif(n = 2558)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2559)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2561)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dex
	Jne *-5
	plp
.elseif(n = 2562)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2565)
	; 9 bytes
	php
	ldx #<$9D ;hides 'sta $00A2,X'
	ldx #0
	dex
	Jne *-4
	plp
.elseif(n = 2568)
	; 9 bytes
	php
	ldx #0
	inc @zptemp
	dex
	Jne *-3
	plp
.elseif(n = 2569)
	; 10 bytes
	php
	cmp #<$CD ;hides 'cmp $17A2'
	ldx #23
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2574)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579 || n = 2583 || n = 2587 || n = 2589 || n = 2594 || n = 2596 || n = 2597 || n = 2599 || n = 2607 || n = 2611 || n = 2616)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2580)
	; 9 bytes
	php
	ldx #(n-6)/11 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2581)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
	plp
.elseif(n = 2582)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 2584)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2585)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	inc $07F6,X ;hides 'inc $07,X'
	inx
	Jne *-5
	plp
.elseif(n = 2586)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	nop
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 2588 || n = 2600 || n = 2612)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2590 || n = 2614 || n = 2617)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2591)
	; 9 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2592)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $15A2'
	ldx #21
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2593 || n = 2604 || n = 2615)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2595 || n = 2608)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2598)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 2601)
	; 9 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 2602)
	; 9 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2603)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2605)
	; 10 bytes
	php
	ldx #236 ;hides 'cpx abs'
	inc @zptemp
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2606)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	inc $F6,X
	inx
	Jne *-4
	plp
.elseif(n = 2609)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2610)
	; 9 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inc $EA,X ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 2613)
	; 9 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2620 || n = 2623 || n = 2627 || (n > 2628 && n < 2634) || n = 2641 || n = 2642 || n = 2644 || n = 2649 || n = 2651 || n = 2652 || n = 2655)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2619)
	; 9 bytes
	php
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2621 || n = 2628 || n = 2638 || n = 2639 || n = 2650)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2622)
	; 9 bytes
	php
	ldx #(n-6)/12 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 2624 || n = 2636 || n = 2648)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2625)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2626 || n = 2637)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2634)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2635)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif(n = 2640 || n = 2654)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2643)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2645)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
	plp
.elseif(n = 2646)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2647)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 6
	inx
	Jne *-5
	plp
.elseif(n = 2653)
	; 10 bytes
	php
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 2656)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	inx
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if(n = 2657)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2658)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $0FA2'
	ldx #15
	nop
	inx
	Jne *-5
	plp
.elseif((n .mod 11 = 8) && ((n > 2658 && n < 2693)))
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2660 || n = 2672 || n = 2684)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2661 || n = 2662 || n = 2674 || n = 2683 || n = 2686 || n = 2694)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 2662 && n < 2666) || n = 2667 || n = 2675 || n = 2676 || n = 2678 || n = 2685 || n = 2687 || n = 2688 || n = 2695)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2666)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	inc $EA,X ;hides 'nop'
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2668 || n = 2682)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2669)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2671)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif(n = 2673)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2677)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2679)
	; 9 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2680)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2689)
	; 9 bytes
	php
	ldx #(n-5)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2690)
	; 9 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2691)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2693)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	.byte $CD,$E6,@zptemp ;cmp abs; hides 'inc @zptemp'
	dex
	Jne *-5
	plp
.endif
.else
; 2696 to 2734
.if((n .mod 12 = 8) && ((n > 2695 && n < 2733)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2697 || n = 2704 || n = 2706 || n = 2707 || n = 2709 || n = 2715 || n = 2718 || n = 2721 || n = 2726 || n = 2729 || n = 2730 || n = 2733)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2698)
	; 9 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	inc $EA,X ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2699)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2700 || n = 2705 || n = 2716 || n = 2722 || n = 2727)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2701)
	; 9 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2702)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2703 || n = 2714 || n = 2725)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2710 || n = 2724)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2711 || n = 2728)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2712)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2713)
	; 10 bytes
	php
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2717)
	; 10 bytes
	php
	ldx #226 ;hides 'nop #imm'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dex
	Jne *-5
	plp
.elseif(n = 2719)
	; 10 bytes
	php
	ldx #194 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2723)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $09A2'
	ldx #9
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2731)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2734)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
	plp
.elseif((n .mod 11 = 8) && ((n > 2735 && n < 2770)))
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2737)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 2738 || n = 2752)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2739 || n = 2740 || n = 2748 || n = 2754 || n = 2759 || n = 2763 || n = 2772)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2741)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 2742)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2743)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 6
	dex
	Jne *-5
	plp
.elseif(n = 2744 || n = 2756 || n = 2768)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2745)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2746 || n = 2749 || n = 2760 || n = 2770 || n = 2771)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2750)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inc $EA,X ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2751 || n = 2764)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2753)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 2755)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2757)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2761)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
	plp
.elseif(n = 2762)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2765)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 2766)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	inc @zptemp
	inx
	Jne *-4
	plp
.elseif(n = 2767)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2773)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.endif
.else
; 2774 to 2812
.if(n = 2774)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2775)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 6
	dex
	Jne *-5
	plp
.elseif(n = 2776 || n = 2781 || (n > 2783 && n < 2788) || n = 2795 || (n > 2796 && n < 2800) || n = 2805 || n = 2807 || n = 2809 || n = 2810)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2777 || n = 2803)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2778)
	; 9 bytes
	php
	ldx #58 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
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
.elseif(n = 2780 || n = 2792 || n = 2804)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2782 || n = 2793)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2783)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inc $EA,X ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2788)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2789)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2790)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	inc @zptemp
	inx
	Jne *-4
	plp
.elseif(n = 2791 || n = 2802)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2794 || n = 2808)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2796)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2800)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2801)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2806)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 2811)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2812)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	inx
	Jne *-5
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
; 2813 to 2890
.if(n = 2813)
	; 9 bytes
	php
	ldx #(n-8)/11
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2814)
	; 9 bytes
	php
	ldx #(n-6)/12 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 2815 || n = 2818 || n = 2826 || n = 2830 || n = 2842 || n = 2854 || n = 2866 || n = 2878 || n = 2890)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 8) && ((n > 2815 && n < 2865) || n = 2888))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2817)
	; 10 bytes
	php
	ldx #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif((n > 2818 && n < 2822) || n = 2825 || (n > 2830 && n < 2836) || n = 2839 || n = 2841 || (n > 2842 && n < 2847) || n = 2849 || n = 2851 || n = 2853 || (n > 2855 && n < 2859) || n = 2860 || n = 2861 || n = 2863 || n = 2865 || n = 2869 || (n > 2870 && n < 2876) || n = 2877 || n = 2879 || (n > 2881 && n < 2888) || n = 2889)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2822 || n = 2836 || n = 2850)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2823)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2824)
	; 9 bytes
	php
	ldx #0
	inc $F6,X
	dex
	Jne *-3
	plp
.elseif(n = 2827)
	; 10 bytes
	php
	ldx #68 ;hides 'nop zp'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 2829 || n = 2855 || n = 2868 || n = 2881)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2837)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	inx
	Jne *-5
	plp
.elseif(n = 2838)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jne *-5
	plp
.elseif(n = 2847)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2848)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	plp
.elseif(n = 2859)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inx
	Jne *-5
	plp
.elseif(n = 2862)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop
	inc @zptemp
	inx
	Jne *-5
	plp
.elseif(n = 2867)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	inc @zptemp
	inx
	Jne *-5
	plp
.elseif(n = 2870)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	plp
.elseif(n = 2876)
	; 9 bytes
	php
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2880)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
	plp
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2893 || (n > 2894 && n < 2898) || n = 2899 || n = 2901 || (n > 2902 && n < 2906) || (n > 2907 && n < 2912) || n = 2913 || (n > 2916 && n < 2920) || n = 2921 || n = 2923 || n = 2925 || (n > 2926 && n < 2932) || n = 2935 || n = 2937 || (n > 2938 && n < 2942) || (n > 2942 && n < 2946) || (n > 2950 && n < 2957) || n = 2961 || (n > 2962 && n < 2966) || n = 2967)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 14 = 8) && ((n > 2891 && n < 2935) || n = 2962))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2894 || n = 2907 || n = 2933 || n = 2946 || n = 2959)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2898 || n = 2915 || n = 2949 || n = 2966)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif((n .mod 12 = 8) && ((n > 2899 && n < 2961)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif((n .mod 12 = 10) && ((n > 2901 && n < 2951)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2916)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	cmp $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
	plp
.elseif(n = 2922)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	dex
	Jne *-5
	plp
.elseif(n = 2932)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 2942)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2947)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 2957)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2958)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	inc @zptemp
	dex
	Jne *-5
	plp
.elseif(n = 2968)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if(n = 2969 || n = 2970 || n = 2973 || n = 2975 || (n > 2977 && n < 2982) || n = 2987 || n = 2989 || (n > 2990 && n < 2996) || n = 2997 || n = 2999 || n = 3001 || n = 3003 || n = 3005 || n = 3007 || (n > 3011 && n < 3017) || n = 3019 || n = 3021 || n = 3023 || (n > 3024 && n < 3030) || n = 3031 || n = 3033 || n = 3035 || n = 3036 || (n > 3037 && n < 3044) || n = 3045)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2971)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	inc @zptemp
	dex
	Jne *-5
	plp
.elseif((n .mod 12 = 8) && ((n > 2971 && n < 3045)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2974 || n = 2986 || n = 2998 || n = 3010 || n = 3034)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 8) && ((n > 2975 && n < 3019) || n = 3046))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2977)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 2982)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 2983 || n = 3000 || n = 3017)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2985 || n = 3011 || n = 3024 || n = 3037)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 2988)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	inc @zptemp
	dex
	Jne *-5
	plp
.elseif(n = 3002)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
	plp
.elseif(n = 3006)
	; 9 bytes
	php
	ldx #(n-6)/12 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
	plp
.elseif(n = 3009)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
	plp
.elseif(n = 3022)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	inc $F6,X
	inx
	Jne *-4
	plp
.elseif(n = 3030)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
	plp
.endif
.else
; 3047 to 3125
.if(n = 3047 || n = 3049 || (n > 3051 && n < 3056) || n = 3057 || n = 3059 || n = 3064 || n = 3067 || n = 3069 || n = 3071 || n = 3075 || n = 3078 || n = 3079 || n = 3084 || (n > 3089 && n < 3101) || (n > 3102 && n < 3115) || n = 3117 || n = 3118 || (n > 3119 && n < 3126))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3048)
	; 9 bytes
	php
	ldx #(n-6)/13 ;hides 'nop'
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif((n .mod 13 = 8) && ((n > 3049 && n < 3090) || n = 3115))
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 3051)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
	plp
.elseif(n = 3056 || n = 3068)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 3058)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif((n .mod 14 = 8) && ((n > 3059 && n < 3117)))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3061)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.elseif(n = 3062)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
	plp
.elseif(n = 3065)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	inc $07F6,X ;hides 'inc $07,X'
	inx
	Jne *-5
	plp
.elseif(n = 3066)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 3070 || n = 3082)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3072)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	inx
	Jne *-5
	plp
.elseif(n = 3073)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 3077)
	; 10 bytes
	php
	ldx #<$9D ;hides 'sta $00A2,X'
	ldx #0
	nop
	dex
	Jne *-5
	plp
.elseif(n = 3080)
	; 9 bytes
	php
	ldx #0
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 3081)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 3083)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
	plp
.elseif(n = 3085 || n = 3119)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3086)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	inc $07F6,X ;hides 'inc $07,X'
	inx
	Jne *-5
	plp
.elseif(n = 3087)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 3101)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inc $F6,X
	inx
	Jne *-5
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126 || n = 3127 || n = 3129 || n = 3131 || n = 3132 || n = 3134 || n = 3137 || n = 3138 || n = 3140 || n = 3142 || n = 3143 || (n > 3144 && n < 3153) || (n > 3154 && n < 3158) || n = 3159 || (n > 3160 && n < 3165) || n = 3166 || n = 3168 || n = 3169 || n = 3173 || n = 3175 || n = 3179 || n = 3181 || n = 3183 || n = 3184 || (n > 3187 && n < 3191) || n = 3192 || n = 3194 || n = 3196 || n = 3198 || n = 3202 || n = 3203)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 13 = 8) && ((n > 3127 && n < 3194)))
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif((n .mod 14 = 8) && ((n > 3129 && n < 3201)))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3133)
	; 9 bytes
	php
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3135)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3136 || n = 3153 || n = 3170 || n = 3187)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3139)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3160)
	; 10 bytes
	php
	ldx #90 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 3165)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 3171)
	; 10 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3174)
	; 10 bytes
	php
	ldx #58 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3176)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 3177)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 3178)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	delay_n 6
	dex
	Jne *-5
	plp
.elseif(n = 3182)
	; 10 bytes
	php
	ldx #(n-2)/15 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
	plp
.elseif(n = 3185)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3191)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 3195)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
	plp
.elseif(n = 3197)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
	plp
.elseif(n = 3199)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3201)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	inc $F6,X
	dex
	Jne *-5
	plp
.endif
.else
; 3204 to 3281
.if(n = 3204 || n = 3221 || n = 3238 || n = 3255 || n = 3272)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3205 || (n > 3206 && n < 3213) || n = 3215 || n = 3217 || n = 3218 || n = 3220 || (n > 3221 && n < 3226) || n = 3227 || n = 3229 || n = 3231 || (n > 3232 && n < 3238) || n = 3239 || n = 3241 || n = 3243 || n = 3244 || (n > 3246 && n < 3254) || n = 3257 || (n > 3258 && n < 3265) || (n > 3265 && n < 3270) || (n > 3272 && n < 3276) || (n > 3276 && n < 3282))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3206)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3213)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3214 || n = 3228 || n = 3242 || n = 3270)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3216)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif((n .mod 13 = 8) && ((n > 3218 && n < 3272)))
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 3226)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 3230)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 3240)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	inc @zptemp
	dex
	dex
	Jne *-5
	plp
.elseif(n = 3246)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3254)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 3256)
	; 9 bytes
	php
	ldx #(n-6)/13 ;hides 'nop'
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 3265)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 3276)
	; 10 bytes
	php
	ldx #(n-6)/15 ;hides 'nop'
	nop
	inc $F6,X
	dex
	Jne *-5
	plp
.endif
.endif
.else
; 3282 to 3437
.if(n = 3282)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3283 || n = 3285 || n = 3287 || n = 3288 || n = 3290 || n = 3291 || (n > 3292 && n < 3296) || (n > 3298 && n < 3302) || (n > 3302 && n < 3306) || n = 3308 || n = 3309 || n = 3311 || (n > 3312 && n < 3322) || n = 3325 || (n > 3326 && n < 3336) || (n > 3336 && n < 3340) || (n > 3340 && n < 3354) || n = 3355 || n = 3356 || (n > 3357 && n < 3368) || (n > 3368 && n < 3374) || (n > 3374 && n < 3382) || (n > 3382 && n < 3390) || n = 3394 || n = 3395 || n = 3397 || (n > 3398 && n < 3408) || n = 3411 || n = 3413 || (n > 3414 && n < 3419) || n = 3420 || n = 3421 || n = 3423 || n = 3426 || n = 3428 || n = 3429 || (n > 3430 && n < 3436) || n = 3437)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 14 = 8) && ((n > 3283 && n < 3425)))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3286)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3289 || n = 3306 || n = 3357 || n = 3374 || n = 3391 || n = 3408 || n = 3425)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3292)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3296)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
	plp
.elseif(n = 3297 || n = 3310 || n = 3323)
	; 10 bytes
	php
	ldx #(n-8)/13
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 3302)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 3307)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 9
	inx
	Jne *-5
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
.elseif(n = 3324)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
	plp
.elseif(n = 3336)
	; 10 bytes
	php
	ldx #0
	nop
	inc $F6,X
	dex
	Jne *-4
	plp
.elseif(n = 3390)
	; 9 bytes
	php
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3392)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3393)
	; 10 bytes
	php
	ldx #(n-1)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3398)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3409)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
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
.elseif(n = 3414)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3419)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dex
	Jne *-5
	plp
.elseif(n = 3422)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3427)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 9
	dex
	Jne *-5
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
.elseif(n = 3436)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
; 3438 to 3593
.if((n .mod 14 = 8) && ((n > 3437 && n < 3579)))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif((n > 3438 && n < 3442) || (n > 3442 && n < 3452) || (n > 3452 && n < 3456) || n = 3458 || n = 3460 || n = 3461 || (n > 3462 && n < 3466) || (n > 3466 && n < 3471) || (n > 3471 && n < 3476) || n = 3477 || n = 3479 || (n > 3480 && n < 3486) || (n > 3486 && n < 3493) || (n > 3494 && n < 3502) || (n > 3502 && n < 3506) || n = 3507 || n = 3509 || (n > 3510 && n < 3516) || (n > 3516 && n < 3522) || (n > 3522 && n < 3527) || (n > 3527 && n < 3531) || (n > 3531 && n < 3536) || (n > 3536 && n < 3542) || n = 3543 || (n > 3546 && n < 3550) || (n > 3550 && n < 3561) || n = 3562 || n = 3563 || (n > 3564 && n < 3578) || (n > 3578 && n < 3592) || n = 3593)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3442)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3456)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	nop
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 3457)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	plp
.elseif(n = 3459 || n = 3476 || n = 3493 || n = 3510 || n = 3527 || n = 3544 || n = 3561)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3462)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3471)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	inc $F6,X
	dex
	dex
	Jne *-5
	plp
.elseif(n = 3478)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3486)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 3502)
	; 10 bytes
	php
	ldx #184 ;hides 'clv'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 3506)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3516)
	; 10 bytes
	php
	ldx #(n-6)/15 ;hides 'nop'
	nop
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3531)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 3542)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	inx
	Jne *-5
	plp
.elseif(n = 3545)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inc $07F6,X ;hides 'inc $07,X'
	inx
	Jne *-5
	plp
.elseif(n = 3546)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop
	inc $F6,X
	inx
	Jne *-5
	plp
.elseif(n = 3592)
	; 10 bytes
	php
	ldx #0
	delay_n 9
	dex
	Jne *-4
	plp
.endif
.else
; 3594 to 3750
.if(n = 3594 || (n > 3595 && n < 3612) || (n > 3612 && n < 3629) || (n > 3629 && n < 3646) || n = 3648 || n = 3650 || (n > 3651 && n < 3660) || n = 3661 || n = 3664 || n = 3665 || (n > 3666 && n < 3680) || (n > 3681 && n < 3686) || (n > 3686 && n < 3697) || (n > 3697 && n < 3702) || (n > 3702 && n < 3714) || (n > 3714 && n < 3718) || (n > 3718 && n < 3726) || (n > 3726 && n < 3731) || (n > 3731 && n < 3748) || n = 3749)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3595 || n = 3612 || n = 3629 || n = 3646 || n = 3663 || n = 3680 || n = 3697 || n = 3714 || n = 3731 || n = 3748)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3647)
	; 9 bytes
	php
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3649)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3651)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3660)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
	plp
.elseif(n = 3662)
	; 10 bytes
	php
	ldx #(n-2)/15 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	dex
	Jne *-5
	plp
.elseif(n = 3666)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	nop
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3681)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3686)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3702)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	plp
.elseif(n = 3718)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3726)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3750)
	; 10 bytes
	php
	ldx #(n-6)/16 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
; 3751 to 4375
.if((n > 3750 && n < 3756) || (n > 3756 && n < 3765) || n = 3767 || (n > 3768 && n < 3777) || (n > 3777 && n < 3782) || (n > 3782 && n < 3798) || (n > 3799 && n < 3806) || (n > 3806 && n < 3816) || (n > 3816 && n < 3833) || (n > 3833 && n < 3850) || (n > 3850 && n < 3863) || (n > 3863 && n < 3867) || (n > 3867 && n < 3884) || (n > 3884 && n < 3894) || (n > 3894 && n < 3901) || n = 3902 || n = 3903 || (n > 3906 && n < 3910) || (n > 3910 && n < 3918) || (n > 3918 && n < 3926) || (n > 3926 && n < 3935) || (n > 3935 && n < 3952) || (n > 3952 && n < 3969) || (n > 3969 && n < 3974) || (n > 3974 && n < 3986) || (n > 3986 && n < 4003) || n = 4004 || n = 4005 || (n > 4006 && n < 4020) || (n > 4020 && n < 4037) || (n > 4037 && n < 4054) || (n > 4054 && n < 4071) || (n > 4071 && n < 4088) || (n > 4088 && n < 4105) || (n > 4105 && n < 4110) || (n > 4110 && n < 4122) || (n > 4122 && n < 4139) || (n > 4139 && n < 4148) || (n > 4148 && n < 4156) || (n > 4156 && n < 4161) || n = 4162 || (n > 4163 && n < 4173) || (n > 4173 && n < 4190) || (n > 4190 && n < 4207) || (n > 4207 && n < 4224) || (n > 4224 && n < 4241) || (n > 4241 && n < 4258) || (n > 4258 && n < 4275) || (n > 4275 && n < 4292) || (n > 4292 && n < 4309) || (n > 4309 && n < 4326) || (n > 4326 && n < 4343) || (n > 4343 && n < 4360) || (n > 4360 && n < 4376))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3756)
	; 10 bytes
	php
	ldx #(n-6)/15 ;hides 'nop'
	nop
	inc $F6,X
	dex
	Jne *-5
	plp
.elseif(n = 3765 || n = 3782 || n = 3799 || n = 3816 || n = 3833 || n = 3850 || n = 3867 || n = 3884 || n = 3901 || n = 3918 || n = 3935 || n = 3952 || n = 3969 || n = 3986 || n = 4003 || n = 4020 || n = 4037 || n = 4054 || n = 4071 || n = 4088 || n = 4105 || n = 4122 || n = 4139 || n = 4156 || n = 4173 || n = 4190 || n = 4207 || n = 4224 || n = 4241 || n = 4258 || n = 4275 || n = 4292 || n = 4309 || n = 4326 || n = 4343)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3766)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3768)
	; 10 bytes
	php
	ldx #58 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 3777)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 3798)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3806)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 3863)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	plp
.elseif(n = 3894)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3904)
	; 9 bytes
	php
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3905)
	; 10 bytes
	php
	ldx #(n-1)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3906 || n = 4163)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3926)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3974)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 4006)
	; 10 bytes
	php
	ldx #(n-6)/16 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 4110)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4148)
	; 10 bytes
	php
	ldx #(n-6)/19 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4161)
	; 9 bytes
	php
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4360)
	; 10 bytes
	php
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.endif
.else
; 4376 to 5000
.if(n = 4376)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif((n > 4376 && n < 4414) || (n > 4414 && n < 4418) || n = 4419 || (n > 4420 && n < 4452) || (n > 4452 && n < 4675) || n = 4676 || (n > 4677 && n < 4718) || (n > 4718 && n < 4756) || (n > 4756 && n < 4932) || n = 4933 || (n > 4934 && n < 5001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4414)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 4418)
	; 9 bytes
	php
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4420 || n = 4677 || n = 4934)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4452 || n = 4756)
	; 10 bytes
	php
	ldx #(n-6)/19 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4675)
	; 9 bytes
	php
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4718)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4932)
	; 9 bytes
	php
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5189) || n = 5190 || (n > 5191 && n < 5446) || n = 5447 || (n > 5448 && n < 5703) || n = 5704 || (n > 5705 && n < 5960) || n = 5961 || (n > 5962 && n < 6217) || n = 6218 || (n > 6219 && n < 6474) || n = 6475 || (n > 6476 && n < 6731) || n = 6732 || (n > 6733 && n < 6988) || n = 6989 || (n > 6990 && n < 7245) || n = 7246 || (n > 7247 && n < 7502) || n = 7503 || (n > 7504 && n < 7759) || n = 7760 || (n > 7761 && n < 8016) || n = 8017 || (n > 8018 && n < 8273) || n = 8274 || (n > 8275 && n < 8530) || n = 8531 || (n > 8532 && n < 8787) || n = 8788 || (n > 8789 && n < 9044) || n = 9045 || (n > 9046 && n < 9301) || n = 9302 || (n > 9303 && n < 9558) || n = 9559 || (n > 9560 && n < 9815) || n = 9816 || (n > 9817 && n < 10001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5189)
	; 9 bytes
	php
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5191 || n = 5448 || n = 5705 || n = 5962 || n = 6219 || n = 6476 || n = 6733 || n = 6990 || n = 7247 || n = 7504 || n = 7761 || n = 8018 || n = 8275 || n = 8532 || n = 8789 || n = 9046 || n = 9303 || n = 9560 || n = 9817)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5446)
	; 9 bytes
	php
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5703)
	; 9 bytes
	php
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5960)
	; 9 bytes
	php
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6217)
	; 9 bytes
	php
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6474)
	; 9 bytes
	php
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6731)
	; 9 bytes
	php
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6988)
	; 9 bytes
	php
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7245)
	; 9 bytes
	php
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7502)
	; 9 bytes
	php
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7759)
	; 9 bytes
	php
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8016)
	; 9 bytes
	php
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8273)
	; 9 bytes
	php
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8530)
	; 9 bytes
	php
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8787)
	; 9 bytes
	php
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9044)
	; 9 bytes
	php
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9301)
	; 9 bytes
	php
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9558)
	; 9 bytes
	php
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9815)
	; 9 bytes
	php
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10072) || n = 10073 || (n > 10074 && n < 10329) || n = 10330 || (n > 10331 && n < 10586) || n = 10587 || (n > 10588 && n < 10843) || n = 10844 || (n > 10845 && n < 11100) || n = 11101 || (n > 11102 && n < 11357) || n = 11358 || (n > 11359 && n < 11614) || n = 11615 || (n > 11616 && n < 11871) || n = 11872 || (n > 11873 && n < 12128) || n = 12129 || (n > 12130 && n < 12385) || n = 12386 || (n > 12387 && n < 12642) || n = 12643 || (n > 12644 && n < 12899) || n = 12900 || (n > 12901 && n < 13156) || n = 13157 || (n > 13158 && n < 13413) || n = 13414 || (n > 13415 && n < 13670) || n = 13671 || (n > 13672 && n < 13927) || n = 13928 || (n > 13929 && n < 14184) || n = 14185 || (n > 14186 && n < 14441) || n = 14442 || (n > 14443 && n < 14698) || n = 14699 || (n > 14700 && n < 14955) || n = 14956 || (n > 14957 && n < 15001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10072)
	; 9 bytes
	php
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10074 || n = 10331 || n = 10588 || n = 10845 || n = 11102 || n = 11359 || n = 11616 || n = 11873 || n = 12130 || n = 12387 || n = 12644 || n = 12901 || n = 13158 || n = 13415 || n = 13672 || n = 13929 || n = 14186 || n = 14443 || n = 14700 || n = 14957)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10329)
	; 9 bytes
	php
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10586)
	; 9 bytes
	php
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10843)
	; 9 bytes
	php
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11100)
	; 9 bytes
	php
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11357)
	; 9 bytes
	php
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11614)
	; 9 bytes
	php
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11871)
	; 9 bytes
	php
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12128)
	; 9 bytes
	php
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12385)
	; 9 bytes
	php
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12642)
	; 9 bytes
	php
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12899)
	; 9 bytes
	php
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13156)
	; 9 bytes
	php
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13413)
	; 9 bytes
	php
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13670)
	; 9 bytes
	php
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13927)
	; 9 bytes
	php
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14184)
	; 9 bytes
	php
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14441)
	; 9 bytes
	php
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14698)
	; 9 bytes
	php
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14955)
	; 9 bytes
	php
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15212) || n = 15213 || (n > 15214 && n < 15469) || n = 15470 || (n > 15471 && n < 15726) || n = 15727 || (n > 15728 && n < 15983) || n = 15984 || (n > 15985 && n < 16240) || n = 16241 || (n > 16242 && n < 16497) || n = 16498 || (n > 16499 && n < 16754) || n = 16755 || (n > 16756 && n < 17011) || n = 17012 || (n > 17013 && n < 17268) || n = 17269 || (n > 17270 && n < 17525) || n = 17526 || (n > 17527 && n < 17782) || n = 17783 || (n > 17784 && n < 18039) || n = 18040 || (n > 18041 && n < 18296) || n = 18297 || (n > 18298 && n < 18553) || n = 18554 || (n > 18555 && n < 18810) || n = 18811 || (n > 18812 && n < 19067) || n = 19068 || (n > 19069 && n < 19324) || n = 19325 || (n > 19326 && n < 19581) || n = 19582 || (n > 19583 && n < 19838) || n = 19839 || (n > 19840 && n < 20001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15212)
	; 9 bytes
	php
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15214 || n = 15471 || n = 15728 || n = 15985 || n = 16242 || n = 16499 || n = 16756 || n = 17013 || n = 17270 || n = 17527 || n = 17784 || n = 18041 || n = 18298 || n = 18555 || n = 18812 || n = 19069 || n = 19326 || n = 19583 || n = 19840)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15469)
	; 9 bytes
	php
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15726)
	; 9 bytes
	php
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15983)
	; 9 bytes
	php
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16240)
	; 9 bytes
	php
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16497)
	; 9 bytes
	php
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16754)
	; 9 bytes
	php
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17011)
	; 9 bytes
	php
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17268)
	; 9 bytes
	php
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17525)
	; 9 bytes
	php
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17782)
	; 9 bytes
	php
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18039)
	; 9 bytes
	php
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18296)
	; 9 bytes
	php
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18553)
	; 9 bytes
	php
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18810)
	; 9 bytes
	php
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19067)
	; 9 bytes
	php
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19324)
	; 9 bytes
	php
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19581)
	; 9 bytes
	php
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19838)
	; 9 bytes
	php
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
