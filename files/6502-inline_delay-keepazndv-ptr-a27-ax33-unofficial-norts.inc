; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Y, and C
; Will preserve A, S, Z+N, D, V, and I (interrupt-disable flag)

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
; - 4 bytes or less when n <= 11
; - 5 bytes or less when n <= 14
; - 6 bytes or less when n <= 19
; - 7 bytes or less when n <= 21
; - 8 bytes or less when n <= 26
; - 9 bytes or less when n <= 1298
; -10 bytes or less when n <= 1352
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=4, avg=5.4, max=7
; - When 21 <= n <= 30: min=6, avg=7.7, max=9
; - When 31 <= n <= 50: min=7, avg=8.2, max=9
; - When 51 <= n <= 75: min=6, avg=7.9, max=9
; - When 76 <= n <= 100: min=7, avg=8.1, max=9
; - When 101 <= n <= 200: min=6, avg=8.2, max=9
; - When 201 <= n <= 300: min=6, avg=8.2, max=9
; - When 301 <= n <= 500: min=6, avg=8.2, max=9
; - When 501 <= n <= 750: min=6, avg=8.2, max=9
; - When 751 <= n <= 1000: min=7, avg=8.2, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.9, max=11
; - When 2001 <= n <= 3000: min=8, avg=10.1, max=11
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
.if((n >= 1) && (n < 20))
; 1 to 19
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	nop $04
.elseif(n = 4 || n = 6 || n = 12)
	; 2..5 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 3 bytes
	clc
	Jcc *+2
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
.elseif(n = 14)
	; 4 bytes
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
	; 8 bytes
	php
	bit $24
	rol $36,X
	ror $36,X
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
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.if(n = 79 || n = 81 || n = 82 || n = 84 || n = 89 || n = 91 || n = 94 || n = 97 || n = 101 || n = 102 || n = 104 || n = 107 || n = 109 || n = 111 || n = 112 || n = 116 || n = 117 || n = 119 || n = 121 || n = 122 || n = 124 || n = 129 || n = 136 || n = 139 || n = 142 || n = 144 || n = 146 || n = 147 || n = 149 || n = 151 || n = 152 || n = 154 || n = 156)
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
.elseif(n = 86)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	nop $04
	Jmi *-3
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
.elseif(n = 132)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	dex
	dex
	Jpl *-3
	plp
.elseif(n = 137)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	Jne *-1
	sta (@ptrtemp,x)
	plp
.endif
.endif
.else
.if((n >= 157) && (n < 235))
; 157 to 234
.if(n = 157 || n = 159 || n = 161 || n = 164 || n = 166 || n = 167 || n = 171 || n = 172 || n = 179 || n = 182 || n = 184 || n = 187 || n = 189 || n = 191 || n = 192 || n = 194 || n = 196 || n = 199 || n = 201 || n = 202 || n = 207 || n = 209 || n = 212 || n = 214 || n = 216 || n = 217 || n = 219 || n = 221 || n = 224 || n = 226 || n = 227 || n = 229 || n = 234)
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif((n .mod 5 = 3) && ((n > 157 && n < 179) || n = 208 || n = 233))
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
.elseif(n = 162 || n = 169 || n = 197 || n = 204)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif((n .mod 5 = 0) && ((n > 164 && n < 186) || (n > 199 && n < 221) || n = 230))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 174)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	dex
	Jne *-2
	plp
.elseif(n = 176 || n = 190 || n = 211 || n = 225 || n = 232)
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
.elseif((n .mod 5 = 3) && ((n > 182 && n < 204) || (n > 212 && n < 229)))
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
	ldx #26 ;hides 'nop'
	dex
	Jpl *-2
	plp
.elseif(n = 206)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	nop $04
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
.endif
.else
; 235 to 312
.if((n .mod 5 = 0) && ((n > 234 && n < 256) || (n > 264 && n < 291) || (n > 299 && n < 311)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 236 || n = 237 || n = 241 || n = 242 || n = 244 || n = 247 || n = 251 || n = 252 || n = 254 || n = 256 || n = 257 || n = 259 || n = 261 || n = 262 || n = 264 || n = 266 || n = 269 || n = 271 || n = 277 || n = 279 || n = 282 || n = 284 || n = 289 || n = 292 || n = 296)
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 238 || n = 243 || n = 253 || n = 268 || n = 293 || n = 298 || n = 303)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 239 || n = 246)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 248 || n = 258 || n = 263 || n = 273 || n = 278 || n = 283 || n = 288 || n = 308)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 249)
	; 8 bytes
	php
	ldy #202 ;hides 'dex'
	iny
	iny
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 259 && n < 282) || (n > 294 && n < 310)))
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
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
.elseif(n = 297)
	; 8 bytes
	php
	ldy #200 ;hides 'iny'
	Jne *-1
	sta (@ptrtemp),y
	plp
