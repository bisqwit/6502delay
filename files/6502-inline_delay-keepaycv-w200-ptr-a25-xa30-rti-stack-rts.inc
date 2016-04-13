; delay_n, delays n cycles with small code
; Will clobber X, S, Z+N, and D
; Will preserve A, Y, C, V, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; WARNING: May write garbage into $200-$2FF.
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

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
; - 6 bytes or less when n <= 36
; - 7 bytes or less when n <= 1289
; - 8 bytes or less when n <= 1849
; - 9 bytes or less when n <= 2529
; -10 bytes or less when n <= 3329
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.2, max=6
; - When 31 <= n <= 50: min=5, avg=6.0, max=7
; - When 51 <= n <= 75: min=5, avg=6.0, max=7
; - When 76 <= n <= 100: min=5, avg=6.1, max=7
; - When 101 <= n <= 200: min=4, avg=6.0, max=7
; - When 201 <= n <= 300: min=5, avg=6.1, max=7
; - When 301 <= n <= 500: min=4, avg=6.1, max=7
; - When 501 <= n <= 750: min=5, avg=6.1, max=7
; - When 751 <= n <= 1000: min=5, avg=6.1, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.8, max=9
; - When 2001 <= n <= 3000: min=6, avg=8.0, max=10
; - When 3001 <= n <= 5000: min=6, avg=8.9, max=11
; - When 5001 <= n <= 7500: min=7, avg=10.4, max=11
; - When 7501 <= n <= 10000: min=8, avg=10.9, max=11
; - When 10001 <= n <= 20000: min=10, avg=11.0, max=11
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
	; 4 bytes
	.repeat 2
	 php
	 plp
	.endrepeat
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
	; 6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 31)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 34)
	; 6 bytes
	pha
	delay_n (n-3)
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
.if(n = 40 || n = 48 || n = 52 || n = 53 || n = 58 || n = 60 || n = 63 || n = 68 || n = 70 || n = 73 || n = 75)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 41 || n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 42 || n = 72)
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
.elseif(n = 44 || n = 54 || n = 59 || n = 69 || n = 74)
	; 6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 49 || n = 65)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
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
.endif
.else
; 79 to 156
.if(n = 79 || n = 84 || n = 94 || n = 104 || n = 109 || n = 114 || n = 119 || n = 127 || n = 139 || n = 144 || n = 149 || n = 154)
	; 5..6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 83 || n = 87 || n = 88 || n = 90 || n = 93 || n = 95 || n = 98 || n = 100 || n = 103 || n = 108 || n = 110 || n = 118 || n = 122 || n = 123 || n = 125 || n = 128 || n = 133 || n = 135 || n = 138 || n = 140 || n = 143)
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
.elseif(n = 89 || n = 105 || n = 145)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
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
.elseif(n = 124)
	; 4 bytes
	ldx #232 ;hides 'inx'
	Jne *-1
.elseif(n = 137)
	; 6 bytes
	brk
	delay_n (n-13)
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
.if(n = 157 || n = 158 || n = 163 || n = 165 || n = 168 || n = 173 || n = 175 || n = 178 || n = 180 || n = 188 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 203 || n = 205 || n = 208 || n = 210 || n = 213 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233 || n = 235 || n = 238 || n = 240 || n = 243 || n = 245 || n = 248 || n = 250 || n = 258 || n = 263 || n = 268 || n = 270 || n = 273 || n = 275 || n = 278 || n = 280 || n = 283 || n = 285 || n = 293 || n = 298 || n = 300 || n = 303 || n = 308 || n = 310)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 170 || n = 184 || n = 189 || n = 194 || n = 202 || n = 214 || n = 219 || n = 229 || n = 234 || n = 244 || n = 254 || n = 259 || n = 264 || n = 269 || n = 282 || n = 284 || n = 294 || n = 304)
	; 6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 172 || n = 262 || n = 307)
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
.elseif(n = 177 || n = 185 || n = 217 || n = 289 || n = 297 || n = 305)
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
.elseif(n = 182 || n = 187 || n = 212 || n = 222 || n = 237 || n = 242 || n = 252 || n = 292 || n = 312)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 199)
	; 5 bytes
	ldx #232 ;hides 'inx'
	pha
	Jne *-2
.elseif(n = 207 || n = 247 || n = 287)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 209 || n = 249 || n = 257)
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
.elseif(n = 220 || n = 255 || n = 290)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 224)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	dex
	Jpl *-3
.elseif(n = 265)
	; 5 bytes
	ldx #202 ;hides 'dex'
	dex
	Jmi *-2
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
; 313 to 625
.if(n = 313 || n = 315 || n = 318 || n = 320 || n = 328 || n = 332 || n = 333 || n = 335 || n = 338 || n = 340 || n = 343 || n = 348 || n = 350 || n = 353 || n = 355 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 380 || n = 383 || n = 388 || n = 390 || n = 402 || n = 403 || n = 408 || n = 410 || n = 413 || n = 415 || n = 418 || n = 420 || n = 423 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 448 || n = 450 || n = 453 || n = 455 || n = 458 || n = 460 || n = 468 || n = 472 || n = 473 || n = 475 || n = 478 || n = 480 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 528 || n = 530 || n = 535 || n = 538 || n = 542 || n = 543 || n = 548 || n = 553 || n = 555 || n = 558 || n = 560 || n = 563 || n = 565 || n = 570 || n = 573 || n = 578 || n = 580 || n = 583 || n = 588 || n = 590 || n = 593 || n = 598 || n = 600 || n = 605 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314 || n = 319 || n = 324 || n = 334 || n = 339 || n = 349 || n = 354 || n = 364 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459 || n = 464 || n = 469 || n = 474 || n = 479 || n = 494 || n = 499 || n = 504 || n = 509 || n = 514 || n = 524 || n = 534 || n = 539 || n = 544 || n = 549 || n = 559 || n = 564 || n = 574 || n = 579 || n = 584 || n = 594 || n = 599 || n = 604 || n = 614 || n = 619)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462) || (n > 475 && n < 497) || (n > 505 && n < 517) || (n > 525 && n < 547) || n = 556 || n = 561 || (n > 570 && n < 582) || n = 596 || (n > 605 && n < 617)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 322 || n = 347 || n = 357 || n = 387 || n = 432 || n = 447 || n = 462 || n = 482 || n = 492 || n = 502 || n = 522 || n = 552 || n = 562 || n = 567 || n = 587 || n = 602)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 321 || n = 326 || n = 336 || n = 356 || n = 406 || n = 416 || n = 421 || n = 431 || n = 436 || n = 446 || n = 466 || n = 471 || n = 501 || n = 521 || n = 551 || n = 566 || n = 586 || n = 591 || n = 601 || n = 621)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 7 = 1) && (n = 323 || n = 330 || n = 344 || n = 365 || (n > 399 && n < 415) || (n > 427 && n < 443) || n = 463 || n = 477 || n = 484 || (n > 504 && n < 520) || (n > 532 && n < 555) || n = 575 || n = 617))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 325 || n = 360 || n = 395 || n = 430 || n = 500)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 327 || n = 342 || n = 362 || n = 412 || n = 422 || n = 427 || n = 452 || n = 527 || n = 557 || n = 572 || n = 592 || n = 597)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 329 || n = 345 || n = 369 || n = 377 || n = 417 || n = 457 || n = 465 || n = 489 || n = 497 || n = 529 || n = 545 || n = 577 || n = 585 || n = 609 || n = 625)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 337 || n = 358 || n = 372 || n = 393 || n = 449 || n = 470 || n = 498 || n = 568 || n = 582 || n = 589 || n = 603 || n = 610 || n = 624)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 352 || n = 397 || n = 532 || n = 550 || n = 595)
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
.elseif(n = 385 || n = 409 || n = 425 || n = 537 || n = 569)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 392)
	; 6 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 405 || n = 510)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 467)
	; 6 bytes
	ldx #24 ;hides 'clc'
	inx
	inx
	Jpl *-3
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
.elseif(n = 503)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
.elseif(n = 517)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 607)
	; 5 bytes
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
.elseif(n = 622)
	; 5 bytes
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
.endif
.endif
.else
.if((n >= 626) && (n < 938))
; 626 to 937
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 937)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 5 = 2) && (n = 627 || n = 632 || n = 642 || n = 662 || n = 672 || n = 677 || n = 692 || n = 702 || n = 707 || n = 732 || n = 742 || n = 747 || (n > 761 && n < 773) || n = 782 || n = 807 || n = 812 || n = 837 || n = 842 || n = 852 || n = 867 || n = 872 || n = 882 || n = 887 || (n > 901 && n < 923)))
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 628 || n = 630 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 668 || n = 670 || n = 675 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 693 || n = 695 || n = 698 || n = 700 || n = 703 || n = 713 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 752 || n = 753 || n = 758 || n = 760 || n = 763 || n = 765 || n = 768 || n = 770 || n = 773 || n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 803 || n = 805 || n = 808 || n = 810 || n = 818 || n = 822 || n = 823 || n = 828 || n = 830 || n = 833 || n = 838 || n = 840 || n = 843 || n = 845 || n = 853 || n = 858 || n = 860 || n = 863 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 903 || n = 908 || n = 910 || n = 913 || n = 915 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 629 || n = 634 || n = 639 || n = 644 || n = 654 || n = 664 || n = 669 || n = 674 || n = 679 || n = 684 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 730 || n = 734 || n = 739 || n = 744 || n = 749 || n = 754 || n = 759 || n = 774 || n = 779 || n = 784 || n = 789 || n = 794 || n = 804 || n = 814 || n = 819 || n = 824 || n = 829 || n = 839 || n = 844 || n = 854 || n = 859 || n = 864 || n = 874 || n = 879 || n = 884 || n = 894 || n = 909 || n = 914 || n = 919 || n = 924 || n = 934)
	; 6 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 7 = 1) && (n = 638 || n = 645 || (n > 672 && n < 695) || (n > 707 && n < 723) || (n > 742 && n < 765) || n = 778 || n = 785 || n = 799 || n = 813 || n = 834 || (n > 847 && n < 863) || n = 883 || n = 904))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 640 || n = 775 || n = 802 || n = 847)
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
.elseif(n = 649 || n = 665 || n = 689 || n = 697 || n = 745 || n = 905 || n = 937)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 652 || n = 659 || n = 729 || n = 792 || n = 820 || n = 827 || n = 869 || n = 890 || n = 897 || n = 918 || n = 925 || n = 932)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657 || n = 705 || n = 737 || n = 769 || n = 777 || n = 809 || n = 817 || n = 825 || n = 849 || n = 857 || n = 865 || n = 889 || n = 929)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 667 || n = 685 || n = 712 || n = 892)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 710 || n = 780 || n = 815 || n = 850 || n = 885 || n = 920)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
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
	; 7 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
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
.else
; 938 to 1250
.if(n = 938 || n = 940 || n = 943 || n = 948 || n = 950 || n = 958 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 980 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1063 || n = 1067 || n = 1068 || n = 1070 || n = 1073 || n = 1075 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1088 || n = 1090 || n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1110 || n = 1113 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1133 || n = 1138 || n = 1140 || n = 1143 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1158 || n = 1160 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1183 || n = 1188 || n = 1190 || n = 1193 || n = 1195 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1228 || n = 1230 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248 || n = 1250)
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
.elseif(n = 942 || n = 947 || n = 952 || n = 957 || n = 987 || n = 992 || n = 1012 || n = 1020 || n = 1022 || n = 1042 || n = 1047 || n = 1062 || n = 1077 || n = 1082 || n = 1087 || n = 1092 || n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1152 || n = 1157 || n = 1167 || n = 1182 || n = 1192 || n = 1197 || n = 1202 || n = 1222 || n = 1227 || n = 1232)
	; 6..7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1124 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && (n = 945 || n = 1025 || n = 1049 || n = 1057 || (n > 1088 && n < 1106) || (n > 1128 && n < 1146) || n = 1169 || n = 1185 || (n > 1208 && n < 1226) || n = 1249))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 955 || n = 990 || n = 1060 || n = 1095 || n = 1130 || n = 1165 || n = 1200 || n = 1235)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 982 || n = 1162)
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
.elseif(n = 1027)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 1052)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
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
.elseif(n = 1187)
	; 7 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
	dex
	Jne *-4
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
.elseif(n = 1252 || n = 1306 || n = 1315)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1263 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1290 || n = 1291 || n = 1293 || n = 1298 || n = 1307 || n = 1312 || n = 1314 || n = 1318 || n = 1319 || n = 1322 || n = 1323 || n = 1325 || n = 1326 || n = 1328)
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
.elseif(n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1292 || n = 1300 || n = 1304 || n = 1308 || n = 1316 || n = 1320 || n = 1327)
	; 6..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1287 || n = 1302)
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
.elseif(n = 1301 || n = 1311)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1309)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.endif
.else
; 1329 to 1406
.if((n .mod 8 = 1) && (n = 1329 || n = 1337 || (n > 1352 && n < 1394)))
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
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1402)))
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
.elseif((n .mod 9 = 1) && ((n > 1332 && n < 1361) || n = 1378 || n = 1396 || n = 1405))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1334 || n = 1346 || n = 1348 || n = 1356 || n = 1362 || n = 1364 || n = 1370 || n = 1372 || n = 1383 || n = 1388 || n = 1390 || n = 1397 || n = 1402 || n = 1404)
	; 6..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1335)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1336)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 1339 || n = 1340 || n = 1344 || n = 1347 || n = 1349 || n = 1350 || n = 1354 || n = 1355 || n = 1368 || n = 1375 || n = 1379 || n = 1382 || n = 1384 || n = 1389 || n = 1395 || n = 1403)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1341 || n = 1371 || n = 1391)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1357)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1358 || n = 1386 || n = 1400)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1363)
	; 7 bytes
	ldx #248 ;hides 'sed'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1365 || n = 1398)
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
.elseif(n = 1381)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1392)
	; 8 bytes
	ldx #106
	nop
	delay_n 6
	dex
	Jpl *-4
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.elseif(n = 1406)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 1407 to 1562
.if(n = 1407 || n = 1410 || n = 1416 || n = 1419 || n = 1424 || n = 1431 || n = 1434 || n = 1435 || n = 1437 || n = 1438 || n = 1440 || n = 1445 || n = 1447 || n = 1448 || n = 1452 || n = 1454 || n = 1462 || n = 1466 || n = 1467 || n = 1469 || n = 1470 || n = 1479 || n = 1480 || n = 1482 || n = 1483 || n = 1487 || n = 1490 || n = 1494 || n = 1496 || n = 1501 || n = 1503 || n = 1507 || n = 1508 || n = 1510 || n = 1515 || n = 1523 || n = 1533 || n = 1536 || n = 1538 || n = 1539 || n = 1542 || n = 1543 || n = 1547 || n = 1550 || n = 1557 || n = 1559 || n = 1560)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && ((n > 1408 && n < 1450) || (n > 1464 && n < 1506) || (n > 1520 && n < 1562)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1411 || n = 1421 || n = 1451 || n = 1461 || n = 1491 || n = 1551)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1412 || n = 1524)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1413 || n = 1493)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 9 = 1) && (n = 1414 || n = 1432 || (n > 1458 && n < 1505) || (n > 1521 && n < 1559)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1418 || n = 1423 || n = 1428 || n = 1430 || n = 1444 || n = 1446 || n = 1458 || n = 1460 || n = 1474 || n = 1476 || n = 1484 || n = 1488 || n = 1500 || n = 1502 || n = 1509 || n = 1514 || n = 1516 || n = 1528 || n = 1532 || n = 1544 || n = 1556)
	; 6..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1426 || n = 1456 || n = 1463 || n = 1498 || n = 1512 || n = 1517 || n = 1526)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1427 || n = 1518)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1439)
	; 7 bytes
	ldx #56 ;hides 'sec'
	brk
	inx
	Jpl *-4
.elseif(n = 1442 || n = 1475 || n = 1530 || n = 1552)
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
.elseif(n = 1472)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1511)
	; 5 bytes
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
.elseif(n = 1519)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	brk
	Jmi *-4
.elseif(n = 1525)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1535)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1546)
	; 8 bytes
	ldx #102
	delay_n 10
	dex
	Jpl *-4
