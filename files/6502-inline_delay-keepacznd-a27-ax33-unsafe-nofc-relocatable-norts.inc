; delay_n, delays n cycles with small code
; Will clobber X, Y, and V
; Will preserve A, S, Z+N, C, D, and I (interrupt-disable flag)
;
; WARNING: If an interrupt happens in the middle of this code, data corruption can occur.
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
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 11
; - 5 bytes or less when n <= 16
; - 6 bytes or less when n <= 19
; - 7 bytes or less when n <= 23
; - 8 bytes or less when n <= 26
; - 9 bytes or less when n <= 38
; -10 bytes or less when n <= 1308
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=0, avg=2.4, max=4
; - When 11 <= n <= 20: min=4, avg=5.3, max=7
; - When 21 <= n <= 30: min=6, avg=7.6, max=9
; - When 31 <= n <= 50: min=7, avg=8.2, max=10
; - When 51 <= n <= 75: min=6, avg=7.9, max=9
; - When 76 <= n <= 100: min=7, avg=8.2, max=9
; - When 101 <= n <= 200: min=6, avg=8.2, max=9
; - When 201 <= n <= 300: min=6, avg=8.2, max=10
; - When 301 <= n <= 500: min=6, avg=8.2, max=10
; - When 501 <= n <= 750: min=6, avg=8.3, max=10
; - When 751 <= n <= 1000: min=7, avg=8.4, max=10
; - When 1001 <= n <= 2000: min=6, avg=9.1, max=11
; - When 2001 <= n <= 3000: min=8, avg=10.3, max=11
; - When 3001 <= n <= 5000: min=9, avg=11.0, max=11
; - When 5001 <= n <= 7500: min=11, avg=11.0, max=11
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
.if((n >= 1) && (n < 20))
; 1 to 19
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
.error .sprintf("delay_n: INTERNAL ERROR: UNABLE TO DEVISE DELAY CODE FOR %d CYCLES", n)
.elseif(n = 4 || n = 6 || n = 12)
	; 2..5 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 3 bytes
	clv
	Jvc *+2
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
.elseif(n = 14)
	; 4 bytes
	.repeat n/7
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
.elseif(n = 19)
	; 6 bytes
	php
	rol $36,X
	ror $36,X
	plp
.endif
.else
; 20 to 39
.if(n = 20)
	; 7 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 21)
	; 6 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 22)
	; 7 bytes
	php
	tsx
	delay_n 7
	plp
	txs
	plp
.elseif(n = 23)
	; 7 bytes
	nop
	delay_n (n-2)
.elseif(n = 24)
	; 8 bytes
	php
	pha
	rol $26
	ror $26
	pla
	plp
.elseif(n = 25)
	; 8 bytes
	delay_n 4
	delay_n (n-4)
.elseif(n = 26)
	; 8 bytes
	php
	pha
	rol $36,X
	ror $36,X
	pla
	plp
.elseif(n = 27)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 30 || n = 35 || n = 37)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 31)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
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
	; 10 bytes
	php
	ldx $A6
	ldx #(n-18)/7
	nop
	dex
	Jpl *-2
	plp
.endif
.endif
.else
; 40 to 78
.if((n .mod 5 = 0) && (n = 40 || n = 45 || (n > 54 && n < 76)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 41)
	; 7 bytes
	php
	ldy #136 ;hides 'dey'
	dey
	Jmi *-2
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
.elseif(n = 44 || n = 46 || n = 47 || n = 49 || n = 52 || n = 54 || n = 61 || n = 66 || n = 67 || n = 72 || n = 74 || n = 77)
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
.elseif(n = 51)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	nop
	dey
	Jmi *-3
	plp
.elseif(n = 56)
	; 6 bytes
	php
	ldy #136 ;hides 'dey'
	Jmi *-1
	plp
.elseif(n = 57 || n = 71)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 59)
	; 8 bytes
	php
	ldy $A4
	ldy #136 ;hides 'dey'
	Jmi *-1
	plp
.elseif(n = 62)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	inx
	Jne *-2
	plp
.elseif(n = 69)
	; 7 bytes
	php
	ldx #136 ;hides 'dey'
	dex
	Jmi *-2
	plp
.elseif(n = 76)
	; 7 bytes
	php
	ldy #136 ;hides 'dey'
	nop
	Jmi *-2
	plp
.endif
.endif
.else
; 79 to 156
.if(n = 79 || n = 81 || n = 82 || n = 84 || n = 86 || n = 89 || n = 91 || n = 94 || n = 97 || n = 101 || n = 102 || n = 104 || n = 107 || n = 109 || n = 111 || n = 112 || n = 116 || n = 117 || n = 119 || n = 121 || n = 122 || n = 124 || n = 129 || n = 132 || n = 136 || n = 137 || n = 139 || n = 142 || n = 144 || n = 146 || n = 147 || n = 149 || n = 151 || n = 152 || n = 154 || n = 156)
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
.elseif(n = 87)
	; 8 bytes
	php
	ldx #136 ;hides 'dey'
	nop
	dex
	Jmi *-3
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
.elseif(n = 96)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 4
	Jmi *-3
	plp
.elseif(n = 114)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 126)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 7
	Jmi *-3
	plp
.elseif(n = 131)
	; 6 bytes
	php
	ldx #232 ;hides 'inx'
	Jne *-1
	plp
.endif
.endif
.else
; 157 to 312
.if(n = 157 || n = 159 || n = 161 || n = 164 || n = 166 || n = 167 || n = 171 || n = 172 || n = 179 || n = 182 || n = 184 || n = 187 || n = 189 || n = 191 || n = 192 || n = 194 || n = 196 || n = 199 || n = 201 || n = 202 || n = 206 || n = 207 || n = 209 || n = 212 || n = 214 || n = 216 || n = 217 || n = 219 || n = 221 || n = 224 || n = 226 || n = 227 || n = 229 || n = 234 || n = 236 || n = 237 || n = 241 || n = 242 || n = 244 || n = 247 || n = 251 || n = 252 || n = 254 || n = 256 || n = 257 || n = 259 || n = 261 || n = 262 || n = 264 || n = 266 || n = 269 || n = 271 || n = 277 || n = 279 || n = 282 || n = 284 || n = 289 || n = 292 || n = 296 || n = 297)
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
.elseif(n = 165 || n = 170 || n = 175 || n = 180 || n = 185 || n = 195 || n = 200 || n = 205 || n = 210 || n = 215 || n = 220 || n = 230 || n = 235 || n = 240 || n = 245 || n = 250 || n = 255 || n = 265 || n = 270 || n = 275 || n = 280 || n = 285 || n = 290 || n = 299 || n = 300 || n = 305 || n = 310)
	; 8..10 bytes
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
	ldy #200 ;hides 'iny'
	iny
	iny
	Jne *-3
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
	ldy #202 ;hides 'dex'
	iny
	iny
	Jne *-3
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
	ldy #186 ;hides 'tsx'
	dey
	dey
	Jmi *-3
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
.elseif(n = 291)
	; 6 bytes
	php
	ldy #200 ;hides 'iny'
	Jmi *-1
	plp
