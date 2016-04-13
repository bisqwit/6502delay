; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, and S
; Will preserve A, Y, and P
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
; Requires that a label @nearby_rts_15cyc is defined,
; that points _somewhere_ that contains a JMP to a location that does RTS.

; Requires a zeropage two-byte variable @ptrtemp, that points
; into a memory address that can be safely read and overwritten
; with random data.
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
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 36
; - 8 bytes or less when n <= 46
; - 9 bytes or less when n <= 1307
; -10 bytes or less when n <= 1352
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=6, avg=7.2, max=9
; - When 51 <= n <= 75: min=7, avg=8.0, max=9
; - When 76 <= n <= 100: min=7, avg=8.1, max=9
; - When 101 <= n <= 200: min=6, avg=8.0, max=9
; - When 201 <= n <= 300: min=7, avg=8.1, max=9
; - When 301 <= n <= 500: min=6, avg=8.0, max=9
; - When 501 <= n <= 750: min=7, avg=8.1, max=9
; - When 751 <= n <= 1000: min=7, avg=8.1, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.8, max=11
; - When 2001 <= n <= 3000: min=8, avg=10.0, max=11
; - When 3001 <= n <= 5000: min=8, avg=10.6, max=11
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
; 1 to 78
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 1 bytes
	pha
.elseif(n = 4 || n = 5 || n = 8 || n = 17 || n = 28 || n = 31 || n = 41 || n = 44 || n = 45 || n = 55 || n = 60 || n = 65 || n = 70 || n = 75)
	; 2..8 bytes
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
.elseif((n > 8 && n < 12) || (n > 19 && n < 24) || (n > 32 && n < 38) || n = 46)
	; 3..8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 13)
	; 2 bytes
	brk 0
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 16 || n = 19 || n = 26 || n = 29 || n = 32 || n = 39 || n = 42)
	; 3..7 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18 || n = 30)
	; 4..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27 || n = 40)
	; 5..7 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 38 || n = 48 || n = 68 || n = 78)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 43 || n = 53 || n = 58 || n = 63 || n = 73)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 47 || n = 49 || n = 54 || n = 59 || n = 67 || n = 72 || n = 74 || n = 77)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 50 || n = 64)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 51 || n = 56 || n = 61 || n = 66 || n = 76)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 52)
	; 8 bytes
	.repeat 4
	 brk 0
	.endrepeat
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
.elseif(n = 69)
	; 9 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	pla
	plp
.endif
.else
; 79 to 156
.if(n = 79 || n = 82 || n = 84 || n = 87 || n = 89 || n = 94 || n = 97 || n = 102 || n = 104 || n = 107 || n = 109 || n = 112 || n = 117 || n = 119 || n = 122 || n = 124 || n = 129 || n = 139 || n = 142 || n = 147 || n = 149 || n = 152 || n = 154)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif((n .mod 5 = 0) && (n = 80 || (n > 89 && n < 116) || (n > 124 && n < 151)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 81 || n = 86 || n = 91 || n = 96 || n = 101 || n = 111 || n = 116 || n = 121 || n = 126 || n = 134 || n = 136 || n = 146 || n = 151 || n = 156)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 83 || n = 88 || n = 103 || n = 128 || n = 138 || n = 153)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 85 || n = 99 || n = 127 || n = 155)
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
.elseif(n = 137)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 144)
	; 8 bytes
	delay_n (n-13)
	brk 0
.endif
.endif
.else
.if((n >= 157) && (n < 235))
; 157 to 234
.if(n = 157 || n = 159 || n = 164 || n = 167 || n = 172 || n = 179 || n = 182 || n = 187 || n = 189 || n = 192 || n = 194 || n = 199 || n = 202 || n = 207 || n = 209 || n = 212 || n = 214 || n = 217 || n = 219 || n = 224 || n = 227 || n = 229 || n = 234)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
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
.elseif(n = 161 || n = 166 || n = 171 || n = 184 || n = 191 || n = 196 || n = 201 || n = 216 || n = 221 || n = 226)
	; 8 bytes
	pha
	delay_n (n-3)
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
	ldx #(n-13)/7 ;hides 'nop'
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
.if(n = 235 || n = 240 || n = 245 || n = 250 || n = 255 || n = 265 || n = 270 || n = 275 || n = 280 || n = 285 || n = 290 || n = 300 || n = 305 || n = 307 || n = 310)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 236 || n = 241 || n = 251 || n = 256 || n = 261 || n = 266 || n = 271 || n = 289 || n = 291 || n = 296 || n = 301)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 237 || n = 242 || n = 244 || n = 247 || n = 252 || n = 254 || n = 257 || n = 259 || n = 262 || n = 264 || n = 269 || n = 277 || n = 279 || n = 282 || n = 284 || n = 292 || n = 294)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
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
	ldx #(n-15)/9 ;hides 'nop'
	nop
	dex
	Jpl *-3
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
.elseif(n = 297)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 299)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if((n .mod 5 = 3) && ((n > 312 && n < 324) || n = 338 || (n > 347 && n < 359) || (n > 367 && n < 409) || n = 418 || n = 433 || n = 448 || (n > 457 && n < 469)))
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
.elseif(n = 315 || n = 317 || n = 320 || n = 322 || n = 325 || n = 327 || n = 335 || n = 339 || n = 340 || n = 342 || n = 345 || n = 347 || n = 350 || n = 355 || n = 357 || n = 360 || n = 362 || n = 370 || n = 374 || n = 375 || n = 377 || n = 380 || n = 382 || n = 385 || n = 387 || n = 390 || n = 395 || n = 397 || n = 409 || n = 410 || n = 415 || n = 417 || n = 420 || n = 425 || n = 427 || n = 430 || n = 440 || n = 444 || n = 445 || n = 447 || n = 450 || n = 452 || n = 455 || n = 457 || n = 460 || n = 462 || n = 465 || n = 467)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 316 || n = 330 || n = 337 || n = 351 || n = 372 || n = 407 || n = 414 || n = 421 || n = 435 || n = 442 || n = 449)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif((n .mod 5 = 4) && ((n > 318 && n < 335) || n = 349 || n = 354 || n = 364 || n = 369 || n = 394 || (n > 428 && n < 440) || n = 454 || n = 459))
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 321)
	; 8 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	inx
	Jpl *-3
	plp
.elseif(n = 326 || n = 331 || n = 336 || n = 341 || n = 346 || n = 356 || n = 361 || n = 371 || n = 376 || n = 381 || n = 389 || n = 391 || n = 396 || n = 401 || n = 406 || n = 416 || n = 422 || n = 426 || n = 431 || n = 436 || n = 441 || n = 446 || n = 451 || n = 461 || n = 466)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 328 || n = 333 || n = 343 || n = 363 || n = 413 || n = 423 || n = 428 || n = 438 || n = 443 || n = 453)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 332 || n = 367 || n = 437)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 344 || n = 365 || n = 379 || n = 400 || n = 456)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 352 || n = 464)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 359 || n = 404)
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
.elseif(n = 392)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 399)
	; 8 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 402)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 405)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
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
.elseif(n = 432)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 509 || n = 529 || n = 534)
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
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
.elseif(n = 472 || n = 504)
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
.elseif(n = 476 || n = 481 || n = 486 || n = 499 || n = 501 || n = 506 || n = 511 || n = 516 || n = 521 || n = 531 || n = 536 || n = 541 || n = 546)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 552 || n = 592)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif((n .mod 5 = 1) && (n = 556 || (n > 565 && n < 592) || (n > 600 && n < 622)))
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 584)
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
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
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
; 626 to 781
.if(n = 626 || n = 632 || n = 639 || n = 641 || n = 646 || n = 651 || n = 656 || n = 661 || n = 671 || n = 676 || n = 686 || n = 691 || n = 696 || n = 706 || n = 711 || n = 716 || n = 721 || n = 726 || n = 731 || n = 737 || n = 741 || n = 746 || n = 756 || n = 761 || n = 766 || n = 776 || n = 781)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 634 || n = 649 || n = 669 || n = 679 || n = 684 || n = 709 || n = 714 || n = 739 || n = 749 || n = 754 || n = 769 || n = 774 || n = 779)
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
.elseif((n .mod 7 = 1) && (n = 645 || n = 652 || (n > 679 && n < 702) || (n > 714 && n < 730) || (n > 749 && n < 772)))
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
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
.elseif(n = 664 || n = 712 || n = 744)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
	plp
.elseif(n = 672 || n = 704 || n = 752)
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
.elseif(n = 784 || n = 824 || n = 832 || n = 864)
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
.elseif((n .mod 5 = 1) && ((n > 785 && n < 802) || n = 811 || (n > 820 && n < 837) || (n > 845 && n < 872) || (n > 880 && n < 902) || (n > 915 && n < 937)))
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 940 || n = 945 || n = 950 || n = 955 || n = 957 || n = 965 || n = 970 || n = 972 || n = 975 || n = 977 || n = 980 || n = 982 || n = 985 || n = 990 || n = 1000 || n = 1005 || n = 1007 || n = 1010 || n = 1012 || n = 1015 || n = 1017 || n = 1020 || n = 1022 || n = 1025 || n = 1035 || n = 1039 || n = 1040 || n = 1042 || n = 1045 || n = 1047 || n = 1050 || n = 1052 || n = 1055 || n = 1057 || n = 1060 || n = 1062 || n = 1070 || n = 1074 || n = 1075 || n = 1077 || n = 1080 || n = 1082 || n = 1085 || n = 1087 || n = 1090 || n = 1092)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 941 || n = 947 || n = 951 || n = 956 || n = 961 || n = 966 || n = 971 || n = 976 || n = 986 || n = 991 || n = 996 || n = 1001 || n = 1006 || n = 1011 || n = 1024 || n = 1026 || n = 1031 || n = 1036 || n = 1046 || n = 1056 || n = 1061 || n = 1066 || n = 1071 || n = 1076 || n = 1081 || n = 1091)
	; 7..8 bytes
	pha
	delay_n (n-3)
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
	sta (@ptrtemp,x)
	plp