.elseif(n = 1554)
	; 8 bytes
	pha
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1574 || n = 1596 || n = 1607 || n = 1706)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1564 || n = 1566 || n = 1571 || n = 1578 || n = 1579 || n = 1587 || n = 1592 || n = 1595 || n = 1598 || n = 1599 || n = 1602 || n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1615 || n = 1619 || n = 1620 || n = 1622 || n = 1627 || n = 1634 || n = 1635 || n = 1647 || n = 1654 || n = 1655 || n = 1658 || n = 1659 || n = 1662 || n = 1664 || n = 1669 || n = 1672 || n = 1676 || n = 1678 || n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1699 || n = 1704 || n = 1707 || n = 1710 || n = 1714 || n = 1715 || n = 1718)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1570 || n = 1572 || n = 1580 || n = 1584 || n = 1586 || n = 1588 || n = 1600 || n = 1614 || n = 1626 || n = 1628 || n = 1640 || n = 1642 || n = 1652 || n = 1656 || n = 1663 || n = 1668 || n = 1670 || n = 1677 || n = 1692 || n = 1698 || n = 1700 || n = 1708 || n = 1712)
	; 6..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1593 && n < 1622) || n = 1648 || (n > 1665 && n < 1694) || n = 1711))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1568 || n = 1582 || n = 1589 || n = 1616 || n = 1624 || n = 1629 || n = 1638 || n = 1643 || n = 1680 || n = 1687 || n = 1694)
	; 7..8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1717)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1573 || n = 1645 || n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1575)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif((n .mod 8 = 1) && ((n > 1576 && n < 1618) || (n > 1632 && n < 1674) || (n > 1688 && n < 1714)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1581 || n = 1591 || n = 1631 || n = 1651 || n = 1661 || n = 1671 || n = 1691 || n = 1701)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1610)
	; 6 bytes
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
.elseif(n = 1623)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
.elseif(n = 1630 || n = 1644)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1636 || n = 1650)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1679 || n = 1703)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1696)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1724 || n = 1726 || n = 1733 || n = 1738 || n = 1740 || n = 1754 || n = 1764 || n = 1768 || n = 1775 || n = 1780 || n = 1782 || n = 1796)
	; 6..7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1722 || n = 1725 || n = 1727 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1749 || n = 1755 || n = 1760 || n = 1762 || n = 1763 || n = 1766 || n = 1767 || n = 1770 || n = 1781 || n = 1784 || n = 1787 || n = 1788 || n = 1790 || n = 1795)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1728 || n = 1750 || n = 1794)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 1736 || n = 1757 || n = 1778)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 1748)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1752)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1759)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1773 || n = 1776)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1793)
	; 6 bytes
	ldx #0
	nop
	dex
	Jne *-2
.endif
.else
; 1797 to 1875
.if(n = 1797 || n = 1798 || n = 1803 || n = 1814 || n = 1815 || n = 1818 || n = 1822 || n = 1830 || n = 1835 || n = 1840 || n = 1843 || n = 1845 || n = 1848 || n = 1850 || n = 1859 || n = 1862 || n = 1866 || n = 1867 || n = 1869 || n = 1870 || n = 1875)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1800 || n = 1806)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 8 = 1) && ((n > 1800 && n < 1874)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1802 || n = 1804 || n = 1812 || n = 1820 || n = 1829 || n = 1836 || n = 1844 || n = 1852 || n = 1858 || n = 1868 || n = 1874)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 11 = 1) && ((n > 1804 && n < 1839) || n = 1860))
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
.elseif(n = 1808 || n = 1834)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1809 && n < 1865)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1811 || n = 1821 || n = 1831 || n = 1851 || n = 1871)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1813 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
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
.elseif(n = 1832)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1839)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1842)
	; 8 bytes
	pha
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 1847)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1853)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1854)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1856 || n = 1872)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1863)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
; 1876 to 1953
.if(n = 1876 || n = 1884 || n = 1892 || n = 1896 || n = 1908 || n = 1916 || n = 1924 || n = 1930 || n = 1932 || n = 1940 || n = 1944 || n = 1946)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1877 || n = 1919)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1878 || n = 1883 || n = 1887 || n = 1890 || n = 1894 || n = 1899 || n = 1902 || n = 1907 || n = 1910 || n = 1920 || n = 1923 || n = 1925 || n = 1928 || n = 1934 || n = 1935 || n = 1938 || n = 1939 || n = 1942 || n = 1947)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879 || n = 1895 || n = 1903)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1880 || n = 1952)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1954)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1937)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1885 || n = 1933)
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
.elseif((n .mod 11 = 1) && ((n > 1892 && n < 1927) || n = 1948))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1898 || n = 1950)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1901 || n = 1911 || n = 1931 || n = 1941 || n = 1951)
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
.elseif(n = 1914)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1917)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1922)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 1943)
	; 6 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
.elseif(n = 1949)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.endif
.else
; 1954 to 2031
.if((n .mod 9 = 1) && ((n > 1953 && n < 1991) || n = 2008 || n = 2026))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1955 || n = 1958 || n = 1965 || n = 1974 || n = 1979 || n = 1982 || n = 1983 || n = 1987 || n = 1994 || n = 1995 || n = 1997 || n = 1998 || n = 2006 || n = 2007 || n = 2010 || n = 2019 || n = 2022 || n = 2027 || n = 2030)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1956 || n = 1960 || n = 1962 || n = 1964 || n = 1980 || n = 1984 || n = 1988 || n = 1996 || n = 2002 || n = 2004 || n = 2012 || n = 2020 || n = 2028)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1957 || n = 2005)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1959 || n = 1970 || n = 1992 || n = 2003 || n = 2014)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif((n .mod 8 = 1) && ((n > 1960 && n < 2026)))
	; 6 bytes
	ldx #(n-1)/8
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 1968)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 1971 || n = 1991 || n = 2011 || n = 2021 || n = 2031)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 1973 || n = 2024)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1975 || n = 1989)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1976 || n = 2000)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 1978)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1986)
	; 9 bytes
	pha
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	pla
.elseif(n = 1999)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pha
	inx
	Jne *-3
.elseif(n = 2013)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2015)
	; 8 bytes
	pha
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
	pla
.elseif(n = 2016)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2018)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 2023)
	; 8 bytes
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 2029)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop
	pha
	Jne *-3
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
; 2032 to 2109
.if(n = 2032 || n = 2034 || n = 2050 || n = 2052 || n = 2074 || n = 2088 || n = 2090 || n = 2108)
	; 7..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2033 || n = 2041)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2108)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2036 || n = 2047 || n = 2058 || n = 2102)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2037 || n = 2038 || n = 2043 || n = 2046 || n = 2051 || n = 2054 || n = 2055 || n = 2057 || n = 2060 || n = 2063 || n = 2064 || n = 2066 || n = 2067 || n = 2070 || n = 2073 || n = 2082 || n = 2083 || n = 2100 || n = 2103 || n = 2104 || n = 2106 || n = 2109)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2039)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2040 || n = 2048 || n = 2056)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2042 || n = 2068)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2045 || n = 2059)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2049)
	; 6 bytes
	ldx #0
	pha
	dex
	Jne *-2
.elseif(n = 2061 || n = 2081 || n = 2091)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2065 || n = 2077 || n = 2101)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2069)
	; 7 bytes
	ldx #<$8D ;hides 'sta $1AA2'
	ldx #26
	inx
	Jne *-4
.elseif(n = 2072)
	; 8 bytes
	pha
	ldx #(n-4)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2075 || n = 2092)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 2076)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $E6A2'
	ldx #230
	dex
	Jne *-4
	plp
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
.elseif(n = 2084 || n = 2093 || n = 2095)
	; 8..9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2085 || n = 2094 || n = 2096)
	; 8..9 bytes
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
.elseif(n = 2097)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2099)
	; 9 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	inx
	inx
	Jne *-6
.elseif(n = 2105)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2114 || n = 2119 || n = 2128 || n = 2138 || n = 2140 || n = 2142 || n = 2150 || n = 2155 || n = 2162 || n = 2164 || n = 2174 || n = 2182 || n = 2184)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2111 || n = 2121 || n = 2131 || n = 2151 || n = 2171 || n = 2181)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2112 || n = 2132 || n = 2156 || n = 2180 || n = 2186)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2113 || n = 2137 || n = 2149 || n = 2173 || n = 2185)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2115 || n = 2129)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2115 && n < 2180)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2117)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2118 || n = 2123 || n = 2127 || n = 2136 || n = 2145 || n = 2148 || n = 2153 || n = 2154 || n = 2163 || n = 2172 || n = 2175 || n = 2178 || n = 2183 || n = 2187)
	; 8..9 bytes
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
.elseif((n .mod 11 = 1) && ((n > 2123 && n < 2169)))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2126 || n = 2160 || n = 2177)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
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
.elseif(n = 2139)
	; 7 bytes
	ldx #234 ;hides 'nop'
	brk
	dex
	Jmi *-4
.elseif(n = 2141)
	; 7 bytes
	ldx #<$8D ;hides 'sta $12A2'
	ldx #18
	inx
	Jne *-4
.elseif(n = 2144)
	; 8 bytes
	pha
	ldx #214
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2147)
	; 7 bytes
	ldx #232 ;hides 'inx'
	pha
	inx
	inx
	Jne *-4
.elseif(n = 2158)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2159)
	; 6 bytes
	ldx #216 ;hides 'cld'
	pha
	dex
	Jne *-3
.elseif(n = 2165)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2166)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2167)
	; 9 bytes
	ldx #(n-20)/19
	delay_n 14
	dex
	Jpl *-5
.elseif(n = 2169)
	; 9 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2176)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if((n .mod 9 = 1) && ((n > 2187 && n < 2261)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2189 || n = 2192 || n = 2193 || n = 2203 || n = 2208 || n = 2210 || n = 2214 || n = 2217 || n = 2219 || n = 2225 || n = 2226 || n = 2229 || n = 2243 || n = 2244 || n = 2246 || n = 2247 || n = 2253 || n = 2258 || n = 2259 || n = 2262)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2190 || n = 2212 || n = 2223 || n = 2234 || n = 2256)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2191 || n = 2201 || n = 2211 || n = 2241 || n = 2261)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2194)
	; 8 bytes
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 2195 || n = 2222 || n = 2238 || n = 2240 || n = 2249 || n = 2252)
	; 8..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2196)
	; 9 bytes
	pha
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
.elseif(n = 2198 || n = 2250 || n = 2263)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2199)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2200 || n = 2202 || n = 2204 || n = 2216 || n = 2218 || n = 2235 || n = 2248 || n = 2254 || n = 2264)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2205)
	; 9 bytes
	ldx #(n-20)/19
	delay_n 14
	dex
	Jpl *-5
.elseif(n = 2207)
	; 9 bytes
	php
	ldx #36 ;hides 'bit zp'
	delay_n 4
	inx
	Jne *-4
	plp
.elseif(n = 2209 || n = 2221 || n = 2245 || n = 2257)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2213)
	; 7 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	inx
	Jne *-4
.elseif(n = 2220)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jne *-4
	plp
.elseif(n = 2227 || n = 2255)
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
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
.elseif(n = 2236)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2237)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2239)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
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
; 2266 to 2343
.if(n = 2266)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2267 || n = 2289 || n = 2300 || n = 2322 || n = 2333)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2268 || n = 2273 || n = 2280 || n = 2282 || n = 2295 || n = 2298 || n = 2303 || n = 2307 || n = 2309 || n = 2316 || n = 2324 || n = 2327 || n = 2335 || n = 2343)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 1) && ((n > 2268 && n < 2297)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2270 || n = 2272 || n = 2274 || n = 2284 || n = 2288 || n = 2290 || n = 2292 || n = 2294 || n = 2299 || n = 2304 || n = 2308 || n = 2314 || n = 2320 || n = 2332 || n = 2334 || n = 2336 || n = 2342)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 10 = 1) && (n = 2271 || (n > 2290 && n < 2332)))
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2275)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	inx
	Jne *-4
	plp
.elseif(n = 2276 || n = 2302 || n = 2315 || n = 2328)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2277)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 2279 || n = 2313 || n = 2330)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2281 || n = 2293 || n = 2317 || n = 2329 || n = 2341)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2283 || n = 2297 || n = 2325)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2285)
	; 7 bytes
	ldx #<$8D ;hides 'sta $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2286)
	; 9 bytes
	php
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
	plp
.elseif(n = 2305)
	; 7 bytes
	ldx #0
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2306 || n = 2318)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 2310)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	dex
	Jne *-4
	plp
.elseif(n = 2312)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2319)
	; 6 bytes
	ldx #24 ;hides 'clc'
	pha
	inx
	Jne *-3
.elseif(n = 2323)
	; 7 bytes
	ldx #128
	brk
	dex
	Jpl *-3
.elseif(n = 2326)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	inx
	inx
	Jne *-5
.elseif(n = 2337)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
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
	; 6 bytes
	ldx #234 ;hides 'nop'
	pha
	dex
	Jne *-3
.elseif(n = 2340)
	; 9 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.else
; 2344 to 2500
.if((n .mod 11 = 1) && ((n > 2343 && n < 2367) || (n > 2387 && n < 2411) || (n > 2431 && n < 2499)))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2345 || n = 2350 || n = 2387)
	; 8..9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2346 || n = 2357 || n = 2363 || n = 2368 || n = 2370 || n = 2373 || n = 2379 || n = 2383 || n = 2390 || n = 2397 || n = 2400 || n = 2403 || n = 2405 || n = 2412 || n = 2415 || n = 2417 || n = 2418 || n = 2427 || n = 2429 || n = 2430 || n = 2433 || n = 2434 || n = 2436 || n = 2439 || n = 2453 || n = 2456 || n = 2459 || n = 2463 || n = 2469 || n = 2475 || n = 2477 || n = 2478 || n = 2489 || n = 2495 || n = 2496 || n = 2499)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347 || n = 2364 || n = 2398 || n = 2466 || n = 2483 || n = 2500)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2348 || n = 2360 || n = 2372 || n = 2382 || n = 2396 || n = 2408 || n = 2420 || n = 2426 || n = 2442 || n = 2480)
	; 8..9 bytes
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
.elseif((n .mod 10 = 1) && ((n > 2350 && n < 2392) || n = 2411 || n = 2421 || n = 2441 || n = 2451 || (n > 2470 && n < 2492)))
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2352 || n = 2448 || n = 2462 || n = 2492)
	; 8..9 bytes
	brk
	delay_n (n-13)
.elseif((n .mod 12 = 1) && ((n > 2352 && n < 2498)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2354 || n = 2380 || n = 2393 || n = 2406 || n = 2445 || n = 2458 || n = 2484)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2356 || n = 2386 || n = 2416 || n = 2446)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2358 || n = 2362 || n = 2369 || n = 2374 || n = 2378 || n = 2384 || n = 2392 || n = 2394 || n = 2402 || n = 2404 || n = 2414 || n = 2422 || n = 2424 || n = 2428 || n = 2438 || n = 2440 || n = 2444 || n = 2452 || n = 2457 || n = 2464 || n = 2468 || n = 2470 || n = 2474 || n = 2482 || n = 2488 || n = 2490 || n = 2494)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2359 || n = 2395 || n = 2431 || n = 2467)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2367 || n = 2409 || n = 2423 || n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2375)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2376)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2385)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2407 || n = 2455)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2419)
	; 7 bytes
	ldx #248 ;hides 'sed'
	brk
	dex
	Jmi *-4
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2447)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 2450)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 2460)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 16
	dex
	Jmi *-5
.elseif(n = 2472 || n = 2486)
	; 8..9 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2479)
	; 6 bytes
	ldx #248 ;hides 'sed'
	pha
	dex
	Jne *-3
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
.if(n = 2501 || n = 2511 || n = 2531 || n = 2541)
	; 7 bytes
	ldx #(n-1)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2502 || n = 2505 || n = 2508 || n = 2513 || n = 2519 || n = 2522 || n = 2523 || n = 2525 || n = 2530 || n = 2538 || n = 2543 || n = 2544 || n = 2547 || n = 2552 || n = 2555 || n = 2559 || n = 2565 || n = 2570 || n = 2571)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2503 || n = 2539 || n = 2575)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2504 || n = 2512 || n = 2514 || n = 2524 || n = 2532 || n = 2548 || n = 2554 || n = 2556 || n = 2560 || n = 2567 || n = 2572 || n = 2576)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2506 || n = 2566)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && (n = 2507 || (n > 2534 && n < 2578)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 12 = 1) && ((n > 2508 && n < 2570)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2510 || n = 2536 || n = 2562)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2515 || n = 2546)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2516 || n = 2528 || n = 2558)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2517 || n = 2534 || n = 2568)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2518)
	; 9 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2520 || n = 2542 || n = 2553 || n = 2564)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2526)
	; 9 bytes
	ldx #100
	delay_n 7
	brk
	dex
	Jpl *-5
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
	ldx #<$8D ;hides 'sta $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
.elseif(n = 2537)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2540)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	dex
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
.elseif(n = 2561)
	; 7 bytes
	ldx #0
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2573)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2574)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 2578)
	; 8 bytes
	pha
	ldx #(n-4)/11
	delay_n 6
	dex
	Jne *-3
.endif
.else
; 2579 to 2656
.if(n = 2579 || n = 2583 || n = 2595 || n = 2598 || n = 2607 || n = 2609 || n = 2610 || n = 2613 || n = 2615 || n = 2616 || n = 2628 || n = 2631 || n = 2632 || n = 2634 || n = 2635 || n = 2643 || n = 2645 || n = 2646 || n = 2649 || n = 2651 || n = 2654 || n = 2655)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2580 || n = 2612 || n = 2618 || n = 2624 || n = 2648)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 12 = 1) && ((n > 2580 && n < 2654)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2582)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 2584 || n = 2589 || n = 2594 || n = 2600 || n = 2620 || n = 2622 || n = 2639 || n = 2644 || n = 2650)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2585 || n = 2602 || n = 2636)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2585 && n < 2631) || n = 2652))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2587)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2588 || n = 2614 || n = 2627 || n = 2640)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2590)
	; 9 bytes
	pha
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
.elseif(n = 2591)
	; 7 bytes
	ldx #216 ;hides 'cld'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2592)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2596 || n = 2626 || n = 2656)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2599)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-4
