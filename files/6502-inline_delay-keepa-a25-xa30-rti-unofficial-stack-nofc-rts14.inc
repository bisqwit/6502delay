; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Y, S, Z+N, C, D, and V
; Will preserve A, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).
;
; Requires that a label @nearby_rts_14cyc is defined,
; that points _somewhere_ that contains a dummy 2-cycle
; opcode, such as NOP or LDA #1, followed by a $60 byte (RTS).
;
; Requires a function called "delay_256x_a_30_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays X*256 clocks + overhead
;; Clobbers A,X. Preserves Y.
;; Depends on delay_a_25_clocks within short branch distance
;; Time: X*256+16 clocks (including JSR)
;;;;;;;;;;;;;;;;;;;;;;;;;
;delay_256x_a_30_clocks:
;	cpx #0
;	beq delay_a_25_clocks
;	;4 cycles done. Loop is 256 cycles
;:       pha
;	 lda #(256-7-2-2-3)
;	 jsr delay_a_25_clocks
;	pla
;	dex
;	beq delay_a_25_clocks ; count as 2
;	bne :-
;	;Loop end is -1+1 = 0 cycles. Total: 4+JSR+RTS = 16
;
; Requires a function called "delay_a_25_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A clocks + overhead
;; Clobbers A. Preserves X,Y.
;; Time: A+25 clocks (including JSR)
;;;;;;;;;;;;;;;;;;;;;;;;;
;                  ;       Cycles              Accumulator         Carry flag
;                  ; 0  1  2  3  4  5  6          (hex)           0 1 2 3 4 5 6
;                  ;
;                  ; 6  6  6  6  6  6  6   00 01 02 03 04 05 06
;:      sbc #7     ; carry set by CMP
;delay_a_25_clocks:
;       cmp #7     ; 2  2  2  2  2  2  2   00 01 02 03 04 05 06   0 0 0 0 0 0 0
;       bcs :-     ; 2  2  2  2  2  2  2   00 01 02 03 04 05 06   0 0 0 0 0 0 0
;       lsr        ; 2  2  2  2  2  2  2   00 00 01 01 02 02 03   0 1 0 1 0 1 0
;       bcs *+2    ; 2  3  2  3  2  3  2   00 00 01 01 02 02 03   0 1 0 1 0 1 0
;       beq :+     ; 3  3  2  2  2  2  2   00 00 01 01 02 02 03   0 1 0 1 0 1 0
;       lsr        ;       2  2  2  2  2         00 00 01 01 01       1 1 0 0 1
;       beq @rts   ;       3  3  2  2  2         00 00 01 01 01       1 1 0 0 1
;       bcc @rts   ;             3  3  2               01 01 01           0 0 1
;:      bne @rts   ; 2  2              3   00 00             01   0 1         0
;@rts:  rts        ; 6  6  6  6  6  6  6   00 00 00 00 01 01 01   0 1 1 1 0 0 1
;; Total cycles:    25 26 27 28 29 30 31
;
; Generated code is:
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 39
; - 7 bytes or less when n <= 1301
; - 8 bytes or less when n <= 2087
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=4, avg=5.8, max=7
; - When 51 <= n <= 75: min=5, avg=5.8, max=7
; - When 76 <= n <= 100: min=5, avg=6.0, max=7
; - When 101 <= n <= 200: min=4, avg=6.0, max=7
; - When 201 <= n <= 300: min=4, avg=6.0, max=7
; - When 301 <= n <= 500: min=4, avg=6.0, max=7
; - When 501 <= n <= 750: min=4, avg=6.0, max=7
; - When 751 <= n <= 1000: min=5, avg=6.0, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.6, max=8
; - When 2001 <= n <= 3000: min=6, avg=7.7, max=9
; - When 3001 <= n <= 5000: min=6, avg=8.5, max=9
; - When 5001 <= n <= 7500: min=7, avg=8.9, max=9
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
.elseif(n = 4 || n = 5 || n = 8 || n = 15 || n = 17 || n = 18 || n = 28 || n = 30 || n = 33 || n = 38)
	; 2..6 bytes
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
.elseif((n > 8 && n < 12) || n = 20 || n = 22)
	; 3..5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 13)
	; 2 bytes
	brk
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 39)
	; 3..6 bytes
	delay_n (n-13)
	brk
.elseif(n = 21 || n = 36)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 31)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
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
	; 6 bytes
	pha
	delay_n (n-3)
.endif
.else
; 40 to 78
.if(n = 40 || n = 42 || n = 45 || n = 47 || n = 60 || n = 67 || n = 70 || n = 75 || n = 77)
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
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
.elseif(n = 52 || n = 54 || n = 59 || n = 72 || n = 74)
	; 5..6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 82 || n = 84 || n = 94 || n = 104 || n = 109 || n = 114 || n = 127 || n = 139 || n = 144 || n = 149 || n = 154)
	; 5..6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 87 || n = 90 || n = 95 || n = 100 || n = 102 || n = 110 || n = 112 || n = 115 || n = 117 || n = 122 || n = 132 || n = 135 || n = 140 || n = 142 || n = 147 || n = 150 || n = 152)
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
.elseif(n = 89)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 4
	Jmi *-3
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
.elseif(n = 105 || n = 145)
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
.if((n >= 157) && (n < 235))
; 157 to 234
.if(n = 157 || n = 160 || n = 165 || n = 172 || n = 175 || n = 180 || n = 182 || n = 187 || n = 192 || n = 195 || n = 200 || n = 205 || n = 207 || n = 210 || n = 212 || n = 220 || n = 222 || n = 227 || n = 230)
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
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
	ldx #216 ;hides 'cld'
	inx
	inx
	Jne *-3
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
.if(n = 235 || n = 237 || n = 240 || n = 245 || n = 247 || n = 250 || n = 252 || n = 255 || n = 262 || n = 270 || n = 272 || n = 275 || n = 277 || n = 285)
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
.elseif(n = 236 || n = 246 || n = 261 || n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 238 || n = 243 || n = 248 || n = 258 || n = 263 || n = 268 || n = 273 || n = 278 || n = 283 || n = 293 || n = 298 || n = 303 || n = 308 || n = 310)
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
	ldy #202 ;hides 'dex'
	iny
	iny
	Jne *-3
.elseif(n = 244 || n = 254 || n = 259 || n = 264 || n = 282 || n = 287 || n = 294)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 249 || n = 257)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 289 || n = 305)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 290)
	; 6 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 292 || n = 297)
	; 6..7 bytes
	brk
	delay_n (n-13)
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
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 315 || n = 318 || n = 322 || n = 328 || n = 333 || n = 335 || n = 338 || n = 340 || n = 343 || n = 348 || n = 350 || n = 353 || n = 355 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 383 || n = 388 || n = 390)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	iny
	iny
	Jne *-3
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 387)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 327 || n = 342 || n = 347 || n = 357 || n = 362)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 319 || n = 324 || n = 334 || n = 339 || n = 349 || n = 354 || n = 364 || n = 374 || n = 380 || n = 382 || n = 384 || n = 389)
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
.elseif(n = 325 || n = 360)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 352)
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
.elseif(n = 385)
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
.endif
.else
; 391 to 468
.if(n = 391 || n = 396 || n = 401 || n = 411 || n = 426 || n = 441 || n = 451 || n = 456 || n = 461)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 392)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 393 || n = 449)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 394 || n = 399 || n = 415 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 395)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	inx
	Jne *-3
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
.elseif(n = 402 || n = 403 || n = 408 || n = 410 || n = 413 || n = 418 || n = 420 || n = 423 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 448 || n = 450 || n = 453 || n = 458 || n = 460 || n = 468)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 404)
	; 6 bytes
	ldx #216 ;hides 'cld'
	dex
	dex
	Jmi *-3
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
.elseif(n = 422 || n = 427 || n = 432 || n = 447 || n = 452 || n = 462)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 430)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 455)
	; 5 bytes
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
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
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 474 || n = 479 || n = 492 || n = 499 || n = 504 || n = 509 || n = 514 || n = 522 || n = 527 || n = 534)
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
	ldy #186 ;hides 'tsx'
	iny
	Jne *-2
.elseif(n = 494)
	; 6 bytes
	ldy #182 ;hides 'ldx zp,y'
	nop
	dey
	Jmi *-3
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
.elseif(n = 502)
	; 7 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 548 || n = 553 || n = 555 || n = 558 || n = 562 || n = 563 || n = 565 || n = 570 || n = 573 || n = 578 || n = 580 || n = 583 || n = 587 || n = 588 || n = 590 || n = 593 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 618 || n = 620 || n = 623)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 549 || n = 559 || n = 564 || n = 572 || n = 574 || n = 579 || n = 584 || n = 592 || n = 594 || n = 599 || n = 604 || n = 614 || n = 619)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 550)
	; 7 bytes
	tay
	lax #2
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 595)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
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
.elseif(n = 627 || n = 628 || n = 630 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 668 || n = 670 || n = 675 || n = 678 || n = 682 || n = 683 || n = 688 || n = 693 || n = 695 || n = 698 || n = 700 || n = 703)
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
.elseif(n = 632 || n = 634 || n = 639 || n = 644 || n = 654 || n = 662 || n = 664 || n = 669 || n = 679 || n = 687 || n = 699)
	; 5..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 637)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 638 || n = 645 || n = 673 || n = 680 || n = 694)
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
.elseif(n = 642 || n = 697)
	; 6..7 bytes
	brk
	delay_n (n-13)
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 649 || n = 689)
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
	; 7 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-3
.elseif(n = 674)
	; 6 bytes
	ldy #202 ;hides 'dex'
	nop
	dey
	Jmi *-3
.elseif(n = 677 || n = 690 || n = 702)
	; 6..7 bytes
	delay_n 6
	delay_n (n-6)
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
.endif
.else
; 704 to 781
.if(n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 730 || n = 739 || n = 749 || n = 754 || n = 759 || n = 772 || n = 774 || n = 779)
	; 6..7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 707 || n = 717 || n = 718 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 752 || n = 753 || n = 758 || n = 760 || n = 763 || n = 765 || n = 768 || n = 770 || n = 773)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 710 || n = 780)
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
	ldx #168 ;hides 'tay'
	dex
	dex
	Jne *-3
