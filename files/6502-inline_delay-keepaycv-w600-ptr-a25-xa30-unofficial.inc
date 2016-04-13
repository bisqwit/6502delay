; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Z+N, and D
; Will preserve A, Y, S, C, V, and I (interrupt-disable flag)
;
; WARNING: May write garbage into $600-$6FF.
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 19
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 1289
; - 8 bytes or less when n <= 1317
; - 9 bytes or less when n <= 2317
; -10 bytes or less when n <= 3188
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.2, max=6
; - When 21 <= n <= 30: min=5, avg=5.8, max=7
; - When 31 <= n <= 50: min=5, avg=6.3, max=7
; - When 51 <= n <= 75: min=5, avg=6.2, max=7
; - When 76 <= n <= 100: min=5, avg=6.3, max=7
; - When 101 <= n <= 200: min=4, avg=6.2, max=7
; - When 201 <= n <= 300: min=5, avg=6.2, max=7
; - When 301 <= n <= 500: min=4, avg=6.2, max=7
; - When 501 <= n <= 750: min=5, avg=6.2, max=7
; - When 751 <= n <= 1000: min=5, avg=6.3, max=7
; - When 1001 <= n <= 2000: min=4, avg=7.0, max=9
; - When 2001 <= n <= 3000: min=6, avg=8.2, max=10
; - When 3001 <= n <= 5000: min=7, avg=9.4, max=11
; - When 5001 <= n <= 7500: min=8, avg=10.7, max=11
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
	; 2 bytes
	ldx $A6
.elseif((n > 3 && n < 7) || (n > 15 && n < 19) || n = 20 || n = 23 || n = 25 || n = 28 || n = 33 || n = 38 || n = 40 || n = 48 || n = 52 || n = 53 || n = 58 || n = 60 || n = 63 || n = 65 || n = 68 || n = 70 || n = 73 || n = 75)
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
.elseif(n = 9 || n = 11 || n = 13 || n = 19 || n = 22)
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
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 21 || n = 26 || n = 36 || n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27 || n = 30)
	; 6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.elseif(n = 32 || n = 42 || n = 72)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif((n .mod 5 = 4) && ((n > 33 && n < 60) || n = 69 || n = 74))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 43 || n = 57)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 45)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 47 || n = 67 || n = 77)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 55)
	; 5 bytes
	ldx #248 ;hides 'sed'
	inx
	Jne *-2
.elseif(n = 62)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 79 to 156
.if((n .mod 5 = 4) && ((n > 78 && n < 95) || (n > 103 && n < 120) || n = 129 || (n > 138 && n < 155)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 80 || n = 115 || n = 150)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 83 || n = 87 || n = 88 || n = 90 || n = 93 || n = 95 || n = 98 || n = 100 || n = 103 || n = 105 || n = 108 || n = 110 || n = 118 || n = 122 || n = 123 || n = 128 || n = 133 || n = 135 || n = 138 || n = 140 || n = 143 || n = 145)
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
.elseif(n = 102 || n = 130 || n = 137 || n = 152)
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
; 157 to 312
.if(n = 157 || n = 158 || n = 163 || n = 165 || n = 168 || n = 173 || n = 175 || n = 178 || n = 180 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 203 || n = 205 || n = 208 || n = 210 || n = 213 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233 || n = 235 || n = 238 || n = 240 || n = 243 || n = 245 || n = 248 || n = 250 || n = 258 || n = 263 || n = 268 || n = 270 || n = 273 || n = 275 || n = 278 || n = 280 || n = 283 || n = 285 || n = 293 || n = 297 || n = 298 || n = 300 || n = 303 || n = 305 || n = 308 || n = 310)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 177 || n = 184 || n = 189 || n = 194 || n = 209 || n = 214 || n = 219 || n = 229 || n = 234 || n = 244 || n = 249 || n = 254 || n = 259 || n = 264 || n = 282 || n = 284 || n = 289 || n = 294 || n = 304)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 160 || n = 272)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 5 = 1) && ((n > 160 && n < 172) || n = 201 || n = 226 || n = 241 || n = 251 || n = 256 || (n > 265 && n < 282) || n = 301))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 7 = 1) && (n = 162 || n = 190 || n = 197 || (n > 252 && n < 275) || (n > 287 && n < 310)))
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
.elseif(n = 170)
	; 6 bytes
	ldx #90 ;hides 'nop'
	inx
	inx
	Jpl *-3
.elseif(n = 172 || n = 217 || n = 262 || n = 307)
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
.elseif((n .mod 5 = 1) && ((n > 175 && n < 197) || (n > 205 && n < 222) || n = 231 || n = 236 || n = 246 || n = 261 || (n > 285 && n < 297) || n = 306 || n = 311))
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
.elseif(n = 182 || n = 187 || n = 202 || n = 212 || n = 222 || n = 237 || n = 252 || n = 292 || n = 312)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 185 || n = 220 || n = 255 || n = 290)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 188)
	; 5 bytes
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jpl *-2
.elseif(n = 199)
	; 6 bytes
	ldx #232 ;hides 'inx'
	nop $04
	Jne *-3
.elseif(n = 207 || n = 247 || n = 257 || n = 287)
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
.elseif(n = 242)
	; 6 bytes
	ldx #(n-8)/9 ;hides 'nop'
	nop
	dex
	Jpl *-3
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
.elseif(n = 299)
	; 6 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
.endif
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 315 || n = 318 || n = 320 || n = 328 || n = 332 || n = 333 || n = 335 || n = 338 || n = 340 || n = 343 || n = 345 || n = 348 || n = 350 || n = 353 || n = 355 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 380 || n = 383 || n = 385 || n = 388 || n = 390 || n = 402 || n = 403 || n = 408 || n = 410 || n = 413 || n = 415 || n = 418 || n = 420 || n = 423 || n = 425 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 448 || n = 450 || n = 453 || n = 455 || n = 458 || n = 460 || n = 468)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314)
	; 6 bytes
	ldx #58 ;hides 'nop'
	inx
	inx
	Jpl *-3
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 322 || n = 347 || n = 357 || n = 377 || n = 387 || n = 392 || n = 432 || n = 447 || n = 462)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 349 || n = 354 || n = 364 || n = 369 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 409 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459 || n = 464)
	; 6..7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 395)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
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
	; 5 bytes
	ldx #58 ;hides 'nop'
	dex
	Jne *-2
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
.if((n .mod 5 = 4) && ((n > 468 && n < 480) || (n > 493 && n < 515) || (n > 523 && n < 540)))
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
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	inx
	Jpl *-3
.elseif(n = 473 || n = 475 || n = 478 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 528 || n = 535 || n = 538 || n = 542 || n = 543 || n = 545)
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
	ldx #68 ;hides 'nop zp'
	nop
	inx
	Jpl *-3
.elseif(n = 482 || n = 492 || n = 497 || n = 502 || n = 522 || n = 537)
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
	ldx #58 ;hides 'nop'
	inx
	Jpl *-2
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
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 517)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 527)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 530)
	; 6 bytes
	ldx #(n-8)/9 ;hides 'nop'
	nop
	dex
	Jpl *-3
.elseif(n = 532)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 544)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	nop
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
.elseif(n = 548 || n = 553 || n = 555 || n = 558 || n = 563 || n = 565 || n = 570 || n = 573 || n = 578 || n = 580 || n = 583 || n = 585 || n = 588 || n = 590 || n = 593 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623 || n = 625)
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
.elseif(n = 552)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	nop
	dex
	Jpl *-3
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
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 562 || n = 567 || n = 587)
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
.elseif(n = 572 || n = 592 || n = 597)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 577)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 602)
	; 6 bytes
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 607)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop $04
	Jmi *-3
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
; 626 to 781
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 5 = 2) && (n = 627 || n = 632 || n = 642 || n = 657 || n = 662 || n = 672 || n = 677 || n = 702 || n = 707 || (n > 731 && n < 748) || (n > 761 && n < 778)))
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 628 || n = 630 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 665 || n = 668 || n = 670 || n = 675 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 693 || n = 695 || n = 698 || n = 700 || n = 703 || n = 705 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 752 || n = 753 || n = 758 || n = 760 || n = 763 || n = 765 || n = 768 || n = 770 || n = 773)
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
.elseif((n .mod 5 = 4) && ((n > 633 && n < 655) || n = 664 || n = 669 || (n > 678 && n < 690) || (n > 698 && n < 725) || (n > 738 && n < 760) || (n > 768 && n < 780)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 640 || n = 730 || n = 775)
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
.elseif(n = 652 || n = 659 || n = 729)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 667 || n = 685 || n = 712)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 674)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jpl *-3
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
.elseif(n = 710 || n = 745 || n = 780)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 713)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
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
.elseif(n = 755)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
.endif
.else
; 782 to 937
.if(n = 782 || n = 807 || n = 812 || n = 837 || n = 842 || n = 852 || n = 867 || n = 872 || n = 882 || n = 887 || n = 902 || n = 907 || n = 912 || n = 922)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 810 || n = 818 || n = 822 || n = 823 || n = 825 || n = 828 || n = 830 || n = 833 || n = 835 || n = 838 || n = 840 || n = 843 || n = 845 || n = 857 || n = 858 || n = 863 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 903 || n = 905 || n = 908 || n = 910 || n = 913 || n = 915 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
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
.elseif(n = 802 || n = 847)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 809)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	dex
	Jne *-3
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
; 938 to 1250
.if(n = 938 || n = 940 || n = 943 || n = 945 || n = 948 || n = 950 || n = 958 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 983 || n = 985 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1063 || n = 1067 || n = 1068 || n = 1070 || n = 1073 || n = 1075 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1088 || n = 1090 || n = 1098 || n = 1102 || n = 1103 || n = 1105 || n = 1108 || n = 1110 || n = 1113 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1133 || n = 1137 || n = 1138 || n = 1140 || n = 1143 || n = 1145 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1158 || n = 1160 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1183 || n = 1185 || n = 1188 || n = 1190 || n = 1193 || n = 1195 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1225 || n = 1228 || n = 1230 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || (n > 1092 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942 || n = 947 || n = 952 || n = 957 || n = 977 || n = 987 || n = 992 || n = 1012 || n = 1020 || n = 1022 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1082 || n = 1087 || n = 1092 || n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1152 || n = 1157 || n = 1167 || n = 1182 || n = 1192 || n = 1197 || n = 1202 || n = 1217 || n = 1222 || n = 1227 || n = 1232)
	; 6..7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1039 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1089 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1129 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1209 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244 || n = 1249)
	; 6..7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 955 || n = 990 || n = 1025 || n = 1060 || n = 1095 || n = 1130 || n = 1165 || n = 1200 || n = 1235)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 980)
	; 6 bytes
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 982 || n = 1027 || n = 1162)
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
.elseif(n = 1057 || n = 1237)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1097)
	; 6 bytes
	ldx #122 ;hides 'nop'
	nop
	dex
	Jne *-3
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
.elseif(n = 1187)
	; 7 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1250)
	; 6 bytes
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
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
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1263 || n = 1265 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1290 || n = 1291 || n = 1293 || n = 1298 || n = 1300 || n = 1305 || n = 1307 || n = 1308 || n = 1311 || n = 1312 || n = 1319 || n = 1326 || n = 1328)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1325)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1257 || n = 1262 || n = 1267 || n = 1272 || n = 1287 || n = 1302 || n = 1316)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1259)
	; 6 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1292 || n = 1299 || n = 1318 || n = 1320 || n = 1327)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1288)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1294)
	; 7 bytes
	php
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1295)
	; 7 bytes
	nop #<$0C ;hides 'nop $70A2'
	ldx #112
	inx
	Jne *-4
.elseif(n = 1301)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 1304)
	; 7 bytes
	nop #<$0C ;hides 'nop $91A2'
	ldx #145
	dex
	Jne *-4
.elseif(n = 1309)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1313)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 1314)
	; 7 bytes
	nop <$0C ;hides 'nop $6EA2'
	ldx #110
	inx
	Jne *-4
.elseif(n = 1321)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1322)
	; 7 bytes
	nop #<$0C ;hides 'nop $6DA2'
	ldx #109
	inx
	Jne *-4
.elseif(n = 1323)
	; 7 bytes
	nop <$0C ;hides 'nop $6DA2'
	ldx #109
	inx
	Jne *-4
.elseif(n = 1325)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 6
	inx
	Jpl *-5
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1337 || n = 1353)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
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
	nop <$0C ;hides 'nop $6CA2'
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
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1339)
	; 7 bytes
	ldx #122 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 1340 || n = 1344 || n = 1346 || n = 1347 || n = 1354 || n = 1356 || n = 1361)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1341)
	; 7 bytes
	ldx #122 ;hides 'nop'
	delay_n 4
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
	nop #<$0C ;hides 'nop $96A2'
	ldx #150
	dex
	Jne *-4
.elseif(n = 1350)
	; 7 bytes
	nop <$0C ;hides 'nop $6AA2'
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
	nop #<$0C ;hides 'nop $97A2'
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
	nop <$0C ;hides 'nop $04A2'
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
	nop #<$0C ;hides 'nop $68A2'
	ldx #104
	inx
	Jne *-4
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1370 || n = 1371 || n = 1375 || n = 1382 || n = 1384 || n = 1389 || n = 1403)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1369 || n = 1378 || n = 1396 || n = 1405)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1372 || n = 1391 || n = 1400)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	nop $04
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
	nop <$0C ;hides 'nop $66A2'
	ldx #102
	inx
	Jne *-4
.elseif(n = 1388)
	; 7 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1395)
	; 7 bytes
	nop <$0C ;hides 'nop $9BA2'
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
	nop <$0C ;hides 'nop $64A2'
	ldx #100
	inx
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1410 || n = 1416 || n = 1417 || n = 1419 || n = 1424 || n = 1431 || n = 1434 || n = 1437 || n = 1438 || n = 1445)
	; 7..9 bytes
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
	nop $04
	dex
	Jne *-3
.elseif(n = 1411 || n = 1418 || n = 1444)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1412)
	; 7 bytes
	nop #<$0C ;hides 'nop $9DA2'
	ldx #157
	dex
	Jne *-4
.elseif(n = 1413)
	; 7 bytes
	nop <$0C ;hides 'nop $9DA2'
	ldx #157
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1421)
	; 7 bytes
	nop #<$0C ;hides 'nop $9EA2'
	ldx #158
	dex
	Jne *-4
.elseif(n = 1426 || n = 1428 || n = 1435)
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
	nop #<$0C ;hides 'nop $9FA2'
	ldx #159
	dex
	Jne *-4
.elseif(n = 1439)
	; 7 bytes
	nop #<$0C ;hides 'nop $A0A2'
	ldx #160
	dex
	Jne *-4
.elseif(n = 1440)
	; 7 bytes
	nop <$0C ;hides 'nop $60A2'
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
.elseif(n = 1447 || n = 1452 || n = 1454 || n = 1466 || n = 1469 || n = 1470 || n = 1473 || n = 1479 || n = 1480 || n = 1482)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1448)
	; 7 bytes
	nop #<$0C ;hides 'nop $A1A2'
	ldx #161
	dex
	Jne *-4
.elseif(n = 1449 || n = 1481)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
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
	nop $04
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
	nop <$0C ;hides 'nop $A2A2'
	ldx #162
	dex
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
	nop #<$0C ;hides 'nop $85A2'
	ldx #133
	nop
	dex
	Jne *-5
.elseif(n = 1467)
	; 7 bytes
	nop <$0C ;hides 'nop $A3A2'
	ldx #163
	dex
	Jne *-4
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
	nop #<$0C ;hides 'nop $A4A2'
	ldx #164
	dex
	Jne *-4
.elseif(n = 1476)
	; 7 bytes
	nop <$0C ;hides 'nop $A4A2'
	ldx #164
	dex
	Jne *-4
.elseif(n = 1483)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 1484)
	; 7 bytes
	nop #<$0C ;hides 'nop $A5A2'
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
.elseif(n = 1487 || n = 1490 || n = 1491 || n = 1494 || n = 1498 || n = 1501 || n = 1508 || n = 1510 || n = 1515)
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
.elseif(n = 1497)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jmi *-4
.elseif(n = 1500 || n = 1518)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1502)
	; 7 bytes
	nop #<$0C ;hides 'nop $A7A2'
	ldx #167
	dex
	Jne *-4
.elseif(n = 1503)
	; 7 bytes
	nop <$0C ;hides 'nop $A7A2'
	ldx #167
	dex
	Jne *-4
.elseif(n = 1504)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	nop
	inx
	Jne *-3
.elseif(n = 1505 || n = 1521)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
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
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1517)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1519)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
	dex
	Jne *-2
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
	; 7 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1529 || n = 1533 || n = 1535 || n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1552 || n = 1557)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1530)
	; 7 bytes
	nop <$0C ;hides 'nop $AAA2'
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
.elseif(n = 1537 || n = 1561)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 1544 || n = 1551)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1545 || n = 1553)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 1546)
	; 9 bytes
	ldx #102
	rol $26
	ror $26
	dex
	Jpl *-5
.elseif(n = 1547)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	inx
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
	; 7 bytes
	ldx #100 ;hides 'nop zp'
	delay_n 4
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1564 || n = 1571 || n = 1578 || n = 1580 || n = 1592 || n = 1599)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565)
	; 7 bytes
	nop #<$0C ;hides 'nop $AEA2'
	ldx #174
	dex
	Jne *-4
.elseif(n = 1566)
	; 7 bytes
	nop <$0C ;hides 'nop $AEA2'
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
	sta (@ptrtemp,x)
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
.elseif(n = 1572 || n = 1579 || n = 1586 || n = 1600)
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
	nop #<$0C ;hides 'nop $AFA2'
	ldx #175
	dex
	Jne *-4
.elseif(n = 1575)
	; 7 bytes
	nop <$0C ;hides 'nop $AFA2'
	ldx #175
	dex
	Jne *-4
.elseif(n = 1577 || n = 1593 || n = 1601)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 1581)
	; 8 bytes
	ldx #(n-21)/20
	jsr @nearby_rts_15cyc
	dex
	Jpl *-4