.elseif(n = 2601 || n = 2621)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2603)
	; 9 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
.elseif(n = 2604 || n = 2623 || n = 2642)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2606)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 2611 || n = 2647)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2625)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2633)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2637)
	; 9 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2638)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 6
	Jne *-4
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if(n = 2657 || n = 2662 || n = 2667 || n = 2669 || n = 2676 || n = 2679 || n = 2682 || n = 2691 || n = 2693 || n = 2694 || n = 2697 || n = 2698 || n = 2700 || n = 2706 || n = 2709 || n = 2711 || n = 2712 || n = 2715 || n = 2720 || n = 2723 || n = 2733)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2658 || n = 2724)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2659 || n = 2695)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2660 || n = 2672 || n = 2690 || n = 2708 || n = 2726 || n = 2734)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2661 || n = 2675 || n = 2703 || n = 2717 || n = 2731)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2662 && n < 2730)))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2664 || n = 2668 || n = 2680 || n = 2688 || n = 2699 || n = 2710 || n = 2722 || n = 2728 || n = 2730 || n = 2732)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 1) && ((n > 2664 && n < 2726)))
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
.elseif(n = 2670 || n = 2687 || n = 2704 || n = 2721)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2671 || n = 2686 || n = 2716)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2673)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2678 || n = 2702 || n = 2714)
	; 9 bytes
	brk
	delay_n (n-13)
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
	ldx #232 ;hides 'inx'
	delay_n 6
	inx
	inx
	Jne *-5
.elseif(n = 2705)
	; 8 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
.elseif(n = 2727)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jne *-4
.endif
.else
; 2735 to 2812
.if(n = 2735 || n = 2790)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2736)
	; 9 bytes
	delay_n (n-7)
	delay_n 7
.elseif((n .mod 12 = 1) && ((n > 2736 && n < 2810)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2738 || n = 2772 || n = 2789)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2739 || n = 2742 || n = 2747 || n = 2748 || n = 2753 || n = 2757 || n = 2763 || n = 2764 || n = 2775 || n = 2777 || n = 2778 || n = 2786 || n = 2803 || n = 2808 || n = 2811)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 2740 || n = 2751 || n = 2762 || n = 2784 || n = 2795 || n = 2806)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2741 || n = 2781)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2743 || n = 2752 || n = 2754 || n = 2758 || n = 2765 || n = 2774 || n = 2788 || n = 2798 || n = 2800 || n = 2802 || n = 2810 || n = 2812)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2744 || n = 2770 || n = 2796)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2745 || n = 2759 || n = 2787 || n = 2801)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2746 || n = 2776)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2750)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 2755 || n = 2791)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif((n .mod 12 = 8) && ((n > 2755 && n < 2805)))
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2760)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 2766)
	; 9 bytes
	pha
	ldx #138 ;hides 'txa'
	brk
	dex
	Jne *-4
	pla
.elseif(n = 2767 || n = 2779 || n = 2805)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2769)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2771)
	; 7 bytes
	ldx #232 ;hides 'inx'
	pha
	dex
	dex
	Jne *-4
.elseif(n = 2782)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 16
	dex
	Jmi *-5
.elseif(n = 2783)
	; 7 bytes
	ldx #24 ;hides 'clc'
	nop
	pha
	inx
	Jne *-4
.elseif(n = 2793)
	; 8 bytes
	ldx #<$8D ;hides 'sta $02A2'
	ldx #2
	nop
	inx
	Jne *-5
.elseif(n = 2794)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 2799)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
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
.else
.if((n >= 2813) && (n < 2969))
; 2813 to 2968
.if(n = 2813 || n = 2814 || n = 2819 || n = 2823 || n = 2825 || n = 2831 || n = 2835 || n = 2837 || n = 2838 || n = 2842 || n = 2847 || n = 2849 || n = 2850 || n = 2859 || n = 2862 || n = 2865 || n = 2867 || n = 2868 || n = 2880 || n = 2883 || n = 2886 || n = 2895 || n = 2897 || n = 2898 || n = 2904 || n = 2907 || n = 2909 || n = 2910 || n = 2915 || n = 2919 || n = 2922 || n = 2928 || n = 2931 || n = 2933 || n = 2937 || n = 2940 || n = 2943 || n = 2945 || n = 2950 || n = 2958 || n = 2963 || n = 2964 || n = 2967)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2815 || n = 2829 || n = 2843 || n = 2871 || n = 2885 || n = 2913 || n = 2927 || n = 2955)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2816 || n = 2828 || n = 2834 || n = 2852 || n = 2864 || n = 2876 || n = 2888 || n = 2892 || n = 2906 || n = 2912 || n = 2924 || n = 2936 || n = 2948 || n = 2960)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2817)
	; 7 bytes
	ldx #0
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2818 || n = 2820 || n = 2824 || n = 2830 || n = 2844 || n = 2860 || n = 2872 || n = 2879 || n = 2884 || n = 2890 || n = 2902 || n = 2914 || n = 2916 || n = 2920 || n = 2932 || n = 2938 || n = 2944 || n = 2968)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 1) && ((n > 2820 && n < 2966)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2822 || n = 2848 || n = 2887 || n = 2900 || n = 2926 || n = 2939 || n = 2952)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2826)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 2827 || n = 2863 || n = 2899 || n = 2935)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2832 || n = 2870 || n = 2889 || n = 2946)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2836 || n = 2851 || n = 2866 || n = 2896 || n = 2911 || n = 2956)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2839 || n = 2875 || n = 2923 || n = 2947)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2840 || n = 2874 || n = 2891 || n = 2908 || n = 2925 || n = 2942 || n = 2959)
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
.elseif(n = 2846 || n = 2854 || n = 2856 || n = 2858 || n = 2882 || n = 2894 || n = 2918 || n = 2930 || n = 2954 || n = 2966)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 2853)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 2855)
	; 8 bytes
	ldx #<$8D ;hides 'sta $12A2'
	ldx #18
	pha
	inx
	Jne *-5
.elseif(n = 2861 || n = 2901 || n = 2921 || n = 2961)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2873 || n = 2903 || n = 2957)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2877)
	; 9 bytes
	pha
	ldx #(n-4)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2878 || n = 2962)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 2934)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	pha
	inx
	inx
	Jne *-5
.elseif(n = 2949)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 2951)
	; 8 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	pha
	inx
	Jne *-5
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n .mod 14 = 1) && ((n > 2968 && n < 3012) || n = 3039))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2970 || n = 2973 || n = 2979 || n = 2982 || n = 2985 || n = 2987 || n = 2988 || n = 2991 || n = 2994 || n = 2998 || n = 3003 || n = 3005 || n = 3006 || n = 3009 || n = 3012 || n = 3027 || n = 3029 || n = 3033 || n = 3042 || n = 3045)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2971 || n = 3007 || n = 3043)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2972 || n = 2984 || n = 2996 || n = 3008 || n = 3014 || n = 3020 || n = 3032)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2974 || n = 2980 || n = 2992 || n = 3000 || n = 3018 || n = 3024 || n = 3026 || n = 3028 || n = 3034 || n = 3040)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2975)
	; 7 bytes
	ldx #248 ;hides 'sed'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2976 || n = 2993 || n = 3010)
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
.elseif(n = 2978 || n = 3004 || n = 3017 || n = 3030)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2981 || n = 3021)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2986 || n = 3016 || n = 3031 || n = 3046)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2990 || n = 3036 || n = 3038)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 2995 || n = 3019)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 2999)
	; 8 bytes
	ldx #56 ;hides 'sec'
	nop
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3002)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 3015)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-4
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
.elseif(n = 3035)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3041)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3044)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
.endif
.else
; 3047 to 3125
.if(n = 3047 || n = 3052 || n = 3064 || n = 3072 || n = 3076 || n = 3094 || n = 3100 || n = 3118)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3048 || n = 3051 || n = 3054 || n = 3055 || n = 3058 || n = 3063 || n = 3065 || n = 3066 || n = 3071 || n = 3075 || n = 3077 || n = 3083 || n = 3084 || n = 3090 || n = 3096 || n = 3099 || n = 3102 || n = 3107 || n = 3111 || n = 3113 || n = 3114 || n = 3117 || n = 3119 || n = 3120 || n = 3125)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3049 || n = 3061)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3050 || n = 3068 || n = 3080 || n = 3086 || n = 3104 || n = 3116 || n = 3122)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 14 = 1) && ((n > 3052 && n < 3124)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3056 || n = 3069 || n = 3082 || n = 3108)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3057 || n = 3089 || n = 3105)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3059 || n = 3103)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3060 || n = 3098)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3062 || n = 3074 || n = 3092 || n = 3110)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3070)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
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
.elseif(n = 3085 || n = 3087)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 3093)
	; 9 bytes
	ldx #<$8D ;hides 'sta $12A2'
	ldx #18
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3101 || n = 3121)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3124)
	; 9 bytes
	pha
	ldx #(n-4)/13
	nop
	delay_n 6
	dex
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3127 || n = 3131 || n = 3135 || n = 3138 || n = 3139 || n = 3142 || n = 3143 || n = 3148 || n = 3149 || n = 3153 || n = 3156 || n = 3159 || n = 3162 || n = 3167 || n = 3168 || n = 3171 || n = 3177 || n = 3178 || n = 3182 || n = 3183 || n = 3188 || n = 3189 || n = 3191 || n = 3192 || n = 3195 || n = 3198 || n = 3203)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3128 || n = 3164 || n = 3200)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3129 || n = 3146 || n = 3163 || n = 3180 || n = 3197)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3130 || n = 3172)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3132)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3133 || n = 3151 || n = 3169 || n = 3187)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif((n .mod 13 = 1) && ((n > 3133 && n < 3187)))
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3136 || n = 3166 || n = 3196)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3137 || n = 3165 || n = 3179 || n = 3193)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3140 || n = 3158 || n = 3170 || n = 3176 || n = 3194)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3141 || n = 3161 || n = 3181 || n = 3201)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3144 || n = 3150 || n = 3154 || n = 3190 || n = 3202)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3145)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 3152)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 3155 || n = 3174)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3157 || n = 3175)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3184)
	; 9 bytes
	pha
	ldx #(n-4)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3185)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
.endif
.else
; 3204 to 3281
.if(n = 3204 || n = 3208 || n = 3210 || n = 3215 || n = 3222 || n = 3224 || n = 3234 || n = 3242 || n = 3244 || n = 3250 || n = 3252 || n = 3262 || n = 3268 || n = 3274 || n = 3278 || n = 3280)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3205 || n = 3223 || n = 3241 || n = 3259 || n = 3277)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3206 || n = 3230 || n = 3254 || n = 3266 || n = 3270)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 14 = 1) && ((n > 3206 && n < 3264)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3209 || n = 3216 || n = 3225 || n = 3227 || n = 3228 || n = 3232 || n = 3237 || n = 3240 || n = 3243 || n = 3245 || n = 3246 || n = 3258 || n = 3267 || n = 3273 || n = 3279)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3211 || n = 3226 || n = 3256 || n = 3271)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3212 || n = 3238 || n = 3251 || n = 3264)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3213 || n = 3257)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3214 || n = 3231 || n = 3248 || n = 3265)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3217)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3218 || n = 3236 || n = 3260 || n = 3272)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3219)
	; 9 bytes
	ldx #<$8D ;hides 'sta $1AA2'
	ldx #26
	nop
	pha
	inx
	Jne *-6
.elseif(n = 3220 || n = 3229 || n = 3253 || n = 3255)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3233)
	; 8 bytes
	ldx #232 ;hides 'inx'
	nop
	pha
	dex
	dex
	Jne *-5
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
.elseif(n = 3261 || n = 3281)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3269)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3276)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
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
.elseif(n = 3283 || n = 3284 || n = 3292 || n = 3293 || n = 3297 || n = 3300 || n = 3310 || n = 3315 || n = 3317 || n = 3318 || n = 3328 || n = 3335 || n = 3342 || n = 3343 || n = 3348 || n = 3351 || n = 3353 || n = 3354 || n = 3358)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285 || n = 3289 || n = 3294 || n = 3306 || n = 3322 || n = 3324 || n = 3332 || n = 3334 || n = 3336 || n = 3352)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3286 || n = 3346)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3287)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3288 || n = 3307 || n = 3326)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3290 || n = 3303)
	; 8 bytes
	ldx #(n-1)/13
	nop
	delay_n 6
	dex
	Jne *-4
.elseif((n .mod 14 = 1) && ((n > 3290 && n < 3348)))
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
.elseif(n = 3296)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3298 || n = 3340)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3301 || n = 3321 || n = 3341)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3302 || n = 3312 || n = 3320 || n = 3338 || n = 3356 || n = 3357)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3304)
	; 9 bytes
	pha
	ldx #(n-4)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3308 || n = 3314 || n = 3344)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 3309)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3311 || n = 3325 || n = 3327 || n = 3337 || n = 3339 || n = 3355)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3323)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3329)
	; 8 bytes
	ldx #0
	nop
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3330)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3345)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3359)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.endif
.else
; 3360 to 3437
.if(n = 3360 || n = 3362 || n = 3398 || n = 3416 || n = 3434)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 3361 || n = 3375 || n = 3389 || n = 3417 || n = 3431)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3363 || n = 3366 || n = 3369 || n = 3371 || n = 3372 || n = 3378 || n = 3386 || n = 3387 || n = 3390 || n = 3395 || n = 3405 || n = 3407 || n = 3408 || n = 3413 || n = 3419 || n = 3420 || n = 3423 || n = 3429 || n = 3433 || n = 3437)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3364 || n = 3383 || n = 3402)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
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
.elseif(n = 3368 || n = 3370 || n = 3379 || n = 3388 || n = 3394 || n = 3404)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3373 || n = 3427)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3374 || n = 3392 || n = 3396 || n = 3410 || n = 3428)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3376 || n = 3391 || n = 3406 || n = 3436)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif((n .mod 16 = 1) && ((n > 3376 && n < 3426)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3380)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3381)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3382 || n = 3424)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3384 || n = 3401 || n = 3418 || n = 3435)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3397 || n = 3415)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 10
	inx
	Jne *-5
.elseif(n = 3400)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	inx
	inx
	Jne *-6
.elseif(n = 3411)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
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
.elseif(n = 3426)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
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
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if(n = 3438 || n = 3444 || n = 3447 || n = 3454 || n = 3463 || n = 3468 || n = 3477 || n = 3480 || n = 3483 || n = 3485 || n = 3488 || n = 3495 || n = 3498 || n = 3507 || n = 3510 || n = 3513)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3439 || n = 3457 || n = 3475 || n = 3493 || n = 3511)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3440 || n = 3497)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3441 || n = 3461 || n = 3481)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3442 || n = 3448 || n = 3453 || n = 3458 || n = 3460 || n = 3462 || n = 3467 || n = 3472 || n = 3474 || n = 3478 || n = 3490 || n = 3512 || n = 3514)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3443 || n = 3491)
	; 9..10 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 14 = 1) && ((n > 3444 && n < 3516)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3446 || n = 3482 || n = 3494 || n = 3500)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3449)
	; 9 bytes
	ldx #<$8D ;hides 'sta $1AA2'
	ldx #26
	delay_n 6
	inx
	Jne *-6
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
.elseif(n = 3456 || n = 3465 || n = 3492)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3464)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-5
.elseif(n = 3470 || n = 3484 || n = 3506)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3471)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3476)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3479)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3489 || n = 3505)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3499)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3502)
	; 10 bytes
	php
	ldx #184 ;hides 'clv'
	jsr @nearby_rts
	dex
	Jne *-5
	plp
.elseif(n = 3504)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
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
.if(n = 3516 || n = 3535 || n = 3573)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3517 || n = 3519 || n = 3522 || n = 3523 || n = 3525 || n = 3528 || n = 3531 || n = 3533 || n = 3534 || n = 3538 || n = 3539 || n = 3545 || n = 3549 || n = 3551 || n = 3552 || n = 3558 || n = 3559 || n = 3562 || n = 3563 || n = 3567 || n = 3570 || n = 3580 || n = 3587 || n = 3590)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3518 || n = 3527 || n = 3536 || n = 3548 || n = 3572)
	; 9..10 bytes
	php
	delay_n (n-7)
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
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3524 || n = 3542 || n = 3560 || n = 3578)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3526 || n = 3556 || n = 3586)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3529 || n = 3547 || n = 3565 || n = 3583)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3530)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 26
	dex
	Jmi *-6
