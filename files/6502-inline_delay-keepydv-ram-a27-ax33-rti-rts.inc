; delay_n, delays n cycles with small code
; Will clobber A, X, Z+N, and C
; Will preserve Y, S, D, V, and I (interrupt-disable flag)
;
; Requires a dummy interrupt handler that does nothing but RTI.
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
; - 2 bytes or less when n <= 5
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 18
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 44
; - 7 bytes or less when n <= 1289
; - 8 bytes or less when n <= 1345
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.6, max=4
; - When 11 <= n <= 20: min=2, avg=3.7, max=5
; - When 21 <= n <= 30: min=4, avg=5.2, max=6
; - When 31 <= n <= 50: min=4, avg=5.8, max=7
; - When 51 <= n <= 75: min=5, avg=6.1, max=7
; - When 76 <= n <= 100: min=5, avg=6.2, max=7
; - When 101 <= n <= 200: min=4, avg=6.2, max=7
; - When 201 <= n <= 300: min=5, avg=6.2, max=7
; - When 301 <= n <= 500: min=4, avg=6.2, max=7
; - When 501 <= n <= 750: min=5, avg=6.2, max=7
; - When 751 <= n <= 1000: min=5, avg=6.3, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.9, max=9
; - When 2001 <= n <= 3000: min=6, avg=8.0, max=9
; - When 3001 <= n <= 5000: min=6, avg=8.7, max=9
; - When 5001 <= n <= 7500: min=7, avg=9.0, max=9
; - When 7501 <= n <= 10000: min=8, avg=9.0, max=9
; - When 10001 <= n <= 20000: min=9, avg=9.0, max=9
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
.if((n >= 1) && (n < 20))
; 1 to 19
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif(n = 4 || n = 6 || n = 15 || n = 17)
	; 2..4 bytes
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
.elseif(n = 9 || n = 11 || n = 19)
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
	; 2 bytes
	brk
.elseif(n = 14)
	; 4 bytes
	.repeat 2
	 php
	 plp
	.endrepeat
.elseif(n = 16)
	; 4 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 18)
	; 4 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.else
; 20 to 39
.if(n = 20 || n = 22)
	; 4..5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 21)
	; 5 bytes
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
.elseif(n = 23 || n = 33)
	; 6 bytes
	clc
	delay_n (n-2)
.elseif(n = 24)
	; 4 bytes
	lda #$0A ;hides 'asl a'
	Jpl *-1
.elseif(n = 25)
	; 5 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26)
	; 4 bytes
	delay_n (n-13)
	brk
.elseif(n = 27)
	; 6 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 28 || n = 30 || n = 38)
	; 5..6 bytes
	delay_n (n-2)
	nop
.elseif(n = 29)
	; 6 bytes
	clc
	lda #$2A ;hides 'rol a'
	nop
	Jcc *-2
.elseif(n = 31)
	; 5 bytes
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
.elseif(n = 32)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 34)
	; 5 bytes
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
.elseif(n = 35)
	; 6 bytes
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
.elseif(n = 36)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 37)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 39)
	; 4 bytes
	lda #$4A ;hides 'lsr a'
	Jne *-1
.endif
.endif
.else
; 40 to 78
.if(n = 40)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	dex
	Jne *-3
.elseif(n = 41)
	; 5 bytes
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
.elseif(n = 42)
	; 6 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 43)
	; 6 bytes
	sec
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
.elseif(n = 44)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	delay_n 4
	Jpl *-3
.elseif(n = 45 || n = 47 || n = 49 || n = 54 || n = 60 || n = 62 || n = 65 || n = 67 || n = 70 || n = 72 || n = 74 || n = 75 || n = 77)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 48 || n = 58 || n = 63 || n = 68 || n = 73)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 52)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 53)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
.elseif(n = 55)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
.elseif(n = 57)
	; 6 bytes
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
.elseif(n = 59)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
.elseif(n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 69)
	; 5 bytes
	ldx #10 ;hides 'asl a'
	dex
	Jne *-2
.endif
.endif
.else
; 79 to 156
.if(n = 79 || n = 80 || n = 82 || n = 84 || n = 87 || n = 90 || n = 94 || n = 97 || n = 100 || n = 102 || n = 104 || n = 105 || n = 109 || n = 110 || n = 112 || n = 114 || n = 115 || n = 117 || n = 119 || n = 122 || n = 125 || n = 132 || n = 135 || n = 139 || n = 140 || n = 142 || n = 144 || n = 145 || n = 147 || n = 149 || n = 150 || n = 152 || n = 154)
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
.elseif(n = 89)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	nop
	dex
	Jne *-3
.elseif(n = 92 || n = 120 || n = 127 || n = 148 || n = 155)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 95)
	; 6 bytes
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
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
.elseif(n = 137)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 153)
	; 5 bytes
	ldx #234 ;hides 'nop'
	inx
	Jne *-2
.endif
.endif
.else
; 157 to 312
.if(n = 157 || n = 159 || n = 160 || n = 164 || n = 165 || n = 170 || n = 172 || n = 175 || n = 177 || n = 180 || n = 182 || n = 184 || n = 185 || n = 187 || n = 189 || n = 192 || n = 194 || n = 195 || n = 200 || n = 202 || n = 205 || n = 207 || n = 209 || n = 210 || n = 212 || n = 214 || n = 217 || n = 219 || n = 220 || n = 222 || n = 227 || n = 229 || n = 230 || n = 234 || n = 235 || n = 237 || n = 240 || n = 244 || n = 245 || n = 247 || n = 249 || n = 250 || n = 252 || n = 254 || n = 255 || n = 257 || n = 259 || n = 262 || n = 264 || n = 269 || n = 270 || n = 272 || n = 275 || n = 277 || n = 279 || n = 280 || n = 282 || n = 284 || n = 285 || n = 287 || n = 289 || n = 290)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 158 || n = 163 || n = 168 || n = 173 || n = 178 || n = 188 || n = 193 || n = 198 || n = 203 || n = 208 || n = 213 || n = 223 || n = 228 || n = 233 || n = 238 || n = 243 || n = 248 || n = 258 || n = 263 || n = 268 || n = 273 || n = 278 || n = 283 || n = 297 || n = 298 || n = 303 || n = 308)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 160 && n < 172) || n = 201 || n = 226 || n = 241 || n = 251 || n = 256 || (n > 265 && n < 282) || n = 301))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 7 = 1) && (n = 162 || n = 190 || n = 197 || (n > 252 && n < 275) || (n > 294 && n < 310)))
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
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	dex
	Jne *-3
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
.elseif(n = 242)
	; 6 bytes
	ldx #74 ;hides 'lsr a'
	inx
	inx
	Jpl *-3
.elseif(n = 265)
	; 5 bytes
	ldx #202 ;hides 'dex'
	dex
	Jmi *-2
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
.elseif(n = 293)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	dex
	Jne *-2
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
	ldx #42 ;hides 'rol a'
	dex
	Jpl *-2
.elseif(n = 304)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	nop
	dex
	Jmi *-3
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
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 318 || n = 328 || n = 333 || n = 338 || n = 343 || n = 348 || n = 353 || n = 363 || n = 367 || n = 368 || n = 373 || n = 378 || n = 388 || n = 403 || n = 408 || n = 413 || n = 418 || n = 423 || n = 433 || n = 437 || n = 438 || n = 443 || n = 448 || n = 453 || n = 458 || n = 468)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314 || n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 349 || n = 354 || n = 359 || n = 364 || n = 369 || n = 374 || n = 382 || n = 389 || n = 394 || n = 399 || n = 404 || n = 409 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459)
	; 6..7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 385 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
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
.elseif(n = 325 || n = 335 || n = 340 || n = 355 || n = 360 || n = 370 || n = 375 || n = 380 || n = 390 || n = 405 || n = 420 || n = 430 || n = 440 || n = 445 || n = 450 || n = 465)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 327 || n = 342 || n = 362 || n = 412 || n = 422 || n = 427 || n = 452)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 332)
	; 6 bytes
	ldx #74 ;hides 'lsr a'
	dex
	dex
	Jne *-3
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
	ldx #74 ;hides 'lsr a'
	inx
	Jpl *-2
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
.elseif(n = 392)
	; 6 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
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
.if(n = 469 || n = 474 || n = 479 || n = 489 || n = 499 || n = 504 || n = 509 || n = 514 || n = 527 || n = 529 || n = 534 || n = 539)
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
	ldx #197 ;hides 'cmp zp'
	nop
	inx
	Jne *-3
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
	cmp $C5
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
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	dex
	Jne *-2