.elseif(n = 299)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.elseif(n = 315 || n = 317 || n = 320 || n = 322 || n = 325 || n = 329 || n = 335 || n = 340 || n = 342 || n = 345 || n = 347 || n = 350 || n = 355 || n = 357 || n = 360 || n = 362 || n = 370 || n = 374 || n = 375 || n = 377 || n = 380 || n = 382 || n = 385 || n = 387 || n = 390)
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
.elseif(n = 324 || n = 334 || n = 349 || n = 354 || n = 364 || n = 369)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.elseif(n = 394 || n = 399 || n = 429 || n = 434 || n = 439 || n = 454 || n = 459)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 395 || n = 397 || n = 409 || n = 410 || n = 415 || n = 417 || n = 420 || n = 422 || n = 425 || n = 427 || n = 430 || n = 440 || n = 444 || n = 445 || n = 447 || n = 450 || n = 452 || n = 455 || n = 457 || n = 460 || n = 465 || n = 467)
	; 8..9 bytes
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
.elseif(n = 462)
	; 8 bytes
	php
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
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
.if(n = 469)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
	plp
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
.elseif(n = 475 || n = 480 || n = 482 || n = 485 || n = 489 || n = 490 || n = 495 || n = 497 || n = 500 || n = 502)
	; 8..9 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 509 || n = 529)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
	plp
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
.elseif(n = 514 || n = 515 || n = 520 || n = 522 || n = 525 || n = 527 || n = 530 || n = 532 || n = 535 || n = 542 || n = 545)
	; 8..9 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 549 || n = 550 || n = 555 || n = 560 || n = 562 || n = 565 || n = 569 || n = 570 || n = 577 || n = 580 || n = 585 || n = 587 || n = 590 || n = 595 || n = 597 || n = 600 || n = 605 || n = 607 || n = 612 || n = 615 || n = 619 || n = 620 || n = 625)
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
	sta (@ptrtemp,x)
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
	nop $04
	Jmi *-3
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
.elseif(n = 627 || n = 630 || n = 635 || n = 637 || n = 640 || n = 642 || n = 650 || n = 655 || n = 657 || n = 660 || n = 662 || n = 665 || n = 667 || n = 670 || n = 675 || n = 677 || n = 682 || n = 685 || n = 689 || n = 690 || n = 695 || n = 700 || n = 702)
	; 8..9 bytes
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
.elseif(n = 634 || n = 639 || n = 649 || n = 669 || n = 679 || n = 684)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.elseif(n = 654)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
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
.elseif(n = 697)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	Jne *-1
	sta (@ptrtemp),y
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
.elseif(n = 705 || n = 707 || n = 710 || n = 724 || n = 725 || n = 730 || n = 732 || n = 735 || n = 740 || n = 742 || n = 745 || n = 747 || n = 759 || n = 760 || n = 765 || n = 767 || n = 770 || n = 772 || n = 775 || n = 777 || n = 780)
	; 8..9 bytes
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
.elseif(n = 709 || n = 714 || n = 739 || n = 749 || n = 769 || n = 774 || n = 779)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.elseif(n = 720)
	; 7 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
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
.endif
.endif
.else
.if((n >= 782) && (n < 860))
; 782 to 859
.if(n = 782 || n = 809 || n = 854)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif((n .mod 5 = 3) && ((n > 782 && n < 859)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 784 || n = 824 || n = 832 || n = 856)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 785 || n = 792 || n = 806 || n = 820 || n = 841 || n = 855)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif((n .mod 5 = 1) && ((n > 785 && n < 802) || n = 811 || (n > 820 && n < 837) || n = 851))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 787 || n = 822 || n = 857)
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
.elseif(n = 790 || n = 794 || n = 795 || n = 797 || n = 800 || n = 802 || n = 805 || n = 810 || n = 815 || n = 825 || n = 829 || n = 830 || n = 835 || n = 837 || n = 840 || n = 842 || n = 845 || n = 847 || n = 850 || n = 852)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 799 || n = 827 || n = 834)
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
.elseif(n = 814 || n = 844 || n = 859)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 816)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 817)
	; 9 bytes
	php
	tay
	lax #3
	jsr delay_256a_x_33_clocks
	tya
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
.endif
.else
; 860 to 937
.if(n = 860)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif((n .mod 5 = 1) && ((n > 860 && n < 872) || (n > 880 && n < 892) || n = 901 || (n > 915 && n < 932)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 862 || n = 869 || n = 890 || n = 911)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif((n .mod 5 = 3) && ((n > 862 && n < 934)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 864 || n = 896 || n = 936)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 865 || n = 870 || n = 875 || n = 877 || n = 880 || n = 882 || n = 885 || n = 887 || n = 895 || n = 900 || n = 902 || n = 905 || n = 907 || n = 910 || n = 915 || n = 917 || n = 920 || n = 922 || n = 930 || n = 934 || n = 935 || n = 937)
	; 8..9 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 874 || n = 879 || n = 889 || n = 894 || n = 909 || n = 914 || n = 919 || n = 929)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 876 || n = 897 || n = 904 || n = 925 || n = 932)
	; 8 bytes
	php
	ldx #(n-8)/7
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
.elseif(n = 892 || n = 927)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
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
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if((n .mod 5 = 3) && ((n > 937 && n < 1014)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 7 = 1) && (n = 939 || n = 946 || (n > 959 && n < 982) || (n > 994 && n < 1010)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 940 || n = 945 || n = 947 || n = 950 || n = 955 || n = 957 || n = 970 || n = 972 || n = 975 || n = 977 || n = 980 || n = 982 || n = 985 || n = 990 || n = 1000 || n = 1005 || n = 1007 || n = 1010 || n = 1012 || n = 1015)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && (n = 941 || (n > 955 && n < 972) || (n > 985 && n < 1012)))
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
.elseif(n = 949 || n = 954 || n = 959 || n = 964 || n = 994 || n = 999)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 951)
	; 8 bytes
	php
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
	plp
.elseif(n = 952)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 962 || n = 997)
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
.elseif(n = 987)
	; 8 bytes
	php
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 989)
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
.endif
.else
; 1016 to 1093
.if((n .mod 7 = 1) && (n = 1016 || (n > 1029 && n < 1052) || (n > 1064 && n < 1087)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1017 || n = 1020 || n = 1022 || n = 1025 || n = 1035 || n = 1039 || n = 1040 || n = 1042 || n = 1045 || n = 1047 || n = 1050 || n = 1052 || n = 1055 || n = 1057 || n = 1060 || n = 1062 || n = 1070 || n = 1075 || n = 1080 || n = 1082 || n = 1085 || n = 1087 || n = 1090 || n = 1092)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 1017 && n < 1094)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 1019 || n = 1049 || n = 1054 || n = 1069 || n = 1084 || n = 1089)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.elseif((n .mod 5 = 1) && ((n > 1025 && n < 1037) || n = 1046 || (n > 1060 && n < 1082) || n = 1091))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 1027)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	sta (@ptrtemp,x)
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
.elseif(n = 1032 || n = 1056 || n = 1064)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1034)
	; 9 bytes
	php
	ldx #(n-17)/9
	delay_n 4
	dex
	Jpl *-3
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
.elseif(n = 1067)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1074)
	; 9 bytes
	php
	tay
	lax #4
	jsr delay_256a_x_33_clocks
	tya
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
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1099 || n = 1119 || n = 1129 || n = 1134 || n = 1139 || n = 1154 || n = 1159 || n = 1164 || n = 1174 || n = 1204 || n = 1209 || n = 1229 || n = 1234)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 1095 || n = 1097 || n = 1105 || n = 1109 || n = 1110 || n = 1115 || n = 1117 || n = 1120 || n = 1125 || n = 1127 || n = 1130 || n = 1132 || n = 1140 || n = 1145 || n = 1147 || n = 1150 || n = 1155 || n = 1157 || n = 1160 || n = 1162 || n = 1165 || n = 1167 || n = 1175 || n = 1179 || n = 1180 || n = 1185 || n = 1187 || n = 1190 || n = 1195 || n = 1197 || n = 1200 || n = 1202 || n = 1210 || n = 1215 || n = 1217 || n = 1220 || n = 1222 || n = 1225 || n = 1227 || n = 1230 || n = 1232 || n = 1235 || n = 1237 || n = 1245 || n = 1249 || n = 1250)
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
.elseif((n .mod 5 = 1) && ((n > 1100 && n < 1117) || n = 1126 || (n > 1140 && n < 1152) || (n > 1160 && n < 1172) || n = 1181 || n = 1186 || (n > 1200 && n < 1212) || n = 1221 || (n > 1230 && n < 1247)))
	; 9 bytes
	php
	ldx $A6
	ldx #(n-11)/5
	dex
	Jne *-1
	plp
.elseif(n = 1102)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
	plp
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
.elseif(n = 1137 || n = 1207 || n = 1242)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
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
	sta (@ptrtemp,x)
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
.if(n = 1290 || n = 1292 || n = 1298 || n = 1305 || n = 1307 || n = 1321 || n = 1323 || n = 1326)
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
.elseif(n = 1299 || n = 1306)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1300)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	sta (@ptrtemp,x)
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
.elseif(n = 1308)
	; 7 bytes
	php
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
	plp