.elseif(n = 3532 || n = 3540 || n = 3546 || n = 3564 || n = 3568 || n = 3574 || n = 3584 || n = 3591)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3543 || n = 3557 || n = 3571)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3544)
	; 9 bytes
	pha
	ldx #(n-4)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3550 || n = 3592)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3553 || n = 3569)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3555)
	; 9 bytes
	ldx #<$8D ;hides 'sta $02A2'
	ldx #2
	nop
	pha
	inx
	Jne *-6
.elseif(n = 3566)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3575)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3576)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3577)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 3579)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
.elseif(n = 3582)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3589)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3593)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
; 3594 to 3671
.if(n = 3594 || n = 3603 || n = 3606 || n = 3607 || n = 3610 || n = 3615 || n = 3618 || n = 3620 || n = 3624 || n = 3636 || n = 3643 || n = 3648 || n = 3652 || n = 3657 || n = 3663 || n = 3667 || n = 3670)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3600 || n = 3602 || n = 3604 || n = 3640 || n = 3642 || n = 3656 || n = 3658)
	; 7..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3596 || n = 3614 || n = 3632 || n = 3650 || n = 3654 || n = 3666)
	; 8..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 3597)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3598)
	; 10 bytes
	ldx #108
	nop
	delay_n 26
	dex
	Jpl *-6
.elseif(n = 3599)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 3601 || n = 3619 || n = 3637 || n = 3655)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3605 || n = 3622 || n = 3639)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3608 || n = 3626 || n = 3629 || n = 3644 || n = 3660 || n = 3662)
	; 8..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3609)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3611 || n = 3630 || n = 3668)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3612 || n = 3635)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 3613 || n = 3634)
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
.elseif((n .mod 16 = 1) && ((n > 3616 && n < 3666)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3621 || n = 3641 || n = 3661)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3623)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3625 || n = 3627 || n = 3645 || n = 3647 || n = 3659)
	; 8..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3628)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 3638)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 3651)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3653)
	; 6 bytes
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
.elseif(n = 3664)
	; 9 bytes
	pha
	ldx #(n-4)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3669)
	; 9 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3671)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 10
	dex
	Jne *-5
.endif
.else
; 3672 to 3750
.if(n = 3672 || n = 3704 || n = 3722 || n = 3737 || n = 3740)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 3673 || n = 3691 || n = 3709 || n = 3727 || n = 3745)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3674 || n = 3684 || n = 3694 || n = 3700 || n = 3710 || n = 3712 || n = 3714 || n = 3730 || n = 3742 || n = 3746 || n = 3748)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 3675 || n = 3677 || n = 3678 || n = 3682 || n = 3683 || n = 3685 || n = 3692 || n = 3693 || n = 3696 || n = 3699 || n = 3703 || n = 3705 || n = 3708 || n = 3720 || n = 3723 || n = 3726 || n = 3728 || n = 3729 || n = 3731 || n = 3732 || n = 3735 || n = 3738 || n = 3747 || n = 3749)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3676 || n = 3706 || n = 3736)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3679)
	; 9 bytes
	ldx #<$8D ;hides 'sta $1AA2'
	ldx #26
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3680 || n = 3688 || n = 3698 || n = 3702 || n = 3716 || n = 3734)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3681 || n = 3701 || n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3686)
	; 9 bytes
	php
	ldx #184 ;hides 'clv'
	brk
	dex
	Jne *-4
	plp
.elseif(n = 3687 || n = 3725 || n = 3744)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3689)
	; 9 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	delay_n 6
	inx
	Jne *-6
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
.elseif(n = 3697 || n = 3718 || n = 3739)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3711)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3713)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3715 || n = 3717 || n = 3733)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3719)
	; 8 bytes
	ldx #248 ;hides 'sed'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3743)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3750)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
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
.elseif(n = 3752 || n = 3770 || n = 3788 || n = 3806 || n = 3808 || n = 3816 || n = 3824 || n = 3833 || n = 3842 || n = 3850 || n = 3878)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3753 || n = 3755 || n = 3762 || n = 3765 || n = 3767 || n = 3768 || n = 3771 || n = 3779 || n = 3783 || n = 3785 || n = 3786 || n = 3789 || n = 3790 || n = 3794 || n = 3797 || n = 3798 || n = 3800 || n = 3804 || n = 3813 || n = 3819 || n = 3822 || n = 3827 || n = 3828 || n = 3831 || n = 3837 || n = 3840 || n = 3845 || n = 3855 || n = 3862 || n = 3863 || n = 3867 || n = 3870 || n = 3873 || n = 3875 || n = 3879 || n = 3882 || n = 3883 || n = 3885 || n = 3891 || n = 3893 || n = 3898 || n = 3903 || n = 3906)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3754 || n = 3764 || n = 3778 || n = 3780 || n = 3784 || n = 3791 || n = 3795 || n = 3810 || n = 3820 || n = 3838 || n = 3846 || n = 3847 || n = 3864 || n = 3868 || n = 3874 || n = 3880 || n = 3892 || n = 3897 || n = 3899 || n = 3904)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3756 || n = 3774 || n = 3776 || n = 3812 || n = 3830 || n = 3834 || n = 3836 || n = 3848 || n = 3854 || n = 3866 || n = 3869 || n = 3884 || n = 3890 || n = 3902)
	; 9..10 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3758 || n = 3775 || n = 3792 || n = 3809 || n = 3826 || n = 3843 || n = 3860 || n = 3877 || n = 3894)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3759 || n = 3852)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3760 || n = 3802 || n = 3823 || n = 3844 || n = 3865 || n = 3886)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3761 || n = 3801 || n = 3821 || n = 3841 || n = 3861 || n = 3881 || n = 3901)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3763 || n = 3781 || n = 3799 || n = 3817 || n = 3835 || n = 3853 || n = 3871 || n = 3889)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3769 || n = 3787 || n = 3805 || n = 3815 || n = 3832 || n = 3849 || n = 3859 || n = 3900)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
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
.elseif(n = 3773 || n = 3888)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 3777 || n = 3793 || n = 3825 || n = 3857 || n = 3905)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3782 || n = 3839 || n = 3858 || n = 3896)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3803)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3807)
	; 9 bytes
	ldx #<$8D ;hides 'sta $12A2'
	ldx #18
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3814)
	; 9 bytes
	ldx #(n-32)/31
	delay_n 26
	dex
	Jpl *-5
.elseif(n = 3818 || n = 3872)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 3829 || n = 3851 || n = 3895)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3856)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
.elseif(n = 3876)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 3887)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 11
	dex
	Jne *-6
.endif
.else
; 3907 to 4062
.if(n = 3907 || n = 3925 || n = 3943 || n = 3961 || n = 3979 || n = 3997 || n = 4015 || n = 4033 || n = 4051)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3908 || n = 3914 || n = 3918 || n = 3932 || n = 3950 || n = 3968 || n = 3986 || n = 3988 || n = 4004 || n = 4006 || n = 4008 || n = 4022 || n = 4040 || n = 4058)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3909 || n = 3912 || n = 3913 || n = 3916 || n = 3919 || n = 3922 || n = 3923 || n = 3927 || n = 3929 || n = 3930 || n = 3933 || n = 3936 || n = 3947 || n = 3955 || n = 3958 || n = 3963 || n = 3965 || n = 3966 || n = 3971 || n = 3978 || n = 3984 || n = 3987 || n = 3993 || n = 3998 || n = 4003 || n = 4007 || n = 4014 || n = 4017 || n = 4019 || n = 4020 || n = 4023 || n = 4025 || n = 4032 || n = 4034 || n = 4035 || n = 4037 || n = 4038 || n = 4042 || n = 4043 || n = 4045 || n = 4053 || n = 4055 || n = 4056 || n = 4060)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3924 || n = 3940 || n = 3942 || n = 3944 || n = 3946 || n = 3948 || n = 3952 || n = 3960 || n = 3964 || n = 3973 || n = 3980 || n = 3982 || n = 3989 || n = 3994 || n = 4000 || n = 4002 || n = 4016 || n = 4018 || n = 4024 || n = 4036 || n = 4044 || n = 4050 || n = 4052 || n = 4062)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3915 || n = 3934 || n = 3972 || n = 4010 || n = 4029 || n = 4048)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3917 || n = 3939 || n = 4005 || n = 4027)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 3920 || n = 3954 || n = 3956 || n = 3974 || n = 3975 || n = 3990 || n = 3992 || n = 4028 || n = 4046)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 3921 || n = 3941 || n = 3981 || n = 4001 || n = 4021 || n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3926 || n = 3951 || n = 3976 || n = 4026)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 3931 || n = 4039 || n = 4057)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3935)
	; 9 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	delay_n 7
	inx
	Jne *-6
.elseif(n = 3937)
	; 8 bytes
	ldx #232 ;hides 'inx'
	brk
	inx
	inx
	Jne *-5
.elseif(n = 3938 || n = 4031)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 3949 || n = 3970 || n = 3991 || n = 4012 || n = 4054)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3953 || n = 3969 || n = 3985 || n = 4049)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3957)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 3959 || n = 3995)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3967)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3977)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3983)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3999)
	; 7 bytes
	ldx #56 ;hides 'sec'
	brk
	inx
	Jne *-4
.elseif(n = 4009)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4011)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
.if((n >= 4063) && (n < 4141))
; 4063 to 4140
.if(n = 4063)
	; 9 bytes
	ldx #<$8D ;hides 'sta $02A2'
	ldx #2
	delay_n 7
	inx
	Jne *-6
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132)
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
.elseif(n = 4066 || n = 4068 || n = 4071 || n = 4073 || n = 4074 || n = 4077 || n = 4080 || n = 4083 || n = 4085 || n = 4088 || n = 4089 || n = 4091 || n = 4092 || n = 4100 || n = 4102 || n = 4107 || n = 4109 || n = 4110 || n = 4113 || n = 4114 || n = 4119 || n = 4125 || n = 4127 || n = 4128 || n = 4131 || n = 4134 || n = 4140)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4067 || n = 4086 || n = 4124)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4069 || n = 4087 || n = 4105 || n = 4123)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4070 || n = 4111 || n = 4129)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4072 || n = 4078 || n = 4079 || n = 4084 || n = 4090 || n = 4099 || n = 4104 || n = 4106 || n = 4108 || n = 4120 || n = 4126 || n = 4135)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4075 || n = 4096 || n = 4117 || n = 4138)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4076 || n = 4094 || n = 4112 || n = 4122 || n = 4130)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4082 || n = 4118 || n = 4136)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4093)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4095)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4097)
	; 9 bytes
	ldx #0
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4101 || n = 4121)
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
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
.elseif(n = 4133)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
	plp
.elseif(n = 4137)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4139)
	; 10 bytes
	ldx #<$8D ;hides 'sta $1AA2'
	ldx #26
	delay_n 9
	inx
	Jne *-7
.endif
.else
; 4141 to 4218
.if(n = 4141 || n = 4159 || n = 4177 || n = 4195 || n = 4213)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4142 || n = 4143 || n = 4145 || n = 4146 || n = 4150 || n = 4151 || n = 4168 || n = 4170 || n = 4174 || n = 4178 || n = 4179 || n = 4182 || n = 4185 || n = 4197 || n = 4202 || n = 4212 || n = 4215)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4144 || n = 4152 || n = 4158 || n = 4160 || n = 4162 || n = 4169 || n = 4198 || n = 4204 || n = 4216)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4147 || n = 4165 || n = 4167 || n = 4189 || n = 4206)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4148 || n = 4156 || n = 4173 || n = 4184 || n = 4188 || n = 4207)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4153)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4154 || n = 4172 || n = 4190 || n = 4194 || n = 4196 || n = 4208)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4155 || n = 4186)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4157)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
.elseif(n = 4161 || n = 4181 || n = 4201)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4163 || n = 4205)
	; 9..10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4164 || n = 4187 || n = 4210)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4171)
	; 10 bytes
	ldx #139
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 4175)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 4176)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4180)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4191 || n = 4209)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4192)
	; 10 bytes
	ldx #127
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 4193)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4199)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 4203)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4211)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4214)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop
	brk
	inx
	inx
	Jne *-6
.elseif(n = 4218)
	; 9 bytes
	pha
	ldx #248 ;hides 'sed'
	delay_n 10
	dex
	Jne *-5
.endif
.endif
.else
; 4219 to 4375
.if(n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295 || n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4220 || n = 4228 || n = 4256 || n = 4258 || n = 4274 || n = 4292 || n = 4310 || n = 4326 || n = 4328 || n = 4346 || n = 4364 || n = 4368)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4221 || n = 4241 || n = 4261 || n = 4281 || n = 4301 || n = 4341 || n = 4361)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4222 || n = 4243 || n = 4264 || n = 4306 || n = 4327 || n = 4348 || n = 4369)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4223 || n = 4224 || n = 4227 || n = 4233 || n = 4235 || n = 4236 || n = 4239 || n = 4240 || n = 4245 || n = 4250 || n = 4253 || n = 4259 || n = 4263 || n = 4266 || n = 4269 || n = 4271 || n = 4272 || n = 4278 || n = 4282 || n = 4283 || n = 4286 || n = 4287 || n = 4289 || n = 4290 || n = 4293 || n = 4294 || n = 4299 || n = 4300 || n = 4304 || n = 4305 || n = 4307 || n = 4308 || n = 4311 || n = 4312 || n = 4315 || n = 4318 || n = 4323 || n = 4325 || n = 4329 || n = 4337 || n = 4338 || n = 4340 || n = 4344 || n = 4350 || n = 4354 || n = 4355 || n = 4358 || n = 4359 || n = 4362 || n = 4363 || n = 4365 || n = 4366 || n = 4373 || n = 4374)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4225 || n = 4247 || n = 4291 || n = 4313 || n = 4335)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4226 || n = 4230 || n = 4244 || n = 4275 || n = 4280 || n = 4298 || n = 4316 || n = 4332 || n = 4334 || n = 4349 || n = 4370)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4229)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4231 || n = 4249 || n = 4267 || n = 4285 || n = 4303 || n = 4321 || n = 4339 || n = 4357 || n = 4375)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4232 || n = 4242 || n = 4296 || n = 4320)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4234 || n = 4251 || n = 4268 || n = 4302 || n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4237 || n = 4255 || n = 4273 || n = 4309 || n = 4345 || n = 4347 || n = 4367)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4246 || n = 4252 || n = 4254 || n = 4260 || n = 4265 || n = 4270 || n = 4277 || n = 4284 || n = 4288 || n = 4317 || n = 4322 || n = 4324 || n = 4330 || n = 4342 || n = 4356 || n = 4360)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4248 || n = 4279 || n = 4372)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4262)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-4
.elseif(n = 4297)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4319)
	; 7 bytes
	ldx #216 ;hides 'cld'
	brk
	dex
	Jne *-4
.elseif(n = 4331)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4343)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4351)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4353)
	; 8 bytes
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
; 4376 to 4453
.if(n = 4376 || n = 4426)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4377 || n = 4379 || n = 4380 || n = 4383 || n = 4385 || n = 4386 || n = 4392 || n = 4395 || n = 4397 || n = 4398 || n = 4405 || n = 4413 || n = 4415 || n = 4416 || n = 4419 || n = 4420 || n = 4430 || n = 4431 || n = 4433 || n = 4437 || n = 4438 || n = 4443 || n = 4446 || n = 4449 || n = 4451 || n = 4452)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4378 || n = 4384 || n = 4396 || n = 4402 || n = 4404 || n = 4410 || n = 4414 || n = 4439 || n = 4444 || n = 4448 || n = 4450)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4381 || n = 4401 || n = 4421 || n = 4441)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4382 || n = 4400 || n = 4408 || n = 4418 || n = 4436)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4387 || n = 4417 || n = 4435)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4388 || n = 4406 || n = 4412 || n = 4424 || n = 4442)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4389 || n = 4422)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4390 || n = 4432 || n = 4453)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4391)
	; 10 bytes
	pha
	ldx #232 ;hides 'inx'
	delay_n 10
	dex
	dex
	Jne *-6
.elseif(n = 4393 || n = 4411 || n = 4429 || n = 4447)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4394 || n = 4440)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4403 || n = 4434)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4407)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4409 || n = 4428)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4423)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4425)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4427)
	; 10 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4445)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.endif
