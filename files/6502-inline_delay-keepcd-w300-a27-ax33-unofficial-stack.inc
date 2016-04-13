; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, Y, S, Z+N, and V
; Will preserve C, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 12
; - 4 bytes or less when n <= 19
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 41
; - 7 bytes or less when n <= 1301
; - 8 bytes or less when n <= 2359
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=3, avg=3.7, max=5
; - When 21 <= n <= 30: min=5, avg=5.6, max=6
; - When 31 <= n <= 50: min=4, avg=5.8, max=7
; - When 51 <= n <= 75: min=5, avg=5.7, max=7
; - When 76 <= n <= 100: min=5, avg=5.8, max=7
; - When 101 <= n <= 200: min=4, avg=5.8, max=7
; - When 201 <= n <= 300: min=4, avg=5.8, max=7
; - When 301 <= n <= 500: min=4, avg=5.8, max=7
; - When 501 <= n <= 750: min=4, avg=5.8, max=7
; - When 751 <= n <= 1000: min=5, avg=5.9, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.4, max=8
; - When 2001 <= n <= 3000: min=6, avg=7.4, max=9
; - When 3001 <= n <= 5000: min=7, avg=8.6, max=9
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
	; 1 bytes
	pha
.elseif(n = 4)
	; 1 bytes
	pla
.elseif(n = 5 || n = 16 || n = 17 || n = 28 || n = 33 || n = 38)
	; 2..6 bytes
	delay_n (n-2)
	nop
.elseif(n = 6 || n = 11)
	; 2..3 bytes
	delay_n (n-3)
	pha
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 2 bytes
	delay_n (n-4)
	pla
.elseif(n = 9 || n = 10 || n = 13 || n = 22)
	; 3..5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 19 || n = 23 || n = 27 || n = 30)
	; 4..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 20)
	; 5 bytes
	.repeat 5
	 pla
	.endrepeat
.elseif(n = 21 || n = 26 || n = 36)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 25)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
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
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	dex
	Jne *-3
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
.elseif(n = 37)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 39)
	; 6 bytes
	pha
	delay_n (n-3)
.endif
.else
; 40 to 78
.if(n = 40)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	dex
	Jne *-3
.elseif(n = 41 || n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 42 || n = 47 || n = 67 || n = 77)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
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
.elseif(n = 45 || n = 53 || n = 60 || n = 70 || n = 75)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 48 || n = 58 || n = 63 || n = 68 || n = 73)
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
.elseif(n = 52 || n = 54 || n = 59 || n = 72 || n = 74)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 55)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 62)
	; 5 bytes
	ldx #136 ;hides 'dey'
	dex
	Jmi *-2
.elseif(n = 65)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
	; 5 bytes
	ldy #136 ;hides 'dey'
	pha
	Jmi *-2
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
.elseif(n = 82 || n = 100)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif((n .mod 5 = 3) && ((n > 82 && n < 109) || n = 118 || n = 123 || (n > 132 && n < 144)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 84 || n = 94 || n = 104 || n = 109 || n = 114 || n = 127 || n = 139 || n = 144 || n = 149 || n = 154)
	; 5..6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 87 || n = 102 || n = 112 || n = 117 || n = 122 || n = 142 || n = 147 || n = 152)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 89)
	; 6 bytes
	ldy #136 ;hides 'dey'
	nop
	nop
	Jmi *-3
.elseif(n = 90 || n = 95 || n = 110 || n = 115 || n = 128 || n = 135 || n = 140 || n = 150)
	; 5..6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 105 || n = 137 || n = 145)
	; 6 bytes
	ldx #(n-9)/8
	pha
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
.elseif(n = 130)
	; 6 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 132)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
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
.if(n = 157 || n = 160 || n = 165 || n = 175 || n = 180 || n = 192 || n = 195 || n = 200 || n = 205 || n = 210 || n = 220 || n = 230)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 3) && ((n > 157 && n < 179) || (n > 192 && n < 214) || (n > 222 && n < 234)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 184 || n = 189 || n = 194 || n = 202 || n = 214 || n = 219 || n = 229 || n = 234)
	; 6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 172)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 177 || n = 185 || n = 217)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 179)
	; 6 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 182 || n = 187 || n = 207 || n = 212 || n = 222 || n = 227)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 188)
	; 5 bytes
	ldx #26 ;hides 'nop'
	dex
	Jpl *-2
.elseif(n = 199)
	; 5 bytes
	ldx #232 ;hides 'inx'
	pha
	Jne *-2
.elseif(n = 209)
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
.endif
.else
; 235 to 312
.if(n = 235 || n = 262 || n = 307)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 236 || n = 246 || n = 261 || n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 237 || n = 247 || n = 252 || n = 272 || n = 277 || n = 282)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif((n .mod 5 = 3) && ((n > 237 && n < 249) || (n > 257 && n < 284) || (n > 292 && n < 309)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 239)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 240 || n = 245 || n = 250 || n = 255 || n = 270 || n = 275 || n = 285 || n = 288 || n = 290 || n = 310)
	; 5..6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 244 || n = 254 || n = 259 || n = 264 || n = 279 || n = 287 || n = 294)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 249 || n = 257)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 289 || n = 297 || n = 305)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 292)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
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
.if(n = 313 || n = 318 || n = 322 || n = 328 || n = 333 || n = 338 || n = 343 || n = 348 || n = 353 || n = 363 || n = 367 || n = 368 || n = 373 || n = 378 || n = 388)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	iny
	iny
	Jne *-3
.elseif(n = 315 || n = 335 || n = 340 || n = 350 || n = 355 || n = 360 || n = 375 || n = 380 || n = 383 || n = 390)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 387)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 327 || n = 347 || n = 357 || n = 362)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 319 || n = 324 || n = 334 || n = 339 || n = 349 || n = 354 || n = 359 || n = 364 || n = 374 || n = 382 || n = 384 || n = 389)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 320)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jmi *-3
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
.elseif(n = 325 || n = 352 || n = 370)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 329 || n = 345 || n = 369)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 332)
	; 6 bytes
	ldx #200 ;hides 'iny'
	dex
	dex
	Jmi *-3
.elseif(n = 337 || n = 358 || n = 372)
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
.elseif(n = 377)
	; 5 bytes
	ldy #202 ;hides 'dex'
	iny
	Jne *-2
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 385)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 387)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
.endif
.else
; 391 to 468
.if(n = 391 || n = 396 || n = 401 || n = 411 || n = 426 || n = 441 || n = 451 || n = 456 || n = 461)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 392 || n = 427 || n = 447 || n = 452 || n = 462)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 393 || n = 449)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 4) && (n = 394 || n = 399 || (n > 418 && n < 445) || n = 454 || n = 459))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 395)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jpl *-3
.elseif(n = 397 || n = 415)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
.elseif(n = 402 || n = 410 || n = 420 || n = 430 || n = 440 || n = 445 || n = 450)
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 409 || n = 425)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 412)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jmi *-2
.elseif(n = 417 || n = 457 || n = 465)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
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
.elseif(n = 455)
	; 5 bytes
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
.elseif(n = 460)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.if(n = 469 || n = 474 || n = 479 || n = 492 || n = 499 || n = 504 || n = 509 || n = 514 || n = 527 || n = 534)
	; 6 bytes
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
	ldx #69 ;hides 'eor zp'
	nop
	inx
	Jpl *-3
.elseif(n = 473 || n = 478 || n = 482 || n = 483 || n = 488 || n = 493 || n = 508 || n = 513 || n = 518 || n = 523 || n = 528 || n = 538 || n = 542 || n = 543)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 475 || n = 490 || n = 495 || n = 500 || n = 507 || n = 515 || n = 520 || n = 525 || n = 535)
	; 6 bytes
	pla
	delay_n (n-4)
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 489)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	iny
	Jne *-2
.elseif(n = 494)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 497 || n = 529 || n = 545)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 502)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 522)
	; 7 bytes
	bit <$2C ;hides 'bit $B9A2'
	ldx #185
	dex
	Jmi *-4
.elseif(n = 524)
	; 5 bytes
	ldy #202 ;hides 'dex'
	dey
	Jmi *-2
.elseif(n = 530)
	; 6 bytes
	ldx #138 ;hides 'txa'
	inx
	inx
	Jne *-3
.elseif(n = 532)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 537)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 539)
	; 6 bytes
	ldx #136 ;hides 'dey'
	inx
	inx
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
.elseif(n = 548 || n = 553 || n = 558 || n = 562 || n = 563 || n = 573 || n = 578 || n = 583 || n = 587 || n = 588 || n = 593 || n = 598 || n = 608 || n = 612 || n = 613 || n = 618 || n = 623)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 549 || n = 559 || n = 564 || n = 572 || n = 574 || n = 579 || n = 584 || n = 594 || n = 599 || n = 604 || n = 614 || n = 619)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 550 || n = 595 || n = 622)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
.elseif(n = 555 || n = 565 || n = 570 || n = 580 || n = 590 || n = 600 || n = 605)
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 567)
	; 7 bytes
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	inx
	Jne *-4
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
.elseif(n = 592)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	dex
	Jpl *-4
.elseif(n = 597)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 615)
	; 5 bytes
	ldx #168 ;hides 'tay'
	inx
	Jne *-2
.elseif(n = 620)
	; 6 bytes
	ldx #138 ;hides 'txa'
	dex
	dex
	Jne *-3
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
.elseif(n = 627 || n = 628 || n = 633 || n = 643 || n = 648 || n = 653 || n = 658 || n = 663 || n = 668 || n = 678 || n = 682 || n = 683 || n = 693 || n = 698 || n = 703)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 5 bytes
	ldx #90 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 630 || n = 635 || n = 650 || n = 655 || n = 660 || n = 670 || n = 675 || n = 688 || n = 690 || n = 695 || n = 700)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 632 || n = 634 || n = 639 || n = 644 || n = 654 || n = 662 || n = 664 || n = 669 || n = 679 || n = 687 || n = 699)
	; 5..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 637)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 638 || n = 645 || n = 673 || n = 680 || n = 694)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 640)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 642 || n = 677)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 649 || n = 689 || n = 697)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 652 || n = 659)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 665)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 667 || n = 685)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 672)
	; 7 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-3
.elseif(n = 674)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	inx
	Jne *-3
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
.elseif(n = 702)
	; 7 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	inx
	Jne *-4
