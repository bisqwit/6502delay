; delay_n, delays n cycles with small code
; Will clobber A, X, S, Z+N, and C
; Will preserve Y, D, V, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 12
; - 4 bytes or less when n <= 19
; - 5 bytes or less when n <= 24
; - 6 bytes or less when n <= 51
; - 7 bytes or less when n <= 1301
; - 8 bytes or less when n <= 2371
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=3, avg=3.7, max=5
; - When 21 <= n <= 30: min=4, avg=5.2, max=6
; - When 31 <= n <= 50: min=4, avg=5.5, max=6
; - When 51 <= n <= 75: min=5, avg=5.8, max=7
; - When 76 <= n <= 100: min=5, avg=5.8, max=7
; - When 101 <= n <= 200: min=4, avg=5.8, max=7
; - When 201 <= n <= 300: min=5, avg=5.9, max=7
; - When 301 <= n <= 500: min=4, avg=5.9, max=7
; - When 501 <= n <= 750: min=5, avg=5.9, max=7
; - When 751 <= n <= 1000: min=5, avg=5.9, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.5, max=8
; - When 2001 <= n <= 3000: min=6, avg=7.5, max=9
; - When 3001 <= n <= 5000: min=7, avg=8.7, max=9
; - When 5001 <= n <= 7500: min=8, avg=9.0, max=9
; - When 7501 <= n <= 10000: min=9, avg=9.0, max=9
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
	; 1 bytes
	pha
.elseif(n = 4)
	; 1 bytes
	pla
.elseif(n = 5 || n = 16 || n = 17)
	; 2..4 bytes
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
.elseif(n = 9 || n = 10 || n = 13)
	; 3..4 bytes
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
.elseif(n = 18 || n = 19)
	; 4 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.else
; 20 to 39
.if(n = 20)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
.elseif(n = 21)
	; 5 bytes
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
.elseif(n = 22)
	; 5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 23)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	pha
	Jpl *-2
.elseif(n = 24)
	; 4 bytes
	lda #$0A ;hides 'asl a'
	Jpl *-1
.elseif(n = 25 || n = 28)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 26)
	; 5 bytes
	sec
	delay_n (n-2)
.elseif(n = 27 || n = 37)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 29)
	; 6 bytes
	clc
	lda #$2A ;hides 'rol a'
	nop
	Jcc *-2
.elseif(n = 30)
	; 6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.elseif(n = 33)
	; 6 bytes
	clc
	delay_n (n-2)
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
.elseif(n = 38)
	; 6 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 42 || n = 72 || n = 74)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 43 || n = 60 || n = 67 || n = 70 || n = 75)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 44)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	nop
	nop
	Jpl *-3
.elseif(n = 45 || n = 48 || n = 58 || n = 68 || n = 73)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 47)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 49)
	; 6 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	Jcc *-2
.elseif(n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 52 || n = 62 || n = 77)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 53)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
.elseif(n = 54)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	delay_n 6
	Jpl *-3
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
.elseif(n = 63)
	; 5 bytes
	lda #$4A ;hides 'lsr a'
	pha
	Jne *-2
.elseif(n = 65)
	; 6 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	Jne *-2
.elseif(n = 69)
	; 5 bytes
	ldx #10 ;hides 'asl a'
	dex
	Jne *-2
.endif
.endif
.else
; 79 to 156
.if(n = 79)
	; 6 bytes
	lda #$2A ;hides 'rol a'
	pha
	clc
	Jne *-3
.elseif(n = 80 || n = 90 || n = 110 || n = 115 || n = 125 || n = 128 || n = 135 || n = 140 || n = 150)
	; 5..6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 84 || n = 94 || n = 104 || n = 109 || n = 114 || n = 119 || n = 127 || n = 139 || n = 144 || n = 149 || n = 154)
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
.elseif(n = 87)
	; 6 bytes
	lda #$4A ;hides 'lsr a'
	delay_n 6
	Jne *-3
.elseif(n = 89)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	nop
	dex
	Jne *-3
.elseif(n = 92 || n = 120 || n = 148 || n = 155)
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
.elseif(n = 97 || n = 129)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 102 || n = 112 || n = 117 || n = 122 || n = 142 || n = 147 || n = 152)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
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
.elseif(n = 124)
	; 4 bytes
	ldx #232 ;hides 'inx'
	Jne *-1
.elseif(n = 130)
	; 6 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.if(n = 157 || n = 160 || n = 165 || n = 175 || n = 180 || n = 195 || n = 200 || n = 205 || n = 210 || n = 220 || n = 230)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 3) && ((n > 157 && n < 179) || (n > 187 && n < 214) || (n > 222 && n < 234)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 170 || n = 184 || n = 189 || n = 194 || n = 202 || n = 214 || n = 219 || n = 229 || n = 234)
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
.elseif(n = 182 || n = 187 || n = 192 || n = 207 || n = 212 || n = 222 || n = 227)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
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
.if(n = 235 || n = 262 || n = 280 || n = 307)
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
.elseif(n = 237 || n = 247 || n = 252 || n = 272 || n = 277 || n = 282 || n = 287)
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif((n .mod 5 = 3) && ((n > 237 && n < 249) || (n > 257 && n < 284) || (n > 297 && n < 309)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 239)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 240 || n = 245 || n = 250 || n = 255 || n = 270 || n = 275 || n = 285 || n = 290 || n = 310)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 241 || n = 251 || n = 256 || n = 266 || n = 271 || n = 276 || n = 281 || n = 301)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 242)
	; 6 bytes
	ldx #74 ;hides 'lsr a'
	inx
	inx
	Jpl *-3
.elseif(n = 244 || n = 254 || n = 259 || n = 264 || n = 269 || n = 279 || n = 284 || n = 294)
	; 6 bytes
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
.elseif(n = 288)
	; 6 bytes
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
.elseif(n = 289 || n = 297 || n = 305)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 292)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 293)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	dex
	Jne *-2
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
; 313 to 468
.if(n = 313 || n = 318 || n = 328 || n = 333 || n = 338 || n = 343 || n = 348 || n = 353 || n = 363 || n = 367 || n = 368 || n = 373 || n = 378 || n = 388 || n = 403 || n = 408 || n = 413 || n = 418 || n = 423 || n = 433 || n = 437 || n = 438 || n = 443 || n = 448 || n = 453 || n = 458 || n = 468)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314 || n = 319 || n = 324 || n = 334 || n = 339 || n = 349 || n = 354 || n = 359 || n = 364 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 404 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 315 || n = 320 || n = 335 || n = 340 || n = 350 || n = 355 || n = 360 || n = 375 || n = 380 || n = 383 || n = 390 || n = 402 || n = 405 || n = 410 || n = 420 || n = 430 || n = 440 || n = 445 || n = 450 || n = 455)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 322 || n = 347 || n = 357 || n = 392 || n = 432 || n = 447 || n = 462)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.elseif(n = 325 || n = 352 || n = 370 || n = 397 || n = 415)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 327 || n = 342 || n = 362 || n = 412 || n = 422 || n = 427 || n = 452)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 329 || n = 345 || n = 369 || n = 417 || n = 457 || n = 465)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
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
.elseif(n = 377)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	inx
	Jpl *-2
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
	; 6 bytes
	delay_n 8
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
.elseif(n = 460)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.if(n = 469 || n = 474 || n = 479 || n = 499 || n = 504 || n = 509 || n = 514 || n = 527 || n = 534 || n = 539)
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
	ldx #197 ;hides 'cmp zp'
	nop
	inx
	Jne *-3