.else
; 4454 to 4531
.if(n = 4454 || n = 4472 || n = 4490 || n = 4508 || n = 4526 || n = 4528)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4455 || n = 4462 || n = 4467 || n = 4469 || n = 4470 || n = 4473 || n = 4479 || n = 4480 || n = 4485 || n = 4487 || n = 4488 || n = 4491 || n = 4492 || n = 4497 || n = 4500 || n = 4503 || n = 4505 || n = 4506 || n = 4510 || n = 4515 || n = 4518 || n = 4520 || n = 4524 || n = 4529)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4456 || n = 4457 || n = 4464 || n = 4468 || n = 4475 || n = 4477 || n = 4484 || n = 4486 || n = 4493 || n = 4498 || n = 4499 || n = 4504 || n = 4512 || n = 4522 || n = 4530)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4458 || n = 4460 || n = 4478 || n = 4494 || n = 4514)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4459)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4461 || n = 4481 || n = 4521)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4463)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4465 || n = 4483 || n = 4501 || n = 4519)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4466 || n = 4523)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4471 || n = 4507 || n = 4525)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4474 || n = 4495 || n = 4516)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4476)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4482)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4489 || n = 4511)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4496 || n = 4527)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4502)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4509)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4513)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4517)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4531)
	; 10 bytes
	ldx #151
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if(n = 4532 || n = 4550 || n = 4554 || n = 4568 || n = 4574)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4533)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4534 || n = 4539 || n = 4543 || n = 4545 || n = 4546 || n = 4552 || n = 4553 || n = 4557 || n = 4559 || n = 4560 || n = 4563 || n = 4565 || n = 4566 || n = 4570 || n = 4575 || n = 4577 || n = 4582 || n = 4584 || n = 4585 || n = 4588 || n = 4589 || n = 4593 || n = 4595 || n = 4596 || n = 4600 || n = 4602 || n = 4603 || n = 4606 || n = 4607)
	; 7..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4535)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 4536 || n = 4538 || n = 4540 || n = 4564 || n = 4576 || n = 4583 || n = 4590 || n = 4594 || n = 4601 || n = 4608)
	; 7..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4537 || n = 4555 || n = 4573 || n = 4591)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4541 || n = 4561 || n = 4581)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4542)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4544 || n = 4548 || n = 4562 || n = 4587 || n = 4605)
	; 8..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4547 || n = 4567 || n = 4586 || n = 4597)
	; 8..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4549 || n = 4592)
	; 9..10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4551)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4556 || n = 4569 || n = 4572)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4558 || n = 4579)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4571)
	; 10 bytes
	ldx #<$8D ;hides 'sta $02A2'
	ldx #2
	delay_n 9
	inx
	Jne *-7
.elseif(n = 4578)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4580)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 4598)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
.elseif(n = 4599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 16
	inx
	Jne *-5
.elseif(n = 4604)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	inx
	Jne *-3
	sta (@ptrtemp,x)
.elseif(n = 4609)
	; 7 bytes
	ldx #0
	brk
	dex
	Jne *-3
.endif
.else
; 4610 to 4687
.if(n = 4610)
	; 9 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4611 || n = 4613 || n = 4614 || n = 4617 || n = 4623 || n = 4625 || n = 4635 || n = 4644 || n = 4653 || n = 4658 || n = 4667 || n = 4677 || n = 4678 || n = 4683)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4636 || n = 4640 || n = 4649 || n = 4650 || n = 4659 || n = 4660 || n = 4664 || n = 4666 || n = 4671 || n = 4682)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4615 || n = 4645 || n = 4685)
	; 9 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4616 || n = 4628 || n = 4646 || n = 4648 || n = 4686)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4618 || n = 4637 || n = 4656 || n = 4675)
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
.elseif(n = 4620 || n = 4651)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4621 || n = 4641 || n = 4661 || n = 4681)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4622 || n = 4634 || n = 4652 || n = 4655 || n = 4674)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4624 || n = 4647 || n = 4670)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4626 || n = 4676)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4627)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	brk
	inx
	Jne *-5
	plp
.elseif(n = 4629)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif((n > 4629 && n < 4633) || n = 4638 || n = 4662 || n = 4680)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4633 || n = 4657)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4639)
	; 7 bytes
	ldx #24 ;hides 'clc'
	brk
	inx
	Jne *-4
.elseif(n = 4642 || n = 4663 || n = 4684)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4643 || n = 4665 || n = 4687)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4654)
	; 10 bytes
	ldx #141
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 4668)
	; 8 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 16
	Jne *-5
.elseif(n = 4669)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4672)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 4673)
	; 10 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	delay_n 10
	inx
	Jne *-7
.elseif(n = 4679)
	; 7 bytes
	ldx #234 ;hides 'nop'
	brk
	dex
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
; 4688 to 4765
.if(n = 4688 || n = 4718 || n = 4748 || n = 4758)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4689 || n = 4695 || n = 4696 || n = 4703 || n = 4706 || n = 4707 || n = 4710 || n = 4715 || n = 4723 || n = 4725 || n = 4728 || n = 4730 || n = 4733 || n = 4734 || n = 4743 || n = 4745 || n = 4746 || n = 4749 || n = 4752 || n = 4755 || n = 4763 || n = 4765)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4690)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4691)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4692 || n = 4760)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4693 || n = 4716 || n = 4739 || n = 4762)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4694 || n = 4713 || n = 4732)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4697 || n = 4704 || n = 4708 || n = 4712 || n = 4714 || n = 4719 || n = 4724 || n = 4735 || n = 4742 || n = 4750 || n = 4754 || n = 4756 || n = 4764)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4698 || n = 4700 || n = 4702 || (n > 4735 && n < 4739) || n = 4740)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4699)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 4701 || n = 4721 || n = 4741 || n = 4761)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4705 || n = 4726 || n = 4747)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4709 || n = 4731 || n = 4753)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4711)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4717 || n = 4727 || n = 4757)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4720)
	; 10 bytes
	ldx #143
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 4722)
	; 10 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 4729)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4744)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4751)
	; 8 bytes
	ldx #216 ;hides 'cld'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4759)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.endif
.else
; 4766 to 4843
.if(n = 4766 || n = 4772 || n = 4778 || n = 4779 || n = 4783 || n = 4791 || n = 4793 || n = 4803 || n = 4805 || n = 4812 || n = 4815 || n = 4823 || n = 4829 || n = 4833 || n = 4835 || n = 4836 || n = 4839 || n = 4843)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4767 || n = 4787)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4768 || n = 4789 || n = 4810 || n = 4831)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4769 || n = 4782 || n = 4790 || n = 4798 || n = 4818 || n = 4820 || n = 4842)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4770 || n = 4808 || n = 4827)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4771 || n = 4773 || n = 4784 || n = 4792 || n = 4800 || n = 4804 || n = 4809 || n = 4811 || n = 4813 || n = 4822 || n = 4824 || n = 4830 || n = 4834 || n = 4840)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4774)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	brk
	dex
	Jne *-5
	plp
.elseif(n = 4775 || n = 4806 || n = 4837)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4776 || n = 4826)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4777 || n = 4825)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4780 || n = 4807)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 4781 || n = 4801 || n = 4821 || n = 4841)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4785)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4786)
	; 10 bytes
	ldx #145
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 4788 || n = 4828)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4794 || n = 4802 || n = 4814)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4795)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	brk
	dex
	Jne *-5
	plp
.elseif(n = 4796 || n = 4838)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4797 || n = 4819)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4799)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	brk
	inx
	Jne *-6
.elseif(n = 4816)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	brk
	dex
	Jne *-5
	plp
.elseif(n = 4817)
	; 10 bytes
	ldx #172
	delay_n 23
	dex
	Jne *-6
.elseif(n = 4832)
	; 10 bytes
	ldx #232 ;hides 'inx'
	nop
	delay_n 16
	inx
	inx
	Jne *-7
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
; 4844 to 4921
.if(n = 4844 || n = 4855 || n = 4864 || n = 4874 || n = 4880 || n = 4884 || n = 4902 || n = 4904 || n = 4910 || n = 4916 || n = 4918)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4845 || n = 4848 || n = 4853 || n = 4857 || n = 4866 || n = 4867 || n = 4870 || n = 4875 || n = 4878 || n = 4883 || n = 4887 || n = 4896 || n = 4898 || n = 4903 || n = 4905 || n = 4906 || n = 4909 || n = 4917 || n = 4919 || n = 4920)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4846)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4847)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4849 || n = 4897)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4850)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
.elseif(n = 4851 || n = 4876)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4852 || n = 4873 || n = 4894 || n = 4915)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4854 || n = 4877 || n = 4900)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 4856)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4858 || n = 4879)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4859)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4860 || n = 4862 || n = 4869 || n = 4872 || n = 4882 || n = 4890 || n = 4892 || n = 4893 || n = 4895 || n = 4911 || n = 4912)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4861 || n = 4881 || n = 4901 || n = 4921)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4863 || n = 4885 || n = 4907)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4865)
	; 9 bytes
	ldx #0
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4868 || n = 4899)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4871)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 19
	Jne *-5
.elseif(n = 4886 || n = 4914)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4888 || n = 4908)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4889)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 4891)
	; 10 bytes
	ldx #163
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 4913)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 14
	dex
	Jne *-6
.endif
.else
; 4922 to 5000
.if(n = 4922)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4923 || n = 4925 || n = 4931 || n = 4932 || n = 4938 || n = 4940 || n = 4943 || n = 4953 || n = 4963 || n = 4975 || n = 4980 || n = 4982 || n = 4983 || n = 4985 || n = 4986 || n = 4994)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4924 || n = 4933 || n = 4939 || n = 4944 || n = 4960 || n = 4962 || n = 4964 || n = 4970 || n = 4979 || n = 4984 || n = 4998)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4926 || n = 4976)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 4927 || n = 4947 || n = 4965 || n = 4987)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4928 || n = 4948 || n = 4966 || n = 4968 || n = 4988)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4929 || n = 4951 || n = 4973 || n = 4995)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 4930 || n = 4992)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 4934 || n = 4949 || n = 4954 || n = 4972 || n = 4974 || n = 4991)
	; 9..10 bytes
	brk
	delay_n (n-13)
.elseif(n = 4935 || n = 4950 || n = 4952 || n = 4955 || n = 4956 || n = 4958 || n = 4977 || n = 4989 || n = 4990 || n = 5000)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4936 || n = 4957 || n = 4978 || n = 4999)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4937)
	; 10 bytes
	pha
	ldx #9 ;hides 'ora #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
	pla
.elseif(n = 4941 || n = 4961 || n = 4981)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4942 || n = 4996)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 4945 || n = 4993)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 4946 || n = 4969)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
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
.elseif(n = 4971)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4997)
	; 10 bytes
	ldx #<$8D ;hides 'sta $12A2'
	ldx #18
	jsr @nearby_rts
	inx
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
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5002 || n = 5004 || n = 5018 || n = 5024 || n = 5029 || n = 5042 || n = 5044 || n = 5057 || n = 5077)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5003 || n = 5005 || n = 5006 || n = 5019 || n = 5022 || n = 5025 || n = 5040 || n = 5043 || n = 5045 || n = 5046 || n = 5053 || n = 5056 || n = 5063 || n = 5064 || n = 5066 || n = 5078)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5007 || n = 5027 || n = 5047 || n = 5067)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 5008 || n = 5028 || n = 5048 || n = 5068)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5009 || n = 5010 || n = 5016 || (n > 5029 && n < 5033) || (n > 5034 && n < 5038) || n = 5049 || n = 5052 || n = 5055 || n = 5058 || n = 5060 || n = 5070 || n = 5072 || n = 5073)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5011)
	; 10 bytes
	ldx #167
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5012 || n = 5014 || n = 5034 || n = 5075)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5013)
	; 10 bytes
	ldx #179
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5015 || n = 5038)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5017 || n = 5039)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5020 || n = 5062)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5023 || n = 5054)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5026 || n = 5051 || n = 5076)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5033)
	; 10 bytes
	ldx #148
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5050)
	; 10 bytes
	ldx #153
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5059)
	; 9 bytes
	ldx #<$8D ;hides 'sta $1AA2'
	ldx #26
	brk
	inx
	Jne *-6
.elseif(n = 5065)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5069)
	; 10 bytes
	ldx #181
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5071)
	; 10 bytes
	ldx #169
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5074)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
.endif
.else
; 5079 to 5156
.if(n = 5079 || n = 5086 || n = 5093 || n = 5100 || n = 5113 || n = 5120 || n = 5124 || n = 5128 || n = 5133 || n = 5140 || n = 5150 || n = 5156)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5080 || n = 5096 || n = 5109)
	; 9..10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 5081 || n = 5101)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5082 || n = 5136 || (n > 5140 && n < 5146))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5083 || n = 5104 || n = 5125 || n = 5146)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5084 || n = 5107 || n = 5130 || n = 5153)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5085)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5087 || n = 5091 || n = 5092 || n = 5094 || n = 5095 || n = 5098 || n = 5099 || n = 5106 || n = 5112 || n = 5114 || n = 5115 || n = 5118 || n = 5119 || n = 5123 || n = 5129 || n = 5132 || n = 5139 || n = 5148 || n = 5155)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5088 || n = 5097 || n = 5108 || n = 5117 || n = 5154)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5089 || n = 5137)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5090)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 5102 || n = 5122)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5103)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5105 || n = 5127 || n = 5149)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5110)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 5111)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5116)
	; 9 bytes
	php
	ldx #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	inx
	Jne *-4
	plp
.elseif(n = 5121)
	; 8 bytes
	ldx #0
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5126 || n = 5151)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5131)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5134 || n = 5138)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5135)
	; 10 bytes
	ldx #151
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5147)
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5152)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
; 5157 to 5234
.if(n = 5157 || n = 5162 || n = 5166 || n = 5168 || n = 5177 || n = 5184 || n = 5189 || n = 5197 || n = 5200 || n = 5205 || n = 5206 || n = 5208 || n = 5219 || n = 5220 || n = 5223 || n = 5231)
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
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5159 || n = 5160)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5161 || n = 5185 || n = 5233)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5163 || n = 5169 || n = 5172 || n = 5173 || n = 5180 || n = 5187 || n = 5190 || n = 5192 || n = 5195 || n = 5203 || n = 5211 || n = 5213 || n = 5214 || n = 5217 || n = 5224 || n = 5228 || n = 5232 || n = 5234)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5164 || n = 5170 || n = 5174 || n = 5179 || n = 5181 || n = 5186 || n = 5191 || n = 5196 || n = 5202 || n = 5204 || n = 5210 || n = 5212 || n = 5218 || n = 5225 || n = 5229)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5165)
	; 10 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
.elseif(n = 5167 || n = 5188 || n = 5209 || n = 5230)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5171 || n = 5193 || n = 5215)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5175 || n = 5227)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5176 || n = 5201 || n = 5226)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5178)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5182)
	; 10 bytes
	ldx #157
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5183)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 4
	brk
	dex
	Jne *-6
.elseif(n = 5194)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5198)
	; 10 bytes
	pha
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
	pla
.elseif(n = 5199 || n = 5222)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5207)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 14
	dex
	Jne *-6
.elseif(n = 5216)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5221)
	; 10 bytes
	ldx #174
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.endif
.else
; 5235 to 5312
.if(n = 5235)
	; 9 bytes
	ldx #<$8D ;hides 'sta $12A2'
	ldx #18
	brk
	inx
	Jne *-6
.elseif(n = 5236)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5237 || n = 5259 || n = 5281 || n = 5303)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5238 || n = 5243 || n = 5254 || n = 5260 || n = 5262 || n = 5275 || n = 5280 || n = 5284 || n = 5294 || n = 5296 || n = 5308)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5239 || n = 5266)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5240 || n = 5271 || n = 5302)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5241 || n = 5244 || n = 5246 || n = 5249 || n = 5250 || n = 5252 || n = 5263 || n = 5269 || n = 5282 || (n > 5285 && n < 5291) || n = 5292 || n = 5309 || n = 5310)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5242 || n = 5247 || n = 5253 || n = 5255 || n = 5256 || n = 5261 || n = 5270 || n = 5273 || n = 5274 || n = 5278 || n = 5283 || n = 5295 || n = 5297 || n = 5298 || n = 5304 || n = 5307)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5245 || n = 5268 || n = 5291)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5248)
	; 10 bytes
	ldx #159
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5251 || n = 5272 || n = 5293)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5257 || n = 5305)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5258 || n = 5300)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5264 || n = 5285 || n = 5306)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 5265)
	; 10 bytes
	ldx #188
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5267)
	; 10 bytes
	pha
	ldx #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
	pla
.elseif(n = 5276 || n = 5301)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5277)
	; 9 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 19
	Jne *-6
.elseif(n = 5279)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5299)
	; 10 bytes
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
	pla
.elseif(n = 5311)
	; 10 bytes
	ldx #177
	jsr @nearby_rts
	brk
	dex
	Jne *-6
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
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
; 5313 to 5390
.if(n = 5313 || n = 5315 || (n > 5321 && n < 5325) || n = 5330 || n = 5334 || n = 5343 || n = 5344 || n = 5346 || n = 5365 || n = 5389)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5314 || n = 5335 || n = 5356)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5316 || n = 5318 || n = 5319 || n = 5327 || n = 5328 || n = 5340 || n = 5347 || n = 5349 || n = 5350 || n = 5355 || n = 5358 || n = 5361 || n = 5368 || n = 5370 || n = 5371 || n = 5378 || n = 5379 || n = 5382 || n = 5385)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5317 || n = 5332 || n = 5336 || n = 5338 || n = 5348 || n = 5359 || n = 5363 || n = 5367 || n = 5369 || n = 5380 || n = 5384 || n = 5386)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5320 || n = 5374)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5321)
	; 10 bytes
	ldx #190
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5325)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5326 || n = 5351 || n = 5376)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5329 || n = 5353)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5331 || n = 5357)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5333 || n = 5364)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5337 || n = 5360 || n = 5383)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5339)
	; 10 bytes
	ldx #157
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5341)
	; 10 bytes
	ldx #178
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5342 || n = 5352 || n = 5373 || n = 5388)
	; 9..10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5345)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 5354)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 26
	dex
	dex
	Jne *-7