.elseif(n = 524)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	dex
	Jpl *-2
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
	cmp $C5
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
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
.elseif(n = 556 || n = 561 || n = 571 || n = 576 || n = 581 || n = 596 || n = 606 || n = 611 || n = 616)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 557 || n = 572 || n = 597)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 560)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
.elseif(n = 567 || n = 602)
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
.elseif(n = 577)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 587)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 592)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	dex
	Jpl *-4
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
; 626 to 781
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 627 || n = 632 || n = 642 || n = 662 || n = 672 || n = 677 || n = 692 || n = 702 || n = 707 || n = 732 || n = 742 || n = 747 || n = 762 || n = 767 || n = 772)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 628 || n = 633 || n = 643 || n = 648 || n = 653 || n = 658 || n = 663 || n = 668 || n = 678 || n = 682 || n = 683 || n = 693 || n = 698 || n = 703 || n = 713 || n = 717 || n = 718 || n = 723 || n = 728 || n = 733 || n = 738 || n = 753 || n = 758 || n = 763 || n = 768 || n = 773)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
	plp
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
.elseif((n .mod 5 = 4) && ((n > 633 && n < 655) || n = 664 || n = 669 || (n > 678 && n < 690) || (n > 698 && n < 725) || (n > 738 && n < 760) || (n > 768 && n < 780)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 5 = 0) && (n = 635 || n = 655 || n = 660 || n = 670 || n = 675 || (n > 684 && n < 711) || (n > 724 && n < 746) || (n > 759 && n < 781)))
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
.elseif((n .mod 7 = 1) && (n = 638 || n = 645 || (n > 672 && n < 695) || (n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
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
.elseif(n = 665)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 667 || n = 712)
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
.elseif(n = 697)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.else
; 782 to 937
.if(n = 782)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.elseif(n = 783 || n = 787 || n = 788 || n = 793 || n = 798 || n = 803 || n = 808 || n = 818 || n = 822 || n = 823 || n = 828 || n = 833 || n = 838 || n = 843 || n = 853 || n = 857 || n = 858 || n = 863 || n = 868 || n = 873 || n = 878 || n = 888 || n = 893 || n = 898 || n = 903 || n = 908 || n = 913 || n = 923 || n = 927 || n = 928 || n = 933)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 795) || (n > 803 && n < 830) || (n > 838 && n < 865) || (n > 873 && n < 895) || (n > 908 && n < 935)))
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
.elseif((n .mod 5 = 0) && ((n > 789 && n < 816) || (n > 829 && n < 851) || (n > 859 && n < 886) || (n > 894 && n < 921) || n = 930))
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
.elseif(n = 802 || n = 847 || n = 937)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 5 = 2) && (n = 807 || n = 812 || n = 837 || n = 842 || n = 852 || n = 867 || n = 872 || n = 882 || n = 887 || (n > 901 && n < 923)))
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.if(n = 938 || n = 943 || n = 948 || n = 958 || n = 963 || n = 968 || n = 973 || n = 978 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1003 || n = 1008 || n = 1013 || n = 1028 || n = 1032 || n = 1033 || n = 1038 || n = 1043 || n = 1048 || n = 1053 || n = 1067 || n = 1068 || n = 1073 || n = 1078 || n = 1083 || n = 1088 || n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1113 || n = 1118 || n = 1123 || n = 1133 || n = 1137 || n = 1138 || n = 1143 || n = 1148 || n = 1153 || n = 1158 || n = 1168 || n = 1172 || n = 1173 || n = 1178 || n = 1183 || n = 1188 || n = 1193 || n = 1203 || n = 1208 || n = 1213 || n = 1218 || n = 1223 || n = 1228 || n = 1238 || n = 1242 || n = 1243 || n = 1248)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || (n > 1092 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 0) && ((n > 939 && n < 956) || (n > 969 && n < 991) || (n > 999 && n < 1016) || n = 1025 || (n > 1034 && n < 1061) || (n > 1069 && n < 1096) || (n > 1104 && n < 1131) || (n > 1139 && n < 1166) || (n > 1174 && n < 1201) || (n > 1209 && n < 1236) || n = 1245 || n = 1250))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942 || n = 947 || n = 952 || n = 957 || n = 987 || n = 1012 || n = 1020 || n = 1022 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1082 || n = 1087 || n = 1092 || n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1152 || n = 1157 || n = 1167 || n = 1182 || n = 1192 || n = 1197 || n = 1202 || n = 1222 || n = 1227 || n = 1232)
	; 6..7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1089 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1124 || n = 1129 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1209 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244 || n = 1249)
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
	cmp $C5
	dex
	Jpl *-3
.elseif(n = 982 || n = 1162)
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
.elseif(n = 1027)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 1049)
	; 5 bytes
	ldx #106 ;hides 'ror a'
	inx
	Jne *-2
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
.elseif(n = 1063)
	; 5 bytes
	ldx #152 ;hides 'tya'
	dex
	Jne *-2
.elseif(n = 1097 || n = 1217)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
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
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	inx
	Jpl *-4
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
.elseif(n = 1253 || n = 1258 || n = 1263 || n = 1277 || n = 1278 || n = 1283)
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
	cmp $C5
	dex
	Jne *-3
.elseif((n .mod 5 = 4) && ((n > 1258 && n < 1285)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1287)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1273)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	inx
	Jne *-2
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
.elseif(n = 1288)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 1290 to 1328
.if(n = 1290 || n = 1291 || n = 1298 || n = 1305 || n = 1308 || n = 1314 || n = 1319 || n = 1322 || n = 1326)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1292 || n = 1299 || n = 1327)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1293 || n = 1300 || n = 1307)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1294)
	; 7 bytes
	php
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1295)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $70A2'
	ldx #112
	inx
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
.elseif(n = 1301 || n = 1311)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1302 || n = 1318)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1304)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $91A2'
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
	cmp $C5
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
	cmp <$CD ;hides 'cmp $6DA2'
	ldx #109
	inx
	Jne *-4
.elseif(n = 1325)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	nop
	inx
	inx
	Jne *-5
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
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1330 || n = 1339 || n = 1340 || n = 1344 || n = 1347 || n = 1354 || n = 1361)
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
	cmp <$CD ;hides 'cmp $6CA2'
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
.elseif(n = 1341)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1343)
	; 8 bytes
	ldx #121
	delay_n 6
	dex
	Jpl *-4
.elseif(n = 1346)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1349)
	; 6 bytes
	ldx #106 ;hides 'ror a'
	nop
	inx
	Jne *-3
.elseif(n = 1350)
	; 7 bytes
	cmp <$CD ;hides 'cmp $96A2'
	ldx #150
	dex
	Jne *-4
.elseif(n = 1357)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1358)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $97A2'
	ldx #151
	dex
	Jne *-4
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
	; 6 bytes
	ldx #152 ;hides 'tya'
	nop
	dex
	Jne *-3
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1375 || n = 1382 || n = 1388 || n = 1389)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1369 || n = 1378 || n = 1396 || n = 1405)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1370 || n = 1384 || n = 1398)
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
	; 8 bytes
	pha
	delay_n (n-7)
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
	cmp $C5
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
.elseif(n = 1383 || n = 1406)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1386)
	; 7 bytes
	cmp <$CD ;hides 'cmp $66A2'
	ldx #102
	inx
	Jne *-4
.elseif(n = 1390 || n = 1397 || n = 1402)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1392 || n = 1404)
	; 7..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1395)
	; 7 bytes
	cmp <$CD ;hides 'cmp $9BA2'
	ldx #155
	dex
	Jne *-4
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
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $9DA2'
	ldx #157
	dex
	Jne *-4
.elseif(n = 1413)
	; 7 bytes
	cmp <$CD ;hides 'cmp $9DA2'
	ldx #157
	dex
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
	cmp #<$CD ;hides 'cmp $61A2'
	ldx #97
	inx
	Jne *-4
.elseif(n = 1437 || n = 1444)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1439)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $A0A2'
	ldx #160
	dex
	Jne *-4
.elseif(n = 1440)
	; 7 bytes
	cmp <$CD ;hides 'cmp $60A2'
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
	cmp #<$CD ;hides 'cmp $A1A2'
	ldx #161
	dex
	Jne *-4
.elseif(n = 1449 || n = 1481)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
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
.elseif(n = 1452 || n = 1463 || n = 1466 || n = 1470 || n = 1473 || n = 1480)
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
	cmp <$CD ;hides 'cmp $A2A2'
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
.elseif(n = 1467)
	; 7 bytes
	cmp <$CD ;hides 'cmp $A3A2'
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
	cmp #<$CD ;hides 'cmp $A4A2'
	ldx #164
	dex
	Jne *-4
.elseif(n = 1476)
	; 7 bytes
	cmp <$CD ;hides 'cmp $A4A2'
	ldx #164
	dex
	Jne *-4
.elseif(n = 1479)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	brk
	dex
	Jne *-4