.elseif(n = 952 || n = 1032 || n = 1064)
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
.elseif(n = 979)
	; 9 bytes
	php
	pha
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
	pla
	plp
.elseif(n = 984 || n = 992)
	; 9 bytes
	php
	ldx #n/8-2
	bit $24
	dex
	Jpl *-3
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
.elseif(n = 1021)
	; 6 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	plp
.elseif(n = 1027)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 1034)
	; 8 bytes
	delay_n (n-13)
	brk 0
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
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && (n = 1094 || n = 1099 || n = 1119 || (n > 1128 && n < 1140) || (n > 1153 && n < 1165) || n = 1174 || n = 1189 || (n > 1198 && n < 1210) || (n > 1228 && n < 1240)))
	; 9 bytes
	php
	ldx #(n-14)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 1095 || n = 1097 || n = 1105 || n = 1109 || n = 1110 || n = 1115 || n = 1117 || n = 1120 || n = 1125 || n = 1127 || n = 1130 || n = 1132 || n = 1140 || n = 1145 || n = 1147 || n = 1150 || n = 1155 || n = 1157 || n = 1160 || n = 1162 || n = 1165 || n = 1167 || n = 1175 || n = 1179 || n = 1180 || n = 1182 || n = 1185 || n = 1187 || n = 1190 || n = 1195 || n = 1197 || n = 1200 || n = 1202 || n = 1210 || n = 1215 || n = 1217 || n = 1220 || n = 1222 || n = 1225 || n = 1227 || n = 1230 || n = 1232 || n = 1235 || n = 1237 || n = 1245 || n = 1249 || n = 1250)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 1095 && n < 1117) || n = 1126 || (n > 1135 && n < 1152) || (n > 1160 && n < 1187) || (n > 1195 && n < 1222) || (n > 1230 && n < 1247)))
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1112 || n = 1144 || n = 1152 || n = 1192 || n = 1224)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
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
; 1251 to 1328
.if(n = 1251 || n = 1256 || n = 1271 || n = 1276 || n = 1281 || n = 1286 || n = 1291 || n = 1297 || n = 1299 || n = 1306 || n = 1325 || n = 1327)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1252 || n = 1255 || n = 1260 || n = 1262 || n = 1265 || n = 1267 || n = 1270 || n = 1280 || n = 1284 || n = 1285 || n = 1287 || n = 1290 || n = 1292 || n = 1298 || n = 1305 || n = 1314 || n = 1315 || n = 1319 || n = 1326)
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
.elseif(n = 1257)
	; 8 bytes
	php
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	inx
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
.elseif(n = 1264 || n = 1272 || n = 1312 || n = 1320 || n = 1328)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
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
.elseif(n = 1307)
	; 9 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1308 || n = 1318)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 1309 || n = 1323)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
	plp
.elseif(n = 1311)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $91A2'
	ldx #145
	dex
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
.elseif(n = 1321)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $92A2'
	ldx #146
	dex
	Jne *-4
	plp
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
.elseif(n = 1332 || n = 1333 || n = 1335 || n = 1343 || n = 1347 || n = 1351 || n = 1353 || n = 1354 || n = 1361 || n = 1363)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1334 || n = 1341 || n = 1355 || n = 1362)
	; 9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1342)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
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
	delay_n 4
	nop
	dex
	Jpl *-4
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
.elseif(n = 1369 || n = 1377 || n = 1386 || n = 1390 || n = 1395 || n = 1397 || n = 1404)
	; 8..9 bytes
	pha
	delay_n (n-3)
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
	delay_n 4
	nop
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
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	brk 0
	Jmi *-3
	plp
.elseif(n = 1375 || n = 1382 || n = 1389 || n = 1391 || n = 1396)
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
	clv
	Jvc *+2
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
	; 9 bytes
	delay_n (n-13)
	brk 0
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
.elseif(n = 1409 || n = 1418 || n = 1425 || n = 1430)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1410 || n = 1414 || n = 1417 || n = 1423 || n = 1426 || n = 1431 || n = 1438 || n = 1441 || n = 1442 || n = 1444 || n = 1445)
	; 9..11 bytes
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
.elseif(n = 1412 || n = 1421 || n = 1439)
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
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
.elseif(n = 1451 || n = 1453 || n = 1467 || n = 1481)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1452 || n = 1454 || n = 1459 || n = 1461 || n = 1473 || n = 1476 || n = 1477)
	; 9..10 bytes
	delay_n (n-2)
	nop
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
	clv
	Jvc *+2
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
	bit <$2C ;hides 'bit $A2A2'
	ldx #162
	dex
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
.elseif(n = 1479)
	; 11 bytes
	php
	pha
	ldx #(n-15)/8
	bit $24
	dex
	Jne *-3
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
.elseif(n = 1486 || n = 1487 || n = 1489 || n = 1490 || n = 1494 || n = 1497 || n = 1501 || n = 1505 || n = 1515 || n = 1517 || n = 1522)
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
.elseif(n = 1495 || n = 1514 || n = 1516 || n = 1521 || n = 1523)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1498)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
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
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
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
.elseif(n = 1530 || n = 1535 || n = 1542 || n = 1551 || n = 1558)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1531)
	; 9 bytes
	delay_n (n-13)
	brk 0
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
.elseif(n = 1540 || n = 1543 || n = 1549 || n = 1550 || n = 1553 || n = 1557 || n = 1559)
	; 9..10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1554)
	; 8 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	nop
	inx
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
.elseif(n = 1564 || n = 1571 || n = 1577 || n = 1578 || n = 1585 || n = 1587 || n = 1593 || n = 1595 || n = 1599)
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
.elseif(n = 1570 || n = 1579 || n = 1586 || n = 1594)
	; 9 bytes
	pha
	delay_n (n-3)
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
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 1589 || n = 1596)
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
.elseif(n = 1598)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.endif
.else
; 1602 to 1640
.if(n = 1602 || n = 1606 || n = 1612 || n = 1613 || n = 1615 || n = 1620 || n = 1627 || n = 1634)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1603)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
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
	delay_n 4
	nop
	dex
	Jmi *-5
	plp
.elseif(n = 1607 || n = 1614 || n = 1619 || n = 1621 || n = 1633)
	; 8..9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1610 || n = 1628 || n = 1637)
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
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $B4A2'
	ldx #180
	dex
	Jne *-4
	plp
.elseif(n = 1629)
	; 9 bytes
	delay_n (n-13)
	brk 0
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
.elseif(n = 1635)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $B5A2'
	ldx #181
	dex
	Jne *-4
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
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641 || n = 1658 || n = 1669 || n = 1675)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1642 || n = 1647 || n = 1649 || n = 1665 || n = 1670 || n = 1677)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1643 || n = 1657)
	; 9 bytes
	delay_n (n-13)
	brk 0
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
.elseif(n = 1648 || n = 1656 || n = 1672)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1650)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	sta (@ptrtemp,x)
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
.elseif(n = 1659)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
	plp
.elseif(n = 1661)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 1663)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $B8A2'
	ldx #184
	dex
	Jne *-4
	plp
.elseif(n = 1666)
	; 9 bytes
	php
	ldx #90 ;hides 'nop'
	nop $04
	inx
	Jne *-4
	plp
.elseif(n = 1668 || n = 1678)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
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
.elseif(n = 1679)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $98A2'
	ldx #152
	nop
	dex
	Jne *-5
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
.elseif(n = 1683 || n = 1685 || n = 1690 || n = 1692 || n = 1693 || n = 1697 || n = 1706 || n = 1710 || n = 1711 || n = 1713)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1703 || n = 1705)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1686)
	; 10 bytes
	php
	ldx #116 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jne *-5
	plp
.elseif(n = 1687 || n = 1694 || n = 1701 || n = 1715)
	; 10 bytes
	php
	ldx #n/7-2
	nop
	dex
	Jne *-2
	sta (@ptrtemp,x)
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
.if(n = 1719 || n = 1731 || n = 1733 || n = 1740 || n = 1745 || n = 1747)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1720 || n = 1728 || n = 1752)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1721 || n = 1722 || n = 1725 || n = 1729 || n = 1732 || n = 1739 || n = 1741 || n = 1746 || n = 1750 || n = 1753)
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
.elseif(n = 1727 || n = 1736 || n = 1754)
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
	clv
	Jvc *+2
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
.elseif(n = 1755)
	; 9 bytes
	delay_n (n-13)
	brk 0
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
	delay_n 4
	nop
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
	sta (@ptrtemp,x)
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
.elseif(n = 1778 || n = 1782 || n = 1787 || n = 1791)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1780)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C5A2'
	ldx #197
	dex
	Jne *-4
	plp
.elseif(n = 1783)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
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
; 1797 to 1835
.if(n = 1797)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $C7A2'
	ldx #199
	dex
	Jne *-4
	plp
.elseif(n = 1798)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $C7A2'
	ldx #199
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
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1802 || n = 1804 || n = 1805 || n = 1810 || n = 1819 || n = 1822 || n = 1825 || n = 1827)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1803 || n = 1809)
	; 9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1811)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $A4A2'
	ldx #164
	nop
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
.elseif(n = 1816 || n = 1824 || n = 1832)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1818 || n = 1828)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
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
.else
; 1836 to 1875
.if(n = 1836 || n = 1854 || n = 1859)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1837 || n = 1841 || n = 1847 || n = 1850 || n = 1855 || n = 1863 || n = 1866 || n = 1873 || n = 1874)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1838 || n = 1858)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
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
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 1846)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	bit $24
	dex
	Jne *-4
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
.elseif(n = 1857)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	sta (@ptrtemp,x)
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
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
.elseif(n = 1875)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
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
.elseif(n = 1881 || n = 1890 || n = 1891 || n = 1893 || n = 1894)
	; 10..11 bytes
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
.elseif(n = 1883 || n = 1885 || n = 1901 || n = 1908)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1902)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1903)
	; 11 bytes
	php
	pha
	ldx #(n-15)/8
	bit $24
	dex
	Jne *-3
	pla
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
.elseif(n = 1910)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $ADA2'
	ldx #173
	nop
	dex
	Jne *-5
	plp