.elseif(n = 294)
	; 8 bytes
	php
	ldy $A4
	ldy #200 ;hides 'iny'
	Jmi *-1
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
.elseif(n = 307)
	; 7 bytes
	php
	ldy #170 ;hides 'tax'
	dey
	Jmi *-2
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
.elseif(n = 315 || n = 317 || n = 320 || n = 322 || n = 324 || n = 325 || n = 329 || n = 334 || n = 335 || n = 340 || n = 342 || n = 345 || n = 347 || n = 349 || n = 350 || n = 354 || n = 355 || n = 357 || n = 360 || n = 362 || n = 364 || n = 369 || n = 370 || n = 374 || n = 375 || n = 377 || n = 380 || n = 382 || n = 385 || n = 387 || n = 390)
	; 8..10 bytes
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
.elseif(n = 319)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	nop
	dey
	Jmi *-3
	plp
.elseif(n = 321)
	; 8 bytes
	php
	ldy #186 ;hides 'tsx'
	iny
	iny
	Jne *-3
	plp
.elseif(n = 326 || n = 341 || n = 356 || n = 361 || n = 371 || n = 381)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 327)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jmi *-3
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
.elseif(n = 339)
	; 8 bytes
	php
	ldx #200 ;hides 'iny'
	dex
	dex
	Jmi *-3
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
	; 7 bytes
	php
	ldy #202 ;hides 'dex'
	iny
	Jne *-2
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
.if(n = 391 || n = 396 || n = 401 || n = 406 || n = 436 || n = 451 || n = 461 || n = 466)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 392 || n = 416 || n = 432)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 5 = 3) && ((n > 392 && n < 409) || n = 418 || n = 433 || n = 448 || (n > 457 && n < 469)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 394 || n = 395 || n = 397 || n = 399 || n = 409 || n = 410 || n = 412 || n = 415 || n = 417 || n = 420 || n = 422 || n = 425 || n = 427 || n = 429 || n = 430 || n = 434 || n = 439 || n = 440 || n = 444 || n = 445 || n = 447 || n = 450 || n = 452 || n = 454 || n = 455 || n = 457 || n = 460 || n = 462 || n = 465 || n = 467)
	; 8..10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 413 || n = 423 || n = 428 || n = 438 || n = 443 || n = 453)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 419)
	; 7 bytes
	php
	ldy #186 ;hides 'tsx'
	dey
	Jmi *-2
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
.elseif(n = 459)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
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
.if((n >= 469) && (n < 508))
; 469 to 507
.if(n = 469 || n = 475 || n = 480 || n = 482 || n = 485 || n = 489 || n = 490 || n = 495 || n = 497 || n = 500 || n = 502)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 470 || n = 484 || n = 491)
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
.elseif(n = 472 || n = 504)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 473 || n = 478)
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
.elseif(n = 476 || n = 481)
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
.elseif((n .mod 5 = 3) && ((n > 482 && n < 504)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 486 || n = 506)
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
	ldy #186 ;hides 'tsx'
	iny
	Jne *-2
	plp
.elseif(n = 499)
	; 9 bytes
	php
	ldy $A4
	ldy #186 ;hides 'tsx'
	iny
	Jne *-2
	plp
.elseif(n = 501)
	; 8 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	nop
	dey
	Jmi *-3
	plp
.elseif(n = 507)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 508 to 546
.if(n = 508 || n = 528)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 509 || n = 514 || n = 515 || n = 520 || n = 522 || n = 525 || n = 527 || n = 530 || n = 532 || n = 535 || n = 542 || n = 545)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 510)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
	plp
.elseif(n = 511)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 512 || n = 519 || n = 526 || n = 540)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 513 || n = 518 || n = 523 || n = 533 || n = 538 || n = 543)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 516 || n = 521 || n = 541)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 517)
	; 7 bytes
	php
	ldx #200 ;hides 'iny'
	dex
	Jmi *-2
	plp
.elseif(n = 524)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 529)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-18)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 531)
	; 7 bytes
	php
	ldy #202 ;hides 'dex'
	dey
	Jmi *-2
	plp
.elseif(n = 534)
	; 9 bytes
	php
	ldy $A4
	ldy #202 ;hides 'dex'
	dey
	Jmi *-2
	plp
.elseif(n = 536)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 537)
	; 8 bytes
	php
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jmi *-3
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
.elseif(n = 546)
	; 8 bytes
	php
	ldx #136 ;hides 'dey'
	inx
	inx
	Jne *-3
	plp
.endif
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
.elseif(n = 549 || n = 550 || n = 555 || n = 560 || n = 562 || n = 565 || n = 569 || n = 570 || n = 574 || n = 577 || n = 579 || n = 580 || n = 585 || n = 587 || n = 590 || n = 594 || n = 595 || n = 597 || n = 600 || n = 604 || n = 605 || n = 607 || n = 609 || n = 612 || n = 614 || n = 615 || n = 619 || n = 620 || n = 625)
	; 8..10 bytes
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
	; 10 bytes
	php
	ldx $A6
	ldx #(n-18)/7
	nop
	dex
	Jpl *-2
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
.elseif(n = 599)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 601 || n = 611)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 622)
	; 7 bytes
	php
	ldx #168 ;hides 'tay'
	inx
	Jne *-2
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if(n = 626 || n = 641 || n = 656)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-16)/5
	dex
	Jpl *-1
	plp
.elseif(n = 627 || n = 630 || n = 634 || n = 635 || n = 637 || n = 639 || n = 640 || n = 642 || n = 650 || n = 655 || n = 657 || n = 660 || n = 662 || n = 665 || n = 667 || n = 670 || n = 675 || n = 677 || n = 682 || n = 684 || n = 685 || n = 689 || n = 690 || n = 695 || n = 697 || n = 700 || n = 702)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && (n = 628 || n = 633 || n = 643 || n = 648 || (n > 657 && n < 704)))
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
.elseif(n = 631 || n = 659 || n = 666)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 632 || n = 664)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 636)
	; 8 bytes
	php
	ldy #186 ;hides 'tsx'
	nop
	iny
	Jne *-3
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
.elseif(n = 645 || n = 652 || n = 680 || n = 687 || n = 694 || n = 701)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 646 || n = 651 || n = 661 || n = 671 || n = 676 || n = 686)
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 647)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 649)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 654)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 669)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 672)
	; 8 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	nop
	iny
	Jne *-3
	plp