.elseif(n = 1483)
	; 7 bytes
	ldx #106 ;hides 'ror a'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 1484)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $A5A2'
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
.elseif(n = 1487 || n = 1490 || n = 1494 || n = 1498 || n = 1501 || n = 1508 || n = 1514 || n = 1515)
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
	; 7 bytes
	cmp #<$CD ;hides 'cmp $A6A2'
	ldx #166
	dex
	Jne *-4
.elseif(n = 1496)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	nop
	inx
	Jne *-3
.elseif(n = 1497)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	inx
	Jne *-2
.elseif(n = 1500 || n = 1509 || n = 1516 || n = 1523)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1502)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1503)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1504)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
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
.elseif(n = 1510)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1512)
	; 7 bytes
	cmp <$CD ;hides 'cmp $A8A2'
	ldx #168
	dex
	Jne *-4
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
.if(n = 1524 || n = 1552)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1525 || n = 1537 || n = 1561)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
	; 8 bytes
	cmp #<$CD ;hides 'cmp $8BA2'
	ldx #139
	nop
	dex
	Jne *-5
.elseif(n = 1529)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1530)
	; 7 bytes
	cmp <$CD ;hides 'cmp $AAA2'
	ldx #170
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1530 && n < 1559)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1532 || n = 1544)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1533 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1557 || n = 1560)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1538)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1539)
	; 7 bytes
	cmp <$CD ;hides 'cmp $ABA2'
	ldx #171
	dex
	Jne *-4
.elseif(n = 1545 || n = 1553)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
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
.elseif(n = 1554)
	; 8 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1556)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $ADA2'
	ldx #173
	dex
	Jne *-4
.elseif(n = 1559)
	; 7 bytes
	ldx #173 ;hides 'lda abs'
	inc @zptemp
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1564 || n = 1570 || n = 1571 || n = 1578 || n = 1586 || n = 1592 || n = 1595 || n = 1599)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $AEA2'
	ldx #174
	dex
	Jne *-4
.elseif(n = 1566)
	; 7 bytes
	cmp <$CD ;hides 'cmp $AEA2'
	ldx #174
	dex
	Jne *-4
.elseif(n = 1567 || n = 1585 || n = 1594)
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
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1598)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1572)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1573)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1574)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $AFA2'
	ldx #175
	dex
	Jne *-4
.elseif(n = 1575)
	; 7 bytes
	cmp <$CD ;hides 'cmp $AFA2'
	ldx #175
	dex
	Jne *-4
.elseif(n = 1577 || n = 1593 || n = 1601)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1579 || n = 1600)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1582 || n = 1589)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1584)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1588)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1598)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	delay_n 4
	inx
	inx
	Jne *-5
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 7 bytes
	cmp <$CD ;hides 'cmp $B2A2'
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
.elseif(n = 1607 || n = 1626 || n = 1635 || n = 1640)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1608 || n = 1615 || n = 1622)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
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
.elseif(n = 1614)
	; 8 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 1616)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1617)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
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
.elseif(n = 1624 || n = 1629)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
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
.elseif(n = 1630)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1631)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1636)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1638)
	; 7 bytes
	cmp <$CD ;hides 'cmp $B6A2'
	ldx #182
	dex
	Jne *-4
.endif
.endif
.else
; 1641 to 1718
.if(n = 1641)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1642 || n = 1658 || n = 1672 || n = 1679 || n = 1714)
	; 7..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1643 || n = 1680 || n = 1687 || n = 1694 || n = 1708)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1644)
	; 7 bytes
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
	ldx #73 ;hides 'eor #imm'
	lda $EA ;hides 'nop'
	inx
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
	cmp $C5
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
.elseif(n = 1652 || n = 1655 || n = 1659 || n = 1662 || n = 1668 || n = 1669 || n = 1676 || n = 1683 || n = 1686 || n = 1690 || n = 1697 || n = 1703 || n = 1704 || n = 1707 || n = 1715 || n = 1718)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1654 || n = 1696)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1656)
	; 7 bytes
	cmp <$CD ;hides 'cmp $B8A2'
	ldx #184
	dex
	Jne *-4
.elseif(n = 1661 || n = 1691 || n = 1701)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1663 || n = 1670 || n = 1677 || n = 1698 || n = 1712)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1664)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $B9A2'
	ldx #185
	dex
	Jne *-4
.elseif(n = 1671)
	; 7 bytes
	ldx #152 ;hides 'tya'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1678 || n = 1685 || n = 1699 || n = 1706)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1692)
	; 7 bytes
	cmp <$CD ;hides 'cmp $BCA2'
	ldx #188
	dex
	Jne *-4
.elseif(n = 1700)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $BDA2'
	ldx #189
	dex
	Jne *-4
.elseif(n = 1710)
	; 7 bytes
	cmp <$CD ;hides 'cmp $BEA2'
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
	cmp <$CD ;hides 'cmp $BFA2'
	ldx #191
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1719 && n < 1757)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1721)
	; 5 bytes
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
.elseif(n = 1722 || n = 1725 || n = 1732 || n = 1739 || n = 1743 || n = 1746 || n = 1749 || n = 1753 || n = 1754)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1752)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724 || n = 1733 || n = 1740)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1726 || n = 1735)
	; 7..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1727 || n = 1757)
	; 7..8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1728)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1731)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1734)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1736)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $C1A2'
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
.elseif(n = 1745)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1748)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1750)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1752)
	; 6 bytes
	ldx #37 ;hides 'and zp'
	nop
	inx
	Jne *-3
.elseif(n = 1755)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C3A2'
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
.elseif(n = 1759)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1760)
	; 6 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1761 || n = 1769 || n = 1785)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1762 || n = 1767 || n = 1781 || n = 1787 || n = 1788 || n = 1795)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1763)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $C4A2'
	ldx #196
	dex
	Jne *-4
.elseif(n = 1764)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C4A2'
	ldx #196
	dex
	Jne *-4
.elseif(n = 1766 || n = 1778)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1768 || n = 1796)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1770 || n = 1784)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1771 || n = 1791)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1773)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C5A2'
	ldx #197
	dex
	Jne *-4
.elseif(n = 1774 || n = 1783 || n = 1792)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
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
.elseif(n = 1782)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C6A2'
	ldx #198
	dex
	Jne *-4
.elseif(n = 1790)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $C7A2'
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
	tya
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	tay
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
	cmp <$CD ;hides 'cmp $C8A2'
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
	cmp #<$CD ;hides 'cmp $C9A2'
	ldx #201
	dex
	Jne *-4
.elseif(n = 1809)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $A5A2'
	ldx #165
	nop
	dex
	Jne *-5
.endif
.else
; 1816 to 1835
.if(n = 1816)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1817)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1818)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CAA2'
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
	cmp <$CD ;hides 'cmp $CBA2'
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
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1834)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1835)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
.endif
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 7 bytes
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1874)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1838)
	; 7 bytes
	ldx #204 ;hides 'cpy abs'
	inc @zptemp
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
.elseif(n = 1840)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1841 || n = 1857 || n = 1865)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1842)
	; 8 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1843 || n = 1848 || n = 1856 || n = 1859 || n = 1866 || n = 1867 || n = 1874 || n = 1875)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1844)
	; 7 bytes
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	dex
	Jne *-4
.elseif(n = 1845)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CDA2'
	ldx #205
	dex
	Jne *-4
.elseif(n = 1847)
	; 7 bytes
	ldx #205 ;hides 'cmp abs'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 1849 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1850 || n = 1852 || n = 1868)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1851 || n = 1871)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1853)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $CEA2'
	ldx #206
	dex
	Jne *-4
.elseif(n = 1854)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CEA2'
	ldx #206
	dex
	Jne *-4
.elseif(n = 1858)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $A9A2'
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
.elseif(n = 1862)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $CFA2'
	ldx #207
	dex
	Jne *-4
.elseif(n = 1863)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CFA2'
	ldx #207
	dex
	Jne *-4
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
.elseif(n = 1872)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D0A2'
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
.if(n = 1876 || n = 1878 || n = 1886 || (n > 1893 && n < 1897) || n = 1906)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1877)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
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
	cmp $C5
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1910)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1883 || n = 1884 || n = 1887 || n = 1902 || n = 1903)
	; 8 bytes
	delay_n (n-2)
	nop
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
	cmp <$CD ;hides 'cmp $D2A2'
	ldx #210
	dex
	Jne *-4
.elseif(n = 1892)
	; 8 bytes
	cmp <$CD ;hides 'cmp $ACA2'
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
.elseif(n = 1898)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $D3A2'
	ldx #211
	dex
	Jne *-4
.elseif(n = 1899)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D3A2'
	ldx #211
	dex
	Jne *-4
.elseif(n = 1901 || n = 1911)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1907)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1908)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D4A2'
	ldx #212
	dex
	Jne *-4