.elseif(n = 1584)
	; 7 bytes
	nop <$0C ;hides 'nop $B0A2'
	ldx #176
	dex
	Jne *-4
.elseif(n = 1587)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1588)
	; 9 bytes
	php
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 1591)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1595)
	; 8 bytes
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jmi *-5
.elseif(n = 1598)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jmi *-5
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 7 bytes
	nop <$0C ;hides 'nop $B2A2'
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
.elseif(n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1620 || n = 1622 || n = 1627 || n = 1634 || n = 1636)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1607 || n = 1614 || n = 1626 || n = 1635 || n = 1640)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1615 || n = 1624 || n = 1629 || n = 1631)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1616 || n = 1630)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1617)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 1619)
	; 7 bytes
	nop #<$0C ;hides 'nop $B4A2'
	ldx #180
	dex
	Jne *-4
.elseif(n = 1623)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
.elseif(n = 1628)
	; 7 bytes
	nop #<$0C ;hides 'nop $B5A2'
	ldx #181
	dex
	Jne *-4
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
	nop <$0C ;hides 'nop $B6A2'
	ldx #182
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641 || n = 1655 || n = 1658 || n = 1662 || n = 1668 || n = 1669 || n = 1671 || n = 1676 || n = 1678)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1642 || n = 1663 || n = 1670 || n = 1677)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1643 || n = 1652)
	; 7..8 bytes
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
	nop <$0C ;hides 'nop $B7A2'
	ldx #183
	dex
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
	nop $04
	dex
	Jne *-3
.elseif(n = 1650)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1651)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1654)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 1656)
	; 7 bytes
	nop <$0C ;hides 'nop $B8A2'
	ldx #184
	dex
	Jne *-4
.elseif(n = 1659)
	; 7 bytes
	ldx #90 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 1661)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 1664)
	; 7 bytes
	nop #<$0C ;hides 'nop $B9A2'
	ldx #185
	dex
	Jne *-4
.elseif(n = 1672)
	; 8 bytes
	nop <$0C ;hides 'nop $98A2'
	ldx #152
	nop
	dex
	Jne *-5
.elseif(n = 1679)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jne *-5
.endif
.else
; 1680 to 1718
.if(n = 1680 || n = 1687 || n = 1694 || n = 1708 || n = 1715)
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
.elseif(n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1697 || n = 1699 || n = 1703 || n = 1704 || n = 1706 || n = 1714 || n = 1718)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1693 || n = 1711)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1689 || n = 1713)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 1691)
	; 7 bytes
	nop #<$0C ;hides 'nop $BCA2'
	ldx #188
	dex
	Jne *-4
.elseif(n = 1692)
	; 7 bytes
	nop <$0C ;hides 'nop $BCA2'
	ldx #188
	dex
	Jne *-4
.elseif(n = 1696)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1698 || n = 1712)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1700)
	; 7 bytes
	nop #<$0C ;hides 'nop $BDA2'
	ldx #189
	dex
	Jne *-4
.elseif(n = 1701)
	; 7 bytes
	nop <$0C ;hides 'nop $BDA2'
	ldx #189
	dex
	Jne *-4
.elseif(n = 1705 || n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
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
	nop <$0C ;hides 'nop $BEA2'
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
	nop <$0C ;hides 'nop $BFA2'
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
	nop $04
	dex
	Jne *-3
.elseif(n = 1722 || n = 1725 || n = 1731 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1748 || n = 1753)
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
	nop #<$0C ;hides 'nop $C0A2'
	ldx #192
	dex
	Jne *-4
.elseif(n = 1728)
	; 7 bytes
	nop <$0C ;hides 'nop $C0A2'
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
	nop #<$0C ;hides 'nop $C1A2'
	ldx #193
	dex
	Jne *-4
.elseif(n = 1741)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1749)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1750)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1752)
	; 7 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1754)
	; 7 bytes
	nop #<$0C ;hides 'nop $C3A2'
	ldx #195
	dex
	Jne *-4
.elseif(n = 1755 || n = 1757)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 1760 || n = 1762 || n = 1766 || n = 1767 || n = 1788 || n = 1795)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1761 || n = 1769 || n = 1785)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 1763)
	; 7 bytes
	nop #<$0C ;hides 'nop $C4A2'
	ldx #196
	dex
	Jne *-4
.elseif(n = 1764)
	; 7 bytes
	nop <$0C ;hides 'nop $C4A2'
	ldx #196
	dex
	Jne *-4
.elseif(n = 1768 || n = 1780 || n = 1796)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1770)
	; 8 bytes
	nop #<$0C ;hides 'nop $A1A2'
	ldx #161
	nop
	dex
	Jne *-5
.elseif(n = 1771 || n = 1778)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1773)
	; 7 bytes
	nop <$0C ;hides 'nop $C5A2'
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
	delay_n (n-4)
	delay_n 4
.elseif(n = 1777 || n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1781)
	; 6 bytes
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 1782)
	; 7 bytes
	nop <$0C ;hides 'nop $C6A2'
	ldx #198
	dex
	Jne *-4
.elseif(n = 1784)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1787)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 1790)
	; 7 bytes
	nop #<$0C ;hides 'nop $C7A2'
	ldx #199
	dex
	Jne *-4
.elseif(n = 1791)
	; 7 bytes
	nop <$0C ;hides 'nop $C7A2'
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
	nop <$0C ;hides 'nop $C8A2'
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
	nop <$0C ;hides 'nop $A4A2'
	ldx #164
	nop
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
	nop #<$0C ;hides 'nop $C9A2'
	ldx #201
	dex
	Jne *-4
.elseif(n = 1809 || n = 1817 || n = 1833)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 1811)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
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
	nop #<$0C ;hides 'nop $A5A2'
	ldx #165
	nop
	dex
	Jne *-5
.elseif(n = 1818)
	; 7 bytes
	nop <$0C ;hides 'nop $CAA2'
	ldx #202
	dex
	Jne *-4
.elseif(n = 1820)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
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
	nop <$0C ;hides 'nop $CBA2'
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
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 7 bytes
	nop <$0C ;hides 'nop $CCA2'
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
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1840)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1841 || n = 1857 || n = 1865)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
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
.elseif(n = 1843 || n = 1848 || n = 1851 || n = 1856 || n = 1859 || n = 1866 || n = 1867 || n = 1874)
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
	nop <$0C ;hides 'nop $CDA2'
	ldx #205
	dex
	Jne *-4
.elseif(n = 1847)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1849 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1850)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1852)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1853)
	; 7 bytes
	nop #<$0C ;hides 'nop $CEA2'
	ldx #206
	dex
	Jne *-4
.elseif(n = 1854)
	; 7 bytes
	nop <$0C ;hides 'nop $CEA2'
	ldx #206
	dex
	Jne *-4
.elseif(n = 1858)
	; 8 bytes
	nop #<$0C ;hides 'nop $A9A2'
	ldx #169
	nop
	dex
	Jne *-5
.elseif(n = 1862)
	; 7 bytes
	nop #<$0C ;hides 'nop $CFA2'
	ldx #207
	dex
	Jne *-4
.elseif(n = 1863)
	; 7 bytes
	nop <$0C ;hides 'nop $CFA2'
	ldx #207
	dex
	Jne *-4
.elseif(n = 1868)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
.elseif(n = 1869)
	; 8 bytes
	nop #<$0C ;hides 'nop $AAA2'
	ldx #170
	nop
	dex
	Jne *-5
.elseif(n = 1870)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1871)
	; 7 bytes
	nop #<$0C ;hides 'nop $D0A2'
	ldx #208
	dex
	Jne *-4
.elseif(n = 1872)
	; 7 bytes
	nop <$0C ;hides 'nop $D0A2'
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
.elseif(n = 1878)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $D0A2'
	ldx #208
	dex
	Jne *-4
	plp
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
.elseif(n = 1881 || n = 1889 || n = 1905 || n = 1913)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1910)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1883 || n = 1884 || n = 1886 || n = 1887 || n = 1892 || n = 1911)
	; 8..9 bytes
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
	ldx #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
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
.elseif(n = 1896 || n = 1912)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1898)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1899)
	; 7 bytes
	nop <$0C ;hides 'nop $D3A2'
	ldx #211
	dex
	Jne *-4
.elseif(n = 1901)
	; 8 bytes
	ldx #(n-21)/20
	jsr @nearby_rts_15cyc
	dex
	Jpl *-4
.elseif(n = 1902)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1903)
	; 8 bytes
	nop <$0C ;hides 'nop $ADA2'
	ldx #173
	nop
	dex
	Jne *-5
.elseif(n = 1907)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1908)
	; 7 bytes
	nop <$0C ;hides 'nop $D4A2'
	ldx #212
	dex
	Jne *-4
.elseif(n = 1910)
	; 8 bytes
	delay_n (n-3)
	nop $04
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1916)
	; 6 bytes
	ldx #(n-8)/9 ;hides 'nop zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1917)
	; 7 bytes
	nop <$0C ;hides 'nop $D5A2'
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
.elseif(n = 1920 || n = 1928)
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
	nop #<$0C ;hides 'nop $AFA2'
	ldx #175
	nop
	dex
	Jne *-5
.elseif(n = 1925)
	; 7 bytes
	nop #<$0C ;hides 'nop $D6A2'
	ldx #214
	dex
	Jne *-4
.elseif(n = 1926)
	; 7 bytes
	nop <$0C ;hides 'nop $D6A2'
	ldx #214
	dex
	Jne *-4
.elseif(n = 1929)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 1930)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1931)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
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
.endif
.else
; 1934 to 1953
.if(n = 1934)
	; 7 bytes
	nop #<$0C ;hides 'nop $D7A2'
	ldx #215
	dex
	Jne *-4
.elseif(n = 1935)
	; 7 bytes
	nop <$0C ;hides 'nop $D7A2'
	ldx #215
	dex
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
	nop $04
	dex
	Jne *-3
.elseif(n = 1938 || n = 1947)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1939)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1940)
	; 8 bytes
	ldx #194 ;hides 'nop #imm'
	nop $FF44 ;hides 'nop zp'
	dex
	Jne *-5
.elseif(n = 1941)
	; 8 bytes
	ldx #(n-21)/20
	jsr @nearby_rts_15cyc
	dex
	Jpl *-4
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
	nop <$0C ;hides 'nop $D8A2'
	ldx #216
	dex
	Jne *-4
.elseif(n = 1946)
	; 8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1948)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1949)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1950)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1951)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
	dex
	Jne *-4
.elseif(n = 1952)
	; 7 bytes
	nop #<$0C ;hides 'nop $D9A2'
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
.elseif(n = 1955 || n = 1956 || n = 1965 || n = 1974 || n = 1982 || n = 1983 || n = 1987)
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
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 1959 || n = 1992)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1960 || n = 1976 || n = 1984)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1961)
	; 6 bytes
	ldx #218 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 1962)
	; 7 bytes
	nop <$0C ;hides 'nop $DAA2'
	ldx #218
	dex
	Jne *-4
.elseif(n = 1964)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1966)
	; 9 bytes
	php
	ldx #196 ;hides 'cpy zp'
	cmp $EA ;hides 'nop'
	dex
	Jne *-4
	plp
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
	nop #<$0C ;hides 'nop $DBA2'
	ldx #219
	dex
	Jne *-4
.elseif(n = 1971)
	; 7 bytes
	nop <$0C ;hides 'nop $DBA2'
	ldx #219
	dex
	Jne *-4
.elseif(n = 1973)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1975)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1977 || n = 1985)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 1978)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1979)
	; 7 bytes
	ldx #58 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 1980)
	; 7 bytes
	nop <$0C ;hides 'nop $DCA2'
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
	nop #<$0C ;hides 'nop $DDA2'
	ldx #221
	dex
	Jne *-4
.elseif(n = 1989)
	; 7 bytes
	nop <$0C ;hides 'nop $DDA2'
	ldx #221
	dex
	Jne *-4
.elseif(n = 1991)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
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
.elseif(n = 1995 || n = 2000 || n = 2010 || n = 2018 || n = 2019 || n = 2027 || n = 2028)
	; 8 bytes
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
	nop #<$0C ;hides 'nop $DEA2'
	ldx #222
	dex
	Jne *-4
.elseif(n = 1998)
	; 7 bytes
	nop <$0C ;hides 'nop $DEA2'
	ldx #222
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1998 && n < 2027)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2001 || n = 2009 || n = 2025)
	; 7 bytes
	ldx #(n-1)/8
	nop $04
	dex
	Jne *-3
.elseif(n = 2002)
	; 8 bytes
	nop <$0C ;hides 'nop $B6A2'
	ldx #182
	nop
	dex
	Jne *-5
.elseif(n = 2003 || n = 2014)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2004)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $DEA2'
	ldx #222
	dex
	Jne *-4
	plp
.elseif(n = 2006)
	; 7 bytes
	nop #<$0C ;hides 'nop $DFA2'
	ldx #223
	dex
	Jne *-4
.elseif(n = 2007)
	; 7 bytes
	nop <$0C ;hides 'nop $DFA2'
	ldx #223
	dex
	Jne *-4
.elseif(n = 2011)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
	dex
	Jne *-4
.elseif(n = 2012)
	; 8 bytes
	nop #<$0C ;hides 'nop $B7A2'
	ldx #183
	nop
	dex
	Jne *-5
.elseif(n = 2013)
	; 8 bytes
	nop <$0C ;hides 'nop $B7A2'
	ldx #183
	nop
	dex
	Jne *-5
.elseif(n = 2015)
	; 7 bytes
	nop #<$0C ;hides 'nop $E0A2'
	ldx #224
	dex
	Jne *-4
.elseif(n = 2016)
	; 7 bytes
	nop <$0C ;hides 'nop $E0A2'
	ldx #224
	dex
	Jne *-4
.elseif(n = 2020)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2021)
	; 8 bytes
	ldx #(n-21)/20
	jsr @nearby_rts_15cyc
	dex
	Jpl *-4
.elseif(n = 2022)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $E0A2'
	ldx #224
	dex
	Jne *-4
	plp
.elseif(n = 2023)
	; 8 bytes
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 2024)
	; 7 bytes
	nop #<$0C ;hides 'nop $1FA2'
	ldx #31
	inx
	Jne *-4
.elseif(n = 2030)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
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
; 2032 to 2070
.if(n = 2032)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
.elseif(n = 2033)
	; 7 bytes
	ldx #<$8D ;hides 'sta $1EA2'
	ldx #30
	inx
	Jne *-4
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
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2037 || n = 2039 || n = 2046 || n = 2054 || n = 2055 || n = 2064 || n = 2066 || n = 2067)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2038)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2040 || n = 2048)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2041 || n = 2065)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2042)
	; 7 bytes
	nop #<$0C ;hides 'nop $E3A2'
	ldx #227
	dex
	Jne *-4
.elseif(n = 2043)
	; 7 bytes
	nop <$0C ;hides 'nop $E3A2'
	ldx #227
	dex
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
	nop $04
	dex
	Jne *-3
.elseif(n = 2050)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2051)
	; 7 bytes
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	inx
	Jne *-4
.elseif(n = 2052)
	; 7 bytes
	nop <$0C ;hides 'nop $E4A2'
	ldx #228
	dex
	Jne *-4
.elseif(n = 2056)
	; 8 bytes
	nop #<$0C ;hides 'nop $BBA2'
	ldx #187
	nop
	dex
	Jne *-5
.elseif(n = 2057)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	nop $04
	inx
	Jne *-5
.elseif(n = 2060)
	; 7 bytes
	nop #<$0C ;hides 'nop $E5A2'
	ldx #229
	dex
	Jne *-4
.elseif(n = 2061)
	; 7 bytes
	nop <$0C ;hides 'nop $E5A2'
	ldx #229
	dex
	Jne *-4
.elseif(n = 2063)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jne *-5
.elseif(n = 2068)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	nop
	nop $04
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
	nop <$0C ;hides 'nop $E6A2'
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
.elseif(n = 2072)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2073 || n = 2082 || n = 2099 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074 || n = 2081 || n = 2083 || n = 2090 || n = 2108)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2075 || n = 2092)
	; 8 bytes
	ldx #(n-18)/17
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
	nop <$0C ;hides 'nop $19A2'
	ldx #25
	inx
	Jne *-4
.elseif(n = 2084 || n = 2093 || n = 2095)
	; 8..9 bytes
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
	nop <$0C ;hides 'nop $18A2'
	ldx #24
	inx
	Jne *-4
.elseif(n = 2091 || n = 2102)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2096)
	; 7 bytes
	nop #<$0C ;hides 'nop $E9A2'
	ldx #233
	dex
	Jne *-4
.elseif(n = 2097)
	; 7 bytes
	nop <$0C ;hides 'nop $E9A2'
	ldx #233
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
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	inx
	Jne *-4
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
.elseif(n = 2111 || n = 2120 || n = 2138)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 2115)
	; 7 bytes
	nop <$0C ;hides 'nop $EBA2'
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
	nop $04
.elseif(n = 2118 || n = 2127 || n = 2136 || n = 2139 || n = 2144 || n = 2145)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2119 || n = 2128 || n = 2140)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
	nop #<$0C ;hides 'nop $C1A2'
	ldx #193
	nop
	dex
	Jne *-5
.elseif(n = 2123)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 2124)
	; 7 bytes
	nop <$0C ;hides 'nop $ECA2'
	ldx #236
	dex
	Jne *-4
.elseif(n = 2126)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
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
.elseif(n = 2131)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
	dex
	Jne *-4
.elseif(n = 2132)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2133)
	; 7 bytes
	nop <$0C ;hides 'nop $13A2'
	ldx #19
	inx
	Jne *-4
.elseif(n = 2135 || n = 2146)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2141)
	; 7 bytes
	nop #<$0C ;hides 'nop $EEA2'
	ldx #238
	dex
	Jne *-4
.elseif(n = 2142)
	; 7 bytes
	nop <$0C ;hides 'nop $EEA2'
	ldx #238
	dex
	Jne *-4
.elseif(n = 2147)
	; 8 bytes
	ldx #232 ;hides 'inx'
	nop $04
	inx
	inx
	Jne *-5
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
	nop #<$0C ;hides 'nop $11A2'
	ldx #17
	inx
	Jne *-4