.elseif(n = 674 || n = 692)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 679)
	; 10 bytes
	php
	ldx #(n-19)/11
	delay_n 6
	dex
	Jpl *-4
	plp
.elseif(n = 681)
	; 8 bytes
	php
	ldy #202 ;hides 'dex'
	nop
	dey
	Jmi *-3
	plp
.elseif(n = 691)
	; 6 bytes
	php
	ldy #136 ;hides 'dey'
	Jne *-1
	plp
.elseif(n = 696)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 699)
	; 8 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.endif
.else
; 704 to 781
.if(n = 704 || n = 752)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 705 || n = 707 || n = 709 || n = 710 || n = 714 || n = 720 || n = 724 || n = 725 || n = 730 || n = 732 || n = 735 || n = 740 || n = 742 || n = 745 || n = 747 || n = 749 || n = 759 || n = 760 || n = 765 || n = 767 || n = 769 || n = 770 || n = 772 || n = 775 || n = 777 || n = 779 || n = 780)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 705 && n < 732) || n = 746 || (n > 755 && n < 767) || n = 781))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 5 = 3) && ((n > 707 && n < 779)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 712 || n = 744 || n = 776)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 714 && n < 730) || (n > 749 && n < 772)))
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 717)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 719)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	iny
	Jne *-3
	plp
.elseif(n = 727)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	nop
	iny
	Jne *-3
	plp
.elseif(n = 734)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
	plp
.elseif(n = 736)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 737)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 739)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 741)
	; 7 bytes
	php
	ldy #232 ;hides 'inx'
	dey
	Jmi *-2
	plp
.elseif(n = 751)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 754)
	; 9 bytes
	php
	ldx #136 ;hides 'dey'
	nop
	dex
	dex
	Jne *-4
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
	ldx #168 ;hides 'tay'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 774)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-18)/7
	nop
	dex
	Jpl *-2
	plp
.endif
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
.elseif(n = 784 || n = 816 || n = 824 || n = 832 || n = 856 || n = 864 || n = 896 || n = 936)
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
.elseif((n .mod 5 = 1) && ((n > 785 && n < 802) || n = 811 || (n > 820 && n < 837) || n = 851 || (n > 860 && n < 872) || (n > 880 && n < 892) || n = 901 || (n > 915 && n < 932)))
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
.elseif(n = 790 || n = 794 || n = 795 || n = 797 || n = 800 || n = 802 || n = 805 || n = 810 || n = 814 || n = 815 || n = 817 || n = 825 || n = 829 || n = 830 || n = 835 || n = 837 || n = 840 || n = 842 || n = 845 || n = 847 || n = 850 || n = 852 || n = 859 || n = 860 || n = 865 || n = 867 || n = 870 || n = 874 || n = 875 || n = 877 || n = 880 || n = 882 || n = 885 || n = 887 || n = 889 || n = 894 || n = 895 || n = 900 || n = 902 || n = 905 || n = 907 || n = 909 || n = 910 || n = 914 || n = 915 || n = 917 || n = 919 || n = 920 || n = 922 || n = 924 || n = 929 || n = 930 || n = 934 || n = 935 || n = 937)
	; 8..10 bytes
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
.elseif(n = 819)
	; 9 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	iny
	Jne *-4
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
.elseif(n = 844)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-18)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 846)
	; 7 bytes
	php
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
	plp
.elseif(n = 849)
	; 9 bytes
	php
	ldy $A4
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
	plp
.elseif(n = 872 || n = 899)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
	plp
.elseif(n = 879)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
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
.elseif(n = 940 || n = 945 || n = 947 || n = 950 || n = 954 || n = 955 || n = 957 || n = 959 || n = 964 || n = 970 || n = 972 || n = 975 || n = 977 || n = 980 || n = 982 || n = 985 || n = 987 || n = 990 || n = 994 || n = 999 || n = 1000 || n = 1005 || n = 1007 || n = 1010 || n = 1012 || n = 1015 || n = 1017 || n = 1020 || n = 1022 || n = 1025 || n = 1027 || n = 1035 || n = 1039 || n = 1040 || n = 1042 || n = 1045 || n = 1047 || n = 1049 || n = 1050 || n = 1052 || n = 1055 || n = 1057 || n = 1060 || n = 1062 || n = 1069 || n = 1070 || n = 1074 || n = 1075 || n = 1080 || n = 1082 || n = 1084 || n = 1085 || n = 1087 || n = 1090 || n = 1092)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && (n = 941 || (n > 955 && n < 972) || (n > 985 && n < 1012) || (n > 1025 && n < 1047) || (n > 1060 && n < 1082) || n = 1091))
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
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 949 || n = 1019 || n = 1054 || n = 1089)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 951)
	; 8 bytes
	php
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
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
.elseif(n = 965)
	; 7 bytes
	php
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
	plp
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
.elseif(n = 1029)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	nop
	dey
	dey
	Jne *-4
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
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	plp
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1095 || n = 1097 || n = 1099 || n = 1105 || n = 1109 || n = 1110 || n = 1115 || n = 1117 || n = 1119 || n = 1120 || n = 1125 || n = 1127 || n = 1129 || n = 1130 || n = 1132 || n = 1134 || n = 1139 || n = 1140 || n = 1145 || n = 1147 || n = 1150 || n = 1154 || n = 1155 || n = 1157 || n = 1160 || n = 1162 || n = 1164 || n = 1165 || n = 1167 || n = 1174 || n = 1175 || n = 1179 || n = 1180 || n = 1185 || n = 1187 || n = 1190 || n = 1195 || n = 1197 || n = 1200 || n = 1202 || n = 1204 || n = 1209 || n = 1210 || n = 1215 || n = 1217 || n = 1220 || n = 1222 || n = 1225 || n = 1227 || n = 1230 || n = 1232 || n = 1234 || n = 1235 || n = 1237 || n = 1245 || n = 1249 || n = 1250)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1096 || n = 1104 || n = 1112 || n = 1136 || n = 1144 || n = 1152 || n = 1176 || n = 1192 || n = 1216 || n = 1224)
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
.elseif((n .mod 5 = 1) && ((n > 1100 && n < 1117) || n = 1126 || n = 1131 || (n > 1140 && n < 1152) || (n > 1160 && n < 1172) || n = 1181 || n = 1186 || (n > 1200 && n < 1212) || n = 1221 || (n > 1230 && n < 1247)))
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
.elseif(n = 1159 || n = 1229)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
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
.elseif(n = 1182)
	; 7 bytes
	php
	ldx #168 ;hides 'tay'
	dex
	Jne *-2
	plp