.elseif(n = 473 || n = 478 || n = 482 || n = 483 || n = 488 || n = 493 || n = 508 || n = 513 || n = 518 || n = 523 || n = 528 || n = 538 || n = 542 || n = 543)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 475 || n = 490 || n = 495 || n = 500 || n = 507 || n = 510 || n = 515 || n = 520 || n = 525 || n = 535)
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 489 || n = 497 || n = 529 || n = 545)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 492 || n = 502 || n = 522)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 494)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 503)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
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
.elseif(n = 549 || n = 559 || n = 564 || n = 574 || n = 579 || n = 584 || n = 594 || n = 599 || n = 604 || n = 614 || n = 619)
	; 6 bytes
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
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
.elseif(n = 555 || n = 565 || n = 570 || n = 580 || n = 590 || n = 600 || n = 605 || n = 615)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 556 || n = 561 || n = 571 || n = 576 || n = 581 || n = 596 || n = 606 || n = 611 || n = 616)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 557 || n = 572 || n = 597)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	sta (@ptrtemp,x)
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
	; 5 bytes
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
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
; 626 to 781
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 627 || n = 632 || n = 642 || n = 662 || n = 672 || n = 677 || n = 692 || n = 702 || n = 707 || n = 732 || n = 742 || n = 747 || n = 762 || n = 767 || n = 772)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 628 || n = 633 || n = 643 || n = 648 || n = 653 || n = 658 || n = 663 || n = 668 || n = 678 || n = 682 || n = 683 || n = 693 || n = 698 || n = 703 || n = 713 || n = 717 || n = 718 || n = 723 || n = 728 || n = 733 || n = 738 || n = 753 || n = 758 || n = 763 || n = 768 || n = 773)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 629 || n = 634 || n = 639 || n = 644 || n = 654 || n = 664 || n = 669 || n = 679 || n = 684 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 730 || n = 734 || n = 739 || n = 744 || n = 749 || n = 754 || n = 759 || n = 774 || n = 779)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 630 || n = 635 || n = 650 || n = 655 || n = 660 || n = 670 || n = 675 || n = 690 || n = 695 || n = 700 || n = 710 || n = 725 || n = 735 || n = 740 || n = 752 || n = 760 || n = 765 || n = 770 || n = 780)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 649 || n = 689 || n = 697 || n = 745)
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
.elseif(n = 665)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 667 || n = 685 || n = 712)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
.endif
.else
; 782 to 937
.if(n = 782)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.elseif(n = 783 || n = 787 || n = 788 || n = 793 || n = 798 || n = 803 || n = 808 || n = 818 || n = 822 || n = 823 || n = 828 || n = 833 || n = 838 || n = 843 || n = 853 || n = 858 || n = 863 || n = 868 || n = 873 || n = 878 || n = 888 || n = 893 || n = 898 || n = 903 || n = 908 || n = 913 || n = 923 || n = 927 || n = 928 || n = 933)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 795) || n = 804 || (n > 813 && n < 830) || n = 839 || n = 844 || (n > 853 && n < 865) || (n > 873 && n < 885) || n = 894 || (n > 908 && n < 925) || n = 934))
	; 6 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 5 = 0) && ((n > 789 && n < 816) || n = 830 || (n > 839 && n < 851) || n = 860 || (n > 869 && n < 886) || n = 895 || n = 900 || n = 915 || n = 920 || n = 930))
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 802 || n = 847)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif((n .mod 5 = 2) && (n = 807 || n = 812 || n = 837 || n = 842 || n = 852 || n = 867 || n = 872 || n = 882 || n = 887 || (n > 901 && n < 923)))
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 809 || n = 817 || n = 849 || n = 857 || n = 865 || n = 889 || n = 929)
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
.elseif(n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.elseif(n = 905 || n = 937)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.if(n = 938 || n = 943 || n = 948 || n = 958 || n = 963 || n = 968 || n = 973 || n = 978 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1003 || n = 1008 || n = 1013 || n = 1028 || n = 1032 || n = 1033 || n = 1038 || n = 1043 || n = 1048 || n = 1053 || n = 1068 || n = 1073 || n = 1078 || n = 1083 || n = 1088 || n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1113 || n = 1118 || n = 1123 || n = 1133 || n = 1138 || n = 1143 || n = 1148 || n = 1153 || n = 1158 || n = 1168 || n = 1172 || n = 1173 || n = 1178 || n = 1183 || n = 1188 || n = 1193 || n = 1203 || n = 1208 || n = 1213 || n = 1218 || n = 1223 || n = 1228 || n = 1238 || n = 1242 || n = 1243 || n = 1248)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || (n > 1092 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 950 || n = 970 || n = 975 || n = 980 || n = 990 || n = 1005 || n = 1010 || n = 1015 || n = 1018 || n = 1035 || n = 1040 || n = 1050 || n = 1055 || n = 1060 || n = 1067 || n = 1070 || n = 1075 || n = 1080 || n = 1085 || n = 1095 || n = 1110 || n = 1115 || n = 1120 || n = 1125 || n = 1130 || n = 1140 || n = 1150 || n = 1155 || n = 1160 || n = 1165 || n = 1175 || n = 1190 || n = 1195 || n = 1200 || n = 1210 || n = 1215 || n = 1220 || n = 1230 || n = 1235 || n = 1245 || n = 1250)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942 || n = 947 || n = 952 || n = 957 || n = 987 || n = 1012 || n = 1020 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1082 || n = 1087 || n = 1092 || n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1152 || n = 1157 || n = 1167 || n = 1182 || n = 1192 || n = 1197 || n = 1202 || n = 1222 || n = 1227 || n = 1232)
	; 6..7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 979 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1124 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && (n = 945 || n = 1025 || n = 1057 || (n > 1088 && n < 1106) || (n > 1128 && n < 1146) || n = 1169 || n = 1185 || (n > 1208 && n < 1226) || n = 1249))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 955 || n = 982 || n = 1000 || n = 1027 || n = 1045 || n = 1090 || n = 1162)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
.elseif(n = 1022)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
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
.elseif(n = 1063)
	; 5 bytes
	ldx #152 ;hides 'tya'
	dex
	Jne *-2
.elseif(n = 1117 || n = 1180 || n = 1207)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1187)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	inx
	Jpl *-4
.elseif(n = 1237)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.elseif(n = 1252 || n = 1270 || n = 1288 || n = 1306 || n = 1315)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1253 || n = 1258 || n = 1263 || n = 1278 || n = 1283 || n = 1290 || n = 1291 || n = 1298 || n = 1307 || n = 1314 || n = 1319 || n = 1322 || n = 1326)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1325)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1255 || n = 1260 || n = 1277 || n = 1280 || n = 1285 || n = 1293 || n = 1325)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 8 = 1) && (n = 1257 || n = 1265 || (n > 1296 && n < 1322)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1292 || n = 1300 || n = 1308 || n = 1316 || n = 1318 || n = 1327)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1287 || n = 1302)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 1286)
	; 7 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	dex
	Jne *-4
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
.elseif(n = 1320)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	nop
	dex
	Jne *-3