.elseif(n = 775)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
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
.elseif(n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 818 || n = 822 || n = 823 || n = 828 || n = 830 || n = 833 || n = 838 || n = 840 || n = 843 || n = 845 || n = 858)
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
	; 7 bytes
	tay
	lax #3
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 809)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 810)
	; 7 bytes
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 839)
	; 5 bytes
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
.elseif(n = 852)
	; 7 bytes
	brk
	delay_n (n-13)
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
.if(n = 938 || n = 943 || n = 948 || n = 950 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1063 || n = 1068 || n = 1073 || n = 1075 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1088 || n = 1090)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || n = 1093))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 949 || n = 954 || n = 959 || n = 964 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1092)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942 || n = 952 || n = 957 || n = 987 || n = 992 || n = 1012 || n = 1020 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1087 || n = 1092)
	; 6..7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 944)
	; 6 bytes
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
.elseif(n = 945 || n = 1049 || n = 1057 || n = 1089)
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
.elseif(n = 982)
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
.elseif(n = 1025)
	; 6 bytes
	ldy #200 ;hides 'iny'
	brk
	Jmi *-3
.elseif(n = 1027)
	; 6 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1067)
	; 7 bytes
	pha
	lax #4
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1110 || n = 1113 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1133 || n = 1138 || n = 1140 || n = 1143 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1158 || n = 1160 || n = 1168 || n = 1172 || n = 1173 || n = 1178 || n = 1180 || n = 1183 || n = 1188 || n = 1190 || n = 1193 || n = 1195 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1228 || n = 1230 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248)
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
.elseif(n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1157 || n = 1167 || n = 1197 || n = 1222 || n = 1227)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 1202)
	; 7 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1252 || n = 1306 || n = 1315)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1263 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1291 || n = 1298 || n = 1307 || n = 1314 || n = 1318 || n = 1319 || n = 1322 || n = 1323 || n = 1326)
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
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1293)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290 || n = 1292 || n = 1316 || n = 1327)
	; 6..7 bytes
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
	ldx #216 ;hides 'cld'
	pha
	dex
	dex
	Jne *-4
.elseif(n = 1299)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1300 || n = 1302)
	; 7..8 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1325)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 6
	inx
	Jpl *-4
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
.if(n = 1329 || n = 1337 || n = 1353 || n = 1361)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 1332 || n = 1343)
	; 7 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-3
.elseif((n .mod 9 = 1) && ((n > 1332 && n < 1361)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1334 || n = 1346 || n = 1348 || n = 1356 || n = 1362 || n = 1364)
	; 7..8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1339)
	; 6 bytes
	ldx #122 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1340 || n = 1344 || n = 1347 || n = 1349 || n = 1354 || n = 1355)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1341)
	; 7 bytes
	ldx #122 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
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
.elseif(n = 1358)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 1363)
	; 7 bytes
	ldx #248 ;hides 'sed'
	nop
	dex
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
	ldx #202 ;hides 'dex'
	brk
	Jmi *-3
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1375 || n = 1379 || n = 1382 || n = 1384 || n = 1389 || n = 1395 || n = 1403)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1369 || n = 1377 || n = 1393)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 1372 || n = 1383 || n = 1388 || n = 1390 || n = 1397 || n = 1402 || n = 1404)
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
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
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
.elseif(n = 1398 || n = 1400)
	; 7..8 bytes
	brk
	delay_n (n-13)
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1410 || n = 1419 || n = 1424 || n = 1431 || n = 1434 || n = 1435 || n = 1438 || n = 1445 || n = 1447 || n = 1448 || n = 1452 || n = 1454 || n = 1462 || n = 1466 || n = 1467 || n = 1470 || n = 1479 || n = 1480 || n = 1482 || n = 1483)
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
.elseif(n = 1412 || n = 1426 || n = 1456)
	; 7..8 bytes
	brk
	delay_n (n-13)
.elseif(n = 1413)
	; 5 bytes
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
.elseif(n = 1414 || n = 1432 || n = 1459 || n = 1468 || n = 1477)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1416 || n = 1418 || n = 1423 || n = 1428 || n = 1444 || n = 1446 || n = 1460 || n = 1474 || n = 1476 || n = 1484)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
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
	ldx #56 ;hides 'sec'
	brk
	inx
	Jpl *-4
.elseif(n = 1440)
	; 7 bytes
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
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
.elseif(n = 1458)
	; 7 bytes
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
.elseif(n = 1463)
	; 7 bytes
	ldx #122 ;hides 'nop'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 1469)
	; 7 bytes
	ldy #232 ;hides 'inx'
	delay_n 7
	dey
	Jmi *-4
.elseif(n = 1472)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
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
.elseif(n = 1487 || n = 1490 || n = 1494 || n = 1498 || n = 1501 || n = 1503 || n = 1507 || n = 1508 || n = 1510 || n = 1515 || n = 1523)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488 || n = 1500 || n = 1502 || n = 1509 || n = 1514 || n = 1516)
	; 6..7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1511)
	; 5 bytes
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
.elseif(n = 1512)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1517)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1518)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1519)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	brk
	Jmi *-4
.endif
.else
; 1524 to 1562
.if(n = 1524 || n = 1526)
	; 7..8 bytes
	brk
	delay_n (n-13)
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
	dex
	Jne *-2
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1528 || n = 1544)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1529)
	; 6 bytes
	ldy #170 ;hides 'tax'
	nop
	dey
	Jne *-3
.elseif(n = 1530 || n = 1552)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 1533 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1557)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1535)
	; 8 bytes
	delay_n (n-1529)
	ldx #191
	pha
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && ((n > 1536 && n < 1562)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 1551)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
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
.if(n = 1563 || n = 1574 || n = 1596 || n = 1607 || n = 1629)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1564 || n = 1566 || n = 1570 || n = 1571 || n = 1579 || n = 1586 || n = 1587 || n = 1592 || n = 1595 || n = 1599 || n = 1602 || n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1620 || n = 1627 || n = 1634 || n = 1635)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1572 || n = 1580 || n = 1588 || n = 1600 || n = 1612 || n = 1614 || n = 1626 || n = 1640)
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
.elseif(n = 1578)
	; 7 bytes
	tay
	lax #6
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 1581)
	; 7 bytes
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1582)
	; 8 bytes
	tay
	pla
	lax #6
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 1584)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1589)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1591 || n = 1631)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
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
.elseif(n = 1615)
	; 7 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1619)
	; 6 bytes
	ldx #180 ;hides 'ldy zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1622 || n = 1636 || n = 1638)
	; 7..8 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.endif
.else
; 1641 to 1718
.if((n .mod 8 = 1) && ((n > 1640 && n < 1666) || (n > 1688 && n < 1714)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1642 || n = 1652 || n = 1656 || n = 1658 || n = 1663 || n = 1668 || n = 1677 || n = 1692 || n = 1698 || n = 1700 || n = 1708 || n = 1712)
	; 7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1647 || n = 1662 || n = 1664 || n = 1669 || n = 1672 || n = 1676 || n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1704 || n = 1710 || n = 1715 || n = 1718)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 1) && (n = 1648 || (n > 1665 && n < 1694) || n = 1711))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1650)
	; 7 bytes
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1651 || n = 1661 || n = 1671 || n = 1691 || n = 1701)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
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
	; 6 bytes
	ldx #90 ;hides 'nop'
	pha
	inx
	Jne *-3
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
.elseif(n = 1678)
	; 7 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 1679)
	; 6 bytes
	ldx #168 ;hides 'tay'
	pha
	dex
	Jne *-3
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1687 || n = 1694)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 1696)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 1699)
	; 6 bytes
	ldy #170 ;hides 'tax'
	pha
	dey
	Jne *-3
.elseif(n = 1703)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
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
.if(n = 1719)
	; 7 bytes
	ldy #170 ;hides 'tax'
	brk
	iny
	Jne *-4
.elseif(n = 1720 || n = 1747 || n = 1756 || n = 1774 || n = 1783 || n = 1792)
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
.elseif(n = 1722 || n = 1725 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1754 || n = 1755 || n = 1762 || n = 1763 || n = 1767 || n = 1770 || n = 1787 || n = 1788 || n = 1795)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724 || n = 1726 || n = 1733 || n = 1738 || n = 1740 || n = 1764 || n = 1775 || n = 1780 || n = 1782 || n = 1784 || n = 1796)
	; 6..7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1735)
	; 5 bytes
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
.elseif(n = 1736 || n = 1748 || n = 1778)
	; 7..8 bytes
	brk
	delay_n (n-13)
.elseif(n = 1741 || n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1742)
	; 7 bytes
	php
	delay_n (n-7)
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
.elseif(n = 1757 || n = 1773)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1759)
	; 7 bytes
	ldx #168 ;hides 'tay'
	brk
	inx
	Jne *-4
.elseif(n = 1760)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
.elseif(n = 1766)
	; 8 bytes
	ldy #200 ;hides 'iny'
	delay_n 26
	Jmi *-5
.elseif(n = 1768)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1776)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1781)
	; 6 bytes
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
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
.if(n = 1797 || n = 1798 || n = 1803 || n = 1814 || n = 1818 || n = 1822 || n = 1830 || n = 1834)
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
.elseif(n = 1801 || n = 1809 || n = 1825 || n = 1833)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1802 || n = 1804 || n = 1812 || n = 1829)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1805)
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
.elseif(n = 1810 || n = 1819 || n = 1828)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
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
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	pha
	dex
	Jne *-4
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
.elseif(n = 1820)
	; 7 bytes
	pha
	ldx #(n-4)/8
	pha
	dex
	Jne *-2
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
.if(n = 1836 || n = 1842 || n = 1852 || n = 1862 || n = 1868 || n = 1874)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1865)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1838)
	; 7 bytes
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1839)
	; 6 bytes
	ldx #184 ;hides 'clv'
	pha
	dex
	Jne *-3
.elseif(n = 1840 || n = 1843 || n = 1848 || n = 1866 || n = 1867)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 1) && ((n > 1840 && n < 1874)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 1850)
	; 8 bytes
	tax
	ldy #(n-5)/9
	pla
	dey
	Jne *-2
	txa
.elseif(n = 1851 || n = 1871)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1854)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1856)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1858)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jmi *-5
.elseif(n = 1859)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	pha
	dey
	Jne *-3
.elseif(n = 1860)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
.if((n >= 1876) && (n < 1915))
; 1876 to 1914
.if(n = 1876 || n = 1884 || n = 1892 || n = 1896 || n = 1908 || n = 1910)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1877)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1878 || n = 1883 || n = 1887 || n = 1894 || n = 1899)
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
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1914)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1910)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1885)
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
.elseif(n = 1893 || n = 1904)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1895)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1898)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
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
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1914)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1926 || n = 1948)
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
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
.elseif(n = 1918 || n = 1927 || n = 1936)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1919 || n = 1924 || n = 1928 || n = 1932 || n = 1940 || n = 1944 || n = 1946)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1920)
	; 7 bytes
	ldx #192 ;hides 'cpy #imm'
	ldy $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif((n .mod 8 = 1) && ((n > 1920 && n < 1954)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1922)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 1923 || n = 1934 || n = 1935 || n = 1938 || n = 1939 || n = 1942 || n = 1947 || n = 1952)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1925)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1930)
	; 8 bytes
	ldx #224 ;hides 'cpx #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dex
	Jmi *-5