.elseif(n = 1910)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1912)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1914)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 1915) && (n < 1934))
; 1915 to 1933
.if(n = 1915)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1916)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1917)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D5A2'
	ldx #213
	dex
	Jne *-4
.elseif(n = 1918 || n = 1927)
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
.elseif(n = 1920)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1921 || n = 1933)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1922)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1923 || n = 1932)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1924)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $AFA2'
	ldx #175
	nop
	dex
	Jne *-5
.elseif(n = 1925)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1926)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D6A2'
	ldx #214
	dex
	Jne *-4
.elseif(n = 1928)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1929)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1930)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1931)
	; 7 bytes
	ldx #138 ;hides 'txa'
	pla
	pha
	dex
	Jne *-4
.endif
.else
; 1934 to 1953
.if(n = 1934)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $D7A2'
	ldx #215
	dex
	Jne *-4
.elseif(n = 1935)
	; 7 bytes
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1936 || n = 1945)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1937 || n = 1953)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1938 || n = 1939 || n = 1946 || n = 1947)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1940 || n = 1942)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1941 || n = 1951)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1943)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $D8A2'
	ldx #216
	dex
	Jne *-4
.elseif(n = 1944)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D8A2'
	ldx #216
	dex
	Jne *-4
.elseif(n = 1948)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1949)
	; 8 bytes
	ldx #106 ;hides 'ror a'
	delay_n 6
	inx
	Jne *-5
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
	cmp #<$CD ;hides 'cmp $D9A2'
	ldx #217
	dex
	Jne *-4
.endif
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
	cmp <$CD ;hides 'cmp $B2A2'
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
.elseif(n = 1960)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1961 || n = 1977 || n = 1985)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1962)
	; 7 bytes
	cmp <$CD ;hides 'cmp $DAA2'
	ldx #218
	dex
	Jne *-4
.elseif(n = 1966 || n = 1976 || n = 1984 || n = 1986)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1967)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	pla
	pha
	inx
	inx
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
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1988)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $DDA2'
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
.elseif(n = 1994 || n = 1996 || n = 2030)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1995 || n = 2002 || n = 2004 || n = 2010 || n = 2012 || (n > 2017 && n < 2021) || n = 2027 || n = 2028)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1997)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $DEA2'
	ldx #222
	dex
	Jne *-4
.elseif(n = 1998)
	; 7 bytes
	cmp <$CD ;hides 'cmp $DEA2'
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
	cmp $C5
	dex
	Jne *-3
.elseif(n = 2003 || n = 2014)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2006)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $DFA2'
	ldx #223
	dex
	Jne *-4
.elseif(n = 2007)
	; 7 bytes
	cmp <$CD ;hides 'cmp $DFA2'
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
	cmp #<$CD ;hides 'cmp $E0A2'
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
	; 8 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2024)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $E1A2'
	ldx #225
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
	cmp <$CD ;hides 'cmp $E2A2'
	ldx #226
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
.elseif(n = 2037 || n = 2046 || n = 2054 || n = 2055 || n = 2064 || n = 2067)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2038 || n = 2040 || n = 2048 || n = 2066)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2039)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	lda $A5
	inx
	Jne *-5
.elseif(n = 2041 || n = 2065)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2042)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $E3A2'
	ldx #227
	dex
	Jne *-4
.elseif(n = 2043)
	; 7 bytes
	cmp <$CD ;hides 'cmp $1DA2'
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
	cmp $C5
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
	cmp <$CD ;hides 'cmp $E4A2'
	ldx #228
	dex
	Jne *-4
.elseif(n = 2056)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $BBA2'
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
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
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
	; 7 bytes
	cmp #<$CD ;hides 'cmp $E6A2'
	ldx #230
	dex
	Jne *-4
.elseif(n = 2070)
	; 7 bytes
	cmp <$CD ;hides 'cmp $E6A2'
	ldx #230
	dex
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
.elseif(n = 2072 || n = 2073 || n = 2083 || n = 2093 || n = 2100 || n = 2103)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074 || n = 2076 || n = 2086 || n = 2095 || n = 2104)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2075)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
	cmp <$CD ;hides 'cmp $E7A2'
	ldx #231
	dex
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
.elseif(n = 2085 || n = 2094)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2087)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
.elseif(n = 2088)
	; 7 bytes
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	inx
	Jne *-4
.elseif(n = 2090 || n = 2108)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2092)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2096)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $17A2'
	ldx #23
	inx
	Jne *-4
.elseif(n = 2097)
	; 7 bytes
	cmp <$CD ;hides 'cmp $17A2'
	ldx #23
	inx
	Jne *-4
.elseif(n = 2099)
	; 7 bytes
	ldx #106 ;hides 'ror a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 2102)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2105)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2106)
	; 7 bytes
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
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
.if(n = 2110 || n = 2128)
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
	cmp <$CD ;hides 'cmp $EBA2'
	ldx #235
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2115 && n < 2144)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2117)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2118 || n = 2136 || n = 2144 || n = 2145)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2119)
	; 7 bytes
	ldx #106 ;hides 'ror a'
	brk
	dex
	Jne *-4
.elseif(n = 2120)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2122)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $C1A2'
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
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	inx
	Jne *-4
.elseif(n = 2126)
	; 7 bytes
	ldx #236 ;hides 'cpx abs'
	inc @zptemp
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
	; 7 bytes
	cmp #<$CD ;hides 'cmp $EDA2'
	ldx #237
	dex
	Jne *-4
.elseif(n = 2133)
	; 7 bytes
	cmp <$CD ;hides 'cmp $EDA2'
	ldx #237
	dex
	Jne *-4
.elseif(n = 2135 || n = 2146)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2138 || n = 2140 || n = 2148)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2139)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	lda $A5
	inx
	Jne *-4
.elseif(n = 2142)
	; 7 bytes
	cmp <$CD ;hides 'cmp $12A2'
	ldx #18
	inx
	Jne *-4
.elseif(n = 2147)
	; 8 bytes
	ldx #232 ;hides 'inx'
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $EFA2'
	ldx #239
	dex
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
	cmp #<$CD ;hides 'cmp $C4A2'
	ldx #196
	nop
	dex
	Jne *-5
.elseif(n = 2156)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	cmp $C5
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
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 2159)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	inx
	Jne *-4
.elseif(n = 2160)
	; 7 bytes
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	inx
	Jne *-4
.elseif(n = 2164 || n = 2165 || n = 2174 || n = 2176 || n = 2184)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2166)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $C5C5 ;hides 'cmp zp'
	dex
	Jne *-5
.elseif(n = 2167)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2168)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $F1A2'
	ldx #241
	dex
	Jne *-4
.elseif(n = 2169)
	; 7 bytes
	cmp <$CD ;hides 'cmp $0FA2'
	ldx #15
	inx
	Jne *-4
.elseif(n = 2177)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $0EA2'
	ldx #14
	inx
	Jne *-4
.elseif(n = 2178)
	; 7 bytes
	cmp <$CD ;hides 'cmp $F2A2'
	ldx #242
	dex
	Jne *-4
.elseif(n = 2180)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2183)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2186)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $F3A2'
	ldx #243
	dex
	Jne *-4
.elseif(n = 2187)
	; 7 bytes
	cmp <$CD ;hides 'cmp $F3A2'
	ldx #243
	dex
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
	ldx #37 ;hides 'and zp'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2191 || n = 2201 || n = 2211)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2192 || n = 2193 || n = 2198 || n = 2203 || n = 2208 || n = 2217 || n = 2225 || n = 2226)
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
	; 7 bytes
	cmp #<$CD ;hides 'cmp $F4A2'
	ldx #244
	dex
	Jne *-4
.elseif(n = 2196)
	; 7 bytes
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
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
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2202)
	; 8 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 2204)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $0BA2'
	ldx #11
	inx
	Jne *-4
.elseif(n = 2205)
	; 7 bytes
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	inx
	Jne *-4
.elseif(n = 2207)
	; 8 bytes
	ldx #138 ;hides 'txa'
	nop
	pla
	pha
	dex
	Jne *-5
.elseif(n = 2209 || n = 2221)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2210)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $C9A2'
	ldx #201
	nop
	dex
	Jne *-5
.elseif(n = 2212)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2213)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
.elseif(n = 2214)
	; 7 bytes
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	inx
	Jne *-4
.elseif(n = 2216)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2218)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2219)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 2220)
	; 8 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2222)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $F7A2'
	ldx #247
	dex
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
.elseif(n = 2229 || n = 2230 || n = 2236 || n = 2237 || n = 2239 || n = 2248 || n = 2264)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2231)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	inx
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
.elseif(n = 2235 || n = 2244 || n = 2247 || n = 2252 || n = 2253 || n = 2262 || n = 2263)
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
	ldx #204 ;hides 'cpy abs'
	inc @zptemp
	nop
	dex
	Jne *-5