.elseif(n = 1323)
	; 7 bytes
	cmp <$CD ;hides 'cmp $6DA2'
	ldx #109
	inx
	Jne *-4
.elseif(n = 1328)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
	Jne *-3
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
.elseif(n = 1330 || n = 1335 || n = 1339 || n = 1340 || n = 1344 || n = 1347 || n = 1354 || n = 1355 || n = 1363)
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
	cmp <$CD ;hides 'cmp $6CA2'
	ldx #108
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1332 && n < 1361)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1334 || n = 1336 || n = 1348 || n = 1356 || n = 1362 || n = 1364)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1341)
	; 7 bytes
	cmp <$CD ;hides 'cmp $95A2'
	ldx #149
	dex
	Jne *-4
.elseif(n = 1343)
	; 7 bytes
	ldx #121
	delay_n 6
	dex
	Jpl *-3
.elseif(n = 1346)
	; 7 bytes
	pla
	delay_n (n-4)
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
.if((n .mod 7 = 3) && ((n > 1367 && n < 1390)))
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 1) && ((n > 1368 && n < 1394)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1370 || n = 1384 || n = 1400 || n = 1403)
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
.elseif(n = 1386)
	; 7 bytes
	cmp <$CD ;hides 'cmp $66A2'
	ldx #102
	inx
	Jne *-4
.elseif(n = 1392)
	; 7 bytes
	ldx #(n-14)/13
	delay_n 8
	dex
	Jpl *-3
.elseif(n = 1395)
	; 7 bytes
	cmp <$CD ;hides 'cmp $9BA2'
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
	cmp <$CD ;hides 'cmp $64A2'
	ldx #100
	inx
	Jne *-4
.elseif(n = 1406)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1410 || n = 1416 || n = 1419 || n = 1431 || n = 1434 || n = 1435 || n = 1438 || n = 1445)
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
.elseif(n = 1414 || n = 1432)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1418 || n = 1423 || n = 1428 || n = 1444)
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
	cmp #<$CD ;hides 'cmp $62A2'
	ldx #98
	inx
	Jne *-4
.elseif(n = 1424 || n = 1437)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1426)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 1447 || n = 1454 || n = 1463 || n = 1469 || n = 1472 || n = 1482)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1448)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $A1A2'
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
.elseif(n = 1452 || n = 1466 || n = 1479 || n = 1480)
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
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1458)
	; 7 bytes
	cmp <$CD ;hides 'cmp $A2A2'
	ldx #162
	dex
	Jne *-4
.elseif(n = 1459 || n = 1468 || n = 1477)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1467)
	; 7 bytes
	cmp <$CD ;hides 'cmp $A3A2'
	ldx #163
	dex
	Jne *-4
.elseif(n = 1470)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1487 || n = 1490 || n = 1494 || n = 1498 || n = 1507 || n = 1508 || n = 1514 || n = 1515 || n = 1523)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1489 || n = 1505 || n = 1521)
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
.elseif(n = 1500 || n = 1509 || n = 1516)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1501 || n = 1510)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1502)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $A7A2'
	ldx #167
	dex
	Jne *-4
.elseif(n = 1503)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1504)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1511)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $A8A2'
	ldx #168
	dex
	Jne *-4
.elseif(n = 1512)
	; 7 bytes
	cmp <$CD ;hides 'cmp $A8A2'
	ldx #168
	dex
	Jne *-4
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
.if(n = 1524 || n = 1544)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1525)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1526 || n = 1559)
	; 7 bytes
	pla
	delay_n (n-4)
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1532)
	; 7 bytes
	pha
	ldx #(n-4)/8
	pha
	dex
	Jne *-2
.elseif(n = 1533 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1557 || n = 1560)
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
	cmp <$CD ;hides 'cmp $ABA2'
	ldx #171
	dex
	Jne *-4
.elseif(n = 1546)
	; 8 bytes
	ldx #102
	delay_n 10
	dex
	Jpl *-4
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
	; 7 bytes
	cmp #<$CD ;hides 'cmp $ADA2'
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
.if(n = 1563 || n = 1596 || n = 1607)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1564 || n = 1570 || n = 1571 || n = 1578 || n = 1579 || n = 1586 || n = 1587 || n = 1592 || n = 1595 || n = 1599 || n = 1605 || n = 1606 || n = 1613 || n = 1620 || n = 1634 || n = 1635)
	; 7 bytes
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
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1593 && n < 1631)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 1572 || n = 1580 || n = 1588 || n = 1600 || n = 1614 || n = 1624 || n = 1626 || n = 1636 || n = 1640)
	; 6..7 bytes
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
.elseif((n .mod 8 = 1) && ((n > 1576 && n < 1618) || n = 1633))
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
.elseif(n = 1582 || n = 1629)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1584)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1589)
	; 7 bytes
	pla
	ldx #(n-5)/9
	pla
	dex
	Jne *-2
.elseif(n = 1598 || n = 1608 || n = 1615 || n = 1616 || n = 1622 || n = 1627)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1602)
	; 7 bytes
	cmp <$CD ;hides 'cmp $B2A2'
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
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if((n .mod 8 = 1) && ((n > 1640 && n < 1674)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1642 || n = 1652 || n = 1663 || n = 1668 || n = 1670 || n = 1677)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1643)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 1655 || n = 1658 || n = 1659 || n = 1662 || n = 1669 || n = 1676)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1656)
	; 7 bytes
	cmp <$CD ;hides 'cmp $B8A2'
	ldx #184
	dex
	Jne *-4
.elseif(n = 1661)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1664)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $B9A2'
	ldx #185
	dex
	Jne *-4
.elseif(n = 1671)
	; 6 bytes
	ldx #152 ;hides 'tya'
	pla
	dex
	Jne *-3
.elseif(n = 1672)
	; 8 bytes
	cmp <$CD ;hides 'cmp $98A2'
	ldx #152
	nop
	dex
	Jne *-5
.elseif(n = 1678)
	; 7 bytes
	ldx #(n-14)/13
	delay_n 8
	dex
	Jpl *-3
.elseif(n = 1679)
	; 7 bytes
	pla
	delay_n (n-4)
.endif
.else
; 1680 to 1718
.if(n = 1680 || n = 1694)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 1683 || n = 1686 || n = 1690 || n = 1699 || n = 1703 || n = 1704 || n = 1707 || n = 1715 || n = 1718)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1693 || n = 1711)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1685)
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
	cmp #<$CD ;hides 'cmp $BCA2'
	ldx #188
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
.elseif(n = 1701)
	; 7 bytes
	cmp <$CD ;hides 'cmp $BDA2'
	ldx #189
	dex
	Jne *-4
.elseif(n = 1706)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1710)
	; 7 bytes
	cmp <$CD ;hides 'cmp $BEA2'
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
	cmp <$CD ;hides 'cmp $BFA2'
	ldx #191
	dex
	Jne *-4