.elseif(n = 2151)
	; 7 bytes
	nop <$0C ;hides 'nop $11A2'
	ldx #17
	inx
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
	nop #<$0C ;hides 'nop $C4A2'
	ldx #196
	nop
	dex
	Jne *-5
.elseif(n = 2156)
	; 8 bytes
	nop <$0C ;hides 'nop $C4A2'
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
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2159)
	; 7 bytes
	ldx #216 ;hides 'cld'
	nop $04
	dex
	Jne *-4
.elseif(n = 2160)
	; 7 bytes
	nop <$0C ;hides 'nop $10A2'
	ldx #16
	inx
	Jne *-4
.elseif(n = 2164 || n = 2182)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2166)
	; 8 bytes
	nop #<$0C ;hides 'nop $C5A2'
	ldx #197
	nop
	dex
	Jne *-5
.elseif(n = 2167)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2168)
	; 7 bytes
	nop #<$0C ;hides 'nop $0FA2'
	ldx #15
	inx
	Jne *-4
.elseif(n = 2169)
	; 7 bytes
	nop <$0C ;hides 'nop $F1A2'
	ldx #241
	dex
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
	ldx #<$8D ;hides 'sta $0EA2'
	ldx #14
	inx
	Jne *-4
.elseif(n = 2178)
	; 7 bytes
	nop <$0C ;hides 'nop $0EA2'
	ldx #14
	inx
	Jne *-4
.elseif(n = 2183)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	inx
	Jpl *-5
.elseif(n = 2184)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	plp
.elseif(n = 2186)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 2187)
	; 7 bytes
	nop <$0C ;hides 'nop $F3A2'
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
.elseif(n = 2189 || n = 2207)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2190 || n = 2201 || n = 2212)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2191)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
	dex
	Jne *-4
.elseif(n = 2192 || n = 2210)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2193 || n = 2202)
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
.elseif(n = 2198 || n = 2203 || n = 2208 || n = 2211 || n = 2216 || n = 2217 || n = 2219 || n = 2225 || n = 2226)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2199)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2200)
	; 8 bytes
	nop <$0C ;hides 'nop $C8A2'
	ldx #200
	nop
	dex
	Jne *-5
.elseif(n = 2204)
	; 6 bytes
	ldx #(n-8)/9 ;hides 'nop zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 2205)
	; 7 bytes
	nop <$0C ;hides 'nop $0BA2'
	ldx #11
	inx
	Jne *-4
.elseif(n = 2209 || n = 2221)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2213)
	; 7 bytes
	nop #<$0C ;hides 'nop $F6A2'
	ldx #246
	dex
	Jne *-4
.elseif(n = 2214)
	; 7 bytes
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	inx
	Jne *-4
.elseif(n = 2218)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2220)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jne *-4
	plp
.elseif(n = 2222)
	; 7 bytes
	nop #<$0C ;hides 'nop $09A2'
	ldx #9
	inx
	Jne *-4
.elseif(n = 2223)
	; 7 bytes
	nop <$0C ;hides 'nop $F7A2'
	ldx #247
	dex
	Jne *-4
.endif
.else
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227)
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
.elseif(n = 2229 || n = 2235 || n = 2244)
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
.elseif(n = 2233 || n = 2242)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2234)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2236)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2237)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2238)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2239)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 2240)
	; 7 bytes
	nop #<$0C ;hides 'nop $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2241)
	; 7 bytes
	nop <$0C ;hides 'nop $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2243)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2245)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.endif
.else
; 2246 to 2265
.if(n = 2246)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
.elseif(n = 2247 || n = 2253 || n = 2262)
	; 8 bytes
	delay_n (n-2)
	nop
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
	nop <$0C ;hides 'nop $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2251 || n = 2260)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2252)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2254)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2255)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2256)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2257)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2258)
	; 7 bytes
	nop #<$0C ;hides 'nop $05A2'
	ldx #5
	inx
	Jne *-4
.elseif(n = 2259)
	; 7 bytes
	nop <$0C ;hides 'nop $05A2'
	ldx #5
	inx
	Jne *-4
.elseif(n = 2261)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2263)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2264)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2265)
	; 8 bytes
	nop #<$0C ;hides 'nop $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
.if((n >= 2266) && (n < 2285))
; 2266 to 2284
.if(n = 2266)
	; 8 bytes
	nop <$0C ;hides 'nop $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
.elseif(n = 2267)
	; 7 bytes
	nop #<$0C ;hides 'nop $04A2'
	ldx #4
	inx
	Jne *-4
.elseif(n = 2268)
	; 7 bytes
	nop <$0C ;hides 'nop $04A2'
	ldx #4
	inx
	Jne *-4
.elseif(n = 2269 || n = 2278)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2270 || n = 2271 || n = 2273 || n = 2280 || n = 2282)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2272)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 2276)
	; 7 bytes
	nop #<$0C ;hides 'nop $03A2'
	ldx #3
	inx
	Jne *-4
.elseif(n = 2277)
	; 7 bytes
	nop <$0C ;hides 'nop $03A2'
	ldx #3
	inx
	Jne *-4
.elseif(n = 2279)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2281)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2283)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2284)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	inx
	Jne *-4
	plp
.endif
.else
; 2285 to 2304
.if(n = 2285)
	; 7 bytes
	nop #<$0C ;hides 'nop $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2286)
	; 7 bytes
	nop <$0C ;hides 'nop $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2287 || n = 2296)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2288 || n = 2298)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2289 || n = 2300)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2290)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2291)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
	dex
	Jne *-4
.elseif(n = 2292)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	inx
	Jne *-4
	plp
.elseif(n = 2293)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2294)
	; 7 bytes
	nop #<$0C ;hides 'nop $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2295)
	; 7 bytes
	nop <$0C ;hides 'nop $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2297)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2299)
	; 7 bytes
	ldx #26 ;hides 'nop'
	nop $04
	inx
	Jne *-4
.elseif(n = 2301)
	; 8 bytes
	ldx #(n-21)/20
	jsr @nearby_rts_15cyc
	dex
	Jpl *-4
.elseif(n = 2302)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	inx
	Jne *-4
	plp
.elseif(n = 2303)
	; 7 bytes
	nop #<$0C ;hides 'nop $00A2'
	ldx #0
	inx
	Jne *-4
.elseif(n = 2304)
	; 7 bytes
	nop <$0C ;hides 'nop $00A2'
	ldx #0
	dex
	Jne *-4
.endif
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
.elseif(n = 2308)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2309)
	; 8 bytes
	nop #<$0C ;hides 'nop $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
.elseif(n = 2310)
	; 8 bytes
	nop <$0C ;hides 'nop $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
.elseif(n = 2311 || n = 2322)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2312 || n = 2318)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2313)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2314)
	; 9 bytes
	nop <$0C ;hides 'nop $B2A2'
	ldx #178
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2315)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2316)
	; 9 bytes
	nop <$0C ;hides 'nop $C1A2'
	ldx #193
	nop $04
	dex
	Jne *-6
.elseif(n = 2317)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2319)
	; 7 bytes
	ldx #24 ;hides 'clc'
	nop $04
	inx
	Jne *-4
.elseif(n = 2320)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2321)
	; 8 bytes
	ldx #(n-21)/20
	jsr @nearby_rts_15cyc
	dex
	Jpl *-4
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
	; 9 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 2327 || n = 2332 || n = 2343)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2328)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
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
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2333)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2334)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2335)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 2336)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jmi *-5
.elseif(n = 2337)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2338)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2339)
	; 7 bytes
	ldx #234 ;hides 'nop'
	nop $04
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
.elseif(n = 2345 || n = 2350 || n = 2363 || n = 2368 || n = 2373 || n = 2379)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2346)
	; 9 bytes
	php
	ldx #234 ;hides 'nop'
	bit $24
	dex
	Jne *-4
	plp
.elseif(n = 2347 || n = 2364)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2348 || n = 2360 || n = 2362 || n = 2372 || n = 2382)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2349)
	; 9 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2351 || n = 2371)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
	dex
	Jne *-4
.elseif(n = 2352)
	; 9 bytes
	nop <$0C ;hides 'nop $C4A2'
	ldx #196
	nop $04
	dex
	Jne *-6
.elseif(n = 2353 || n = 2365 || n = 2377)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2354)
	; 8 bytes
	nop <$0C ;hides 'nop $D6A2'
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
.elseif(n = 2357 || n = 2376)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2358)
	; 10 bytes
	php
	ldx #235
	nop
	cmp $C5
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
.elseif(n = 2361)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
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
.elseif(n = 2370)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
	plp
.elseif(n = 2374)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 2375)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2378)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2380)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2390 || n = 2391 || n = 2394 || n = 2400 || n = 2403 || n = 2405 || n = 2412 || n = 2415 || n = 2417)
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
	nop #<$0C ;hides 'nop $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
.elseif(n = 2387)
	; 8 bytes
	nop <$0C ;hides 'nop $D9A2'
	ldx #217
	nop
	dex
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
.elseif(n = 2407)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2408)
	; 8 bytes
	nop #<$0C ;hides 'nop $DBA2'
	ldx #219
	nop
	dex
	Jne *-5
.elseif(n = 2411)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
	dex
	Jne *-4
.elseif(n = 2414)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2416)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2418)
	; 9 bytes
	nop <$0C ;hides 'nop $BAA2'
	ldx #186
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2419)
	; 8 bytes
	nop #<$0C ;hides 'nop $DCA2'
	ldx #220
	nop
	dex
	Jne *-5
.elseif(n = 2420)
	; 8 bytes
	nop <$0C ;hides 'nop $DCA2'
	ldx #220
	nop
	dex
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
	nop <$0C ;hides 'nop $CAA2'
	ldx #202
	nop $04
	dex
	Jne *-6
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
	nop #<$0C ;hides 'nop $DDA2'
	ldx #221
	nop
	dex
	Jne *-5
.elseif(n = 2431)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
	dex
	Jne *-4
.elseif(n = 2432 || n = 2443 || n = 2454)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
	; 9 bytes
	nop <$0C ;hides 'nop $CBA2'
	ldx #203
	nop $04
	dex
	Jne *-6
.elseif(n = 2441 || n = 2455)
	; 8..9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2442 || n = 2444 || n = 2456)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2445 || n = 2458)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
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
.elseif(n = 2448)
	; 9 bytes
	nop <$0C ;hides 'nop $CCA2'
	ldx #204
	nop $04
	dex
	Jne *-6
.elseif(n = 2450)
	; 9 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2452)
	; 8 bytes
	ldx #(n-20)/19
	jsr @nearby_rts_14cyc
	dex
	Jpl *-4
.elseif(n = 2453)
	; 8 bytes
	nop <$0C ;hides 'nop $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
.elseif(n = 2457)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	nop
	nop $04
	inx
	Jne *-6
.elseif(n = 2459)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	inx
	Jne *-5
.elseif(n = 2460)
	; 9 bytes
	ldx #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jmi *-6
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
	; 10 bytes
	ldx #106
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jpl *-6
.elseif(n = 2463 || n = 2467 || n = 2475 || n = 2477 || n = 2478 || n = 2489 || n = 2495)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2464)
	; 8 bytes
	nop <$0C ;hides 'nop $E0A2'
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
.elseif(n = 2466 || n = 2483 || n = 2500)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2468 || n = 2480 || n = 2492 || n = 2494)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2469)
	; 9 bytes
	nop #<$0C ;hides 'nop $BEA2'
	ldx #190
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2470)
	; 9 bytes
	nop <$0C ;hides 'nop $BEA2'
	ldx #190
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2471 || n = 2490)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2472)
	; 9 bytes
	nop <$0C ;hides 'nop $CEA2'
	ldx #206
	nop $04
	dex
	Jne *-6
.elseif(n = 2474)
	; 8 bytes
	nop #<$0C ;hides 'nop $1FA2'
	ldx #31
	nop
	inx
	Jne *-5
.elseif(n = 2479)
	; 7 bytes
	ldx #248 ;hides 'sed'
	nop $04
	dex
	Jne *-4
.elseif(n = 2481)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2482)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2484)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2486)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2488)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2491)
	; 8 bytes
	ldx #(n-1)/10
	nop
	nop $04
	dex
	Jne *-4
.elseif(n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2496)
	; 8 bytes
	nop #<$0C ;hides 'nop $E3A2'
	ldx #227
	nop
	dex
	Jne *-5
.elseif(n = 2499)
	; 7 bytes
	ldx #250 ;hides 'nop'
	nop $04
	dex
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
.if(n = 2501)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2502 || n = 2511 || n = 2513 || n = 2522 || n = 2523 || n = 2525)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2503 || n = 2515 || n = 2539)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2504 || n = 2516 || n = 2538)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2505)
	; 9 bytes
	ldx #232 ;hides 'inx'
	nop
	nop $04
	inx
	inx
	Jne *-6
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
	nop <$0C ;hides 'nop $E4A2'
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
	ldx #116 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
.elseif(n = 2517 || n = 2534)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2518)
	; 8 bytes
	nop #<$0C ;hides 'nop $E5A2'
	ldx #229
	nop
	dex
	Jne *-5
.elseif(n = 2519)
	; 8 bytes
	nop <$0C ;hides 'nop $E5A2'
	ldx #229
	nop
	dex
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
	nop <$0C ;hides 'nop $E6A2'
	ldx #230
	nop
	dex
	Jne *-5
.elseif(n = 2532)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2537)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2541 || n = 2561)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2541 && n < 2576)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2543)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	nop $04
	dex
	Jne *-5
.elseif(n = 2544 || n = 2547 || n = 2559 || n = 2565 || n = 2570 || n = 2571)
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
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2548 || n = 2554 || n = 2560 || n = 2572)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
	nop <$0C ;hides 'nop $18A2'
	ldx #24
	nop
	inx
	Jne *-5
.elseif(n = 2555)
	; 8 bytes
	ldx #(n-11)/12 ;hides 'nop zp,x'
	inx ;first loop only
	nop $04
	dex
	Jne *-5
.elseif(n = 2556)
	; 9 bytes
	nop <$0C ;hides 'nop $D5A2'
	ldx #213
	nop $04
	dex
	Jne *-6
.elseif(n = 2558 || n = 2576)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2562)
	; 8 bytes
	nop #<$0C ;hides 'nop $E9A2'
	ldx #233
	nop
	dex
	Jne *-5
.elseif(n = 2566)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2567)
	; 9 bytes
	ldx #234 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jmi *-6
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
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	nop
	inx
	Jne *-5
.elseif(n = 2578)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	inx
	Jne *-6
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579 || n = 2583 || n = 2596 || n = 2607 || n = 2609 || n = 2610)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2580 || n = 2588 || n = 2600 || n = 2612 || n = 2613)
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
.elseif(n = 2582)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jmi *-5
.elseif(n = 2584)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2585 || n = 2602)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2586 || n = 2597 || n = 2608)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2587)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2589)
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
	nop <$0C ;hides 'nop $D8A2'
	ldx #216
	nop $04
	dex
	Jne *-6
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
.elseif(n = 2598)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2601)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2603)
	; 9 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
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
.elseif(n = 2611)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2614)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2615)
	; 8 bytes
	ldx #218 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
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
.if(n = 2618)
	; 8 bytes
	nop <$0C ;hides 'nop $EEA2'
	ldx #238
	nop
	dex
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
.elseif(n = 2621)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2623 || n = 2642)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2624 || n = 2637 || n = 2648)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2625)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
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
	nop #<$0C ;hides 'nop $11A2'
	ldx #17
	nop
	inx
	Jne *-5
.elseif(n = 2629 || n = 2641 || n = 2653)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2631 || n = 2632 || n = 2634 || n = 2635 || n = 2643 || n = 2645 || n = 2654 || n = 2655)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2633 || n = 2647)
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
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
.elseif(n = 2639)
	; 8 bytes
	nop #<$0C ;hides 'nop $10A2'
	ldx #16
	nop
	inx
	Jne *-5
.elseif(n = 2640)
	; 8 bytes
	nop <$0C ;hides 'nop $10A2'
	ldx #16
	nop
	inx
	Jne *-5
.elseif(n = 2646)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 2649)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	inx
	Jne *-5
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
	delay_n 6
	inx
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if(n = 2657 || n = 2658 || n = 2667 || n = 2669 || n = 2676 || n = 2679 || n = 2691 || n = 2693)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2659 || n = 2678 || n = 2688)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2660)
	; 9 bytes
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
	nop <$0C ;hides 'nop $0EA2'
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
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2668)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2670 || n = 2687)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2671 || n = 2686)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2672)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2673)
	; 8 bytes
	nop <$0C ;hides 'nop $F3A2'
	ldx #243
	nop
	dex
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
	ldx #12 ;hides 'nop abs'
	nop $0
	nop
	inx
	Jne *-5
.elseif(n = 2690)
	; 9 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
.elseif(n = 2694)
	; 7 bytes
	ldx #(n-10)/11 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2695)
	; 8 bytes
	nop <$0C ;hides 'nop $0BA2'
	ldx #11
	nop
	inx
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
.elseif(n = 2697)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2698 || n = 2709 || n = 2714 || n = 2715 || n = 2719 || n = 2722 || n = 2723 || n = 2733)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2699)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2700 || n = 2702)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	nop #<$0C ;hides 'nop $F6A2'
	ldx #246
	nop
	dex
	Jne *-5
.elseif(n = 2706)
	; 8 bytes
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
.elseif(n = 2708 || n = 2720 || n = 2732 || n = 2734)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2710)
	; 9 bytes
	ldx #128
	nop
	jsr @nearby_rts_14cyc
	dex
	Jpl *-5
.elseif(n = 2711)
	; 9 bytes
	nop #<$0C ;hides 'nop $E2A2'
	ldx #226
	nop $04
	dex
	Jne *-6
.elseif(n = 2712)
	; 8 bytes
	ldx #194 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2716)
	; 8 bytes
	nop #<$0C ;hides 'nop $09A2'
	ldx #9
	nop
	inx
	Jne *-5