.elseif(n = 1931 || n = 1941 || n = 1951)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1933)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1943)
	; 6 bytes
	ldx #216 ;hides 'cld'
	nop
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
.elseif(n = 1955 || n = 1962 || n = 1965 || n = 1974 || n = 1983 || n = 1987)
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
	; 7 bytes
	pha
	ldx #(n-4)/8
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
.elseif(n = 1976 || n = 1986)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
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
.elseif(n = 1994)
	; 8 bytes
	ldy #232 ;hides 'inx'
	jsr @nearby_rts
	dey
	Jmi *-5
.elseif(n = 1995 || n = 1998 || n = 2006 || n = 2007 || n = 2018 || n = 2027)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1996 || n = 2002 || n = 2004 || n = 2020 || n = 2022 || n = 2028)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1997 || n = 2013)
	; 8 bytes
	tsx
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	txs
.elseif(n = 1999)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pha
	inx
	Jne *-3
.elseif(n = 2000)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
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
	ldy $48,X ;hides 'pha'
	dex
	Jne *-4
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
	ldx #168 ;hides 'tay'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2016)
	; 7 bytes
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2019)
	; 6 bytes
	ldy #202 ;hides 'dex'
	pha
	dey
	Jne *-3
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
.elseif(n = 2029)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop
	pha
	Jne *-3
.elseif(n = 2030)
	; 7 bytes
	ldy #203 ;hides 'sbx #imm'
	nop $48,X ;hides 'pha'
	dey
	Jne *-4
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
.elseif(n = 2037 || n = 2038 || n = 2043 || n = 2046 || n = 2051 || n = 2055 || n = 2060 || n = 2064 || n = 2070)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2039)
	; 7 bytes
	ldx #26 ;hides 'nop'
	brk
	inx
	Jpl *-4
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
.elseif(n = 2045)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 4
	dey
	Jne *-4
.elseif(n = 2049)
	; 6 bytes
	ldx #0
	pha
	dex
	Jne *-2
.elseif(n = 2054)
	; 7 bytes
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-4
.elseif(n = 2057)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2059)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
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
.if((n .mod 9 = 1) && ((n > 2070 && n < 2108)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2072 || n = 2074 || n = 2084 || n = 2090 || n = 2108)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2073 || n = 2082 || n = 2083 || n = 2093 || n = 2094 || n = 2097 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
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
	ldx #24 ;hides 'clc'
	brk
	inx
	Jpl *-4
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
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2092)
	; 7 bytes
	tay
	lax #8
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2095)
	; 7 bytes
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2096)
	; 8 bytes
	tay
	pla
	lax #8
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2099)
	; 7 bytes
	ldy #232 ;hides 'inx'
	brk
	dey
	Jmi *-4
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
	; 8 bytes
	ldy #162 ;hides 'ldx #imm'
	ldx $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
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
.elseif(n = 2117 || n = 2119 || n = 2126 || n = 2128 || n = 2138 || n = 2140 || n = 2144)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2118 || n = 2127 || n = 2136)
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
.elseif(n = 2122)
	; 8 bytes
	ldx #100
	delay_n 16
	dex
	Jpl *-4
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
.elseif(n = 2139)
	; 7 bytes
	ldx #234 ;hides 'nop'
	brk
	dex
	Jmi *-4
.elseif(n = 2142)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	brk
	dex
	Jmi *-5
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
.elseif(n = 2150 || n = 2162 || n = 2164 || n = 2174 || n = 2184)
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
.elseif(n = 2153 || n = 2163 || n = 2169 || n = 2172 || n = 2175 || n = 2180 || n = 2187)
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
	ldy $48,X ;hides 'pha'
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
.elseif(n = 2158)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2159)
	; 6 bytes
	ldx #216 ;hides 'cld'
	pha
	dex
	Jne *-3
.elseif(n = 2160)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
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
	ldy $48,X ;hides 'pha'
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
.elseif(n = 2182)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $48,X ;hides 'pha'
	dey
	Jne *-4
.elseif(n = 2183)
	; 7 bytes
	ldx #168 ;hides 'tay'
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 2189 || n = 2198 || n = 2218)
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
.elseif(n = 2216)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2219)
	; 8 bytes
	tax
	ldy #(n-5)/9
	pla
	dey
	Jne *-2
	txa
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
.elseif(n = 2229 || n = 2230 || n = 2265)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2231)
	; 6 bytes
	ldx #248 ;hides 'sed'
	nop
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
.elseif(n = 2235 || n = 2252 || n = 2264)
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
	delay_n (n-7)
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
.elseif(n = 2261)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
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
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
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
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 7 bytes
	ldx #0
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2306 || n = 2307)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2314)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2309)
	; 8 bytes
	ldy #232 ;hides 'inx'
	nop
	brk
	dey
	Jmi *-5
.elseif(n = 2310)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 2311 || n = 2321)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2312)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 2313)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2315)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2316 || n = 2318)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
.if(n = 2324 || n = 2332 || n = 2335 || n = 2343)
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
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
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
.elseif(n = 2336)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
.elseif(n = 2337)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2346 || n = 2354 || n = 2368 || n = 2373)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2348 || n = 2350 || n = 2360 || n = 2372)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2349)
	; 7 bytes
	tay
	lax #9
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2351)
	; 7 bytes
	ldx #168 ;hides 'tay'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 2352)
	; 7 bytes
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2353 || n = 2365 || n = 2377)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2356)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2357)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2358 || n = 2362 || n = 2369 || n = 2374 || n = 2382)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2359)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2361 || n = 2371 || n = 2381)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
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
.elseif(n = 2367)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2370)
	; 8 bytes
	ldx #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 2375)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
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
.elseif(n = 2380)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2390 || n = 2403 || n = 2412 || n = 2415)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2392 || n = 2394 || n = 2400 || n = 2404)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2385 || n = 2396 || n = 2408)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2386 || n = 2416)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2387)
	; 7 bytes
	ldy #200 ;hides 'iny'
	pha
	dey
	dey
	Jne *-4
.elseif(n = 2388 || n = 2410)
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
.elseif(n = 2393 || n = 2406)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2395)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2397)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2398)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2399)
	; 7 bytes
	ldx #136 ;hides 'dey'
	brk
	inx
	Jne *-4
.elseif(n = 2402)
	; 8 bytes
	pha
	ldx #(n-4)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 2414)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 2419)
	; 7 bytes
	ldx #248 ;hides 'sed'
	brk
	dex
	Jmi *-4
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
.if(n = 2422 || n = 2427 || n = 2434 || n = 2453 || n = 2456)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423)
	; 7 bytes
	ldy #202 ;hides 'dex'
	nop
	pha
	dey
	Jne *-4
.elseif(n = 2424 || n = 2426 || n = 2428 || n = 2438 || n = 2440 || n = 2444 || n = 2450 || n = 2457)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2425 || n = 2437 || n = 2449)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2429 || n = 2430 || n = 2455)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2431)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2432 || n = 2443 || n = 2454)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 2436)
	; 8 bytes
	ldy #203 ;hides 'sbx #imm'
	ldx $EA ;hides 'nop'
	pha
	dey
	Jne *-5
.elseif(n = 2439)
	; 7 bytes
	ldx #122 ;hides 'nop'
	brk
	dex
	Jne *-4
.elseif(n = 2441 || n = 2451)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
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
.elseif(n = 2448)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 2452)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2458)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2459)
	; 7 bytes
	ldx #250 ;hides 'nop'
	brk
	dex
	Jmi *-4
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
.elseif(n = 2462)
	; 8 bytes
	pha
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2463 || n = 2469 || n = 2475 || n = 2489)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2464)
	; 8 bytes
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2465)
	; 6 bytes
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
.elseif(n = 2466 || n = 2500)
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
.elseif(n = 2468 || n = 2470 || n = 2474 || n = 2482 || n = 2488 || n = 2494)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2471 || n = 2481 || n = 2491)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2472)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2476 || n = 2487 || n = 2498)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2477 || n = 2480)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2478 || n = 2492)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 2479)
	; 6 bytes
	ldx #248 ;hides 'sed'
	pha
	dex
	Jne *-3
.elseif(n = 2483)
	; 8 bytes
	tsx
	ldy #248 ;hides 'sed'
	pha
	dey
	Jne *-3
	txs
.elseif(n = 2484)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
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
.elseif(n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
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
	ldy $48,X ;hides 'pha'
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
.elseif(n = 2502 || n = 2504 || n = 2512 || n = 2514 || n = 2524 || n = 2532)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2503 || n = 2539)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2505)
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 7
	dey
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
	ldy $48,X ;hides 'pha'
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
.elseif(n = 2510)
	; 8 bytes
	ldx #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 2513 || n = 2522 || n = 2523)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2515 || n = 2516 || (n > 2524 && n < 2528) || n = 2530 || n = 2537 || n = 2538)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2517 || n = 2534)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2518)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	pha
	dex
	Jne *-5
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
.elseif(n = 2535)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2536)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
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
.elseif(n = 2544 || n = 2547 || n = 2552 || n = 2559 || n = 2571)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2545 || n = 2557 || n = 2569)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2546 || n = 2548 || n = 2556 || n = 2558 || n = 2560 || n = 2572 || n = 2576)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2554)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	ldy $48,X ;hides 'pha'
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
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jmi *-5
.elseif(n = 2565)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2570)
	; 8 bytes
	ldx #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
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
	ldx #(n-4)/11
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
	ldx #84 ;hides 'nop zp,x'
	nop
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2580 || n = 2592 || n = 2616)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2583 || n = 2596 || n = 2607 || n = 2613)
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
.elseif(n = 2585)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2586 || n = 2597 || n = 2608)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2587 || n = 2589 || n = 2598 || n = 2600 || n = 2612)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2588 || n = 2614)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2590)
	; 8 bytes
	ldx #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 2591)
	; 7 bytes
	ldx #216 ;hides 'cld'
	nop
	pha
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
.elseif(n = 2601)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
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
	tay
	lax #10
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2609)
	; 7 bytes
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2610)
	; 8 bytes
	tay
	pla
	lax #10
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2611)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2615)
	; 7 bytes
	ldx #218 ;hides 'nop'
	nop
	pha
	dex
	Jne *-4
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2620 || n = 2622 || n = 2644)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2619 || n = 2630 || n = 2652)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2621)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2623 || n = 2642)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2624 || n = 2634 || n = 2635 || n = 2637 || n = 2645 || n = 2648)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2625)
	; 7 bytes
	ldy #202 ;hides 'dex'
	delay_n 6
	dey
	Jne *-4