.elseif(n = 1720 || n = 1738 || n = 1747 || n = 1756)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1721)
	; 5 bytes
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
.elseif(n = 1722 || n = 1732 || n = 1739 || n = 1746 || n = 1749 || n = 1754)
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
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1725 || n = 1734 || n = 1757)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1727)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1728)
	; 7 bytes
	delay_n (n-4)
	pla
.elseif(n = 1729 || n = 1745 || n = 1753)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1731)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1735)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
.elseif(n = 1743)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 1750)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 1759 || n = 1768 || n = 1775 || n = 1780 || n = 1796)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1760)
	; 6 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif((n .mod 8 = 1) && ((n > 1760 && n < 1786)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1762 || n = 1767 || n = 1776 || n = 1781 || n = 1784 || n = 1787 || n = 1788 || n = 1795)
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
.elseif(n = 1766)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1770)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $A1A2'
	ldx #161
	nop
	dex
	Jne *-5
.elseif(n = 1771)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1773)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C5A2'
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
.elseif(n = 1782)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C6A2'
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
	; 7 bytes
	cmp #<$CD ;hides 'cmp $C7A2'
	ldx #199
	dex
	Jne *-4
.elseif(n = 1791)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C7A2'
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
	cmp <$CD ;hides 'cmp $C8A2'
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
	cmp <$CD ;hides 'cmp $CAA2'
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
	; 8 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1826)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop
	nop
	Jne *-3
.elseif(n = 1827)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CBA2'
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
	ldx #204 ;hides 'cpy abs'
	cmp $C5
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
.elseif(n = 1839 || n = 1843 || n = 1848 || n = 1856 || n = 1859 || n = 1866 || n = 1867 || n = 1874 || n = 1875)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1840 || n = 1852 || n = 1868)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && ((n > 1840 && n < 1874)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1842)
	; 8 bytes
	pha
	delay_n (n-7)
	pla
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
	ldx #(n-1)/13
	delay_n 8
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
	; 7 bytes
	ldx #169 ;hides 'lda #imm'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
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
	nop
	nop
	inx
	Jne *-4
.elseif(n = 1871)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $D0A2'
	ldx #208
	dex
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
.if(n = 1876 || n = 1884 || n = 1892 || n = 1894 || n = 1896 || n = 1903 || n = 1910)
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
.elseif(n = 1878 || n = 1883 || n = 1887 || n = 1902)
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
	cmp <$CD ;hides 'cmp $D2A2'
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
	nop
	pha
	dex
	Jne *-3
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
.elseif(n = 1908)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D4A2'
	ldx #212
	dex
	Jne *-4
.elseif(n = 1914)
	; 8 bytes
	php
	delay_n (n-7)
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
.elseif(n = 1918 || n = 1927 || n = 1936)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1919 || n = 1924 || n = 1928 || n = 1930 || n = 1932 || n = 1940)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1920 || n = 1923 || n = 1939 || n = 1942 || n = 1946 || n = 1947)
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
	cmp #<$CD ;hides 'cmp $D9A2'
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
.elseif(n = 1955 || n = 1965 || n = 1974 || n = 1982 || n = 1983 || n = 1986 || n = 1987)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1956 || n = 1964 || n = 1966 || n = 1984)
	; 7 bytes
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
.elseif(n = 1960)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif((n .mod 8 = 1) && ((n > 1960 && n < 1986)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1962)
	; 7 bytes
	cmp <$CD ;hides 'cmp $DAA2'
	ldx #218
	dex
	Jne *-4
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
	nop
	nop
	dex
	Jne *-4
.elseif(n = 1971)
	; 7 bytes
	cmp <$CD ;hides 'cmp $DBA2'
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
	ldx #165 ;hides 'lda zp'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
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
.elseif(n = 1995 || n = 2002 || n = 2010 || n = 2018 || n = 2019 || n = 2027)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 4) && ((n > 1995 && n < 2029)))
	; 7 bytes
	pha
	delay_n (n-3)
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
	nop
	pha
	dex
	Jne *-3
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
	sta (@ptrtemp,x)
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
	cmp <$CD ;hides 'cmp $E2A2'
	ldx #226
	dex
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
.elseif(n = 2037 || n = 2046 || n = 2054 || n = 2064 || n = 2067)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2039 || n = 2045 || n = 2048 || n = 2057 || n = 2066)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2040)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
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
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	inx
	Jne *-4
.elseif(n = 2052)
	; 7 bytes
	cmp <$CD ;hides 'cmp $E4A2'
	ldx #228
	dex
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
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
	Jne *-4
.elseif(n = 2061)
	; 7 bytes
	cmp <$CD ;hides 'cmp $E5A2'
	ldx #229
	dex
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2072 || n = 2073 || n = 2076 || n = 2082 || n = 2095 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074 || n = 2083 || n = 2090 || n = 2092 || n = 2108)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2075 || n = 2084 || n = 2093)
	; 7 bytes
	pla
	delay_n (n-4)
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
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2085)
	; 8 bytes
	php
	delay_n (n-7)
	plp
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
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	inx
	Jne *-4
.elseif(n = 2094)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
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
	cmp <$CD ;hides 'cmp $EBA2'
	ldx #235
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2115 && n < 2144)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2118 || n = 2136 || n = 2144 || n = 2145)
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
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2139)
	; 6 bytes
	ldx #42 ;hides 'rol a'
	pha
	inx
	Jne *-3
.elseif(n = 2141)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $12A2'
	ldx #18
	inx
	Jne *-4
.elseif(n = 2142)
	; 7 bytes
	cmp <$CD ;hides 'cmp $12A2'
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
	cmp #<$CD ;hides 'cmp $EFA2'
	ldx #239
	dex
	Jne *-4
.elseif(n = 2151)
	; 7 bytes
	cmp <$CD ;hides 'cmp $EFA2'
	ldx #239
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2151 && n < 2180)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2153 || n = 2154 || n = 2158 || n = 2162 || n = 2163 || n = 2175)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2155 || n = 2164 || n = 2182 || n = 2184)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2156)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2157)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 2165)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2166)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	lda $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2167)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	pha
	dex
	Jne *-4
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
.elseif(n = 2174 || n = 2183)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2176)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
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
	ldx #37 ;hides 'and zp'
	nop
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
.elseif(n = 2192 || n = 2203 || n = 2210 || n = 2219)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2193 || n = 2208 || n = 2217 || n = 2225 || n = 2226)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2194)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2202 || n = 2216 || n = 2218)
	; 7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2212)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 2220)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
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
.if(n = 2227 || n = 2235 || n = 2236 || n = 2248 || n = 2254)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2228 || n = 2246 || n = 2255 || n = 2264)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2229 || n = 2230 || n = 2244 || n = 2247 || n = 2252 || n = 2253 || n = 2262)
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
	delay_n (n-7)
	delay_n 7
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
	nop
	pha
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
.elseif(n = 2272 || n = 2274 || n = 2284 || n = 2290 || n = 2292 || n = 2299)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2273 || n = 2282)
	; 7 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2302)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
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
	; 6 bytes
	ldx #0
	pla
	dex
	Jne *-2