.elseif(n = 1913)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
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
.elseif(n = 1917 || n = 1919 || n = 1926 || n = 1935 || n = 1953)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1918 || n = 1938 || n = 1948)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
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
	bit <$2C ;hides 'bit $D5A2'
	ldx #213
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
.elseif(n = 1937)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	dex
	Jmi *-5
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
.elseif(n = 1949)
	; 11 bytes
	php
	pha
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
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
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1956)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
	sta (@ptrtemp,x)
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
	clv
	Jvc *+2
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
.elseif(n = 1962 || n = 1963 || n = 1972 || n = 1980 || n = 1981 || n = 1989 || n = 1990)
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
.elseif(n = 1971 || n = 1973 || n = 1991)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1974)
	; 10 bytes
	php
	ldx #250 ;hides 'nop'
	delay_n 9
	dex
	Jmi *-5
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
.elseif(n = 1983)
	; 11 bytes
	php
	pha
	ldx #(n-15)/8
	bit $24
	dex
	Jne *-3
	pla
	plp
.elseif(n = 1985)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
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
.if(n = 1993)
	; 11 bytes
	php
	pha
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	pla
	plp
.elseif(n = 1994 || n = 2002 || n = 2007 || n = 2010 || n = 2017 || n = 2025 || n = 2026)
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
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 1999 || n = 2021)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
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
.elseif(n = 2001)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2003)
	; 10 bytes
	php
	ldx #(n-27)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
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
.elseif(n = 2011 || n = 2027)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2020)
	; 10 bytes
	php
	bit <$2C ;hides 'bit $B7A2'
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
.elseif(n = 2029)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2034 || n = 2035 || n = 2037 || n = 2044 || n = 2053 || n = 2061 || n = 2062 || n = 2070)
	; 10..11 bytes
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
.elseif(n = 2038)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2045)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2046)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	brk 0
	inx
	Jpl *-4
	plp
.elseif(n = 2047 || n = 2055)
	; 11 bytes
	php
	pha
	ldx #(n-15)/8
	bit $24
	dex
	Jne *-3
	pla
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
.elseif(n = 2057)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
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
.if(n = 2071 || n = 2073 || n = 2074 || n = 2089 || n = 2106 || n = 2107)
	; 10..11 bytes
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
.elseif((n .mod 9 = 8) && ((n > 2077 && n < 2106)))
	; 9 bytes
	php
	ldx #(n-8)/9
	delay_n 4
	dex
	Jne *-3
	plp
.elseif(n = 2079 || n = 2081 || n = 2088 || n = 2090 || n = 2097)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2080)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2082 || n = 2099)
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
.elseif(n = 2093)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
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
.elseif(n = 2098 || n = 2109)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
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
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
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
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2116 || n = 2125 || n = 2134 || n = 2143)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2111 || n = 2115 || n = 2124 || n = 2126 || n = 2133 || n = 2135 || n = 2142 || n = 2147)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2117)
	; 10 bytes
	php
	ldx #(n-27)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
	plp
.elseif(n = 2118 || n = 2128 || n = 2138)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
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
.elseif(n = 2145)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	sta (@ptrtemp,x)
	plp
.elseif(n = 2146)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	brk 0
	dex
	Jmi *-4
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
.elseif(n = 2151 || n = 2152 || n = 2160 || n = 2161 || n = 2165 || n = 2169 || n = 2170 || n = 2172 || n = 2179 || n = 2181 || n = 2182 || n = 2187)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2153 || n = 2164)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
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
.elseif(n = 2155)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.elseif(n = 2171 || n = 2183)
	; 10 bytes
	pha
	delay_n (n-3)
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
	clv
	Jvc *+2
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
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 2189 || n = 2196 || n = 2205 || n = 2210 || n = 2214 || n = 2225)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2190)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	inx
	Jpl *-5
	plp
.elseif(n = 2191)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 2197)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
	plp
.elseif(n = 2199 || n = 2209 || n = 2215 || n = 2223 || n = 2224 || n = 2226)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
	delay_n 4
	nop
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
.elseif(n = 2232 || n = 2233 || n = 2236 || n = 2242 || n = 2251 || n = 2254 || n = 2260)
	; 10..11 bytes
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
.elseif(n = 2237)
	; 11 bytes
	php
	pha
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
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
	delay_n 4
	nop
	Jne *-4
	plp
.elseif(n = 2241 || n = 2243 || n = 2259)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2253)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
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
.elseif(n = 2263)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
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
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 2269 || n = 2271 || n = 2279 || n = 2289 || n = 2295 || n = 2299)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2270 || n = 2280 || n = 2291)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2281)
	; 11 bytes
	php
	pha
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
	pla
	plp
.elseif(n = 2282)
	; 11 bytes
	php
	pha
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
	pla
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
	delay_n 4
	nop
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
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
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
.elseif(n = 2307 || n = 2318)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2308)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 2309 || n = 2315)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2319)
	; 11 bytes
	php
	pha
	ldx #0
	delay_n 4
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2320)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2321 || n = 2323)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2322)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.endif
.else
; 2324 to 2343
.if(n = 2324 || n = 2336)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2325)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2328)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 2329 || n = 2340)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
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
.elseif(n = 2331)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
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
.elseif(n = 2334)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2335)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
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
	pha
	delay_n (n-3)
.elseif(n = 2342)
	; 10 bytes
	php
	ldx #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
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
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 2345)
	; 10 bytes
	php
	ldx #(n-27)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
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
.elseif(n = 2350 || n = 2353 || n = 2361 || n = 2380)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2351 || n = 2362 || n = 2373)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2352 || n = 2363 || n = 2375)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2354)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2355 || n = 2367 || n = 2379)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2356)
	; 11 bytes
	php
	pha
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
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
.elseif(n = 2358 || n = 2368 || n = 2378)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 2359)
	; 9 bytes
	php
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2364)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2365 || n = 2381)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2366)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
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
	delay_n 4
	nop
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
.elseif(n = 2376)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2377)
	; 10 bytes
	php
	ldx #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	inx
	Jne *-5
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
.elseif(n = 2385 || n = 2387 || n = 2399 || n = 2407 || n = 2411)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2386 || n = 2390 || n = 2397 || n = 2400 || n = 2410 || n = 2412 || n = 2419)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2388 || n = 2398 || n = 2418)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 2389)
	; 11 bytes
	php
	pha
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2391 || n = 2403)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2392)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
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
.elseif(n = 2395 || n = 2406 || n = 2417)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2401 || n = 2409 || n = 2414)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2402)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
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
.elseif(n = 2413)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
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
.elseif(n = 2416)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2421)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2424 || n = 2434 || n = 2436)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2423 || n = 2429 || n = 2435 || n = 2441 || n = 2445 || n = 2447)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2425 || n = 2431 || n = 2433 || n = 2443 || n = 2457)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2426)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	brk 0
	dex
	Jmi *-4
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
	delay_n 4
	nop
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
.elseif(n = 2438)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 2440)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 2446)
	; 9 bytes
	php
	ldx #(n-6)/20 ;hides 'nop'
	brk 0
	dex
	Jne *-4
	plp
.elseif(n = 2448 || n = 2458)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
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
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2452)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
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
.elseif(n = 2455)
	; 10 bytes
	delay_n (n-13)
	brk 0
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
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2462 || n = 2464 || n = 2467 || n = 2479)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif((n .mod 12 = 3) && ((n > 2462 && n < 2500)))
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2465 || n = 2491)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2466)
	; 9 bytes
	php
	ldx #250 ;hides 'nop'
	brk 0
	dex
	Jmi *-4
	plp
.elseif(n = 2468 || n = 2480 || n = 2492)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2469 || n = 2477 || n = 2489 || n = 2495)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2470 || n = 2476 || n = 2482 || n = 2484 || n = 2485)
	; 10..11 bytes
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
.elseif(n = 2473 || n = 2490)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2474)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 2478 || n = 2488 || n = 2498)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
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
.elseif(n = 2496)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2497)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.if((n >= 2501) && (n < 2540))
; 2501 to 2539
.if(n = 2501 || n = 2521 || n = 2522 || n = 2533)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2502 || n = 2512 || n = 2520 || n = 2529 || n = 2530 || n = 2532)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2505 || n = 2527 || n = 2538)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
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
.elseif(n = 2507 || n = 2524)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2508 || n = 2518)
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 2509 || n = 2513 || n = 2519 || n = 2531 || n = 2539)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2510)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 2511 || n = 2523)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
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
	ldx #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	inx
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
.elseif(n = 2534)
	; 11 bytes
	php
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
	pla
	plp
.elseif(n = 2535)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.elseif(n = 2543 || n = 2555 || n = 2567)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2544)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2545 || n = 2553 || n = 2563)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2546)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
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
	; 10 bytes
	php
	ldx #(n-8)/10
	clv
	Jvc *+2
	dex
	Jne *-4
	plp
.elseif(n = 2549 || n = 2560 || n = 2571)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
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
.elseif(n = 2551 || n = 2554 || n = 2566 || n = 2572 || n = 2578)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2557)
	; 9 bytes
	php
	ldx #0 ;hides 'brk imm'
	nop
	inx
	inx
	Jne *-4
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
	; 11 bytes
	php
	pha
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	Jne *-4
	pla
	plp
.elseif(n = 2568)
	; 10 bytes
	php
	ldx #0
	clv
	Jvc *+2
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
.elseif(n = 2573)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 2577)
	; 10 bytes
	php
	ldx #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dex
	Jpl *-5
	plp
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579 || n = 2583 || n = 2585 || n = 2594 || n = 2605)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2586 || n = 2590 || n = 2614 || n = 2617)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2587)
	; 11 bytes
	php
	pha
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2588 || n = 2600 || n = 2612)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2589)
	; 10 bytes
	php
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
	plp
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
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2595 || n = 2607)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2596 || n = 2599 || n = 2610)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2597)
	; 10 bytes
	php
	ldx #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dex
	Jne *-5
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
	delay_n 4
	nop
	inx
	Jne *-5
	plp