.elseif(n = 1189)
	; 9 bytes
	php
	pha
	ldx #168 ;hides 'tay'
	dex
	Jne *-2
	pla
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
.elseif(n = 1196)
	; 7 bytes
	php
	ldy #170 ;hides 'tax'
	dey
	Jne *-2
	plp
.elseif(n = 1199)
	; 9 bytes
	php
	ldy $A4
	ldy #170 ;hides 'tax'
	dey
	Jne *-2
	plp
.elseif(n = 1239)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
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
; 1251 to 1328
.if((n .mod 5 = 1) && (n = 1251 || (n > 1265 && n < 1287)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 1252 || n = 1255 || n = 1257 || n = 1260 || n = 1262 || n = 1265 || n = 1267 || n = 1269 || n = 1270 || n = 1274 || n = 1279 || n = 1280 || n = 1284 || n = 1285 || n = 1287 || n = 1290 || n = 1292 || n = 1298 || n = 1300 || n = 1305 || n = 1307 || n = 1314 || n = 1321 || n = 1323 || n = 1326)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 1252 && n < 1284)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 7 = 1) && (n = 1254 || n = 1261 || (n > 1274 && n < 1325)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1256 || n = 1264 || n = 1272 || n = 1312 || n = 1320 || n = 1328)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1259 || n = 1304 || n = 1313 || n = 1322)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
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
.elseif(n = 1297)
	; 9 bytes
	php
	ldx $A6
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1299 || n = 1306)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
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
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	bit $24
	dex
	dex
	Jne *-5
	plp
.elseif(n = 1308)
	; 7 bytes
	php
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
	plp
.elseif(n = 1309 || n = 1325)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1311)
	; 8 bytes
	php
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1315)
	; 9 bytes
	php
	ldy #136 ;hides 'dey'
	nop
	iny
	iny
	Jne *-4
	plp
.elseif(n = 1316)
	; 9 bytes
	php
	ldx #(n-20)/12
	delay_n 7
	dex
	Jpl *-3
	plp
.elseif(n = 1318)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1319)
	; 8 bytes
	php
	ldx #164 ;hides 'ldy zp'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1327)
	; 8 bytes
	php
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1330 || n = 1333 || n = 1346 || n = 1347 || n = 1351 || n = 1354 || n = 1356 || (n > 1360 && n < 1364) || n = 1375 || n = 1377 || n = 1382 || n = 1386 || n = 1389 || n = 1391 || n = 1396 || n = 1402)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1402)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1332)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.elseif(n = 1334 || n = 1341 || n = 1355 || n = 1369 || n = 1390 || n = 1397 || n = 1404)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1335)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	nop
	dey
	Jne *-3
	plp
.elseif(n = 1336 || n = 1344 || n = 1360 || n = 1368 || n = 1384 || n = 1392 || n = 1400)
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
.elseif(n = 1339 || n = 1350)
	; 10 bytes
	php
	ldx #(n-19)/11
	delay_n 6
	dex
	Jpl *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1339 && n < 1386) || n = 1403))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1342 || n = 1405)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1343)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
	plp
.elseif(n = 1348 || n = 1378 || n = 1398)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1353 || n = 1365 || n = 1379 || n = 1395 || n = 1399)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1357)
	; 10 bytes
	php
	ldy #166 ;hides 'ldx zp'
	delay_n 9
	iny
	Jne *-5
	plp
.elseif(n = 1364)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 1372)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 1374)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dex
	dex
	Jne *-5
	plp
.elseif(n = 1381)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	inx
	Jpl *-5
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
.elseif(n = 1393)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	inx
	Jpl *-5
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
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1435 || n = 1437 || n = 1451 || n = 1455 || n = 1463 || n = 1469 || n = 1470 || n = 1479 || n = 1483)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1479)))
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
.elseif(n = 1410 || n = 1414 || n = 1417 || n = 1419 || n = 1426 || n = 1431 || n = 1433 || n = 1438 || n = 1441 || n = 1442 || n = 1445 || n = 1452 || n = 1454 || n = 1459 || n = 1461 || n = 1473 || n = 1474 || n = 1477 || n = 1482)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1411 || n = 1425 || n = 1446 || n = 1453 || n = 1467 || n = 1481)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif((n .mod 9 = 8) && ((n > 1411 && n < 1449) || (n > 1465 && n < 1485)))
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
.elseif((n .mod 8 = 0) && ((n > 1415 && n < 1441) || n = 1456 || n = 1472 || n = 1480))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1418 || n = 1428 || n = 1458 || n = 1468)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1420)
	; 7 bytes
	php
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
	plp
.elseif(n = 1423)
	; 9 bytes
	php
	ldy $A4
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
	plp
.elseif(n = 1427)
	; 7 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
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
.elseif(n = 1444)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.elseif(n = 1447)
	; 9 bytes
	php
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	plp
.elseif(n = 1449)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 1465)
	; 9 bytes
	php
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
	plp
.elseif(n = 1476)
	; 9 bytes
	php
	ldy #232 ;hides 'inx'
	delay_n 7
	dey
	Jmi *-4
	plp
.endif
.else
; 1485 to 1562
.if((n .mod 7 = 1) && ((n > 1484 && n < 1563)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1486 || n = 1487 || n = 1489 || n = 1490 || n = 1494 || n = 1501 || n = 1503 || n = 1514 || n = 1515 || n = 1517 || n = 1522 || n = 1524 || n = 1530 || n = 1540 || n = 1543 || n = 1545 || n = 1546 || n = 1549 || n = 1550 || n = 1554 || n = 1557 || n = 1559)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 0) && ((n > 1487 && n < 1513) || n = 1528 || (n > 1543 && n < 1561)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1491 || n = 1497 || n = 1505 || n = 1519 || n = 1533 || n = 1535 || n = 1542)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 9 = 8) && ((n > 1492 && n < 1512) || (n > 1528 && n < 1557)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1495 || n = 1509 || n = 1516 || n = 1523 || n = 1537 || n = 1551)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1498 || n = 1558)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1500)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
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
.elseif(n = 1508 || n = 1532)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1510 || n = 1531)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1518)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1521)
	; 9 bytes
	php
	ldx $A6
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
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
.elseif(n = 1526)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 1536)
	; 8 bytes
	php
	ldy #170 ;hides 'tax'
	nop
	dey
	Jne *-3
	plp
.elseif(n = 1539)
	; 10 bytes
	php
	ldy $A4
	ldy #170 ;hides 'tax'
	nop
	dey
	Jne *-3
	plp