.elseif(n = 2626 || n = 2656)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2627)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 2631 || n = 2632 || n = 2643 || n = 2649 || n = 2654 || n = 2655)
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
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
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
.elseif(n = 2647)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
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
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if((n > 2656 && n < 2661) || n = 2662 || n = 2669 || n = 2673 || n = 2678 || n = 2690 || n = 2693 || n = 2695)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2661 || n = 2675)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2663 || n = 2674 || n = 2685)
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
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2667 || n = 2676 || n = 2691)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2668 || n = 2688)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2679)
	; 7 bytes
	ldx #122 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 2680)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2681)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
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
.elseif(n = 2698 || n = 2709 || n = 2715 || n = 2720)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2699)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2700 || n = 2702 || n = 2706 || n = 2708 || n = 2723 || n = 2726 || n = 2733 || n = 2734)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
	ldx #250 ;hides 'nop'
	nop
	brk
	dex
	Jmi *-5
.elseif(n = 2710 || n = 2722 || n = 2728 || n = 2732)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2714)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 2716)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2719)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2724)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2727)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2730)
	; 8 bytes
	ldx #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
.if((n >= 2735) && (n < 2754))
; 2735 to 2753
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
.elseif(n = 2737 || n = 2749)
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
.elseif(n = 2739)
	; 7 bytes
	ldy #136 ;hides 'dey'
	nop
	brk
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
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2742)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2743 || n = 2752)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2744)
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
.elseif(n = 2747)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2753)
	; 8 bytes
	delay_n (n-2)
	nop
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
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2756)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2757 || n = 2763 || n = 2764)
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2765)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2766 || n = 2769)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2770)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 2776)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n > 2776 && n < 2781) || n = 2790 || n = 2792 || n = 2803)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 2789)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2832 || n = 2851)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2814 || n = 2818 || n = 2825 || n = 2826 || (n > 2836 && n < 2840) || n = 2842 || n = 2849 || n = 2850)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2815)
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
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2823 || n = 2835 || n = 2847)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2824 || n = 2834 || n = 2844 || n = 2846)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2827)
	; 7 bytes
	ldy #202 ;hides 'dex'
	delay_n 7
	dey
	Jne *-4
.elseif(n = 2828)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 16
	dex
	Jmi *-5
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
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2856 || n = 2862 || n = 2864 || n = 2875 || n = 2877 || n = 2880 || n = 2882 || n = 2886 || n = 2888 || n = 2890)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2857 || n = 2869 || n = 2881)
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
.elseif(n = 2859 || n = 2865 || n = 2868 || n = 2883)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2860 || n = 2872 || n = 2884)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2861)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2863)
	; 7 bytes
	tay
	lax #11
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2866)
	; 7 bytes
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2867)
	; 8 bytes
	tay
	pla
	lax #11
	jsr delay_256x_a_30_clocks
	tya
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
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	pha
	inx
	Jne *-5
.elseif(n = 2874)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2876)
	; 7 bytes
	ldy #136 ;hides 'dey'
	delay_n 16
	Jne *-4
.elseif(n = 2878)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 2879)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 2887)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 2892 || n = 2894 || n = 2897 || n = 2898 || n = 2903 || n = 2904 || n = 2906 || n = 2909 || n = 2910 || n = 2914 || n = 2916 || (n > 2921 && n < 2925))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif((n .mod 12 = 1) && ((n > 2892 && n < 2930)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2895 || n = 2907 || n = 2919)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2896)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2899)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2900)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2901 || n = 2921)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2902 || n = 2918 || n = 2920)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2911)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	delay_n 9
	dey
	Jne *-5
.elseif(n = 2912)
	; 8 bytes
	ldx #224 ;hides 'cpx #imm'
	ldy $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2913)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
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
.endif
.else
; 2930 to 2968
.if(n = 2930 || n = 2932 || n = 2944 || n = 2968)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2931 || n = 2937 || n = 2943 || n = 2967)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2933)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	ldy $48,X ;hides 'pha'
	dex
	dex
	Jne *-5
.elseif(n = 2934)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	pha
	inx
	inx
	Jne *-5
.elseif(n = 2935)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2936 || n = 2945 || (n > 2947 && n < 2951) || n = 2954 || n = 2957 || n = 2958 || n = 2960)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2946)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2947)
	; 8 bytes
	ldx #122 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 2951)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	nop
	pha
	dex
	Jne *-5
.elseif(n = 2952)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
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
	nop
	brk
	dex
	Jne *-4
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
.elseif(n = 2971 || n = 3007)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2972 || n = 2982 || n = 2985 || n = 2990 || n = 2994 || n = 2996 || n = 2998 || n = 3005 || n = 3006)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2973 || n = 2979 || n = 2991 || n = 3003)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2974 || n = 2980 || n = 2992)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2975)
	; 7 bytes
	ldx #248 ;hides 'sed'
	nop
	pha
	dex
	Jne *-4
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
.elseif(n = 2978 || n = 3004)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2981)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2984)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2986)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
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
.endif
.else
; 3008 to 3046
.if(n = 3008 || n = 3014 || n = 3019 || n = 3020 || (n > 3031 && n < 3035) || n = 3038 || n = 3042)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3009 || n = 3027 || n = 3045)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3010)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3011 || n = 3039)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3012)
	; 8 bytes
	cmp <$CD ;hides 'cmp $05A2'
	ldx #5
	pha
	inx
	Jne *-5
.elseif(n = 3013 || n = 3025 || n = 3037)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 3017 || n = 3030)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3018 || n = 3026 || n = 3028 || n = 3040)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3021)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3022)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3023)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3024)
	; 8 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	pha
	inx
	Jne *-5
.elseif(n = 3029)
	; 8 bytes
	ldy #202 ;hides 'dex'
	tax
	pla
	txa
	dey
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
.elseif(n = 3043)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3044)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
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
.elseif(n = 3050 || n = 3055 || n = 3062 || n = 3065 || n = 3070 || n = 3077 || (n > 3082 && n < 3086))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3051)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3053 || n = 3067 || n = 3081)
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
.elseif(n = 3056 || n = 3069 || n = 3082)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3057)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	brk
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
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.elseif(n = 3068)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
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
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3079)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3080)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	inx
	Jne *-5
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3087 || n = 3089 || n = 3090 || n = 3092 || n = 3093 || n = 3096 || (n > 3101 && n < 3106) || n = 3107 || n = 3110 || n = 3111 || n = 3113 || n = 3114 || n = 3116 || n = 3119)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3088)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
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
.elseif(n = 3095 || n = 3109)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3097 || n = 3115)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3098 || n = 3117)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3099 || n = 3122 || n = 3125)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3100 || n = 3118)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3101 || n = 3121)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3108)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3120)
	; 7 bytes
	tay
	lax #12
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3123)
	; 7 bytes
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3124)
	; 8 bytes
	tay
	pla
	lax #12
	jsr delay_256x_a_30_clocks
	tya
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
.if(n = 3126 || n = 3154)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3127)
	; 8 bytes
	ldx #184 ;hides 'clv'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3128 || n = 3131 || n = 3132 || (n > 3137 && n < 3141) || (n > 3141 && n < 3146) || n = 3148 || n = 3149 || n = 3152 || n = 3156 || n = 3157 || n = 3159 || n = 3162)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3129 || n = 3146 || n = 3163)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3130)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3133 || n = 3151)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3134 || n = 3147 || n = 3160)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3135)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3136 || n = 3155)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3137)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3141 || n = 3161)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3150)
	; 8 bytes
	ldy #136 ;hides 'dey'
	nop
	delay_n 16
	Jne *-5
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
	delay_n 6
	dex
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
.elseif(n = 3168 || n = 3175 || n = 3176 || n = 3182 || n = 3185 || n = 3188 || n = 3195 || n = 3198 || n = 3200 || n = 3202 || n = 3203)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3169 || n = 3187)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
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
.elseif(n = 3172)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3173 || n = 3186)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3174)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.elseif(n = 3180 || n = 3197)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3181 || n = 3201)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
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
.elseif(n = 3189)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3190)
	; 8 bytes
	ldx #160 ;hides 'ldy #imm'
	.byte $AC,0,0 ;ldy abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3191)
	; 8 bytes
	ldx #168 ;hides 'tay'
	jsr @nearby_rts
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
.endif
.else
; 3204 to 3281
.if(n = 3204 || n = 3210 || n = 3213 || (n > 3214 && n < 3219) || n = 3220 || n = 3224 || n = 3227 || n = 3228 || n = 3232 || n = 3234 || n = 3236 || n = 3237 || n = 3240 || n = 3242 || n = 3245 || n = 3246 || n = 3253 || n = 3254 || n = 3257 || n = 3258 || n = 3260 || (n > 3265 && n < 3269) || n = 3270 || (n > 3271 && n < 3275))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3205 || n = 3223 || n = 3241 || n = 3259 || n = 3277)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3206)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3207 || n = 3221 || n = 3235 || n = 3263)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3208 || n = 3222 || n = 3244 || n = 3252 || n = 3262 || n = 3278 || n = 3280)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3209)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3211 || n = 3226 || n = 3256 || n = 3271)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3212 || n = 3250 || n = 3269)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 3225 || n = 3243)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3229)
	; 8 bytes
	ldy #170 ;hides 'tax'
	jsr @nearby_rts
	dey
	Jne *-5
.elseif(n = 3230)
	; 8 bytes
	ldy #162 ;hides 'ldx #imm'
	.byte $AE,0,0 ;ldx abs; hides 'brk'
	dey
	Jne *-5
.elseif(n = 3231)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 9
	dey
	Jne *-5
.elseif(n = 3233)
	; 8 bytes
	ldx #232 ;hides 'inx'
	nop
	pha
	dex
	dex
	Jne *-5
.elseif(n = 3238 || n = 3251 || n = 3264)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3239)
	; 8 bytes
	ldx #216 ;hides 'cld'
	nop
	delay_n 6
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
.elseif(n = 3255)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3261 || n = 3281)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3276)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	nop
	brk
	inx
	Jne *-5
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
.if(n = 3282 || n = 3299 || n = 3316 || n = 3350)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283 || n = 3284 || (n > 3291 && n < 3295) || n = 3296 || n = 3304 || n = 3306 || (n > 3307 && n < 3313) || n = 3314 || n = 3318 || n = 3320 || (n > 3322 && n < 3326) || n = 3327 || n = 3328 || n = 3330 || n = 3332 || (n > 3334 && n < 3340) || (n > 3341 && n < 3345) || n = 3348 || (n > 3352 && n < 3359))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3285)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3286 || n = 3346)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3287)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 19
	Jne *-5