.elseif(n = 2724)
	; 9 bytes
	nop <$0C ;hides 'nop $E3A2'
	ldx #227
	nop $04
	dex
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
	nop <$0C ;hides 'nop $08A2'
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
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 2736)
	; 9 bytes
	nop <$0C ;hides 'nop $E4A2'
	ldx #228
	nop $04
	dex
	Jne *-6
.elseif((n .mod 12 = 1) && ((n > 2736 && n < 2774)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2738 || n = 2755 || n = 2772)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2739 || n = 2747 || n = 2758 || (n > 2762 && n < 2766))
	; 8..9 bytes
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
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2742)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 2743)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2744 || n = 2766)
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
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 2748)
	; 9 bytes
	nop <$0C ;hides 'nop $E5A2'
	ldx #229
	nop $04
	dex
	Jne *-6
.elseif(n = 2750)
	; 8 bytes
	nop <$0C ;hides 'nop $06A2'
	ldx #6
	nop
	inx
	Jne *-5
.elseif(n = 2752)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2753)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dex
	Jne *-5
.elseif(n = 2754)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2756)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2757 || n = 2770)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2760)
	; 8 bytes
	nop #<$0C ;hides 'nop $05A2'
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
.elseif(n = 2769)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2771)
	; 7 bytes
	ldx #58 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2775 || n = 2777 || n = 2786 || n = 2803 || n = 2808 || n = 2810 || n = 2811)
	; 8..10 bytes
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
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2779)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2781)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2782)
	; 8 bytes
	nop #<$0C ;hides 'nop $03A2'
	ldx #3
	nop
	inx
	Jne *-5
.elseif(n = 2783)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	nop $04
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
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
	nop #<$0C ;hides 'nop $02A2'
	ldx #2
	nop
	inx
	Jne *-5
.elseif(n = 2794)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
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
	nop #<$0C ;hides 'nop $01A2'
	ldx #1
	nop
	inx
	Jne *-5
.elseif(n = 2805)
	; 8 bytes
	nop <$0C ;hides 'nop $01A2'
	ldx #1
	nop
	inx
	Jne *-5
.elseif(n = 2807)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 6
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
	nop <$0C ;hides 'nop $00A2'
	ldx #0
	nop
	dex
	Jne *-5
.elseif(n = 2817)
	; 8 bytes
	ldx #0
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2818 || n = 2823 || n = 2825 || n = 2826 || n = 2834 || n = 2835 || n = 2837 || n = 2838 || n = 2842 || n = 2847 || n = 2849)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2819)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop $04
	inx
	Jne *-5
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
.elseif(n = 2827 || n = 2839)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2828 || n = 2848 || n = 2850)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2830)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	inx
	Jne *-5
.elseif(n = 2831)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	nop $04
	inx
	Jne *-5
.elseif(n = 2836)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
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
.elseif(n = 2846)
	; 9 bytes
	nop #<$0C ;hides 'nop $DBA2'
	ldx #219
	delay_n 4
	dex
	Jne *-6
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2858 || n = 2864 || n = 2876 || n = 2888)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2853 || n = 2859 || n = 2862 || n = 2882 || n = 2883 || n = 2890)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2854)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	inx
	Jne *-6
.elseif(n = 2855)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	inx
	Jne *-6
.elseif(n = 2856)
	; 9 bytes
	nop <$0C ;hides 'nop $EEA2'
	ldx #238
	nop $04
	dex
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
.elseif(n = 2861)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
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
	nop #<$0C ;hides 'nop $11A2'
	ldx #17
	nop $04
	inx
	Jne *-6
.elseif(n = 2868)
	; 9 bytes
	nop <$0C ;hides 'nop $11A2'
	ldx #17
	nop $04
	inx
	Jne *-6
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
	; 9 bytes
	php
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2874)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2875 || n = 2877)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2878)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 2879)
	; 9 bytes
	nop #<$0C ;hides 'nop $10A2'
	ldx #16
	nop $04
	inx
	Jne *-6
.elseif(n = 2880)
	; 9 bytes
	nop <$0C ;hides 'nop $10A2'
	ldx #16
	nop $04
	inx
	Jne *-6
.elseif(n = 2886)
	; 9 bytes
	nop <$0C ;hides 'nop $DEA2'
	ldx #222
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2887)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
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
	; 9 bytes
	nop <$0C ;hides 'nop $F1A2'
	ldx #241
	nop $04
	dex
	Jne *-6
.elseif((n .mod 12 = 1) && ((n > 2892 && n < 2930)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2894 || n = 2920)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2895 || n = 2897 || n = 2902 || n = 2903 || n = 2907 || n = 2909 || n = 2910 || n = 2914 || n = 2919 || n = 2922)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2896 || n = 2911)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2898)
	; 9 bytes
	nop #<$0C ;hides 'nop $DFA2'
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
.elseif(n = 2900 || n = 2906 || n = 2912 || n = 2924)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2901 || n = 2921)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2904)
	; 9 bytes
	nop <$0C ;hides 'nop $0EA2'
	ldx #14
	nop $04
	inx
	Jne *-6
.elseif(n = 2915)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop $04
	dex
	Jne *-5
.elseif(n = 2916)
	; 9 bytes
	nop <$0C ;hides 'nop $F3A2'
	ldx #243
	nop $04
	dex
	Jne *-6
.elseif(n = 2918)
	; 9 bytes
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 2923)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2926)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	dex
	Jne *-5
.elseif(n = 2928)
	; 9 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	nop $04
	inx
	Jne *-6
.endif
.else
; 2930 to 2968
.if(n = 2930 || n = 2932 || n = 2944 || n = 2968)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2931 || n = 2933 || n = 2943 || n = 2945 || n = 2954 || n = 2958 || n = 2963 || n = 2967)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2934 || n = 2936 || n = 2948 || n = 2960)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2935)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2937)
	; 9 bytes
	ldx #<$8D ;hides 'sta $1EA2'
	ldx #30
	delay_n 4
	inx
	Jne *-6
.elseif(n = 2938)
	; 9 bytes
	ldx #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2939)
	; 8 bytes
	ldx #(n-11)/12 ;hides 'nop zp,x'
	inx ;first loop only
	nop $04
	dex
	Jne *-5
.elseif(n = 2940)
	; 9 bytes
	nop <$0C ;hides 'nop $0BA2'
	ldx #11
	nop $04
	inx
	Jne *-6
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
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 2949)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 2950)
	; 9 bytes
	nop #<$0C ;hides 'nop $E3A2'
	ldx #227
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2951)
	; 9 bytes
	nop #<$0C ;hides 'nop $F6A2'
	ldx #246
	nop $04
	dex
	Jne *-6
.elseif(n = 2952)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
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
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2962)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 2964)
	; 9 bytes
	nop <$0C ;hides 'nop $F7A2'
	ldx #247
	nop $04
	dex
	Jne *-6
.elseif(n = 2966)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
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
.elseif(n = 2970 || n = 2973 || n = 2974 || n = 2979 || n = 2985 || n = 2991 || n = 2994 || n = 2998 || n = 3003 || n = 3005 || n = 3006)
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
.elseif(n = 2972 || n = 2996)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2975)
	; 8 bytes
	ldx #248 ;hides 'sed'
	nop
	nop $04
	dex
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
.elseif(n = 2981)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 2982)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2984)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 2987)
	; 9 bytes
	nop #<$0C ;hides 'nop $07A2'
	ldx #7
	nop $04
	inx
	Jne *-6
.elseif(n = 2988)
	; 9 bytes
	nop <$0C ;hides 'nop $07A2'
	ldx #7
	nop $04
	inx
	Jne *-6
.elseif(n = 2990)
	; 9 bytes
	nop <$0C ;hides 'nop $E6A2'
	ldx #230
	delay_n 4
	dex
	Jne *-6
.elseif(n = 2995 || n = 3007)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2999)
	; 8 bytes
	ldx #250 ;hides 'nop'
	nop
	nop $04
	dex
	Jne *-5
.elseif(n = 3000)
	; 9 bytes
	nop <$0C ;hides 'nop $06A2'
	ldx #6
	nop $04
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
.elseif(n = 3012 || n = 3024 || n = 3027 || n = 3029 || n = 3033 || n = 3034 || n = 3045)
	; 8..10 bytes
	delay_n (n-2)
	nop
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
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3035)
	; 9 bytes
	nop #<$0C ;hides 'nop $03A2'
	ldx #3
	nop $04
	inx
	Jne *-6
.elseif(n = 3036)
	; 9 bytes
	nop <$0C ;hides 'nop $03A2'
	ldx #3
	nop $04
	inx
	Jne *-6
.elseif(n = 3038)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3041)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3042)
	; 9 bytes
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3044)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047)
	; 9 bytes
	nop #<$0C ;hides 'nop $02A2'
	ldx #2
	nop $04
	inx
	Jne *-6
.elseif(n = 3048)
	; 9 bytes
	nop <$0C ;hides 'nop $02A2'
	ldx #2
	nop $04
	inx
	Jne *-6
.elseif(n = 3049 || n = 3061)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3050)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3051)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
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
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 3055 || n = 3062 || n = 3063 || n = 3070 || n = 3075 || n = 3077 || (n > 3082 && n < 3086))
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3056 || n = 3058 || n = 3068)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3057)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3059)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3060 || n = 3079)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3065)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	inx
	Jne *-5
.elseif(n = 3066)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 3069 || n = 3082)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3071)
	; 9 bytes
	nop #<$0C ;hides 'nop $00A2'
	ldx #0
	nop $04
	inx
	Jne *-6
.elseif(n = 3072)
	; 9 bytes
	nop <$0C ;hides 'nop $00A2'
	ldx #0
	nop $04
	dex
	Jne *-6
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3074)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3076)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3078)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
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
.if(n = 3086 || n = 3102 || n = 3116 || n = 3124)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3087)
	; 9 bytes
	nop #<$0C ;hides 'nop $C1A2'
	ldx #193
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3088)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3089 || n = 3105)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3090)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
.elseif(n = 3091 || n = 3106)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3092 || n = 3096 || n = 3097 || n = 3099 || n = 3100 || n = 3110 || n = 3111 || n = 3114 || n = 3119 || n = 3125)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3093)
	; 9 bytes
	nop #<$0C ;hides 'nop $EEA2'
	ldx #238
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3094)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3095 || n = 3109 || n = 3123)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3098 || n = 3117)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3101 || n = 3121)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3103)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3104)
	; 9 bytes
	ldx #194 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3107)
	; 9 bytes
	nop <$0C ;hides 'nop $11A2'
	ldx #17
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3108)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3113)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 3115)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3118)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3120)
	; 9 bytes
	nop <$0C ;hides 'nop $10A2'
	ldx #16
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3122)
	; 10 bytes
	nop <$0C ;hides 'nop $DFA2'
	ldx #223
	nop
	nop $04
	dex
	Jne *-7
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
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3127 || n = 3131 || n = 3138 || n = 3139 || n = 3143 || (n > 3147 && n < 3151) || n = 3157 || n = 3162)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3128 || n = 3144)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3129 || n = 3146 || n = 3163)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3130)
	; 9 bytes
	php
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3132)
	; 9 bytes
	nop #<$0C ;hides 'nop $0FA2'
	ldx #15
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3133)
	; 9 bytes
	nop <$0C ;hides 'nop $F1A2'
	ldx #241
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3134 || n = 3147)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3135)
	; 9 bytes
	nop #<$0C ;hides 'nop $C4A2'
	ldx #196
	delay_n 7
	dex
	Jne *-6
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
.elseif(n = 3140)
	; 9 bytes
	ldx #100 ;hides 'nop zp'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 3141 || n = 3161)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3142)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3145)
	; 9 bytes
	ldx #<$8D ;hides 'sta $0EA2'
	ldx #14
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3152)
	; 9 bytes
	nop <$0C ;hides 'nop $C5A2'
	ldx #197
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3153)
	; 8 bytes
	ldx #90 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3156)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3158)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3159)
	; 9 bytes
	nop <$0C ;hides 'nop $F3A2'
	ldx #243
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3160)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3164)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-6
.endif
.else
.if((n >= 3165) && (n < 3184))
; 3165 to 3183
.if(n = 3165 || n = 3179)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3166)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3167)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3168)
	; 9 bytes
	nop <$0C ;hides 'nop $C6A2'
	ldx #198
	delay_n 7
	dex
	Jne *-6
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
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3172)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3173)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3174)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3175 || n = 3176 || n = 3182 || n = 3183)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3177)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	nop
	dex
	Jne *-6
.elseif(n = 3178)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3180)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3181)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.endif
.else
; 3184 to 3203
.if(n = 3184)
	; 8 bytes
	ldx #(n-12)/13 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 3185)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3186)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3187)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3188 || n = 3195 || n = 3203)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3189)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3190)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3191)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3192)
	; 10 bytes
	nop <$0C ;hides 'nop $E4A2'
	ldx #228
	nop
	nop $04
	dex
	Jne *-7
.elseif(n = 3193)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3194)
	; 9 bytes
	ldx #(n-14)/15 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-6
.elseif(n = 3196)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3197)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3198)
	; 9 bytes
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3200)
	; 9 bytes
	nop <$0C ;hides 'nop $C8A2'
	ldx #200
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3201)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3202)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.endif
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if(n = 3204 || n = 3206 || n = 3208 || n = 3228 || n = 3230)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3205)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3207 || n = 3221 || n = 3235)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3209 || n = 3216 || n = 3218 || n = 3227 || n = 3237 || n = 3240)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3210)
	; 9 bytes
	nop #<$0C ;hides 'nop $09A2'
	ldx #9
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3211 || n = 3226)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3212)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3213)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3214 || n = 3231)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3215)
	; 9 bytes
	nop #<$0C ;hides 'nop $C9A2'
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
.elseif(n = 3219)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3220 || n = 3229)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3222)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3223)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3224)
	; 9 bytes
	nop <$0C ;hides 'nop $08A2'
	ldx #8
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3225 || n = 3238)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3232)
	; 9 bytes
	nop <$0C ;hides 'nop $CAA2'
	ldx #202
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3234)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3236)
	; 9 bytes
	nop #<$0C ;hides 'nop $07A2'
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
.elseif(n = 3241)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3242)
	; 9 bytes
	ldx #116 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3245 || n = 3252 || n = 3260 || n = 3266 || n = 3267 || n = 3273)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3244)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3246 || n = 3274)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
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
.elseif(n = 3249 || n = 3263 || n = 3277)
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
.elseif(n = 3251 || n = 3264)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3253 || n = 3255)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3254 || n = 3270 || n = 3272)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3256 || n = 3271)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3257)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3258)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3259)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3261 || n = 3281)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3262)
	; 9 bytes
	nop #<$0C ;hides 'nop $05A2'
	ldx #5
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3268 || n = 3278)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3276)
	; 9 bytes
	nop <$0C ;hides 'nop $04A2'
	ldx #4
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3279)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3280)
	; 9 bytes
	nop <$0C ;hides 'nop $CDA2'
	ldx #205
	delay_n 7
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3299 || n = 3316)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283 || n = 3284 || n = 3293 || n = 3300 || n = 3309 || n = 3318 || n = 3320)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285 || n = 3294 || n = 3304 || n = 3308 || n = 3310)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3286)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3287)
	; 10 bytes
	pha
	ldx #37 ;hides 'and zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
	pla
.elseif(n = 3288 || n = 3307)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3289)
	; 9 bytes
	nop <$0C ;hides 'nop $03A2'
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
.elseif(n = 3292)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3295)
	; 9 bytes
	nop #<$0C ;hides 'nop $CEA2'
	ldx #206
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3296)
	; 9 bytes
	nop <$0C ;hides 'nop $CEA2'
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
.elseif(n = 3298)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3301)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3302)
	; 9 bytes
	nop <$0C ;hides 'nop $02A2'
	ldx #2
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3306)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3311)
	; 9 bytes
	nop #<$0C ;hides 'nop $CFA2'
	ldx #207
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3312)
	; 9 bytes
	nop <$0C ;hides 'nop $CFA2'
	ldx #207
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3314)
	; 9 bytes
	nop #<$0C ;hides 'nop $01A2'
	ldx #1
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3315)
	; 9 bytes
	nop <$0C ;hides 'nop $01A2'
	ldx #1
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3317)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	nop $04
	inx
	Jne *-6
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3341)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3322)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3323)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3324 || n = 3336)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3325 || n = 3332 || n = 3339 || n = 3353 || n = 3356)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3326 || n = 3345)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3327)
	; 9 bytes
	nop #<$0C ;hides 'nop $00A2'
	ldx #0
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3328 || n = 3330 || n = 3335 || n = 3337 || n = 3342 || n = 3343 || n = 3348 || n = 3349 || n = 3351 || n = 3352)
	; 9..10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3333 || n = 3347)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3334)
	; 10 bytes
	nop #<$0C ;hides 'nop $91A2'
	ldx #145
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 3338)
	; 10 bytes
	nop #<$0C ;hides 'nop $9FA2'
	ldx #159
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3340)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3344)
	; 9 bytes
	nop <$0C ;hides 'nop $D1A2'
	ldx #209
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3350)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3354 || n = 3357)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3355)
	; 10 bytes
	ldx #128
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jpl *-6
.elseif(n = 3358)
	; 10 bytes
	nop <$0C ;hides 'nop $6EA2'
	ldx #110
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 3359)
	; 9 bytes
	nop #<$0C ;hides 'nop $D2A2'
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
	nop <$0C ;hides 'nop $D2A2'
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
.elseif(n = 3362 || n = 3363 || n = 3366 || n = 3369 || n = 3371 || n = 3378 || n = 3385 || n = 3388 || n = 3395)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3364 || n = 3383)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3365)
	; 9 bytes
	ldx #58 ;hides 'nop'
	rol $26
	ror $26
	inx
	Jne *-6