.elseif(n = 1553)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 4
	iny
	iny
	Jne *-5
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
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1589 || n = 1605 || n = 1609 || n = 1623 || n = 1631)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1564 || n = 1566 || n = 1570 || n = 1571 || n = 1573 || n = 1577 || n = 1578 || n = 1582 || (n > 1584 && n < 1588) || n = 1593 || n = 1595 || n = 1599 || n = 1602 || n = 1606 || n = 1612 || n = 1613 || n = 1615 || n = 1620 || n = 1622 || n = 1627 || n = 1629 || n = 1634 || n = 1636)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 8) && (n = 1565 || n = 1574 || (n > 1591 && n < 1638)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1567)
	; 8 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif((n .mod 8 = 0) && (n = 1568 || n = 1584 || (n > 1599 && n < 1625) || n = 1640))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1640)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1572 || n = 1579 || n = 1607 || n = 1614 || n = 1621)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1575)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1580)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1581 || n = 1603)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 1588 || n = 1598 || n = 1638)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 1596)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.elseif(n = 1617)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 1626)
	; 8 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1630)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
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
	; 8 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
.endif
.else
; 1641 to 1718
.if((n > 1640 && n < 1644) || n = 1654 || n = 1658 || n = 1659 || n = 1669 || n = 1671 || n = 1675 || n = 1679 || n = 1683 || n = 1685 || n = 1690 || n = 1692 || n = 1693 || n = 1697 || n = 1699 || n = 1711 || n = 1713 || n = 1714)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1644)
	; 7 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1645 || n = 1661 || n = 1694 || n = 1701 || n = 1703 || n = 1707 || n = 1710 || n = 1715 || n = 1717)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 7 = 1) && ((n > 1645 && n < 1717)))
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
.elseif(n = 1648 || n = 1656 || n = 1672 || n = 1696 || n = 1704 || n = 1712)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1649 || n = 1663 || n = 1670 || n = 1684 || n = 1705)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1650)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 1651)
	; 9 bytes
	php
	pha
	ldx #234 ;hides 'nop'
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
.elseif((n .mod 9 = 8) && ((n > 1654 && n < 1701) || n = 1718))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1657)
	; 9 bytes
	php
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1662)
	; 8 bytes
	php
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
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
	ldy #166 ;hides 'ldx zp'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
	plp
.elseif(n = 1668 || n = 1678 || n = 1698 || n = 1708)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1677)
	; 9 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	plp
.elseif(n = 1680)
	; 8 bytes
	php
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jne *-3
	plp
.elseif(n = 1686)
	; 9 bytes
	php
	ldx #168 ;hides 'tay'
	ldy $A4
	dex
	Jne *-4
	plp
.elseif(n = 1687)
	; 10 bytes
	php
	pha
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jne *-3
	pla
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
.elseif(n = 1706)
	; 9 bytes
	php
	ldy #170 ;hides 'tax'
	ldx $A6
	dey
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if((n .mod 7 = 4) && ((n > 1718 && n < 1748) || n = 1782 || n = 1789))
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif((n .mod 8 = 0) && (n = 1720 || n = 1728 || (n > 1751 && n < 1793)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1721 || n = 1731 || n = 1743 || n = 1773 || n = 1785 || n = 1791)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1722 || n = 1725 || n = 1729 || n = 1732 || n = 1734 || n = 1739 || n = 1741 || n = 1746 || n = 1750 || n = 1753 || n = 1755 || n = 1756 || n = 1761 || n = 1762 || (n > 1768 && n < 1772) || n = 1774 || n = 1777 || n = 1788 || n = 1794 || n = 1795)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1794)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1724 || n = 1748 || n = 1796)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 1726 && n < 1764) || n = 1781 || n = 1790))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
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
.elseif(n = 1738 || n = 1778)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1742)
	; 7 bytes
	php
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
.elseif(n = 1757)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 1759)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1764 || n = 1780)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
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
.elseif(n = 1775)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1783)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1787)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 8
	Jne *-5
	plp
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797)
	; 8 bytes
	php
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	plp
.elseif(n = 1798 || n = 1818 || n = 1828)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 1802 || n = 1804 || n = 1810 || n = 1819 || n = 1822 || n = 1825 || n = 1827 || n = 1834)
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
.elseif(n = 1805 || n = 1815 || n = 1821 || n = 1829)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
	; 10 bytes
	php
	pha
	ldx #0
	nop
	dex
	Jne *-2
	pla
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
	ldx #164 ;hides 'ldy zp'
	nop
	ldy $A4
	dex
	Jne *-5
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
.elseif(n = 1816 || n = 1832)
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
	php
	ldy #202 ;hides 'dex'
	nop
	dey
	Jne *-3
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
.elseif(n = 1837 || (n > 1840 && n < 1844) || n = 1850 || n = 1855 || n = 1873 || n = 1874)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1838 || n = 1858)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1839)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif((n .mod 8 = 0) && ((n > 1839 && n < 1873)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 1843 && n < 1872)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1845 || n = 1867)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
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
.elseif((n .mod 2 = 1) && (n = 1847 || (n > 1856 && n < 1866) || n = 1869 || n = 1875))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
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
	; 10 bytes
	php
	tax
	ldy #229 ;hides 'sbc zp'
	iny ;first loop only
	dey
	Jne *-3
	txa
	plp
.elseif(n = 1852 || n = 1860)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.elseif(n = 1854)
	; 9 bytes
	php
	ldx #168 ;hides 'tay'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 1866)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	ldx $A6
	dey
	Jne *-4
	plp
.elseif(n = 1868)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1870)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
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
; 1876 to 1953
.if(n = 1876)
	; 9 bytes
	php
	ldy #170 ;hides 'tax'
	delay_n 4
	dey
	Jne *-4
	plp
.elseif(n = 1877 || n = 1879 || n = 1881 || n = 1883 || (n > 1884 && n < 1888) || n = 1893 || n = 1895 || n = 1897 || n = 1899 || (n > 1900 && n < 1904) || n = 1905 || n = 1913 || n = 1915 || n = 1919 || n = 1929 || n = 1931 || n = 1937 || n = 1941 || n = 1947 || n = 1949 || n = 1951)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1878 || n = 1900 || n = 1911 || n = 1933)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1879 && n < 1953)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1882 || n = 1890 || n = 1891 || n = 1894 || n = 1906 || n = 1909 || n = 1922 || n = 1927 || n = 1930 || n = 1942 || n = 1945 || n = 1946)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1884 || n = 1926)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif((n .mod 8 = 0) && ((n > 1887 && n < 1945)))
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1892 || n = 1940)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1908 || n = 1918 || n = 1938 || n = 1948)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1910)
	; 9 bytes
	php
	ldx #136 ;hides 'dey'
	delay_n 7
	dex
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
.elseif(n = 1917)
	; 10 bytes
	php
	ldx $A6
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
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
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	plp
.elseif(n = 1932)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
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
.elseif(n = 1950)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 1956 || n = 1980)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
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
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1959 || n = 1962 || n = 1963 || n = 1969 || n = 1972 || n = 1981 || n = 1989 || n = 1990)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 0) && ((n > 1959 && n < 1993)))
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
.elseif(n = 1964)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 1965 || n = 1971 || n = 1973 || n = 1983 || n = 1991)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 1974)
	; 10 bytes
	php
	ldx #164 ;hides 'ldy zp'
	ldy $EA ;hides 'nop'
	nop
	dex
	Jne *-5
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
.elseif(n = 1985)
	; 9 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 1986)
	; 9 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	delay_n 4
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
.if(n = 1993 || n = 2001 || n = 2003 || n = 2005 || n = 2013 || n = 2029 || n = 2031)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1994 || n = 2002 || n = 2009 || n = 2010 || n = 2014 || n = 2017 || n = 2025)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1995)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	dex
	Jne *-5
	plp