.endif
.else
; 704 to 781
.if(n = 704)
	; 6 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	iny
	Jne *-3
.elseif(n = 705 || n = 737 || n = 769 || n = 777)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 705 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 707 || n = 718 || n = 723 || n = 728 || n = 733 || n = 738 || n = 753 || n = 758 || n = 763 || n = 768 || n = 773)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 709 || n = 714 || n = 719 || n = 724 || n = 730 || n = 739 || n = 749 || n = 754 || n = 759 || n = 772 || n = 774 || n = 779)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 710 || n = 717 || n = 725 || n = 735 || n = 740 || n = 752 || n = 760 || n = 765 || n = 770 || n = 780)
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 732 || n = 742 || n = 762 || n = 767)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 734)
	; 5 bytes
	ldy #232 ;hides 'inx'
	dey
	Jmi *-2
.elseif(n = 744)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
.elseif(n = 745)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 775)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.endif
.endif
.else
.if((n >= 782) && (n < 860))
; 782 to 859
.if(n = 782)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 783 || n = 787 || n = 788 || n = 793 || n = 798 || n = 803 || n = 808 || n = 818 || n = 822 || n = 823 || n = 828 || n = 833 || n = 838 || n = 843 || n = 858)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 784 || n = 789 || n = 794 || n = 804 || n = 814 || n = 819 || n = 824 || n = 829 || n = 842 || n = 844 || n = 854 || n = 859)
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
.elseif(n = 790 || n = 795 || n = 805 || n = 810 || n = 815 || n = 830 || n = 840 || n = 845 || n = 850)
	; 6 bytes
	pla
	delay_n (n-4)
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
	ldx #100 ;hides 'nop zp'
	nop
	dex
	Jne *-3
.elseif(n = 802 || n = 847)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 807 || n = 852)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 809)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 812)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	iny
	Jne *-4
.elseif(n = 817 || n = 849 || n = 857)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
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
.elseif(n = 835)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jmi *-3
.elseif(n = 837)
	; 7 bytes
	bit <$2C ;hides 'bit $A3A2'
	ldx #163
	inx
	Jne *-4
.elseif(n = 839)
	; 5 bytes
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
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
.elseif(n = 863 || n = 868 || n = 873 || n = 878 || n = 888 || n = 893 || n = 898 || n = 903 || n = 908 || n = 913 || n = 923 || n = 927 || n = 928 || n = 933)
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
.elseif(n = 870 || n = 875 || n = 880 || n = 885 || n = 895 || n = 900 || n = 915 || n = 920 || n = 930)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 872)
	; 7 bytes
	ldx #<$2C ;hides 'bit $61A2'
	ldx #97
	dex
	Jne *-4
.elseif(n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 882)
	; 7 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jpl *-4
.elseif(n = 887 || n = 902 || n = 907)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 892 || n = 910)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if(n = 938 || n = 943 || n = 948 || n = 963 || n = 968 || n = 973 || n = 978 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1003 || n = 1008 || n = 1013)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 950 || n = 970 || n = 975 || n = 990 || n = 1005 || n = 1010 || n = 1015)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1012)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942 || n = 952 || n = 957 || n = 987 || n = 1012)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 944)
	; 6 bytes
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
.elseif(n = 945)
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
.elseif((n .mod 5 = 4) && ((n > 948 && n < 965) || n = 979 || (n > 988 && n < 1005)))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 955 || n = 982 || n = 1000)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
.elseif(n = 965)
	; 5 bytes
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
.elseif(n = 969 || n = 977 || n = 985)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 972)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 980)
	; 6 bytes
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
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
.endif
.else
; 1016 to 1093
.if((n .mod 5 = 1) && ((n > 1015 && n < 1092)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1018 || n = 1035 || n = 1040 || n = 1050 || n = 1055 || n = 1060 || n = 1067 || n = 1075 || n = 1080 || n = 1085)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1020 || n = 1042 || n = 1047 || n = 1077 || n = 1087 || n = 1092)
	; 6..7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1022)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 7 = 1) && ((n > 1022 && n < 1045) || (n > 1057 && n < 1080) || n = 1093))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1025 || n = 1049 || n = 1057 || n = 1089)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1027 || n = 1045 || n = 1090)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 1028 || n = 1032 || n = 1033 || n = 1038 || n = 1043 || n = 1048 || n = 1053 || n = 1068 || n = 1073 || n = 1078 || n = 1083 || n = 1088)
	; 6..7 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1062)
	; 7 bytes
	bit <$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jpl *-4
.elseif(n = 1063)
	; 5 bytes
	ldx #152 ;hides 'tya'
	dex
	Jne *-2
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
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1174 || n = 1179 || n = 1192 || n = 1194 || n = 1199 || n = 1204 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1095)
	; 5 bytes
	ldy #136 ;hides 'dey'
	pha
	Jne *-2
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
.elseif(n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1113 || n = 1118 || n = 1123 || n = 1133 || n = 1138 || n = 1143 || n = 1148 || n = 1153 || n = 1158 || n = 1168 || n = 1172 || n = 1173 || n = 1178 || n = 1183 || n = 1188 || n = 1193 || n = 1203 || n = 1208 || n = 1213 || n = 1218 || n = 1223 || n = 1228 || n = 1238 || n = 1242 || n = 1243 || n = 1248)
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
.elseif((n .mod 5 = 0) && ((n > 1109 && n < 1131) || n = 1140 || (n > 1149 && n < 1166) || (n > 1189 && n < 1201) || (n > 1209 && n < 1221) || n = 1230 || n = 1235 || n = 1245))
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1157 || n = 1167 || n = 1202 || n = 1222 || n = 1227)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1117 || n = 1180 || n = 1207)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1124)
	; 6 bytes
	ldx #250 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 1152)
	; 7 bytes
	bit <$2C ;hides 'bit $80A2'
	ldx #128
	dex
	Jne *-4
.elseif(n = 1162)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	inx
	Jpl *-4
.elseif(n = 1189)
	; 5 bytes
	ldy #170 ;hides 'tax'
	dey
	Jne *-2
.elseif(n = 1197)
	; 7 bytes
	bit <$2C ;hides 'bit $85A2'
	ldx #133
	dex
	Jne *-4
.elseif(n = 1232)
	; 6 bytes
	ldy #136 ;hides 'dey'
	nop
	nop
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
.elseif(n = 1252 || n = 1270 || n = 1288)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 1255 || n = 1260 || n = 1280 || n = 1285)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1257 || n = 1265)
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
.elseif(n = 1262 || n = 1267 || n = 1272)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 5 = 4) && ((n > 1263 && n < 1285)))
	; 6 bytes
	pha
	delay_n (n-3)
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
.endif
.else
; 1290 to 1328
.if(n = 1290 || n = 1292 || n = 1300 || n = 1316 || n = 1318)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1291 || n = 1325)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1293)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif((n .mod 8 = 1) && ((n > 1296 && n < 1322)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1298 || n = 1307 || n = 1314 || n = 1322 || n = 1326)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1299)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1301)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
.elseif(n = 1302)
	; 8 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1304)
	; 6 bytes
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1306 || n = 1315)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 1311)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1312)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1319)
	; 6 bytes
	ldx #136 ;hides 'dey'
	pla
	inx
	Jne *-3
.elseif(n = 1320)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	nop
	dex
	Jne *-3
.elseif(n = 1323)
	; 7 bytes
	bit <$2C ;hides 'bit $6DA2'
	ldx #109
	inx
	Jne *-4
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
.if(n = 1329 || n = 1337 || n = 1353 || n = 1361)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1330 || n = 1335 || n = 1340 || n = 1346 || n = 1347 || n = 1354 || n = 1355 || n = 1363)
	; 7 bytes
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1334 || n = 1348 || n = 1356 || n = 1362 || n = 1364)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1336)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
.elseif(n = 1339)
	; 6 bytes
	ldx #122 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1341)
	; 6 bytes
	ldx #122 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 1343)
	; 7 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-3
.elseif(n = 1344)
	; 6 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	dey
	Jne *-3
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
.elseif(n = 1365)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
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
.if(n = 1368 || n = 1375 || n = 1382)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1369 || n = 1377 || n = 1393)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1370 || n = 1384 || n = 1389 || n = 1400 || n = 1403)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1371 || n = 1391)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1372 || n = 1383 || n = 1388 || n = 1390 || n = 1397 || n = 1402)
	; 6..7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1378 || n = 1396 || n = 1405)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1379)
	; 6 bytes
	ldx #138 ;hides 'txa'
	pha
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
	; 7 bytes
	bit <$2C ;hides 'bit $9AA2'
	ldx #154
	dex
	Jne *-4
.elseif(n = 1392 || n = 1406)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1395)
	; 7 bytes
	bit <$2C ;hides 'bit $9BA2'
	ldx #155
	dex
	Jne *-4
.elseif(n = 1398)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.if(n = 1407 || n = 1410 || n = 1419 || n = 1431 || n = 1434 || n = 1435 || n = 1438 || n = 1445)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1444)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && ((n > 1408 && n < 1442)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1411)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
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
.elseif(n = 1414 || n = 1432)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1416 || n = 1418 || n = 1423 || n = 1428 || n = 1444)
	; 6..7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1424 || n = 1437)
	; 6..7 bytes
	pla
	delay_n (n-4)
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
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
.elseif(n = 1442)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.endif
.else
; 1446 to 1484
.if(n = 1446 || n = 1460 || n = 1462 || n = 1474)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1447 || n = 1454 || n = 1463 || n = 1472 || n = 1482)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1448)
	; 7 bytes
	ldx #<$2C ;hides 'bit $A1A2'
	ldx #161
	dex
	Jne *-4
.elseif(n = 1449 || n = 1465 || n = 1473 || n = 1481)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 7 = 1) && ((n > 1449 && n < 1479)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1451 || n = 1461)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1452 || n = 1466 || n = 1479 || n = 1480 || n = 1483)
	; 7 bytes
	delay_n (n-2)
	nop
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
	ldx #96
	delay_n 10
	dex
	Jpl *-4
.elseif(n = 1458)
	; 7 bytes
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
.elseif(n = 1459 || n = 1468 || n = 1477)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 1470)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1487 || n = 1490 || n = 1494 || n = 1498 || n = 1501 || n = 1507 || n = 1508 || n = 1514 || n = 1515 || n = 1523)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1489 || n = 1497 || n = 1505 || n = 1521)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1491)
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
	ldx #69 ;hides 'eor zp'
	nop
	inx
	Jne *-3