.elseif(n = 3367 || n = 3384)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3368 || n = 3374 || n = 3396)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3370)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3372)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3373 || n = 3390 || n = 3397)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3376)
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
	nop #<$0C ;hides 'nop $A1A2'
	ldx #161
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3381)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3382)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3386)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3387)
	; 9 bytes
	php
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3391)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3392)
	; 9 bytes
	nop <$0C ;hides 'nop $D4A2'
	ldx #212
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3394)
	; 10 bytes
	delay_n (n-3)
	nop $04
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
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3402 || n = 3421)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3403 || n = 3417 || n = 3431)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3404 || n = 3405 || n = 3413 || n = 3419 || n = 3420 || n = 3423 || n = 3426 || n = 3433 || n = 3437)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3406 || n = 3436)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3407)
	; 8 bytes
	ldx #(n-15)/16 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3408)
	; 9 bytes
	nop <$0C ;hides 'nop $D5A2'
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
	; 9 bytes
	ldx #100 ;hides 'nop zp'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 3411)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
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
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dex
	Jne *-6
.elseif(n = 3415)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	dex
	Jne *-6
.elseif(n = 3416)
	; 10 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	nop
	nop $04
	inx
	Jne *-7
.elseif(n = 3422)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
	plp
.elseif(n = 3424)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3427)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3428)
	; 10 bytes
	php
	ldx #171
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	plp
.elseif(n = 3429)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	nop $04
	dex
	Jne *-6
.elseif(n = 3430)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3432)
	; 9 bytes
	ldx #100 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
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
	nop #<$0C ;hides 'nop $D7A2'
	ldx #215
	delay_n 7
	dex
	Jne *-6
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
.elseif((n > 3441 && n < 3445) || n = 3447 || n = 3454 || n = 3463 || n = 3468 || n = 3475)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3445 || n = 3459 || n = 3473)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3446 || n = 3456 || n = 3465 || n = 3467)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3448 || n = 3453 || n = 3458 || n = 3474 || n = 3476)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3449)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3450)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 3451 || n = 3466)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3452 || n = 3469)
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
.elseif(n = 3457)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3460 || n = 3470)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3464)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
.elseif(n = 3471)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3472)
	; 9 bytes
	nop <$0C ;hides 'nop $D9A2'
	ldx #217
	delay_n 7
	dex
	Jne *-6
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3492)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3478 || n = 3497)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3479)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3480 || n = 3482 || n = 3483 || n = 3488 || n = 3498 || n = 3507 || n = 3513)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3481)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3484 || n = 3490 || n = 3500)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 3489 || n = 3505)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3491)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3493)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3494 || n = 3506 || n = 3510)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3495)
	; 10 bytes
	nop <$0C ;hides 'nop $E9A2'
	ldx #233
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3496 || n = 3511)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3499)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3502)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3504)
	; 9 bytes
	nop <$0C ;hides 'nop $DBA2'
	ldx #219
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3508)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3509)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3512)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3514)
	; 10 bytes
	nop <$0C ;hides 'nop $05A2'
	ldx #5
	nop
	nop $04
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516 || n = 3535)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3517 || n = 3518 || n = 3522 || n = 3523 || n = 3525 || n = 3528 || n = 3531 || n = 3533 || n = 3539 || n = 3545 || n = 3547)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3519)
	; 9 bytes
	nop #<$0C ;hides 'nop $DCA2'
	ldx #220
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3520 || n = 3537 || n = 3554)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3521 || n = 3541)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3524)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3526)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3527 || n = 3542 || n = 3544 || n = 3548)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3529 || n = 3543)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3530 || n = 3534)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3532 || n = 3540 || n = 3546)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3536)
	; 9 bytes
	nop <$0C ;hides 'nop $DDA2'
	ldx #221
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3538)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3549)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3550)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3551)
	; 9 bytes
	nop #<$0C ;hides 'nop $DEA2'
	ldx #222
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3552)
	; 9 bytes
	nop <$0C ;hides 'nop $DEA2'
	ldx #222
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3553)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.endif
.else
; 3555 to 3593
.if(n = 3555 || n = 3558 || n = 3559 || n = 3563 || n = 3575 || n = 3583 || n = 3587 || n = 3589 || n = 3590)
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
.elseif(n = 3560 || n = 3574 || n = 3576)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3561 || n = 3581)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3562 || n = 3593)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3564 || n = 3578 || n = 3580)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3565)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3566)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3567)
	; 9 bytes
	nop #<$0C ;hides 'nop $DFA2'
	ldx #223
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3568)
	; 9 bytes
	nop <$0C ;hides 'nop $DFA2'
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
	nop <$0C ;hides 'nop $01A2'
	ldx #1
	nop
	nop $04
	inx
	Jne *-7
.elseif(n = 3572)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	delay_n 13
	inx
	Jne *-7
.elseif(n = 3573 || n = 3592)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3577 || n = 3591)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3579)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
.elseif(n = 3582)
	; 10 bytes
	nop <$0C ;hides 'nop $C7A2'
	ldx #199
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3584)
	; 9 bytes
	nop <$0C ;hides 'nop $E0A2'
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
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3596 || n = 3603 || n = 3607 || n = 3618 || n = 3623 || n = 3624 || n = 3626 || n = 3632)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3597 || n = 3606)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3598 || n = 3627 || n = 3628)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3599)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 3600)
	; 9 bytes
	nop <$0C ;hides 'nop $E1A2'
	ldx #225
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3601 || n = 3621)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3602)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 6
	dex
	Jne *-7
.elseif(n = 3604 || n = 3608 || n = 3614 || n = 3625)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3605 || n = 3622)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3609)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3610)
	; 10 bytes
	nop #<$0C ;hides 'nop $9DA2'
	ldx #157
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 3611 || n = 3630)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3612)
	; 9 bytes
	ldx #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
.elseif(n = 3613)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3615)
	; 9 bytes
	ldx #<$8D ;hides 'sta $1EA2'
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
.elseif(n = 3619)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3620)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 8
	dex
	Jne *-7
.elseif(n = 3629)
	; 10 bytes
	ldx #<$8D ;hides 'sta $0EA2'
	ldx #14
	delay_n 6
	inx
	Jne *-7
.endif
.else
; 3633 to 3671
.if(n = 3633 || n = 3665)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3634 || n = 3655)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3635 || n = 3636 || n = 3643 || n = 3650 || n = 3658 || n = 3660 || n = 3663 || n = 3667 || n = 3670)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3637)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3638)
	; 10 bytes
	ldx #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 3639 || n = 3656)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3640 || n = 3669)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3641 || n = 3661)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3642 || n = 3652)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3644)
	; 9 bytes
	php
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3645 || n = 3662)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3646)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3647)
	; 9 bytes
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3648)
	; 9 bytes
	nop <$0C ;hides 'nop $E4A2'
	ldx #228
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3649 || n = 3668)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3651)
	; 8 bytes
	ldx #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 3653)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
.elseif(n = 3654)
	; 10 bytes
	nop <$0C ;hides 'nop $CBA2'
	ldx #203
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3657)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	nop
	dex
	Jne *-6
.elseif(n = 3659)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3664)
	; 9 bytes
	nop <$0C ;hides 'nop $E5A2'
	ldx #229
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3666)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
.elseif(n = 3671)
	; 9 bytes
	ldx #216 ;hides 'cld'
	rol $26
	ror $26
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672 || n = 3675 || n = 3677 || n = 3678 || n = 3683 || n = 3685 || n = 3689 || (n > 3691 && n < 3695) || n = 3699 || n = 3703 || n = 3708 || n = 3709)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3673 || n = 3690 || n = 3707)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3674)
	; 9 bytes
	ldx #(n-14)/15 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-6
.elseif(n = 3676 || n = 3691)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3679)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3680)
	; 9 bytes
	nop <$0C ;hides 'nop $E6A2'
	ldx #230
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3681 || n = 3701)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3682)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3684 || n = 3698 || n = 3704 || n = 3710)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3686 || n = 3688 || n = 3702)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3687 || n = 3706)
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
.elseif(n = 3696)
	; 9 bytes
	nop <$0C ;hides 'nop $19A2'
	ldx #25
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3697)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3700)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 3705)
	; 9 bytes
	ldx #218 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
.endif
.else
; 3711 to 3750
.if(n = 3711)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3712)
	; 9 bytes
	nop <$0C ;hides 'nop $18A2'
	ldx #24
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3713 || n = 3729 || n = 3745)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3714)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3715 || n = 3720 || n = 3723 || n = 3726 || n = 3727 || n = 3731 || n = 3738 || n = 3742 || n = 3746 || n = 3747)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3716)
	; 10 bytes
	nop #<$0C ;hides 'nop $B1A2'
	ldx #177
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3717 || n = 3730)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3718 || n = 3739)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3719)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3722 || n = 3733)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3724 || n = 3741)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3725 || n = 3744)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3728)
	; 9 bytes
	nop <$0C ;hides 'nop $E9A2'
	ldx #233
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3732 || n = 3748 || n = 3750)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3734)
	; 10 bytes
	nop #<$0C ;hides 'nop $07A2'
	ldx #7
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3735)
	; 10 bytes
	nop <$0C ;hides 'nop $07A2'
	ldx #7
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3736)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3737)
	; 10 bytes
	nop #<$0C ;hides 'nop $B2A2'
	ldx #178
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3740)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 3743)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3749)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 8
	dex
	Jne *-6
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
; 3751 to 3789
.if(n = 3751 || n = 3766)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3752 || n = 3755 || n = 3756)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3753 || n = 3760 || n = 3762 || n = 3765 || n = 3768 || n = 3772 || n = 3779 || n = 3783 || n = 3784 || n = 3786)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3754)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 3757)
	; 10 bytes
	pha
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
	pla
.elseif(n = 3758 || n = 3775)
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
.elseif(n = 3761 || n = 3781)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3763 || n = 3782)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3764)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 3770)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3771)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3773)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3774)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	nop $04
	inx
	Jne *-7
.elseif(n = 3776)
	; 9 bytes
	nop <$0C ;hides 'nop $ECA2'
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
.elseif(n = 3778)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3780)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 3785)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
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
.elseif(n = 3788 || n = 3789)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 3790 to 3828
.if(n = 3790)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3791)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3792 || n = 3809 || n = 3826)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3793 || n = 3825)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3794 || n = 3795 || n = 3798 || n = 3813 || n = 3822 || n = 3823 || n = 3827 || n = 3828)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3796 || n = 3811)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3797)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3799)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3800)
	; 10 bytes
	nop #<$0C ;hides 'nop $B5A2'
	ldx #181
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3801 || n = 3821)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3802)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3803)
	; 9 bytes
	ldx #200
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	nop
.elseif(n = 3804 || n = 3812)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3805)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3806 || n = 3816)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3807)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3808)
	; 9 bytes
	nop <$0C ;hides 'nop $EEA2'
	ldx #238
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3810)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
.elseif(n = 3814)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3815)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 3817)
	; 9 bytes
	ldx #90 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 3818)
	; 10 bytes
	nop <$0C ;hides 'nop $A6A2'
	ldx #166
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 3819)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3820)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3824)
	; 9 bytes
	nop <$0C ;hides 'nop $11A2'
	ldx #17
	delay_n 7
	inx
	Jne *-6
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3851)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3830 || n = 3831 || n = 3845 || n = 3847 || n = 3862 || n = 3864)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3832 || n = 3849 || n = 3866)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3833)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 9
	dex
	Jne *-6
.elseif(n = 3834)
	; 10 bytes
	nop <$0C ;hides 'nop $D5A2'
	ldx #213
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3835 || n = 3853)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3836 || n = 3837 || n = 3854)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3838)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dex
	Jne *-7
.elseif(n = 3839 || n = 3858)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3840)
	; 9 bytes
	nop <$0C ;hides 'nop $10A2'
	ldx #16
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3841 || n = 3861)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3842 || n = 3846 || n = 3859)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3843 || n = 3860)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3844 || n = 3865)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3848 || n = 3850 || n = 3863 || n = 3867)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3852)
	; 10 bytes
	nop <$0C ;hides 'nop $D6A2'
	ldx #214
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3855)
	; 9 bytes
	nop #<$0C ;hides 'nop $0FA2'
	ldx #15
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3856)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
.elseif(n = 3857)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3884)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3869)
	; 10 bytes
	nop #<$0C ;hides 'nop $D7A2'
	ldx #215
	delay_n 9
	dex
	Jne *-7
.elseif(n = 3870)
	; 10 bytes
	php
	ldx #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	plp
.elseif(n = 3871)
	; 9 bytes
	ldx #<$8D ;hides 'sta $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3872)
	; 9 bytes
	nop <$0C ;hides 'nop $0EA2'
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
.elseif(n = 3874 || n = 3875 || n = 3879 || n = 3883 || n = 3885 || n = 3891 || n = 3900 || n = 3906)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3876 || n = 3878 || n = 3882 || n = 3892)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3877 || n = 3894)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3880 || n = 3897 || n = 3899)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3881 || n = 3901)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3886)
	; 9 bytes
	ldx #(n-1)/21
	nop
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
.elseif(n = 3888)
	; 9 bytes
	nop <$0C ;hides 'nop $F3A2'
	ldx #243
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3890)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3893 || n = 3902)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3895)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3896)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3898)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3903)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3904)
	; 9 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	delay_n 7
	inx
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
; 3907 to 3945
.if(n = 3907)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3908 || n = 3910 || n = 3918 || n = 3922)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3909)
	; 9 bytes
	ldx #26 ;hides 'nop'
	rol $26
	ror $26
	inx
	Jne *-6
.elseif(n = 3911 || n = 3928 || n = 3945)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3912)
	; 10 bytes
	nop <$0C ;hides 'nop $A3A2'
	ldx #163
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 3913 || n = 3923 || n = 3930 || n = 3936 || n = 3938)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3914 || n = 3924 || n = 3931 || n = 3944)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3915 || n = 3934)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3916)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 3917 || n = 3939)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3919)
	; 8 bytes
	ldx #(n-15)/16 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3920)
	; 9 bytes
	nop <$0C ;hides 'nop $0BA2'
	ldx #11
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3921 || n = 3941)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3925)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
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
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3929)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3932)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 3933)
	; 10 bytes
	nop <$0C ;hides 'nop $ABA2'
	ldx #171
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 3935)
	; 9 bytes
	nop #<$0C ;hides 'nop $F6A2'
	ldx #246
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3937)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3940)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3942)
	; 10 bytes
	nop <$0C ;hides 'nop $DBA2'
	ldx #219
	delay_n 9
	dex
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
.if(n = 3946 || n = 3958)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3947 || n = 3954 || n = 3955 || n = 3957 || n = 3963 || n = 3964 || n = 3966 || n = 3971 || n = 3974 || n = 3976)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3948 || n = 3956 || n = 3965 || n = 3975 || n = 3982)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3949 || n = 3970)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 3950)
	; 10 bytes
	ldx #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	inx
	Jne *-7
.elseif(n = 3951)
	; 9 bytes
	nop #<$0C ;hides 'nop $09A2'
	ldx #9
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3952)
	; 9 bytes
	nop <$0C ;hides 'nop $F7A2'
	ldx #247
	delay_n 7
	dex
	Jne *-6
.elseif(n = 3953 || n = 3972)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3959 || n = 3973)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3960)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3961 || n = 3981)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 3962 || n = 3979)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3967)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3968)
	; 9 bytes
	nop <$0C ;hides 'nop $08A2'
	ldx #8
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3969)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3977)
	; 9 bytes
	ldx #234 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 3978)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 3980)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 3983)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 3984)
	; 9 bytes
	nop <$0C ;hides 'nop $07A2'
	ldx #7
	delay_n 7
	inx
	Jne *-6
.endif
.endif
.else
; 3985 to 4062
.if(n = 3985 || n = 4017 || n = 4033 || n = 4049)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3986 || n = 3988 || n = 4006 || n = 4008 || n = 4020 || n = 4028 || n = 4037 || n = 4055)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3987 || n = 3993 || n = 3995 || n = 3998 || n = 4003 || n = 4007 || n = 4015 || n = 4018 || n = 4019 || n = 4023 || n = 4025 || n = 4031 || n = 4032 || n = 4034 || n = 4035 || n = 4043 || n = 4045 || n = 4050 || n = 4052 || n = 4056)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3989)
	; 10 bytes
	nop #<$0C ;hides 'nop $BEA2'
	ldx #190
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3990)
	; 10 bytes
	nop <$0C ;hides 'nop $BEA2'
	ldx #190
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 3992 || (n > 4037 && n < 4041) || n = 4042 || n = 4058 || n = 4060)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3994 || n = 4004 || n = 4024 || n = 4044 || n = 4062)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3997 || n = 4051)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3999)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 4000)
	; 9 bytes
	nop <$0C ;hides 'nop $06A2'
	ldx #6
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4001 || n = 4021 || n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4002 || n = 4036 || n = 4053)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4005 || n = 4027)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4009 || n = 4057)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4011)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	.repeat 5
	 nop
	.endrepeat
	inx
	Jne *-7
.elseif(n = 4012 || n = 4054)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4014)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4016)
	; 9 bytes
	nop <$0C ;hides 'nop $05A2'
	ldx #5
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4022)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dex
	Jne *-7
.elseif(n = 4026)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4046)
	; 10 bytes
	ldx #(n-18)/19 ;hides 'nop zp,x'
	inx ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4059)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
; 4063 to 4101
.if(n = 4063 || n = 4066 || n = 4068 || n = 4069 || n = 4073 || n = 4077 || n = 4082 || n = 4083 || n = 4085 || n = 4088 || n = 4090 || n = 4099 || n = 4100)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4064 || n = 4081 || n = 4098)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4065)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4067 || n = 4086)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4070 || n = 4092)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4071 || n = 4093)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4072)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4074)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4075 || n = 4096)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4076 || n = 4091)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4078)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4079)
	; 9 bytes
	nop #<$0C ;hides 'nop $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4080)
	; 9 bytes
	nop <$0C ;hides 'nop $01A2'
	ldx #1
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4084 || n = 4089)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4087)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4094)
	; 10 bytes
	nop #<$0C ;hides 'nop $C3A2'
	ldx #195
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4095)
	; 9 bytes
	nop #<$0C ;hides 'nop $00A2'
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
.elseif(n = 4101)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.endif
.else
; 4102 to 4140
.if(n = 4102 || n = 4107 || n = 4117 || n = 4119 || n = 4123 || n = 4125 || n = 4126 || n = 4134 || n = 4140)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4103)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4104 || n = 4109 || n = 4111 || n = 4130)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4105 || n = 4124)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4106 || n = 4118 || n = 4135)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4108 || n = 4110 || n = 4112 || n = 4122 || n = 4128 || n = 4131)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4113 || n = 4120 || n = 4133)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4114)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 4115 || n = 4132)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4116)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	inx
	inx
	Jne *-6