.elseif(n = 1996)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
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
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1999 || n = 2021)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2000 || n = 2008 || n = 2016)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2004 || n = 2020)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.elseif(n = 2007)
	; 9 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
	plp
.elseif(n = 2011)
	; 10 bytes
	php
	tax
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	iny
	Jne *-3
	txa
	plp
.elseif(n = 2012)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2019)
	; 9 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	nop
	dey
	Jne *-4
	plp
.elseif(n = 2022)
	; 10 bytes
	php
	pha
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
	pla
	plp
.elseif(n = 2023)
	; 9 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2026)
	; 9 bytes
	php
	ldy #202 ;hides 'dex'
	ldx $A6
	dey
	Jne *-4
	plp
.elseif(n = 2027)
	; 10 bytes
	php
	tax
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	iny
	Jne *-3
	txa
	plp
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
.endif
.else
.if((n >= 2032) && (n < 2110))
; 2032 to 2109
.if(n = 2032 || n = 2040 || n = 2048)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 2032 && n < 2106)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2034 || n = 2035 || n = 2044 || n = 2050 || n = 2053 || n = 2058 || n = 2062 || n = 2071 || n = 2074 || n = 2086 || n = 2089 || n = 2107)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2036 || n = 2072 || n = 2084 || n = 2108)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2037 || n = 2039 || n = 2041 || n = 2045 || n = 2047 || n = 2049 || n = 2055 || n = 2057 || n = 2059 || n = 2063 || n = 2064 || n = 2067 || n = 2070 || n = 2073 || n = 2075 || n = 2077 || n = 2079 || (n > 2080 && n < 2084) || n = 2090 || n = 2091 || n = 2093 || n = 2095 || n = 2099 || n = 2100 || n = 2103 || n = 2104 || n = 2106)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2038 || n = 2068 || n = 2088)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif((n .mod 11 = 8) && ((n > 2042 && n < 2077) || n = 2098 || n = 2109))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2046)
	; 10 bytes
	php
	ldy #170 ;hides 'tax'
	nop
	ldx $A6
	dey
	Jne *-5
	plp
.elseif(n = 2052)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	delay_n 4
	dey
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
.elseif(n = 2061)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-5
	plp
.elseif(n = 2066 || n = 2080)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2085)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
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
.elseif(n = 2094)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
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
	ldy #136 ;hides 'dey'
	delay_n 7
	dey
	dey
	Jne *-5
	plp
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2116 || n = 2125 || n = 2134 || n = 2143 || n = 2146 || n = 2152 || n = 2161 || n = 2170 || n = 2179 || n = 2182)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2111 || n = 2113 || n = 2117 || n = 2121 || n = 2124 || n = 2126 || n = 2127 || n = 2129 || n = 2130 || n = 2133 || n = 2135 || n = 2139 || n = 2140 || n = 2145 || n = 2147 || n = 2149 || n = 2151 || n = 2155 || n = 2157 || n = 2160 || n = 2162 || n = 2165 || n = 2167 || n = 2169 || (n > 2170 && n < 2174) || n = 2176 || n = 2181 || n = 2183 || n = 2184 || n = 2187)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2112)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif((n .mod 9 = 8) && ((n > 2113 && n < 2187)))
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
.elseif((n .mod 10 = 8) && ((n > 2117 && n < 2159) || n = 2178))
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 2120 || n = 2144 || n = 2156 || n = 2180)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2122 || n = 2136)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif((n .mod 11 = 8) && ((n > 2130 && n < 2176)))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
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
.elseif(n = 2154)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	bit $24
	inx
	inx
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
.elseif(n = 2166)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	bit $24
	dex
	Jne *-4
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
.elseif(n = 2185)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if(n = 2188 || n = 2198 || n = 2218)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2189 || n = 2191 || n = 2193 || n = 2196 || n = 2200 || n = 2201 || n = 2203 || n = 2205 || (n > 2209 && n < 2213) || n = 2221 || n = 2223 || n = 2225 || n = 2226 || n = 2229 || n = 2232 || (n > 2234 && n < 2238) || n = 2243 || n = 2244 || n = 2246 || n = 2253 || n = 2255 || n = 2261)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2190)
	; 10 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	nop
	ldx $A6
	dey
	Jne *-5
	plp
.elseif(n = 2192 || n = 2216 || n = 2228 || n = 2252 || n = 2264)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2194 || n = 2199 || n = 2209 || n = 2215 || n = 2224 || n = 2233 || n = 2242 || n = 2250 || n = 2251 || n = 2254 || n = 2259 || n = 2260)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 8) && ((n > 2194 && n < 2259)))
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
.elseif(n = 2202)
	; 10 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	ldx $A6
	dey
	Jne *-5
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
.elseif((n .mod 11 = 8) && ((n > 2207 && n < 2242) || n = 2263))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2214)
	; 10 bytes
	php
	ldx #184 ;hides 'clv'
	nop
	bit $24
	dex
	Jne *-5
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
.elseif(n = 2220 || n = 2234 || n = 2262)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2227)
	; 10 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
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
.elseif(n = 2256)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	inx
	Jne *-4
	plp