.elseif(n = 3288 || n = 3307 || n = 3326 || n = 3345)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 3290 || n = 3303)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3291 || n = 3305 || n = 3333 || n = 3347)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3295 || n = 3313 || n = 3331 || n = 3349)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3297 || n = 3351)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3298 || n = 3340)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3300)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3301 || n = 3321 || n = 3341)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
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
.elseif(n = 3319)
	; 7 bytes
	ldx #90 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 3322 || n = 3334 || n = 3352)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3329)
	; 8 bytes
	ldx #0
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3359)
	; 7 bytes
	ldx #168 ;hides 'tay'
	brk
	dex
	Jne *-4
.endif
.else
; 3360 to 3437
.if(n = 3360 || n = 3363 || n = 3366 || n = 3368 || (n > 3370 && n < 3375) || n = 3378 || n = 3386 || n = 3390 || (n > 3391 && n < 3399) || n = 3400 || n = 3404 || (n > 3407 && n < 3413) || n = 3416 || n = 3419 || n = 3422 || (n > 3424 && n < 3429) || n = 3430 || n = 3432 || n = 3434 || n = 3437)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3361 || n = 3375 || n = 3389 || n = 3417 || n = 3431)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3362 || n = 3370 || n = 3383 || n = 3388)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3364 || n = 3402)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3365)
	; 8 bytes
	ldx #202 ;hides 'dex'
	brk
	dex
	dex
	Jne *-5
.elseif(n = 3367 || n = 3385 || n = 3403 || n = 3421)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3369 || n = 3379 || n = 3382 || n = 3387)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3376 || n = 3406 || n = 3436)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3377)
	; 7 bytes
	tay
	lax #13
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3380)
	; 7 bytes
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3381)
	; 8 bytes
	tay
	pla
	lax #13
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3384 || n = 3401 || n = 3418)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
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
	; 7 bytes
	ldy #170 ;hides 'tax'
	brk
	dey
	Jne *-4
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
.elseif(n = 3424)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	brk
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
	ldx #164 ;hides 'ldy zp'
	.byte $AC,0,0 ;ldy abs; hides 'brk'
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
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if(n = 3438 || n = 3443 || (n > 3445 && n < 3449) || n = 3454 || n = 3456 || n = 3458 || n = 3462 || n = 3463 || n = 3467 || n = 3468 || n = 3470 || n = 3472 || n = 3476 || n = 3480 || (n > 3481 && n < 3485) || (n > 3487 && n < 3493) || n = 3494 || n = 3498 || n = 3500 || (n > 3503 && n < 3507) || n = 3510 || n = 3512)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3439 || n = 3457 || n = 3475 || n = 3493 || n = 3511)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3440 || n = 3478 || n = 3497)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3441 || n = 3461 || n = 3481)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3442 || n = 3453 || n = 3460 || n = 3474 || n = 3502 || n = 3514)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3444)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	nop
	brk
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 3444 && n < 3516)))
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
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 3451 || n = 3466 || n = 3496)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3452 || n = 3469 || n = 3486 || n = 3503)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3455)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 9
	dex
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
	ldx #164 ;hides 'ldy zp'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 3471)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3477 || n = 3513)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3479)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	delay_n 6
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
.elseif(n = 3507)
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	brk
	dey
	Jne *-5
.elseif(n = 3508)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3509)
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	delay_n 6
	dex
	Jne *-5
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3535 || n = 3573 || n = 3592)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif((n > 3516 && n < 3520) || n = 3522 || n = 3523 || n = 3525 || n = 3528 || n = 3530 || n = 3534 || n = 3536 || n = 3540 || n = 3542 || (n > 3543 && n < 3547) || n = 3548 || (n > 3550 && n < 3554) || n = 3555 || (n > 3557 && n < 3561) || (n > 3561 && n < 3565) || n = 3566 || n = 3570 || n = 3572 || (n > 3573 && n < 3581) || n = 3582 || n = 3584 || n = 3587 || (n > 3588 && n < 3592) || n = 3593)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3520 || n = 3537 || n = 3554 || n = 3588)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3521 || n = 3541 || n = 3561 || n = 3581)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3524)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
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
.elseif(n = 3529 || n = 3547 || n = 3565 || n = 3583)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3531 || n = 3549 || n = 3567)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3532 || n = 3568)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3533)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	jsr @nearby_rts
	dey
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
.elseif(n = 3543 || n = 3557 || n = 3571)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3550)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
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
.if((n > 3593 && n < 3601) || n = 3602 || (n > 3605 && n < 3611) || n = 3612 || n = 3614 || n = 3615 || n = 3617 || n = 3618 || n = 3620 || (n > 3622 && n < 3630) || n = 3632 || n = 3633 || n = 3635 || n = 3642 || n = 3643 || n = 3645 || n = 3647 || n = 3648 || n = 3650 || n = 3652 || n = 3654 || (n > 3661 && n < 3666) || n = 3667 || n = 3669 || n = 3670)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3601 || n = 3619 || n = 3655)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3603 || n = 3636 || n = 3639 || n = 3657)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3604 || n = 3640 || n = 3656)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3605 || n = 3622)
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
.elseif(n = 3613)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3616 || n = 3631 || n = 3646)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3621 || n = 3641 || n = 3661)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3634)
	; 7 bytes
	tay
	lax #14
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3637)
	; 7 bytes
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3638)
	; 8 bytes
	tay
	pla
	lax #14
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3644)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3651)
	; 8 bytes
	ldx #90 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 3653)
	; 6 bytes
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
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
.elseif(n = 3660)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3666)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 3671)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 10
	dex
	Jne *-5
.endif
.else
; 3672 to 3750
.if(n = 3672 || n = 3677 || n = 3678 || n = 3680 || (n > 3682 && n < 3687) || n = 3688 || n = 3689 || n = 3692 || n = 3696 || (n > 3697 && n < 3701) || (n > 3701 && n < 3705) || n = 3708 || n = 3710 || (n > 3712 && n < 3718) || n = 3720 || n = 3723 || n = 3726 || n = 3728 || (n > 3730 && n < 3736) || n = 3737 || n = 3738 || n = 3740 || n = 3742 || n = 3746 || n = 3750)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3673 || n = 3691 || n = 3709 || n = 3727 || n = 3745)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3674)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3675 || n = 3693 || n = 3729 || n = 3747)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3676 || n = 3736)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3679)
	; 7 bytes
	ldx #184 ;hides 'clv'
	brk
	dex
	Jne *-4
.elseif(n = 3681 || n = 3701 || n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3682 || n = 3694 || n = 3712 || n = 3722 || n = 3730 || n = 3748)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3687 || n = 3706 || n = 3725 || n = 3744)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3690 || n = 3707 || n = 3724 || n = 3741)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3695)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 3697 || n = 3718)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
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
.elseif(n = 3719)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	brk
	dey
	Jne *-4
.elseif(n = 3739)
	; 8 bytes
	ldy #170 ;hides 'tax'
	nop
	brk
	dey
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
.elseif((n > 3751 && n < 3758) || n = 3762 || n = 3764 || (n > 3766 && n < 3772) || n = 3773 || n = 3774 || (n > 3775 && n < 3781) || (n > 3784 && n < 3791) || (n > 3792 && n < 3796) || n = 3797 || n = 3798 || n = 3800 || n = 3803 || n = 3804 || (n > 3805 && n < 3809) || n = 3810 || (n > 3811 && n < 3817) || n = 3818 || n = 3822 || n = 3824 || n = 3825 || (n > 3826 && n < 3830) || (n > 3830 && n < 3835) || n = 3836 || n = 3840 || n = 3842 || (n > 3844 && n < 3853) || n = 3854 || n = 3857 || n = 3862 || n = 3864 || (n > 3865 && n < 3870) || n = 3872 || n = 3875 || n = 3876 || (n > 3877 && n < 3881) || (n > 3881 && n < 3886) || n = 3888 || n = 3890 || (n > 3897 && n < 3901) || n = 3902 || n = 3904 || n = 3906)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3758 || n = 3775 || n = 3792 || n = 3809 || n = 3826 || n = 3843 || n = 3860 || n = 3877)
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
.elseif(n = 3760 || n = 3802 || n = 3823 || n = 3844 || n = 3865 || n = 3886)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3761)
	; 8 bytes
	ldx #58 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3763 || n = 3781 || n = 3799 || n = 3817 || n = 3835 || n = 3853 || n = 3871 || n = 3889)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3765 || n = 3783 || n = 3819 || n = 3855 || n = 3873 || n = 3893 || n = 3896)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3772)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3782 || n = 3801 || n = 3820 || n = 3839 || n = 3858)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3784 || n = 3838 || n = 3859 || n = 3874 || n = 3892 || n = 3897)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3791)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3805)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3821 || n = 3841 || n = 3861 || n = 3881 || n = 3901)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3830)
	; 8 bytes
	ldx #192 ;hides 'cpy #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dex
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
.elseif(n = 3870)
	; 8 bytes
	ldx #194 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3887)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3891)
	; 7 bytes
	tay
	lax #15
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3894)
	; 7 bytes
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3895)
	; 8 bytes
	tay
	pla
	lax #15
	jsr delay_256x_a_30_clocks
	tya
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
.else
; 3907 to 4062
.if(n = 3907 || n = 3925 || n = 3943 || n = 3961 || n = 3979 || n = 3997 || n = 4015 || n = 4033 || n = 4051)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3908 || (n > 3911 && n < 3915) || (n > 3915 && n < 3919) || n = 3920 || (n > 3921 && n < 3925) || n = 3926 || (n > 3928 && n < 3934) || n = 3935 || n = 3936 || (n > 3937 && n < 3941) || n = 3942 || n = 3944 || n = 3947 || (n > 3949 && n < 3953) || (n > 3953 && n < 3959) || n = 3960 || n = 3965 || n = 3966 || n = 3968 || n = 3971 || (n > 3972 && n < 3977) || n = 3978 || n = 3980 || (n > 3982 && n < 3991) || (n > 3992 && n < 3996) || n = 3998 || (n > 4003 && n < 4010) || n = 4011 || n = 4014 || n = 4016 || n = 4019 || n = 4020 || (n > 4021 && n < 4025) || (n > 4025 && n < 4029) || n = 4031 || n = 4032 || n = 4034 || n = 4037 || n = 4038 || n = 4040 || (n > 4042 && n < 4047) || n = 4049 || n = 4052 || (n > 4054 && n < 4059) || n = 4060)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3909 || n = 3927 || n = 3963 || n = 4017 || n = 4035 || n = 4053)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3946 || n = 3964 || n = 3982 || n = 4000 || n = 4002 || n = 4018 || n = 4036 || n = 4042 || n = 4062)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3996 || n = 4013 || n = 4030 || n = 4047)
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
.elseif(n = 3919)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3921 || n = 3941 || n = 3981 || n = 4001 || n = 4021 || n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3937)
	; 8 bytes
	ldx #232 ;hides 'inx'
	brk
	inx
	inx
	Jne *-5