.elseif(n = 4121)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4127)
	; 9 bytes
	ldx #84 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.elseif(n = 4129)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4136)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 4137)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4138)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4139)
	; 9 bytes
	ldx #26 ;hides 'nop'
	delay_n 11
	inx
	Jne *-6
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4161)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4142 || n = 4154 || n = 4171 || n = 4174)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4143 || n = 4162)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4144 || n = 4146 || n = 4152 || n = 4165 || n = 4169)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4145 || n = 4147 || n = 4151 || n = 4153 || n = 4160 || n = 4163 || n = 4164 || n = 4168 || n = 4170)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4148 || n = 4150 || n = 4156 || n = 4173)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4149 || n = 4166)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4155 || n = 4167 || n = 4172)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4157)
	; 8 bytes
	ldx #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4158)
	; 9 bytes
	php
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4159)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4175)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 4176)
	; 10 bytes
	nop <$0C ;hides 'nop $18A2'
	ldx #24
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4177)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4178)
	; 10 bytes
	nop #<$0C ;hides 'nop $C7A2'
	ldx #199
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4179)
	; 10 bytes
	nop <$0C ;hides 'nop $C7A2'
	ldx #199
	jsr @nearby_rts
	dex
	Jne *-7
.endif
.else
; 4180 to 4218
.if(n = 4180)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4181)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4182 || n = 4185 || n = 4187 || n = 4202 || n = 4204)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4183 || n = 4200 || n = 4217)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4184 || n = 4186)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4188 || n = 4190 || n = 4207 || n = 4208)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4189 || n = 4206)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4191 || n = 4192 || (n > 4195 && n < 4199) || n = 4214 || n = 4216 || n = 4218)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4193)
	; 10 bytes
	nop #<$0C ;hides 'nop $E9A2'
	ldx #233
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4194)
	; 10 bytes
	nop <$0C ;hides 'nop $E9A2'
	ldx #233
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4195 || n = 4213)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4201)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4203)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4205)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4209)
	; 10 bytes
	nop <$0C ;hides 'nop $B7A2'
	ldx #183
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4210)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4211)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4212)
	; 10 bytes
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4215)
	; 9 bytes
	ldx #248 ;hides 'sed'
	rol $26
	ror $26
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4220 || n = 4237 || n = 4244 || n = 4254 || n = 4260 || n = 4271 || n = 4279 || n = 4284 || n = 4288)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4221 || n = 4241 || n = 4261 || n = 4281)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4222 || n = 4243 || n = 4264)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4223 || n = 4227 || n = 4236 || n = 4240 || n = 4253 || n = 4255 || n = 4259 || n = 4263 || n = 4266 || n = 4270 || n = 4272 || n = 4278 || n = 4280 || n = 4283 || n = 4287 || n = 4289 || n = 4293)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4224 || n = 4226 || n = 4228 || n = 4245 || n = 4248 || n = 4258 || n = 4275 || n = 4292)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4225 || n = 4247 || n = 4269 || n = 4291)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4229)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 4230)
	; 10 bytes
	nop <$0C ;hides 'nop $EBA2'
	ldx #235
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4231 || n = 4267)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4232)
	; 10 bytes
	nop <$0C ;hides 'nop $B8A2'
	ldx #184
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4233 || n = 4256)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4234 || n = 4251 || n = 4268 || n = 4285)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4235 || n = 4250 || n = 4252 || n = 4286 || n = 4290)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4239)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4242)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	inx
	Jne *-6
.elseif(n = 4246)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4249)
	; 9 bytes
	ldx #250 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 4262)
	; 8 bytes
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
.elseif(n = 4265)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 9
	inx
	Jne *-6
.elseif(n = 4273)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4274 || n = 4282)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4277)
	; 10 bytes
	nop #<$0C ;hides 'nop $BAA2'
	ldx #186
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4294)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4296)
	; 10 bytes
	nop <$0C ;hides 'nop $B3A2'
	ldx #179
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.endif
.else
; 4297 to 4375
.if(n = 4297 || n = 4299 || n = 4303 || n = 4304 || n = 4315 || n = 4316 || n = 4318 || n = 4323 || n = 4329 || n = 4335 || n = 4337 || n = 4338 || n = 4340 || n = 4343 || n = 4349 || n = 4350 || n = 4354 || n = 4363 || n = 4365 || n = 4373)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4298 || n = 4305 || n = 4317 || n = 4322 || n = 4344 || n = 4356 || n = 4364 || n = 4374)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4300)
	; 10 bytes
	nop #<$0C ;hides 'nop $BBA2'
	ldx #187
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4301 || n = 4321 || n = 4341 || n = 4361)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4302 || n = 4319 || n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4306 || n = 4327 || n = 4348)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4307 || n = 4308 || n = 4320 || n = 4325 || n = 4342 || n = 4358 || n = 4359 || n = 4367)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4309 || n = 4326 || n = 4328 || n = 4360 || n = 4362 || n = 4368)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 4309 && n < 4313) || (n > 4329 && n < 4333) || n = 4334 || n = 4351 || n = 4366)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4313 || n = 4357)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4324)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.elseif(n = 4339 || n = 4375)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4345)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4346)
	; 10 bytes
	nop #<$0C ;hides 'nop $CFA2'
	ldx #207
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4347)
	; 9 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
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
.elseif(n = 4370)
	; 10 bytes
	nop <$0C ;hides 'nop $BEA2'
	ldx #190
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4372)
	; 10 bytes
	delay_n (n-3)
	nop $04
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
.if(n = 4376 || n = 4378 || n = 4397 || n = 4406 || n = 4408 || n = 4414)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4377 || n = 4387 || n = 4396 || n = 4413)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4379)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4380 || n = 4382 || n = 4398 || n = 4400)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4381 || n = 4401)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4383 || n = 4385 || n = 4392 || n = 4394 || n = 4403 || n = 4405)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4384 || n = 4404 || n = 4410 || n = 4412)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4386)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dex
	Jne *-6
.elseif(n = 4388)
	; 10 bytes
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
.elseif(n = 4389)
	; 10 bytes
	nop <$0C ;hides 'nop $D1A2'
	ldx #209
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4390 || n = 4409)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4391)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4393)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4395)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 4402)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4407)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4411)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.endif
.else
; 4415 to 4453
.if(n = 4415)
	; 9 bytes
	php
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4416 || n = 4435 || n = 4452)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4417 || n = 4430 || n = 4434 || n = 4443 || n = 4449)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n > 4417 && n < 4421) || n = 4422 || n = 4426 || n = 4433 || n = 4436 || n = 4437 || n = 4442)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4421 || n = 4441)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4423)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4424 || n = 4431 || n = 4444 || n = 4448 || n = 4450)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4425)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4427)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4428 || n = 4447)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4429)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4432 || n = 4453)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4438)
	; 10 bytes
	nop #<$0C ;hides 'nop $C1A2'
	ldx #193
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4439)
	; 10 bytes
	nop <$0C ;hides 'nop $C1A2'
	ldx #193
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4440)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4445)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4446)
	; 10 bytes
	nop <$0C ;hides 'nop $F7A2'
	ldx #247
	delay_n 9
	dex
	Jne *-7
.elseif(n = 4451)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454 || n = 4473 || n = 4488 || n = 4492)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4455 || n = 4468 || n = 4470 || n = 4476 || n = 4483 || n = 4487 || n = 4491)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4456)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif((n > 4456 && n < 4461) || n = 4475 || n = 4477 || n = 4479 || n = 4480 || n = 4490)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4461 || n = 4481)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4462)
	; 10 bytes
	ldx #n/23 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4463)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4464 || n = 4469 || n = 4484)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4465)
	; 8 bytes
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
.elseif(n = 4466 || n = 4485)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4467 || n = 4489)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4471)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4472)
	; 9 bytes
	ldx #(n-20)/21 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4474)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4478)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	inx
	Jne *-7
.elseif(n = 4482)
	; 10 bytes
	nop <$0C ;hides 'nop $07A2'
	ldx #7
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4486)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.endif
.else
; 4493 to 4531
.if(n = 4493)
	; 10 bytes
	nop #<$0C ;hides 'nop $D6A2'
	ldx #214
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4494)
	; 10 bytes
	nop <$0C ;hides 'nop $D6A2'
	ldx #214
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4495 || n = 4516)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4496 || n = 4498 || n = 4520 || n = 4522)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4497 || n = 4503 || n = 4505 || n = 4506 || n = 4518 || n = 4525 || n = 4527)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4499)
	; 9 bytes
	ldx #250 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4500)
	; 10 bytes
	nop <$0C ;hides 'nop $06A2'
	ldx #6
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4501 || n = 4521)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4502)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	rol $26
	ror $26
	inx
	Jne *-7
.elseif(n = 4504 || n = 4523)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4507 || n = 4524 || n = 4526)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4508 || n = 4528 || n = 4530)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4509)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4510)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4511)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4512)
	; 10 bytes
	ldx #68 ;hides 'nop zp'
	delay_n 4
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 4513)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4514)
	; 10 bytes
	nop #<$0C ;hides 'nop $D7A2'
	ldx #215
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4515)
	; 10 bytes
	nop <$0C ;hides 'nop $D7A2'
	ldx #215
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4517)
	; 10 bytes
	nop #<$0C ;hides 'nop $05A2'
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
.elseif(n = 4529)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4531)
	; 10 bytes
	nop <$0C ;hides 'nop $C5A2'
	ldx #197
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if(n = 4532)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4533 || n = 4555)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4534 || n = 4540 || n = 4550 || n = 4552 || n = 4562 || n = 4566 || n = 4569 || n = 4570)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4535)
	; 8 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4536)
	; 10 bytes
	nop <$0C ;hides 'nop $04A2'
	ldx #4
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4537 || n = 4558)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4538 || n = 4545 || n = 4564)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4539 || n = 4543 || n = 4544 || n = 4546 || n = 4557 || n = 4560 || n = 4565)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4541)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4542 || n = 4561)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4547 || n = 4567)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4548 || n = 4549 || n = 4568)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4551)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4553)
	; 9 bytes
	ldx #58 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 4554)
	; 10 bytes
	nop <$0C ;hides 'nop $03A2'
	ldx #3
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4556)
	; 10 bytes
	nop #<$0C ;hides 'nop $D9A2'
	ldx #217
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4559)
	; 10 bytes
	nop #<$0C ;hides 'nop $BEA2'
	ldx #190
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 4563)
	; 9 bytes
	ldx #228
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	nop
.endif
.else
; 4571 to 4609
.if(n = 4571)
	; 10 bytes
	nop #<$0C ;hides 'nop $02A2'
	ldx #2
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4572)
	; 10 bytes
	nop <$0C ;hides 'nop $02A2'
	ldx #2
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4573 || n = 4591)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4574 || n = 4575 || (n > 4591 && n < 4598))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4576)
	; 10 bytes
	nop #<$0C ;hides 'nop $C7A2'
	ldx #199
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4577)
	; 8 bytes
	ldx #218 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4578)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4579 || n = 4600)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4580 || n = 4599)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4581 || n = 4601)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif((n > 4581 && n < 4585) || n = 4603 || n = 4605)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4585)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4586 || n = 4607)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4587 || n = 4588 || n = 4606 || n = 4608)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4589)
	; 10 bytes
	nop #<$0C ;hides 'nop $01A2'
	ldx #1
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4590)
	; 10 bytes
	nop <$0C ;hides 'nop $01A2'
	ldx #1
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4598)
	; 10 bytes
	nop #<$0C ;hides 'nop $DBA2'
	ldx #219
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4602 || n = 4604)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4609)
	; 10 bytes
	ldx #0
	delay_n 13
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if((n > 4609 && n < 4616) || n = 4617 || n = 4626 || n = 4636 || n = 4638)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4616)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4618 || n = 4637)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4619)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
.elseif(n = 4620 || n = 4622 || n = 4623 || n = 4639)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4621 || n = 4641)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4624 || n = 4640)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4625 || n = 4628 || n = 4644 || n = 4648)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4627)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4629)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4630)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dex
	Jne *-7
.elseif(n = 4631)
	; 10 bytes
	nop #<$0C ;hides 'nop $C1A2'
	ldx #193
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 4632)
	; 10 bytes
	nop <$0C ;hides 'nop $C1A2'
	ldx #193
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 4633)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4634)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	delay_n 7
	dex
	Jne *-7
.elseif(n = 4635)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4642)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4643)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4645)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4646)
	; 10 bytes
	nop <$0C ;hides 'nop $CAA2'
	ldx #202
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4647)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.endif
.else
; 4649 to 4687
.if((n > 4648 && n < 4653) || n = 4666 || n = 4671 || n = 4673 || n = 4676)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4653)
	; 10 bytes
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	inx
	inx
	Jne *-7
.elseif(n = 4654)
	; 10 bytes
	ldx #(n-18)/19 ;hides 'nop zp,x'
	inx ;first loop only
	rol $26
	ror $26
	dex
	Jne *-7
.elseif(n = 4655)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4656 || n = 4675)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4657)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4658 || n = 4660 || n = 4667 || n = 4674 || n = 4677 || n = 4683 || n = 4685 || n = 4686)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4659 || n = 4664 || n = 4678)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4661 || n = 4681)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4662)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4663 || n = 4684)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4665 || n = 4687)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4668)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
.elseif(n = 4669)
	; 10 bytes
	nop <$0C ;hides 'nop $CBA2'
	ldx #203
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4670)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4672)
	; 9 bytes
	php
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4679)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4680)
	; 10 bytes
	nop <$0C ;hides 'nop $C3A2'
	ldx #195
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 4682)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if(n = 4688 || n = 4708 || n = 4718 || n = 4720)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4689 || n = 4696 || n = 4698 || n = 4703 || n = 4715 || n = 4723 || n = 4725)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4690)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4691)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 4692)
	; 10 bytes
	nop <$0C ;hides 'nop $CCA2'
	ldx #204
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4693)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4694 || n = 4713)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4695 || n = 4702 || n = 4706 || n = 4710 || n = 4712 || n = 4722)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4697 || n = 4704 || n = 4716 || n = 4724)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4699)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4700 || n = 4717 || n = 4719)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4701 || n = 4721)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4705 || n = 4726)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4707)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4709)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4711)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4714)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.endif
.else
; 4727 to 4765
.if(n = 4727 || n = 4738 || n = 4757)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4728 || n = 4739 || n = 4748 || n = 4756 || n = 4758)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4729)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4730 || n = 4740 || n = 4750)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4731)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4732 || n = 4751)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4733 || n = 4734 || n = 4736 || n = 4743 || n = 4753 || n = 4763 || n = 4765)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4735 || n = 4744 || n = 4754 || n = 4764)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4737)
	; 10 bytes
	nop #<$0C ;hides 'nop $CEA2'
	ldx #206
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4741 || n = 4761)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4742)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dex
	Jne *-7
.elseif(n = 4745)
	; 10 bytes
	ldx #<$8D ;hides 'sta $1EA2'
	ldx #30
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4746)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4747)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4752)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 4755)
	; 10 bytes
	ldx #216
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
	nop
.elseif(n = 4759)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4760)
	; 10 bytes
	nop #<$0C ;hides 'nop $CFA2'
	ldx #207
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4762)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.endif
.endif
.else
; 4766 to 4843
.if(n = 4766)
	; 10 bytes
	nop #<$0C ;hides 'nop $E3A2'
	ldx #227
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4767 || n = 4776 || n = 4787 || n = 4814 || n = 4835)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4768 || n = 4810 || n = 4831)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4769 || n = 4771 || n = 4778 || n = 4779 || n = 4782 || n = 4786 || n = 4790 || n = 4794 || (n > 4815 && n < 4819) || n = 4820 || n = 4822 || n = 4826 || (n > 4837 && n < 4841) || n = 4842)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4770 || n = 4789 || n = 4808 || n = 4827)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4772 || n = 4774 || n = 4783 || n = 4785 || n = 4791 || n = 4793 || n = 4803 || n = 4805 || n = 4823 || n = 4833 || n = 4843)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4773 || n = 4784 || n = 4792 || n = 4804 || n = 4811 || n = 4824 || n = 4830)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4775 || n = 4797 || n = 4819)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4777 || n = 4825)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4780 || n = 4807)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4781 || n = 4801 || n = 4821 || n = 4841)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4788 || n = 4796 || n = 4802 || n = 4815 || n = 4828 || n = 4834 || n = 4836)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4795)
	; 8 bytes
	ldx #218 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4798 || n = 4832)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 4799)
	; 9 bytes
	ldx #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.elseif(n = 4800)
	; 10 bytes
	nop <$0C ;hides 'nop $C8A2'
	ldx #200
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 4806)
	; 10 bytes
	nop #<$0C ;hides 'nop $D1A2'
	ldx #209
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4809)
	; 10 bytes
	nop <$0C ;hides 'nop $E5A2'
	ldx #229
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4812)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4813)
	; 10 bytes
	ldx #90 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 4829)
	; 8 bytes
	ldx #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4837)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4864 || n = 4874)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4845 || n = 4848 || n = 4867 || n = 4869)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4846)
	; 8 bytes
	ldx #(n-1)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4847)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4849)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4850)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
.elseif(n = 4851)
	; 10 bytes
	nop <$0C ;hides 'nop $19A2'
	ldx #25
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4852 || n = 4873)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4853 || n = 4868 || n = 4872 || n = 4878)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4854 || n = 4877)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif((n > 4854 && n < 4859) || n = 4860 || n = 4862 || n = 4866 || n = 4880 || n = 4882)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4859)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 11
	dex
	Jne *-7
.elseif(n = 4861 || n = 4881)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4863)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4865)
	; 8 bytes
	ldx #0
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
.elseif(n = 4870)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dex
	Jne *-7
.elseif(n = 4871)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	inx
	Jne *-5
.elseif(n = 4875)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4876)
	; 10 bytes
	nop <$0C ;hides 'nop $D4A2'
	ldx #212
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4879)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 13
	dex
	Jne *-7