.elseif(n = 2257)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $06A2'
	ldx #6
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
.elseif((n .mod 9 = 8) && ((n > 2266 && n < 2304)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2268 || n = 2278 || n = 2298)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2269 || n = 2279 || n = 2289 || n = 2295 || n = 2299)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 2269 && n < 2274) || (n > 2279 && n < 2283) || n = 2291)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
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
.if(n = 2305 || n = 2313 || n = 2314)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2306 || n = 2309 || (n > 2314 && n < 2318) || (n > 2318 && n < 2324) || n = 2325 || n = 2327 || n = 2331 || n = 2334 || n = 2335 || n = 2337 || n = 2339 || (n > 2340 && n < 2344))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 11 = 8) && ((n > 2306 && n < 2341)))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2308 || n = 2328)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
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
.elseif(n = 2330)
	; 10 bytes
	php
	tsx
	ldy #24 ;hides 'clc'
	pha
	iny
	Jne *-3
	txs
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
.elseif(n = 2338)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if(n = 2344 || (n > 2351 && n < 2358) || n = 2361 || (n > 2362 && n < 2368) || n = 2369 || (n > 2374 && n < 2378) || (n > 2378 && n < 2382) || n = 2387 || (n > 2388 && n < 2394) || n = 2397 || (n > 2398 && n < 2402) || (n > 2402 && n < 2406) || n = 2407 || n = 2409 || (n > 2410 && n < 2416) || n = 2419 || n = 2421)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2345)
	; 10 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	nop
	dex
	Jne *-5
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
.elseif((n .mod 12 = 8) && ((n > 2347 && n < 2421)))
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
.elseif(n = 2350 || n = 2410)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2351 || n = 2362 || n = 2373 || n = 2395 || n = 2406 || n = 2417)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2358)
	; 9 bytes
	php
	ldx #168 ;hides 'tay'
	delay_n 7
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
.elseif((n .mod 10 = 8) && ((n > 2367 && n < 2399) || n = 2418))
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 2374 || n = 2402 || n = 2416)
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
.elseif(n = 2383)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2385)
	; 10 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	plp
.elseif(n = 2386)
	; 9 bytes
	php
	ldy #170 ;hides 'tax'
	delay_n 7
	dey
	Jne *-4
	plp
.elseif(n = 2394)
	; 10 bytes
	php
	ldy #200 ;hides 'iny'
	bit $24
	dey
	dey
	Jne *-5
	plp
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2434 || n = 2446 || n = 2470 || n = 2482)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423 || n = 2425 || n = 2426 || n = 2429 || n = 2431 || n = 2433 || (n > 2434 && n < 2438) || n = 2440 || n = 2441 || n = 2443 || n = 2447 || n = 2451 || n = 2453 || n = 2455 || n = 2457 || n = 2459 || n = 2460 || (n > 2461 && n < 2467) || n = 2469 || (n > 2472 && n < 2478) || n = 2479 || n = 2481 || n = 2484 || n = 2485 || n = 2487 || n = 2489 || n = 2491 || n = 2493 || n = 2495 || n = 2496 || n = 2499)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2424)
	; 10 bytes
	php
	ldy #186 ;hides 'tsx'
	delay_n 6
	dey
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
.elseif((n .mod 11 = 8) && ((n > 2427 && n < 2495)))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 2438 || n = 2448 || n = 2458 || n = 2478 || n = 2488 || n = 2498)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 2449)
	; 10 bytes
	php
	pha
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	pla
	plp
.elseif(n = 2452)
	; 10 bytes
	php
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	delay_n 7
	dex
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
.elseif(n = 2467)
	; 10 bytes
	php
	ldx #164 ;hides 'ldy zp'
	delay_n 9
	dex
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
.elseif(n = 2486)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2490)
	; 10 bytes
	php
	tsx
	ldy #248 ;hides 'sed'
	pha
	dey
	Jne *-3
	txs
	plp
.elseif(n = 2497)
	; 10 bytes
	php
	ldy #166 ;hides 'ldx zp'
	delay_n 9
	dey
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
; 2501 to 2656
.if((n > 2500 && n < 2504) || n = 2507 || (n > 2508 && n < 2512) || n = 2513 || n = 2517 || (n > 2518 && n < 2526) || n = 2529 || (n > 2530 && n < 2538) || n = 2539 || n = 2541 || (n > 2542 && n < 2547) || n = 2551 || n = 2553 || n = 2555 || n = 2557 || n = 2559 || n = 2561 || n = 2563 || n = 2565 || n = 2567 || n = 2569 || n = 2572 || n = 2573 || n = 2575 || n = 2577 || n = 2579 || n = 2581 || n = 2583 || (n > 2584 && n < 2588) || n = 2589 || n = 2591 || n = 2592 || (n > 2593 && n < 2598) || n = 2599 || n = 2601 || n = 2603 || n = 2605 || n = 2607 || n = 2608 || n = 2611 || n = 2613 || (n > 2615 && n < 2624) || n = 2625 || (n > 2626 && n < 2632) || n = 2633 || n = 2634 || n = 2639 || (n > 2640 && n < 2645) || n = 2649 || (n > 2650 && n < 2654) || n = 2655 || n = 2656)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 12 = 8) && ((n > 2503 && n < 2649)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif((n .mod 11 = 8) && (n = 2505 || (n > 2526 && n < 2572) || (n > 2592 && n < 2638)))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif((n .mod 12 = 10) && (n = 2506 || n = 2530 || (n > 2553 && n < 2615) || n = 2638 || n = 2650))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2508 || n = 2518 || n = 2548)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2512)
	; 10 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	plp
.elseif((n .mod 14 = 8) && (n = 2514 || (n > 2541 && n < 2599) || n = 2640 || n = 2654))
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
.elseif(n = 2526)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	cmp $C5
	dex
	Jne *-5
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
.elseif(n = 2550)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	cmp $C5
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
.elseif(n = 2562)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2568)
	; 10 bytes
	php
	ldx #0
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 2580)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2582)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 2606)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-5
	plp
.elseif(n = 2609)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
	plp
.elseif(n = 2610)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	delay_n 7
	dey
	Jne *-4
	plp