.elseif(n = 2306 || n = 2307 || n = 2324 || n = 2335)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2314 || n = 2318 || n = 2336)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2309 || n = 2312 || n = 2323 || n = 2343)
	; 7..8 bytes
	pla
	delay_n (n-4)
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
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2313 || n = 2330)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2315 || n = 2328)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2316)
	; 8 bytes
	cmp <$CD ;hides 'cmp $C1A2'
	ldx #193
	pha
	dex
	Jne *-5
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
	; 8 bytes
	cmp #<$CD ;hides 'cmp $D3A2'
	ldx #211
	nop
	dex
	Jne *-5
.elseif(n = 2322 || n = 2333)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 2334)
	; 8 bytes
	pha
	ldx #233
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2337)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 2338)
	; 8 bytes
	ldx #122
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2339)
	; 6 bytes
	ldx #234 ;hides 'nop'
	pha
	dex
	Jne *-3
.elseif(n = 2340)
	; 8 bytes
	cmp <$CD ;hides 'cmp $C3A2'
	ldx #195
	pha
	dex
	Jne *-5
.elseif(n = 2342)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
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
.if(n = 2344 || n = 2355 || n = 2366)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2345)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2346 || n = 2368 || n = 2373 || n = 2379 || n = 2382)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2348 || n = 2359 || n = 2370)
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
.elseif((n .mod 10 = 1) && ((n > 2350 && n < 2382)))
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2352)
	; 7 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	pla
	dex
	Jne *-4
.elseif(n = 2353)
	; 6 bytes
	ldx #42 ;hides 'rol a'
	pla
	inx
	Jne *-3
.elseif(n = 2354 || n = 2367 || n = 2380)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2356 || n = 2358 || n = 2369 || n = 2374)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2357 || n = 2360)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2362)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	lda $68,X ;hides 'pla'
	dex
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
.elseif(n = 2365 || n = 2377)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2372)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2375)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $D8A2'
	ldx #216
	nop
	dex
	Jne *-5
.elseif(n = 2376)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 2378)
	; 8 bytes
	ldx #73 ;hides 'eor #imm'
	lda $68,X ;hides 'pla'
	nop
	inx
	Jne *-5
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2390 || n = 2400 || n = 2403 || n = 2412 || n = 2415)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2392 || n = 2394 || n = 2396 || n = 2402 || n = 2404)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2385 || n = 2417)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
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
	; 8 bytes
	cmp #<$CD ;hides 'cmp $DAA2'
	ldx #218
	nop
	dex
	Jne *-5
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
	ldx #37 ;hides 'and zp'
	nop
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
.elseif(n = 2418)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	nop
	pha
	inx
	Jne *-5
.elseif(n = 2420)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	pha
	inx
	Jne *-4
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2424 || n = 2426 || n = 2428 || n = 2438 || n = 2440 || n = 2444 || n = 2448 || n = 2450 || n = 2457)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2423)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2425 || n = 2437 || n = 2449)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2427 || n = 2456)
	; 8 bytes
	delay_n (n-2)
	nop
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
	cmp #<$CD ;hides 'cmp $DDA2'
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
.elseif(n = 2439 || n = 2455)
	; 7..8 bytes
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
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
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
	cmp #<$CD ;hides 'cmp $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
.elseif(n = 2453)
	; 8 bytes
	cmp <$CD ;hides 'cmp $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
.elseif(n = 2459)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	pha
	inx
	Jne *-3
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
.elseif(n = 2462 || n = 2468 || n = 2488 || n = 2494)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2463 || n = 2495)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2464)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E0A2'
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
.elseif(n = 2467 || n = 2475 || n = 2478 || n = 2489 || n = 2499)
	; 8 bytes
	delay_n (n-2)
	nop
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
	cmp #<$CD ;hides 'cmp $E1A2'
	ldx #225
	nop
	dex
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
.elseif(n = 2492)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2502 || n = 2503 || n = 2512 || n = 2513 || n = 2522 || n = 2525 || n = 2538)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2514 || n = 2524 || n = 2526 || n = 2539)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2505)
	; 8 bytes
	ldx #232 ;hides 'inx'
	nop
	pha
	inx
	inx
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
	nop
	nop
	inx
	Jne *-4
.elseif(n = 2515 || n = 2527)
	; 8 bytes
	pla
	delay_n (n-4)
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
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	nop
	inx
	Jne *-5
.elseif(n = 2519)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E5A2'
	ldx #229
	nop
	dex
	Jne *-5
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
.elseif(n = 2532)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $EA ;hides 'nop'
	pha
	dex
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
.elseif((n .mod 11 = 1) && ((n > 2541 && n < 2576)))
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
.elseif(n = 2544 || n = 2559 || n = 2571)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2545 || n = 2557 || n = 2569)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2546 || n = 2548 || n = 2550 || n = 2554 || n = 2556 || n = 2560 || n = 2570 || n = 2572 || n = 2576 || n = 2578)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2547)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 2549 || n = 2562)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2551)
	; 6 bytes
	ldx #24 ;hides 'clc'
	pla
	inx
	Jne *-3
.elseif(n = 2552)
	; 8 bytes
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	nop
	inx
	Jne *-5
.elseif(n = 2555)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	pha
	dex
	Jne *-4
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
.elseif(n = 2565 || n = 2577)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2566)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2567)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	pha
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
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	inx
	Jne *-5
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
.elseif(n = 2582 || n = 2594)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2585 || n = 2590 || n = 2603 || n = 2607 || n = 2610)
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
.elseif(n = 2599)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-4
.elseif(n = 2602)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2604)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2606)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $EDA2'
	ldx #237
	nop
	dex
	Jne *-5
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
	; 8 bytes
	ldx #201 ;hides 'cmp #imm'
	cmp $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2613)
	; 8 bytes
	ldx #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	pla
	dex
	Jne *-5
.elseif(n = 2615)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	lda $68 ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 2616)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	pla
	inx
	Jne *-4
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
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2620 || n = 2622 || n = 2644)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2621)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 2623 || n = 2642)
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
.elseif(n = 2627 || n = 2640)
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
.elseif(n = 2629 || n = 2641 || n = 2653)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2631 || n = 2632 || n = 2643 || n = 2654 || n = 2655)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2633 || n = 2647)
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
.elseif(n = 2637)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	nop
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
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	nop
	inx
	Jne *-5
.elseif(n = 2645)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2646 || n = 2648 || n = 2649)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2650)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $F1A2'
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
.elseif(n = 2656)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
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
	cmp <$CD ;hides 'cmp $F2A2'
	ldx #242
	nop
	dex
	Jne *-5
.elseif(n = 2663 || n = 2674 || n = 2685)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2664)
	; 8 bytes
	cmp <$CD ;hides 'cmp $DEA2'
	ldx #222
	pha
	dex
	Jne *-5
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
.elseif(n = 2668 || n = 2682 || n = 2688)
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
.elseif(n = 2670 || n = 2687)
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
.elseif(n = 2683)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $F4A2'
	ldx #244
	nop
	dex
	Jne *-5
.elseif(n = 2684)
	; 8 bytes
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	nop
	inx
	Jne *-5