.elseif(n = 2249)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2250)
	; 7 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2254)
	; 8 bytes
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	nop
	dex
	Jne *-5
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
.elseif(n = 2261)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2265)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $CEA2'
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
	cmp <$CD ;hides 'cmp $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
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
.elseif(n = 2272 || n = 2274 || n = 2275 || n = 2282 || n = 2284 || n = 2290 || n = 2292 || n = 2302)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
	; 8 bytes
	cmp <$CD ;hides 'cmp $D1A2'
	ldx #209
	nop
	dex
	Jne *-5
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
.elseif(n = 2306 || n = 2307 || n = 2313 || n = 2343)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2312 || (n > 2313 && n < 2317) || n = 2318 || n = 2324 || n = 2327 || n = 2328 || (n > 2333 && n < 2339) || n = 2340)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2309)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
.elseif(n = 2310)
	; 8 bytes
	cmp <$CD ;hides 'cmp $D2A2'
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
	; 8 bytes
	cmp #<$CD ;hides 'cmp $D3A2'
	ldx #211
	nop
	dex
	Jne *-5
.elseif(n = 2322 || n = 2333)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2323)
	; 7 bytes
	ldx #128
	brk
	dex
	Jpl *-3
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
.elseif(n = 2331)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2332)
	; 8 bytes
	cmp <$CD ;hides 'cmp $D4A2'
	ldx #212
	nop
	dex
	Jne *-5
.elseif(n = 2339)
	; 7 bytes
	ldx #234 ;hides 'nop'
	cmp $C5
	dex
	Jne *-4
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
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2355 || n = 2366)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2345 || n = 2346 || n = 2348 || n = 2349 || (n > 2355 && n < 2359) || n = 2360 || n = 2368 || n = 2369 || n = 2372 || n = 2374 || n = 2378 || n = 2380 || n = 2382)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
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
	cmp <$CD ;hides 'cmp $D6A2'
	ldx #214
	nop
	dex
	Jne *-5
.elseif(n = 2359)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2362)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
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
.elseif(n = 2367)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2370)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jpl *-5
.elseif(n = 2373 || n = 2379)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2375)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $D8A2'
	ldx #216
	nop
	dex
	Jne *-5
.elseif(n = 2376)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2393 || n = 2403 || n = 2415)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2385 || n = 2392 || n = 2394 || n = 2396 || n = 2400 || n = 2402 || (n > 2403 && n < 2407) || n = 2412 || n = 2414 || n = 2416 || n = 2417)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2386)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
.elseif(n = 2387)
	; 8 bytes
	cmp <$CD ;hides 'cmp $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
.elseif(n = 2388 || n = 2410)
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
.elseif(n = 2390)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2391 || n = 2411 || n = 2421)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2395)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2397)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $DAA2'
	ldx #218
	nop
	dex
	Jne *-5
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
.elseif(n = 2407)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2408)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $DBA2'
	ldx #219
	nop
	dex
	Jne *-5
.elseif(n = 2409)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
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
	cmp #<$CD ;hides 'cmp $DCA2'
	ldx #220
	nop
	dex
	Jne *-5
.elseif(n = 2420)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	lda $A5
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2424 || n = 2426 || n = 2429 || n = 2436 || n = 2445 || n = 2446 || n = 2450 || (n > 2454 && n < 2459))
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
.elseif(n = 2427 || n = 2433 || n = 2453)
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
	cmp #<$CD ;hides 'cmp $DDA2'
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 2447)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 2448)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 2452)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
.elseif(n = 2459)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	lda $A5
	inx
	Jne *-4
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
.elseif(n = 2462 || n = 2468 || n = 2472 || n = 2477 || n = 2478 || n = 2480 || n = 2482 || n = 2484 || n = 2488 || n = 2489 || n = 2492 || n = 2494 || n = 2495)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2463 || n = 2475 || n = 2483 || n = 2499)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2464)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E0A2'
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
.elseif(n = 2466)
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
.elseif(n = 2469)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
	tay
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
	cmp #<$CD ;hides 'cmp $E1A2'
	ldx #225
	nop
	dex
	Jne *-5
.elseif(n = 2479 || n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2486)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E2A2'
	ldx #226
	nop
	dex
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
	cmp #<$CD ;hides 'cmp $E3A2'
	ldx #227
	nop
	dex
	Jne *-5
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
.elseif(n = 2502 || n = 2512 || n = 2513 || n = 2522 || n = 2523)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2503 || n = 2539)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2504 || n = 2506 || (n > 2513 && n < 2517) || (n > 2523 && n < 2529) || n = 2532 || (n > 2535 && n < 2539))
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
	cmp <$CD ;hides 'cmp $E4A2'
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
.elseif(n = 2517 || n = 2534)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2518)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	nop
	inx
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
.elseif(n = 2529)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $E6A2'
	ldx #230
	nop
	dex
	Jne *-5
.elseif(n = 2530)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E6A2'
	ldx #230
	nop
	dex
	Jne *-5
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
.elseif(n = 2544 || n = 2546 || n = 2556 || n = 2560 || n = 2565 || n = 2566 || n = 2570 || n = 2576)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2545 || n = 2557 || n = 2569)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2547)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 2548)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	inc @zptemp
	dex
	Jne *-5
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
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	nop
	inx
	Jne *-5
.elseif(n = 2554)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dex
	Jne *-5
.elseif(n = 2555)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	cmp $C5
	dex
	Jne *-5
.elseif(n = 2558)
	; 8 bytes
	ldx #73 ;hides 'eor #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 2559 || n = 2571)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2561)
	; 7 bytes
	ldx #0
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2562)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $17A2'
	ldx #23
	nop
	inx
	Jne *-5
.elseif(n = 2567)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2568)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2572)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jpl *-5
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
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 2575)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
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
.if(n = 2579 || n = 2580 || n = 2582 || (n > 2586 && n < 2591) || n = 2592 || n = 2594 || n = 2600 || n = 2603 || n = 2609 || n = 2610 || n = 2612 || n = 2614 || n = 2615)
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
.elseif(n = 2585 || n = 2607 || n = 2613)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2586 || n = 2597 || n = 2608)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2591)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
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
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	nop
	inx
	Jne *-5
.elseif(n = 2598)
	; 8 bytes
	ldx #236 ;hides 'cpx abs'
	inc @zptemp
	nop
	dex
	Jne *-5
.elseif(n = 2599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-5
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
.elseif(n = 2604)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	brk
	inx
	Jpl *-5
.elseif(n = 2606)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $EDA2'
	ldx #237
	nop
	dex
	Jne *-5
.elseif(n = 2611)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
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
	cmp <$CD ;hides 'cmp $12A2'
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
.elseif(n = 2620 || (n > 2621 && n < 2625) || n = 2632 || n = 2634 || n = 2635 || n = 2642 || (n > 2643 && n < 2647) || n = 2648 || n = 2654 || n = 2656)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2621)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts
	dex
	Jne *-5
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
	ldx #37 ;hides 'and zp'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2629 || n = 2641 || n = 2653)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2631 || n = 2643 || n = 2649 || n = 2655)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2633)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
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
	delay_n 8
	Jne *-5
.elseif(n = 2639)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	nop
	inx
	Jne *-5
.elseif(n = 2640)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 2647)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2650)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $F1A2'
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
; 2657 to 2734
.if((n > 2656 && n < 2661) || n = 2664 || n = 2666 || n = 2668 || n = 2669 || n = 2671 || n = 2676 || n = 2678 || n = 2680 || n = 2682 || n = 2686 || n = 2688 || n = 2690 || n = 2692 || n = 2693 || (n > 2696 && n < 2701) || n = 2702 || (n > 2707 && n < 2713) || n = 2714 || n = 2720 || (n > 2721 && n < 2725) || n = 2726 || (n > 2731 && n < 2735))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2661 || n = 2675 || n = 2703 || n = 2717 || n = 2731)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2662)
	; 8 bytes
	cmp <$CD ;hides 'cmp $F2A2'
	ldx #242
	nop
	dex
	Jne *-5
.elseif((n .mod 11 = 1) && ((n > 2662 && n < 2730)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 2670 || n = 2687 || n = 2704 || n = 2721)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2672)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $F3A2'
	ldx #243
	nop
	dex
	Jne *-5
.elseif(n = 2673)
	; 8 bytes
	cmp <$CD ;hides 'cmp $F3A2'
	ldx #243
	nop
	dex
	Jne *-5
.elseif(n = 2681)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2683 || n = 2719)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2684)
	; 8 bytes
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	nop
	inx
	Jne *-5
.elseif(n = 2694)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $0BA2'
	ldx #11
	nop
	inx
	Jne *-5