.endif
.else
; 4883 to 4921
.if(n = 4883 || n = 4887 || n = 4896 || n = 4903 || n = 4905 || n = 4909 || n = 4917)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4884 || n = 4904 || n = 4916)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4885 || n = 4907)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4886 || n = 4890 || n = 4891 || n = 4902 || n = 4906 || (n > 4909 && n < 4913) || n = 4918)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4888 || n = 4908 || n = 4920)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4889)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4892)
	; 10 bytes
	nop #<$0C ;hides 'nop $E9A2'
	ldx #233
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4893)
	; 10 bytes
	nop <$0C ;hides 'nop $E9A2'
	ldx #233
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4894 || n = 4915)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4895)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.elseif(n = 4897)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4898)
	; 9 bytes
	ldx #(n-22)/23 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4899)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 11
	dex
	Jne *-7
.elseif(n = 4900)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4901 || n = 4921)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4913)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4914)
	; 10 bytes
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4919)
	; 9 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.endif
.endif
.else
; 4922 to 5000
.if(n = 4922)
	; 10 bytes
	nop <$0C ;hides 'nop $D6A2'
	ldx #214
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4923 || n = 4925 || n = 4931 || n = 4938 || n = 4943 || n = 4953 || n = 4963 || n = 4965 || n = 4975 || n = 4980 || n = 4983 || n = 4985 || n = 4997)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4924 || n = 4944 || n = 4964 || n = 4984)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4926 || n = 4930 || n = 4932 || n = 4933 || n = 4937 || n = 4939 || n = 4940 || n = 4950 || n = 4952 || n = 4954 || n = 4958 || n = 4960 || n = 4962 || n = 4966 || (n > 4969 && n < 4973) || n = 4974 || n = 4979 || n = 4982 || n = 4986 || n = 4989 || n = 4994 || n = 5000)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4927 || n = 4947 || n = 4987)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4928 || n = 4948 || n = 4968 || n = 4988)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4929)
	; 9 bytes
	php
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4934)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4935)
	; 10 bytes
	nop <$0C ;hides 'nop $EBA2'
	ldx #235
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4936 || n = 4957 || n = 4978 || n = 4999)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 4941 || n = 4961 || n = 4981)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 4942 || n = 4996)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 4945 || n = 4969 || n = 4993)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 4946 || n = 4992)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4949)
	; 10 bytes
	ldx #58 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 4951 || n = 4973 || n = 4995)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 4955)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4956)
	; 10 bytes
	nop <$0C ;hides 'nop $ECA2'
	ldx #236
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 4959)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4967)
	; 9 bytes
	ldx #216 ;hides 'cld'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 4976)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts
	inx
	Jne *-6
.elseif(n = 4977)
	; 10 bytes
	nop <$0C ;hides 'nop $13A2'
	ldx #19
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 4990)
	; 10 bytes
	nop #<$0C ;hides 'nop $D9A2'
	ldx #217
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 4991)
	; 10 bytes
	nop #<$0C ;hides 'nop $D0A2'
	ldx #208
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 4998)
	; 10 bytes
	nop <$0C ;hides 'nop $EEA2'
	ldx #238
	jsr @nearby_rts
	dex
	Jne *-7
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
; 5001 to 5078
.if(n = 5001 || n = 5021 || n = 5041 || n = 5061)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 5002 || n = 5006 || (n > 5008 && n < 5013) || n = 5026 || (n > 5028 && n < 5036) || n = 5042 || n = 5046 || n = 5049 || (n > 5050 && n < 5059) || n = 5069 || n = 5070 || n = 5072 || n = 5073 || n = 5075 || n = 5076 || n = 5078)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5003 || n = 5005 || n = 5022 || n = 5023 || n = 5025 || n = 5043 || n = 5063 || n = 5064)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5004 || n = 5024 || n = 5044)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5007 || n = 5027 || n = 5047 || n = 5067)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 5008 || n = 5028 || n = 5048 || n = 5066 || n = 5068)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5013)
	; 9 bytes
	ldx #218 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5014)
	; 10 bytes
	nop <$0C ;hides 'nop $DAA2'
	ldx #218
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5015 || n = 5038)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5016)
	; 10 bytes
	nop <$0C ;hides 'nop $D1A2'
	ldx #209
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5017 || n = 5039)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5018)
	; 10 bytes
	nop #<$0C ;hides 'nop $11A2'
	ldx #17
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5019 || n = 5045 || n = 5071)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5020 || n = 5062)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5036)
	; 10 bytes
	nop #<$0C ;hides 'nop $DBA2'
	ldx #219
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5037)
	; 10 bytes
	nop <$0C ;hides 'nop $DBA2'
	ldx #219
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5040)
	; 10 bytes
	nop <$0C ;hides 'nop $10A2'
	ldx #16
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5050 || n = 5077)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5059)
	; 8 bytes
	ldx #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 5060)
	; 10 bytes
	nop #<$0C ;hides 'nop $0FA2'
	ldx #15
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5065)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5074)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	jsr @nearby_rts_14cyc
	Jne *-7
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5080 || n = 5086 || (n > 5089 && n < 5097) || n = 5100 || (n > 5113 && n < 5118))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5081 || n = 5101)
	; 8 bytes
	ldx #(n-1)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 5082)
	; 10 bytes
	nop <$0C ;hides 'nop $0EA2'
	ldx #14
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5083 || n = 5104)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5084)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5085)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5087)
	; 9 bytes
	ldx #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5088 || n = 5108 || n = 5110)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5089 || n = 5113)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5097)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5098)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5099)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5102)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5103)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	inx
	Jne *-5
.elseif(n = 5105)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5106)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5107)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5109)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 5111)
	; 9 bytes
	ldx #(n-23)/24 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5112)
	; 10 bytes
	nop <$0C ;hides 'nop $D5A2'
	ldx #213
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.endif
.else
; 5118 to 5156
.if((n > 5117 && n < 5121) || n = 5122 || n = 5126 || (n > 5131 && n < 5135) || (n > 5137 && n < 5144) || n = 5155 || n = 5156)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5121)
	; 8 bytes
	ldx #0
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
.elseif(n = 5123 || n = 5129 || n = 5148 || n = 5151)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5124 || n = 5150)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5125 || n = 5146)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5127 || n = 5149)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5128 || n = 5154)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5130 || n = 5153)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5131)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5135)
	; 10 bytes
	nop #<$0C ;hides 'nop $D6A2'
	ldx #214
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5136)
	; 10 bytes
	nop <$0C ;hides 'nop $D6A2'
	ldx #214
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5137)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5144)
	; 9 bytes
	ldx #(n-20)/21 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5145)
	; 10 bytes
	nop <$0C ;hides 'nop $0BA2'
	ldx #11
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5147)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5152)
	; 10 bytes
	nop <$0C ;hides 'nop $E0A2'
	ldx #224
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
; 5157 to 5195
.if(n = 5157 || (n > 5161 && n < 5165) || n = 5168 || n = 5170 || n = 5172 || (n > 5176 && n < 5183) || n = 5189 || n = 5191 || n = 5192 || n = 5194)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5158)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5159)
	; 10 bytes
	nop #<$0C ;hides 'nop $D7A2'
	ldx #215
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5160)
	; 10 bytes
	nop <$0C ;hides 'nop $D7A2'
	ldx #215
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5161 || n = 5185)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5165)
	; 10 bytes
	nop #<$0C ;hides 'nop $F6A2'
	ldx #246
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5166)
	; 10 bytes
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5167 || n = 5188)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5169 || n = 5173 || n = 5190 || n = 5195)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5171 || n = 5193)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5174)
	; 10 bytes
	nop #<$0C ;hides 'nop $1FA2'
	ldx #31
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5175)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5176)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5183)
	; 9 bytes
	ldx #216 ;hides 'cld'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5184)
	; 10 bytes
	nop <$0C ;hides 'nop $D8A2'
	ldx #216
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
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
	nop <$0C ;hides 'nop $F7A2'
	ldx #247
	jsr @nearby_rts
	dex
	Jne *-7
.endif
.else
; 5196 to 5234
.if(n = 5196 || n = 5200 || (n > 5201 && n < 5207) || n = 5216 || n = 5218 || n = 5219 || (n > 5222 && n < 5227) || n = 5234)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5197)
	; 10 bytes
	ldx #<$8D ;hides 'sta $1EA2'
	ldx #30
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5198)
	; 10 bytes
	ldx #n/23 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5199)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5201 || n = 5227)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5207)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5208)
	; 10 bytes
	nop <$0C ;hides 'nop $08A2'
	ldx #8
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5209 || n = 5230)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5210)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5211 || n = 5217 || n = 5232)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5212)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5213)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 5214)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5215)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5220)
	; 10 bytes
	nop #<$0C ;hides 'nop $E3A2'
	ldx #227
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5221)
	; 10 bytes
	nop <$0C ;hides 'nop $E3A2'
	ldx #227
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5222)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5228)
	; 10 bytes
	nop #<$0C ;hides 'nop $07A2'
	ldx #7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5229)
	; 10 bytes
	nop <$0C ;hides 'nop $07A2'
	ldx #7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5231)
	; 9 bytes
	ldx #218 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5233)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
; 5235 to 5273
.if(n = 5235 || n = 5236 || n = 5238 || (n > 5239 && n < 5243) || (n > 5245 && n < 5249) || n = 5254 || n = 5258 || n = 5260 || (n > 5261 && n < 5266) || n = 5269 || n = 5273)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5237 || n = 5259)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5239 || n = 5266)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5243)
	; 10 bytes
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5244)
	; 10 bytes
	nop <$0C ;hides 'nop $E4A2'
	ldx #228
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5245 || n = 5268)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5249)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5250)
	; 10 bytes
	nop <$0C ;hides 'nop $06A2'
	ldx #6
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5251 || n = 5272)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5252)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5253)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5255)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 5256)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5257)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5261)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5267)
	; 10 bytes
	nop <$0C ;hides 'nop $E5A2'
	ldx #229
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5270)
	; 10 bytes
	nop #<$0C ;hides 'nop $05A2'
	ldx #5
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5271)
	; 10 bytes
	nop <$0C ;hides 'nop $05A2'
	ldx #5
	jsr @nearby_rts
	inx
	Jne *-7
.endif
.else
; 5274 to 5312
.if(n = 5274 || n = 5283 || n = 5295)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5275 || n = 5276 || n = 5278 || n = 5282 || (n > 5283 && n < 5289) || n = 5294 || n = 5296 || n = 5297 || n = 5300 || n = 5301 || (n > 5305 && n < 5312))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5277)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
.elseif(n = 5279)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5280)
	; 10 bytes
	nop <$0C ;hides 'nop $DCA2'
	ldx #220
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5281 || n = 5303)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5289)
	; 9 bytes
	ldx #26 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 5290)
	; 10 bytes
	nop <$0C ;hides 'nop $E6A2'
	ldx #230
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5291)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5292)
	; 10 bytes
	nop <$0C ;hides 'nop $04A2'
	ldx #4
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5293)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5298)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5299)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5302)
	; 10 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5304)
	; 10 bytes
	nop <$0C ;hides 'nop $DDA2'
	ldx #221
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5305)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5312)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
; 5313 to 5390
.if(n = 5313)
	; 10 bytes
	nop <$0C ;hides 'nop $03A2'
	ldx #3
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5314 || n = 5335 || n = 5356)
	; 9 bytes
	ldx #(n-1)/21
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5315 || (n > 5316 && n < 5320) || (n > 5320 && n < 5324) || n = 5326 || n = 5330 || n = 5332 || (n > 5337 && n < 5347) || n = 5348 || n = 5350 || (n > 5360 && n < 5369) || n = 5370 || n = 5372 || n = 5373 || n = 5378 || n = 5380 || (n > 5383 && n < 5391))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5316 || n = 5327 || n = 5349 || n = 5358 || n = 5371 || n = 5379)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5320 || n = 5374)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5324)
	; 10 bytes
	ldx #(n-24)/25 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5325 || n = 5347 || n = 5369)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5328)
	; 10 bytes
	nop <$0C ;hides 'nop $DEA2'
	ldx #222
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5329 || n = 5353)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5331 || n = 5357)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5333)
	; 10 bytes
	nop #<$0C ;hides 'nop $02A2'
	ldx #2
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5334)
	; 10 bytes
	nop <$0C ;hides 'nop $02A2'
	ldx #2
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5336)
	; 10 bytes
	nop <$0C ;hides 'nop $18A2'
	ldx #24
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5337 || n = 5360 || n = 5383)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5351)
	; 10 bytes
	nop #<$0C ;hides 'nop $DFA2'
	ldx #223
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5352)
	; 10 bytes
	nop <$0C ;hides 'nop $DFA2'
	ldx #223
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5354)
	; 10 bytes
	nop #<$0C ;hides 'nop $01A2'
	ldx #1
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5355)
	; 10 bytes
	nop <$0C ;hides 'nop $01A2'
	ldx #1
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5359)
	; 10 bytes
	nop <$0C ;hides 'nop $E9A2'
	ldx #233
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5375)
	; 10 bytes
	nop #<$0C ;hides 'nop $00A2'
	ldx #0
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5376)
	; 10 bytes
	nop <$0C ;hides 'nop $00A2'
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5377)
	; 9 bytes
	ldx #0
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
.elseif(n = 5381)
	; 9 bytes
	ldx #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5382)
	; 10 bytes
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.endif
.else
; 5391 to 5468
.if(n = 5391 || n = 5413 || n = 5435 || n = 5457)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5392 || (n > 5393 && n < 5399) || n = 5402 || n = 5403 || n = 5407 || n = 5408 || (n > 5409 && n < 5413) || n = 5414 || (n > 5415 && n < 5422) || (n > 5429 && n < 5435) || n = 5436 || (n > 5437 && n < 5443) || n = 5444 || n = 5446 || n = 5453 || n = 5454 || n = 5456 || n = 5460 || (n > 5462 && n < 5469))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5393 || n = 5415 || n = 5437 || n = 5445 || n = 5459)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5399)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5400)
	; 10 bytes
	nop <$0C ;hides 'nop $E1A2'
	ldx #225
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5401 || n = 5425 || n = 5449)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5404)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 5405)
	; 10 bytes
	nop <$0C ;hides 'nop $EBA2'
	ldx #235
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5406 || n = 5429 || n = 5452)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5409 || n = 5461)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5422)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	inx
	Jne *-7
.elseif(n = 5423)
	; 10 bytes
	ldx #<$8D ;hides 'sta $1EA2'
	ldx #30
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5424)
	; 10 bytes
	ldx #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5426)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5427)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 5428)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5443)
	; 9 bytes
	php
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5447)
	; 10 bytes
	nop #<$0C ;hides 'nop $E3A2'
	ldx #227
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5448)
	; 10 bytes
	nop <$0C ;hides 'nop $E3A2'
	ldx #227
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5450)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	inx
	Jne *-6
.elseif(n = 5451)
	; 10 bytes
	nop <$0C ;hides 'nop $13A2'
	ldx #19
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5455)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5458)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5462)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
; 5469 to 5546
.if(n = 5469 || n = 5470 || (n > 5475 && n < 5479) || n = 5483 || n = 5484 || n = 5486 || (n > 5487 && n < 5495) || n = 5500 || n = 5504 || n = 5507 || n = 5508 || n = 5512 || (n > 5513 && n < 5519) || n = 5522 || n = 5524 || (n > 5525 && n < 5536) || n = 5538 || n = 5540 || n = 5541 || n = 5546)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5471)
	; 10 bytes
	nop #<$0C ;hides 'nop $1CA2'
	ldx #28
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5472)
	; 10 bytes
	nop <$0C ;hides 'nop $E4A2'
	ldx #228
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5473 || n = 5497 || n = 5521)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5474)
	; 10 bytes
	nop <$0C ;hides 'nop $EEA2'
	ldx #238
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5475 || n = 5498 || n = 5544)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5479 || n = 5501 || n = 5523 || n = 5545)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5480)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
.elseif(n = 5481 || n = 5503 || n = 5525)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5482 || n = 5509 || n = 5536)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5485)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5487 || n = 5513 || n = 5539)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5495)
	; 10 bytes
	nop #<$0C ;hides 'nop $E5A2'
	ldx #229
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5496)
	; 10 bytes
	nop #<$0C ;hides 'nop $11A2'
	ldx #17
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5499)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5502)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5505)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 5506)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5510)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5511)
	; 10 bytes
	ldx #212 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5519)
	; 9 bytes
	ldx #26 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.elseif(n = 5520)
	; 10 bytes
	nop <$0C ;hides 'nop $10A2'
	ldx #16
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5537)
	; 10 bytes
	ldx #(n-25)/26 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5542)
	; 10 bytes
	nop #<$0C ;hides 'nop $0FA2'
	ldx #15
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5543)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
.endif
.else
; 5547 to 5625
.if(n = 5547 || n = 5591)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 5547 && n < 5563) || n = 5564 || (n > 5569 && n < 5588) || (n > 5593 && n < 5599) || (n > 5599 && n < 5606) || (n > 5606 && n < 5610) || n = 5614 || (n > 5617 && n < 5626))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5563 || n = 5590)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5565)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5566)
	; 10 bytes
	nop <$0C ;hides 'nop $0EA2'
	ldx #14
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5567 || n = 5589 || n = 5611)
	; 9 bytes
	ldx #(n-1)/22
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5568)
	; 10 bytes
	nop <$0C ;hides 'nop $18A2'
	ldx #24
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5569 || n = 5593 || n = 5617)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5588)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5592)
	; 10 bytes
	nop <$0C ;hides 'nop $E9A2'
	ldx #233
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5606)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dex
	Jne *-7