.elseif(n = 2608)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
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
.elseif(n = 2611)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
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
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2654)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 2619 || n = 2621 || n = 2627 || n = 2639 || n = 2651)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2620 || n = 2638 || n = 2642 || n = 2650 || n = 2652)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2622)
	; 10 bytes
	php
	ldx #(n-6)/12 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
	plp
.elseif(n = 2623 || n = 2629 || n = 2641 || n = 2644 || n = 2653)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
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
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2628)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2630 || n = 2649)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2631 || n = 2655)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2632)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 2633)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 2634)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
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
.elseif(n = 2640)
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
	.repeat 4
	 nop
	.endrepeat
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
	delay_n 4
	nop
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2660 || n = 2672 || n = 2684)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 2661 || n = 2662 || n = 2664 || n = 2674 || n = 2676 || n = 2683)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2663 || n = 2675 || n = 2693)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2665 || n = 2666 || n = 2685)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2667)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
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
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2677 || n = 2694)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2678)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
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
.elseif(n = 2686)
	; 9 bytes
	php
	ldx #122 ;hides 'nop'
	brk 0
	inx
	Jne *-4
	plp
.elseif(n = 2687)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2688)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
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
.elseif(n = 2695)
	; 11 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	cmp $C5
	dex
	Jne *-6
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
.elseif(n = 2697)
	; 11 bytes
	php
	pha
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2698 || n = 2700 || n = 2705 || n = 2716 || n = 2722 || n = 2733)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2699 || n = 2704 || n = 2729)
	; 10 bytes
	pha
	delay_n (n-3)
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2706)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2707)
	; 11 bytes
	php
	pha
	ldx #245 ;hides 'sbc zp,x'
	cmp $EA ;hides 'nop'
	dex
	Jne *-4
	pla
	plp
.elseif(n = 2709 || n = 2717 || n = 2718 || n = 2721 || n = 2730)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 2715 || n = 2727)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
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
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
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
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2737)
	; 10 bytes
	php
	ldx #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.elseif(n = 2738 || n = 2752 || n = 2766)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2739 || n = 2751)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2740 || n = 2746 || n = 2754 || n = 2764 || n = 2770 || n = 2772)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
	delay_n 4
	nop
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
.elseif(n = 2748)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2749)
	; 10 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.elseif(n = 2750)
	; 11 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
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
.elseif(n = 2759 || n = 2765 || n = 2771)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2760)
	; 10 bytes
	php
	ldx #212 ;hides 'nop zp,x'
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
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 2763)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
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
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2776)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 2777 || n = 2803)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
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
.elseif(n = 2781 || n = 2783 || n = 2795 || n = 2807 || n = 2809)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2782 || n = 2784 || n = 2793 || n = 2810)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2785 || n = 2786 || n = 2797 || n = 2805)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2787 || n = 2799)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
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
	clv
	Jvc *+2
	inx
	Jne *-5
	plp
.elseif(n = 2791 || n = 2802)
	; 10 bytes
	php
	ldx #(n-8)/11
	delay_n 4
	nop
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
.elseif(n = 2796)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2798)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
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
	delay_n 4
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2814)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	clv
	Jvc *+2
	dex
	Jne *-5
	plp
.elseif(n = 2815 || n = 2818 || n = 2825 || n = 2830 || n = 2832 || n = 2842 || n = 2844 || n = 2854 || n = 2856 || n = 2866 || n = 2872 || n = 2875 || n = 2890)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 8) && ((n > 2815 && n < 2889)))
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
.elseif(n = 2819 || n = 2831 || n = 2843 || n = 2851 || n = 2855 || n = 2867 || n = 2879)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2820 || n = 2839 || n = 2858 || n = 2877)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2821 || n = 2833 || n = 2841 || n = 2845 || n = 2846 || n = 2849 || n = 2853 || n = 2857 || (n > 2859 && n < 2864) || n = 2865 || n = 2869 || n = 2874 || n = 2882 || (n > 2883 && n < 2888) || n = 2889)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2822 || n = 2836 || n = 2850 || n = 2878)
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
	delay_n 4
	nop
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
.elseif(n = 2829)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2834 || n = 2870)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 2835 || n = 2859 || n = 2871 || n = 2883)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
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
.elseif(n = 2847 || n = 2881)
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
.elseif(n = 2868)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2873)
	; 9 bytes
	php
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2880)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2903 || n = 2909 || n = 2927 || n = 2939 || n = 2945 || n = 2951 || n = 2963)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2892 || n = 2920 || n = 2934 || n = 2962)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 2893 || n = 2897 || n = 2899 || n = 2901 || n = 2905 || n = 2910 || n = 2911 || n = 2913 || n = 2917 || n = 2921 || n = 2923 || n = 2925 || n = 2929 || n = 2930 || n = 2935 || n = 2937 || n = 2941 || n = 2956 || n = 2961 || n = 2965)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2894 || n = 2959)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif((n .mod 12 = 3) && ((n > 2894 && n < 2968)))
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2896 || n = 2953)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 2902 || n = 2904 || n = 2914 || n = 2916 || n = 2926 || n = 2938 || n = 2940 || n = 2944 || n = 2950 || n = 2952)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2906 || n = 2942)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 2908 || n = 2928 || n = 2968)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2918)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
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
.elseif(n = 2933)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
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
.elseif(n = 2954)
	; 10 bytes
	php
	ldx #122 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
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
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	clc
	Jcc *+2
	dex
	Jne *-6
	plp
.elseif(n = 2964)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if(n = 2969 || n = 2973 || n = 2989 || n = 2997 || n = 3001 || n = 3005)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2970)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	cmp $C5
	dex
	Jne *-6
	plp
.elseif(n = 2971)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	clc
	Jcc *+2
	dex
	Jne *-6
	plp
.elseif(n = 2972 || n = 2984 || n = 2996)
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
	plp
.elseif((n .mod 6 = 4) && ((n > 2973 && n < 2999)))
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2975 || n = 2981 || n = 2987 || n = 2999)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2976 || n = 2990 || n = 3004)
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
.elseif(n = 2978)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 2979 || n = 3003)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 2982)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	clv
	Jvc *+2
	dex
	Jne *-5
	plp
.elseif(n = 2983 || n = 3000)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 2985)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 2988)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 2991)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 2993)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 2994)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 2995)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3002)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	clc
	Jcc *+2
	dex
	Jne *-6
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
.elseif(n = 3007)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	inx
	Jne *-6
	plp
.endif
.else
; 3008 to 3046
.if((n .mod 12 = 8) && ((n > 3007 && n < 3045)))
	; 9 bytes
	php
	ldx #(n-8)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 3010 || n = 3012 || n = 3016 || n = 3022 || n = 3024 || n = 3034 || n = 3036)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3011 || n = 3023 || n = 3033 || n = 3035)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3013 || n = 3021 || n = 3025 || n = 3026 || n = 3040 || n = 3041 || n = 3045)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3014)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3015 || n = 3027 || n = 3039)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 3017)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3018 || n = 3046)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3019)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3028)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3029)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
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
.elseif(n = 3031)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3037)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3038)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3042)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3043)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	inx
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047 || n = 3053 || n = 3059 || n = 3071 || n = 3083)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3048)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 3049 || n = 3057 || n = 3065 || n = 3069 || n = 3077 || n = 3081)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3050)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3051)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	php
	cmp $C5
	plp
	Jne *-5
	plp
.elseif(n = 3052 || n = 3062 || n = 3070 || n = 3082 || n = 3084)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3054)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3055)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	inx
	Jne *-6
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
.elseif(n = 3060 || n = 3074)
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
.elseif(n = 3063)
	; 11 bytes
	php
	pha
	ldx #(n-15)/12
	delay_n 7
	dex
	Jne *-3
	pla
	plp
.elseif(n = 3064)
	; 10 bytes
	php
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 3066)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3067)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.elseif(n = 3075)
	; 10 bytes
	php
	ldx #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	inx
	Jne *-5
	plp
.elseif(n = 3076)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3078)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	dex
	Jne *-6
	plp
.elseif(n = 3079)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	inx
	Jne *-6
	plp
.elseif(n = 3080)
	; 9 bytes
	php
	ldx #0
	delay_n 7
	dex
	Jne *-3
	plp
.elseif(n = 3085)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3104 || n = 3122)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
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
.elseif(n = 3088 || n = 3102 || n = 3116)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3089 || n = 3107 || n = 3125)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3090 || n = 3106 || n = 3118)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n > 3090 && n < 3096) || n = 3097 || n = 3099 || n = 3100 || n = 3103 || (n > 3108 && n < 3112) || n = 3114 || n = 3117 || n = 3120 || n = 3121 || n = 3123)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3096 || n = 3112)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3098 || n = 3113)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3101)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 4
	nop
	inx
	Jne *-6
	plp