.elseif(n = 1309)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
	plp
.elseif(n = 1311)
	; 8 bytes
	php
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1312 || n = 1320 || n = 1328)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1314)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.elseif(n = 1325)
	; 10 bytes
	php
	ldx #164 ;hides 'ldy zp'
	nop
	dex
	Jne *-3
	sta (@ptrtemp,x)
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
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1333 || n = 1347 || n = 1351 || n = 1354 || (n > 1360 && n < 1364))
	; 9..10 bytes
	delay_n (n-2)
	nop
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
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
	plp
.elseif(n = 1334)
	; 9 bytes
	php
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1335)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	nop
	dey
	Jne *-3
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
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
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
	bit <$2C ;hides 'bit $95A2'
	ldx #149
	dex
	Jne *-4
	plp
.elseif(n = 1350)
	; 10 bytes
	php
	ldx #121
	delay_n 6
	dex
	Jpl *-4
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
.elseif(n = 1369 || n = 1390 || n = 1397 || n = 1404)
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
.elseif(n = 1372)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 1378)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1379)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
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
.elseif(n = 1398)
	; 9 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
	sta (@ptrtemp,x)
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
.elseif(n = 1405)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
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
	sta (@ptrtemp,x)
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
.elseif(n = 1410 || n = 1414 || n = 1417 || n = 1431 || n = 1438 || n = 1441 || n = 1442 || n = 1445)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1411)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $9CA2'
	ldx #156
	dex
	Jne *-4
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
.elseif(n = 1418)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 1425)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1426)
	; 9 bytes
	php
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
	sta (@ptrtemp),y
	plp
.elseif(n = 1427)
	; 7 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	plp
.elseif(n = 1428)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $62A2'
	ldx #98
	inx
	Jne *-4
	plp
.elseif(n = 1433)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	sta (@ptrtemp,x)
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
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
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
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif((n .mod 7 = 1) && ((n > 1449 && n < 1479)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1451 || n = 1479)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
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
.elseif(n = 1463 || n = 1470)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
	plp
.elseif(n = 1465)
	; 9 bytes
	php
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
	plp
.elseif(n = 1469)
	; 10 bytes
	php
	ldx #<$2C ;hides 'bit $7BA2'
	ldx #123
	nop
	inx
	Jne *-5
	plp
.elseif(n = 1474)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $A3A2'
	ldx #163
	dex
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
.elseif(n = 1495 || n = 1516 || n = 1523)
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1497)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1498)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	sta (@ptrtemp,x)
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
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1528 || n = 1544 || n = 1552 || n = 1560)
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
.elseif(n = 1530 || n = 1540 || n = 1543 || n = 1549 || n = 1550 || n = 1557 || n = 1559)
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
	sta (@ptrtemp,x)
	plp
.elseif(n = 1535)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1536)
	; 8 bytes
	php
	ldy #170 ;hides 'tax'
	nop
	dey
	Jne *-3
	plp
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
	ldy #170 ;hides 'tax'
	nop
	dey
	Jne *-3
	sta (@ptrtemp),y
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
	ldy #136 ;hides 'dey'
	delay_n 4
	iny
	iny
	Jne *-5
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
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 1564 || n = 1570 || n = 1571 || n = 1577 || n = 1578 || (n > 1584 && n < 1588) || n = 1593 || n = 1595 || n = 1599)
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
	; 10 bytes
	php
	ldx $A6
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
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
.elseif(n = 1588)
	; 9 bytes
	php
	tay
	lax #6
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 1589)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
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
.elseif(n = 1598)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.endif
.else
; 1602 to 1640
.if(n = 1602 || n = 1606 || n = 1612 || n = 1613 || n = 1615 || n = 1620 || n = 1627 || n = 1629 || n = 1634)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1603)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
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
	; 10 bytes
	php
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jmi *-5
	plp
.elseif(n = 1607 || n = 1614 || n = 1621)
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
.elseif(n = 1622)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
	sta (@ptrtemp,x)
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
.elseif(n = 1631)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
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
.elseif(n = 1636)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
	sta (@ptrtemp,x)
	plp
.elseif(n = 1638)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if((n > 1640 && n < 1644) || n = 1658 || n = 1659 || n = 1669 || n = 1675 || n = 1679)
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
.elseif(n = 1649 || n = 1670)
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
	; 9 bytes
	php
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
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
	ldy #166 ;hides 'ldx zp'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
	plp
.elseif(n = 1668 || n = 1678)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1671)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $B9A2'
	ldx #185
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
.endif
.else
; 1680 to 1718
.if(n = 1680)
	; 8 bytes
	php
	ldy #186 ;hides 'tsx'
	nop
	dey
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
.elseif(n = 1683 || n = 1685 || n = 1690 || n = 1692 || n = 1693 || n = 1697 || n = 1710 || n = 1711 || n = 1713)
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
.elseif(n = 1694 || n = 1701 || n = 1715)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
	plp
.elseif(n = 1696 || n = 1704 || n = 1712)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1698)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $BCA2'
	ldx #188
	dex
	Jne *-4
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
.elseif(n = 1706)
	; 9 bytes
	php
	ldy #170 ;hides 'tax'
	ldx $A6
	dey
	Jne *-4
	plp
.elseif(n = 1707)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jne *-4
	plp
.elseif(n = 1708)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $BDA2'
	ldx #189
	dex
	Jne *-4
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
.if(n = 1719 || n = 1733 || n = 1740 || n = 1747)
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
.elseif(n = 1721 || n = 1731)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 1743)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C1A2'
	ldx #193
	dex
	Jne *-4
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 1761 || n = 1769 || n = 1774 || n = 1777 || n = 1791 || n = 1794 || n = 1795)
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
.elseif(n = 1764)
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
	sta (@ptrtemp,x)
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
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 1785)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
	plp
.elseif(n = 1787)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 8
	Jne *-5
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
.if((n >= 1797) && (n < 1816))
; 1797 to 1815
.if(n = 1797)
	; 8 bytes
	php
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	plp
.elseif(n = 1798)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C7A2'
	ldx #199
	dex
	Jne *-4
	plp
.elseif(n = 1799 || n = 1808)
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
.elseif(n = 1802 || n = 1804 || n = 1810)
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
.elseif(n = 1815)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C9A2'
	ldx #201
	dex
	Jne *-4
	plp
.endif
.else
; 1816 to 1835
.if(n = 1816 || n = 1832)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1817 || n = 1826 || n = 1835)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 1818 || n = 1828)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1819 || n = 1822 || n = 1825 || n = 1827)
	; 9..10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1824)
	; 8 bytes
	php
	ldy #202 ;hides 'dex'
	nop
	dey
	Jne *-3
	plp
.elseif(n = 1829)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
	sta (@ptrtemp,x)
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
.elseif(n = 1837 || n = 1841 || n = 1847 || n = 1850 || n = 1855 || n = 1863 || n = 1873 || n = 1874)
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
	tay
	lax #7
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 1846)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	bit $24
	dex
	Jne *-4
	plp
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
	ldx #168 ;hides 'tay'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 1857)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	sta (@ptrtemp,x)
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
.elseif(n = 1866)
	; 9 bytes
	php
	ldy #186 ;hides 'tsx'
	ldx $A6
	dey
	Jne *-4
	plp