.elseif(n = 5610)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5612)
	; 10 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5613)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5615)
	; 9 bytes
	ldx #234 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5616)
	; 10 bytes
	nop <$0C ;hides 'nop $16A2'
	ldx #22
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.endif
.endif
.endif
.endif
.else
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
.if((n >= 5626) && (n < 5704))
; 5626 to 5703
.if((n > 5625 && n < 5633) || n = 5637 || n = 5638 || n = 5642 || (n > 5644 && n < 5657) || (n > 5659 && n < 5663) || n = 5666 || n = 5668 || n = 5670 || (n > 5671 && n < 5680) || (n > 5683 && n < 5687) || (n > 5689 && n < 5695) || n = 5696 || n = 5697 || n = 5699 || n = 5701)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5633)
	; 9 bytes
	ldx #0
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-5
.elseif(n = 5634)
	; 9 bytes
	ldx #(n-22)/23 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5635 || n = 5702)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5636 || n = 5659 || n = 5682)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5639)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.elseif(n = 5640)
	; 10 bytes
	nop <$0C ;hides 'nop $EBA2'
	ldx #235
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5641 || n = 5665 || n = 5689)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5643 || n = 5669 || n = 5695)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5644 || n = 5671 || n = 5698)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5657)
	; 10 bytes
	nop #<$0C ;hides 'nop $F6A2'
	ldx #246
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5658)
	; 10 bytes
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5663)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.elseif(n = 5664)
	; 10 bytes
	nop <$0C ;hides 'nop $ECA2'
	ldx #236
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5667)
	; 10 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 5680)
	; 10 bytes
	nop #<$0C ;hides 'nop $09A2'
	ldx #9
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5681)
	; 10 bytes
	nop <$0C ;hides 'nop $F7A2'
	ldx #247
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5683)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 9
	jsr @nearby_rts_14cyc
	Jne *-7
.elseif(n = 5687)
	; 9 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	inx
	Jne *-6
.elseif(n = 5688)
	; 10 bytes
	nop <$0C ;hides 'nop $13A2'
	ldx #19
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5700)
	; 9 bytes
	php
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5703)
	; 9 bytes
	ldx #248 ;hides 'sed'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.endif
.else
; 5704 to 5781
.if(n = 5704)
	; 10 bytes
	nop <$0C ;hides 'nop $08A2'
	ldx #8
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5705 || n = 5728 || n = 5751 || n = 5774)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif((n > 5705 && n < 5711) || (n > 5713 && n < 5721) || (n > 5721 && n < 5725) || (n > 5728 && n < 5735) || (n > 5737 && n < 5741) || (n > 5741 && n < 5747) || n = 5748 || (n > 5752 && n < 5759) || (n > 5761 && n < 5772) || (n > 5774 && n < 5779) || n = 5780 || n = 5781)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5711)
	; 10 bytes
	nop #<$0C ;hides 'nop $EEA2'
	ldx #238
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5712)
	; 10 bytes
	nop <$0C ;hides 'nop $EEA2'
	ldx #238
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5713 || n = 5737 || n = 5761)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5721 || n = 5747 || n = 5773)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5725 || n = 5752 || n = 5779)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5726)
	; 10 bytes
	nop #<$0C ;hides 'nop $07A2'
	ldx #7
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5727)
	; 10 bytes
	nop <$0C ;hides 'nop $07A2'
	ldx #7
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5735)
	; 10 bytes
	nop #<$0C ;hides 'nop $11A2'
	ldx #17
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5736)
	; 10 bytes
	nop <$0C ;hides 'nop $11A2'
	ldx #17
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5741)
	; 10 bytes
	ldx #58 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5749)
	; 9 bytes
	ldx #250 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5750)
	; 10 bytes
	nop <$0C ;hides 'nop $06A2'
	ldx #6
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5759)
	; 10 bytes
	nop #<$0C ;hides 'nop $10A2'
	ldx #16
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5760)
	; 10 bytes
	nop <$0C ;hides 'nop $10A2'
	ldx #16
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5772)
	; 10 bytes
	nop #<$0C ;hides 'nop $05A2'
	ldx #5
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if(n = 5782 || (n > 5785 && n < 5795) || n = 5798 || (n > 5799 && n < 5806) || (n > 5809 && n < 5818) || (n > 5820 && n < 5825) || (n > 5825 && n < 5831) || (n > 5833 && n < 5841) || (n > 5843 && n < 5849) || n = 5850 || (n > 5851 && n < 5855) || n = 5858 || n = 5859)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5783)
	; 10 bytes
	nop #<$0C ;hides 'nop $0FA2'
	ldx #15
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5784)
	; 10 bytes
	nop <$0C ;hides 'nop $F1A2'
	ldx #241
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5785 || n = 5809 || n = 5833 || n = 5857)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5795)
	; 10 bytes
	nop #<$0C ;hides 'nop $04A2'
	ldx #4
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5796)
	; 10 bytes
	nop <$0C ;hides 'nop $04A2'
	ldx #4
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5797 || n = 5820 || n = 5843)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5799 || n = 5825 || n = 5851)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5806)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5807)
	; 10 bytes
	ldx #<$8D ;hides 'sta $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5808)
	; 10 bytes
	nop <$0C ;hides 'nop $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5818)
	; 10 bytes
	nop #<$0C ;hides 'nop $03A2'
	ldx #3
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5819)
	; 10 bytes
	nop <$0C ;hides 'nop $03A2'
	ldx #3
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5831)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5832)
	; 10 bytes
	nop <$0C ;hides 'nop $F3A2'
	ldx #243
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 5841)
	; 10 bytes
	nop #<$0C ;hides 'nop $02A2'
	ldx #2
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5842)
	; 10 bytes
	nop <$0C ;hides 'nop $02A2'
	ldx #2
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5849)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5855)
	; 9 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5856)
	; 10 bytes
	ldx #12 ;hides 'nop abs'
	nop $0
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.endif
.else
; 5860 to 5937
.if(n = 5860 || n = 5887 || n = 5914)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif((n > 5860 && n < 5864) || (n > 5866 && n < 5874) || n = 5875 || n = 5876 || n = 5878 || (n > 5881 && n < 5886) || (n > 5889 && n < 5898) || (n > 5899 && n < 5903) || (n > 5905 && n < 5914) || (n > 5914 && n < 5924) || n = 5925 || n = 5926 || (n > 5929 && n < 5938))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5864)
	; 10 bytes
	nop #<$0C ;hides 'nop $01A2'
	ldx #1
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5865)
	; 10 bytes
	nop <$0C ;hides 'nop $01A2'
	ldx #1
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5866)
	; 10 bytes
	ldx #(n-1)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5874)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5877 || n = 5903)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5879)
	; 9 bytes
	ldx #(n-23)/24 ;hides 'nop zp,x'
	inx ;first loop only
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5880)
	; 10 bytes
	nop <$0C ;hides 'nop $0BA2'
	ldx #11
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5881 || n = 5905 || n = 5929)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5886)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
.elseif(n = 5888)
	; 10 bytes
	nop <$0C ;hides 'nop $00A2'
	ldx #0
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 5889)
	; 10 bytes
	ldx #0
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5898)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5899)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5904)
	; 10 bytes
	nop <$0C ;hides 'nop $0AA2'
	ldx #10
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5924)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 5927)
	; 10 bytes
	nop #<$0C ;hides 'nop $09A2'
	ldx #9
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5928)
	; 10 bytes
	nop <$0C ;hides 'nop $F7A2'
	ldx #247
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if((n > 5937 && n < 5941) || (n > 5941 && n < 5951) || n = 5954 || n = 5956 || n = 5958 || (n > 5959 && n < 5968) || (n > 5968 && n < 5975) || n = 5978 || n = 5980 || (n > 5981 && n < 5995) || (n > 5995 && n < 5999) || (n > 6001 && n < 6005) || n = 6006 || (n > 6007 && n < 6022) || (n > 6025 && n < 6031) || n = 6032 || (n > 6033 && n < 6047) || (n > 6049 && n < 6059) || (n > 6059 && n < 6071) || n = 6075 || (n > 6076 && n < 6083) || n = 6084 || (n > 6085 && n < 6094))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5941 || n = 5968 || n = 5995 || n = 6022 || n = 6076)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5951)
	; 9 bytes
	ldx #248 ;hides 'sed'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 5952)
	; 10 bytes
	nop <$0C ;hides 'nop $08A2'
	ldx #8
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5953 || n = 5977 || n = 6001 || n = 6025 || n = 6049 || n = 6073)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 5955 || n = 5981 || n = 6007 || n = 6033 || n = 6059 || n = 6085)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 5957)
	; 9 bytes
	php
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5959)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5975)
	; 10 bytes
	nop #<$0C ;hides 'nop $07A2'
	ldx #7
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5976)
	; 10 bytes
	nop <$0C ;hides 'nop $07A2'
	ldx #7
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 5979)
	; 10 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5999)
	; 9 bytes
	ldx #250 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 6000)
	; 10 bytes
	nop <$0C ;hides 'nop $06A2'
	ldx #6
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6005)
	; 10 bytes
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-7
.elseif(n = 6023)
	; 10 bytes
	nop #<$0C ;hides 'nop $05A2'
	ldx #5
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6024)
	; 10 bytes
	nop <$0C ;hides 'nop $05A2'
	ldx #5
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6031)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 6047)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 6048)
	; 10 bytes
	nop <$0C ;hides 'nop $04A2'
	ldx #4
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6071)
	; 10 bytes
	nop #<$0C ;hides 'nop $03A2'
	ldx #3
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6072)
	; 10 bytes
	nop <$0C ;hides 'nop $03A2'
	ldx #3
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6074)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 6083)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-7
.endif
.else
; 6094 to 6250
.if(n = 6094 || (n > 6099 && n < 6103) || (n > 6103 && n < 6109) || n = 6110 || (n > 6111 && n < 6119) || n = 6122 || n = 6123 || (n > 6124 && n < 6130) || (n > 6130 && n < 6134) || n = 6136 || (n > 6137 && n < 6143) || (n > 6145 && n < 6157) || (n > 6157 && n < 6161) || n = 6162 || (n > 6163 && n < 6184) || (n > 6184 && n < 6189) || (n > 6189 && n < 6199) || (n > 6199 && n < 6211) || n = 6212 || n = 6213 || (n > 6216 && n < 6238) || n = 6239 || n = 6240 || (n > 6241 && n < 6249) || n = 6250)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6095)
	; 10 bytes
	nop #<$0C ;hides 'nop $02A2'
	ldx #2
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6096)
	; 10 bytes
	nop <$0C ;hides 'nop $02A2'
	ldx #2
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6097 || n = 6121)
	; 10 bytes
	ldx #(n-1)/24
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 6098)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 6099)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 6103 || n = 6130 || n = 6157 || n = 6184 || n = 6211 || n = 6238)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 6109)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6111 || n = 6137 || n = 6163 || n = 6189 || n = 6215 || n = 6241)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 6119)
	; 10 bytes
	nop #<$0C ;hides 'nop $01A2'
	ldx #1
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6120)
	; 10 bytes
	nop <$0C ;hides 'nop $01A2'
	ldx #1
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6124)
	; 10 bytes
	ldx #(n-24)/25 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 6134)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6135)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6143)
	; 10 bytes
	nop #<$0C ;hides 'nop $00A2'
	ldx #0
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6144)
	; 10 bytes
	nop <$0C ;hides 'nop $00A2'
	ldx #0
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 6145)
	; 10 bytes
	ldx #0
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 6161)
	; 10 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6199)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 6214)
	; 9 bytes
	php
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6216)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6249)
	; 10 bytes
	ldx #250 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.endif
.endif
.endif
.endif
.else
.if((n >= 6251) && (n < 6876))
; 6251 to 6875
.if((n > 6250 && n < 6263) || n = 6264 || n = 6266 || (n > 6267 && n < 6292) || (n > 6293 && n < 6317) || n = 6318 || n = 6320 || (n > 6321 && n < 6342) || n = 6344 || (n > 6346 && n < 6369) || n = 6370 || n = 6372 || (n > 6373 && n < 6397) || n = 6398 || n = 6399 || (n > 6400 && n < 6423) || (n > 6423 && n < 6427) || (n > 6427 && n < 6439) || (n > 6439 && n < 6447) || n = 6448 || (n > 6449 && n < 6454) || (n > 6454 && n < 6471) || n = 6472 || n = 6474 || (n > 6475 && n < 6481) || (n > 6481 && n < 6495) || (n > 6495 && n < 6499) || n = 6500 || (n > 6501 && n < 6508) || (n > 6508 && n < 6527) || (n > 6527 && n < 6535) || (n > 6535 && n < 6551) || n = 6552 || (n > 6553 && n < 6562) || (n > 6562 && n < 6579) || (n > 6579 && n < 6589) || (n > 6589 && n < 6605) || (n > 6605 && n < 6616) || (n > 6616 && n < 6631) || (n > 6631 && n < 6643) || (n > 6643 && n < 6657) || (n > 6657 && n < 6669) || (n > 6670 && n < 6697) || (n > 6698 && n < 6724) || n = 6725 || n = 6726 || n = 6729 || (n > 6730 && n < 6751) || (n > 6751 && n < 6778) || (n > 6778 && n < 6785) || (n > 6785 && n < 6805) || (n > 6805 && n < 6832) || (n > 6832 && n < 6859) || (n > 6859 && n < 6876))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6263)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 6265 || n = 6292 || n = 6346 || n = 6373 || n = 6400 || n = 6427 || n = 6454 || n = 6481 || n = 6508 || n = 6535 || n = 6562 || n = 6589 || n = 6616 || n = 6643 || n = 6670 || n = 6697 || n = 6724 || n = 6751 || n = 6778 || n = 6805 || n = 6832 || n = 6859)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 6267 || n = 6293 || n = 6319 || n = 6345 || n = 6371 || n = 6397 || n = 6423 || n = 6449 || n = 6475 || n = 6501 || n = 6527 || n = 6553 || n = 6579 || n = 6605 || n = 6631)
	; 10 bytes
	ldx #(n-1)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 6317)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 6321)
	; 10 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 6342)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 6343)
	; 10 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 6369)
	; 10 bytes
	ldx #(n-25)/26 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 6439)
	; 10 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
.elseif(n = 6447)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 6471)
	; 9 bytes
	php
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6473 || n = 6730)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6495)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-7
.elseif(n = 6499)
	; 10 bytes
	ldx #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dex
	Jne *-7
.elseif(n = 6551)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 6657)
	; 10 bytes
	ldx #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
.elseif(n = 6669)
	; 10 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6698)
	; 10 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_14cyc
	Jne *-7
.elseif(n = 6727)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
.elseif(n = 6728)
	; 9 bytes
	php
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6785)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.endif
.else
; 6876 to 7500
.if((n > 6875 && n < 6886) || (n > 6886 && n < 6901) || (n > 6901 && n < 6913) || (n > 6913 && n < 6943) || (n > 6943 && n < 6985) || n = 6986 || (n > 6987 && n < 6999) || (n > 6999 && n < 7104) || (n > 7104 && n < 7191) || (n > 7191 && n < 7242) || n = 7243 || (n > 7244 && n < 7249) || (n > 7249 && n < 7499) || n = 7500)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6886)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 6901)
	; 10 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	Jne *-7
.elseif(n = 6913)
	; 10 bytes
	ldx #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
.elseif(n = 6943)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 6985)
	; 9 bytes
	php
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6987 || n = 7244)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6999)
	; 10 bytes
	ldx #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
.elseif(n = 7104)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 30
	Jne *-7
.elseif(n = 7191)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
.elseif(n = 7242)
	; 9 bytes
	php
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7249)
	; 10 bytes
	ldx #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-7
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
.endif
.else
; 7501 to 10000
.if(n = 7501 || n = 7758 || n = 8015 || n = 8272 || n = 8529 || n = 8786 || n = 9043 || n = 9300 || n = 9557 || n = 9814)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 7501 && n < 7756) || n = 7757 || (n > 7758 && n < 8013) || n = 8014 || (n > 8015 && n < 8270) || n = 8271 || (n > 8272 && n < 8527) || n = 8528 || (n > 8529 && n < 8784) || n = 8785 || (n > 8786 && n < 9041) || n = 9042 || (n > 9043 && n < 9298) || n = 9299 || (n > 9300 && n < 9555) || n = 9556 || (n > 9557 && n < 9812) || n = 9813 || (n > 9814 && n < 10001))
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
.if((n > 10000 && n < 10069) || n = 10070 || (n > 10071 && n < 10326) || n = 10327 || (n > 10328 && n < 10583) || n = 10584 || (n > 10585 && n < 10840) || n = 10841 || (n > 10842 && n < 11097) || n = 11098 || (n > 11099 && n < 11354) || n = 11355 || (n > 11356 && n < 11611) || n = 11612 || (n > 11613 && n < 11868) || n = 11869 || (n > 11870 && n < 12125) || n = 12126 || (n > 12127 && n < 12382) || n = 12383 || (n > 12384 && n < 12639) || n = 12640 || (n > 12641 && n < 12896) || n = 12897 || (n > 12898 && n < 13153) || n = 13154 || (n > 13155 && n < 13410) || n = 13411 || (n > 13412 && n < 13667) || n = 13668 || (n > 13669 && n < 13924) || n = 13925 || (n > 13926 && n < 14181) || n = 14182 || (n > 14183 && n < 14438) || n = 14439 || (n > 14440 && n < 14695) || n = 14696 || (n > 14697 && n < 14952) || n = 14953 || (n > 14954 && n < 15001))
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
.if((n > 15000 && n < 15209) || n = 15210 || (n > 15211 && n < 15466) || n = 15467 || (n > 15468 && n < 15723) || n = 15724 || (n > 15725 && n < 15980) || n = 15981 || (n > 15982 && n < 16237) || n = 16238 || (n > 16239 && n < 16494) || n = 16495 || (n > 16496 && n < 16751) || n = 16752 || (n > 16753 && n < 17008) || n = 17009 || (n > 17010 && n < 17265) || n = 17266 || (n > 17267 && n < 17522) || n = 17523 || (n > 17524 && n < 17779) || n = 17780 || (n > 17781 && n < 18036) || n = 18037 || (n > 18038 && n < 18293) || n = 18294 || (n > 18295 && n < 18550) || n = 18551 || (n > 18552 && n < 18807) || n = 18808 || (n > 18809 && n < 19064) || n = 19065 || (n > 19066 && n < 19321) || n = 19322 || (n > 19323 && n < 19578) || n = 19579 || (n > 19580 && n < 19835) || n = 19836 || (n > 19837 && n < 20001))
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