.elseif(n = 2632)
	; 10 bytes
	php
	ldy #202 ;hides 'dex'
	delay_n 6
	dey
	Jne *-5
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
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if(n = 2657 || n = 2658 || n = 2661 || (n > 2662 && n < 2668) || n = 2669 || n = 2671 || n = 2673 || (n > 2674 && n < 2681) || n = 2683 || n = 2685 || (n > 2686 && n < 2692) || n = 2693 || n = 2695 || n = 2697 || (n > 2698 && n < 2703) || (n > 2703 && n < 2708) || n = 2709 || (n > 2710 && n < 2714) || (n > 2714 && n < 2720) || n = 2721 || n = 2723 || (n > 2726 && n < 2731) || n = 2733)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 11 = 8) && ((n > 2658 && n < 2726)))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif((n .mod 12 = 8) && ((n > 2659 && n < 2733)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2662 || n = 2674 || n = 2686 || n = 2698 || n = 2722)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2668 || n = 2682 || n = 2710 || n = 2724)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2694)
	; 10 bytes
	php
	ldx #168 ;hides 'tay'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 2726)
	; 10 bytes
	php
	ldy #170 ;hides 'tax'
	delay_n 9
	dey
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
.else
; 2735 to 2812
.if(n = 2735 || n = 2737 || (n > 2738 && n < 2742) || (n > 2747 && n < 2752) || n = 2753 || n = 2754 || (n > 2758 && n < 2762) || (n > 2762 && n < 2766) || (n > 2770 && n < 2774) || n = 2776 || n = 2777 || n = 2781 || (n > 2782 && n < 2788) || n = 2793 || (n > 2794 && n < 2800) || n = 2803 || n = 2805 || n = 2807 || n = 2809 || n = 2810)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 11 = 8) && ((n > 2735 && n < 2770) || n = 2791 || n = 2802))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2738 || n = 2752 || n = 2766 || n = 2794 || n = 2808)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
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
.elseif((n .mod 12 = 8) && ((n > 2743 && n < 2805)))
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
.elseif(n = 2746 || n = 2770 || n = 2782)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2762)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
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
.elseif(n = 2774)
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
.elseif(n = 2778)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	bit $24
	dex
	dex
	Jne *-5
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
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	cmp $C5
	inx
	Jne *-5
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
.else
; 2813 to 3125
.if(n = 2813)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2814)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2815 || n = 2817 || (n > 2818 && n < 2822) || (n > 2824 && n < 2828) || n = 2829 || (n > 2830 && n < 2834) || n = 2835 || (n > 2836 && n < 2840) || n = 2841 || (n > 2842 && n < 2848) || n = 2849 || n = 2851 || n = 2853 || (n > 2854 && n < 2864) || n = 2865 || (n > 2866 && n < 2870) || (n > 2870 && n < 2876) || n = 2877 || (n > 2878 && n < 2886) || n = 2887 || n = 2889 || n = 2891 || (n > 2892 && n < 2900) || n = 2901 || (n > 2902 && n < 2906) || (n > 2906 && n < 2912) || n = 2913 || (n > 2914 && n < 2918) || n = 2919 || (n > 2920 && n < 2924) || n = 2925 || (n > 2926 && n < 2932) || n = 2933 || n = 2935 || n = 2937 || (n > 2938 && n < 2947) || n = 2949 || (n > 2950 && n < 2960) || n = 2961 || (n > 2962 && n < 2972) || n = 2973 || n = 2975 || (n > 2977 && n < 2982) || n = 2983 || n = 2985 || (n > 2986 && n < 2990) || (n > 2990 && n < 2996) || n = 2997 || (n > 2998 && n < 3004) || (n > 3004 && n < 3008) || (n > 3010 && n < 3018) || n = 3019 || n = 3021 || (n > 3022 && n < 3030) || n = 3031 || n = 3033 || (n > 3034 && n < 3044) || n = 3045 || n = 3047 || n = 3049 || n = 3050 || (n > 3051 && n < 3056) || n = 3057 || n = 3059 || (n > 3060 && n < 3068) || n = 3069 || (n > 3070 && n < 3074) || (n > 3074 && n < 3080) || n = 3081 || (n > 3082 && n < 3088) || (n > 3088 && n < 3102) || (n > 3102 && n < 3116) || (n > 3116 && n < 3126))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 12 = 8) && ((n > 2815 && n < 3069)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif((n .mod 12 = 10) && ((n > 2817 && n < 2867) || (n > 2889 && n < 2951) || (n > 2973 && n < 3035) || (n > 3057 && n < 3083)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 8) && ((n > 2821 && n < 2851) || (n > 2877 && n < 2935) || (n > 2961 && n < 3019) || (n > 3045 && n < 3117)))
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
	; 10 bytes
	php
	ldx #0
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2834)
	; 9 bytes
	php
	ldy #202 ;hides 'dex'
	delay_n 7
	dey
	Jne *-4
	plp
.elseif(n = 2848)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
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
.elseif(n = 2886)
	; 10 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 2918)
	; 10 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	delay_n 9
	dey
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
.elseif(n = 2947)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
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
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	cmp $C5
	dex
	Jne *-5
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
.elseif(n = 3030)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3048)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-5
	plp
.elseif(n = 3051)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 10
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
.endif
.endif
.else
; 3126 to 3750
.if((n > 3125 && n < 3130) || (n > 3130 && n < 3144) || (n > 3144 && n < 3158) || (n > 3158 && n < 3172) || (n > 3172 && n < 3186) || (n > 3186 && n < 3190) || (n > 3190 && n < 3200) || (n > 3200 && n < 3206) || (n > 3206 && n < 3214) || (n > 3214 && n < 3228) || n = 3229 || (n > 3230 && n < 3238) || (n > 3238 && n < 3242) || (n > 3242 && n < 3254) || n = 3255 || (n > 3256 && n < 3270) || (n > 3270 && n < 3282) || n = 3283 || (n > 3284 && n < 3292) || (n > 3292 && n < 3298) || (n > 3298 && n < 3307) || (n > 3307 && n < 3312) || (n > 3312 && n < 3322) || (n > 3322 && n < 3326) || (n > 3326 && n < 3340) || (n > 3340 && n < 3354) || (n > 3354 && n < 3368) || (n > 3368 && n < 3382) || (n > 3382 && n < 3396) || n = 3397 || (n > 3398 && n < 3410) || n = 3411 || n = 3413 || (n > 3414 && n < 3424) || n = 3425 || n = 3426 || n = 3428 || n = 3429 || (n > 3430 && n < 3438) || (n > 3438 && n < 3442) || (n > 3442 && n < 3452) || (n > 3452 && n < 3462) || (n > 3462 && n < 3466) || (n > 3466 && n < 3478) || n = 3479 || (n > 3480 && n < 3494) || (n > 3494 && n < 3508) || (n > 3508 && n < 3522) || (n > 3522 && n < 3536) || (n > 3536 && n < 3550) || (n > 3550 && n < 3564) || (n > 3564 && n < 3578) || (n > 3578 && n < 3592) || (n > 3592 && n < 3702) || (n > 3702 && n < 3718) || (n > 3718 && n < 3750))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 14 = 8) && ((n > 3129 && n < 3579)))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3190)
	; 10 bytes
	php
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	plp
.elseif(n = 3206)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3230)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 6
	dex
	Jne *-5
	plp
.elseif(n = 3238)
	; 10 bytes
	php
	ldy #202 ;hides 'dex'
	delay_n 9
	dey
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
.elseif(n = 3282)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	delay_n 7
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
.elseif(n = 3398)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
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
.elseif(n = 3442)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3462)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 9
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
.elseif(n = 3592)
	; 10 bytes
	php
	ldx #0
	delay_n 9
	dex
	Jne *-4
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
.elseif(n = 3750)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
	plp
.endif
.endif
.else
; 3751 to 5000
.if((n > 3750 && n < 3863) || (n > 3863 && n < 3974) || (n > 3974 && n < 5001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3863)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 14
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
.endif
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 10001))
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