.elseif(n = 1500 || n = 1509 || n = 1516)
	; 7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1510)
	; 7 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1517)
	; 6 bytes
	ldx #138 ;hides 'txa'
	pla
	dex
	Jne *-3
.elseif(n = 1518)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1519)
	; 6 bytes
	ldx #152 ;hides 'tya'
	pha
	dex
	Jne *-3
.endif
.else
; 1524 to 1562
.if(n = 1524 || n = 1528 || n = 1544)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1526)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1532)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1533 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1557)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1535)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif((n .mod 8 = 1) && ((n > 1536 && n < 1562)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 1546)
	; 7 bytes
	ldy #136 ;hides 'dey'
	pla
	iny
	iny
	Jne *-4
.elseif(n = 1547)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1551)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1552)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
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
	ldx #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1560)
	; 7 bytes
	ldx #100 ;hides 'nop zp'
	nop
	nop
	inx
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
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1564 || n = 1568 || n = 1571 || n = 1578 || n = 1579 || n = 1587 || n = 1592 || n = 1595 || n = 1599)
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
.elseif(n = 1567 || n = 1594)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 1572 || n = 1580 || n = 1586 || n = 1588 || n = 1600)
	; 7 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 8 = 1) && ((n > 1576 && n < 1602)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1581 || n = 1591)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1582)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 1584)
	; 7 bytes
	php
	lax #6
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1589)
	; 7 bytes
	pla
	ldx #(n-5)/9
	pla
	dex
	Jne *-2
.elseif(n = 1598)
	; 7 bytes
	pla
	delay_n (n-4)
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 7 bytes
	bit <$2C ;hides 'bit $B2A2'
	ldx #178
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1602 && n < 1631)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif((n .mod 7 = 1) && ((n > 1603 && n < 1640)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1605 || n = 1606 || n = 1620 || n = 1634 || n = 1635)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1607)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1608 || n = 1613 || n = 1615 || n = 1622 || n = 1627)
	; 6..7 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1614 || n = 1624 || n = 1626 || n = 1636 || n = 1640)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1616)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1617 || n = 1633)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 1628)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1629)
	; 7 bytes
	bit <$2C ;hides 'bit $B5A2'
	ldx #181
	dex
	Jne *-4
.elseif(n = 1631)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
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
.if((n .mod 8 = 1) && ((n > 1640 && n < 1666)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1642 || n = 1652 || n = 1658 || n = 1663 || n = 1668 || n = 1677)
	; 7 bytes
	pha
	delay_n (n-3)
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
	ldx #73 ;hides 'eor #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1648 || n = 1666 || n = 1675)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1650)
	; 7 bytes
	ldx #165 ;hides 'lda zp'
	nop
	nop
	dex
	Jne *-4
.elseif(n = 1651)
	; 7 bytes
	ldx #138 ;hides 'txa'
	pla
	pha
	inx
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
.elseif(n = 1656)
	; 7 bytes
	bit <$2C ;hides 'bit $B8A2'
	ldx #184
	dex
	Jne *-4
.elseif(n = 1659)
	; 6 bytes
	ldx #90 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1661)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1662 || n = 1669 || n = 1672 || n = 1676)
	; 7..8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1671)
	; 6 bytes
	ldx #152 ;hides 'tya'
	pla
	dex
	Jne *-3
.elseif(n = 1673)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jne *-3
.elseif(n = 1678)
	; 7 bytes
	ldx #128
	delay_n 8
	dex
	Jpl *-3
.elseif(n = 1679)
	; 6 bytes
	ldx #168 ;hides 'tay'
	pha
	dex
	Jne *-3
.endif
.else
; 1680 to 1718
.if(n = 1680)
	; 7 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
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
.elseif(n = 1683 || n = 1686 || n = 1690 || n = 1704 || n = 1715 || n = 1718)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1693 || n = 1711)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1685 || n = 1703)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1687 || n = 1696 || n = 1698 || n = 1708 || n = 1712 || n = 1714)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && ((n > 1688 && n < 1714)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 1699)
	; 6 bytes
	ldy #170 ;hides 'tax'
	pha
	dey
	Jne *-3
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
.elseif(n = 1710)
	; 7 bytes
	bit <$2C ;hides 'bit $BEA2'
	ldx #190
	dex
	Jne *-4
.elseif(n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 1720 || n = 1738 || n = 1747 || n = 1756)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1721 || n = 1729 || n = 1745 || n = 1753)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1722 || n = 1725 || n = 1732 || n = 1739 || n = 1746 || n = 1754)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1752)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724 || n = 1726 || n = 1733 || n = 1740 || n = 1748)
	; 7 bytes
	pha
	delay_n (n-3)
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
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1734 || n = 1757)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1735)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
.elseif(n = 1743)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 1749)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1750)
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
.elseif(n = 1759 || n = 1775 || n = 1780 || n = 1784 || n = 1796)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1760)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
.elseif((n .mod 8 = 1) && ((n > 1760 && n < 1786)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1762 || n = 1766 || n = 1767 || n = 1770 || n = 1776 || n = 1787 || n = 1788 || n = 1795)
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
.elseif(n = 1768)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1771)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1773)
	; 7 bytes
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	dex
	Jne *-4
.elseif(n = 1774 || n = 1783 || n = 1792)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1778)
	; 7 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797 || n = 1814 || n = 1823 || n = 1832)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1798 || n = 1803 || n = 1830)
	; 7..8 bytes
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
.elseif(n = 1801 || n = 1809 || n = 1825 || n = 1833)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1802 || n = 1804 || n = 1812 || n = 1822 || n = 1829)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1805 || n = 1816)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1806)
	; 7 bytes
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	pla
	inx
	Jne *-4
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
.elseif(n = 1810 || n = 1819 || n = 1828)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1811 || n = 1821 || n = 1831)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1813)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1815)
	; 7 bytes
	ldx #165 ;hides 'lda zp'
	nop
	pha
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
	bit <$2C ;hides 'bit $CAA2'
	ldx #202
	dex
	Jne *-4
.elseif(n = 1820)
	; 7 bytes
	pha
	ldx #(n-4)/8
	pha
	dex
	Jne *-2
.elseif(n = 1824)
	; 7 bytes
	ldy #166 ;hides 'ldx zp'
	ldx $48 ;hides 'pha'
	dey
	Jne *-4
.elseif(n = 1826)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop
	nop
	Jne *-3
.elseif(n = 1827)
	; 7 bytes
	bit <$2C ;hides 'bit $CBA2'
	ldx #203
	dex
	Jne *-4
.elseif(n = 1834)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 1835)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 7 bytes
	bit <$2C ;hides 'bit $CCA2'
	ldx #204
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1865)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1838)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1839)
	; 6 bytes
	ldx #184 ;hides 'clv'
	pha
	dex
	Jne *-3
.elseif(n = 1840 || n = 1842 || n = 1852 || n = 1868)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && ((n > 1840 && n < 1874)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1843 || n = 1848 || n = 1856 || n = 1866 || n = 1867 || n = 1874)
	; 7..8 bytes
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
	; 6 bytes
	ldx #168 ;hides 'tay'
	pla
	dex
	Jne *-3
.elseif(n = 1850)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1851)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
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
	; 7 bytes
	ldx #169 ;hides 'lda #imm'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 1859)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	pha
	dey
	Jne *-3
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
.elseif(n = 1869)
	; 6 bytes
	ldy #170 ;hides 'tax'
	pla
	dey
	Jne *-3
.elseif(n = 1870)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	nop
	nop
	inx
	Jne *-4
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
.if((n >= 1876) && (n < 1915))
; 1876 to 1914
.if(n = 1876 || n = 1884 || n = 1892 || n = 1894 || n = 1896 || n = 1910)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1877)
	; 7 bytes
	pla
	ldx #(n-5)/9
	pla
	dex
	Jne *-2
.elseif(n = 1878 || n = 1883 || n = 1887)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1880)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	nop
	nop
	inx
	Jne *-4
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1914)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1910)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1885)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1886 || n = 1912)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
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
.elseif(n = 1893 || n = 1904)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1895)
	; 7 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1903)
	; 7 bytes
	ldx #136 ;hides 'dey'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 1906)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
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
	delay_n 6
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
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
.elseif(n = 1918 || n = 1927 || n = 1936)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1919 || n = 1924 || n = 1930 || n = 1932 || n = 1940)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1920 || n = 1923 || n = 1928 || n = 1939 || n = 1946 || n = 1947)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 1) && ((n > 1920 && n < 1954)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1922)
	; 7 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1931)
	; 7 bytes
	ldx #138 ;hides 'txa'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 1933)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 1938)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 1941 || n = 1951)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1942)
	; 8 bytes
	ldx #139 ;hides 'ane #imm'
	nop $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
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
.elseif(n = 1949)
	; 7 bytes
	pla
	ldx #(n-5)/9
	pla
	dex
	Jne *-2
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1955 || n = 1965 || n = 1974 || n = 1983 || n = 1986 || n = 1987)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1956 || n = 1960 || n = 1966 || n = 1982 || n = 1984)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1957)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1958 || n = 1967 || n = 1973 || n = 1976)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1959 || n = 1992)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 1964)
	; 7 bytes
	pha
	ldx #(n-4)/8
	pha
	dex
	Jne *-2
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
	; 7 bytes
	ldx #152 ;hides 'tya'
	nop
	pla
	dex
	Jne *-4
.elseif(n = 1978)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 1979)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pha
	inx
	Jne *-3
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
.elseif(n = 1991)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
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
.elseif(n = 1994 || n = 2013 || n = 2030)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1995 || n = 2002 || n = 2010 || n = 2018 || n = 2027)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1996 || n = 2004 || n = 2020 || n = 2022 || n = 2028)
	; 7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1999)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pha
	inx
	Jne *-3
.elseif(n = 2000)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inx
	Jne *-3
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
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2012)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	nop
	dey
	Jne *-4
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
.elseif(n = 2019)
	; 6 bytes
	ldy #202 ;hides 'dex'
	pha
	dey
	Jne *-3
.elseif(n = 2023)
	; 6 bytes
	ldx #184 ;hides 'clv'
	pla
	dex
	Jne *-3
.elseif(n = 2024)
	; 7 bytes
	ldx #<$2C ;hides 'bit $1FA2'
	ldx #31
	inx
	Jne *-4