.elseif(n = 5362 || n = 5372 || n = 5375 || n = 5387)
	; 9..10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5366)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	nop
	pha
	dex
	Jne *-4
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
.elseif(n = 5390)
	; 10 bytes
	brk
	delay_n (n-13)
.endif
.else
; 5391 to 5468
.if(n = 5391 || n = 5413 || n = 5435)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5392 || n = 5396 || n = 5400 || n = 5410 || n = 5412 || (n > 5416 && n < 5425) || n = 5430 || n = 5434 || n = 5436 || n = 5439 || n = 5440 || (n > 5441 && n < 5446) || n = 5447 || n = 5448 || n = 5450 || n = 5456 || (n > 5462 && n < 5468))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5393 || n = 5397 || n = 5403 || n = 5408 || n = 5415 || n = 5427 || n = 5428 || n = 5437 || n = 5453 || n = 5454 || n = 5459)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5394 || n = 5398 || n = 5402 || n = 5404 || n = 5414 || n = 5416 || n = 5432 || n = 5438 || n = 5458 || n = 5460)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5395 || n = 5426 || n = 5457)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5399)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	brk
	inx
	Jne *-6
.elseif(n = 5401 || n = 5451)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5405)
	; 10 bytes
	ldx #193
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5406 || n = 5429 || n = 5452)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5407)
	; 10 bytes
	ldx #159
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5409)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5411)
	; 9 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	brk
	inx
	Jne *-6
.elseif(n = 5425 || n = 5449)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5431)
	; 10 bytes
	ldx #181
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5433)
	; 10 bytes
	ldx #194
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5441)
	; 10 bytes
	ldx #160
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5446)
	; 10 bytes
	ldx #165
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5455)
	; 8 bytes
	ldx #248 ;hides 'sed'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5461)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 5462)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5468)
	; 10 bytes
	brk
	delay_n (n-13)
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
; 5469 to 5546
.if((n > 5468 && n < 5473) || n = 5474 || (n > 5483 && n < 5487) || (n > 5491 && n < 5497) || n = 5502 || (n > 5504 && n < 5509) || n = 5510 || n = 5511 || (n > 5513 && n < 5517) || n = 5518 || n = 5520 || n = 5527 || (n > 5529 && n < 5536) || n = 5537 || n = 5538 || (n > 5539 && n < 5543))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5473 || n = 5497)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5475 || n = 5498 || n = 5521 || n = 5544)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5476 || n = 5526)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5477 || n = 5478 || n = 5481 || n = 5490 || n = 5499 || n = 5500 || n = 5503 || n = 5525 || n = 5528 || n = 5546)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5479 || n = 5501 || n = 5523 || n = 5545)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5480)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 9
	brk
	Jne *-6
.elseif(n = 5482 || n = 5509 || n = 5536)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5483 || n = 5504 || n = 5522 || n = 5524 || n = 5529)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5487 || n = 5513 || n = 5539)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5488 || n = 5519)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5489)
	; 10 bytes
	ldx #196
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5491)
	; 10 bytes
	ldx #183
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5512)
	; 10 bytes
	ldx #167
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5517)
	; 10 bytes
	ldx #197
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5543)
	; 9 bytes
	ldx #232 ;hides 'inx'
	nop
	brk
	dex
	dex
	Jne *-6
.endif
.else
; 5547 to 5625
.if(n = 5547 || n = 5552 || n = 5553 || n = 5571 || n = 5583 || n = 5591 || n = 5592 || n = 5595 || n = 5602 || n = 5604 || n = 5605 || n = 5614 || n = 5619 || n = 5624)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5548 || n = 5554 || n = 5570 || n = 5572 || n = 5579 || n = 5584 || n = 5596 || n = 5603 || n = 5616 || n = 5618 || n = 5620 || n = 5625)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5549 || (n > 5554 && n < 5563) || n = 5564 || n = 5566 || n = 5568 || n = 5574 || n = 5575 || n = 5580 || n = 5582 || n = 5585 || n = 5586 || n = 5588 || n = 5594 || n = 5597 || n = 5598 || (n > 5607 && n < 5611) || n = 5621 || n = 5623)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5550 || n = 5581 || n = 5612)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5551 || n = 5576 || n = 5601)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5563)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5565)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5567 || n = 5589 || n = 5611)
	; 9 bytes
	ldx #(n-1)/22
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5569 || n = 5593 || n = 5617)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5573)
	; 10 bytes
	ldx #199
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5577)
	; 10 bytes
	ldx #164
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5578)
	; 10 bytes
	ldx #169
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5587)
	; 9 bytes
	ldx #<$8D ;hides 'sta $02A2'
	ldx #2
	brk
	inx
	Jne *-6
.elseif(n = 5590 || n = 5613)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5599)
	; 10 bytes
	ldx #56 ;hides 'sec'
	nop
	delay_n 19
	inx
	Jne *-7
.elseif(n = 5600)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
.elseif(n = 5606)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 5607)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5615)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	brk
	dex
	Jne *-6
.elseif(n = 5622)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	nop
	dex
	Jne *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
.if((n >= 5626) && (n < 5704))
; 5626 to 5703
.if(n = 5626 || n = 5651 || n = 5676 || n = 5701)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
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
	delay_n (n-7)
	plp
.elseif(n = 5630)
	; 10 bytes
	pha
	ldx #0 ;hides 'brk imm'
	inc $02B5,X ;hides 'lda $02,X'
	dex
	Jne *-5
	pla
.elseif(n = 5631 || n = 5632 || n = 5634 || n = 5637 || n = 5640 || n = 5642 || (n > 5646 && n < 5651) || n = 5652 || n = 5655 || n = 5656 || n = 5658 || n = 5660 || n = 5663 || n = 5664 || n = 5666 || n = 5670 || n = 5672 || n = 5673 || n = 5675 || n = 5680 || n = 5681 || n = 5687 || n = 5688 || n = 5690 || n = 5693 || n = 5694 || n = 5696 || n = 5697 || n = 5699 || n = 5700 || n = 5702)
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
	delay_n 4
	brk
	dex
	Jne *-5
.elseif(n = 5635 || n = 5638 || n = 5645 || n = 5653 || n = 5661 || n = 5667 || n = 5678 || n = 5684 || n = 5691)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5636 || n = 5659 || n = 5682)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5639 || n = 5646 || n = 5654 || n = 5662 || n = 5668 || n = 5686 || n = 5692)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5641 || n = 5665 || n = 5689)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5643 || n = 5674)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5644)
	; 10 bytes
	ldx #171
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5657)
	; 10 bytes
	ldx #202
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5669 || n = 5695)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5671 || n = 5698)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5677)
	; 10 bytes
	ldx #172
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5679)
	; 10 bytes
	ldx #167
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5683)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 23
	Jne *-6
.elseif(n = 5685)
	; 10 bytes
	ldx #203
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5703)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 16
	dex
	Jne *-5
.endif
.else
; 5704 to 5781
.if(n = 5704 || n = 5706 || n = 5708 || n = 5716 || n = 5729 || n = 5739 || n = 5740 || n = 5754 || n = 5764 || n = 5770 || n = 5777)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5705 || n = 5736 || n = 5767)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5707 || n = 5715 || n = 5730 || n = 5738 || n = 5753 || n = 5763 || n = 5769 || n = 5778)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5709)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5710)
	; 10 bytes
	ldx #173
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5711)
	; 10 bytes
	ldx #<$8D ;hides 'sta $12A2'
	ldx #18
	nop
	brk
	inx
	Jne *-7
.elseif(n = 5712 || n = 5714 || (n > 5716 && n < 5721) || (n > 5721 && n < 5725) || n = 5727 || (n > 5731 && n < 5736) || n = 5742 || (n > 5743 && n < 5747) || n = 5748 || n = 5750 || (n > 5754 && n < 5761) || n = 5762 || n = 5765 || n = 5766 || n = 5768 || n = 5771 || n = 5772 || n = 5775 || n = 5780)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5713 || n = 5737 || n = 5761)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5721 || n = 5773)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5725 || n = 5752 || n = 5779)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5726 || n = 5751 || n = 5776)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5728 || n = 5774)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5731)
	; 10 bytes
	ldx #191
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5741)
	; 10 bytes
	ldx #205
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5743)
	; 10 bytes
	ldx #174
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5747)
	; 10 bytes
	ldx #169
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5749)
	; 10 bytes
	ldx #<$8D ;hides 'sta $1AA2'
	ldx #26
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5781)
	; 10 bytes
	ldx #170
	delay_n 29
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if((n > 5781 && n < 5785) || n = 5786 || n = 5789 || n = 5790 || (n > 5791 && n < 5797) || n = 5805 || n = 5807 || n = 5808 || n = 5810 || n = 5813 || n = 5814 || (n > 5815 && n < 5820) || n = 5824 || n = 5827 || n = 5830 || (n > 5836 && n < 5842) || n = 5844 || n = 5847 || n = 5848 || n = 5850 || n = 5856)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5785 || n = 5809 || n = 5833 || n = 5857)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5787 || n = 5800 || n = 5803 || n = 5811 || n = 5822 || n = 5828 || n = 5835 || n = 5845 || n = 5853 || n = 5859)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5788 || n = 5802 || n = 5804 || n = 5812 || n = 5823 || n = 5832 || n = 5834 || n = 5836 || n = 5846 || n = 5852 || n = 5854 || n = 5858)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5791)
	; 10 bytes
	ldx #193
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5797 || n = 5820 || n = 5843)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5798 || n = 5829)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5799)
	; 9 bytes
	ldx #24 ;hides 'clc'
	nop
	delay_n 16
	inx
	Jne *-6
.elseif(n = 5801 || n = 5826 || n = 5851)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5806)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5815)
	; 10 bytes
	ldx #171
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5821)
	; 10 bytes
	ldx #194
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5825)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5831)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	brk
	dex
	Jne *-6
.elseif(n = 5842)
	; 10 bytes
	ldx #177
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5849)
	; 9 bytes
	ldx #234 ;hides 'nop'
	nop
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5855)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	pha
	dex
	Jne *-5
.endif
.else
; 5860 to 5937
.if(n = 5860 || n = 5891 || n = 5922)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5861)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5862 || n = 5868 || n = 5880 || n = 5882 || n = 5893 || n = 5903 || n = 5907 || n = 5924 || n = 5928 || n = 5931)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5863 || n = 5869 || n = 5879 || n = 5884 || n = 5892 || n = 5894 || n = 5904 || n = 5925 || n = 5932)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5864 || n = 5865 || n = 5867 || (n > 5869 && n < 5875) || n = 5888 || n = 5890 || (n > 5894 && n < 5901) || n = 5902 || n = 5906 || n = 5910 || n = 5912 || n = 5913 || n = 5915 || n = 5916 || (n > 5917 && n < 5922) || n = 5923 || n = 5927 || n = 5930 || (n > 5932 && n < 5937))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5866)
	; 9 bytes
	ldx #(n-1)/23
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5875)
	; 10 bytes
	ldx #178
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5876 || n = 5901 || n = 5926)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5877)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5878)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	pha
	dex
	Jne *-5
.elseif(n = 5881 || n = 5905 || n = 5929)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5883)
	; 10 bytes
	ldx #173
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5885)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5886)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
.elseif(n = 5887 || n = 5914)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5889)
	; 9 bytes
	ldx #0
	nop
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5908)
	; 10 bytes
	ldx #179
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5909)
	; 10 bytes
	ldx #211
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5911)
	; 10 bytes
	ldx #197
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5917)
	; 10 bytes
	ldx #174
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5937)
	; 10 bytes
	ldx #212
	delay_n 23
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
; 5938 to 6015
.if((n > 5937 && n < 5941) || (n > 5941 && n < 5949) || n = 5950 || n = 5952 || (n > 5956 && n < 5965) || n = 5966 || n = 5967 || n = 5969 || n = 5970 || n = 5972 || n = 5973 || n = 5975 || n = 5982 || n = 5983 || (n > 5987 && n < 5993) || n = 5994 || (n > 5995 && n < 5999) || n = 6000 || n = 6002 || n = 6006 || (n > 6007 && n < 6015))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5941)
	; 10 bytes
	ldx #180
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5949)
	; 10 bytes
	ldx #<$8D ;hides 'sta $12A2'
	ldx #18
	delay_n 16
	inx
	Jne *-7
.elseif(n = 5951 || n = 5976 || n = 6001)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 5953 || n = 5984 || n = 6015)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 5954 || n = 5956 || n = 5980 || n = 5987 || n = 6004)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5955 || n = 5978 || n = 5979 || n = 5986 || n = 6003)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5965)
	; 10 bytes
	ldx #213
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5968 || n = 5995)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 5971)
	; 10 bytes
	ldx #199
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 5974)
	; 10 bytes
	ldx #181
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 5977)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 5981)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 5985)
	; 10 bytes
	ldx #176
	delay_n 29
	dex
	Jne *-6
.elseif(n = 5993)
	; 10 bytes
	ldx #214
	delay_n 23
	dex
	Jne *-6
.elseif(n = 5999)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 23
	inx
	Jne *-7
.elseif(n = 6005)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	brk
	dex
	dex
	Jne *-7
.elseif(n = 6007)
	; 10 bytes
	ldx #182
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.else
; 6016 to 6093
.if(n = 6016 || n = 6020 || n = 6023 || n = 6024 || n = 6030 || n = 6032 || (n > 6034 && n < 6040) || (n > 6040 && n < 6046) || n = 6050 || (n > 6054 && n < 6059) || n = 6060 || (n > 6061 && n < 6073) || n = 6074 || n = 6081 || n = 6082 || n = 6084 || n = 6088 || n = 6090 || n = 6093)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6017 || n = 6027 || n = 6028 || n = 6048 || n = 6053 || n = 6075 || n = 6078 || n = 6079)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6018 || n = 6029 || n = 6034 || n = 6052 || n = 6054 || n = 6080 || n = 6086 || n = 6092)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6019)
	; 10 bytes
	ldx #177
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6021)
	; 10 bytes
	ldx #215
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6022)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6025 || n = 6049 || n = 6073)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 6026 || n = 6051 || n = 6076)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6031)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 19
	inx
	Jne *-6
.elseif(n = 6033 || n = 6059 || n = 6085)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6040)
	; 10 bytes
	ldx #183
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6046 || n = 6077)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6047)
	; 10 bytes
	ldx #216 ;hides 'cld'
	nop
	delay_n 19
	dex
	Jne *-7
.elseif(n = 6061)
	; 10 bytes
	ldx #202
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6083)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6087)
	; 10 bytes
	ldx #179
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6089)
	; 9 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	brk
	Jne *-6
.elseif(n = 6091)
	; 10 bytes
	ldx #203
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
; 6094 to 6171
.if(n = 6094 || n = 6096 || n = 6098 || n = 6102 || n = 6107 || n = 6109 || (n > 6117 && n < 6121) || n = 6122 || n = 6125 || n = 6127 || n = 6131 || n = 6132 || n = 6135 || n = 6143 || n = 6144 || n = 6146 || n = 6150 || n = 6152 || n = 6156 || (n > 6157 && n < 6161) || n = 6162 || (n > 6163 && n < 6170) || n = 6171)
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
	ldx #<$8D ;hides 'sta $02A2'
	ldx #2
	nop
	brk
	inx
	Jne *-7
.elseif(n = 6097 || n = 6121)
	; 9 bytes
	ldx #(n-1)/24
	delay_n 19
	dex
	Jne *-5
.elseif(n = 6099 || n = 6112 || n = 6114 || n = 6115 || n = 6123 || n = 6128 || n = 6136 || n = 6138 || n = 6141 || n = 6147 || n = 6153)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6100 || n = 6104 || n = 6111 || n = 6113 || n = 6124 || n = 6129 || n = 6137 || n = 6142 || n = 6148 || n = 6154)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6101 || n = 6126 || n = 6151)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6103 || n = 6130 || n = 6157)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6105)
	; 10 bytes
	ldx #218
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6106)
	; 10 bytes
	ldx #185
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6108 || n = 6139 || n = 6170)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6110)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 6116 || n = 6140)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6117)
	; 10 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6133)
	; 10 bytes
	ldx #219
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6134)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 6145)
	; 9 bytes
	ldx #0
	delay_n 19
	dex
	Jne *-5