.elseif(n = 3105 || n = 3124)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3108)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3115)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3119)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126 || n = 3127 || n = 3129 || n = 3131 || n = 3135 || n = 3137 || n = 3138 || n = 3145 || n = 3147 || (n > 3148 && n < 3153) || (n > 3154 && n < 3158) || n = 3159 || n = 3163 || n = 3164 || n = 3166 || n = 3169 || n = 3171 || n = 3175 || (n > 3181 && n < 3185) || n = 3189 || n = 3190 || n = 3195)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3128 || n = 3148 || n = 3168 || n = 3188)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3130)
	; 9 bytes
	php
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3132 || n = 3142 || n = 3146 || n = 3196 || n = 3202)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3133 || n = 3161 || n = 3179 || n = 3197)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3134)
	; 11 bytes
	php
	ldx #184 ;hides 'clv'
	rol $26
	ror $26
	dex
	Jne *-6
	plp
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
.elseif(n = 3140 || n = 3158 || n = 3176 || n = 3194)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3141 || n = 3154 || n = 3180 || n = 3193)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3143 || n = 3162 || n = 3181)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3144 || n = 3172 || n = 3186 || n = 3200)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
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
.elseif(n = 3167)
	; 10 bytes
	php
	ldx #(n-3)/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3173 || n = 3203)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
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
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 3185)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	delay_n 4
	nop
	dex
	Jne *-6
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
.elseif(n = 3192)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3198)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 3199)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3201)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
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
.elseif(n = 3205 || n = 3207 || (n > 3208 && n < 3212) || n = 3217 || n = 3220 || n = 3222 || n = 3225 || n = 3227 || n = 3231 || (n > 3233 && n < 3238) || n = 3239 || n = 3241 || n = 3247 || n = 3249 || n = 3252 || n = 3253 || n = 3259 || n = 3260 || n = 3264 || n = 3267 || (n > 3272 && n < 3276) || n = 3277 || (n > 3278 && n < 3282))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 3208 || n = 3268)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3212 || n = 3230 || n = 3248 || n = 3266)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3213)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 4
	nop
	dex
	Jne *-6
	plp
.elseif((n .mod 14 = 8) && ((n > 3213 && n < 3271)))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3215 || n = 3229 || n = 3233 || n = 3251 || n = 3269)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3216 || n = 3232 || n = 3244 || n = 3250)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3218 || n = 3263 || n = 3278)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3219 || n = 3257 || n = 3276)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3223)
	; 11 bytes
	php
	ldx #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3224 || n = 3240)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
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
.elseif(n = 3243)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3245 || n = 3258 || n = 3271)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3246)
	; 11 bytes
	php
	ldx #216 ;hides 'cld'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3254)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
	plp
.elseif(n = 3261)
	; 11 bytes
	php
	pha
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
	pla
	plp
.elseif(n = 3262)
	; 10 bytes
	php
	ldx #100 ;hides 'nop zp'
	inx ;first loop only
	brk 0
	inx
	Jne *-5
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
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282)
	; 9 bytes
	php
	ldx #(n-6)/14 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3283)
	; 10 bytes
	php
	ldx #100 ;hides 'nop zp'
	nop
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 3284 || n = 3302 || n = 3320)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3285 || n = 3287 || n = 3305)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3286)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3288 || n = 3308)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3289 || n = 3306)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3290 || n = 3291 || n = 3299 || n = 3301 || n = 3303 || n = 3311 || n = 3313 || (n > 3314 && n < 3318) || n = 3319)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
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
.elseif(n = 3293)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3294 || n = 3300 || n = 3304)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3295 || n = 3314)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3296)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3297 || n = 3310)
	; 11 bytes
	php
	ldx #(n-8)/13
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3298 || n = 3312)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3307)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 9
	inx
	Jne *-5
	plp
.elseif(n = 3309)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3318)
	; 11 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 11
	dex
	Jne *-6
	plp
.endif
.else
; 3321 to 3359
.if(n = 3321)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	inx
	Jne *-6
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
.elseif(n = 3323 || n = 3357)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3324 || n = 3342 || n = 3358)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3325 || n = 3327 || (n > 3329 && n < 3333) || n = 3337 || (n > 3342 && n < 3348) || (n > 3348 && n < 3352))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3326)
	; 9 bytes
	php
	ldx #90 ;hides 'nop'
	brk 0
	inx
	Jne *-4
	plp
.elseif(n = 3328 || n = 3348)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3329 || n = 3341 || n = 3359)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3333 || n = 3352)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3334)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	dex
	Jne *-6
	plp
.elseif(n = 3335)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	inx
	Jne *-6
	plp
.elseif(n = 3336)
	; 11 bytes
	php
	ldx #0
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-5
	plp
.elseif(n = 3338 || n = 3356)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3339)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3340 || n = 3354)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3353)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3355)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.endif
.endif
.else
; 3360 to 3437
.if((n > 3359 && n < 3368) || n = 3369 || n = 3375 || (n > 3377 && n < 3382) || n = 3385 || n = 3386 || n = 3397 || n = 3399 || (n > 3400 && n < 3406) || n = 3407 || n = 3411 || n = 3415 || (n > 3416 && n < 3422) || n = 3423 || (n > 3432 && n < 3438))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3368 || n = 3382 || n = 3396 || n = 3424)
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3370 || n = 3376 || n = 3389 || n = 3394 || n = 3426 || n = 3429)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3371)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3372)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	brk 0
	dex
	dex
	Jne *-5
	plp
.elseif(n = 3373)
	; 11 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3374 || n = 3392 || n = 3410 || n = 3428)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3377 || n = 3390 || n = 3395 || n = 3413 || n = 3431)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3383)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif((n .mod 16 = 8) && ((n > 3383 && n < 3433)))
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3387)
	; 9 bytes
	php
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3388)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3391 || n = 3408 || n = 3425)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.elseif(n = 3406)
	; 11 bytes
	php
	ldx #56 ;hides 'sec'
	rol $26
	ror $26
	inx
	Jne *-6
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
.endif
.else
.if((n >= 3438) && (n < 3594))
; 3438 to 3593
.if((n .mod 14 = 8) && ((n > 3437 && n < 3523) || (n > 3549 && n < 3579)))
	; 10 bytes
	php
	ldx #(n-8)/14
	delay_n 9
	dex
	Jne *-4
	plp
.elseif(n = 3439 || n = 3441 || n = 3445 || n = 3450 || n = 3451 || n = 3453 || n = 3455 || n = 3456 || n = 3461 || n = 3463 || n = 3465 || n = 3469 || n = 3470 || n = 3472 || n = 3474 || n = 3475 || n = 3477 || n = 3479 || n = 3483 || n = 3487 || (n > 3488 && n < 3492) || n = 3495 || (n > 3496 && n < 3500) || n = 3501 || n = 3505 || n = 3507 || n = 3511 || (n > 3512 && n < 3516) || n = 3517 || n = 3519 || n = 3525 || n = 3526 || (n > 3528 && n < 3533) || n = 3534 || n = 3535 || n = 3537 || n = 3540 || n = 3541 || n = 3543 || (n > 3544 && n < 3548) || n = 3549 || n = 3551 || n = 3553 || n = 3555 || n = 3558 || n = 3559 || n = 3562 || n = 3565 || n = 3567 || (n > 3568 && n < 3572) || n = 3573 || n = 3577 || n = 3579 || (n > 3580 && n < 3588) || n = 3589)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3440 || n = 3444 || n = 3454 || n = 3484 || n = 3520 || n = 3524 || n = 3538 || n = 3552 || n = 3556 || n = 3566 || n = 3574)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3442)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3443 || n = 3458 || n = 3473 || n = 3533 || n = 3563)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3446 || n = 3464 || n = 3482 || n = 3500 || n = 3518 || n = 3536 || n = 3554 || n = 3572 || n = 3590)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3447 || n = 3485 || n = 3504 || n = 3523 || n = 3542 || n = 3580)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3448 || n = 3468 || n = 3488 || n = 3528 || n = 3548 || n = 3568 || n = 3588)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3449 || n = 3460 || n = 3467 || n = 3481 || n = 3503 || n = 3509 || n = 3521 || n = 3539 || n = 3557 || n = 3575 || n = 3593)
	; 10 bytes
	pha
	delay_n (n-3)
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
	; 11 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 4
	nop
	dex
	dex
	Jne *-6
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
	; 11 bytes
	php
	ldx #24 ;hides 'clc'
	.repeat 4
	 nop
	.endrepeat
	inx
	Jne *-6
	plp
.elseif(n = 3492)
	; 10 bytes
	php
	ldx #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 3496 || n = 3512 || n = 3560 || n = 3576)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
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
	; 11 bytes
	php
	ldx #234 ;hides 'nop'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3591)
	; 11 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	bit $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
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
.if(n = 3594 || n = 3610 || n = 3646 || n = 3664 || n = 3682 || n = 3700 || n = 3736)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3612 || n = 3629 || n = 3697 || n = 3714 || n = 3731 || n = 3748)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif((n > 3595 && n < 3599) || (n > 3599 && n < 3606) || n = 3607 || n = 3609 || (n > 3612 && n < 3618) || (n > 3618 && n < 3623) || n = 3625 || n = 3627 || (n > 3629 && n < 3637) || n = 3639 || (n > 3640 && n < 3644) || n = 3645 || (n > 3648 && n < 3653) || n = 3654 || n = 3655 || n = 3657 || n = 3659 || n = 3661 || (n > 3668 && n < 3672) || n = 3674 || n = 3676 || n = 3677 || n = 3679 || n = 3681 || n = 3684 || n = 3685 || n = 3687 || (n > 3689 && n < 3694) || n = 3695 || n = 3696 || n = 3699 || n = 3703 || (n > 3704 && n < 3708) || (n > 3708 && n < 3713) || n = 3715 || n = 3717 || (n > 3720 && n < 3726) || n = 3727 || n = 3729 || n = 3733 || n = 3735 || (n > 3737 && n < 3743) || n = 3744 || n = 3745 || n = 3749)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3599 || n = 3618 || n = 3637 || n = 3656 || n = 3675 || n = 3694 || n = 3713 || n = 3732)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3606)
	; 11 bytes
	php
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-6
	plp
.elseif(n = 3608 || n = 3626 || n = 3662 || n = 3680 || n = 3698 || n = 3716 || n = 3734)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3611 || n = 3647 || n = 3663 || n = 3665 || n = 3683 || n = 3689 || n = 3701 || n = 3719 || n = 3737)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3623 || n = 3638 || n = 3653 || n = 3743)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3624 || n = 3640 || n = 3672 || n = 3704 || n = 3720)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3628 || n = 3648 || n = 3668 || n = 3688 || n = 3708 || n = 3728)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3644)
	; 9 bytes
	php
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3658)
	; 10 bytes
	php
	ldx #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 3660)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	brk 0
	Jne *-3
	plp