.elseif(n = 1867)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
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
	; 9 bytes
	php
	ldy #170 ;hides 'tax'
	delay_n 4
	dey
	Jne *-4
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
.elseif(n = 1878)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $D0A2'
	ldx #208
	dex
	Jne *-4
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
.elseif(n = 1881 || n = 1890 || n = 1891 || n = 1894)
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
.elseif(n = 1883 || n = 1885 || n = 1893 || n = 1895 || (n > 1900 && n < 1904) || n = 1913)
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 1908)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
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
.endif
.else
; 1915 to 1953
.if(n = 1915)
	; 9 bytes
	php
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	plp
.elseif((n .mod 9 = 8) && ((n > 1915 && n < 1953)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
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
.elseif(n = 1918 || n = 1938 || n = 1948)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1919 || n = 1937 || n = 1949)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 8 = 0) && ((n > 1919 && n < 1945)))
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
.elseif(n = 1922 || n = 1927 || n = 1930 || n = 1945 || n = 1946)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1926)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 1929)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
	sta (@ptrtemp,x)
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
.elseif(n = 1947)
	; 10 bytes
	php
	ldx #194 ;hides 'nop #imm'
	nop $FF44 ;hides 'nop zp'
	dex
	Jne *-5
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
.elseif(n = 1958)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 1962 || n = 1963 || n = 1971 || n = 1972 || n = 1981 || n = 1989 || n = 1990)
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
	ldx #164 ;hides 'ldy zp'
	ldy $EAEA ;hides two 'nop's
	dex
	Jne *-5
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
.elseif(n = 1978)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $DBA2'
	ldx #219
	dex
	Jne *-4
	plp
.elseif(n = 1983 || n = 1991)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.if(n = 1993 || n = 2001 || n = 2003 || n = 2029)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1994 || n = 2002 || n = 2009 || n = 2010 || n = 2017 || n = 2025)
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
	; 9 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	nop
	dey
	Jne *-4
	plp
.elseif(n = 2020)
	; 10 bytes
	php
	tsx
	ldy #(n-12)/8
	pha
	dey
	Jne *-2
	txs
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
.elseif(n = 2034 || n = 2035 || n = 2044 || n = 2053 || n = 2062 || n = 2070)
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
.elseif(n = 2037)
	; 10 bytes
	php
	ldy #203 ;hides 'sbx #imm'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	plp
.elseif(n = 2038)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2039 || n = 2045 || n = 2047 || n = 2055 || n = 2057)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2058)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	inx
	Jne *-4
	plp
.elseif(n = 2059)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	dex
	Jne *-4
	plp
.elseif(n = 2061)
	; 10 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-5
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
.elseif(n = 2066)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2067)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
	Jne *-4
	plp
.elseif(n = 2068)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $1BA2'
	ldx #27
	inx
	Jne *-4
	plp
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2074 || n = 2079 || n = 2089 || n = 2106 || n = 2107)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2072 || n = 2084 || n = 2108)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2073 || n = 2081 || n = 2090 || n = 2093 || n = 2099)
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
.elseif(n = 2078 || n = 2087 || n = 2096)
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
.elseif(n = 2082)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	inx
	Jne *-5
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
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2091)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	sta (@ptrtemp,x)
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
.elseif(n = 2095)
	; 9 bytes
	php
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	inx
	Jne *-4
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
.elseif(n = 2098 || n = 2109)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2100)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
	sta (@ptrtemp,x)
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
	; 9 bytes
	php
	tay
	lax #8
	jsr delay_256a_x_33_clocks
	tya
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
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2116 || n = 2124 || n = 2125 || n = 2133 || n = 2134 || n = 2143 || n = 2146)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2111 || n = 2117 || n = 2126 || n = 2135 || n = 2147)
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
.elseif(n = 2118 || n = 2128 || n = 2138)
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
.elseif(n = 2127)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
	sta (@ptrtemp,x)
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2145)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	sta (@ptrtemp,x)
	plp
.elseif(n = 2148)
	; 9 bytes
	php
	cmp #<$CD ;hides 'cmp $12A2'
	ldx #18
	inx
	Jne *-4
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
.elseif(n = 2151 || n = 2152 || n = 2160 || n = 2161 || n = 2169 || n = 2170 || n = 2179 || n = 2182 || n = 2187)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2153 || n = 2164)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 2155 || n = 2171 || n = 2172 || n = 2181 || n = 2183)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2158)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	dex
	Jne *-4
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
.elseif(n = 2165)
	; 10 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $A4A4 ;hides 'ldy zp'
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
.elseif(n = 2178)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
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
.if(n = 2188 || n = 2198 || n = 2218)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2189)
	; 10 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	ldx $A6A6 ;hides 'ldx zp'
	dey
	Jne *-5
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
.elseif(n = 2191 || n = 2201 || n = 2210 || n = 2225 || n = 2226)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2196 || n = 2199 || n = 2205 || n = 2209 || n = 2215 || n = 2223 || n = 2224)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2197)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2200)
	; 10 bytes
	php
	pha
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2202)
	; 8 bytes
	php
	ldx #244 ;hides 'nop zp,x'
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
.elseif(n = 2208 || n = 2219)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
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
.elseif((n > 2234 && n < 2238) || n = 2253)
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2243)
	; 10 bytes
	php
	ldx #160 ;hides 'ldy #imm'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 2244)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	sta (@ptrtemp,x)
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
.elseif(n = 2270 || (n > 2279 && n < 2283) || n = 2291)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2271)
	; 10 bytes
	php
	ldy #162 ;hides 'ldx #imm'
	ldx $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
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
.elseif(n = 2306)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	nop $04
	inx
	Jne *-4
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
.elseif(n = 2309 || n = 2315 || (n > 2318 && n < 2324) || n = 2325 || n = 2331 || n = 2334 || n = 2335 || n = 2341 || n = 2343)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2342)
	; 10 bytes
	php
	ldx #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344)
	; 10 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $EAEA ;hides two 'nop's
	dex
	Jne *-5
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
.elseif(n = 2350 || n = 2361 || n = 2364)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2351 || n = 2373)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif((n > 2351 && n < 2357) || n = 2363 || (n > 2364 && n < 2368) || (n > 2374 && n < 2378) || (n > 2378 && n < 2382))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2357)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $EAEA ;hides two 'nop's
	dex
	Jne *-5
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
	; 9 bytes
	php
	tay
	lax #9
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 2362)
	; 9 bytes
	php
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2368 || n = 2378)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
.elseif(n = 2387 || (n > 2388 && n < 2393) || n = 2397 || (n > 2398 && n < 2402) || n = 2403 || n = 2407 || n = 2409 || (n > 2410 && n < 2415) || n = 2419 || n = 2421)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2388 || n = 2398 || n = 2418)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
	ldy #200 ;hides 'iny'
	bit $24
	dey
	dey
	Jne *-5
	plp