.elseif(n = 6149)
	; 10 bytes
	ldx #<$8D ;hides 'sta $0AA2'
	ldx #10
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6155)
	; 10 bytes
	ldx #181
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6161)
	; 10 bytes
	ldx #220
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6163)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.endif
.else
; 6172 to 6250
.if(n = 6172)
	; 10 bytes
	ldx #187
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6173 || n = 6179 || n = 6202 || n = 6204 || n = 6229 || n = 6235)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6174 || n = 6175 || n = 6177 || n = 6180 || n = 6182 || n = 6183 || (n > 6184 && n < 6189) || (n > 6189 && n < 6199) || n = 6200 || (n > 6205 && n < 6211) || (n > 6211 && n < 6215) || n = 6216 || (n > 6217 && n < 6223) || n = 6224 || n = 6225 || n = 6227 || n = 6230 || n = 6231 || n = 6233 || n = 6237 || n = 6239 || n = 6240 || (n > 6241 && n < 6245) || (n > 6245 && n < 6251))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6176 || n = 6226)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6178 || n = 6203 || n = 6228 || n = 6234)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6181)
	; 10 bytes
	ldx #206
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6184 || n = 6211)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6189)
	; 10 bytes
	ldx #182
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6199)
	; 9 bytes
	ldx #248 ;hides 'sed'
	nop
	delay_n 16
	dex
	Jne *-6
.elseif(n = 6201 || n = 6232)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6205)
	; 10 bytes
	ldx #188
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6215)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6217)
	; 10 bytes
	ldx #222
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6223)
	; 10 bytes
	ldx #183
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6236)
	; 10 bytes
	ldx #232 ;hides 'inx'
	nop
	delay_n 16
	dex
	dex
	Jne *-7
.elseif(n = 6238)
	; 10 bytes
	ldx #189
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6241)
	; 10 bytes
	ldx #208
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6245)
	; 10 bytes
	ldx #223
	delay_n 23
	dex
	Jne *-6
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
; 6251 to 6328
.if(n = 6251 || n = 6276 || n = 6301 || n = 6326)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6252 || n = 6255 || n = 6256 || (n > 6257 && n < 6263) || (n > 6267 && n < 6271) || n = 6272 || n = 6274 || n = 6275 || n = 6277 || (n > 6279 && n < 6291) || n = 6300 || n = 6302 || (n > 6305 && n < 6317) || n = 6318 || (n > 6320 && n < 6325))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6253 || n = 6265 || n = 6278 || n = 6296 || n = 6303 || n = 6327 || n = 6328)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6254 || n = 6266 || n = 6279 || n = 6295 || n = 6297 || n = 6320)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6257)
	; 10 bytes
	ldx #184
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6263 || n = 6294 || n = 6325)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6264)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 26
	inx
	inx
	Jne *-7
.elseif(n = 6267 || n = 6293)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6271)
	; 10 bytes
	ldx #190
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6273)
	; 10 bytes
	ldx #224
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6291)
	; 10 bytes
	ldx #185
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6292)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
.elseif(n = 6298)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 6299)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6304)
	; 10 bytes
	ldx #191
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6305)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 6317)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	brk
	dex
	Jne *-6
.elseif(n = 6319)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.endif
.else
; 6329 to 6406
.if(n = 6329)
	; 10 bytes
	ldx #226
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6330 || (n > 6331 && n < 6337) || (n > 6337 && n < 6345) || n = 6347 || n = 6348 || n = 6350 || n = 6352 || n = 6355 || n = 6360 || (n > 6361 && n < 6365) || n = 6366 || n = 6374 || n = 6375 || n = 6377 || (n > 6379 && n < 6385) || n = 6386 || n = 6388 || n = 6398 || n = 6402 || n = 6405 || n = 6406)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6331)
	; 10 bytes
	ldx #211
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6337)
	; 10 bytes
	ldx #192
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6345)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6346 || n = 6373 || n = 6400)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6349)
	; 10 bytes
	ldx #<$8D ;hides 'sta $02A2'
	ldx #2
	delay_n 16
	inx
	Jne *-7
.elseif(n = 6351 || n = 6376)
	; 9 bytes
	ldx #(n-1)/25
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6353 || n = 6358 || n = 6367 || n = 6369 || n = 6378 || n = 6389 || n = 6392 || n = 6394 || n = 6395)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6354 || n = 6368 || n = 6379 || n = 6393 || n = 6404)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6356 || n = 6387)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6357)
	; 10 bytes
	ldx #227
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6359)
	; 10 bytes
	ldx #187
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6361)
	; 10 bytes
	ldx #212
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6365)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 6370)
	; 10 bytes
	ldx #193
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6371)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 6372 || n = 6397)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6385)
	; 10 bytes
	ldx #228
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6390)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 6391)
	; 10 bytes
	ldx #213
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6396)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 9
	inx
	Jne *-5
	sta (@ptrtemp,x)
.elseif(n = 6399)
	; 10 bytes
	ldx #56 ;hides 'sec'
	jsr @nearby_rts
	brk
	inx
	Jne *-7
.elseif(n = 6401)
	; 9 bytes
	ldx #0
	delay_n 7
	brk
	dex
	Jne *-5
.elseif(n = 6403)
	; 10 bytes
	ldx #194
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.else
; 6407 to 6562
.if((n > 6406 && n < 6413) || (n > 6413 && n < 6418) || n = 6419 || n = 6422 || (n > 6423 && n < 6427) || (n > 6427 && n < 6436) || (n > 6436 && n < 6441) || (n > 6441 && n < 6447) || n = 6448 || n = 6453 || (n > 6454 && n < 6461) || (n > 6461 && n < 6467) || n = 6468 || (n > 6469 && n < 6475) || (n > 6475 && n < 6479) || (n > 6483 && n < 6495) || n = 6496 || (n > 6497 && n < 6501) || (n > 6502 && n < 6508) || n = 6509 || n = 6510 || n = 6512 || (n > 6514 && n < 6525) || n = 6526 || n = 6528 || (n > 6529 && n < 6535) || (n > 6535 && n < 6541) || n = 6543 || (n > 6545 && n < 6551) || n = 6552 || (n > 6553 && n < 6562))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6413)
	; 10 bytes
	ldx #229
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6418 || n = 6449 || n = 6480 || n = 6511 || n = 6542)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6420 || n = 6451 || n = 6482 || n = 6513 || n = 6544)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6421)
	; 10 bytes
	ldx #214
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6423 || n = 6475 || n = 6501 || n = 6527 || n = 6553)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6427 || n = 6454 || n = 6481 || n = 6508 || n = 6562)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6436)
	; 10 bytes
	ldx #195
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6441)
	; 10 bytes
	ldx #230
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6447)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6450 || n = 6452 || n = 6483 || n = 6514 || n = 6545)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6461)
	; 10 bytes
	ldx #190
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6467)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 19
	dex
	dex
	Jne *-7
.elseif(n = 6469)
	; 10 bytes
	ldx #196
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6479)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 23
	dex
	Jne *-7
.elseif(n = 6495)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-7
.elseif(n = 6497)
	; 10 bytes
	ldx #232
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6502)
	; 10 bytes
	ldx #197
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6525)
	; 10 bytes
	ldx #233
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6529)
	; 10 bytes
	ldx #192
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6535)
	; 10 bytes
	ldx #198
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6541)
	; 10 bytes
	ldx #218
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6551)
	; 10 bytes
	ldx #234 ;hides 'nop'
	nop
	delay_n 19
	dex
	Jne *-7
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if(n = 6563)
	; 10 bytes
	ldx #193
	delay_n 29
	dex
	Jne *-6
.elseif((n > 6563 && n < 6568) || n = 6569 || n = 6570 || n = 6572 || n = 6574 || n = 6577 || n = 6578 || n = 6580 || (n > 6581 && n < 6589) || (n > 6589 && n < 6597) || n = 6598 || n = 6600 || n = 6603 || n = 6608 || (n > 6609 && n < 6616) || (n > 6616 && n < 6620) || n = 6621 || (n > 6623 && n < 6631) || n = 6632 || n = 6633 || n = 6636 || n = 6639 || n = 6640)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6568)
	; 10 bytes
	ldx #199
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6571)
	; 10 bytes
	ldx #219
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6573 || n = 6604 || n = 6635)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6575 || n = 6606 || n = 6622 || n = 6637)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6576 || n = 6602 || n = 6607 || n = 6623 || n = 6638)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6579 || n = 6605)
	; 10 bytes
	ldx #(n-1)/26
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6581)
	; 10 bytes
	ldx #235
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6589 || n = 6616)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6597)
	; 10 bytes
	ldx #194
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6599)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 26
	inx
	Jne *-6
.elseif(n = 6601)
	; 10 bytes
	ldx #200
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6609)
	; 10 bytes
	ldx #236
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6620)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	delay_n 6
	dex
	Jne *-6
.elseif(n = 6631)
	; 10 bytes
	ldx #221
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6634)
	; 10 bytes
	ldx #201
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.else
; 6641 to 6718
.if(n = 6641 || n = 6642 || n = 6644 || n = 6645 || n = 6647 || (n > 6649 && n < 6657) || (n > 6657 && n < 6661) || (n > 6661 && n < 6665) || (n > 6670 && n < 6691) || n = 6692 || n = 6694 || n = 6696 || (n > 6701 && n < 6719))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6643 || n = 6670)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6646)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	delay_n 6
	dex
	Jne *-6
.elseif(n = 6648 || n = 6668 || n = 6699)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6649 || n = 6669 || n = 6701)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6657)
	; 10 bytes
	ldx #0
	nop
	delay_n 19
	dex
	Jne *-6
.elseif(n = 6661)
	; 10 bytes
	ldx #222
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6665)
	; 10 bytes
	ldx #196
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6666 || n = 6697)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6667)
	; 10 bytes
	ldx #202
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6691)
	; 10 bytes
	ldx #223
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6693)
	; 10 bytes
	ldx #239
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6695)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	brk
	dex
	Jne *-6
.elseif(n = 6698)
	; 9 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 26
	Jne *-6
.elseif(n = 6700)
	; 10 bytes
	ldx #203
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 6719) && (n < 6797))
; 6719 to 6796
.if(n = 6719 || n = 6720 || n = 6722 || n = 6723 || n = 6725 || n = 6726 || n = 6729 || n = 6732 || (n > 6733 && n < 6749) || n = 6750 || (n > 6751 && n < 6759) || n = 6760 || (n > 6762 && n < 6766) || (n > 6767 && n < 6777) || n = 6779 || n = 6780 || (n > 6781 && n < 6785) || (n > 6785 && n < 6790) || n = 6791 || (n > 6793 && n < 6797))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6721)
	; 10 bytes
	ldx #224
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6724 || n = 6751 || n = 6778)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6727)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	brk
	inx
	Jne *-7
.elseif(n = 6728 || n = 6759 || n = 6790)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6730 || n = 6761 || n = 6792)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6731 || n = 6762 || n = 6793)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6733)
	; 10 bytes
	ldx #204
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6749)
	; 10 bytes
	ldx #241
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6766)
	; 10 bytes
	ldx #205
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6767)
	; 10 bytes
	ldx #199
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6777)
	; 10 bytes
	ldx #242
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6781)
	; 10 bytes
	ldx #226
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6785)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	brk
	dex
	Jne *-7
.endif
.else
; 6797 to 6875
.if(n = 6797 || n = 6798 || n = 6800 || (n > 6801 && n < 6805) || (n > 6805 && n < 6811) || (n > 6811 && n < 6821) || n = 6822 || (n > 6824 && n < 6832) || n = 6834 || (n > 6835 && n < 6841) || (n > 6841 && n < 6852) || n = 6853 || (n > 6855 && n < 6859) || n = 6860 || (n > 6861 && n < 6865) || (n > 6865 && n < 6869) || n = 6870 || (n > 6871 && n < 6875))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6799)
	; 10 bytes
	ldx #206
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6801)
	; 10 bytes
	ldx #200
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6805 || n = 6859)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6811)
	; 10 bytes
	ldx #227
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6821 || n = 6852)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6823 || n = 6854)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6824 || n = 6855)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6832)
	; 10 bytes
	ldx #207
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6833)
	; 10 bytes
	ldx #244
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6835)
	; 10 bytes
	ldx #201
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6841)
	; 10 bytes
	ldx #228
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6861)
	; 10 bytes
	ldx #245
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6865)
	; 10 bytes
	ldx #208
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6869)
	; 10 bytes
	ldx #202
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6871)
	; 10 bytes
	ldx #229
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6875)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 9
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
; 6876 to 6953
.if(n = 6876 || (n > 6878 && n < 6883) || n = 6884 || n = 6887 || n = 6888 || (n > 6889 && n < 6898) || n = 6899 || n = 6900 || (n > 6905 && n < 6911) || n = 6912 || n = 6915 || (n > 6917 && n < 6931) || (n > 6931 && n < 6937) || (n > 6937 && n < 6943) || n = 6944 || n = 6946 || (n > 6948 && n < 6954))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6877 || n = 6885 || n = 6904 || n = 6916 || n = 6947)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6878 || n = 6905 || n = 6917 || n = 6948)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6883 || n = 6914 || n = 6945)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6886)
	; 10 bytes
	ldx #(n-1)/27
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6889)
	; 10 bytes
	ldx #246
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6898)
	; 10 bytes
	ldx #209
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6901)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 29
	Jne *-6
.elseif(n = 6902)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 6903)
	; 10 bytes
	ldx #203
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6911)
	; 10 bytes
	ldx #216 ;hides 'cld'
	jsr @nearby_rts
	brk
	dex
	Jne *-7
.elseif(n = 6913)
	; 10 bytes
	ldx #0
	delay_n 9
	brk
	dex
	Jne *-6
.elseif(n = 6931)
	; 10 bytes
	ldx #210
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6937)
	; 10 bytes
	ldx #204
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6943)
	; 10 bytes
	ldx #248 ;hides 'sed'
	nop
	delay_n 19
	dex
	Jne *-7
.endif
.else
; 6954 to 7031
.if((n > 6953 && n < 6959) || n = 6960 || n = 6962 || n = 6963 || (n > 6964 && n < 6971) || n = 6972 || n = 6974 || n = 6975 || n = 6977 || (n > 6979 && n < 6991) || (n > 6991 && n < 6997) || n = 6998 || n = 7000 || (n > 7001 && n < 7005) || n = 7006 || n = 7008 || (n > 7010 && n < 7019) || n = 7020 || (n > 7021 && n < 7029) || n = 7031)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6959)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 23
	inx
	Jne *-7
.elseif(n = 6961)
	; 10 bytes
	ldx #232
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6964)
	; 10 bytes
	ldx #211
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6971)
	; 10 bytes
	ldx #205
	delay_n 29
	dex
	Jne *-6
.elseif(n = 6973)
	; 10 bytes
	ldx #249
	delay_n 23
	dex
	Jne *-6
.elseif(n = 6976 || n = 7007)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 6978 || n = 7009)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6979 || n = 7010)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6991)
	; 10 bytes
	ldx #233
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 6997)
	; 10 bytes
	ldx #212
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 6999)
	; 10 bytes
	ldx #56 ;hides 'sec'
	nop
	delay_n 26
	inx
	Jne *-7
.elseif(n = 7001)
	; 10 bytes
	ldx #250
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7005)
	; 10 bytes
	ldx #206
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7019)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 23
	dex
	Jne *-7
.elseif(n = 7021)
	; 10 bytes
	ldx #234
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7029)
	; 10 bytes
	ldx #251
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7030)
	; 10 bytes
	ldx #213
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.else
; 7032 to 7187
.if((n > 7031 && n < 7038) || (n > 7041 && n < 7051) || (n > 7051 && n < 7057) || (n > 7057 && n < 7063) || (n > 7063 && n < 7069) || n = 7070 || (n > 7073 && n < 7081) || (n > 7081 && n < 7085) || (n > 7085 && n < 7096) || (n > 7096 && n < 7100) || n = 7101 || n = 7105 || n = 7106 || (n > 7107 && n < 7111) || n = 7112 || (n > 7113 && n < 7127) || n = 7128 || (n > 7134 && n < 7141) || (n > 7141 && n < 7158) || n = 7159 || n = 7163 || (n > 7165 && n < 7169) || n = 7170 || (n > 7171 && n < 7175) || (n > 7175 && n < 7188))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7038 || n = 7069 || n = 7100 || n = 7131 || n = 7162)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7039)
	; 10 bytes
	ldx #207
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7040 || n = 7071 || n = 7102 || n = 7132 || n = 7133 || n = 7160 || n = 7164)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7041 || n = 7072 || n = 7103 || n = 7134 || n = 7161 || n = 7165)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7051)
	; 10 bytes
	ldx #235
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7057)
	; 10 bytes
	ldx #252
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7063)
	; 10 bytes
	ldx #214
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7073)
	; 10 bytes
	ldx #208
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7081)
	; 10 bytes
	ldx #236
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7085)
	; 10 bytes
	ldx #253
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7096)
	; 10 bytes
	ldx #215
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7104)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	delay_n 26
	Jne *-7