.elseif(n = 3666)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	brk 0
	Jne *-3
	sta (@ptrtemp,x)
	plp
.elseif(n = 3667)
	; 10 bytes
	php
	pha
	ldx #202 ;hides 'dex'
	brk 0
	Jne *-3
	pla
	plp
.elseif(n = 3673)
	; 10 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3678)
	; 11 bytes
	php
	ldx #216 ;hides 'cld'
	rol $26
	ror $26
	dex
	Jne *-6
	plp
.elseif(n = 3686)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	brk 0
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
.elseif(n = 3726)
	; 11 bytes
	php
	ldx #248 ;hides 'sed'
	.repeat 4
	 nop
	.endrepeat
	dex
	Jne *-6
	plp
.elseif(n = 3730)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 9
	inx
	Jne *-6
	plp
.elseif(n = 3746)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3747)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 11
	inx
	Jne *-6
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
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
; 3751 to 3828
.if(n = 3751 || n = 3789 || n = 3827)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3752 || n = 3770 || n = 3788 || n = 3806 || n = 3824)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3753 || n = 3756 || n = 3757 || (n > 3758 && n < 3764) || n = 3767 || n = 3769 || n = 3771 || (n > 3773 && n < 3777) || n = 3778 || n = 3780 || n = 3781 || n = 3783 || (n > 3784 && n < 3788) || (n > 3791 && n < 3798) || n = 3801 || n = 3802 || n = 3804 || n = 3805 || n = 3807 || n = 3810 || n = 3811 || (n > 3812 && n < 3816) || n = 3817 || (n > 3818 && n < 3822) || n = 3823 || n = 3825)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3754 || n = 3772 || n = 3790 || n = 3826)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3755 || n = 3773 || n = 3791 || n = 3809)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3758 || n = 3803 || n = 3818)
	; 11 bytes
	php
	ldx #(n-8)/15
	rol $26
	ror $26
	dex
	Jne *-5
	plp
.elseif(n = 3764)
	; 11 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
	plp
.elseif(n = 3765 || n = 3782 || n = 3799 || n = 3816)
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
.elseif(n = 3768 || n = 3808 || n = 3828)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3777)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
	plp
.elseif(n = 3779)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	inx
	Jne *-5
	plp
.elseif(n = 3784 || n = 3800)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
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
.elseif(n = 3812)
	; 10 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 3822)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.endif
.else
; 3829 to 3906
.if((n > 3828 && n < 3832) || (n > 3833 && n < 3837) || n = 3838 || n = 3841 || n = 3843 || n = 3847 || n = 3849 || (n > 3850 && n < 3858) || n = 3859 || n = 3861 || n = 3869 || (n > 3870 && n < 3877) || n = 3879 || (n > 3884 && n < 3888) || (n > 3888 && n < 3894) || n = 3895 || n = 3897 || n = 3902 || n = 3906)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3832 || n = 3864)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3833 || n = 3850 || n = 3867 || n = 3884)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3837)
	; 10 bytes
	php
	ldx #192 ;hides 'cpy #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.elseif(n = 3839)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3840)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 3842 || n = 3860 || n = 3878 || n = 3896)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3844 || n = 3862 || n = 3880 || n = 3898 || n = 3903)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3845 || n = 3866 || n = 3881 || n = 3899 || n = 3904)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3846 || n = 3865)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 3848 || n = 3868 || n = 3888)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3858)
	; 11 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3863)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	plp
.elseif(n = 3870)
	; 10 bytes
	php
	ldx #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3877)
	; 10 bytes
	php
	ldx #194 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.elseif(n = 3882)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
	plp
.elseif(n = 3883)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 11
	dex
	Jne *-6
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
.elseif(n = 3900)
	; 11 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
	plp
.elseif(n = 3901)
	; 9 bytes
	php
	pha
	lax #15
	jsr delay_256x_a_30_clocks
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
.endif
.endif
.else
; 3907 to 4062
.if(n = 3907 || n = 3909 || n = 3911 || n = 3913 || n = 3915 || (n > 3918 && n < 3922) || (n > 3922 && n < 3926) || n = 3927 || (n > 3928 && n < 3932) || n = 3933 || (n > 3935 && n < 3941) || n = 3942 || n = 3943 || (n > 3944 && n < 3948) || n = 3949 || n = 3951 || n = 3954 || (n > 3955 && n < 3960) || (n > 3960 && n < 3966) || n = 3967 || n = 3972 || n = 3973 || n = 3975 || n = 3977 || n = 3978 || (n > 3979 && n < 3984) || n = 3985 || n = 3987 || (n > 3992 && n < 3998) || (n > 3998 && n < 4003) || n = 4005 || (n > 4009 && n < 4017) || n = 4018 || n = 4019 || n = 4021 || n = 4026 || n = 4027 || (n > 4028 && n < 4036) || n = 4041 || (n > 4043 && n < 4048) || (n > 4048 && n < 4054) || n = 4057 || n = 4059 || n = 4062)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3908 || n = 3928 || n = 3948 || n = 3988 || n = 4008 || n = 4028 || n = 4048)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3910)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3912 || n = 3992 || n = 4056)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 3914 || n = 3932 || n = 3950 || n = 3968 || n = 3986 || n = 4004 || n = 4022 || n = 4040 || n = 4058)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 3916 || n = 3934 || n = 3970 || n = 4024 || n = 4042 || n = 4060)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3917 || n = 3953 || n = 3971 || n = 3989 || n = 4007 || n = 4009 || n = 4025 || n = 4043 || n = 4061)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3918 || n = 3935 || n = 3952 || n = 3969 || n = 4003 || n = 4020 || n = 4037 || n = 4054)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 3922 || n = 3941 || n = 3960 || n = 3979 || n = 3998 || n = 4017 || n = 4036 || n = 4055)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
.elseif(n = 3944)
	; 10 bytes
	php
	ldx #232 ;hides 'inx'
	brk 0
	inx
	inx
	Jne *-5
	plp
.elseif(n = 3955)
	; 10 bytes
	php
	ldx #68 ;hides 'nop zp'
	nop
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 3966)
	; 9 bytes
	php
	ldx #58 ;hides 'nop'
	brk 0
	inx
	Jne *-4
	plp
.elseif(n = 3974)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3976)
	; 10 bytes
	php
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 3984)
	; 11 bytes
	php
	ldx #234 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
	plp
.elseif(n = 3990)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 3991)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4006)
	; 9 bytes
	php
	ldx #56 ;hides 'sec'
	brk 0
	inx
	Jne *-4
	plp
.elseif(n = 4023)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4038)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4039)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	inx
	Jne *-6
	plp
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
; 4063 to 4218
.if((n > 4062 && n < 4066) || n = 4067 || n = 4073 || n = 4075 || n = 4077 || (n > 4079 && n < 4086) || (n > 4088 && n < 4093) || n = 4095 || (n > 4097 && n < 4102) || n = 4106 || n = 4107 || n = 4109 || n = 4111 || n = 4113 || (n > 4115 && n < 4122) || (n > 4123 && n < 4128) || n = 4129 || (n > 4133 && n < 4139) || (n > 4139 && n < 4144) || (n > 4144 && n < 4148) || n = 4149 || (n > 4151 && n < 4156) || n = 4157 || n = 4159 || n = 4162 || n = 4163 || n = 4167 || (n > 4169 && n < 4173) || (n > 4173 && n < 4182) || n = 4183 || n = 4185 || n = 4189 || (n > 4190 && n < 4202) || n = 4203 || (n > 4208 && n < 4218))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4066)
	; 9 bytes
	php
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	plp
.elseif(n = 4068 || n = 4108 || n = 4128 || n = 4168 || n = 4208)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4069 || n = 4079 || n = 4097 || n = 4115 || n = 4151 || n = 4161 || n = 4187 || n = 4205)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4070)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4071 || n = 4088 || n = 4105 || n = 4122 || n = 4139 || n = 4156 || n = 4173 || n = 4190 || n = 4207)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4072)
	; 11 bytes
	php
	ldx #(n-8)/16
	delay_n 11
	dex
	Jne *-5
	plp
.elseif(n = 4074 || n = 4093 || n = 4131 || n = 4150 || n = 4169 || n = 4188)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4076 || n = 4094 || n = 4112 || n = 4130 || n = 4148 || n = 4166 || n = 4184 || n = 4202)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 4078 || n = 4096 || n = 4114 || n = 4132 || n = 4160 || n = 4186 || n = 4204)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4086)
	; 11 bytes
	php
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4087)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4102)
	; 10 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4103)
	; 11 bytes
	php
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 7
	inx
	Jne *-6
	plp
.elseif(n = 4104)
	; 11 bytes
	php
	ldx #0
	delay_n 11
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
.elseif(n = 4123)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4133)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.elseif(n = 4144)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4158)
	; 9 bytes
	php
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4164)
	; 10 bytes
	php
	ldx #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 4165)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4182)
	; 11 bytes
	php
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-6
	plp
.elseif(n = 4206)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 4218)
	; 11 bytes
	php
	ldx #234 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
	plp