.elseif(n = 2026)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.if(n = 2032 || n = 2038 || n = 2056)
	; 7 bytes
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
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2063)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2036 || n = 2047 || n = 2058)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2037 || n = 2046 || n = 2064)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 5) && ((n > 2038 && n < 2067)))
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2040)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
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
	; 6 bytes
	ldy #186 ;hides 'tsx'
	pla
	dey
	Jne *-3
.elseif(n = 2049)
	; 6 bytes
	ldx #0
	pha
	dex
	Jne *-2
.elseif(n = 2050)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
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
.elseif(n = 2054)
	; 7 bytes
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-4
.elseif(n = 2055 || n = 2068)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
.elseif(n = 2063)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2065)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2067)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
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
.if((n .mod 9 = 1) && ((n > 2070 && n < 2108)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2072 || n = 2074 || n = 2083 || n = 2090 || n = 2092 || n = 2108)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2073 || n = 2082 || n = 2099 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2075 || n = 2084 || n = 2093)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2076 || n = 2085)
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
.elseif(n = 2088)
	; 7 bytes
	bit <$2C ;hides 'bit $E8A2'
	ldx #232
	dex
	Jne *-4
.elseif(n = 2094)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
.if(n = 2110 || n = 2117 || n = 2119 || n = 2126 || n = 2128 || n = 2140)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2111 || n = 2121 || n = 2131)
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2118 || n = 2136 || n = 2139 || n = 2144 || n = 2145)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2120)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
.elseif(n = 2129 || n = 2138)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2130)
	; 8 bytes
	delay_n (n-4)
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
	delay_n 6
	dex
	Jne *-3
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
	; 7 bytes
	ldx #232 ;hides 'inx'
	pha
	inx
	inx
	Jne *-4
.elseif(n = 2148)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2153 || n = 2154 || n = 2162 || n = 2163 || n = 2175)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2155 || n = 2164 || n = 2182 || n = 2184)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 5) && ((n > 2155 && n < 2184)))
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2157)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2158)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $48,X ;hides 'pha'
	dex
	Jne *-4
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
.elseif(n = 2171 || n = 2181)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2172)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2176)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2177)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pla
	inx
	Jne *-3
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
.if((n >= 2188) && (n < 2207))
; 2188 to 2206
.if(n = 2188 || n = 2197 || n = 2206)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 2191 || n = 2201)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2192 || n = 2203)
	; 7 bytes
	pla
	delay_n (n-4)
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
	bit <$2C ;hides 'bit $F4A2'
	ldx #244
	dex
	Jne *-4
.elseif(n = 2198)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2199)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pla
	inx
	Jne *-3
.elseif(n = 2200)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	nop
	nop
	inx
	Jne *-4
.elseif(n = 2202)
	; 7 bytes
	pha
	delay_n (n-3)
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
.endif
.else
; 2207 to 2226
.if(n = 2207)
	; 7 bytes
	ldx #184 ;hides 'clv'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2208 || n = 2216 || n = 2217 || n = 2226)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2209)
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
.elseif(n = 2211)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2212)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
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
.elseif(n = 2215 || n = 2224)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2218)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2219 || n = 2225)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2220)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2221)
	; 6 bytes
	ldy #202 ;hides 'dex'
	pla
	dey
	Jne *-3
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
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2235 || n = 2236 || n = 2252)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2228 || n = 2246 || n = 2255 || n = 2264)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2229 || n = 2230 || n = 2244 || n = 2247 || n = 2253 || n = 2262)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2231)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2232)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 2232 && n < 2261)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2234 || n = 2256)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2237 || n = 2263)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
.elseif(n = 2248)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
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
	nop
	pha
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2270 || n = 2275 || n = 2280 || n = 2288 || n = 2298)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2271 || n = 2291 || n = 2301)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2272 || n = 2274 || n = 2284 || n = 2290 || n = 2292)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2273 || n = 2282)
	; 7 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2279)
	; 7 bytes
	ldx #152 ;hides 'tya'
	delay_n 8
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
	delay_n 6
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
	; 6 bytes
	ldx #26 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 2302)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 6 bytes
	ldx #0
	pla
	dex
	Jne *-2
.elseif(n = 2306 || n = 2307)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2314 || n = 2318)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2309 || n = 2312)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2310)
	; 8 bytes
	bit <$2C ;hides 'bit $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
.elseif(n = 2311 || n = 2321)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2313)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2315)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2316)
	; 8 bytes
	bit <$2C ;hides 'bit $C1A2'
	ldx #193
	pha
	dex
	Jne *-5
.elseif(n = 2317)
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
.elseif(n = 2322)
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
.endif
.else
; 2324 to 2343
.if(n = 2324)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2325)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2326)
	; 7 bytes
	ldx #232 ;hides 'inx'
	pla
	inx
	inx
	Jne *-4
.elseif(n = 2327)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2328)
	; 7 bytes
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 2329 || n = 2341)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2330)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2331)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	nop
	dex
	Jne *-4
.elseif(n = 2332)
	; 8 bytes
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
.elseif(n = 2333)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2334)
	; 8 bytes
	pha
	ldx #233
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2335)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 2336)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $68 ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2337)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2338)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2339)
	; 6 bytes
	ldx #234 ;hides 'nop'
	pha
	dex
	Jne *-3
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
.elseif(n = 2343)
	; 7 bytes
	pla
	delay_n (n-4)
.endif
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
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2345)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2346 || n = 2357 || n = 2368 || n = 2373 || n = 2382)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2348 || n = 2370)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2349)
	; 7 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $68 ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2350)
	; 7 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $68,X ;hides 'pla'
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
	ldx #181 ;hides 'lda zp,x'
	nop
	pla
	dex
	Jne *-4
.elseif(n = 2353 || n = 2365 || n = 2377)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2354 || n = 2367 || n = 2380)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2355)
	; 7 bytes
	php
	lax #9
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2356)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2358 || n = 2369 || n = 2374)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2359)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2360 || n = 2372)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2361 || n = 2371 || n = 2381)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2362)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	lda $68 ;hides 'pla'
	dey
	Jne *-4
.elseif(n = 2363)
	; 7 bytes
	ldx #41 ;hides 'and #imm'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2364)
	; 7 bytes
	ldx #41 ;hides 'and #imm'
	lda $68,X ;hides 'pla'
	inx
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
	bit <$2C ;hides 'bit $D8A2'
	ldx #216
	nop
	dex
	Jne *-5
.elseif(n = 2378)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	pla
	dey
	Jne *-4
.elseif(n = 2379)
	; 7 bytes
	ldy #170 ;hides 'tax'
	delay_n 7
	dey
	Jne *-4
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2403 || n = 2412 || n = 2415)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2392 || n = 2394 || n = 2396 || n = 2400 || n = 2402 || n = 2404)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2385)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 2386)
	; 8 bytes
	ldx #<$2C ;hides 'bit $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
.elseif(n = 2387)
	; 7 bytes
	ldy #200 ;hides 'iny'
	pha
	dey
	dey
	Jne *-4
.elseif(n = 2388 || n = 2399 || n = 2410)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2389 || n = 2401 || n = 2413)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2390)
	; 8 bytes
	ldy #171 ;hides 'lax #imm'
	lda $4868 ;hides 'pla' and 'pha'
	dey
	Jne *-5
.elseif(n = 2391 || n = 2411 || n = 2421)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2393 || n = 2406 || n = 2419)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2395)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2397)
	; 6 bytes
	ldx #218 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 2398)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2405)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2407)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2408)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2409)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2414)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2416)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2417)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 6
	dey
	Jne *-4
.elseif(n = 2418)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
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
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2427 || n = 2456)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2424 || n = 2426 || n = 2428 || n = 2438 || n = 2440 || n = 2444 || n = 2448 || n = 2450 || n = 2457)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2425 || n = 2437 || n = 2449)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2429)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
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
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2432 || n = 2443 || n = 2454)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2433)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2434)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2436)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2439)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2442)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2445 || n = 2458)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2446)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2447)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
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
.elseif(n = 2459)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	pha
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
.elseif(n = 2462 || n = 2495)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2463 || n = 2467 || n = 2475 || n = 2478)
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
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2466 || n = 2483)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2468 || n = 2488 || n = 2492 || n = 2494)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2469)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2470)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2471 || n = 2481 || n = 2491)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2472)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	lda $68 ;hides 'pla'
	pha
	dex
	Jne *-5
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
.elseif(n = 2480)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2482)
	; 8 bytes
	lda #<$AD ;hides 'lda $BFA2'
	ldx #191
	pla
	dex
	Jne *-5
.elseif(n = 2484)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2486)
	; 8 bytes
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
.elseif(n = 2489)
	; 7 bytes
	ldx #90 ;hides 'nop'
	delay_n 8
	inx
	Jne *-4
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
	; 6 bytes
	ldx #250 ;hides 'nop'
	pha
	dex
	Jne *-3
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
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2502 || n = 2504 || n = 2514 || n = 2524 || n = 2526 || n = 2532 || n = 2539)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2503 || n = 2515 || n = 2527)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2505)
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
.elseif(n = 2506)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
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
	nop
	nop
	inx
	Jne *-4
.elseif(n = 2512 || n = 2513 || n = 2522 || n = 2525 || n = 2538)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2516)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
	; 7 bytes
	ldx #168 ;hides 'tay'
	delay_n 8
	dex
	Jne *-4
.elseif(n = 2520)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2523 || n = 2536)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2528)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2529)
	; 6 bytes
	ldx #26 ;hides 'nop'
	pla
	inx
	Jne *-3
.elseif(n = 2530)
	; 8 bytes
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
.elseif(n = 2537)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
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
	ldx #212 ;hides 'nop zp,x'
	nop
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
.elseif(n = 2546)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2548 || n = 2554 || n = 2556 || n = 2560 || n = 2567 || n = 2572 || n = 2576)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2549 || n = 2562)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2550 || n = 2570)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2551)
	; 6 bytes
	ldx #24 ;hides 'clc'
	pla
	inx
	Jne *-3
.elseif(n = 2552)
	; 8 bytes
	bit <$2C ;hides 'bit $E8A2'
	ldx #232
	nop
	dex
	Jne *-5