.elseif(n = 2695)
	; 8 bytes
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	nop
	inx
	Jne *-5
.elseif(n = 2705)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2706)
	; 8 bytes
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
.elseif(n = 2716)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $F7A2'
	ldx #247
	nop
	dex
	Jne *-5
.elseif(n = 2728)
	; 8 bytes
	cmp <$CD ;hides 'cmp $F8A2'
	ldx #248
	nop
	dex
	Jne *-5
.elseif(n = 2730)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
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
.elseif(n = 2736 || (n > 2741 && n < 2745) || (n > 2745 && n < 2749) || (n > 2751 && n < 2755) || n = 2756 || n = 2758 || n = 2764 || n = 2765 || n = 2769 || n = 2770)
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
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	nop
	inx
	Jne *-5
.elseif(n = 2739 || n = 2757 || n = 2763)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2740 || n = 2751 || n = 2762)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2741)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2745)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2750)
	; 8 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	nop
	inx
	Jne *-5
.elseif(n = 2755)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2759)
	; 7 bytes
	ldx #138 ;hides 'txa'
	brk
	dex
	Jne *-4
.elseif(n = 2760)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $05A2'
	ldx #5
	nop
	inx
	Jne *-5
.elseif(n = 2766)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	dex
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
	; 8 bytes
	ldx #232 ;hides 'inx'
	cmp $C5
	dex
	dex
	Jne *-5
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
.if(n = 2774 || (n > 2775 && n < 2781) || n = 2786 || n = 2788 || n = 2792 || n = 2796 || n = 2798 || n = 2802 || n = 2803 || n = 2808 || n = 2812)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
	; 7 bytes
	ldx #24 ;hides 'clc'
	inc @zptemp
	inx
	Jne *-4
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
.elseif(n = 2790)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 2791)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
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
.elseif(n = 2800)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
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
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2810)
	; 8 bytes
	ldx #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
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
.if(n = 2813 || n = 2814 || n = 2822 || (n > 2823 && n < 2827) || n = 2828 || n = 2830 || n = 2832 || n = 2836 || n = 2837 || n = 2839 || n = 2842 || n = 2846 || n = 2848 || (n > 2849 && n < 2855) || n = 2856 || n = 2858 || n = 2862 || n = 2864 || (n > 2865 && n < 2869) || n = 2870 || n = 2872 || n = 2873 || (n > 2874 && n < 2879) || n = 2880 || n = 2882 || n = 2884 || n = 2886 || n = 2888 || n = 2889)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif((n .mod 12 = 1) && ((n > 2820 && n < 2882)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2823 || n = 2835 || n = 2847 || n = 2859 || n = 2865 || n = 2883)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2827 || n = 2863)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
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
.elseif(n = 2840 || n = 2874)
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
.elseif(n = 2849)
	; 8 bytes
	ldx #106 ;hides 'ror a'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 2855)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2860)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
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
.elseif(n = 2879)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	pla
	pha
	dex
	Jne *-5
.elseif(n = 2887)
	; 8 bytes
	ldx #152 ;hides 'tya'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 2890)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2908 || n = 2925 || n = 2942 || n = 2959)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2892 || n = 2894 || (n > 2895 && n < 2899) || n = 2900 || (n > 2901 && n < 2905) || n = 2906 || n = 2909 || n = 2910 || n = 2912 || (n > 2913 && n < 2917) || n = 2918 || n = 2920 || (n > 2921 && n < 2925) || n = 2926 || n = 2928 || n = 2930 || (n > 2931 && n < 2935) || n = 2936 || n = 2938 || n = 2939 || (n > 2943 && n < 2950) || n = 2952 || n = 2954 || (n > 2955 && n < 2959) || n = 2960 || n = 2963 || n = 2964 || n = 2966 || n = 2968)
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
.elseif(n = 2895 || n = 2907 || n = 2919 || n = 2931 || n = 2937 || n = 2943 || n = 2967)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2899 || n = 2935)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2901 || n = 2921 || n = 2961)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
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
.elseif(n = 2913 || n = 2927 || n = 2955)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2940)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 2950)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2951)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2962)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n .mod 14 = 1) && ((n > 2968 && n < 3012)))
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
.elseif(n = 2971 || n = 3007 || n = 3043)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2972 || n = 2974 || n = 2975 || n = 2980 || n = 2982 || (n > 2983 && n < 2989) || n = 2990 || n = 2992 || n = 2994 || n = 2996 || n = 2998 || n = 3004 || n = 3005 || n = 3008 || n = 3014 || (n > 3015 && n < 3021) || n = 3022 || n = 3023 || n = 3026 || (n > 3027 && n < 3035) || n = 3036 || n = 3038 || n = 3040 || n = 3042 || n = 3046)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2973 || n = 2979 || n = 2991 || n = 3003 || n = 3009 || n = 3027 || n = 3045)
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2978)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	.byte $CD,$E6,@zptemp ;cmp abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 2981)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 2995)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 2999)
	; 7 bytes
	ldx #106 ;hides 'ror a'
	brk
	inx
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
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3015)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3021)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3024)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 3035)
	; 8 bytes
	ldx #138 ;hides 'txa'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 3039)
	; 7 bytes
	ldx #152 ;hides 'tya'
	brk
	dex
	Jne *-4
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
.if(n = 3047 || n = 3048 || n = 3050 || (n > 3054 && n < 3061) || n = 3062 || (n > 3063 && n < 3067) || (n > 3067 && n < 3073) || n = 3074 || n = 3076 || n = 3077 || n = 3080 || (n > 3081 && n < 3095) || n = 3096 || n = 3098 || n = 3100 || (n > 3101 && n < 3109) || n = 3110 || n = 3111 || n = 3113 || n = 3114 || n = 3116 || (n > 3117 && n < 3121) || n = 3122 || n = 3124 || n = 3125)
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
.elseif(n = 3051 || n = 3063 || n = 3075 || n = 3099 || n = 3117)
	; 8 bytes
	delay_n (n-2)
	nop
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
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3078 || n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3079 || n = 3097 || n = 3115)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3101 || n = 3121)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if((n > 3125 && n < 3129) || (n > 3129 && n < 3133) || n = 3134 || n = 3136 || (n > 3137 && n < 3141) || (n > 3141 && n < 3146) || (n > 3146 && n < 3151) || n = 3152 || (n > 3153 && n < 3161) || n = 3162 || n = 3164 || (n > 3165 && n < 3169) || n = 3170 || (n > 3171 && n < 3179) || (n > 3181 && n < 3187) || n = 3188 || (n > 3189 && n < 3193) || (n > 3193 && n < 3197) || n = 3198 || n = 3200 || (n > 3201 && n < 3205) || n = 3206 || (n > 3207 && n < 3214) || (n > 3214 && n < 3221) || n = 3222 || n = 3224 || (n > 3225 && n < 3231) || n = 3232 || n = 3234 || (n > 3235 && n < 3241) || (n > 3243 && n < 3247) || (n > 3250 && n < 3259) || n = 3262 || n = 3264 || (n > 3265 && n < 3270) || (n > 3270 && n < 3275) || n = 3278)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3129 || n = 3146 || n = 3163 || n = 3180 || n = 3214 || n = 3248 || n = 3265)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3133 || n = 3151 || n = 3169 || n = 3187 || n = 3205 || n = 3223 || n = 3241 || n = 3259 || n = 3277)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3135 || n = 3153 || n = 3171 || n = 3189 || n = 3225 || n = 3243 || n = 3279)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 1) && (n = 3137 || (n > 3164 && n < 3264)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3141 || n = 3161 || n = 3181 || n = 3201 || n = 3261 || n = 3281)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3197)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
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
.elseif(n = 3250)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3260)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
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
.elseif(n = 3280)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.endif
.else
; 3282 to 3437
.if(n = 3282 || n = 3299 || n = 3316 || n = 3350 || n = 3384 || n = 3401 || n = 3418 || n = 3435)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283 || n = 3284 || n = 3287 || n = 3288 || n = 3290 || (n > 3291 && n < 3295) || n = 3296 || n = 3298 || n = 3302 || n = 3304 || (n > 3305 && n < 3313) || n = 3314 || n = 3317 || n = 3318 || n = 3320 || (n > 3321 && n < 3331) || n = 3332 || (n > 3333 && n < 3341) || n = 3342 || (n > 3343 && n < 3347) || n = 3348 || (n > 3351 && n < 3361) || (n > 3361 && n < 3365) || n = 3366 || n = 3368 || (n > 3370 && n < 3375) || (n > 3375 && n < 3381) || n = 3382 || n = 3383 || n = 3386 || n = 3388 || n = 3390 || (n > 3391 && n < 3401) || n = 3404 || n = 3406 || (n > 3407 && n < 3417) || n = 3419 || n = 3420 || n = 3422 || (n > 3423 && n < 3431) || (n > 3431 && n < 3435) || n = 3436 || n = 3437)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3285)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3286)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 3289)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	inc @zptemp
	inx
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 3290 && n < 3390) || n = 3417 || n = 3431))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3295 || n = 3313 || n = 3331 || n = 3349 || n = 3367 || n = 3385 || n = 3403 || n = 3421)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3297 || n = 3315 || n = 3351 || n = 3369 || n = 3387 || n = 3405)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3300)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3301 || n = 3321 || n = 3341 || n = 3381)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3303)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3343)
	; 8 bytes
	ldx #152 ;hides 'tya'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 3365)
	; 8 bytes
	ldx #202 ;hides 'dex'
	brk
	dex
	dex
	Jne *-5