.elseif(n = 2690)
	; 8 bytes
	lda #<$AD ;hides 'lda $CFA2'
	ldx #207
	pla
	dex
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
.endif
.else
; 2696 to 2734
.if((n .mod 11 = 1) && ((n > 2695 && n < 2730)))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2697 || n = 2702 || n = 2726)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2698 || n = 2720 || n = 2727 || n = 2733)
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
.elseif(n = 2705)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	pla
	inx
	Jne *-3
.elseif(n = 2706)
	; 8 bytes
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
.elseif(n = 2708 || n = 2710 || n = 2732 || n = 2734)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2709)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2712)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $68,X ;hides 'pla'
	nop
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
	cmp #<$CD ;hides 'cmp $E3A2'
	ldx #227
	pha
	dex
	Jne *-5
.elseif(n = 2724)
	; 8 bytes
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	pha
	inx
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
.elseif(n = 2731)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
.elseif(n = 2736)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E4A2'
	ldx #228
	pha
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
.elseif(n = 2739 || n = 2742 || n = 2753 || n = 2763 || n = 2764)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2740 || n = 2751 || n = 2762)
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
.elseif(n = 2743 || n = 2747 || n = 2752 || n = 2754)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2744 || n = 2757 || n = 2770)
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
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2748)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2750)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2755)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 2756)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2758)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	delay_n 6
	dex
	Jne *-5
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
.elseif(n = 2765)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	lda $68 ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2766)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2767)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2768)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
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
	ldx #232 ;hides 'inx'
	pha
	dex
	dex
	Jne *-4
.elseif(n = 2772)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pha
	inx
	Jne *-4
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2788 || n = 2798 || n = 2802 || n = 2812)
	; 8 bytes
	pha
	delay_n (n-3)
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
	; 7 bytes
	ldx #42 ;hides 'rol a'
	nop
	pla
	inx
	Jne *-4
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
	lda $4848 ;hides two 'pha's
	inx
	Jne *-5
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
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2832 || n = 2851)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2814)
	; 8 bytes
	ldx #201 ;hides 'cmp #imm'
	lda $48,X ;hides 'pha'
	pla
	dex
	Jne *-5
.elseif(n = 2815 || n = 2829 || n = 2843)
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
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	nop
	pha
	inx
	Jne *-5
.elseif(n = 2828 || n = 2830 || n = 2842)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2846)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 2849)
	; 8 bytes
	ldx #106 ;hides 'ror a'
	jsr @nearby_rts
	inx
	Jne *-5
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2878)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2853 || n = 2854 || n = 2858 || n = 2862 || n = 2875 || n = 2882 || n = 2888)
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
	nop
	pha
	inx
	Jne *-5
.elseif(n = 2856)
	; 8 bytes
	cmp <$CD ;hides 'cmp $12A2'
	ldx #18
	pha
	inx
	Jne *-5
.elseif(n = 2857 || n = 2869 || n = 2881)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2859 || n = 2876 || n = 2883)
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
	cmp #<$CD ;hides 'cmp $EFA2'
	ldx #239
	pha
	dex
	Jne *-5
.elseif(n = 2868)
	; 8 bytes
	cmp <$CD ;hides 'cmp $EFA2'
	ldx #239
	pha
	dex
	Jne *-5
.elseif(n = 2870 || n = 2889)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2871 || n = 2885)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2873)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
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
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	pha
	inx
	Jne *-5
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
.if(n = 2891 || n = 2908 || n = 2925)
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
.elseif(n = 2895 || n = 2902 || n = 2907 || n = 2915 || n = 2919 || n = 2928)
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
.elseif(n = 2899 || n = 2927)
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
	ldx #85 ;hides 'eor zp,x'
	nop
	delay_n 8
	inx
	Jne *-5
.elseif(n = 2909)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif((n .mod 4 = 2) && ((n > 2909 && n < 2923)))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 2916 || n = 2920)
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
.elseif((n > 2933 && n < 2937) || (n > 2946 && n < 2950) || n = 2958 || n = 2960 || n = 2962 || n = 2966)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
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
.elseif(n = 2950)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2951)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	nop
	pha
	inx
	Jne *-4
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
.elseif(n = 2970)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 2971 || n = 2986)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n > 2971 && n < 2975) || n = 2985 || n = 2996)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2975)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	pha
	inx
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
.elseif(n = 2980 || n = 2992 || n = 2994 || n = 2998 || n = 3007)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2981)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2982)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2984)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.elseif(n = 2990)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	lda $68,X ;hides 'pla'
	pla
	inx
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
	cmp #<$CD ;hides 'cmp $04A2'
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
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 3035)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3036)
	; 8 bytes
	cmp <$CD ;hides 'cmp $03A2'
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
.if(n = 3047 || n = 3064)
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
.elseif(n = 3050)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3051)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $68 ;hides 'pla'
	inx
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
	delay_n (n-4)
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
	cmp #<$CD ;hides 'cmp $01A2'
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
.elseif(n = 3065)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.endif
.else
; 3066 to 3085
.if(n = 3066)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	nop
	nop
	pla
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
.elseif(n = 3070 || n = 3076 || n = 3085)
	; 8 bytes
	pha
	delay_n (n-3)
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
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	nop
	pla
	inx
	Jne *-5
.elseif(n = 3081)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3083)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3099 || n = 3125)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3087 || n = 3088 || n = 3090 || n = 3092 || n = 3096 || n = 3100 || (n > 3101 && n < 3105) || (n > 3112 && n < 3117) || n = 3118 || n = 3122)
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
	lda <$AD ;hides 'lda $EEA2'
	ldx #238
	pla
	dex
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
.if((n > 3125 && n < 3129) || n = 3130 || n = 3131 || n = 3135 || n = 3139 || n = 3140 || (n > 3141 && n < 3145) || n = 3148 || n = 3154 || n = 3156 || n = 3157)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3129 || n = 3146 || n = 3163)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.elseif(n = 3138 || n = 3164)
	; 8 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 3149)
	; 8 bytes
	ldx #106 ;hides 'ror a'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 3150)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3152)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3153)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3158)
	; 8 bytes
	lda #<$AD ;hides 'lda $F3A2'
	ldx #243
	pla
	dex
	Jne *-5
.elseif(n = 3159)
	; 8 bytes
	lda <$AD ;hides 'lda $0DA2'
	ldx #13
	pla
	inx
	Jne *-5
.elseif(n = 3162)
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3167 || n = 3170 || n = 3178 || n = 3182 || n = 3183 || n = 3187 || n = 3195)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3168)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3169 || n = 3185)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3171)
	; 8 bytes
	lda #<$AD ;hides 'lda $0CA2'
	ldx #12
	pla
	inx
	Jne *-5
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
.elseif(n = 3176 || n = 3189 || n = 3200 || n = 3202)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3177 || n = 3190 || n = 3203)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3180)
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
.elseif(n = 3184)
	; 8 bytes
	lda #<$AD ;hides 'lda $F5A2'
	ldx #245
	pla
	dex
	Jne *-5