.elseif(n = 2555 || n = 2565 || n = 2577)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2558)
	; 8 bytes
	ldx #73 ;hides 'eor #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 2561)
	; 7 bytes
	ldx #0
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2563)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2566)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2568)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2573)
	; 6 bytes
	ldx #234 ;hides 'nop'
	pla
	dex
	Jne *-3
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
	pha
	ldx #234
	delay_n 6
	dex
	Jne *-3
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2580 || n = 2587 || n = 2592)
	; 8 bytes
	pla
	delay_n (n-4)
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
	nop
	nop
	inx
	Jne *-4
.elseif(n = 2585 || n = 2590 || n = 2607 || n = 2610)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2586 || n = 2597 || n = 2608)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2588 || n = 2601 || n = 2614)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2589 || n = 2598 || n = 2600)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2591)
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
	delay_n 6
	inx
	Jne *-4
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
.elseif(n = 2604)
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
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 2611)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2612)
	; 7 bytes
	php
	lax #10
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2613)
	; 8 bytes
	lda <$AD ;hides 'lda $C9A2'
	ldx #201
	pla
	dex
	Jne *-5
.elseif(n = 2615)
	; 7 bytes
	ldx #218 ;hides 'nop'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2616)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	pla
	inx
	Jne *-4
.endif
.else
.if((n >= 2618) && (n < 2637))
; 2618 to 2636
.if(n = 2618)
	; 8 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	nop
	inx
	Jne *-5
.elseif(n = 2619 || n = 2630)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2620 || n = 2622)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2621)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 2623)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2624)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2625)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2626)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2627)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2628)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	nop
	pla
	inx
	Jne *-4
.elseif(n = 2629)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2631 || n = 2632)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2633)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2634)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2635)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2636)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.endif
.else
; 2637 to 2656
.if(n = 2637)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	bit $68 ;hides 'pla'
	inx
	Jne *-4
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
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2641 || n = 2653)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2642)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2643 || n = 2654 || n = 2655)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2644)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2645)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2646)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2647)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2648)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2649)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2650)
	; 8 bytes
	ldx #<$2C ;hides 'bit $F1A2'
	ldx #241
	nop
	dex
	Jne *-5
.elseif(n = 2651)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 2652)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2656)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if(n = 2657)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif((n > 2657 && n < 2661))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2664)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 2665 || n = 2677 || n = 2689)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2666 || n = 2679 || n = 2692)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2667 || n = 2676 || n = 2691)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2668 || n = 2688)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2669 || n = 2693)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2670)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2671 || n = 2686)
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
.elseif(n = 2673)
	; 8 bytes
	bit <$2C ;hides 'bit $0DA2'
	ldx #13
	nop
	inx
	Jne *-5
.elseif(n = 2678)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
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
	nop
	nop
	dex
	Jne *-4
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
.elseif(n = 2690)
	; 8 bytes
	lda #<$AD ;hides 'lda $CFA2'
	ldx #207
	pla
	dex
	Jne *-5
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
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2697)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2698 || n = 2709 || n = 2720 || n = 2727 || n = 2733)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2699)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2700 || n = 2711 || n = 2722)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2701 || n = 2713 || n = 2725)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2702)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2703 || n = 2717)
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
.elseif(n = 2705 || n = 2731)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2706)
	; 8 bytes
	bit <$2C ;hides 'bit $F6A2'
	ldx #246
	nop
	dex
	Jne *-5
.elseif(n = 2708 || n = 2710 || n = 2732 || n = 2734)
	; 8 bytes
	pha
	delay_n (n-3)
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
	nop
	pla
	dex
	Jne *-5
.elseif(n = 2715)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2716)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2719)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 2723)
	; 8 bytes
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	pha
	inx
	Jne *-5
.elseif(n = 2724)
	; 8 bytes
	bit <$2C ;hides 'bit $1DA2'
	ldx #29
	pha
	inx
	Jne *-5
.elseif(n = 2726)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	lda $68 ;hides 'pla'
	nop
	dey
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
.if((n >= 2735) && (n < 2754))
; 2735 to 2753
.if(n = 2735)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 2736)
	; 8 bytes
	bit <$2C ;hides 'bit $E4A2'
	ldx #228
	pha
	dex
	Jne *-5
.elseif(n = 2737)
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
.elseif(n = 2739)
	; 7 bytes
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_15cyc
	Jne *-4
.elseif(n = 2740 || n = 2751)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2741)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2742)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 2743 || n = 2747)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2744)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
.elseif(n = 2748 || n = 2753)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2749)
	; 6 bytes
	ldx #250 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 2750)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2752)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	lda $68 ;hides 'pla'
	dex
	Jne *-4
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2755)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	pla
	dex
	Jne *-4
.elseif(n = 2756)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2757 || n = 2770)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2758 || n = 2765)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2759)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	lda $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2760)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2761 || n = 2773)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2762)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2763 || n = 2764)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2766)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2767)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 2768)
	; 7 bytes
	ldx #(n-12)/13 ;hides 'nop zp,x'
	inx ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 2769)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 2771)
	; 7 bytes
	ldx #58 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2772)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pha
	inx
	Jne *-4
.endif
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
.elseif(n = 2777)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif((n > 2777 && n < 2781))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2788 || n = 2792 || n = 2798 || n = 2802 || n = 2810 || n = 2812)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2789)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 8
	dey
	Jne *-4
.elseif(n = 2790)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	pla
	pha
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
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
.elseif(n = 2803)
	; 8 bytes
	pla
	ldx #56 ;hides 'sec'
	pla
	pha
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
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2832 || n = 2851)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2814 || n = 2828 || n = 2842)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2815)
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
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2818)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2819)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	pha
	inx
	Jne *-4
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
.elseif(n = 2822 || n = 2835 || n = 2848)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2823 || n = 2837 || n = 2847 || n = 2850)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2824 || n = 2838 || n = 2844)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2825)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2826 || n = 2839)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2827)
	; 7 bytes
	ldy #202 ;hides 'dex'
	delay_n 7
	dey
	Jne *-4
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
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2834)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	nop
	pha
	inx
	Jne *-5
.elseif(n = 2836)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
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
.elseif(n = 2843)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2846)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 2849)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2873 || n = 2878)
	; 8 bytes
	pla
	delay_n (n-4)
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
	ldx #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 2856)
	; 8 bytes
	bit <$2C ;hides 'bit $12A2'
	ldx #18
	pha
	inx
	Jne *-5
.elseif(n = 2857 || n = 2881)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2858)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 2859 || n = 2871 || n = 2883)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2860 || n = 2864 || n = 2872 || n = 2877 || n = 2884)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2861 || n = 2874 || n = 2887)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2862 || n = 2875 || n = 2882 || n = 2888)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2863)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
.elseif(n = 2865)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 2866)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2867)
	; 8 bytes
	ldx #<$2C ;hides 'bit $11A2'
	ldx #17
	pha
	inx
	Jne *-5
.elseif(n = 2868)
	; 8 bytes
	bit <$2C ;hides 'bit $EFA2'
	ldx #239
	pha
	dex
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
.elseif(n = 2876)
	; 8 bytes
	ldy #136 ;hides 'dey'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
.elseif(n = 2879)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	pla
	pha
	dex
	Jne *-5
.elseif(n = 2880)
	; 8 bytes
	bit <$2C ;hides 'bit $F0A2'
	ldx #240
	pha
	dex
	Jne *-5
.elseif(n = 2885)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2886)
	; 8 bytes
	lda <$AD ;hides 'lda $DEA2'
	ldx #222
	pla
	dex
	Jne *-5
.elseif(n = 2890)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891 || n = 2908)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2892)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $68 ;hides 'pla'
	pha
	dex
	Jne *-5
.elseif((n .mod 12 = 1) && ((n > 2892 && n < 2930)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2894)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $48,X ;hides 'pha'
	pla
	dex
	Jne *-5
.elseif(n = 2895 || n = 2902 || n = 2907 || n = 2919 || n = 2928)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2896)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2897)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 2898)
	; 8 bytes
	lda #<$AD ;hides 'lda $DFA2'
	ldx #223
	pla
	dex
	Jne *-5
.elseif(n = 2899)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2900 || n = 2913 || n = 2926)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2901 || n = 2921)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2903)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 2904)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2906)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
.elseif(n = 2909)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2910)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $48,X ;hides 'pha'
	pla
	dey
	Jne *-5
.elseif(n = 2911)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	pla
	pha
	dex
	Jne *-5
.elseif(n = 2912)
	; 8 bytes
	lda <$AD ;hides 'lda $E0A2'
	ldx #224
	pla
	dex
	Jne *-5
.elseif(n = 2914 || n = 2922)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2915)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2916 || n = 2918 || n = 2920)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2923)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 2924)
	; 8 bytes
	lda #<$AD ;hides 'lda $1FA2'
	ldx #31
	pla
	inx
	Jne *-5
.elseif(n = 2925)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2927)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	pha
	dex
	Jne *-4
.endif
.else
; 2930 to 2968
.if(n = 2930)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2931 || n = 2943 || n = 2954 || n = 2967)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2932 || n = 2944 || n = 2968)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2933 || n = 2957)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2934 || n = 2948 || n = 2949 || n = 2958 || n = 2960 || n = 2962)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2935)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $4848 ;hides two 'pha's
	dex
	Jne *-5
.elseif(n = 2936)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	lda $68 ;hides 'pla'
	nop
	dex
	Jne *-5
.elseif(n = 2937)
	; 8 bytes
	lda #<$AD ;hides 'lda $1EA2'
	ldx #30
	pla
	inx
	Jne *-5
.elseif(n = 2938)
	; 8 bytes
	lda <$AD ;hides 'lda $E2A2'
	ldx #226
	pla
	dex
	Jne *-5
.elseif(n = 2939 || n = 2952)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2940)
	; 8 bytes
	bit <$2C ;hides 'bit $F5A2'
	ldx #245
	pha
	dex
	Jne *-5
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
.elseif(n = 2945)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
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
.elseif(n = 2950)
	; 8 bytes
	lda #<$AD ;hides 'lda $1DA2'
	ldx #29
	pla
	inx
	Jne *-5
.elseif(n = 2951)
	; 8 bytes
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	pha
	inx
	Jne *-5