.elseif(n = 2395 || n = 2406 || n = 2417)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 2410)
	; 10 bytes
	delay_n (n-2)
	nop
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
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2434 || n = 2446)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423 || n = 2425 || n = 2429 || n = 2431 || n = 2433 || n = 2435 || n = 2436 || n = 2440 || n = 2441 || n = 2443 || n = 2447 || n = 2451 || n = 2455 || n = 2457)
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
.elseif(n = 2428 || n = 2439 || n = 2450)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2430)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2432 || n = 2444 || n = 2456)
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
.elseif(n = 2438 || n = 2448 || n = 2458)
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
.elseif(n = 2460)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
	plp
.endif
.else
; 2461 to 2500
.if((n .mod 11 = 8) && ((n > 2460 && n < 2495)))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2462)
	; 10 bytes
	php
	ldx #164 ;hides 'ldy zp'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif((n > 2462 && n < 2466) || n = 2469 || (n > 2472 && n < 2478) || n = 2479 || n = 2484 || n = 2485 || n = 2487 || n = 2489 || n = 2491 || n = 2495 || n = 2496 || n = 2499)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2467)
	; 10 bytes
	php
	ldx #164 ;hides 'ldy zp'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 2468 || n = 2480 || n = 2492)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2470 || n = 2482)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2471)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2478 || n = 2488 || n = 2498)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
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
.elseif(n = 2493)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2497)
	; 10 bytes
	php
	ldy #166 ;hides 'ldx zp'
	delay_n 9
	dey
	Jne *-5
	plp
.elseif(n = 2500)
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
.endif
.endif
.else
.if((n >= 2501) && (n < 3751))
.if((n >= 2501) && (n < 3126))
.if((n >= 2501) && (n < 2813))
.if((n >= 2501) && (n < 2657))
.if((n >= 2501) && (n < 2579))
; 2501 to 2578
.if(n = 2501 || n = 2502 || n = 2507 || n = 2509 || n = 2511 || n = 2513 || n = 2517 || (n > 2518 && n < 2525) || n = 2529 || (n > 2530 && n < 2536) || n = 2539 || n = 2541 || (n > 2542 && n < 2547) || n = 2551 || n = 2553 || n = 2555 || n = 2557 || n = 2563 || n = 2565 || n = 2567 || n = 2572 || n = 2573 || n = 2575 || n = 2577)
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
.elseif((n .mod 12 = 8) && ((n > 2503 && n < 2577)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif((n .mod 11 = 8) && (n = 2505 || (n > 2526 && n < 2572)))
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2506)
	; 9 bytes
	php
	ldx #250 ;hides 'nop'
	nop $04
	dex
	Jne *-4
	plp
.elseif(n = 2508 || n = 2518 || n = 2548)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2510)
	; 10 bytes
	php
	tsx
	ldy #250 ;hides 'nop'
	pha
	dey
	Jne *-3
	txs
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
.elseif(n = 2514 || n = 2542 || n = 2556 || n = 2570)
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
.elseif(n = 2530 || n = 2554 || n = 2566 || n = 2578)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2568)
	; 10 bytes
	php
	ldx #0
	nop
	cmp $C5
	dex
	Jne *-4
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
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579 || n = 2583 || (n > 2584 && n < 2588) || n = 2589 || (n > 2593 && n < 2598) || n = 2599 || n = 2605 || n = 2607 || n = 2608 || n = 2611 || n = 2617)
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
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	inx
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
.elseif(n = 2584 || n = 2598)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2588 || n = 2600 || n = 2612)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2590 || n = 2614)
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
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
.elseif(n = 2613)
	; 9 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2616)
	; 9 bytes
	php
	tay
	lax #10
	jsr delay_256a_x_33_clocks
	tya
	plp
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2621 || n = 2638 || n = 2650)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2619)
	; 9 bytes
	php
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2620 || n = 2623 || (n > 2626 && n < 2632) || n = 2633 || n = 2634 || n = 2639 || (n > 2640 && n < 2645) || n = 2649 || (n > 2650 && n < 2654) || n = 2655)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2622)
	; 10 bytes
	php
	ldx #(n-6)/12 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
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
.elseif(n = 2640 || n = 2654)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
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
; 2657 to 2734
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
.elseif(n = 2661 || (n > 2662 && n < 2668) || n = 2673 || (n > 2674 && n < 2679) || n = 2683 || n = 2685 || n = 2687 || n = 2688 || n = 2693 || n = 2695 || n = 2697 || n = 2699 || n = 2700 || (n > 2703 && n < 2708) || n = 2709 || n = 2711 || (n > 2714 && n < 2719) || n = 2721 || (n > 2726 && n < 2731) || n = 2733)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2694)
	; 10 bytes
	php
	ldx #168 ;hides 'tay'
	delay_n 9
	dex
	Jne *-5
	plp
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
.elseif(n = 2719)
	; 10 bytes
	php
	ldx #(n-3)/14 ;hides 'nop #imm'
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2737 || n = 2739 || n = 2740 || n = 2748 || n = 2750 || n = 2751 || n = 2754 || n = 2759 || (n > 2762 && n < 2766) || n = 2771 || n = 2772)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2738 || n = 2752 || n = 2766)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
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
.elseif(n = 2746 || n = 2770)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2749)
	; 10 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
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
.elseif(n = 2760)
	; 10 bytes
	php
	ldx #(n-4)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 2761)
	; 10 bytes
	php
	ldx #(n-5)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
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
.elseif(n = 2776 || n = 2777 || n = 2781 || (n > 2782 && n < 2788) || n = 2793 || (n > 2794 && n < 2800) || n = 2803 || n = 2805 || n = 2807 || n = 2809 || n = 2810)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 2782)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2791 || n = 2802)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2794 || n = 2808)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
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
.elseif(n = 2815 || (n > 2818 && n < 2822) || n = 2825 || n = 2829 || (n > 2830 && n < 2834) || n = 2835 || n = 2839 || n = 2841 || n = 2843 || n = 2844 || n = 2846 || n = 2847 || n = 2849 || n = 2851 || n = 2853 || (n > 2854 && n < 2864) || n = 2865 || (n > 2866 && n < 2870) || n = 2871 || n = 2872 || n = 2874 || n = 2877 || n = 2879 || n = 2880 || (n > 2881 && n < 2886) || n = 2887 || n = 2889)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.elseif(n = 2818 || n = 2830 || n = 2842 || n = 2854 || n = 2866 || n = 2875 || n = 2878 || n = 2890)
	; 10 bytes
	delay_n (n-2)
	nop
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
	; 10 bytes
	php
	ldx #0
	delay_n 6
	dex
	Jne *-4
	plp
.elseif(n = 2826)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop $04
	inx
	Jne *-5
	plp