.endif
.else
; 4219 to 4375
.if(n = 4219 || n = 4221 || n = 4225 || n = 4227 || (n > 4228 && n < 4238) || n = 4239 || (n > 4241 && n < 4245) || n = 4246 || n = 4247 || (n > 4248 && n < 4256) || n = 4257 || (n > 4259 && n < 4264) || (n > 4264 && n < 4268) || (n > 4269 && n < 4274) || (n > 4277 && n < 4283) || (n > 4283 && n < 4288) || (n > 4288 && n < 4292) || n = 4293 || (n > 4295 && n < 4302) || (n > 4302 && n < 4308) || n = 4311 || (n > 4313 && n < 4321) || (n > 4321 && n < 4326) || n = 4327 || (n > 4331 && n < 4340) || n = 4341 || n = 4342 || n = 4344 || n = 4345 || n = 4347 || (n > 4349 && n < 4359) || n = 4361 || n = 4363 || n = 4365 || (n > 4369 && n < 4376))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4220 || n = 4238 || n = 4256 || n = 4274 || n = 4292 || n = 4310 || n = 4328 || n = 4346 || n = 4364)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 4222 || n = 4240 || n = 4276 || n = 4294 || n = 4312 || n = 4330)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4223 || n = 4259 || n = 4277 || n = 4295 || n = 4313 || n = 4329 || n = 4331 || n = 4349 || n = 4367 || n = 4369)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4224 || n = 4241 || n = 4258 || n = 4275 || n = 4309 || n = 4343)
	; 10 bytes
	php
	ldx #(n-8)/17
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4226 || n = 4245 || n = 4264 || n = 4283 || n = 4302 || n = 4321 || n = 4340 || n = 4359)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4228 || n = 4248 || n = 4268 || n = 4288 || n = 4308 || n = 4348 || n = 4368)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4269)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	plp
.elseif(n = 4326)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	brk 0
	dex
	Jne *-4
	plp
.elseif(n = 4360)
	; 10 bytes
	php
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
	plp
.elseif(n = 4362)
	; 10 bytes
	php
	ldx #58 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 4366)
	; 9 bytes
	php
	ldx #(n-6)/20 ;hides 'nop'
	brk 0
	dex
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 4377 || (n > 4378 && n < 4382) || n = 4383 || n = 4386 || n = 4387 || (n > 4388 && n < 4397) || n = 4398 || n = 4399 || n = 4401 || n = 4404 || n = 4405 || n = 4407 || (n > 4408 && n < 4414) || n = 4419 || (n > 4421 && n < 4428) || (n > 4428 && n < 4435) || n = 4437 || (n > 4439 && n < 4448) || (n > 4448 && n < 4452) || n = 4453 || n = 4455 || (n > 4457 && n < 4468) || n = 4469 || n = 4471 || n = 4476 || (n > 4477 && n < 4483) || (n > 4483 && n < 4488) || n = 4489 || n = 4491 || (n > 4494 && n < 4508) || n = 4509 || (n > 4511 && n < 4516) || (n > 4517 && n < 4526) || n = 4527 || n = 4531)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4378 || n = 4397 || n = 4416 || n = 4435 || n = 4473 || n = 4492 || n = 4511 || n = 4530)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4382 || n = 4400 || n = 4418 || n = 4436 || n = 4454 || n = 4472 || n = 4490 || n = 4508 || n = 4526)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 4384 || n = 4402 || n = 4417 || n = 4420 || n = 4438 || n = 4456 || n = 4474 || n = 4510)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4385 || n = 4403 || n = 4421 || n = 4439 || n = 4457 || n = 4475 || n = 4493 || n = 4529)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4388 || n = 4408 || n = 4428 || n = 4448 || n = 4468 || n = 4488 || n = 4528)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4406)
	; 10 bytes
	php
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 4414)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
	plp
.elseif(n = 4415)
	; 9 bytes
	php
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4452)
	; 10 bytes
	php
	ldx #(n-6)/19 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4470)
	; 11 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 11
	dex
	Jne *-6
	plp
.elseif(n = 4477)
	; 10 bytes
	php
	ldx #224 ;hides 'cpx #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.elseif(n = 4483)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	inx
	Jne *-5
	plp
.elseif(n = 4494)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	nop
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 4516)
	; 10 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 4517)
	; 10 bytes
	php
	ldx #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.endif
.else
; 4532 to 4687
.if((n > 4531 && n < 4542) || n = 4543 || n = 4545 || (n > 4549 && n < 4562) || n = 4563 || n = 4566 || n = 4567 || (n > 4568 && n < 4580) || n = 4581 || n = 4585 || n = 4586 || (n > 4594 && n < 4598) || n = 4599 || (n > 4601 && n < 4605) || (n > 4612 && n < 4616) || n = 4617 || (n > 4619 && n < 4625) || n = 4626 || (n > 4628 && n < 4644) || n = 4645 || n = 4647 || (n > 4649 && n < 4659) || (n > 4659 && n < 4663) || (n > 4663 && n < 4668) || n = 4669 || n = 4671 || n = 4673 || (n > 4675 && n < 4681) || (n > 4682 && n < 4686) || n = 4687)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4542)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4544 || n = 4562 || n = 4580 || n = 4598)
	; 9 bytes
	php
	ldx #(n-8)/18
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 4546 || n = 4564 || n = 4582 || n = 4589 || n = 4591 || n = 4592 || n = 4600 || n = 4607 || n = 4609 || n = 4610 || n = 4618 || n = 4674)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4547 || n = 4565 || n = 4583 || n = 4590 || n = 4601 || n = 4608 || n = 4619 || n = 4649 || n = 4675)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4548 || n = 4568 || n = 4588 || n = 4628 || n = 4648 || n = 4668)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4549 || n = 4625 || n = 4644 || n = 4663 || n = 4682)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4584)
	; 10 bytes
	php
	ldx #(n-6)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4587)
	; 8 bytes
	php
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 4593)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
	sta (@ptrtemp,x)
	plp
.elseif(n = 4594)
	; 10 bytes
	php
	pha
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
	pla
	plp
.elseif(n = 4605)
	; 8 bytes
	php
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 4606)
	; 9 bytes
	php
	ldx #26 ;hides 'nop'
	brk 0
	inx
	Jne *-4
	plp
.elseif(n = 4611)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	nop
	inx
	Jne *-3
	sta (@ptrtemp,x)
	plp
.elseif(n = 4612)
	; 10 bytes
	php
	pha
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
	pla
	plp
.elseif(n = 4616)
	; 9 bytes
	php
	ldx #0
	brk 0
	dex
	Jne *-3
	plp
.elseif(n = 4627)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 4646)
	; 9 bytes
	php
	ldx #24 ;hides 'clc'
	brk 0
	inx
	Jne *-4
	plp
.elseif(n = 4659)
	; 10 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.elseif(n = 4670)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4672)
	; 9 bytes
	php
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4681)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.elseif(n = 4686)
	; 9 bytes
	php
	ldx #(n-6)/20 ;hides 'nop'
	brk 0
	dex
	Jne *-4
	plp
.endif
.endif
.else
; 4688 to 5000
.if(n = 4688 || n = 4708 || n = 4728 || n = 4748 || n = 4768 || n = 4788 || n = 4808 || n = 4828 || n = 4848 || n = 4868 || n = 4888 || n = 4908 || n = 4928 || n = 4948 || n = 4968 || n = 4988)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 4689 || n = 4932 || n = 4969)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4690 || n = 4691 || (n > 4692 && n < 4701) || (n > 4701 && n < 4708) || (n > 4708 && n < 4714) || (n > 4714 && n < 4718) || n = 4719 || (n > 4720 && n < 4728) || (n > 4728 && n < 4739) || (n > 4739 && n < 4748) || (n > 4748 && n < 4756) || n = 4757 || (n > 4758 && n < 4768) || (n > 4768 && n < 4774) || n = 4775 || n = 4776 || (n > 4777 && n < 4784) || (n > 4784 && n < 4788) || (n > 4788 && n < 4795) || (n > 4796 && n < 4802) || (n > 4802 && n < 4808) || (n > 4808 && n < 4815) || (n > 4816 && n < 4828) || (n > 4828 && n < 4834) || n = 4835 || (n > 4836 && n < 4848) || (n > 4848 && n < 4853) || (n > 4853 && n < 4868) || (n > 4868 && n < 4872) || (n > 4872 && n < 4878) || (n > 4878 && n < 4888) || (n > 4888 && n < 4908) || (n > 4908 && n < 4920) || (n > 4920 && n < 4928) || n = 4930 || (n > 4932 && n < 4948) || (n > 4948 && n < 4966) || n = 4967 || (n > 4969 && n < 4988) || (n > 4988 && n < 5001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4692)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4701 || n = 4720 || n = 4739 || n = 4758 || n = 4777 || n = 4796 || n = 4815 || n = 4834 || n = 4853)
	; 10 bytes
	php
	ldx #(n-8)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4714)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4718)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4756)
	; 10 bytes
	php
	ldx #(n-6)/19 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 4774)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4784)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.elseif(n = 4795)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4802)
	; 10 bytes
	php
	ldx #(n-6)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 4816)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 4836)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 4872)
	; 10 bytes
	php
	ldx #0
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	plp
.elseif(n = 4878)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
	plp
.elseif(n = 4920)
	; 10 bytes
	php
	ldx #(n-6)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 4929)
	; 9 bytes
	php
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4931)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4966)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	brk 0
	dex
	Jne *-4
	plp
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
.if((n > 5000 && n < 5006) || n = 5007 || (n > 5009 && n < 5028) || (n > 5028 && n < 5048) || (n > 5048 && n < 5066) || n = 5067 || (n > 5068 && n < 5081) || (n > 5081 && n < 5088) || (n > 5088 && n < 5097) || n = 5098 || (n > 5100 && n < 5108) || n = 5109 || (n > 5110 && n < 5116) || (n > 5120 && n < 5128) || (n > 5128 && n < 5154) || (n > 5154 && n < 5176) || (n > 5176 && n < 5186) || (n > 5189 && n < 5198) || (n > 5198 && n < 5214) || (n > 5214 && n < 5220) || (n > 5220 && n < 5256) || (n > 5256 && n < 5313))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5006)
	; 9 bytes
	php
	ldx #(n-6)/20 ;hides 'nop'
	brk 0
	dex
	Jne *-4
	plp
.elseif(n = 5008 || n = 5028 || n = 5048 || n = 5068 || n = 5088 || n = 5108)
	; 10 bytes
	php
	ldx #(n-8)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 5009 || n = 5100 || n = 5120 || n = 5189)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5066)
	; 10 bytes
	php
	ldx #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 5081)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 7
	brk 0
	Jne *-5
	plp