.elseif(n = 3948)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	.byte $AC,0,0 ;ldy abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3949 || n = 3970 || n = 4012 || n = 4054)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3959)
	; 7 bytes
	ldx #58 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 3967)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3969)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3977)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3992)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	.byte $AE,0,0 ;ldx abs; hides 'brk'
	dey
	Jne *-5
.elseif(n = 3999)
	; 7 bytes
	ldx #56 ;hides 'sec'
	brk
	inx
	Jne *-4
.elseif(n = 4003)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	nop
	brk
	dey
	Jne *-5
.elseif(n = 4025)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	brk
	dey
	Jne *-5
.elseif(n = 4039)
	; 7 bytes
	ldy #202 ;hides 'dex'
	brk
	dey
	Jne *-4
.elseif(n = 4050)
	; 8 bytes
	ldy #203 ;hides 'sbx #imm'
	.byte $AE,0,0 ;ldx abs; hides 'brk'
	dey
	Jne *-5
.elseif(n = 4059)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	brk
	Jne *-4
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
; 4063 to 4218
.if(n = 4063 || n = 4065 || n = 4066 || n = 4068 || n = 4070 || n = 4073 || n = 4074 || (n > 4075 && n < 4081) || (n > 4081 && n < 4086) || n = 4088 || (n > 4091 && n < 4095) || n = 4097 || n = 4099 || n = 4100 || n = 4102 || n = 4104 || n = 4106 || (n > 4108 && n < 4115) || (n > 4117 && n < 4121) || n = 4122 || (n > 4126 && n < 4132) || (n > 4132 && n < 4137) || n = 4139 || n = 4140 || n = 4142 || (n > 4144 && n < 4148) || n = 4155 || n = 4156 || n = 4160 || (n > 4162 && n < 4166) || (n > 4166 && n < 4177) || n = 4178 || n = 4182 || (n > 4183 && n < 4195) || n = 4196 || (n > 4201 && n < 4213) || n = 4214 || n = 4218)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132 || n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4067 || n = 4086 || n = 4124 || n = 4143 || n = 4162 || n = 4181)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4069 || n = 4087 || n = 4105 || n = 4123 || n = 4141 || n = 4159 || n = 4177 || n = 4195 || n = 4213)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4071 || n = 4089 || n = 4107 || n = 4125 || n = 4150 || n = 4153 || n = 4179 || n = 4197 || n = 4215)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4072 || n = 4090 || n = 4108 || n = 4144 || n = 4154 || n = 4198 || n = 4216)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4075 || n = 4096 || n = 4117 || n = 4138 || n = 4180)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4091)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	nop
	brk
	dey
	Jne *-5
.elseif(n = 4095)
	; 8 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4101 || n = 4121 || n = 4161 || n = 4201)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4103)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	dex
	Jne *-5
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
.elseif(n = 4148)
	; 7 bytes
	tay
	lax #16
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4151)
	; 7 bytes
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4152)
	; 8 bytes
	tay
	pla
	lax #16
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4157)
	; 8 bytes
	ldx #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4158)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.endif
.else
; 4219 to 4375
.if(n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295 || n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4220 || (n > 4222 && n < 4231) || n = 4232 || (n > 4234 && n < 4238) || n = 4239 || n = 4240 || n = 4242 || n = 4244 || n = 4245 || n = 4247 || n = 4248 || n = 4250 || (n > 4252 && n < 4257) || (n > 4257 && n < 4261) || n = 4263 || n = 4266 || (n > 4270 && n < 4276) || (n > 4277 && n < 4281) || (n > 4281 && n < 4285) || n = 4286 || (n > 4288 && n < 4295) || (n > 4295 && n < 4301) || n = 4304 || (n > 4306 && n < 4314) || (n > 4314 && n < 4319) || n = 4320 || n = 4325 || n = 4326 || (n > 4327 && n < 4333) || n = 4334 || n = 4335 || n = 4337 || n = 4338 || n = 4340 || (n > 4342 && n < 4348) || (n > 4348 && n < 4352) || n = 4354 || n = 4356 || n = 4358 || (n > 4362 && n < 4369) || n = 4370 || (n > 4371 && n < 4375))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4221 || n = 4261 || n = 4281 || n = 4301 || n = 4341 || n = 4361)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4222 || n = 4243 || n = 4264 || n = 4306 || n = 4327 || n = 4348)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4231 || n = 4249 || n = 4267 || n = 4285 || n = 4303 || n = 4321 || n = 4339 || n = 4357 || n = 4375)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4233 || n = 4269 || n = 4287 || n = 4305 || n = 4323)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4234 || n = 4251 || n = 4268 || n = 4302 || n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4241)
	; 8 bytes
	ldy #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
.elseif(n = 4246)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 26
	Jne *-5
.elseif(n = 4252 || n = 4265 || n = 4270 || n = 4288 || n = 4322 || n = 4324 || n = 4342 || n = 4360 || n = 4362)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4262)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-4
.elseif(n = 4277)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	delay_n 16
	dey
	Jne *-5
.elseif(n = 4319)
	; 7 bytes
	ldx #216 ;hides 'cld'
	brk
	dex
	Jne *-4
.elseif(n = 4353)
	; 8 bytes
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4355)
	; 8 bytes
	ldx #58 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4359)
	; 7 bytes
	ldx #218 ;hides 'nop'
	brk
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
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4379 || n = 4380 || (n > 4381 && n < 4390) || n = 4391 || n = 4392 || n = 4394 || n = 4397 || n = 4398 || n = 4400 || (n > 4401 && n < 4405) || n = 4406 || n = 4412 || (n > 4414 && n < 4421) || (n > 4421 && n < 4428) || n = 4430 || (n > 4432 && n < 4441) || n = 4442 || n = 4444 || n = 4446 || n = 4448 || n = 4451 || n = 4452 || (n > 4453 && n < 4461) || (n > 4461 && n < 4465) || n = 4469 || (n > 4470 && n < 4474) || n = 4475 || (n > 4476 && n < 4481) || n = 4482 || n = 4484 || (n > 4487 && n < 4495) || (n > 4495 && n < 4501) || n = 4502 || (n > 4504 && n < 4509) || (n > 4510 && n < 4516) || n = 4517 || n = 4518 || n = 4520 || (n > 4523 && n < 4532))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4377 || n = 4395 || n = 4407 || n = 4410 || n = 4413 || n = 4431 || n = 4449 || n = 4467 || n = 4503)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4378 || n = 4396 || n = 4414 || n = 4450 || n = 4468 || n = 4486 || n = 4522)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4381 || n = 4401 || n = 4421 || n = 4441 || n = 4461 || n = 4481 || n = 4521)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4390 || n = 4428 || n = 4466 || n = 4485 || n = 4504 || n = 4523)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 4405)
	; 7 bytes
	tay
	lax #17
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4408)
	; 7 bytes
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4409)
	; 8 bytes
	tay
	pla
	lax #17
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4432 || n = 4453 || n = 4474 || n = 4495 || n = 4516)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4443)
	; 8 bytes
	ldy #202 ;hides 'dex'
	nop
	brk
	dey
	Jne *-5
.elseif(n = 4445)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4470)
	; 8 bytes
	ldx #224 ;hides 'cpx #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4476)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4487)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4509)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4510)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if((n > 4531 && n < 4535) || n = 4536 || n = 4538 || (n > 4542 && n < 4553) || n = 4554 || n = 4556 || n = 4559 || n = 4560 || (n > 4561 && n < 4573) || n = 4574 || n = 4578 || (n > 4587 && n < 4591) || n = 4592 || (n > 4594 && n < 4598) || (n > 4605 && n < 4609))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4535)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4537 || n = 4555 || n = 4573 || n = 4591)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4539 || n = 4557 || n = 4575 || n = 4582 || n = 4584 || n = 4585 || n = 4593 || n = 4600 || n = 4602 || n = 4603)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4540 || n = 4576 || n = 4583 || n = 4594 || n = 4601)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4541 || n = 4581)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4542 || n = 4561)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4553)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 16
	inx
	Jne *-5
.elseif(n = 4558 || n = 4579)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4577)
	; 8 bytes
	ldx #218 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4580)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 4586 || n = 4604)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4587 || n = 4605)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4598)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
.elseif(n = 4599)
	; 7 bytes
	ldx #26 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 4609)
	; 7 bytes
	ldx #0
	brk
	dex
	Jne *-3
.endif
.else
; 4610 to 4687
.if(n = 4610 || (n > 4612 && n < 4618) || n = 4619 || (n > 4621 && n < 4637) || n = 4638 || n = 4640 || n = 4643 || n = 4644 || (n > 4645 && n < 4652) || (n > 4652 && n < 4656) || (n > 4656 && n < 4661) || (n > 4668 && n < 4674) || (n > 4675 && n < 4679) || n = 4680 || n = 4683 || n = 4686 || n = 4687)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4611 || n = 4664 || n = 4667)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4668 || n = 4682)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4618 || n = 4637 || n = 4656 || n = 4675)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4620)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4621 || n = 4641 || n = 4661 || n = 4681)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4639)
	; 7 bytes
	ldx #24 ;hides 'clc'
	brk
	inx
	Jne *-4
.elseif(n = 4642 || n = 4684)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4645)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 16
	dey
	Jne *-5
.elseif(n = 4652)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4662)
	; 7 bytes
	tay
	lax #18
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4663)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4665)
	; 7 bytes
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4666)
	; 8 bytes
	tay
	pla
	lax #18
	jsr delay_256x_a_30_clocks
	tya
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
.endif
.else
; 4688 to 5000
.if((n > 4687 && n < 4694) || (n > 4694 && n < 4701) || (n > 4701 && n < 4705) || n = 4706 || (n > 4707 && n < 4711) || n = 4712 || (n > 4713 && n < 4721) || (n > 4721 && n < 4726) || (n > 4726 && n < 4732) || (n > 4732 && n < 4741) || (n > 4741 && n < 4747) || n = 4748 || n = 4750 || (n > 4751 && n < 4761) || (n > 4761 && n < 4767) || n = 4769 || (n > 4770 && n < 4777) || (n > 4777 && n < 4781) || (n > 4781 && n < 4788) || (n > 4789 && n < 4795) || (n > 4795 && n < 4801) || (n > 4801 && n < 4808) || (n > 4810 && n < 4821) || (n > 4821 && n < 4827) || n = 4828 || n = 4830 || (n > 4831 && n < 4841) || (n > 4841 && n < 4846) || (n > 4846 && n < 4852) || (n > 4852 && n < 4861) || (n > 4861 && n < 4865) || (n > 4865 && n < 4871) || n = 4872 || (n > 4873 && n < 4881) || (n > 4881 && n < 4894) || (n > 4894 && n < 4901) || (n > 4901 && n < 4913) || n = 4914 || (n > 4915 && n < 4919) || n = 4920 || (n > 4925 && n < 4936) || (n > 4936 && n < 4941) || (n > 4941 && n < 4957) || n = 4958 || n = 4960 || (n > 4962 && n < 4967) || (n > 4967 && n < 4978) || n = 4979 || n = 4980 || (n > 4981 && n < 4999) || n = 5000)
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4694 || n = 4713 || n = 4732 || n = 4751 || n = 4770 || n = 4789 || n = 4808 || n = 4827 || n = 4846)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4701 || n = 4721 || n = 4741 || n = 4761 || n = 4781 || n = 4801 || n = 4821 || n = 4841 || n = 4861 || n = 4881 || n = 4901 || n = 4941 || n = 4961 || n = 4981)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4705 || n = 4726 || n = 4747 || n = 4768 || n = 4810 || n = 4831 || n = 4852 || n = 4873 || n = 4894 || n = 4915 || n = 4936 || n = 4957 || n = 4978)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4707)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4711)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4767)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4777)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4788)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4795)
	; 8 bytes
	ldx #218 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4809)
	; 8 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	brk
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
.elseif(n = 4919)
	; 7 bytes
	tay
	lax #19
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4921 || n = 4924)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4922)
	; 7 bytes
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4923)
	; 8 bytes
	tay
	pla
	lax #19
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4925 || n = 4962)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4959)
	; 7 bytes
	ldx #248 ;hides 'sed'
	brk
	dex
	Jne *-4