.elseif(n = 2827)
	; 10 bytes
	php
	ldx #68 ;hides 'nop zp'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 2834)
	; 9 bytes
	php
	ldy #202 ;hides 'dex'
	delay_n 7
	dey
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
.elseif(n = 2845)
	; 10 bytes
	php
	ldy #203 ;hides 'sbx #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
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
.elseif(n = 2873)
	; 9 bytes
	php
	tay
	lax #11
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 2876)
	; 9 bytes
	php
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2881)
	; 10 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
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
.endif
.else
; 2891 to 2968
.if(n = 2891 || (n > 2892 && n < 2900) || n = 2901 || (n > 2902 && n < 2906) || (n > 2906 && n < 2912) || (n > 2914 && n < 2918) || n = 2919 || n = 2921 || n = 2923 || n = 2925 || (n > 2926 && n < 2932) || n = 2935 || n = 2937 || (n > 2938 && n < 2942) || (n > 2942 && n < 2946) || n = 2949 || (n > 2950 && n < 2957) || n = 2958 || n = 2959 || n = 2961 || (n > 2962 && n < 2969))
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
.elseif(n = 2913)
	; 10 bytes
	php
	ldy #182 ;hides 'ldx zp,y'
	ldx $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
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
.elseif(n = 2933)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
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
.elseif(n = 2946)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop $04
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
.endif
.endif
.else
; 2969 to 3125
.if((n > 2968 && n < 2972) || n = 2973 || n = 2975 || (n > 2977 && n < 2982) || n = 2983 || n = 2985 || (n > 2986 && n < 2990) || (n > 2990 && n < 2996) || n = 2997 || (n > 2998 && n < 3004) || n = 3005 || n = 3007 || (n > 3010 && n < 3018) || n = 3019 || n = 3021 || (n > 3022 && n < 3030) || n = 3031 || n = 3033 || (n > 3034 && n < 3044) || n = 3045 || n = 3047 || n = 3049 || n = 3050 || (n > 3051 && n < 3056) || n = 3057 || n = 3059 || (n > 3061 && n < 3068) || n = 3069 || n = 3071 || (n > 3074 && n < 3080) || n = 3081 || (n > 3082 && n < 3087) || (n > 3088 && n < 3102) || (n > 3102 && n < 3116) || (n > 3116 && n < 3126))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif((n .mod 12 = 8) && ((n > 2971 && n < 3069)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif((n .mod 12 = 10) && ((n > 2973 && n < 3035) || n = 3070 || n = 3082))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 8) && ((n > 2975 && n < 3019) || (n > 3045 && n < 3117)))
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
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 3006)
	; 10 bytes
	php
	ldx #(n-6)/12 ;hides 'nop'
	nop
	nop $04
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
.elseif(n = 3058)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
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
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
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
.elseif(n = 3087)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if((n > 3125 && n < 3130) || n = 3131 || n = 3134 || (n > 3135 && n < 3139) || (n > 3139 && n < 3144) || (n > 3144 && n < 3158) || (n > 3158 && n < 3165) || n = 3166 || (n > 3167 && n < 3172) || n = 3173 || n = 3175 || (n > 3178 && n < 3186) || (n > 3186 && n < 3190) || (n > 3191 && n < 3200) || (n > 3200 && n < 3206) || (n > 3206 && n < 3214) || (n > 3214 && n < 3226) || n = 3227 || n = 3229 || (n > 3230 && n < 3238) || (n > 3238 && n < 3242) || (n > 3242 && n < 3254) || n = 3255 || (n > 3256 && n < 3265) || (n > 3265 && n < 3270) || (n > 3270 && n < 3282))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3130)
	; 9 bytes
	php
	tay
	lax #12
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 3132 || n = 3135)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3133)
	; 9 bytes
	php
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3139)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif((n .mod 14 = 8) && ((n > 3143 && n < 3271)))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
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
.elseif(n = 3167)
	; 10 bytes
	php
	ldx #(n-3)/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
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
	ldx #(n-4)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 3177)
	; 10 bytes
	php
	ldx #(n-5)/13 ;hides 'nop zp,x'
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
.elseif(n = 3190)
	; 10 bytes
	php
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
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
.elseif(n = 3206)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
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
.elseif(n = 3265)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
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
.elseif(n = 3283 || n = 3285 || (n > 3286 && n < 3292) || (n > 3292 && n < 3298) || (n > 3298 && n < 3302) || (n > 3302 && n < 3307) || (n > 3307 && n < 3312) || (n > 3312 && n < 3322) || (n > 3322 && n < 3326) || (n > 3326 && n < 3340) || (n > 3340 && n < 3354) || (n > 3354 && n < 3368) || (n > 3368 && n < 3382) || (n > 3382 && n < 3387) || n = 3388 || n = 3391 || n = 3394 || n = 3395 || n = 3397 || (n > 3398 && n < 3409) || n = 3411 || n = 3413 || (n > 3414 && n < 3422) || n = 3423 || n = 3425 || n = 3426 || n = 3428 || n = 3429 || (n > 3430 && n < 3438))
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
.elseif(n = 3292)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
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
.elseif(n = 3387)
	; 9 bytes
	php
	tay
	lax #13
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 3389 || n = 3392)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3390)
	; 9 bytes
	php
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	plp
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
.endif
.endif
.else
; 3438 to 3750
.if((n .mod 14 = 8) && ((n > 3437 && n < 3579)))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif((n > 3438 && n < 3442) || (n > 3442 && n < 3452) || (n > 3452 && n < 3462) || (n > 3462 && n < 3466) || (n > 3466 && n < 3478) || n = 3479 || (n > 3480 && n < 3494) || (n > 3494 && n < 3506) || n = 3507 || (n > 3508 && n < 3522) || (n > 3522 && n < 3536) || (n > 3536 && n < 3550) || (n > 3550 && n < 3564) || (n > 3564 && n < 3578) || (n > 3578 && n < 3592) || (n > 3592 && n < 3644) || n = 3645 || n = 3648 || (n > 3649 && n < 3686) || (n > 3686 && n < 3702) || (n > 3702 && n < 3718) || (n > 3718 && n < 3750))
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
.elseif(n = 3506)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
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
.elseif(n = 3644)
	; 9 bytes
	php
	tay
	lax #14
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 3646 || n = 3649)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3647)
	; 9 bytes
	php
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
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
.else
; 3751 to 5000
.if((n > 3750 && n < 3766) || (n > 3766 && n < 3777) || (n > 3777 && n < 3782) || (n > 3782 && n < 3798) || (n > 3798 && n < 3863) || (n > 3863 && n < 3894) || (n > 3894 && n < 3901) || n = 3902 || (n > 3906 && n < 3910) || (n > 3910 && n < 3926) || (n > 3926 && n < 3974) || (n > 3974 && n < 4006) || (n > 4006 && n < 4158) || n = 4159 || n = 4162 || (n > 4163 && n < 4415) || n = 4416 || n = 4419 || (n > 4420 && n < 4672) || n = 4673 || n = 4676 || (n > 4677 && n < 4929) || n = 4930 || n = 4933 || (n > 4934 && n < 5001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
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
.elseif(n = 3777)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 3782)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	delay_n 9
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
.elseif(n = 3901)
	; 9 bytes
	php
	tay
	lax #15
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 3903 || n = 3906 || n = 4160 || n = 4163 || n = 4417 || n = 4420 || n = 4674 || n = 4677 || n = 4931 || n = 4934)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4158)
	; 9 bytes
	php
	tay
	lax #16
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 4161)
	; 9 bytes
	php
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4415)
	; 9 bytes
	php
	tay
	lax #17
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 4418)
	; 9 bytes
	php
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4672)
	; 9 bytes
	php
	tay
	lax #18
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 4675)
	; 9 bytes
	php
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4929)
	; 9 bytes
	php
	tay
	lax #19
	jsr delay_256a_x_33_clocks
	tya
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
.else
.if((n >= 5001) && (n < 7501))
; 5001 to 7500
.if((n > 5000 && n < 5186) || n = 5187 || n = 5190 || (n > 5191 && n < 5443) || n = 5444 || n = 5447 || (n > 5448 && n < 5700) || n = 5701 || n = 5704 || (n > 5705 && n < 5957) || n = 5958 || n = 5961 || (n > 5962 && n < 6214) || n = 6215 || n = 6218 || (n > 6219 && n < 6471) || n = 6472 || n = 6475 || (n > 6476 && n < 6728) || n = 6729 || n = 6732 || (n > 6733 && n < 6985) || n = 6986 || n = 6989 || (n > 6990 && n < 7242) || n = 7243 || n = 7246 || (n > 7247 && n < 7499) || n = 7500)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5186)
	; 9 bytes
	php
	tay
	lax #20
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 5188 || n = 5191 || n = 5445 || n = 5448 || n = 5702 || n = 5705 || n = 5959 || n = 5962 || n = 6216 || n = 6219 || n = 6473 || n = 6476 || n = 6730 || n = 6733 || n = 6987 || n = 6990 || n = 7244 || n = 7247)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5189)
	; 9 bytes
	php
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5443)
	; 9 bytes
	php
	tay
	lax #21
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 5446)
	; 9 bytes
	php
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5700)
	; 9 bytes
	php
	tay
	lax #22
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 5703)
	; 9 bytes
	php
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5957)
	; 9 bytes
	php
	tay
	lax #23
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 5960)
	; 9 bytes
	php
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6214)
	; 9 bytes
	php
	tay
	lax #24
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 6217)
	; 9 bytes
	php
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6471)
	; 9 bytes
	php
	tay
	lax #25
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 6474)
	; 9 bytes
	php
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6728)
	; 9 bytes
	php
	tay
	lax #26
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 6731)
	; 9 bytes
	php
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6985)
	; 9 bytes
	php
	tay
	lax #27
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 6988)
	; 9 bytes
	php
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7242)
	; 9 bytes
	php
	tay
	lax #28
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 7245)
	; 9 bytes
	php
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7499)
	; 9 bytes
	php
	tay
	lax #29
	jsr delay_256a_x_33_clocks
	tya
	plp