.elseif(n = 7107)
	; 10 bytes
	ldx #209
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7111)
	; 10 bytes
	ldx #237
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7113)
	; 10 bytes
	ldx #254
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7127)
	; 9 bytes
	ldx #216 ;hides 'cld'
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7129)
	; 10 bytes
	ldx #216
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7130)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 10
	dex
	Jne *-6
.elseif(n = 7141)
	; 10 bytes
	ldx #238
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7158)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 10
	dex
	Jne *-6
.elseif(n = 7169)
	; 10 bytes
	ldx #0
	delay_n 23
	dex
	Jne *-6
.elseif(n = 7171)
	; 10 bytes
	ldx #239
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7175)
	; 10 bytes
	ldx #211
	delay_n 29
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
; 7188 to 7343
.if((n > 7187 && n < 7191) || n = 7192 || n = 7194 || n = 7197 || n = 7198 || n = 7200 || (n > 7201 && n < 7209) || (n > 7209 && n < 7224) || n = 7225 || n = 7229 || n = 7230 || (n > 7231 && n < 7243) || (n > 7243 && n < 7255) || n = 7256 || n = 7259 || n = 7260 || (n > 7261 && n < 7277) || (n > 7277 && n < 7286) || n = 7287 || n = 7290 || n = 7292 || n = 7293 || (n > 7294 && n < 7307) || (n > 7307 && n < 7311) || (n > 7311 && n < 7317) || n = 7318 || (n > 7321 && n < 7327) || (n > 7327 && n < 7344))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7191)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 9
	brk
	dex
	Jne *-7
.elseif(n = 7193 || n = 7224 || n = 7255 || n = 7286 || n = 7317)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7195)
	; 10 bytes
	ldx #218
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7196 || n = 7227 || n = 7258 || n = 7289 || n = 7320)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7199)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 29
	inx
	Jne *-7
.elseif(n = 7201)
	; 10 bytes
	ldx #240
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7209)
	; 10 bytes
	ldx #212
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7226 || n = 7257 || n = 7288 || n = 7319)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7228)
	; 10 bytes
	ldx #219
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7231)
	; 10 bytes
	ldx #241
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7243)
	; 10 bytes
	ldx #213
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7261)
	; 10 bytes
	ldx #220
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7277)
	; 10 bytes
	ldx #214
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7291)
	; 10 bytes
	ldx #243
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7294)
	; 10 bytes
	ldx #221
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7307)
	; 10 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 29
	Jne *-7
.elseif(n = 7311)
	; 10 bytes
	ldx #215
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7321)
	; 10 bytes
	ldx #244
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7327)
	; 10 bytes
	ldx #222
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.else
; 7344 to 7500
.if(n = 7344 || n = 7346 || n = 7347 || n = 7349 || (n > 7351 && n < 7360) || (n > 7360 && n < 7379) || n = 7380 || n = 7383 || n = 7384 || (n > 7385 && n < 7393) || (n > 7393 && n < 7410) || (n > 7414 && n < 7423) || n = 7424 || n = 7425 || (n > 7426 && n < 7439) || n = 7440 || n = 7442 || n = 7445 || n = 7446 || (n > 7447 && n < 7459) || (n > 7459 && n < 7471) || n = 7473 || (n > 7475 && n < 7481) || (n > 7481 && n < 7487) || (n > 7487 && n < 7492) || (n > 7492 && n < 7501))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7345)
	; 10 bytes
	ldx #216
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7348 || n = 7379 || n = 7410 || n = 7441 || n = 7472)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7350 || n = 7381 || n = 7412 || n = 7443 || n = 7474)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7351)
	; 10 bytes
	ldx #245
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7360)
	; 10 bytes
	ldx #223
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7382 || n = 7444 || n = 7475)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7385)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 11
	dex
	Jne *-7
.elseif(n = 7393)
	; 10 bytes
	ldx #224
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7411)
	; 10 bytes
	ldx #247
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7413)
	; 10 bytes
	ldx #218
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7414)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 11
	dex
	Jne *-7
.elseif(n = 7423)
	; 10 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	brk
	inx
	Jne *-7
.elseif(n = 7426)
	; 10 bytes
	ldx #225
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7439)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 23
	dex
	Jne *-7
.elseif(n = 7447)
	; 10 bytes
	ldx #219
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7459)
	; 10 bytes
	ldx #226
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7471)
	; 10 bytes
	ldx #249
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7481)
	; 10 bytes
	ldx #220
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7487)
	; 10 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	brk
	dex
	Jne *-7
.elseif(n = 7492)
	; 10 bytes
	ldx #227
	nop
	delay_n 26
	dex
	Jne *-6
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
; 7501 to 7656
.if(n = 7501)
	; 10 bytes
	ldx #250
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7502 || n = 7504 || (n > 7506 && n < 7510) || (n > 7510 && n < 7515) || (n > 7515 && n < 7525) || (n > 7525 && n < 7531) || n = 7532 || n = 7533 || n = 7535 || (n > 7537 && n < 7549) || (n > 7549 && n < 7558) || n = 7560 || (n > 7561 && n < 7565) || n = 7566 || (n > 7568 && n < 7583) || (n > 7583 && n < 7591) || (n > 7591 && n < 7596) || n = 7597 || (n > 7599 && n < 7617) || (n > 7617 && n < 7621) || n = 7622 || n = 7623 || n = 7625 || n = 7626 || n = 7628 || (n > 7630 && n < 7640) || n = 7641 || (n > 7643 && n < 7651) || (n > 7651 && n < 7655) || n = 7656)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7503 || n = 7534 || n = 7565 || n = 7596 || n = 7627)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7505 || n = 7536 || n = 7567 || n = 7598 || n = 7629 || n = 7642)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7506 || n = 7537 || n = 7568 || n = 7599 || n = 7630 || n = 7643)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7510)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 32
	Jne *-7
.elseif(n = 7515)
	; 10 bytes
	ldx #221
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7525)
	; 10 bytes
	ldx #228
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7531)
	; 10 bytes
	ldx #251
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7549)
	; 10 bytes
	ldx #222
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7558)
	; 10 bytes
	ldx #229
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7559)
	; 10 bytes
	ldx #216 ;hides 'cld'
	nop
	delay_n 26
	dex
	Jne *-7
.elseif(n = 7561)
	; 10 bytes
	ldx #252
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7583)
	; 10 bytes
	ldx #223
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7591)
	; 10 bytes
	ldx #230
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7617)
	; 10 bytes
	ldx #224
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7621)
	; 10 bytes
	ldx #254
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7624)
	; 10 bytes
	ldx #231
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7640)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 7651)
	; 10 bytes
	ldx #255
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7655)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 26
	inx
	Jne *-6
.endif
.else
; 7657 to 7812
.if(n = 7657)
	; 10 bytes
	ldx #232
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7658 || n = 7689 || n = 7720 || n = 7751 || n = 7782)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7659 || (n > 7661 && n < 7670) || n = 7671 || (n > 7673 && n < 7681) || (n > 7681 && n < 7685) || (n > 7685 && n < 7689) || (n > 7692 && n < 7713) || (n > 7713 && n < 7719) || (n > 7724 && n < 7751) || n = 7752 || n = 7755 || (n > 7756 && n < 7775) || (n > 7775 && n < 7782) || n = 7783 || n = 7786 || n = 7788 || (n > 7789 && n < 7813))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7660 || n = 7672 || n = 7691 || n = 7722 || n = 7753 || n = 7784)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7661 || n = 7673 || n = 7692 || n = 7724 || n = 7754 || n = 7785)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7670)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	dex
	Jne *-6
.elseif(n = 7681)
	; 10 bytes
	ldx #0
	jsr @nearby_rts
	brk
	dex
	Jne *-6
.elseif(n = 7685)
	; 10 bytes
	ldx #226
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7690)
	; 10 bytes
	ldx #233
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7713)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 26
	Jne *-7
.elseif(n = 7719)
	; 10 bytes
	ldx #227
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7721)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7723)
	; 10 bytes
	ldx #234
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7756)
	; 10 bytes
	ldx #235
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7775)
	; 10 bytes
	ldx #216 ;hides 'cld'
	delay_n 29
	dex
	Jne *-7
.elseif(n = 7787)
	; 10 bytes
	ldx #229
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7789)
	; 10 bytes
	ldx #236
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
; 7813 to 7968
.if(n = 7813 || n = 7844 || n = 7875 || n = 7906)
	; 9 bytes
	ldx #(n-1)/31
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7814 || (n > 7816 && n < 7821) || (n > 7822 && n < 7844) || n = 7845 || (n > 7847 && n < 7855) || (n > 7855 && n < 7875) || n = 7876 || (n > 7878 && n < 7888) || (n > 7889 && n < 7895) || n = 7896 || (n > 7902 && n < 7906) || n = 7907 || (n > 7909 && n < 7921) || n = 7924 || n = 7925 || n = 7927 || n = 7934 || n = 7936 || n = 7938 || (n > 7940 && n < 7954) || n = 7955 || n = 7956 || (n > 7957 && n < 7969))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7815 || n = 7846 || n = 7877 || n = 7897 || n = 7899 || n = 7900 || n = 7908 || n = 7928 || n = 7930 || n = 7931 || n = 7939)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7816 || n = 7847 || n = 7878 || n = 7898 || n = 7909 || n = 7929 || n = 7940)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7821)
	; 10 bytes
	ldx #230
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7822)
	; 10 bytes
	ldx #237
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7855)
	; 10 bytes
	ldx #238
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7888)
	; 10 bytes
	ldx #239
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7889)
	; 10 bytes
	ldx #232
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7895)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 7901)
	; 10 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 7902 || n = 7933)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7921)
	; 10 bytes
	ldx #240
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7922)
	; 10 bytes
	php
	ldx #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	inx
	Jne *-5
	plp
.elseif(n = 7923)
	; 10 bytes
	ldx #233
	delay_n 29
	dex
	Jne *-6
.elseif(n = 7926)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 7932)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	brk
	inx
	Jne *-5
	sta (@ptrtemp,x)
.elseif(n = 7935)
	; 10 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	brk
	dex
	Jne *-7
.elseif(n = 7937)
	; 9 bytes
	ldx #0
	delay_n 26
	dex
	Jne *-5
.elseif(n = 7954)
	; 10 bytes
	ldx #241
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7957)
	; 10 bytes
	ldx #234
	delay_n 29
	dex
	Jne *-6
.endif
.else
; 7969 to 8125
.if((n > 7968 && n < 7987) || (n > 7987 && n < 7991) || (n > 7991 && n < 8020) || (n > 8020 && n < 8025) || (n > 8025 && n < 8053) || (n > 8053 && n < 8059) || (n > 8059 && n < 8084) || n = 8085 || (n > 8086 && n < 8093) || (n > 8093 && n < 8119) || (n > 8119 && n < 8126))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7987)
	; 10 bytes
	ldx #242
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 7991)
	; 10 bytes
	ldx #235
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8020)
	; 10 bytes
	ldx #243
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8025)
	; 10 bytes
	ldx #236
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8053)
	; 10 bytes
	ldx #244
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8059)
	; 10 bytes
	ldx #237
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8084)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 26
	dex
	dex
	Jne *-7
.elseif(n = 8086)
	; 10 bytes
	ldx #245
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8093)
	; 10 bytes
	ldx #238
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8119)
	; 10 bytes
	ldx #246
	nop
	delay_n 26
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
; 8126 to 8281
.if(n = 8126 || (n > 8127 && n < 8150) || n = 8151 || (n > 8152 && n < 8161) || (n > 8161 && n < 8182) || n = 8184 || n = 8187 || n = 8188 || (n > 8189 && n < 8195) || (n > 8195 && n < 8218) || (n > 8218 && n < 8229) || (n > 8229 && n < 8251) || (n > 8251 && n < 8263) || (n > 8263 && n < 8282))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8127)
	; 10 bytes
	ldx #239
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8150)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 14
	dex
	Jne *-7
.elseif(n = 8152)
	; 10 bytes
	ldx #247
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8161)
	; 10 bytes
	ldx #240
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8182)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 14
	dex
	Jne *-7
.elseif(n = 8183)
	; 9 bytes
	ldx #248 ;hides 'sed'
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8185)
	; 10 bytes
	ldx #248
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8186)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8189)
	; 10 bytes
	ldx #234 ;hides 'nop'
	nop
	delay_n 26
	dex
	Jne *-7
.elseif(n = 8195)
	; 10 bytes
	ldx #241
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8218)
	; 10 bytes
	ldx #249
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8229)
	; 10 bytes
	ldx #242
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8251)
	; 10 bytes
	ldx #250
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8263)
	; 10 bytes
	ldx #243
	delay_n 29
	dex
	Jne *-6
.endif
.else
; 8282 to 8437
.if(n = 8282 || n = 8283 || (n > 8284 && n < 8297) || (n > 8297 && n < 8317) || (n > 8317 && n < 8331) || (n > 8331 && n < 8350) || (n > 8351 && n < 8365) || (n > 8365 && n < 8383) || (n > 8383 && n < 8399) || (n > 8399 && n < 8405) || n = 8406 || (n > 8408 && n < 8416) || (n > 8416 && n < 8423) || (n > 8423 && n < 8433) || (n > 8433 && n < 8438))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8284)
	; 10 bytes
	ldx #251
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8297)
	; 10 bytes
	ldx #244
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8317)
	; 10 bytes
	ldx #252
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8331)
	; 10 bytes
	ldx #245
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8350)
	; 10 bytes
	ldx #253
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8351)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 29
	inx
	Jne *-7
.elseif(n = 8365)
	; 10 bytes
	ldx #246
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8383)
	; 10 bytes
	ldx #254
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8399)
	; 10 bytes
	ldx #247
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8405)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	brk
	dex
	Jne *-6
.elseif(n = 8407)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8408)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8416)
	; 10 bytes
	ldx #255
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8423)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 29
	dex
	Jne *-7
.elseif(n = 8433)
	; 10 bytes
	ldx #248
	delay_n 29
	dex
	Jne *-6
.endif
.endif
.else
; 8438 to 8750
.if(n = 8438)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	brk
	dex
	Jne *-6
.elseif(n = 8439 || (n > 8441 && n < 8449) || (n > 8449 && n < 8467) || (n > 8467 && n < 8501) || (n > 8501 && n < 8535) || (n > 8535 && n < 8569) || (n > 8569 && n < 8603) || (n > 8603 && n < 8637) || (n > 8637 && n < 8660) || n = 8661 || (n > 8663 && n < 8671) || (n > 8671 && n < 8679) || (n > 8679 && n < 8694) || n = 8695 || (n > 8697 && n < 8705) || (n > 8705 && n < 8751))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8440 || n = 8662 || n = 8696)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8441 || n = 8663 || n = 8697)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8449)
	; 10 bytes
	ldx #0
	nop
	delay_n 26
	dex
	Jne *-6
.elseif(n = 8467)
	; 10 bytes
	ldx #249
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8501)
	; 10 bytes
	ldx #250
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8535)
	; 10 bytes
	ldx #251
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8569)
	; 10 bytes
	ldx #252
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8603)
	; 10 bytes
	ldx #253
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8637)
	; 10 bytes
	ldx #254
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8660)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 16
	dex
	Jne *-6
.elseif(n = 8671)
	; 10 bytes
	ldx #255
	delay_n 29
	dex
	Jne *-6
.elseif(n = 8679)
	; 10 bytes
	ldx #248 ;hides 'sed'
	nop
	delay_n 26
	dex
	Jne *-7
.elseif(n = 8694)
	; 9 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 16
	dex
	Jne *-6
.elseif(n = 8705)
	; 10 bytes
	ldx #0
	delay_n 29
	dex
	Jne *-6
.endif
.endif
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8915) || (n > 8915 && n < 8927) || (n > 8927 && n < 8931) || (n > 8931 && n < 8950) || (n > 8950 && n < 9170) || (n > 9170 && n < 9206) || (n > 9206 && n < 9425) || (n > 9425 && n < 9462) || (n > 9462 && n < 9680) || (n > 9680 && n < 9718) || (n > 9718 && n < 10001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8915)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 4
	brk
	dex
	Jne *-7
.elseif(n = 8927)
	; 10 bytes
	ldx #248 ;hides 'sed'
	delay_n 29
	dex
	Jne *-7
.elseif(n = 8931)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
.elseif(n = 8950)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	nop
	brk
	dex
	Jne *-7
.elseif(n = 9170)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	delay_n 16
	dex
	Jne *-7
.elseif(n = 9206)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 4
	delay_n 16
	dex
	Jne *-7
.elseif(n = 9425)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 19
	dex
	Jne *-7
.elseif(n = 9462)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 19
	dex
	Jne *-7
.elseif(n = 9680)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 7
	brk
	dex
	Jne *-7
.elseif(n = 9718)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 9
	brk
	dex
	Jne *-7
.endif
.endif
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 11210) || (n > 11210 && n < 11254) || (n > 11254 && n < 20001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11210)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 26
	dex
	Jne *-7
.elseif(n = 11254)
	; 10 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 26
	dex
	Jne *-7
.endif
.endif

.endif
	rest
.endmacro