.elseif(n = 3370)
	; 8 bytes
	ldx #169 ;hides 'lda #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3391)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3402)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3407)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
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
.endif
.endif
.else
; 3438 to 3750
.if(n = 3438 || n = 3440 || n = 3442 || (n > 3445 && n < 3450) || n = 3451 || n = 3453 || n = 3455 || n = 3456 || n = 3458 || n = 3460 || (n > 3461 && n < 3465) || (n > 3465 && n < 3469) || (n > 3469 && n < 3473) || n = 3474 || n = 3476 || (n > 3477 && n < 3481) || (n > 3481 && n < 3486) || (n > 3487 && n < 3493) || n = 3494 || (n > 3495 && n < 3501) || n = 3502 || (n > 3503 && n < 3511) || n = 3512 || n = 3514 || (n > 3515 && n < 3520) || (n > 3521 && n < 3529) || n = 3530 || (n > 3531 && n < 3537) || (n > 3537 && n < 3541) || n = 3542 || (n > 3543 && n < 3547) || n = 3548 || (n > 3549 && n < 3554) || n = 3555 || n = 3556 || (n > 3557 && n < 3561) || (n > 3561 && n < 3565) || n = 3566 || (n > 3567 && n < 3571) || (n > 3571 && n < 3581) || n = 3582 || n = 3584 || n = 3586 || n = 3587 || (n > 3588 && n < 3601) || n = 3602 || n = 3604 || (n > 3605 && n < 3619) || n = 3620 || (n > 3622 && n < 3637) || n = 3638 || n = 3640 || (n > 3641 && n < 3650) || n = 3651 || n = 3652 || n = 3654 || (n > 3661 && n < 3666) || (n > 3666 && n < 3673) || n = 3674 || (n > 3675 && n < 3681) || (n > 3681 && n < 3690) || n = 3692 || n = 3694 || (n > 3695 && n < 3701) || (n > 3701 && n < 3707) || n = 3708 || n = 3710 || (n > 3711 && n < 3721) || n = 3722 || n = 3723 || n = 3725 || n = 3726 || n = 3728 || (n > 3729 && n < 3739) || n = 3740 || n = 3742 || n = 3744 || n = 3746 || n = 3748 || n = 3749)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3439 || n = 3457 || n = 3475 || n = 3493 || n = 3511 || n = 3529 || n = 3547 || n = 3565 || n = 3583 || n = 3601 || n = 3619 || n = 3637 || n = 3655 || n = 3673 || n = 3691 || n = 3709 || n = 3727 || n = 3745)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3441 || n = 3461 || n = 3481 || n = 3521 || n = 3541 || n = 3561 || n = 3581 || n = 3621 || n = 3641 || n = 3661 || n = 3681 || n = 3701 || n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3443)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 3444)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 3444 && n < 3516) || (n > 3542 && n < 3572)))
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
.elseif(n = 3452 || n = 3469 || n = 3486 || n = 3503 || n = 3520 || n = 3537 || n = 3554 || n = 3588 || n = 3605 || n = 3622 || n = 3656 || n = 3690 || n = 3707 || n = 3724 || n = 3741)
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
.elseif(n = 3465)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 3477 || n = 3495 || n = 3513 || n = 3531 || n = 3549 || n = 3567 || n = 3603 || n = 3675 || n = 3693 || n = 3729 || n = 3747)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3639)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	brk
	inx
	Jne *-4
.elseif(n = 3650)
	; 8 bytes
	ldx #73 ;hides 'eor #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3653)
	; 6 bytes
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
.elseif(n = 3657)
	; 8 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 3658)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3659)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 3660)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3666)
	; 8 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3739)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3743)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
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
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
; 3751 to 4062
.if((n > 3750 && n < 3754) || (n > 3754 && n < 3758) || n = 3762 || n = 3764 || (n > 3765 && n < 3775) || (n > 3775 && n < 3780) || n = 3782 || (n > 3783 && n < 3792) || (n > 3792 && n < 3799) || n = 3800 || (n > 3801 && n < 3809) || (n > 3809 && n < 3817) || n = 3818 || n = 3820 || (n > 3821 && n < 3826) || (n > 3826 && n < 3835) || n = 3836 || (n > 3837 && n < 3841) || n = 3842 || (n > 3843 && n < 3853) || n = 3854 || (n > 3856 && n < 3860) || (n > 3861 && n < 3871) || n = 3872 || (n > 3873 && n < 3877) || (n > 3877 && n < 3881) || (n > 3881 && n < 3889) || n = 3890 || n = 3892 || n = 3893 || (n > 3894 && n < 3901) || (n > 3901 && n < 3906) || n = 3908 || n = 3910 || (n > 3911 && n < 3916) || (n > 3916 && n < 3921) || (n > 3921 && n < 3925) || n = 3926 || (n > 3928 && n < 3935) || n = 3936 || (n > 3937 && n < 3941) || n = 3942 || n = 3944 || n = 3946 || n = 3947 || (n > 3948 && n < 3959) || n = 3960 || (n > 3963 && n < 3970) || (n > 3970 && n < 3979) || n = 3980 || (n > 3981 && n < 3996) || n = 3998 || n = 4000 || n = 4002 || (n > 4003 && n < 4010) || n = 4011 || n = 4012 || n = 4014 || n = 4016 || (n > 4017 && n < 4021) || (n > 4021 && n < 4030) || n = 4031 || n = 4032 || n = 4034 || (n > 4035 && n < 4041) || (n > 4041 && n < 4047) || (n > 4047 && n < 4051) || n = 4052 || (n > 4053 && n < 4059) || n = 4060 || n = 4062)
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
.elseif(n = 3758 || n = 3792 || n = 3809 || n = 3826 || n = 3843 || n = 3860 || n = 3877 || n = 3894 || n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3996 || n = 4013 || n = 4030 || n = 4047)
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
.elseif(n = 3761 || n = 3801 || n = 3821 || n = 3841 || n = 3861 || n = 3881 || n = 3901 || n = 3921 || n = 3941 || n = 3981 || n = 4001 || n = 4021 || n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3763 || n = 3781 || n = 3799 || n = 3817 || n = 3835 || n = 3853 || n = 3871 || n = 3889 || n = 3907 || n = 3925 || n = 3943 || n = 3961 || n = 3979 || n = 3997 || n = 4015 || n = 4033 || n = 4051)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3765)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	pla
	pha
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
.elseif(n = 3783 || n = 3819 || n = 3837 || n = 3855 || n = 3873 || n = 3891 || n = 3909 || n = 3927 || n = 3963 || n = 4017 || n = 4035 || n = 4053)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3856)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
.elseif(n = 3906)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3916)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3935)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3937)
	; 8 bytes
	ldx #232 ;hides 'inx'
	brk
	inx
	inx
	Jne *-5
.elseif(n = 3948)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3959)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 3970)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3999)
	; 7 bytes
	ldx #56 ;hides 'sec'
	brk
	inx
	Jne *-4
.elseif(n = 4003)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4010)
	; 8 bytes
	ldx #201 ;hides 'cmp #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4059)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	brk
	Jne *-4