.endif
.else
; 7501 to 10000
.if(n = 7501 || n = 7504 || n = 7758 || n = 7761 || n = 8015 || n = 8018 || n = 8272 || n = 8275 || n = 8529 || n = 8532 || n = 8786 || n = 8789 || n = 9043 || n = 9046 || n = 9300 || n = 9303 || n = 9557 || n = 9560 || n = 9814 || n = 9817)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7502)
	; 9 bytes
	php
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7503 || (n > 7504 && n < 7756) || n = 7757 || n = 7760 || (n > 7761 && n < 8013) || n = 8014 || n = 8017 || (n > 8018 && n < 8270) || n = 8271 || n = 8274 || (n > 8275 && n < 8527) || n = 8528 || n = 8531 || (n > 8532 && n < 8784) || n = 8785 || n = 8788 || (n > 8789 && n < 9041) || n = 9042 || n = 9045 || (n > 9046 && n < 9298) || n = 9299 || n = 9302 || (n > 9303 && n < 9555) || n = 9556 || n = 9559 || (n > 9560 && n < 9812) || n = 9813 || n = 9816 || (n > 9817 && n < 10001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7756)
	; 9 bytes
	php
	tay
	lax #30
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 7759)
	; 9 bytes
	php
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8013)
	; 9 bytes
	php
	tay
	lax #31
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 8016)
	; 9 bytes
	php
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8270)
	; 9 bytes
	php
	tay
	lax #32
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 8273)
	; 9 bytes
	php
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8527)
	; 9 bytes
	php
	tay
	lax #33
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 8530)
	; 9 bytes
	php
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8784)
	; 9 bytes
	php
	tay
	lax #34
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 8787)
	; 9 bytes
	php
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9041)
	; 9 bytes
	php
	tay
	lax #35
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 9044)
	; 9 bytes
	php
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9298)
	; 9 bytes
	php
	tay
	lax #36
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 9301)
	; 9 bytes
	php
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9555)
	; 9 bytes
	php
	tay
	lax #37
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 9558)
	; 9 bytes
	php
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9812)
	; 9 bytes
	php
	tay
	lax #38
	jsr delay_256a_x_33_clocks
	tya
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
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
; 10001 to 12500
.if((n > 10000 && n < 10069) || n = 10070 || n = 10073 || (n > 10074 && n < 10326) || n = 10327 || n = 10330 || (n > 10331 && n < 10583) || n = 10584 || n = 10587 || (n > 10588 && n < 10840) || n = 10841 || n = 10844 || (n > 10845 && n < 11097) || n = 11098 || n = 11101 || (n > 11102 && n < 11354) || n = 11355 || n = 11358 || (n > 11359 && n < 11611) || n = 11612 || n = 11615 || (n > 11616 && n < 11868) || n = 11869 || n = 11872 || (n > 11873 && n < 12125) || n = 12126 || n = 12129 || (n > 12130 && n < 12382) || n = 12383 || n = 12386 || (n > 12387 && n < 12501))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10069)
	; 9 bytes
	php
	tay
	lax #39
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 10071 || n = 10074 || n = 10328 || n = 10331 || n = 10585 || n = 10588 || n = 10842 || n = 10845 || n = 11099 || n = 11102 || n = 11356 || n = 11359 || n = 11613 || n = 11616 || n = 11870 || n = 11873 || n = 12127 || n = 12130 || n = 12384 || n = 12387)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10072)
	; 9 bytes
	php
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10326)
	; 9 bytes
	php
	tay
	lax #40
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 10329)
	; 9 bytes
	php
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10583)
	; 9 bytes
	php
	tay
	lax #41
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 10586)
	; 9 bytes
	php
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10840)
	; 9 bytes
	php
	tay
	lax #42
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 10843)
	; 9 bytes
	php
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11097)
	; 9 bytes
	php
	tay
	lax #43
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 11100)
	; 9 bytes
	php
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11354)
	; 9 bytes
	php
	tay
	lax #44
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 11357)
	; 9 bytes
	php
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11611)
	; 9 bytes
	php
	tay
	lax #45
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 11614)
	; 9 bytes
	php
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11868)
	; 9 bytes
	php
	tay
	lax #46
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 11871)
	; 9 bytes
	php
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12125)
	; 9 bytes
	php
	tay
	lax #47
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 12128)
	; 9 bytes
	php
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12382)
	; 9 bytes
	php
	tay
	lax #48
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 12385)
	; 9 bytes
	php
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.else
; 12501 to 15000
.if((n > 12500 && n < 12639) || n = 12640 || n = 12643 || (n > 12644 && n < 12896) || n = 12897 || n = 12900 || (n > 12901 && n < 13153) || n = 13154 || n = 13157 || (n > 13158 && n < 13410) || n = 13411 || n = 13414 || (n > 13415 && n < 13667) || n = 13668 || n = 13671 || (n > 13672 && n < 13924) || n = 13925 || n = 13928 || (n > 13929 && n < 14181) || n = 14182 || n = 14185 || (n > 14186 && n < 14438) || n = 14439 || n = 14442 || (n > 14443 && n < 14695) || n = 14696 || n = 14699 || (n > 14700 && n < 14952) || n = 14953 || n = 14956 || (n > 14957 && n < 15001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12639)
	; 9 bytes
	php
	tay
	lax #49
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 12641 || n = 12644 || n = 12898 || n = 12901 || n = 13155 || n = 13158 || n = 13412 || n = 13415 || n = 13669 || n = 13672 || n = 13926 || n = 13929 || n = 14183 || n = 14186 || n = 14440 || n = 14443 || n = 14697 || n = 14700 || n = 14954 || n = 14957)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12642)
	; 9 bytes
	php
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12896)
	; 9 bytes
	php
	tay
	lax #50
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 12899)
	; 9 bytes
	php
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13153)
	; 9 bytes
	php
	tay
	lax #51
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 13156)
	; 9 bytes
	php
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13410)
	; 9 bytes
	php
	tay
	lax #52
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 13413)
	; 9 bytes
	php
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13667)
	; 9 bytes
	php
	tay
	lax #53
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 13670)
	; 9 bytes
	php
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13924)
	; 9 bytes
	php
	tay
	lax #54
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 13927)
	; 9 bytes
	php
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14181)
	; 9 bytes
	php
	tay
	lax #55
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 14184)
	; 9 bytes
	php
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14438)
	; 9 bytes
	php
	tay
	lax #56
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 14441)
	; 9 bytes
	php
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14695)
	; 9 bytes
	php
	tay
	lax #57
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 14698)
	; 9 bytes
	php
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14952)
	; 9 bytes
	php
	tay
	lax #58
	jsr delay_256a_x_33_clocks
	tya
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
.endif
.else
.if((n >= 15001) && (n < 17501))
; 15001 to 17500
.if((n > 15000 && n < 15209) || n = 15210 || n = 15213 || (n > 15214 && n < 15466) || n = 15467 || n = 15470 || (n > 15471 && n < 15723) || n = 15724 || n = 15727 || (n > 15728 && n < 15980) || n = 15981 || n = 15984 || (n > 15985 && n < 16237) || n = 16238 || n = 16241 || (n > 16242 && n < 16494) || n = 16495 || n = 16498 || (n > 16499 && n < 16751) || n = 16752 || n = 16755 || (n > 16756 && n < 17008) || n = 17009 || n = 17012 || (n > 17013 && n < 17265) || n = 17266 || n = 17269 || (n > 17270 && n < 17501))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15209)
	; 9 bytes
	php
	tay
	lax #59
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 15211 || n = 15214 || n = 15468 || n = 15471 || n = 15725 || n = 15728 || n = 15982 || n = 15985 || n = 16239 || n = 16242 || n = 16496 || n = 16499 || n = 16753 || n = 16756 || n = 17010 || n = 17013 || n = 17267 || n = 17270)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15212)
	; 9 bytes
	php
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15466)
	; 9 bytes
	php
	tay
	lax #60
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 15469)
	; 9 bytes
	php
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15723)
	; 9 bytes
	php
	tay
	lax #61
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 15726)
	; 9 bytes
	php
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15980)
	; 9 bytes
	php
	tay
	lax #62
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 15983)
	; 9 bytes
	php
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16237)
	; 9 bytes
	php
	tay
	lax #63
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 16240)
	; 9 bytes
	php
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16494)
	; 9 bytes
	php
	tay
	lax #64
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 16497)
	; 9 bytes
	php
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16751)
	; 9 bytes
	php
	tay
	lax #65
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 16754)
	; 9 bytes
	php
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17008)
	; 9 bytes
	php
	tay
	lax #66
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 17011)
	; 9 bytes
	php
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17265)
	; 9 bytes
	php
	tay
	lax #67
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 17268)
	; 9 bytes
	php
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.else
; 17501 to 20000
.if((n > 17500 && n < 17522) || n = 17523 || n = 17526 || (n > 17527 && n < 17779) || n = 17780 || n = 17783 || (n > 17784 && n < 18036) || n = 18037 || n = 18040 || (n > 18041 && n < 18293) || n = 18294 || n = 18297 || (n > 18298 && n < 18550) || n = 18551 || n = 18554 || (n > 18555 && n < 18807) || n = 18808 || n = 18811 || (n > 18812 && n < 19064) || n = 19065 || n = 19068 || (n > 19069 && n < 19321) || n = 19322 || n = 19325 || (n > 19326 && n < 19578) || n = 19579 || n = 19582 || (n > 19583 && n < 19835) || n = 19836 || n = 19839 || (n > 19840 && n < 20001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17522)
	; 9 bytes
	php
	tay
	lax #68
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 17524 || n = 17527 || n = 17781 || n = 17784 || n = 18038 || n = 18041 || n = 18295 || n = 18298 || n = 18552 || n = 18555 || n = 18809 || n = 18812 || n = 19066 || n = 19069 || n = 19323 || n = 19326 || n = 19580 || n = 19583 || n = 19837 || n = 19840)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17525)
	; 9 bytes
	php
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17779)
	; 9 bytes
	php
	tay
	lax #69
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 17782)
	; 9 bytes
	php
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18036)
	; 9 bytes
	php
	tay
	lax #70
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 18039)
	; 9 bytes
	php
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18293)
	; 9 bytes
	php
	tay
	lax #71
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 18296)
	; 9 bytes
	php
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18550)
	; 9 bytes
	php
	tay
	lax #72
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 18553)
	; 9 bytes
	php
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18807)
	; 9 bytes
	php
	tay
	lax #73
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 18810)
	; 9 bytes
	php
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19064)
	; 9 bytes
	php
	tay
	lax #74
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 19067)
	; 9 bytes
	php
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19321)
	; 9 bytes
	php
	tay
	lax #75
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 19324)
	; 9 bytes
	php
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19578)
	; 9 bytes
	php
	tay
	lax #76
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 19581)
	; 9 bytes
	php
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19835)
	; 9 bytes
	php
	tay
	lax #77
	jsr delay_256a_x_33_clocks
	tya
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

.endif
	rest
.endmacro