.elseif(n = 2955)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2956)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2961)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2963)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 2964)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	pha
	inx
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
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if(n = 2969)
	; 7 bytes
	ldx #58 ;hides 'nop'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 2970 || n = 2974 || n = 2985 || n = 2996 || n = 2998)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2971 || n = 2986)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2972 || n = 2980 || n = 2992 || n = 2994 || n = 3007)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2973 || n = 2982 || n = 2995)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2975)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	delay_n 9
	dey
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
.elseif(n = 2978 || n = 2991 || n = 3004)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2979 || n = 3003)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2981)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2983 || n = 2997)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2984)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2987)
	; 8 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	pha
	inx
	Jne *-5
.elseif(n = 2988)
	; 8 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	pha
	inx
	Jne *-5
.elseif(n = 2990)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 2999)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 3000)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 3002)
	; 7 bytes
	ldx #232 ;hides 'inx'
	pla
	dex
	dex
	Jne *-4
.elseif(n = 3005)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 3006)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.endif
.else
; 3008 to 3046
.if(n = 3008 || n = 3034)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3009)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3010)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3011 || n = 3039)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3012)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	nop
	pla
	inx
	Jne *-4
.elseif(n = 3013 || n = 3025 || n = 3037)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3014 || n = 3019 || n = 3026 || n = 3027 || n = 3032 || n = 3045)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3015)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-4
.elseif(n = 3016 || n = 3031 || n = 3046)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3017 || n = 3030 || n = 3043)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3018 || n = 3020 || n = 3028 || n = 3033 || n = 3040)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3021)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3022)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3023)
	; 8 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	pha
	inx
	Jne *-5
.elseif(n = 3024)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 3029)
	; 7 bytes
	ldy #202 ;hides 'dex'
	delay_n 8
	dey
	Jne *-4
.elseif(n = 3035)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3036)
	; 8 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	pha
	inx
	Jne *-5
.elseif(n = 3038)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3041)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3042)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	inx
	inx
	Jne *-5
.elseif(n = 3044)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
.if(n = 3047)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3048)
	; 8 bytes
	bit <$2C ;hides 'bit $02A2'
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
.elseif(n = 3050)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3051)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3052)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3053)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3054)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	pla
	inx
	Jne *-4
.elseif(n = 3055)
	; 8 bytes
	pla
	ldx #21 ;hides 'ora zp,x'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3056)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3057)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3058 || n = 3063)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3059)
	; 8 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	pha
	inx
	Jne *-5
.elseif(n = 3060)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3062)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3064)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3065)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.endif
.else
; 3066 to 3085
.if(n = 3066)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 3067)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 3068)
	; 8 bytes
	lda <$AD ;hides 'lda $14A2'
	ldx #20
	pla
	inx
	Jne *-5
.elseif(n = 3069 || n = 3082)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3070 || n = 3076 || n = 3083 || n = 3085)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3071)
	; 8 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	pha
	dex
	Jne *-5
.elseif(n = 3072)
	; 8 bytes
	bit <$2C ;hides 'bit $00A2'
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
	ldx #181 ;hides 'lda zp,x'
	lda $68,X ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 3075 || n = 3084)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3077)
	; 8 bytes
	pla
	ldx #0
	pla
	pha
	dex
	Jne *-3
.elseif(n = 3078)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3079)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3080)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 3081)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.endif
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3099 || n = 3125)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3087)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $6868 ;hides two 'pla's
	dey
	Jne *-5
.elseif(n = 3088 || n = 3092 || n = 3096 || n = 3100 || (n > 3101 && n < 3105) || (n > 3112 && n < 3117) || n = 3118 || n = 3122)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3089 || n = 3105)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3090)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	lda $68 ;hides 'pla'
	pla
	dey
	Jne *-5
.elseif(n = 3091 || n = 3106)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3093)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3094)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3095 || n = 3108 || n = 3121)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3097 || n = 3110)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3098 || n = 3117)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3101)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3107)
	; 8 bytes
	lda <$AD ;hides 'lda $EFA2'
	ldx #239
	pla
	dex
	Jne *-5
.elseif(n = 3109 || n = 3123)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3111 || n = 3124)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3119)
	; 8 bytes
	lda #<$AD ;hides 'lda $10A2'
	ldx #16
	pla
	inx
	Jne *-5
.elseif(n = 3120)
	; 8 bytes
	lda <$AD ;hides 'lda $F0A2'
	ldx #240
	pla
	dex
	Jne *-5
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
.if(n = 3126)
	; 7 bytes
	php
	lax #12
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3127)
	; 8 bytes
	ldx #184 ;hides 'clv'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3128 || n = 3149 || n = 3162)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3129 || n = 3150)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3130 || n = 3138 || n = 3164)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3131 || n = 3135 || n = 3139 || n = 3140 || (n > 3141 && n < 3145) || n = 3148 || n = 3152 || n = 3154 || n = 3156 || n = 3157)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3132)
	; 8 bytes
	lda #<$AD ;hides 'lda $F1A2'
	ldx #241
	pla
	dex
	Jne *-5
.elseif(n = 3133)
	; 8 bytes
	lda <$AD ;hides 'lda $F1A2'
	ldx #241
	pla
	dex
	Jne *-5
.elseif(n = 3134 || n = 3147 || n = 3160)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3136 || n = 3155)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3137 || n = 3151)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3141 || n = 3161)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3145)
	; 8 bytes
	lda #<$AD ;hides 'lda $0EA2'
	ldx #14
	pla
	inx
	Jne *-5
.elseif(n = 3146 || n = 3163)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3153)
	; 8 bytes
	ldx #90 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3158)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 3159)
	; 8 bytes
	lda <$AD ;hides 'lda $0DA2'
	ldx #13
	pla
	inx
	Jne *-5
.endif
.else
; 3165 to 3203
.if(n = 3165 || n = 3179 || n = 3193)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3166 || n = 3196)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3167)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3168)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 3169)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 3170)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 3171)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	pla
	dex
	Jne *-4
.elseif(n = 3172)
	; 8 bytes
	lda <$AD ;hides 'lda $0CA2'
	ldx #12
	pla
	inx
	Jne *-5
.elseif(n = 3173 || n = 3186 || n = 3199)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3174)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3175 || n = 3188)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3176 || n = 3187 || n = 3189 || n = 3202)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3177 || n = 3190 || n = 3203)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3178)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3180 || n = 3197)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3181 || n = 3201)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3182 || n = 3192 || n = 3195 || n = 3200)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3183)
	; 8 bytes
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
.elseif(n = 3184)
	; 7 bytes
	ldx #(n-12)/13 ;hides 'nop zp,x'
	inx ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 3185)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3191)
	; 8 bytes
	ldx #168 ;hides 'tay'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 3194)
	; 8 bytes
	ldx #(n-14)/15 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3198)
	; 8 bytes
	lda <$AD ;hides 'lda $0AA2'
	ldx #10
	pla
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if((n > 3203 && n < 3207) || n = 3213 || n = 3218 || n = 3230 || n = 3232 || n = 3234 || n = 3239)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3207 || n = 3221 || n = 3235)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3208)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3209)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68 ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 3210)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68,X ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 3211 || n = 3226)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3212 || n = 3225 || n = 3238)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3214)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3215 || n = 3222 || n = 3228)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3216)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3217)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3219)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3220)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 3223)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	lda $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3224)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3227 || n = 3240)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3229)
	; 8 bytes
	ldy #170 ;hides 'tax'
	jsr @nearby_rts
	dey
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
	nop
	pha
	dex
	dex
	Jne *-5
.elseif(n = 3236)
	; 8 bytes
	lda #<$AD ;hides 'lda $07A2'
	ldx #7
	pla
	inx
	Jne *-5
.elseif(n = 3237)
	; 8 bytes
	lda <$AD ;hides 'lda $07A2'
	ldx #7
	pla
	inx
	Jne *-5
.elseif(n = 3241)
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
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3257 || n = 3259 || n = 3274)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3244)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3245)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3246)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
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
.elseif(n = 3249)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3250)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3251 || n = 3264 || n = 3277)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3252 || n = 3254 || n = 3267 || n = 3272 || n = 3278)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3253 || n = 3266)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3255 || n = 3268 || n = 3273)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3256 || n = 3271)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3258)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3260)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3261 || n = 3281)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3262)
	; 8 bytes
	lda #<$AD ;hides 'lda $05A2'
	ldx #5
	pla
	inx
	Jne *-5
.elseif(n = 3263)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3269)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 8
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
	lda <$AD ;hides 'lda $04A2'
	ldx #4
	pla
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
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3299)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3284)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3285)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3286)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3287)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 3288 || n = 3307)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3289)
	; 8 bytes
	lda <$AD ;hides 'lda $03A2'
	ldx #3
	pla
	inx
	Jne *-5
.elseif(n = 3290 || n = 3303 || n = 3316)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3291 || n = 3305 || n = 3319)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3292 || n = 3318)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3293 || n = 3306)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3294 || n = 3320)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3295)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3296 || n = 3304 || (n > 3307 && n < 3311) || n = 3312)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3297 || n = 3313)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3298)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3300)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3301)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3302)
	; 8 bytes
	lda <$AD ;hides 'lda $02A2'
	ldx #2
	pla
	inx
	Jne *-5
.elseif(n = 3311)
	; 8 bytes
	ldx #184 ;hides 'clv'
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3314)
	; 8 bytes
	lda #<$AD ;hides 'lda $01A2'
	ldx #1
	pla
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
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3341)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif((n > 3321 && n < 3326) || n = 3330 || (n > 3333 && n < 3341) || n = 3342 || n = 3344 || n = 3348 || n = 3349 || (n > 3350 && n < 3360))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3326 || n = 3345)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3327)
	; 8 bytes
	lda #<$AD ;hides 'lda $00A2'
	ldx #0
	pla
	dex
	Jne *-5
.elseif(n = 3328)
	; 8 bytes
	lda <$AD ;hides 'lda $00A2'
	ldx #0
	pla
	dex
	Jne *-5
.elseif(n = 3329)
	; 7 bytes
	ldx #0
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3331 || n = 3346)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3332)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3333 || n = 3347)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3343)
	; 8 bytes
	ldx #152 ;hides 'tya'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3350)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.endif