.endif
.else
; 4063 to 4375
.if(n = 4063 || (n > 4065 && n < 4069) || n = 4070 || (n > 4071 && n < 4081) || (n > 4081 && n < 4087) || n = 4088 || (n > 4089 && n < 4098) || n = 4099 || n = 4100 || (n > 4101 && n < 4105) || n = 4106 || (n > 4107 && n < 4115) || (n > 4116 && n < 4121) || n = 4122 || n = 4124 || (n > 4126 && n < 4132) || (n > 4132 && n < 4137) || (n > 4137 && n < 4141) || n = 4142 || (n > 4143 && n < 4149) || (n > 4149 && n < 4158) || n = 4160 || (n > 4161 && n < 4166) || (n > 4166 && n < 4177) || n = 4178 || n = 4180 || n = 4182 || (n > 4183 && n < 4195) || n = 4196 || n = 4198 || n = 4199 || (n > 4201 && n < 4213) || n = 4214 || n = 4216 || (n > 4217 && n < 4221) || (n > 4221 && n < 4231) || n = 4232 || (n > 4234 && n < 4241) || (n > 4241 && n < 4249) || n = 4250 || (n > 4251 && n < 4261) || (n > 4262 && n < 4267) || (n > 4269 && n < 4279) || n = 4280 || (n > 4281 && n < 4285) || n = 4286 || n = 4288 || n = 4289 || (n > 4290 && n < 4301) || n = 4304 || (n > 4305 && n < 4319) || n = 4320 || n = 4322 || (n > 4323 && n < 4336) || n = 4337 || n = 4338 || n = 4340 || (n > 4341 && n < 4353) || (n > 4353 && n < 4357) || n = 4358 || n = 4360 || (n > 4361 && n < 4375))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132 || n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217 || n = 4234 || n = 4251 || n = 4268 || n = 4302 || n = 4319 || n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4065)
	; 8 bytes
	ldx #42 ;hides 'rol a'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4069 || n = 4087 || n = 4105 || n = 4123 || n = 4141 || n = 4159 || n = 4177 || n = 4195 || n = 4213 || n = 4231 || n = 4249 || n = 4267 || n = 4285 || n = 4303 || n = 4321 || n = 4339 || n = 4357 || n = 4375)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4071 || n = 4089 || n = 4107 || n = 4125 || n = 4143 || n = 4179 || n = 4197 || n = 4215 || n = 4233 || n = 4269 || n = 4287 || n = 4305 || n = 4323 || n = 4359)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4101 || n = 4121 || n = 4161 || n = 4181 || n = 4201 || n = 4221 || n = 4241 || n = 4261 || n = 4281 || n = 4301 || n = 4341 || n = 4361)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4116)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4126)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4137)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4158)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4262)
	; 8 bytes
	ldx #202 ;hides 'dex'
	sta @zptemp
	brk
	Jne *-5
.elseif(n = 4279)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	brk
	inx
	Jne *-4
.elseif(n = 4290)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4353)
	; 8 bytes
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || (n > 4377 && n < 4381) || (n > 4381 && n < 4393) || n = 4394 || (n > 4395 && n < 4399) || n = 4400 || (n > 4401 && n < 4407) || (n > 4407 && n < 4411) || n = 4412 || (n > 4413 && n < 4421) || (n > 4421 && n < 4429) || n = 4430 || (n > 4431 && n < 4441) || n = 4442 || n = 4444 || n = 4446 || n = 4448 || (n > 4449 && n < 4454) || (n > 4454 && n < 4461) || (n > 4461 && n < 4465) || n = 4466 || (n > 4467 && n < 4481) || n = 4482 || n = 4484 || n = 4486 || (n > 4487 && n < 4501) || n = 4502 || (n > 4503 && n < 4519) || n = 4520 || (n > 4521 && n < 4532))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4377 || n = 4395 || n = 4413 || n = 4431 || n = 4449 || n = 4467 || n = 4485 || n = 4503)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4381 || n = 4401 || n = 4421 || n = 4441 || n = 4461 || n = 4481 || n = 4521)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4393 || n = 4411 || n = 4429 || n = 4447 || n = 4465 || n = 4483 || n = 4501 || n = 4519)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4407)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4443)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4445)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4454)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4487)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.endif
.else
; 4532 to 4687
.if((n > 4531 && n < 4537) || n = 4538 || n = 4540 || (n > 4541 && n < 4555) || n = 4556 || (n > 4557 && n < 4561) || (n > 4561 && n < 4573) || n = 4574 || n = 4576 || n = 4577 || n = 4579 || n = 4589 || n = 4590 || n = 4592 || (n > 4593 && n < 4598) || (n > 4605 && n < 4609) || n = 4610 || (n > 4611 && n < 4620) || (n > 4621 && n < 4639) || n = 4640 || (n > 4641 && n < 4661) || n = 4662 || (n > 4663 && n < 4673) || (n > 4674 && n < 4679) || n = 4680 || (n > 4681 && n < 4685) || n = 4686 || n = 4687)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4537 || n = 4555 || n = 4573 || n = 4591)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4539 || n = 4557 || n = 4575 || n = 4582 || n = 4593 || n = 4600 || n = 4611)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4541 || n = 4561 || n = 4581 || n = 4601 || n = 4621 || n = 4641 || n = 4661 || n = 4681)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4578)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4580)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 4583)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4584)
	; 8 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 4585 || n = 4603)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4586 || n = 4604)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4587 || n = 4605)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4588)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4598)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
.elseif(n = 4599)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4602)
	; 8 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 4609)
	; 7 bytes
	ldx #0
	brk
	dex
	Jne *-3
.elseif(n = 4620)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4639)
	; 7 bytes
	ldx #24 ;hides 'clc'
	brk
	inx
	Jne *-4
.elseif(n = 4663)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4673)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4674)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4679)
	; 7 bytes
	ldx #234 ;hides 'nop'
	brk
	dex
	Jne *-4
.elseif(n = 4685)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	brk
	dex
	Jne *-5
.endif
.endif
.else
; 4688 to 5000
.if((n > 4687 && n < 4701) || (n > 4701 && n < 4707) || (n > 4707 && n < 4721) || (n > 4721 && n < 4741) || (n > 4741 && n < 4761) || (n > 4761 && n < 4781) || (n > 4781 && n < 4801) || (n > 4801 && n < 4821) || (n > 4821 && n < 4841) || (n > 4841 && n < 4861) || (n > 4861 && n < 4881) || (n > 4881 && n < 4901) || (n > 4901 && n < 4919) || n = 4920 || (n > 4921 && n < 4930) || (n > 4930 && n < 4941) || (n > 4941 && n < 4961) || (n > 4961 && n < 4981) || (n > 4981 && n < 5001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4701 || n = 4721 || n = 4741 || n = 4761 || n = 4781 || n = 4801 || n = 4821 || n = 4841 || n = 4861 || n = 4881 || n = 4901 || n = 4921 || n = 4941 || n = 4961 || n = 4981)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4707)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4919)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	brk
	inx
	Jne *-4
.elseif(n = 4930)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
.if((n >= 5001) && (n < 5626))
; 5001 to 5625
.if(n = 5001 || n = 5021 || n = 5041 || n = 5061 || n = 5081 || n = 5101)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif((n > 5001 && n < 5021) || (n > 5021 && n < 5041) || (n > 5041 && n < 5061) || (n > 5061 && n < 5074) || (n > 5074 && n < 5081) || (n > 5081 && n < 5090) || n = 5091 || (n > 5092 && n < 5101) || n = 5102 || (n > 5103 && n < 5109) || (n > 5112 && n < 5121) || (n > 5121 && n < 5147) || (n > 5147 && n < 5158) || (n > 5158 && n < 5169) || (n > 5169 && n < 5191) || (n > 5191 && n < 5250) || (n > 5250 && n < 5260) || (n > 5260 && n < 5271) || (n > 5271 && n < 5292) || (n > 5292 && n < 5345) || (n > 5345 && n < 5365) || (n > 5366 && n < 5411) || (n > 5411 && n < 5600) || (n > 5600 && n < 5620) || (n > 5622 && n < 5626))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5074)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
.elseif(n = 5090)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 5092 || n = 5111 || n = 5112)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5103)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5109)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 5110)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 5121)
	; 8 bytes
	ldx #0
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5147)
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5158)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 5169)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5191)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5250)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5260)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 5271)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5292)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5345)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	cmp $C5
	dex
	Jne *-5
.elseif(n = 5365)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $C5C5 ;hides 'cmp zp'
	dex
	Jne *-5
.elseif(n = 5366)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	cmp $C5
	dex
	Jne *-5
.elseif(n = 5411)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5600)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 5620)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
.elseif(n = 5621)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 5622)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 6
	dex
	Jne *-5
.endif
.else
; 5626 to 6250
.if((n > 5625 && n < 5855) || (n > 5855 && n < 5875) || n = 5876 || n = 5877 || (n > 5878 && n < 6251))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5855)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 5875)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	.byte $CD,$E6,@zptemp ;cmp abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 5878)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	inc @zptemp
	dex
	Jne *-5
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6292) || (n > 6292 && n < 6365) || (n > 6365 && n < 6385) || (n > 6385 && n < 6390) || (n > 6390 && n < 7501))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6292)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
.elseif(n = 6365)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 6385)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 6390)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 9
	dex
	Jne *-5
.endif
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7895) || (n > 7895 && n < 7915) || (n > 7915 && n < 7926) || (n > 7926 && n < 10001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7895)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 7915)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 7926)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	brk
	dex
	Jne *-5
.endif
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif

.endif
	rest
.endmacro