.elseif(n = 3191)
	; 8 bytes
	ldx #152 ;hides 'tya'
	jsr @nearby_rts_14cyc
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
.elseif(n = 3197)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	nop
	pla
	inx
	Jne *-4
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
.if((n > 3203 && n < 3207) || n = 3218 || n = 3219 || n = 3222 || n = 3230 || n = 3232 || n = 3234 || n = 3239)
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
	; 7 bytes
	ldx #42 ;hides 'rol a'
	delay_n 8
	inx
	Jne *-4
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
.elseif(n = 3213 || n = 3216 || n = 3229)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3214)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3215 || n = 3228)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3217)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
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
.if(n = 3243 || n = 3252 || (n > 3256 && n < 3261) || (n > 3271 && n < 3275))
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
.elseif(n = 3249 || n = 3263)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3250 || n = 3269)
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
.elseif(n = 3253 || n = 3266 || n = 3279)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3254 || n = 3267 || n = 3278)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3255 || n = 3268)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3256 || n = 3271)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
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
; 3282 to 3359
.if(n = 3282 || n = 3299 || n = 3350)
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
	ldx #37 ;hides 'and zp'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3286 || n = 3331 || n = 3346)
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
.elseif(n = 3288 || n = 3307 || n = 3326 || n = 3345)
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
.elseif((n .mod 14 = 1) && ((n > 3290 && n < 3348)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3292 || n = 3318)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3293 || n = 3306 || n = 3332)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3294 || n = 3320)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3295 || n = 3296 || n = 3298 || n = 3304 || (n > 3307 && n < 3313) || n = 3317 || (n > 3321 && n < 3326) || n = 3330 || (n > 3333 && n < 3341) || n = 3342 || n = 3344 || n = 3348 || n = 3349 || (n > 3350 && n < 3360))
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
.elseif(n = 3300)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3301 || n = 3321 || n = 3341)
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
.elseif(n = 3314)
	; 8 bytes
	lda #<$AD ;hides 'lda $01A2'
	ldx #1
	pla
	inx
	Jne *-5
.elseif(n = 3315)
	; 8 bytes
	lda <$AD ;hides 'lda $01A2'
	ldx #1
	pla
	inx
	Jne *-5
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
.elseif(n = 3343)
	; 8 bytes
	ldx #152 ;hides 'tya'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.endif
.else
; 3360 to 3437
.if(n = 3360 || n = 3362 || n = 3363 || n = 3365 || n = 3366 || (n > 3367 && n < 3375) || (n > 3377 && n < 3381) || n = 3382 || (n > 3384 && n < 3389) || n = 3390 || n = 3392 || (n > 3393 && n < 3399) || n = 3400 || n = 3408 || (n > 3409 && n < 3417) || n = 3419 || n = 3420 || n = 3422 || n = 3424 || (n > 3425 && n < 3431) || n = 3432 || n = 3434 || n = 3437)
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
.elseif(n = 3364 || n = 3383 || n = 3421)
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
.elseif(n = 3402)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3404)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	lda $68 ;hides 'pla'
	pha
	dex
	Jne *-5
.elseif(n = 3405)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $48 ;hides 'pha'
	pla
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
.if(n = 3438 || n = 3439 || n = 3442 || n = 3449 || n = 3455 || n = 3460 || n = 3462 || n = 3463 || n = 3465 || n = 3468 || (n > 3469 && n < 3473) || (n > 3473 && n < 3477))
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
.elseif(n = 3443)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	pla
	pha
	inx
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
.elseif(n = 3446 || n = 3453)
	; 8 bytes
	pha
	delay_n (n-3)
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
	ldx #74 ;hides 'lsr a'
	jsr @nearby_rts
	inx
	Jne *-5
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
.if(n = 3477 || n = 3480 || n = 3484 || n = 3485 || (n > 3489 && n < 3496) || n = 3499 || (n > 3505 && n < 3509))
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
.elseif((n > 3516 && n < 3520) || n = 3525 || n = 3527 || (n > 3529 && n < 3535) || n = 3536 || n = 3538 || n = 3540 || n = 3542 || (n > 3543 && n < 3553) || n = 3555 || (n > 3557 && n < 3561) || (n > 3561 && n < 3569) || n = 3570 || n = 3572 || (n > 3573 && n < 3581) || (n > 3581 && n < 3585) || n = 3587 || (n > 3588 && n < 3592) || n = 3593)
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
.elseif(n = 3539)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	pla
	inx
	Jne *-5
.elseif(n = 3553 || n = 3569)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
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
.if((n > 3593 && n < 3599) || n = 3600 || n = 3602 || n = 3604 || (n > 3605 && n < 3611) || n = 3612 || n = 3613 || n = 3615 || n = 3619 || (n > 3622 && n < 3630) || n = 3632 || (n > 3633 && n < 3637) || n = 3638 || n = 3640 || (n > 3641 && n < 3646) || n = 3647 || n = 3648 || (n > 3649 && n < 3653) || n = 3654 || n = 3655 || (n > 3656 && n < 3661) || (n > 3661 && n < 3665) || n = 3666 || n = 3667 || (n > 3668 && n < 3673) || n = 3674 || n = 3675 || (n > 3676 && n < 3681) || (n > 3681 && n < 3687) || n = 3688 || n = 3694 || n = 3696 || n = 3698 || n = 3699 || n = 3702 || n = 3705 || (n > 3707 && n < 3711) || n = 3712 || (n > 3713 && n < 3721) || n = 3722 || n = 3723 || (n > 3725 && n < 3729) || (n > 3729 && n < 3736) || (n > 3736 && n < 3741) || n = 3742 || (n > 3745 && n < 3750))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 3601 || n = 3621 || n = 3641 || n = 3661 || n = 3681 || n = 3701 || n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3603)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 8
	dex
	Jne *-5
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
.elseif(n = 3614)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $6868 ;hides two 'pla's
	dex
	Jne *-5
.elseif(n = 3616 || n = 3631 || n = 3646 || n = 3676 || n = 3691 || n = 3736)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n .mod 16 = 1) && (n = 3617 || n = 3633 || n = 3665 || (n > 3696 && n < 3746)))
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3618)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	lda $68,X ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 3620)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3637)
	; 8 bytes
	ldx #42 ;hides 'rol a'
	nop
	delay_n 8
	inx
	Jne *-5
.elseif(n = 3653)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
.elseif(n = 3689)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 3692)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3693)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3695)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
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
.if((n >= 3751) && (n < 3907))
; 3751 to 3906
.if(n = 3751 || n = 3766 || n = 3796 || n = 3811)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3752 || n = 3753 || n = 3755 || (n > 3766 && n < 3775) || n = 3776 || n = 3778 || n = 3779 || (n > 3782 && n < 3792) || n = 3794 || n = 3795 || n = 3797 || n = 3798 || n = 3800 || (n > 3801 && n < 3809) || n = 3810 || (n > 3811 && n < 3820) || (n > 3821 && n < 3825) || (n > 3826 && n < 3839) || n = 3840 || n = 3842 || (n > 3843 && n < 3851) || (n > 3851 && n < 3856) || (n > 3861 && n < 3873) || (n > 3873 && n < 3877) || (n > 3877 && n < 3881) || (n > 3881 && n < 3889) || (n > 3889 && n < 3894) || n = 3895 || (n > 3896 && n < 3901) || (n > 3901 && n < 3905) || n = 3906)
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
.elseif(n = 3758 || n = 3792 || n = 3809 || n = 3826 || n = 3843 || n = 3860 || n = 3877 || n = 3894)
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
.elseif(n = 3761 || n = 3781 || n = 3801 || n = 3821 || n = 3841 || n = 3861 || n = 3881 || n = 3901)
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
.elseif(n = 3763 || n = 3782 || n = 3820 || n = 3839 || n = 3858 || n = 3896)
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
.elseif(n = 3775)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif((n .mod 16 = 1) && (n = 3777 || n = 3793 || n = 3825 || (n > 3856 && n < 3906)))
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
.elseif(n = 3799)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3851)
	; 8 bytes
	ldx #42 ;hides 'rol a'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 3856)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