.elseif(n = 4967)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4999)
	; 7 bytes
	ldx #250 ;hides 'nop'
	brk
	dex
	Jne *-4
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
; 5001 to 5312
.if(n = 5001 || n = 5021 || n = 5041 || n = 5061 || n = 5081 || n = 5101)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5002 || n = 5093 || n = 5113 || n = 5182)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n > 5002 && n < 5013) || (n > 5013 && n < 5020) || (n > 5021 && n < 5041) || (n > 5041 && n < 5059) || n = 5060 || (n > 5062 && n < 5074) || (n > 5074 && n < 5081) || n = 5082 || (n > 5083 && n < 5090) || n = 5091 || (n > 5093 && n < 5101) || n = 5102 || (n > 5104 && n < 5109) || (n > 5113 && n < 5121) || (n > 5121 && n < 5125) || (n > 5125 && n < 5146) || (n > 5147 && n < 5167) || n = 5168 || (n > 5169 && n < 5176) || n = 5177 || (n > 5182 && n < 5188) || n = 5189 || n = 5190 || (n > 5191 && n < 5207) || n = 5208 || (n > 5209 && n < 5213) || (n > 5213 && n < 5230) || (n > 5230 && n < 5249) || n = 5250 || (n > 5251 && n < 5272) || (n > 5272 && n < 5289) || (n > 5289 && n < 5293) || (n > 5293 && n < 5313))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5013)
	; 8 bytes
	ldx #218 ;hides 'nop'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5020 || n = 5062 || n = 5083 || n = 5104 || n = 5125 || n = 5146 || n = 5167 || n = 5188 || n = 5209 || n = 5230 || n = 5251 || n = 5272 || n = 5293)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5059)
	; 8 bytes
	ldx #26 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
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
.elseif(n = 5092 || n = 5111 || n = 5112 || n = 5178 || n = 5181)
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
.elseif(n = 5169)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5176)
	; 7 bytes
	tay
	lax #20
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5179)
	; 7 bytes
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5180)
	; 8 bytes
	tay
	pla
	lax #20
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5191)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5207)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5213)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5249)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5289)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 16
	inx
	Jne *-5
.endif
.else
; 5313 to 5625
.if(n = 5313 || (n > 5314 && n < 5335) || (n > 5335 && n < 5345) || n = 5346 || (n > 5348 && n < 5356) || (n > 5356 && n < 5364) || (n > 5369 && n < 5377) || (n > 5377 && n < 5381) || (n > 5381 && n < 5389) || (n > 5389 && n < 5433) || n = 5434 || (n > 5439 && n < 5455) || (n > 5455 && n < 5499) || (n > 5499 && n < 5600) || (n > 5600 && n < 5621) || (n > 5622 && n < 5626))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5314 || n = 5335)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5345)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 5347 || n = 5368 || n = 5435 || n = 5438)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5348 || n = 5369 || n = 5439)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5356)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 5364)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	cmp $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 5365)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	ldy $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 5366)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 5367)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5377)
	; 8 bytes
	ldx #0
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5381)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5389)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 5433)
	; 7 bytes
	tay
	lax #21
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5436)
	; 7 bytes
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5437)
	; 8 bytes
	tay
	pla
	lax #21
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5455)
	; 8 bytes
	ldx #248 ;hides 'sed'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5499)
	; 8 bytes
	ldx #250 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5600)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 4
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
	delay_n 4
	nop
	dex
	Jne *-5
.endif
.endif
.else
; 5626 to 6250
.if((n > 5625 && n < 5690) || n = 5691 || (n > 5696 && n < 5703) || (n > 5703 && n < 5749) || (n > 5749 && n < 5855) || (n > 5855 && n < 5876) || (n > 5878 && n < 5947) || n = 5948 || (n > 5953 && n < 6110) || (n > 6110 && n < 6132) || (n > 6134 && n < 6204) || n = 6205 || (n > 6210 && n < 6251))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5690)
	; 7 bytes
	tay
	lax #22
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5692 || n = 5695 || n = 5949 || n = 5952 || n = 6206 || n = 6209)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5693)
	; 7 bytes
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5694)
	; 8 bytes
	tay
	pla
	lax #22
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5696 || n = 5953 || n = 6210)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5703)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5855)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	pha
	dex
	Jne *-5
.elseif(n = 5876)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $48 ;hides 'pha'
	nop
	dex
	Jne *-5
.elseif(n = 5877)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 5878)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	pha
	dex
	Jne *-5
.elseif(n = 5947)
	; 7 bytes
	tay
	lax #23
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5950)
	; 7 bytes
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5951)
	; 8 bytes
	tay
	pla
	lax #23
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6110)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 6132)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 6133)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	ldy $48,X ;hides 'pha'
	pha
	inx
	Jne *-5