.endif
.else
; 3360 to 3437
.if(n = 3360 || n = 3362 || n = 3363 || n = 3366 || (n > 3367 && n < 3375) || (n > 3377 && n < 3381) || n = 3392 || (n > 3393 && n < 3399) || n = 3400 || n = 3404 || n = 3405 || n = 3408 || (n > 3409 && n < 3413) || n = 3416 || n = 3419 || n = 3420 || (n > 3421 && n < 3425) || (n > 3425 && n < 3429) || n = 3430 || n = 3432 || n = 3434 || n = 3437)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 14 = 1) && ((n > 3360 && n < 3432)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3364 || n = 3402 || n = 3421)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3365)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3367 || n = 3384 || n = 3401 || n = 3418 || n = 3435)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3376 || n = 3406 || n = 3436)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n .mod 16 = 1) && ((n > 3376 && n < 3426)))
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3381)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3382)
	; 8 bytes
	ldy #200 ;hides 'iny'
	delay_n 8
	dey
	dey
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
	pha
	delay_n (n-3)
.elseif(n = 3387)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3388)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	lda $68 ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 3390)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pla
	dex
	Jne *-5
.elseif(n = 3391)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3407)
	; 8 bytes
	ldx #(n-15)/16 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
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
	nop
	nop
	pha
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
.elseif(n = 3433)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	delay_n 8
	inx
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3439 || n = 3442 || n = 3443 || n = 3446 || n = 3455 || n = 3460 || n = 3462 || n = 3463 || n = 3468 || (n > 3469 && n < 3473) || (n > 3473 && n < 3477))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3440)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3441 || n = 3461)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3444)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 3445 || n = 3459 || n = 3473)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3447)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3448)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3449)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 3450)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 3451 || n = 3466)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3452 || n = 3469)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3453)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3454)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3456)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3457)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3458)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3464)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-5
.elseif(n = 3465)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3467)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3480 || (n > 3489 && n < 3495) || (n > 3505 && n < 3509))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3478 || n = 3497)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3479)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 3481)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3482)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3483 || n = 3513)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3484)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 3485)
	; 8 bytes
	ldx #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
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
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 3489 || n = 3505)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3495)
	; 8 bytes
	ldx #184 ;hides 'clv'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 3496 || n = 3511)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3498)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $6868 ;hides two 'pla's
	inx
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
	nop
	pla
	inx
	Jne *-5
.elseif(n = 3502)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3504)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	nop
	delay_n 8
	inx
	Jne *-5
.elseif(n = 3509)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	dex
	Jne *-4
.elseif(n = 3510)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3512)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $68,X ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 3514)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	nop
	pla
	inx
	Jne *-5
.endif
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3535 || n = 3573 || n = 3592)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3517)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	bit $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3518)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3519 || n = 3525 || (n > 3529 && n < 3533) || n = 3534 || n = 3540 || n = 3542 || (n > 3543 && n < 3553) || n = 3555 || (n > 3557 && n < 3561) || n = 3562 || (n > 3563 && n < 3569) || n = 3570 || n = 3572 || (n > 3573 && n < 3581) || (n > 3581 && n < 3585) || n = 3587 || (n > 3588 && n < 3592) || n = 3593)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 3522)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $68,X ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 3523)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	pla
	inx
	Jne *-5
.elseif(n = 3524)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	nop
	pla
	inx
	Jne *-5
.elseif(n = 3526 || n = 3556 || n = 3586)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3527)
	; 8 bytes
	ldx #168 ;hides 'tay'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3528)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	pla
	inx
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 3528 && n < 3572)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3533)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	jsr @nearby_rts
	dey
	Jne *-5
.elseif(n = 3536)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 8
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
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	pla
	inx
	Jne *-5
.elseif(n = 3553)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3563)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 11
	inx
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
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
; 3594 to 3671
.if((n > 3593 && n < 3597) || n = 3598 || n = 3602 || n = 3604 || (n > 3605 && n < 3611) || (n > 3611 && n < 3616) || n = 3618 || n = 3619 || (n > 3622 && n < 3630) || n = 3632 || n = 3634 || (n > 3635 && n < 3639) || n = 3645 || n = 3647 || n = 3648 || n = 3650 || n = 3652 || n = 3654 || n = 3660 || (n > 3661 && n < 3665) || n = 3666 || n = 3667 || (n > 3668 && n < 3672))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3597)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	lda $6868 ;hides two 'pla's
	dex
	Jne *-5
.elseif(n = 3599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 3600)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	lda $68 ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 3601 || n = 3621 || n = 3641 || n = 3661)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3603)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3605 || n = 3622 || n = 3639 || n = 3656)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3611 || n = 3630 || n = 3649 || n = 3668)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3616 || n = 3631 || n = 3646)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3617 || n = 3633 || n = 3665)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3620)
	; 8 bytes
	ldx #(n-16)/17 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3635)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3643)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3644)
	; 8 bytes
	pla
	delay_n (n-4)
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
	nop $4848 ;hides two 'pha's
	dex
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
.endif
.else
; 3672 to 3750
.if(n = 3672 || n = 3675 || n = 3677 || n = 3678 || n = 3680 || (n > 3681 && n < 3687) || n = 3688 || n = 3689 || (n > 3691 && n < 3695) || n = 3696 || n = 3698 || n = 3699 || n = 3702 || (n > 3707 && n < 3711) || n = 3712 || (n > 3713 && n < 3721) || n = 3722 || n = 3723 || (n > 3725 && n < 3729) || (n > 3729 && n < 3736) || n = 3737 || n = 3738 || n = 3740 || n = 3742 || (n > 3745 && n < 3749))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3673 || n = 3690 || n = 3707 || n = 3724 || n = 3741)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3674)
	; 8 bytes
	ldx #(n-14)/15 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3676 || n = 3691 || n = 3736)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3679)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3681 || n = 3701 || n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3687 || n = 3706 || n = 3725 || n = 3744)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3695)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif((n .mod 16 = 1) && ((n > 3696 && n < 3746)))
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3700)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 3703)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3704)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68,X ;hides 'pla'
	pla
	inx
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
.elseif(n = 3739)
	; 8 bytes
	ldy #170 ;hides 'tax'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
.elseif(n = 3743)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3749)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 8
	dex
	Jne *-4
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
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
; 3751 to 3828
.if(n = 3751 || n = 3766 || n = 3796 || n = 3811)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3752)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3753)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3754)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3755 || (n > 3766 && n < 3770) || n = 3771 || n = 3776 || n = 3778 || n = 3779 || (n > 3782 && n < 3791) || n = 3794 || n = 3795 || n = 3797 || n = 3798 || n = 3800 || (n > 3801 && n < 3809) || n = 3810 || (n > 3811 && n < 3820) || (n > 3821 && n < 3825) || n = 3827 || n = 3828)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3756)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3757)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3758 || n = 3792 || n = 3809 || n = 3826)
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
.elseif(n = 3761 || n = 3781 || n = 3801 || n = 3821)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3762)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3763 || n = 3782 || n = 3820)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3764)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
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
.elseif(n = 3772)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3773)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3774)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pla
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
.elseif(n = 3777 || n = 3793 || n = 3825)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
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
.endif
.else
; 3829 to 3906
.if((n > 3828 && n < 3837) || n = 3838 || n = 3840 || n = 3842 || (n > 3843 && n < 3856) || n = 3862 || (n > 3863 && n < 3873) || (n > 3873 && n < 3877) || (n > 3877 && n < 3881) || (n > 3881 && n < 3887) || n = 3888 || (n > 3889 && n < 3894) || n = 3895 || n = 3904 || n = 3906)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3837)
	; 8 bytes
	ldy #202 ;hides 'dex'
	jsr @nearby_rts
	dey
	Jne *-5
.elseif(n = 3839 || n = 3858 || n = 3896)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3841 || n = 3861 || n = 3881)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3843 || n = 3860 || n = 3877 || n = 3894)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3856)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
.elseif(n = 3857 || n = 3873 || n = 3889)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3859 || n = 3900)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3901)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3902)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pla
	dex
	Jne *-5
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
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907 || n = 3908 || n = 3910 || (n > 3911 && n < 3915) || (n > 3915 && n < 3919) || n = 3920 || n = 3922 || n = 3924 || n = 3925 || n = 3927 || (n > 3928 && n < 3934) || n = 3935 || n = 3936 || (n > 3937 && n < 3941) || n = 3942 || n = 3944 || (n > 3945 && n < 3953) || (n > 3953 && n < 3961) || (n > 3962 && n < 3969) || n = 3970 || n = 3971 || (n > 3972 && n < 3977) || n = 3978 || n = 3980 || (n > 3981 && n < 3985))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3909)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3979)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3915 || n = 3934 || n = 3953 || n = 3972)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3919)
	; 8 bytes
	ldx #(n-15)/16 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3921 || n = 3941 || n = 3961 || n = 3981)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3923)
	; 8 bytes
	ldx #218 ;hides 'nop'
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3926)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-5
.elseif(n = 3937 || n = 3969)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
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
.endif
.else
; 3985 to 4062
.if(n = 3985 || n = 4017 || n = 4033 || n = 4049)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3986 || n = 3987 || n = 3989 || n = 3990 || n = 3993 || n = 3995 || n = 3997 || n = 3998 || (n > 4001 && n < 4005) || n = 4006 || n = 4007 || n = 4012 || n = 4015 || (n > 4017 && n < 4021) || (n > 4021 && n < 4028) || n = 4031 || (n > 4033 && n < 4041) || (n > 4041 && n < 4047) || (n > 4049 && n < 4059) || n = 4060)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3988)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3992)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3994)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	delay_n 8
	inx
	Jne *-5