.elseif(n = 5097)
	; 9 bytes
	php
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 5099 || n = 5118 || n = 5119 || n = 5188)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5110)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
	plp
.elseif(n = 5116)
	; 9 bytes
	php
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 5117)
	; 9 bytes
	php
	ldx #0 ;hides 'brk imm'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 5128)
	; 10 bytes
	php
	ldx #0
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 5154)
	; 10 bytes
	php
	ldx #(n-6)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5176)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 5186)
	; 9 bytes
	php
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5187)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	inx
	Jne *-5
	plp
.elseif(n = 5198)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	nop
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 5214)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 5220)
	; 10 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	brk 0
	inx
	Jne *-5
	plp
.elseif(n = 5256)
	; 10 bytes
	php
	ldx #(n-6)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.endif
.else
; 5313 to 5625
.if((n > 5312 && n < 5352) || (n > 5352 && n < 5363) || (n > 5363 && n < 5372) || (n > 5374 && n < 5396) || (n > 5396 && n < 5443) || n = 5444 || (n > 5446 && n < 5462) || (n > 5462 && n < 5506) || (n > 5506 && n < 5607) || (n > 5607 && n < 5626))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5352)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	bit $24
	dex
	Jne *-5
	plp
.elseif(n = 5363)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk 0'
	dex
	Jne *-5
	plp
.elseif(n = 5372)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	inx
	Jne *-5
	plp
.elseif(n = 5373)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	nop
	bit $24
	dex
	Jne *-5
	plp
.elseif(n = 5374)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	nop
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 5396)
	; 10 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 5443)
	; 9 bytes
	php
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5445)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5446)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5462)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5506)
	; 10 bytes
	php
	ldx #(n-6)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	plp
.elseif(n = 5607)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	plp
.endif
.endif
.else
; 5626 to 6250
.if(n = 5626 || (n > 5629 && n < 5700) || n = 5701 || (n > 5703 && n < 5957) || n = 5958 || (n > 5960 && n < 6214) || n = 6215 || (n > 6217 && n < 6251))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5627)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	dex
	Jne *-5
	plp
.elseif(n = 5628)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	inx
	Jne *-5
	plp
.elseif(n = 5629)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	delay_n 4
	nop
	dex
	Jne *-5
	plp
.elseif(n = 5700)
	; 9 bytes
	php
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5702 || n = 5959 || n = 6216)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5703 || n = 5960 || n = 6217)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5957)
	; 9 bytes
	php
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6214)
	; 9 bytes
	php
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6299) || (n > 6299 && n < 6372) || (n > 6372 && n < 6392) || (n > 6392 && n < 6397) || (n > 6397 && n < 6471) || n = 6472 || (n > 6474 && n < 6728) || n = 6729 || (n > 6731 && n < 6985) || n = 6986 || (n > 6988 && n < 7242) || n = 7243 || (n > 7245 && n < 7499) || n = 7500)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6299)
	; 10 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	plp
.elseif(n = 6372)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 6392)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 6397)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 6471)
	; 9 bytes
	php
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6473 || n = 6730 || n = 6987 || n = 7244)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6474 || n = 6731 || n = 6988 || n = 7245)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6728)
	; 9 bytes
	php
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6985)
	; 9 bytes
	php
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7242)
	; 9 bytes
	php
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7499)
	; 9 bytes
	php
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.else
; 7501 to 10000
.if(n = 7501 || n = 7758 || n = 8015 || n = 8272 || n = 8529 || n = 8786 || n = 9043 || n = 9300 || n = 9557 || n = 9814)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7502 || n = 7759 || n = 8016 || n = 8273 || n = 8530 || n = 8787 || n = 9044 || n = 9301 || n = 9558 || n = 9815)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n > 7502 && n < 7756) || n = 7757 || (n > 7759 && n < 7902) || (n > 7902 && n < 7922) || (n > 7922 && n < 7933) || (n > 7933 && n < 8013) || n = 8014 || (n > 8016 && n < 8270) || n = 8271 || (n > 8273 && n < 8527) || n = 8528 || (n > 8530 && n < 8784) || n = 8785 || (n > 8787 && n < 9041) || n = 9042 || (n > 9044 && n < 9298) || n = 9299 || (n > 9301 && n < 9555) || n = 9556 || (n > 9558 && n < 9812) || n = 9813 || (n > 9815 && n < 10001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7756)
	; 9 bytes
	php
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7902)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 7922)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	inx
	Jne *-5
	plp
.elseif(n = 7933)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	nop
	brk 0
	dex
	Jne *-5
	plp
.elseif(n = 8013)
	; 9 bytes
	php
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8270)
	; 9 bytes
	php
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8527)
	; 9 bytes
	php
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8784)
	; 9 bytes
	php
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9041)
	; 9 bytes
	php
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9298)
	; 9 bytes
	php
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9555)
	; 9 bytes
	php
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9812)
	; 9 bytes
	php
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10069) || n = 10070 || (n > 10072 && n < 10326) || n = 10327 || (n > 10329 && n < 10583) || n = 10584 || (n > 10586 && n < 10840) || n = 10841 || (n > 10843 && n < 11097) || n = 11098 || (n > 11100 && n < 11354) || n = 11355 || (n > 11357 && n < 11611) || n = 11612 || (n > 11614 && n < 11868) || n = 11869 || (n > 11871 && n < 12125) || n = 12126 || (n > 12128 && n < 12382) || n = 12383 || (n > 12385 && n < 12639) || n = 12640 || (n > 12642 && n < 12896) || n = 12897 || (n > 12899 && n < 13153) || n = 13154 || (n > 13156 && n < 13410) || n = 13411 || (n > 13413 && n < 13667) || n = 13668 || (n > 13670 && n < 13924) || n = 13925 || (n > 13927 && n < 14181) || n = 14182 || (n > 14184 && n < 14438) || n = 14439 || (n > 14441 && n < 14695) || n = 14696 || (n > 14698 && n < 14952) || n = 14953 || (n > 14955 && n < 15001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10069)
	; 9 bytes
	php
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10071 || n = 10328 || n = 10585 || n = 10842 || n = 11099 || n = 11356 || n = 11613 || n = 11870 || n = 12127 || n = 12384 || n = 12641 || n = 12898 || n = 13155 || n = 13412 || n = 13669 || n = 13926 || n = 14183 || n = 14440 || n = 14697 || n = 14954)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10072 || n = 10329 || n = 10586 || n = 10843 || n = 11100 || n = 11357 || n = 11614 || n = 11871 || n = 12128 || n = 12385 || n = 12642 || n = 12899 || n = 13156 || n = 13413 || n = 13670 || n = 13927 || n = 14184 || n = 14441 || n = 14698 || n = 14955)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10326)
	; 9 bytes
	php
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10583)
	; 9 bytes
	php
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10840)
	; 9 bytes
	php
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11097)
	; 9 bytes
	php
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11354)
	; 9 bytes
	php
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11611)
	; 9 bytes
	php
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11868)
	; 9 bytes
	php
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12125)
	; 9 bytes
	php
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12382)
	; 9 bytes
	php
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12639)
	; 9 bytes
	php
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12896)
	; 9 bytes
	php
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13153)
	; 9 bytes
	php
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13410)
	; 9 bytes
	php
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13667)
	; 9 bytes
	php
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13924)
	; 9 bytes
	php
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14181)
	; 9 bytes
	php
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14438)
	; 9 bytes
	php
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14695)
	; 9 bytes
	php
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14952)
	; 9 bytes
	php
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15209) || n = 15210 || (n > 15212 && n < 15466) || n = 15467 || (n > 15469 && n < 15723) || n = 15724 || (n > 15726 && n < 15980) || n = 15981 || (n > 15983 && n < 16237) || n = 16238 || (n > 16240 && n < 16494) || n = 16495 || (n > 16497 && n < 16751) || n = 16752 || (n > 16754 && n < 17008) || n = 17009 || (n > 17011 && n < 17265) || n = 17266 || (n > 17268 && n < 17522) || n = 17523 || (n > 17525 && n < 17779) || n = 17780 || (n > 17782 && n < 18036) || n = 18037 || (n > 18039 && n < 18293) || n = 18294 || (n > 18296 && n < 18550) || n = 18551 || (n > 18553 && n < 18807) || n = 18808 || (n > 18810 && n < 19064) || n = 19065 || (n > 19067 && n < 19321) || n = 19322 || (n > 19324 && n < 19578) || n = 19579 || (n > 19581 && n < 19835) || n = 19836 || (n > 19838 && n < 20001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15209)
	; 9 bytes
	php
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15211 || n = 15468 || n = 15725 || n = 15982 || n = 16239 || n = 16496 || n = 16753 || n = 17010 || n = 17267 || n = 17524 || n = 17781 || n = 18038 || n = 18295 || n = 18552 || n = 18809 || n = 19066 || n = 19323 || n = 19580 || n = 19837)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15212 || n = 15469 || n = 15726 || n = 15983 || n = 16240 || n = 16497 || n = 16754 || n = 17011 || n = 17268 || n = 17525 || n = 17782 || n = 18039 || n = 18296 || n = 18553 || n = 18810 || n = 19067 || n = 19324 || n = 19581 || n = 19838)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15466)
	; 9 bytes
	php
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15723)
	; 9 bytes
	php
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15980)
	; 9 bytes
	php
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 16237)
	; 9 bytes
	php
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 16494)
	; 9 bytes
	php
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 16751)
	; 9 bytes
	php
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17008)
	; 9 bytes
	php
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17265)
	; 9 bytes
	php
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17522)
	; 9 bytes
	php
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17779)
	; 9 bytes
	php
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18036)
	; 9 bytes
	php
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18293)
	; 9 bytes
	php
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18550)
	; 9 bytes
	php
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18807)
	; 9 bytes
	php
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19064)
	; 9 bytes
	php
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19321)
	; 9 bytes
	php
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19578)
	; 9 bytes
	php
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19835)
	; 9 bytes
	php
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