.elseif(n = 3859)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 3907 to 4062
.if((n > 3906 && n < 3911) || (n > 3911 && n < 3915) || (n > 3915 && n < 3921) || (n > 3921 && n < 3926) || n = 3927 || (n > 3928 && n < 3934) || n = 3936 || (n > 3937 && n < 3941) || n = 3942 || n = 3944 || (n > 3945 && n < 3953) || (n > 3953 && n < 3961) || (n > 3962 && n < 3969) || n = 3970 || n = 3971 || (n > 3972 && n < 3977) || n = 3978 || n = 3980 || (n > 3981 && n < 3985) || n = 3986 || n = 3987 || n = 3989 || n = 3990 || n = 3993 || n = 3995 || (n > 3996 && n < 4000) || n = 4002 || (n > 4003 && n < 4010) || n = 4012 || n = 4015 || (n > 4017 && n < 4021) || (n > 4021 && n < 4029) || n = 4031 || (n > 4033 && n < 4041) || (n > 4041 && n < 4047) || (n > 4049 && n < 4059) || n = 4060)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3979 || n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3915 || n = 3934 || n = 3953 || n = 3972 || n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3921 || n = 3941 || n = 3961 || n = 3981 || n = 4001 || n = 4021 || n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3926)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-5
.elseif(n = 3935)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3937 || n = 3969 || n = 3985 || n = 4017 || n = 4033 || n = 4049)
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
.elseif(n = 3988)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
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
.elseif(n = 4000)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4003)
	; 8 bytes
	ldx #74 ;hides 'lsr a'
	jsr @nearby_rts_15cyc
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
.else
; 4063 to 4375
.if(n = 4063)
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
	ldx #42 ;hides 'rol a'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4066 || (n > 4067 && n < 4081) || (n > 4081 && n < 4086) || (n > 4086 && n < 4097) || n = 4099 || n = 4100 || (n > 4101 && n < 4105) || (n > 4105 && n < 4115) || (n > 4115 && n < 4121) || n = 4122 || n = 4123 || (n > 4124 && n < 4132) || (n > 4132 && n < 4141) || n = 4142 || (n > 4143 && n < 4149) || (n > 4149 && n < 4161) || (n > 4162 && n < 4166) || (n > 4166 && n < 4175) || (n > 4175 && n < 4181) || n = 4182 || (n > 4183 && n < 4199) || (n > 4201 && n < 4211) || (n > 4211 && n < 4217) || n = 4218 || n = 4220 || (n > 4221 && n < 4234) || (n > 4234 && n < 4238) || n = 4239 || n = 4240 || (n > 4241 && n < 4251) || (n > 4251 && n < 4257) || (n > 4257 && n < 4261) || (n > 4262 && n < 4268) || (n > 4268 && n < 4276) || (n > 4276 && n < 4281) || (n > 4281 && n < 4285) || (n > 4285 && n < 4295) || (n > 4295 && n < 4301) || (n > 4302 && n < 4314) || (n > 4314 && n < 4319) || n = 4320 || (n > 4321 && n < 4333) || n = 4334 || n = 4335 || (n > 4336 && n < 4341) || (n > 4341 && n < 4352) || (n > 4353 && n < 4361) || (n > 4361 && n < 4371) || (n > 4371 && n < 4376))
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
.endif
.endif
.else
; 4376 to 5000
.if((n > 4375 && n < 4381) || (n > 4381 && n < 4390) || (n > 4390 && n < 4399) || n = 4400 || (n > 4401 && n < 4407) || n = 4408 || (n > 4409 && n < 4421) || (n > 4421 && n < 4427) || (n > 4428 && n < 4441) || (n > 4441 && n < 4445) || n = 4446 || (n > 4447 && n < 4461) || (n > 4461 && n < 4465) || (n > 4466 && n < 4481) || (n > 4481 && n < 4485) || (n > 4485 && n < 4493) || (n > 4493 && n < 4501) || n = 4502 || n = 4503 || (n > 4504 && n < 4521) || n = 4522 || (n > 4523 && n < 4541) || (n > 4542 && n < 4561) || (n > 4561 && n < 4580) || (n > 4581 && n < 4599) || n = 4600 || (n > 4601 && n < 4618) || n = 4619 || n = 4620 || (n > 4621 && n < 4637) || (n > 4637 && n < 4641) || (n > 4641 && n < 4656) || (n > 4656 && n < 4661) || (n > 4661 && n < 4668) || (n > 4668 && n < 4673) || n = 4674 || (n > 4675 && n < 4681) || (n > 4681 && n < 4694) || (n > 4694 && n < 4701) || (n > 4701 && n < 4707) || (n > 4707 && n < 4713) || (n > 4713 && n < 4721) || (n > 4721 && n < 4732) || (n > 4732 && n < 4741) || (n > 4741 && n < 4751) || (n > 4751 && n < 4761) || (n > 4761 && n < 4770) || (n > 4770 && n < 4781) || (n > 4781 && n < 4789) || (n > 4789 && n < 4801) || (n > 4801 && n < 4808) || (n > 4808 && n < 4821) || (n > 4821 && n < 4827) || (n > 4827 && n < 4841) || (n > 4841 && n < 4846) || (n > 4846 && n < 4861) || (n > 4861 && n < 4865) || (n > 4865 && n < 4871) || (n > 4871 && n < 4881) || (n > 4881 && n < 4901) || (n > 4901 && n < 4913) || (n > 4913 && n < 4921) || (n > 4921 && n < 4941) || (n > 4941 && n < 4961) || (n > 4961 && n < 4981) || (n > 4981 && n < 5001))
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
.elseif(n = 4427)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	delay_n 11
	inx
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
.elseif(n = 4493)
	; 8 bytes
	ldx #42 ;hides 'rol a'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4668)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-5
.elseif(n = 4673)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4707)
	; 8 bytes
	ldx #42 ;hides 'rol a'
	jsr @nearby_rts_15cyc
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
.endif
.endif
.endif
.endif
.else
; 5001 to 10000
.if(n = 5001 || n = 5021 || n = 5041 || n = 5061 || n = 5081 || n = 5101)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif((n > 5001 && n < 5021) || (n > 5021 && n < 5041) || (n > 5041 && n < 5061) || (n > 5061 && n < 5081) || (n > 5081 && n < 5101) || n = 5102 || (n > 5103 && n < 5121) || (n > 5121 && n < 5147) || (n > 5147 && n < 5165) || (n > 5165 && n < 5411) || (n > 5411 && n < 10001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
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
.elseif(n = 5165)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 5411)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
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