.elseif(n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3999)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 4000)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4001 || n = 4021 || n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4005)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 4008)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 4009)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 4011)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4014)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 4016)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4028)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4032)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4059)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
.elseif(n = 4062)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.endif
.endif
.else
; 4063 to 4375
.if(n = 4063 || n = 4158)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132 || n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217 || n = 4234 || n = 4251 || n = 4268 || n = 4285 || n = 4302 || n = 4319 || n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4065)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 4066 || (n > 4067 && n < 4081) || (n > 4081 && n < 4086) || (n > 4086 && n < 4091) || (n > 4091 && n < 4097) || n = 4099 || n = 4100 || (n > 4101 && n < 4105) || (n > 4105 && n < 4115) || (n > 4115 && n < 4121) || n = 4122 || n = 4123 || (n > 4124 && n < 4130) || n = 4131 || (n > 4132 && n < 4139) || n = 4140 || n = 4142 || n = 4146 || n = 4148 || (n > 4149 && n < 4154) || n = 4155 || n = 4159 || n = 4160 || n = 4163 || n = 4165 || (n > 4166 && n < 4175) || (n > 4175 && n < 4181) || n = 4182 || (n > 4183 && n < 4199) || (n > 4201 && n < 4211) || (n > 4211 && n < 4217) || n = 4218 || n = 4220 || (n > 4221 && n < 4234) || (n > 4234 && n < 4238) || n = 4239 || n = 4240 || (n > 4241 && n < 4249) || n = 4250 || (n > 4251 && n < 4257) || (n > 4257 && n < 4261) || (n > 4262 && n < 4268) || (n > 4268 && n < 4276) || (n > 4276 && n < 4281) || (n > 4281 && n < 4285) || (n > 4285 && n < 4295) || (n > 4295 && n < 4301) || (n > 4302 && n < 4314) || (n > 4314 && n < 4319) || n = 4320 || (n > 4321 && n < 4333) || n = 4334 || n = 4335 || (n > 4336 && n < 4341) || (n > 4341 && n < 4352) || n = 4354 || (n > 4355 && n < 4361) || (n > 4361 && n < 4369) || n = 4370 || (n > 4371 && n < 4376))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4067 || n = 4086 || n = 4105 || n = 4124 || n = 4143 || n = 4162 || n = 4181 || n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295 || n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4091)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
.elseif(n = 4097)
	; 8 bytes
	ldx #0
	delay_n 11
	dex
	Jne *-4
.elseif(n = 4101 || n = 4121 || n = 4141 || n = 4161 || n = 4201 || n = 4221 || n = 4241 || n = 4261 || n = 4281 || n = 4301 || n = 4321 || n = 4341 || n = 4361)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4130)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 4139)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 4144)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 4145)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 4147)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	delay_n 8
	dex
	Jne *-5
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
	pha
	delay_n (n-3)
.elseif(n = 4164)
	; 8 bytes
	ldx #(n-16)/17 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 4175)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 4199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4211)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4249)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 10
	dex
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
.if((n > 4375 && n < 4381) || (n > 4381 && n < 4390) || (n > 4390 && n < 4399) || n = 4400 || (n > 4401 && n < 4407) || n = 4408 || n = 4410 || n = 4412 || (n > 4415 && n < 4421) || (n > 4421 && n < 4428) || (n > 4428 && n < 4441) || n = 4442 || n = 4444 || n = 4446 || (n > 4447 && n < 4461) || (n > 4461 && n < 4465) || (n > 4466 && n < 4481) || (n > 4481 && n < 4485) || (n > 4485 && n < 4499) || n = 4500 || n = 4502 || n = 4503 || (n > 4504 && n < 4521) || n = 4522 || (n > 4523 && n < 4541) || (n > 4542 && n < 4561) || (n > 4561 && n < 4577) || n = 4578 || n = 4579 || (n > 4581 && n < 4599) || n = 4600 || (n > 4601 && n < 4618) || n = 4619 || n = 4620 || (n > 4621 && n < 4637) || (n > 4637 && n < 4641) || (n > 4641 && n < 4656) || (n > 4656 && n < 4661) || (n > 4661 && n < 4668) || n = 4669 || n = 4673 || n = 4674 || (n > 4675 && n < 4681) || (n > 4681 && n < 4694) || (n > 4694 && n < 4701) || (n > 4701 && n < 4713) || (n > 4713 && n < 4721) || (n > 4721 && n < 4732) || (n > 4732 && n < 4741) || (n > 4741 && n < 4749) || n = 4750 || (n > 4751 && n < 4761) || (n > 4761 && n < 4770) || (n > 4770 && n < 4781) || (n > 4781 && n < 4789) || (n > 4789 && n < 4795) || (n > 4795 && n < 4801) || (n > 4801 && n < 4808) || (n > 4808 && n < 4821) || (n > 4821 && n < 4827) || n = 4828 || (n > 4829 && n < 4841) || (n > 4841 && n < 4846) || (n > 4846 && n < 4861) || (n > 4861 && n < 4865) || (n > 4865 && n < 4871) || (n > 4871 && n < 4881) || (n > 4881 && n < 4901) || (n > 4901 && n < 4913) || (n > 4913 && n < 4921) || (n > 4921 && n < 4925) || n = 4926 || (n > 4929 && n < 4941) || (n > 4941 && n < 4961) || (n > 4961 && n < 4981) || (n > 4981 && n < 5001))
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
.elseif(n = 4414 || n = 4671 || n = 4928)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4415 || n = 4672 || n = 4929)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4443)
	; 8 bytes
	ldy #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
.elseif(n = 4499)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 11
	dex
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
.elseif((n > 5001 && n < 5021) || (n > 5021 && n < 5041) || (n > 5041 && n < 5059) || n = 5060 || (n > 5061 && n < 5081) || (n > 5081 && n < 5101) || n = 5102 || (n > 5103 && n < 5121) || (n > 5121 && n < 5147) || (n > 5147 && n < 5182) || n = 5183 || (n > 5186 && n < 5249) || (n > 5249 && n < 5439) || n = 5440 || (n > 5443 && n < 5499) || (n > 5499 && n < 5696) || n = 5697 || (n > 5700 && n < 5953) || n = 5954 || (n > 5957 && n < 6210) || n = 6211 || (n > 6214 && n < 6467) || n = 6468 || (n > 6471 && n < 6724) || n = 6725 || (n > 6728 && n < 6981) || n = 6982 || (n > 6985 && n < 7238) || n = 7239 || (n > 7242 && n < 7495) || n = 7496 || n = 7500)
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
.elseif(n = 5185 || n = 5442 || n = 5699 || n = 5956 || n = 6213 || n = 6470 || n = 6727 || n = 6984 || n = 7241 || n = 7498)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5186 || n = 5443 || n = 5700 || n = 5957 || n = 6214 || n = 6471 || n = 6728 || n = 6985 || n = 7242 || n = 7499)
	; 8 bytes
	pla
	delay_n (n-4)
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
.if((n > 7500 && n < 7752) || n = 7753 || (n > 7756 && n < 8009) || n = 8010 || (n > 8013 && n < 8266) || n = 8267 || (n > 8270 && n < 8523) || n = 8524 || (n > 8527 && n < 8780) || n = 8781 || (n > 8784 && n < 9037) || n = 9038 || (n > 9041 && n < 9294) || n = 9295 || (n > 9298 && n < 9551) || n = 9552 || (n > 9555 && n < 9808) || n = 9809 || (n > 9812 && n < 10001))
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
.elseif(n = 7755 || n = 8012 || n = 8269 || n = 8526 || n = 8783 || n = 9040 || n = 9297 || n = 9554 || n = 9811)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 7756 || n = 8013 || n = 8270 || n = 8527 || n = 8784 || n = 9041 || n = 9298 || n = 9555 || n = 9812)
	; 8 bytes
	pla
	delay_n (n-4)
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
.if((n > 10000 && n < 10065) || n = 10066 || (n > 10069 && n < 10322) || n = 10323 || (n > 10326 && n < 10579) || n = 10580 || (n > 10583 && n < 10836) || n = 10837 || (n > 10840 && n < 11093) || n = 11094 || (n > 11097 && n < 11350) || n = 11351 || (n > 11354 && n < 11607) || n = 11608 || (n > 11611 && n < 11864) || n = 11865 || (n > 11868 && n < 12121) || n = 12122 || (n > 12125 && n < 12378) || n = 12379 || (n > 12382 && n < 12635) || n = 12636 || (n > 12639 && n < 12892) || n = 12893 || (n > 12896 && n < 13149) || n = 13150 || (n > 13153 && n < 13406) || n = 13407 || (n > 13410 && n < 13663) || n = 13664 || (n > 13667 && n < 13920) || n = 13921 || (n > 13924 && n < 14177) || n = 14178 || (n > 14181 && n < 14434) || n = 14435 || (n > 14438 && n < 14691) || n = 14692 || (n > 14695 && n < 14948) || n = 14949 || (n > 14952 && n < 15001))
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
.elseif(n = 10068 || n = 10325 || n = 10582 || n = 10839 || n = 11096 || n = 11353 || n = 11610 || n = 11867 || n = 12124 || n = 12381 || n = 12638 || n = 12895 || n = 13152 || n = 13409 || n = 13666 || n = 13923 || n = 14180 || n = 14437 || n = 14694 || n = 14951)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10069 || n = 10326 || n = 10583 || n = 10840 || n = 11097 || n = 11354 || n = 11611 || n = 11868 || n = 12125 || n = 12382 || n = 12639 || n = 12896 || n = 13153 || n = 13410 || n = 13667 || n = 13924 || n = 14181 || n = 14438 || n = 14695 || n = 14952)
	; 8 bytes
	pla
	delay_n (n-4)
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
.if((n > 15000 && n < 15205) || n = 15206 || (n > 15209 && n < 15462) || n = 15463 || (n > 15466 && n < 15719) || n = 15720 || (n > 15723 && n < 15976) || n = 15977 || (n > 15980 && n < 16233) || n = 16234 || (n > 16237 && n < 16490) || n = 16491 || (n > 16494 && n < 16747) || n = 16748 || (n > 16751 && n < 17004) || n = 17005 || (n > 17008 && n < 17261) || n = 17262 || (n > 17265 && n < 17518) || n = 17519 || (n > 17522 && n < 17775) || n = 17776 || (n > 17779 && n < 18032) || n = 18033 || (n > 18036 && n < 18289) || n = 18290 || (n > 18293 && n < 18546) || n = 18547 || (n > 18550 && n < 18803) || n = 18804 || (n > 18807 && n < 19060) || n = 19061 || (n > 19064 && n < 19317) || n = 19318 || (n > 19321 && n < 19574) || n = 19575 || (n > 19578 && n < 19831) || n = 19832 || (n > 19835 && n < 20001))
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
.elseif(n = 15208 || n = 15465 || n = 15722 || n = 15979 || n = 16236 || n = 16493 || n = 16750 || n = 17007 || n = 17264 || n = 17521 || n = 17778 || n = 18035 || n = 18292 || n = 18549 || n = 18806 || n = 19063 || n = 19320 || n = 19577 || n = 19834)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15209 || n = 15466 || n = 15723 || n = 15980 || n = 16237 || n = 16494 || n = 16751 || n = 17008 || n = 17265 || n = 17522 || n = 17779 || n = 18036 || n = 18293 || n = 18550 || n = 18807 || n = 19064 || n = 19321 || n = 19578 || n = 19835)
	; 8 bytes
	pla
	delay_n (n-4)
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