.elseif(n = 6134)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 6204)
	; 7 bytes
	tay
	lax #24
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6207)
	; 7 bytes
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6208)
	; 8 bytes
	tay
	pla
	lax #24
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6292) || (n > 6292 && n < 6365) || (n > 6365 && n < 6390) || (n > 6390 && n < 6461) || n = 6462 || (n > 6467 && n < 6718) || n = 6719 || (n > 6724 && n < 6975) || n = 6976 || (n > 6981 && n < 7232) || n = 7233 || (n > 7238 && n < 7489) || n = 7490 || (n > 7495 && n < 7501))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 6390)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 6461)
	; 7 bytes
	tay
	lax #25
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6463 || n = 6466 || n = 6720 || n = 6723 || n = 6977 || n = 6980 || n = 7234 || n = 7237 || n = 7491 || n = 7494)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6464)
	; 7 bytes
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6465)
	; 8 bytes
	tay
	pla
	lax #25
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6467 || n = 6724 || n = 6981 || n = 7238 || n = 7495)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6718)
	; 7 bytes
	tay
	lax #26
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6721)
	; 7 bytes
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6722)
	; 8 bytes
	tay
	pla
	lax #26
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6975)
	; 7 bytes
	tay
	lax #27
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6978)
	; 7 bytes
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6979)
	; 8 bytes
	tay
	pla
	lax #27
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7232)
	; 7 bytes
	tay
	lax #28
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7235)
	; 7 bytes
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7236)
	; 8 bytes
	tay
	pla
	lax #28
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7489)
	; 7 bytes
	tay
	lax #29
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7492)
	; 7 bytes
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7493)
	; 8 bytes
	tay
	pla
	lax #29
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
; 7501 to 8750
.if((n > 7500 && n < 7746) || n = 7747 || (n > 7752 && n < 7895) || (n > 7895 && n < 7915) || (n > 7915 && n < 7926) || (n > 7926 && n < 8003) || n = 8004 || (n > 8009 && n < 8260) || n = 8261 || (n > 8266 && n < 8517) || n = 8518 || (n > 8523 && n < 8751))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7746)
	; 7 bytes
	tay
	lax #30
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7748 || n = 7751 || n = 8005 || n = 8008 || n = 8262 || n = 8265 || n = 8519 || n = 8522)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 7749)
	; 7 bytes
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7750)
	; 8 bytes
	tay
	pla
	lax #30
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7752 || n = 8009 || n = 8266 || n = 8523)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 8003)
	; 7 bytes
	tay
	lax #31
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8006)
	; 7 bytes
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8007)
	; 8 bytes
	tay
	pla
	lax #31
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8260)
	; 7 bytes
	tay
	lax #32
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8263)
	; 7 bytes
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8264)
	; 8 bytes
	tay
	pla
	lax #32
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8517)
	; 7 bytes
	tay
	lax #33
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8520)
	; 7 bytes
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8521)
	; 8 bytes
	tay
	pla
	lax #33
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8774) || n = 8775 || (n > 8780 && n < 9031) || n = 9032 || (n > 9037 && n < 9288) || n = 9289 || (n > 9294 && n < 9545) || n = 9546 || (n > 9551 && n < 9802) || n = 9803 || (n > 9808 && n < 10001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8774)
	; 7 bytes
	tay
	lax #34
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8776 || n = 8779 || n = 9033 || n = 9036 || n = 9290 || n = 9293 || n = 9547 || n = 9550 || n = 9804 || n = 9807)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8777)
	; 7 bytes
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8778)
	; 8 bytes
	tay
	pla
	lax #34
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8780 || n = 9037 || n = 9294 || n = 9551 || n = 9808)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 9031)
	; 7 bytes
	tay
	lax #35
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9034)
	; 7 bytes
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9035)
	; 8 bytes
	tay
	pla
	lax #35
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9288)
	; 7 bytes
	tay
	lax #36
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9291)
	; 7 bytes
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9292)
	; 8 bytes
	tay
	pla
	lax #36
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9545)
	; 7 bytes
	tay
	lax #37
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9548)
	; 7 bytes
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9549)
	; 8 bytes
	tay
	pla
	lax #37
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9802)
	; 7 bytes
	tay
	lax #38
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9805)
	; 7 bytes
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9806)
	; 8 bytes
	tay
	pla
	lax #38
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
.if((n >= 10001) && (n < 11251))
; 10001 to 11250
.if((n > 10000 && n < 10059) || n = 10060 || (n > 10065 && n < 10316) || n = 10317 || (n > 10322 && n < 10573) || n = 10574 || (n > 10579 && n < 10830) || n = 10831 || (n > 10836 && n < 11087) || n = 11088 || (n > 11093 && n < 11251))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10059)
	; 7 bytes
	tay
	lax #39
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10061 || n = 10064 || n = 10318 || n = 10321 || n = 10575 || n = 10578 || n = 10832 || n = 10835 || n = 11089 || n = 11092)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10062)
	; 7 bytes
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10063)
	; 8 bytes
	tay
	pla
	lax #39
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10065 || n = 10322 || n = 10579 || n = 10836 || n = 11093)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10316)
	; 7 bytes
	tay
	lax #40
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10319)
	; 7 bytes
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10320)
	; 8 bytes
	tay
	pla
	lax #40
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10573)
	; 7 bytes
	tay
	lax #41
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10576)
	; 7 bytes
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10577)
	; 8 bytes
	tay
	pla
	lax #41
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10830)
	; 7 bytes
	tay
	lax #42
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10833)
	; 7 bytes
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10834)
	; 8 bytes
	tay
	pla
	lax #42
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11087)
	; 7 bytes
	tay
	lax #43
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11090)
	; 7 bytes
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11091)
	; 8 bytes
	tay
	pla
	lax #43
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 11251 to 12500
.if((n > 11250 && n < 11344) || n = 11345 || (n > 11350 && n < 11601) || n = 11602 || (n > 11607 && n < 11858) || n = 11859 || (n > 11864 && n < 12115) || n = 12116 || (n > 12121 && n < 12372) || n = 12373 || (n > 12378 && n < 12501))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11344)
	; 7 bytes
	tay
	lax #44
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11346 || n = 11349 || n = 11603 || n = 11606 || n = 11860 || n = 11863 || n = 12117 || n = 12120 || n = 12374 || n = 12377)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 11347)
	; 7 bytes
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11348)
	; 8 bytes
	tay
	pla
	lax #44
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11350 || n = 11607 || n = 11864 || n = 12121 || n = 12378)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11601)
	; 7 bytes
	tay
	lax #45
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11604)
	; 7 bytes
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11605)
	; 8 bytes
	tay
	pla
	lax #45
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11858)
	; 7 bytes
	tay
	lax #46
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11861)
	; 7 bytes
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11862)
	; 8 bytes
	tay
	pla
	lax #46
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12115)
	; 7 bytes
	tay
	lax #47
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12118)
	; 7 bytes
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12119)
	; 8 bytes
	tay
	pla
	lax #47
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12372)
	; 7 bytes
	tay
	lax #48
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12375)
	; 7 bytes
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12376)
	; 8 bytes
	tay
	pla
	lax #48
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.else
.if((n >= 12501) && (n < 13751))
; 12501 to 13750
.if((n > 12500 && n < 12629) || n = 12630 || (n > 12635 && n < 12886) || n = 12887 || (n > 12892 && n < 13143) || n = 13144 || (n > 13149 && n < 13400) || n = 13401 || (n > 13406 && n < 13657) || n = 13658 || (n > 13663 && n < 13751))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12629)
	; 7 bytes
	tay
	lax #49
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12631 || n = 12634 || n = 12888 || n = 12891 || n = 13145 || n = 13148 || n = 13402 || n = 13405 || n = 13659 || n = 13662)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 12632)
	; 7 bytes
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12633)
	; 8 bytes
	tay
	pla
	lax #49
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12635 || n = 12892 || n = 13149 || n = 13406 || n = 13663)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12886)
	; 7 bytes
	tay
	lax #50
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12889)
	; 7 bytes
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12890)
	; 8 bytes
	tay
	pla
	lax #50
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13143)
	; 7 bytes
	tay
	lax #51
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13146)
	; 7 bytes
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13147)
	; 8 bytes
	tay
	pla
	lax #51
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13400)
	; 7 bytes
	tay
	lax #52
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13403)
	; 7 bytes
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13404)
	; 8 bytes
	tay
	pla
	lax #52
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13657)
	; 7 bytes
	tay
	lax #53
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13660)
	; 7 bytes
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13661)
	; 8 bytes
	tay
	pla
	lax #53
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 13751 to 15000
.if((n > 13750 && n < 13914) || n = 13915 || (n > 13920 && n < 14171) || n = 14172 || (n > 14177 && n < 14428) || n = 14429 || (n > 14434 && n < 14685) || n = 14686 || (n > 14691 && n < 14942) || n = 14943 || (n > 14948 && n < 15001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13914)
	; 7 bytes
	tay
	lax #54
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13916 || n = 13919 || n = 14173 || n = 14176 || n = 14430 || n = 14433 || n = 14687 || n = 14690 || n = 14944 || n = 14947)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13917)
	; 7 bytes
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13918)
	; 8 bytes
	tay
	pla
	lax #54
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13920 || n = 14177 || n = 14434 || n = 14691 || n = 14948)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14171)
	; 7 bytes
	tay
	lax #55
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14174)
	; 7 bytes
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14175)
	; 8 bytes
	tay
	pla
	lax #55
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14428)
	; 7 bytes
	tay
	lax #56
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14431)
	; 7 bytes
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14432)
	; 8 bytes
	tay
	pla
	lax #56
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14685)
	; 7 bytes
	tay
	lax #57
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14688)
	; 7 bytes
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14689)
	; 8 bytes
	tay
	pla
	lax #57
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14942)
	; 7 bytes
	tay
	lax #58
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14945)
	; 7 bytes
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14946)
	; 8 bytes
	tay
	pla
	lax #58
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
; 15001 to 16250
.if((n > 15000 && n < 15199) || n = 15200 || (n > 15205 && n < 15456) || n = 15457 || (n > 15462 && n < 15713) || n = 15714 || (n > 15719 && n < 15970) || n = 15971 || (n > 15976 && n < 16227) || n = 16228 || (n > 16233 && n < 16251))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15199)
	; 7 bytes
	tay
	lax #59
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15201 || n = 15204 || n = 15458 || n = 15461 || n = 15715 || n = 15718 || n = 15972 || n = 15975 || n = 16229 || n = 16232)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15202)
	; 7 bytes
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15203)
	; 8 bytes
	tay
	pla
	lax #59
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15205 || n = 15462 || n = 15719 || n = 15976 || n = 16233)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15456)
	; 7 bytes
	tay
	lax #60
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15459)
	; 7 bytes
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15460)
	; 8 bytes
	tay
	pla
	lax #60
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15713)
	; 7 bytes
	tay
	lax #61
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15716)
	; 7 bytes
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15717)
	; 8 bytes
	tay
	pla
	lax #61
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15970)
	; 7 bytes
	tay
	lax #62
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15973)
	; 7 bytes
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15974)
	; 8 bytes
	tay
	pla
	lax #62
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16227)
	; 7 bytes
	tay
	lax #63
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16230)
	; 7 bytes
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16231)
	; 8 bytes
	tay
	pla
	lax #63
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 16251 to 17500
.if((n > 16250 && n < 16484) || n = 16485 || (n > 16490 && n < 16741) || n = 16742 || (n > 16747 && n < 16998) || n = 16999 || (n > 17004 && n < 17255) || n = 17256 || (n > 17261 && n < 17501))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16484)
	; 7 bytes
	tay
	lax #64
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16486 || n = 16489 || n = 16743 || n = 16746 || n = 17000 || n = 17003 || n = 17257 || n = 17260)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 16487)
	; 7 bytes
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16488)
	; 8 bytes
	tay
	pla
	lax #64
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16490 || n = 16747 || n = 17004 || n = 17261)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16741)
	; 7 bytes
	tay
	lax #65
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16744)
	; 7 bytes
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16745)
	; 8 bytes
	tay
	pla
	lax #65
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16998)
	; 7 bytes
	tay
	lax #66
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17001)
	; 7 bytes
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17002)
	; 8 bytes
	tay
	pla
	lax #66
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17255)
	; 7 bytes
	tay
	lax #67
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17258)
	; 7 bytes
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17259)
	; 8 bytes
	tay
	pla
	lax #67
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.else
.if((n >= 17501) && (n < 18751))
; 17501 to 18750
.if((n > 17500 && n < 17512) || n = 17513 || (n > 17518 && n < 17769) || n = 17770 || (n > 17775 && n < 18026) || n = 18027 || (n > 18032 && n < 18283) || n = 18284 || (n > 18289 && n < 18540) || n = 18541 || (n > 18546 && n < 18751))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17512)
	; 7 bytes
	tay
	lax #68
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17514 || n = 17517 || n = 17771 || n = 17774 || n = 18028 || n = 18031 || n = 18285 || n = 18288 || n = 18542 || n = 18545)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 17515)
	; 7 bytes
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17516)
	; 8 bytes
	tay
	pla
	lax #68
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17518 || n = 17775 || n = 18032 || n = 18289 || n = 18546)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17769)
	; 7 bytes
	tay
	lax #69
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17772)
	; 7 bytes
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17773)
	; 8 bytes
	tay
	pla
	lax #69
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18026)
	; 7 bytes
	tay
	lax #70
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18029)
	; 7 bytes
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18030)
	; 8 bytes
	tay
	pla
	lax #70
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18283)
	; 7 bytes
	tay
	lax #71
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18286)
	; 7 bytes
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18287)
	; 8 bytes
	tay
	pla
	lax #71
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18540)
	; 7 bytes
	tay
	lax #72
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18543)
	; 7 bytes
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18544)
	; 8 bytes
	tay
	pla
	lax #72
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 18751 to 20000
.if((n > 18750 && n < 18797) || n = 18798 || (n > 18803 && n < 19054) || n = 19055 || (n > 19060 && n < 19311) || n = 19312 || (n > 19317 && n < 19568) || n = 19569 || (n > 19574 && n < 19825) || n = 19826 || (n > 19831 && n < 20001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18797)
	; 7 bytes
	tay
	lax #73
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18799 || n = 18802 || n = 19056 || n = 19059 || n = 19313 || n = 19316 || n = 19570 || n = 19573 || n = 19827 || n = 19830)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 18800)
	; 7 bytes
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18801)
	; 8 bytes
	tay
	pla
	lax #73
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18803 || n = 19060 || n = 19317 || n = 19574 || n = 19831)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19054)
	; 7 bytes
	tay
	lax #74
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19057)
	; 7 bytes
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19058)
	; 8 bytes
	tay
	pla
	lax #74
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19311)
	; 7 bytes
	tay
	lax #75
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19314)
	; 7 bytes
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19315)
	; 8 bytes
	tay
	pla
	lax #75
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19568)
	; 7 bytes
	tay
	lax #76
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19571)
	; 7 bytes
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19572)
	; 8 bytes
	tay
	pla
	lax #76
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19825)
	; 7 bytes
	tay
	lax #77
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19828)
	; 7 bytes
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19829)
	; 8 bytes
	tay
	pla
	lax #77
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
