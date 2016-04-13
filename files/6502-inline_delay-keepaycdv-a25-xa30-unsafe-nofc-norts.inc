; delay_n, delays n cycles with small code
; Will clobber X, and Z+N
; Will preserve A, Y, S, C, D, V, and I (interrupt-disable flag)
;
; Uses JMP (unrelocatable code).
;
; WARNING: If an interrupt happens in the middle of this code, data corruption can occur.
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
; - 5 bytes or less when n <= 16
; - 6 bytes or less when n <= 19
; - 7 bytes or less when n <= 26
; - 8 bytes or less when n <= 1294
; - 9 bytes or less when n <= 1436
; -10 bytes or less when n <= 2305
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=4, avg=5.2, max=7
; - When 21 <= n <= 30: min=5, avg=6.3, max=8
; - When 31 <= n <= 50: min=5, avg=6.5, max=8
; - When 51 <= n <= 75: min=5, avg=6.4, max=8
; - When 76 <= n <= 100: min=5, avg=6.3, max=8
; - When 101 <= n <= 200: min=4, avg=6.3, max=8
; - When 201 <= n <= 300: min=5, avg=6.4, max=8
; - When 301 <= n <= 500: min=4, avg=6.4, max=8
; - When 501 <= n <= 750: min=5, avg=6.4, max=8
; - When 751 <= n <= 1000: min=5, avg=6.4, max=8
; - When 1001 <= n <= 2000: min=4, avg=7.4, max=10
; - When 2001 <= n <= 3000: min=6, avg=8.8, max=11
; - When 3001 <= n <= 5000: min=7, avg=10.3, max=11
; - When 5001 <= n <= 7500: min=10, avg=11.0, max=11
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
; 1 to 78
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	ldx $A6
.elseif((n > 3 && n < 7) || n = 23 || n = 25 || n = 28 || n = 30 || n = 33 || n = 35 || n = 38 || n = 40 || n = 47 || n = 48 || n = 52 || n = 53 || n = 55 || n = 58 || n = 60 || n = 63 || n = 65 || n = 68 || n = 70 || n = 72 || n = 73 || n = 75 || n = 77)
	; 2..8 bytes
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
.elseif(n = 9 || n = 11 || n = 13)
	; 3..5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10)
	; 4 bytes
	php
	cmp $C5
	plp
.elseif(n = 12)
	; 4 bytes
	rol $36,X
	ror $36,X
.elseif(n = 14)
	; 4 bytes
	.repeat 2
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
.elseif(n = 16 || n = 18 || n = 20)
	; 5..7 bytes
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
	delay_n 12
	plp
.elseif(n = 21 || n = 26 || n = 36 || n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 22 || n = 29 || n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 24 || n = 32 || n = 34 || n = 39 || n = 44 || n = 49 || n = 54 || n = 59 || n = 67 || n = 69 || n = 74)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 27)
	; 8 bytes
	pha
	tsx
	.repeat 4
	 pla
	.endrepeat
	txs
	pla
.elseif(n = 31 || n = 41 || n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 37)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 42)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	inx
	inx
	Jne *-3
	plp
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
.elseif(n = 62)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	inx
	Jne *-2
	plp
.endif
.else
; 79 to 156
.if(n = 79 || n = 84 || n = 89 || n = 94 || n = 102 || n = 104 || n = 109 || n = 114 || n = 119 || n = 129 || n = 137 || n = 139 || n = 144 || n = 149 || n = 154)
	; 7..8 bytes
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
.elseif(n = 83 || n = 87 || n = 88 || n = 90 || n = 93 || n = 95 || n = 98 || n = 100 || n = 103 || n = 105 || n = 108 || n = 110 || n = 112 || n = 117 || n = 118 || n = 122 || n = 123 || n = 125 || n = 128 || n = 130 || n = 132 || n = 133 || n = 135 || n = 138 || n = 140 || n = 143 || n = 145 || n = 147 || n = 152)
	; 6..8 bytes
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
.if(n = 157 || n = 158 || n = 163 || n = 165 || n = 168 || n = 170 || n = 173 || n = 175 || n = 178 || n = 180 || n = 182 || n = 187 || n = 188 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 202 || n = 203 || n = 205 || n = 208 || n = 210 || n = 212 || n = 213 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233 || n = 235 || n = 237 || n = 238 || n = 240 || n = 243 || n = 245 || n = 247 || n = 248 || n = 250 || n = 252 || n = 257 || n = 258 || n = 263 || n = 268 || n = 270 || n = 273 || n = 275 || n = 278 || n = 280 || n = 282 || n = 283 || n = 285 || n = 292 || n = 293 || n = 297 || n = 298 || n = 300 || n = 303 || n = 305 || n = 308 || n = 310)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 177 || n = 179 || n = 184 || n = 189 || n = 194 || n = 199 || n = 207 || n = 209 || n = 214 || n = 219 || n = 229 || n = 234 || n = 242 || n = 244 || n = 249 || n = 254 || n = 259 || n = 264 || n = 269 || n = 279 || n = 284 || n = 289 || n = 294 || n = 304 || n = 312)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 160 || n = 222 || n = 272)
	; 7..8 bytes
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
.elseif(n = 185 || n = 220 || n = 255 || n = 290)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 287)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 299)
	; 6 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
.endif
.endif
.else
; 313 to 625
.if(n = 313 || n = 315 || n = 317 || n = 318 || n = 320 || n = 322 || n = 327 || n = 328 || n = 332 || n = 333 || n = 335 || n = 338 || n = 340 || n = 342 || n = 343 || n = 345 || n = 348 || n = 350 || n = 353 || n = 355 || n = 357 || n = 362 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 377 || n = 378 || n = 380 || n = 383 || n = 385 || n = 387 || n = 388 || n = 390 || n = 392 || n = 402 || n = 403 || n = 408 || n = 410 || n = 412 || n = 413 || n = 415 || n = 418 || n = 420 || n = 422 || n = 423 || n = 425 || n = 427 || n = 432 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 447 || n = 448 || n = 450 || n = 453 || n = 455 || n = 458 || n = 460 || n = 462 || n = 468 || n = 472 || n = 473 || n = 475 || n = 478 || n = 480 || n = 482 || n = 483 || n = 488 || n = 490 || n = 493 || n = 495 || n = 497 || n = 502 || n = 507 || n = 508 || n = 513 || n = 515 || n = 518 || n = 520 || n = 523 || n = 525 || n = 527 || n = 528 || n = 530 || n = 535 || n = 537 || n = 538 || n = 542 || n = 543 || n = 545 || n = 548 || n = 552 || n = 553 || n = 555 || n = 558 || n = 560 || n = 563 || n = 565 || n = 570 || n = 572 || n = 573 || n = 578 || n = 580 || n = 583 || n = 585 || n = 587 || n = 588 || n = 590 || n = 593 || n = 597 || n = 598 || n = 600 || n = 602 || n = 605 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623 || n = 625)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 314 || n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 347 || n = 349 || n = 354 || n = 359 || n = 364 || n = 369 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 404 || n = 409 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 452 || n = 454 || n = 459 || n = 464 || n = 469 || n = 474 || n = 479 || n = 489 || n = 494 || n = 499 || n = 504 || n = 509 || n = 514 || n = 522 || n = 524 || n = 529 || n = 534 || n = 539 || n = 544 || n = 549 || n = 557 || n = 559 || n = 564 || n = 569 || n = 574 || n = 579 || n = 584 || n = 592 || n = 594 || n = 599 || n = 604 || n = 609 || n = 614 || n = 619)
	; 6..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462) || (n > 475 && n < 497) || (n > 505 && n < 517) || (n > 525 && n < 547) || n = 556 || n = 561 || (n > 570 && n < 582) || n = 596 || (n > 605 && n < 617)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
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
.elseif(n = 325 || n = 360 || n = 395 || n = 430 || n = 465 || n = 500)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 337 || n = 358 || n = 372 || n = 393 || n = 449 || n = 470 || n = 498 || n = 568 || n = 582 || n = 589 || n = 603 || n = 610 || n = 624)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 352 || n = 397 || n = 532 || n = 550 || n = 595 || n = 622)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 398)
	; 5 bytes
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
.elseif(n = 405 || n = 492 || n = 510)
	; 7..8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 417)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
.elseif(n = 457 || n = 517)
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
.elseif(n = 485)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 487 || n = 577)
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
.elseif(n = 562)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 567)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 607)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jmp *+3
	Jmi *-4
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
.elseif(n = 627 || n = 634 || n = 639 || n = 644 || n = 649 || n = 654 || n = 662 || n = 664 || n = 669 || n = 674 || n = 679 || n = 684 || n = 689 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 732 || n = 739 || n = 744 || n = 749 || n = 754 || n = 759 || n = 767 || n = 769 || n = 774 || n = 779 || n = 784 || n = 789 || n = 794 || n = 804 || n = 809 || n = 814 || n = 819 || n = 824 || n = 829 || n = 837 || n = 839 || n = 844 || n = 849 || n = 854 || n = 859 || n = 864 || n = 872 || n = 874 || n = 879 || n = 884 || n = 889 || n = 894 || n = 907 || n = 909 || n = 914 || n = 919 || n = 924 || n = 929 || n = 934)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 628 || n = 630 || n = 632 || n = 633 || n = 635 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 657 || n = 658 || n = 660 || n = 663 || n = 665 || n = 668 || n = 670 || n = 675 || n = 677 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 692 || n = 693 || n = 695 || n = 698 || n = 700 || n = 702 || n = 703 || n = 705 || n = 707 || n = 713 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 737 || n = 738 || n = 740 || n = 742 || n = 747 || n = 752 || n = 753 || n = 758 || n = 760 || n = 762 || n = 763 || n = 765 || n = 768 || n = 770 || n = 772 || n = 773 || n = 777 || n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 798 || n = 800 || n = 803 || n = 805 || n = 808 || n = 810 || n = 812 || n = 818 || n = 822 || n = 823 || n = 825 || n = 828 || n = 830 || n = 833 || n = 835 || n = 838 || n = 840 || n = 842 || n = 843 || n = 845 || n = 852 || n = 853 || n = 857 || n = 858 || n = 860 || n = 863 || n = 867 || n = 868 || n = 870 || n = 873 || n = 875 || n = 878 || n = 880 || n = 882 || n = 887 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 902 || n = 903 || n = 905 || n = 908 || n = 910 || n = 912 || n = 913 || n = 915 || n = 917 || n = 922 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
	plp
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
.elseif(n = 640 || n = 730 || n = 775 || n = 802 || n = 847 || n = 937)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 642 || n = 710 || n = 745 || n = 780 || n = 815 || n = 850 || n = 885 || n = 920)
	; 7..8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 652 || n = 659 || n = 729 || n = 792 || n = 820 || n = 827 || n = 869 || n = 890 || n = 897 || n = 918 || n = 925 || n = 932)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 667 || n = 685 || n = 712 || n = 865 || n = 892)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 672)
	; 8 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-4
.elseif(n = 697 || n = 817 || n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 727)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
.elseif(n = 734 || n = 755)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 748)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
.elseif(n = 782)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 797)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	dex
	Jpl *-4
.elseif(n = 807)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 832)
	; 7 bytes
	pha
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
	pla
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
.if(n = 938 || n = 940 || n = 943 || n = 945 || n = 948 || n = 950 || n = 952 || n = 957 || n = 958 || n = 963 || n = 965 || n = 968 || n = 970 || n = 973 || n = 975 || n = 978 || n = 980 || n = 983 || n = 985 || n = 987 || n = 992 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1020 || n = 1022 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1042 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1062 || n = 1063 || n = 1067 || n = 1068 || n = 1070 || n = 1073 || n = 1075 || n = 1077 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1087 || n = 1088 || n = 1090 || n = 1092 || n = 1097 || n = 1098 || n = 1102 || n = 1103 || n = 1105 || n = 1108 || n = 1110 || n = 1112 || n = 1113 || n = 1115 || n = 1118 || n = 1120 || n = 1123 || n = 1125 || n = 1127 || n = 1132 || n = 1133 || n = 1137 || n = 1138 || n = 1140 || n = 1143 || n = 1145 || n = 1147 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1157 || n = 1158 || n = 1160 || n = 1167 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1182 || n = 1183 || n = 1185 || n = 1188 || n = 1190 || n = 1192 || n = 1193 || n = 1195 || n = 1197 || n = 1202 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1217 || n = 1218 || n = 1220 || n = 1223 || n = 1225 || n = 1227 || n = 1228 || n = 1230 || n = 1232 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248 || n = 1250)
	; 6..8 bytes
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
.elseif(n = 942)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 977 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1012 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1047 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1082 || n = 1084 || n = 1089 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1124 || n = 1129 || n = 1134 || n = 1139 || n = 1144 || n = 1152 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1174 || n = 1179 || n = 1187 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1209 || n = 1214 || n = 1222 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244 || n = 1249)
	; 6..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 947)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 1117 || n = 1207)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1122)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	dex
	Jne *-3
	plp
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
; 1251 to 1406
.if((n .mod 5 = 1) && ((n > 1250 && n < 1277)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 9 = 1) && (n = 1252 || (n > 1296 && n < 1316) || (n > 1332 && n < 1379) || n = 1396 || n = 1405))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1262 || n = 1263 || n = 1265 || n = 1267 || n = 1272 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1287 || n = 1290 || n = 1291 || n = 1293 || n = 1298 || n = 1300 || n = 1302 || n = 1305 || n = 1307 || n = 1308 || n = 1311 || n = 1312 || n = 1314 || n = 1319 || n = 1323 || n = 1326 || n = 1328 || n = 1339 || n = 1340 || n = 1344 || n = 1346 || n = 1347 || n = 1349 || n = 1350 || n = 1353 || n = 1354 || n = 1356 || n = 1361 || n = 1363 || n = 1368 || n = 1370 || n = 1371 || n = 1375 || n = 1377 || n = 1382 || n = 1384 || n = 1386 || n = 1389 || n = 1391 || n = 1395 || n = 1403)
	; 6..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1402)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1257 || n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1292 || n = 1299 || n = 1313 || n = 1318 || n = 1320 || n = 1327 || n = 1334 || n = 1341 || n = 1348 || n = 1355 || n = 1362 || n = 1372 || n = 1379 || n = 1390 || n = 1397 || n = 1402 || n = 1404)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1270 || n = 1335 || n = 1398)
	; 7..8 bytes
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
.elseif(n = 1288 || n = 1304 || n = 1316 || n = 1322 || n = 1358 || n = 1364 || n = 1367 || n = 1383 || n = 1388 || n = 1406)
	; 7..9 bytes
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
	; 9 bytes
	php
	pha
	delay_n (n-14)
	pla
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
.elseif(n = 1309 || n = 1381 || n = 1393)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1321 || n = 1357)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1325 || n = 1330)
	; 9 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 1329 || n = 1337 || n = 1385)
	; 8 bytes
	ldx #(n-1)/8
	jmp *+3
	dex
	Jne *-4
.elseif(n = 1332 || n = 1343)
	; 8 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-4
.elseif(n = 1336 || n = 1392 || n = 1400)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1365)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.endif
.else
; 1407 to 1562
.if(n = 1407 || n = 1410 || n = 1412 || n = 1416 || n = 1417 || n = 1419 || n = 1424 || n = 1426 || n = 1428 || n = 1431 || n = 1433 || n = 1434 || n = 1438 || n = 1440 || n = 1445 || n = 1447 || n = 1452 || n = 1454 || n = 1466 || n = 1469 || n = 1470 || n = 1473 || n = 1475 || n = 1476 || n = 1479 || n = 1480 || n = 1482 || n = 1483 || n = 1487 || n = 1490 || n = 1491 || n = 1494 || n = 1496 || n = 1501 || n = 1503 || n = 1508 || n = 1510 || n = 1515 || n = 1517 || n = 1526 || n = 1529 || n = 1533 || n = 1535 || n = 1536 || n = 1538 || n = 1539 || n = 1542 || n = 1543 || n = 1545 || n = 1550 || n = 1552 || n = 1557 || n = 1559 || n = 1560)
	; 7..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1409 || n = 1442 || n = 1519)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1411 || n = 1418 || n = 1425 || n = 1435 || n = 1439 || n = 1444 || n = 1446 || n = 1458 || n = 1460 || n = 1467 || n = 1474 || n = 1481 || n = 1488 || n = 1498 || n = 1500 || n = 1502 || n = 1509 || n = 1516 || n = 1523 || n = 1530 || n = 1544 || n = 1551)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1413)
	; 9 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442) || (n > 1458 && n < 1505) || (n > 1521 && n < 1559)))
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
.elseif(n = 1421 || n = 1427 || n = 1430 || n = 1448 || n = 1462 || n = 1484 || n = 1493 || n = 1532 || n = 1547 || n = 1556)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1437)
	; 10 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 1449 || n = 1505 || n = 1521 || n = 1553)
	; 8 bytes
	ldx #(n-1)/8
	jmp *+3
	dex
	Jne *-4
.elseif(n = 1451)
	; 9 bytes
	ldx #(n-1)/10
	nop
	jmp *+3
	dex
	Jne *-5
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
.elseif(n = 1456 || n = 1472 || n = 1512 || n = 1528)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1461 || n = 1524)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1463 || n = 1514)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 1489 || n = 1525 || n = 1537 || n = 1561)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1497)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jmi *-4
.elseif(n = 1507)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	inx
	Jne *-5
.elseif(n = 1511)
	; 8 bytes
	pha
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 1518)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1546)
	; 9 bytes
	ldx #102
	rol $26
	ror $26
	dex
	Jpl *-5
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
.if(n = 1563 || n = 1574 || n = 1596 || n = 1651 || n = 1673)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1564 || n = 1566 || n = 1571 || n = 1578 || n = 1580 || n = 1581 || n = 1588 || n = 1589 || n = 1592 || n = 1595 || n = 1599 || n = 1601 || n = 1602 || n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1615 || n = 1620 || n = 1622 || n = 1627 || n = 1629 || n = 1631 || n = 1634 || n = 1636 || n = 1638 || n = 1641 || n = 1643 || n = 1647 || n = 1655 || n = 1658 || n = 1659 || n = 1661 || n = 1662 || n = 1664 || n = 1668 || n = 1669 || n = 1671 || n = 1676 || n = 1678 || n = 1679 || n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1692 || n = 1694 || n = 1697 || n = 1699 || n = 1703 || n = 1704 || (n > 1705 && n < 1709) || n = 1714 || n = 1715 || n = 1718)
	; 7..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1570 || n = 1572 || n = 1579 || n = 1586 || n = 1593 || n = 1600 || n = 1607 || n = 1614 || n = 1626 || n = 1628 || n = 1635 || n = 1640 || n = 1642 || n = 1649 || n = 1656 || n = 1663 || n = 1670 || n = 1677 || n = 1687 || n = 1691 || n = 1698 || n = 1712)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1584 && n < 1622) || (n > 1647 && n < 1694) || n = 1711))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1568 || n = 1584 || n = 1616 || n = 1624 || n = 1672 || n = 1680 || n = 1696)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1717)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1573 || n = 1609 || n = 1633 || n = 1645 || n = 1705 || n = 1717)
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
.elseif(n = 1577 || n = 1665 || n = 1689)
	; 8 bytes
	ldx #(n-1)/8
	jmp *+3
	dex
	Jne *-4
.elseif(n = 1582 || n = 1701)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 1587 || n = 1598 || n = 1650 || n = 1713)
	; 8..9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1591)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1610)
	; 6 bytes
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
.elseif(n = 1617 || n = 1619 || n = 1630 || n = 1644 || n = 1652 || n = 1700)
	; 7..9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1623)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1654)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1710)
	; 10 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1726 || n = 1733 || n = 1740 || n = 1754 || n = 1761 || n = 1768 || n = 1780 || n = 1782 || n = 1796)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 9 = 1) && ((n > 1719 && n < 1757) || (n > 1773 && n < 1793)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1721 || n = 1745)
	; 8 bytes
	ldx #(n-1)/8
	jmp *+3
	dex
	Jne *-4
.elseif(n = 1722 || n = 1725 || n = 1727 || n = 1731 || n = 1732 || n = 1734 || n = 1739 || n = 1743 || n = 1746 || n = 1748 || n = 1749 || n = 1753 || n = 1755 || n = 1757 || n = 1760 || n = 1762 || n = 1764 || n = 1767 || n = 1769 || n = 1770 || n = 1773 || n = 1778 || n = 1781 || n = 1785 || n = 1787 || n = 1788 || n = 1790 || n = 1791 || n = 1795)
	; 7..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724 || n = 1736 || n = 1763)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1728 || n = 1750 || n = 1794)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1735)
	; 9 bytes
	php
	pha
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	pla
	plp
.elseif(n = 1741 || n = 1777 || n = 1789)
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
.elseif(n = 1752 || n = 1784)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1759)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
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
.elseif(n = 1771)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
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
.elseif(n = 1793)
	; 6 bytes
	ldx #0
	nop
	dex
	Jne *-2
.endif
.else
; 1797 to 1875
.if(n = 1797 || n = 1798 || n = 1803 || n = 1812 || n = 1814 || n = 1815 || n = 1821 || n = 1830 || n = 1843 || n = 1848 || n = 1850 || n = 1851 || n = 1857 || n = 1859 || n = 1866 || n = 1869 || n = 1874 || n = 1875)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1800 || n = 1806 || n = 1808 || n = 1820 || n = 1833 || n = 1844 || n = 1845 || n = 1853 || n = 1862 || n = 1868 || n = 1870)
	; 8..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 9 = 1) && ((n > 1800 && n < 1874)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1802 || n = 1804 || n = 1822 || n = 1829 || n = 1852 || n = 1858 || n = 1867)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 11 = 1) && ((n > 1804 && n < 1839) || n = 1860 || n = 1871))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1807)
	; 8 bytes
	ldx #128
	delay_n 9
	dex
	Jpl *-4
.elseif(n = 1809 || n = 1817 || n = 1841 || n = 1865)
	; 8 bytes
	ldx #(n-1)/8
	jmp *+3
	dex
	Jne *-4
.elseif(n = 1811)
	; 9 bytes
	ldx #(n-1)/10
	nop
	jmp *+3
	dex
	Jne *-5
.elseif(n = 1813 || n = 1825 || n = 1849 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1818)
	; 9 bytes
	ldx #234 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jmi *-6
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
.elseif(n = 1831)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1832 || n = 1840 || n = 1856 || n = 1872)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1834 || n = 1847)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 1835 || n = 1863)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1836)
	; 10 bytes
	php
	ldx $A6
	delay_n (n-10)
	plp
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
.elseif(n = 1854)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
; 1876 to 1953
.if(n = 1876 || n = 1906 || n = 1951)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 1877 || n = 1905 || n = 1919)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1878 || n = 1880 || n = 1892 || n = 1898 || n = 1907 || n = 1916 || n = 1925 || n = 1934 || n = 1940 || n = 1943 || n = 1952)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1879 || n = 1883 || n = 1884 || n = 1886 || n = 1887 || n = 1890 || n = 1899 || n = 1901 || n = 1902 || n = 1911 || n = 1917 || n = 1920 || n = 1929 || n = 1931 || n = 1935 || n = 1938 || n = 1946 || n = 1947 || n = 1949)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1881 || n = 1889 || n = 1913 || n = 1953)
	; 8 bytes
	ldx #(n-1)/8
	jmp *+3
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1946)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1885 || n = 1897 || n = 1921 || n = 1933)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1888 || n = 1896 || n = 1912 || n = 1928 || n = 1944)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif((n .mod 11 = 1) && ((n > 1892 && n < 1949)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1894 || n = 1903 || n = 1924 || n = 1930)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1895)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1908)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1910)
	; 10 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 1914)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1922)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 1923)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1932)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1939)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 1941)
	; 9 bytes
	ldx #(n-1)/10
	nop
	jmp *+3
	dex
	Jne *-5
.elseif(n = 1942)
	; 9 bytes
	pha
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
.elseif(n = 1950)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
	plp
.endif
.else
; 1954 to 2031
.if((n .mod 9 = 1) && ((n > 1953 && n < 2027)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1955 || n = 1956 || n = 1958 || n = 1962 || n = 1965 || n = 1971 || n = 1974 || n = 1983 || n = 1991 || n = 1995 || n = 2001 || n = 2004 || n = 2007 || n = 2010 || n = 2013 || n = 2019 || n = 2021 || n = 2027 || n = 2028)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1957 || n = 1969 || n = 1993 || n = 2005 || n = 2029)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 11 = 1) && (n = 1959 || n = 1970 || (n > 1991 && n < 2026)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1960 || n = 1976 || n = 1984 || n = 2000 || n = 2016)
	; 9 bytes
	php
	ldx #n/8-1
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 1961 || n = 1975 || n = 1989 || n = 2031)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 1964 || n = 1979 || n = 1982 || n = 1997 || n = 2006 || n = 2012 || n = 2022 || n = 2024)
	; 9..10 bytes
	php
	delay_n (n-7)
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
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 1973)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 1977 || n = 1985 || n = 2009)
	; 8 bytes
	ldx #(n-1)/8
	jmp *+3
	dex
	Jne *-4
.elseif(n = 1978 || n = 1998 || n = 2018)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 1980 || n = 2002 || n = 2020)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1986)
	; 9 bytes
	pha
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
	pla
.elseif(n = 1987)
	; 9 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
	plp
.elseif(n = 1988)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 1994)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1996 || n = 2011)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2015)
	; 8 bytes
	pha
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
	pla
.elseif(n = 2023)
	; 8 bytes
	pha
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
	pla
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
.elseif(n = 2033)
	; 8 bytes
	ldx #(n-1)/8
	jmp *+3
	dex
	Jne *-4
.elseif(n = 2034)
	; 9 bytes
	ldx #(n-20)/19
	delay_n 14
	dex
	Jpl *-5
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2108)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 11 = 1) && ((n > 2035 && n < 2070) || n = 2091 || n = 2102))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2037 || n = 2039 || n = 2043 || n = 2046 || n = 2050 || n = 2055 || n = 2057 || n = 2061 || n = 2063 || n = 2064 || n = 2066 || n = 2067 || n = 2070 || n = 2073 || n = 2079 || n = 2082 || n = 2095 || n = 2100 || n = 2103 || n = 2104 || n = 2106 || n = 2109)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2038 || n = 2074 || n = 2081 || n = 2083 || n = 2090 || n = 2108)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2041 || n = 2065 || n = 2077 || n = 2101)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2042 || n = 2051 || n = 2052 || n = 2054 || n = 2060 || n = 2072 || n = 2076 || n = 2084 || n = 2085 || n = 2094 || n = 2096)
	; 8..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2045 || n = 2059)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2049)
	; 8 bytes
	ldx #0
	jmp *+3
	dex
	Jne *-4
.elseif(n = 2056)
	; 9 bytes
	php
	ldx #0
	bit $24
	dex
	Jne *-3
	plp
.elseif(n = 2068)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2075 || n = 2092)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 2078)
	; 6 bytes
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
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
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif(n = 2093)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2097)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2099)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 12
	inx
	inx
	Jne *-7
.elseif(n = 2105)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2119 || n = 2128 || n = 2140 || n = 2155 || n = 2164 || n = 2182)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2111 || n = 2118 || n = 2127 || n = 2136 || n = 2138 || n = 2139 || n = 2145 || n = 2147 || n = 2148 || n = 2151 || n = 2153 || n = 2154 || n = 2162 || n = 2163 || n = 2165 || n = 2169 || n = 2172 || n = 2174 || n = 2175 || n = 2181 || n = 2183 || n = 2184 || n = 2187)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2112 || n = 2114 || n = 2120 || n = 2122 || n = 2123 || n = 2132 || n = 2141 || n = 2142 || n = 2144 || n = 2150 || n = 2156 || n = 2159 || n = 2177 || n = 2180 || n = 2186)
	; 8..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2113 || n = 2137 || n = 2149 || n = 2173 || n = 2185)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2115 || n = 2129 || n = 2171)
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
.elseif(n = 2121)
	; 8 bytes
	pha
	ldx #21 ;hides 'ora zp,x'
	nop
	inx
	Jne *-3
	pla
.elseif((n .mod 11 = 1) && ((n > 2123 && n < 2169)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2126 || n = 2160)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2130)
	; 8 bytes
	pha
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
	pla
.elseif(n = 2131 || n = 2176)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2133)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2158 || n = 2178)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
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
	ldx #(n-20)/19
	delay_n 14
	dex
	Jpl *-5
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
.elseif(n = 2189 || n = 2203 || n = 2208 || n = 2210 || n = 2211 || n = 2214 || n = 2217 || n = 2219 || n = 2225 || n = 2226 || n = 2229 || n = 2235 || n = 2237 || n = 2244 || n = 2246 || n = 2247 || n = 2253 || n = 2259 || n = 2261 || n = 2262)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 11 = 1) && ((n > 2189 && n < 2235) || n = 2256))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2191 || n = 2236)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2192 || n = 2195 || n = 2204 || n = 2216 || n = 2220 || n = 2222 || n = 2228 || n = 2231 || n = 2239 || n = 2240 || n = 2249 || n = 2252 || n = 2258 || n = 2264)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2193)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2194)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 2196)
	; 9 bytes
	pha
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
	pla
.elseif(n = 2198 || n = 2250)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2199)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2200 || n = 2202 || n = 2218 || n = 2254 || n = 2263)
	; 9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif((n .mod 14 = 1) && ((n > 2212 && n < 2256)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
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
.elseif(n = 2232)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
.elseif(n = 2238)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2243)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2248)
	; 9 bytes
	php
	bit <$2C ;hides 'bit $07A2'
	ldx #7
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
.elseif(n = 2267 || n = 2289 || n = 2300)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2268)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif((n .mod 9 = 1) && ((n > 2268 && n < 2297)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2270 || n = 2272 || n = 2290 || n = 2299)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2271 || n = 2273 || n = 2280 || n = 2282 || n = 2295 || n = 2298)
	; 8..9 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2276 || n = 2285 || n = 2288 || n = 2294 || n = 2303 || n = 2304)
	; 9..10 bytes
	php
	delay_n (n-7)
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
.elseif(n = 2279)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
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
.elseif(n = 2291)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2292)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	inx
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
.if(n = 2305)
	; 7 bytes
	ldx #0
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2306 || n = 2316)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2307 || n = 2309 || n = 2314 || n = 2319 || n = 2321 || n = 2323 || n = 2327 || n = 2331 || n = 2334 || n = 2335 || n = 2343)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2320 || n = 2332 || n = 2342)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2310)
	; 9 bytes
	php
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	dex
	Jne *-4
	plp
.elseif(n = 2311 || n = 2322 || n = 2333)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2312 || n = 2318 || n = 2324 || n = 2336 || n = 2340)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2313 || n = 2330)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2315 || n = 2328)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2317 || n = 2329 || n = 2341)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2325 || n = 2339)
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
.endif
.endif
.endif
.else
; 2344 to 2500
.if((n .mod 11 = 1) && ((n > 2343 && n < 2367) || (n > 2387 && n < 2499)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2345 || n = 2350 || n = 2357 || n = 2363 || n = 2368 || n = 2379 || n = 2383 || n = 2387 || n = 2390 || n = 2391 || n = 2394 || n = 2397 || n = 2400 || n = 2403 || n = 2405 || n = 2407 || n = 2411 || n = 2412 || n = 2415 || n = 2417 || n = 2422 || n = 2427 || n = 2429 || n = 2434 || n = 2436 || n = 2439 || n = 2441 || n = 2455 || n = 2460 || n = 2463 || n = 2467 || n = 2469 || n = 2470 || n = 2475 || n = 2477 || n = 2478 || n = 2482 || n = 2489 || n = 2495 || n = 2499)
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
.elseif(n = 2347 || n = 2364 || n = 2398 || n = 2466 || n = 2483 || n = 2500)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2348 || n = 2360 || n = 2362 || (n > 2371 && n < 2375) || n = 2384 || n = 2396 || n = 2402 || n = 2408 || n = 2420 || n = 2430 || n = 2442 || n = 2444 || n = 2450 || n = 2456 || n = 2468 || n = 2472 || n = 2480 || n = 2492 || n = 2494)
	; 8..10 bytes
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
.elseif(n = 2351 || n = 2361)
	; 9 bytes
	ldx #(n-1)/10
	nop
	jmp *+3
	dex
	Jne *-5
.elseif(n = 2352 || n = 2375 || n = 2459 || n = 2462 || n = 2474)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif((n .mod 12 = 1) && ((n > 2352 && n < 2498)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 13 = 1) && (n = 2354 || (n > 2379 && n < 2420) || (n > 2444 && n < 2485)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2356 || n = 2371 || n = 2386 || n = 2416 || n = 2431 || n = 2446 || n = 2491)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2358 || n = 2378 || n = 2418 || n = 2448)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
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
.elseif((n .mod 14 = 1) && ((n > 2366 && n < 2424) || n = 2451 || n = 2479 || n = 2493))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2369 || n = 2453)
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
.elseif(n = 2376 || n = 2414 || n = 2490)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2382)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2385 || n = 2433 || n = 2481)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2392 || n = 2404 || n = 2424 || n = 2426 || n = 2428 || n = 2438 || n = 2440 || n = 2452 || n = 2457 || n = 2464 || n = 2488 || n = 2496)
	; 8..10 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 2486)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	bit $24
	dex
	Jne *-4
	plp
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
.if((n > 2500 && n < 2504) || n = 2511 || n = 2513 || n = 2519 || n = 2522 || n = 2523 || n = 2525 || n = 2530 || n = 2539 || n = 2544 || n = 2546 || n = 2547 || n = 2555 || n = 2559 || n = 2561 || n = 2565 || n = 2571 || n = 2578)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2505 || n = 2514 || n = 2516 || n = 2528 || n = 2538 || n = 2552 || n = 2556 || n = 2558 || n = 2570 || n = 2576)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2506 || n = 2536 || n = 2566)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && (n = 2507 || (n > 2534 && n < 2578)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2508 || n = 2518)
	; 10 bytes
	php
	ldx #(n-8)/10
	nop
	cmp $C5
	dex
	Jne *-4
	plp
.elseif((n .mod 12 = 1) && ((n > 2508 && n < 2570)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2510 || n = 2562)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2512 || n = 2524 || n = 2543 || n = 2548 || n = 2554 || n = 2560 || n = 2567 || n = 2572)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2515)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2517 || n = 2534 || n = 2568)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif((n .mod 11 = 1) && ((n > 2519 && n < 2576)))
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
.elseif(n = 2529)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2532)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
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
.elseif(n = 2541)
	; 9 bytes
	ldx #(n-1)/10
	nop
	jmp *+3
	dex
	Jne *-5
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
.elseif(n = 2573)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
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
; 2579 to 2656
.if(n = 2579 || n = 2583 || n = 2587 || n = 2592 || n = 2595 || n = 2603 || n = 2607 || n = 2609 || n = 2610 || n = 2613 || n = 2616 || n = 2621 || n = 2628 || n = 2631 || n = 2632 || n = 2634 || n = 2635 || n = 2643 || n = 2645 || (n > 2648 && n < 2652) || n = 2654 || n = 2655)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2580 || n = 2588 || n = 2598 || n = 2600 || n = 2606 || n = 2612 || n = 2615 || n = 2624 || n = 2636 || n = 2637 || n = 2648)
	; 9..10 bytes
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
.elseif(n = 2584 || n = 2589 || n = 2594 || n = 2620 || n = 2622 || n = 2644)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2585 || n = 2602)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif((n .mod 11 = 1) && ((n > 2585 && n < 2631) || n = 2652))
	; 8 bytes
	ldx #(n-1)/11
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
.elseif(n = 2591 || n = 2633 || n = 2647)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2596 || n = 2611 || n = 2626 || n = 2656)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-5
.elseif((n .mod 13 = 1) && ((n > 2600 && n < 2641)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2604 || n = 2623 || n = 2642)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2618 || n = 2639)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2625)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2638)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
.elseif(n = 2646)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	inx
	Jne *-5
	plp
.endif
.endif
.else
; 2657 to 2812
.if(n = 2657 || n = 2658 || n = 2662 || n = 2667 || n = 2669 || n = 2676 || n = 2678 || n = 2679 || n = 2681 || n = 2683 || n = 2688 || n = 2691 || (n > 2692 && n < 2696) || n = 2698 || n = 2700 || n = 2706 || n = 2709 || n = 2711 || n = 2715 || n = 2719 || n = 2722 || n = 2723 || n = 2727 || n = 2730 || n = 2733 || n = 2735 || n = 2739 || n = 2741 || n = 2742 || n = 2747 || n = 2748 || n = 2753 || (n > 2762 && n < 2766) || n = 2775 || n = 2777 || n = 2782 || n = 2786 || n = 2793 || n = 2803 || n = 2805 || n = 2808 || n = 2810 || n = 2811)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2659 || n = 2660 || n = 2672 || n = 2682 || n = 2684 || n = 2708 || n = 2710 || n = 2714 || n = 2720 || n = 2724 || n = 2732 || n = 2736 || n = 2744 || n = 2756 || n = 2758 || n = 2766 || n = 2768 || n = 2780 || n = 2792 || n = 2804)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 14 = 1) && (n = 2661 || n = 2675 || (n > 2702 && n < 2760) || n = 2787 || n = 2801))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2662 && n < 2763) || (n > 2783 && n < 2807)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2664 || n = 2668 || n = 2680 || n = 2728 || n = 2752 || n = 2754 || n = 2788 || n = 2798 || n = 2800 || n = 2802 || n = 2812)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 12 = 1) && ((n > 2664 && n < 2810)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 13 = 1) && (n = 2666 || n = 2692 || n = 2705 || (n > 2756 && n < 2797)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2670 || n = 2687 || n = 2704 || n = 2721 || n = 2738 || n = 2755 || n = 2772 || n = 2789)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2671 || n = 2686 || n = 2716 || n = 2746 || n = 2776 || n = 2791)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2673 || n = 2769)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2690)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 14
	dex
	dex
	Jne *-7
.elseif(n = 2697 || n = 2702 || n = 2712 || n = 2750 || n = 2760 || n = 2781)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2699 || n = 2794)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
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
.elseif(n = 2734)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 4
	dex
	Jne *-4
	plp
.elseif(n = 2743)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	delay_n 6
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
.elseif(n = 2771)
	; 9 bytes
	ldx #232 ;hides 'inx'
	jmp *+3
	dex
	dex
	Jne *-6
.elseif(n = 2774)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
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
.elseif(n = 2790)
	; 10 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	cmp $C5
	inx
	Jne *-5
	plp
.elseif(n = 2799)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2807)
	; 9 bytes
	ldx #234 ;hides 'nop'
	nop
	jmp *+3
	dex
	Jne *-6
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
; 2813 to 2968
.if(n = 2813 || n = 2818 || n = 2819 || n = 2823 || (n > 2824 && n < 2828) || n = 2830 || n = 2831 || n = 2834 || n = 2835 || (n > 2836 && n < 2840) || n = 2842 || n = 2847 || n = 2849 || n = 2853 || n = 2854 || n = 2859 || n = 2862 || n = 2867 || n = 2868 || n = 2875 || n = 2883 || n = 2886 || n = 2890 || n = 2895 || n = 2897 || n = 2898 || (n > 2900 && n < 2904) || n = 2907 || n = 2909 || n = 2910 || n = 2914 || n = 2915 || n = 2919 || (n > 2920 && n < 2924) || n = 2928 || n = 2931 || n = 2933 || n = 2935 || n = 2938 || n = 2943 || n = 2945 || n = 2950 || n = 2954 || n = 2958 || n = 2963 || n = 2967)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2814)
	; 10 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif((n .mod 14 = 1) && ((n > 2814 && n < 2844) || (n > 2870 && n < 2928) || n = 2955))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2816 || n = 2828 || n = 2840 || n = 2848 || n = 2850 || n = 2852 || n = 2864 || n = 2876 || n = 2878 || n = 2888 || n = 2892 || n = 2900 || n = 2906 || n = 2912 || n = 2924 || n = 2934 || n = 2936 || n = 2948 || n = 2960 || n = 2962)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2817)
	; 8 bytes
	ldx #0
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2820 || n = 2824 || n = 2844 || n = 2846 || n = 2860 || n = 2872 || n = 2884 || n = 2916 || n = 2920 || n = 2930 || n = 2932 || n = 2944 || n = 2968)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 12 = 1) && ((n > 2820 && n < 2966)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2822 || n = 2861 || n = 2887 || n = 2939 || n = 2952)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2832 || n = 2870 || n = 2889 || n = 2946)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 2836 || n = 2851 || n = 2866 || n = 2896 || n = 2911 || n = 2926 || n = 2956)
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2841)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
.elseif(n = 2855 || n = 2856 || n = 2858 || n = 2877 || n = 2879 || n = 2880 || n = 2882 || n = 2894 || n = 2904 || n = 2937 || n = 2940 || n = 2949 || n = 2951 || n = 2964 || n = 2966)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2863)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
.elseif(n = 2865 || n = 2961)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 2873 || n = 2957)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2874 || n = 2891 || n = 2908 || n = 2925 || n = 2942 || n = 2959)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2918)
	; 10 bytes
	pha
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	pla
	pha
	dex
	Jne *-5
	pla
.elseif(n = 2947)
	; 10 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.endif
.else
; 2969 to 3125
.if((n .mod 14 = 1) && ((n > 2968 && n < 3012) || (n > 3038 && n < 3124)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2970 || n = 2973 || n = 2974 || n = 2979 || n = 2981 || n = 2985 || n = 2987 || n = 2988 || n = 2991 || n = 2994 || n = 2995 || n = 2998 || n = 3003 || (n > 3004 && n < 3008) || n = 3012 || n = 3019 || n = 3024 || n = 3027 || n = 3029 || n = 3033 || n = 3034 || n = 3045 || n = 3051 || n = 3054 || n = 3055 || n = 3058 || n = 3059 || n = 3062 || n = 3063 || n = 3065 || n = 3066 || n = 3070 || n = 3071 || n = 3075 || n = 3077 || (n > 3082 && n < 3086) || n = 3093 || n = 3097 || n = 3099 || n = 3100 || n = 3107 || n = 3110 || n = 3111 || n = 3113 || n = 3114 || n = 3119 || n = 3125)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 15 = 1) && (n = 2971 || n = 2986 || (n > 3015 && n < 3047) || (n > 3075 && n < 3122)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 2972 || n = 2984 || n = 2990 || n = 2996 || n = 3008 || n = 3018 || n = 3020 || n = 3032 || n = 3048 || n = 3056 || n = 3068 || n = 3074 || n = 3080 || n = 3088 || n = 3102 || n = 3116)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2975 || n = 3021 || n = 3023 || n = 3026 || n = 3035 || n = 3036 || n = 3038 || n = 3050 || n = 3072 || n = 3086 || n = 3087 || n = 3090 || n = 3092 || n = 3094 || n = 3096 || n = 3101 || n = 3103 || n = 3104 || n = 3118 || n = 3120 || n = 3122 || n = 3124)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2976 || n = 2993 || n = 3010 || n = 3078 || n = 3112)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif((n .mod 12 = 1) && ((n > 2976 && n < 3062)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2978 || n = 3004 || n = 3017 || n = 3043 || n = 3069 || n = 3082 || n = 3108)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2980 || n = 2992 || n = 3000 || n = 3014 || n = 3028 || n = 3040 || n = 3047 || n = 3052 || n = 3064)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2982)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	cmp $C5
	dex
	Jne *-5
	plp
.elseif(n = 2999)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3002)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 3009 || n = 3057 || n = 3089 || n = 3105)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3015)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3022 || n = 3060 || n = 3079 || n = 3098 || n = 3117)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3030)
	; 9 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3041)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3042)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	inx
	inx
	Jne *-7
.elseif(n = 3044)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3115)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if(n = 3126 || n = 3140 || n = 3154 || n = 3202 || n = 3210 || n = 3224 || n = 3252 || n = 3278 || n = 3280)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3127 || n = 3131 || n = 3138 || n = 3139 || n = 3141 || n = 3148 || n = 3149 || n = 3157 || n = 3162 || n = 3167 || n = 3168 || n = 3171 || n = 3175 || n = 3176 || n = 3182 || n = 3183 || n = 3188 || n = 3195 || n = 3198 || n = 3203 || n = 3209 || n = 3213 || n = 3216 || n = 3219 || n = 3223 || n = 3227 || n = 3237 || n = 3239 || n = 3240 || n = 3243 || n = 3253 || n = 3258 || n = 3266 || n = 3267 || n = 3273 || n = 3279)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3128 || n = 3132 || n = 3135 || n = 3142 || n = 3143 || n = 3145 || n = 3150 || n = 3152 || n = 3156 || n = 3159 || n = 3161 || n = 3164 || n = 3170 || n = 3177 || n = 3178 || n = 3184 || (n > 3188 && n < 3193) || n = 3194 || n = 3204 || n = 3208 || n = 3215 || n = 3218 || n = 3220 || n = 3222 || n = 3229 || n = 3232 || n = 3234 || n = 3236 || (n > 3243 && n < 3247) || n = 3255 || n = 3257 || (n > 3259 && n < 3263) || n = 3268 || n = 3272 || n = 3274 || n = 3276)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3129 || n = 3146 || n = 3163 || n = 3180 || n = 3197 || n = 3214 || n = 3231 || n = 3248 || n = 3265)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3130 || n = 3144 || n = 3158 || n = 3172 || n = 3200 || n = 3206 || n = 3228 || n = 3242 || n = 3254 || n = 3270)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3133 || n = 3187 || n = 3205 || n = 3259)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif((n .mod 13 = 1) && ((n > 3133 && n < 3187) || (n > 3211 && n < 3265)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif((n .mod 15 = 1) && (n = 3136 || (n > 3165 && n < 3272)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 3136 && n < 3278)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 16 = 1) && ((n > 3152 && n < 3234) || n = 3281))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3155 || n = 3174 || n = 3250 || n = 3269)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3199)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3230)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 6
	dex
	Jne *-5
	plp
.elseif(n = 3247)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.endif
.else
; 3282 to 3437
.if(n = 3282 || n = 3284 || n = 3298 || n = 3312 || n = 3340 || n = 3354 || n = 3368 || n = 3382 || n = 3396 || n = 3410 || n = 3424)
	; 9..10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3283 || n = 3293 || n = 3295 || n = 3315 || n = 3318 || n = 3321 || n = 3323 || n = 3328 || n = 3335 || n = 3337 || n = 3348 || n = 3349 || n = 3351 || n = 3352 || n = 3363 || n = 3365 || n = 3366 || n = 3369 || n = 3378 || n = 3386 || n = 3395 || n = 3404 || n = 3405 || n = 3407 || n = 3408 || n = 3411 || n = 3419 || n = 3420 || n = 3423 || n = 3433 || n = 3437)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285 || n = 3287 || n = 3289 || n = 3296 || n = 3300 || n = 3302 || n = 3304 || n = 3306 || n = 3310 || n = 3311 || n = 3314 || n = 3317 || n = 3320 || n = 3324 || n = 3325 || n = 3327 || n = 3330 || n = 3332 || n = 3334 || n = 3338 || n = 3339 || (n > 3340 && n < 3345) || n = 3353 || (n > 3354 && n < 3361) || n = 3362 || (n > 3369 && n < 3375) || n = 3380 || n = 3381 || n = 3387 || n = 3388 || n = 3390 || n = 3394 || n = 3397 || n = 3413 || n = 3415 || n = 3416 || n = 3422 || n = 3426 || n = 3428 || n = 3429 || n = 3432)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif((n .mod 15 = 1) && ((n > 3285 && n < 3347) || (n > 3375 && n < 3437)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3288 || n = 3307 || n = 3326 || n = 3364 || n = 3383 || n = 3402)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3290 || n = 3303)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif((n .mod 14 = 1) && ((n > 3290 && n < 3432)))
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
.elseif(n = 3294 || n = 3308 || n = 3336 || n = 3392 || n = 3434)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 16 = 1) && (n = 3297 || n = 3313 || n = 3345 || (n > 3376 && n < 3426)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3299 || n = 3350 || n = 3367 || n = 3384 || n = 3401 || n = 3418 || n = 3435)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3309 || n = 3379)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3322)
	; 10 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
	plp
.elseif(n = 3329)
	; 9 bytes
	ldx #0
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3385)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3398)
	; 10 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3399)
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
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
; 3438 to 3593
.if((n .mod 14 = 8) && (n = 3438 || (n > 3479 && n < 3579)))
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3439 || n = 3493 || n = 3565 || n = 3583)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3440 || n = 3497 || n = 3516 || n = 3535 || n = 3573 || n = 3592)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3441 || n = 3457 || n = 3489 || n = 3505 || n = 3521 || n = 3553 || n = 3569)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3442)
	; 10 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
	plp
.elseif(n = 3443 || n = 3447 || n = 3449 || n = 3453 || n = 3454 || n = 3461 || n = 3463 || n = 3468 || n = 3471 || n = 3475 || n = 3477 || n = 3483 || n = 3488 || n = 3498 || n = 3499 || n = 3507 || n = 3513 || n = 3517 || n = 3519 || n = 3523 || n = 3528 || n = 3531 || n = 3533 || n = 3539 || n = 3545 || n = 3547 || n = 3555 || n = 3558 || n = 3559 || n = 3561 || n = 3587 || n = 3589 || n = 3590)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3444 || n = 3446 || n = 3455 || n = 3456 || n = 3458 || n = 3460 || n = 3465 || n = 3467 || n = 3470 || n = 3472 || n = 3474 || n = 3482 || n = 3484 || n = 3485 || n = 3492 || n = 3495 || n = 3500 || n = 3502 || n = 3506 || n = 3510 || n = 3512 || n = 3514 || n = 3524 || n = 3525 || n = 3527 || n = 3530 || n = 3534 || n = 3538 || n = 3540 || n = 3542 || n = 3544 || n = 3548 || n = 3549 || n = 3551 || n = 3552 || n = 3562 || n = 3563 || (n > 3565 && n < 3569) || n = 3570 || n = 3572 || n = 3576 || n = 3577 || (n > 3579 && n < 3583) || n = 3584 || n = 3591 || n = 3593)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif((n .mod 14 = 1) && ((n > 3444 && n < 3572)))
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif((n .mod 14 = 4) && (n = 3448 || (n > 3475 && n < 3575)))
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3450)
	; 9 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 10
	Jne *-6
.elseif((n .mod 15 = 1) && ((n > 3450 && n < 3557) || n = 3586))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3452 || n = 3469 || n = 3486 || n = 3503 || n = 3520 || n = 3537 || n = 3554 || n = 3588)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3462)
	; 10 bytes
	php
	ldx #216 ;hides 'cld'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3464)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
.elseif(n = 3478)
	; 9 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
	plp
.elseif(n = 3479)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3491 || n = 3575)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3509)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3579)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.endif
.else
; 3594 to 3750
.if(n = 3594 || n = 3603 || n = 3607 || n = 3618 || n = 3624 || n = 3632 || n = 3635 || n = 3641 || n = 3648 || n = 3651 || n = 3658 || n = 3663 || n = 3667 || n = 3670 || n = 3675 || n = 3678 || n = 3683 || n = 3689 || n = 3692 || n = 3693 || n = 3699 || n = 3708 || n = 3709 || n = 3715 || n = 3723 || n = 3726 || n = 3731 || n = 3738 || n = 3746 || n = 3747)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 3594 && n < 3599) || n = 3600 || n = 3602 || n = 3604 || n = 3606 || (n > 3607 && n < 3611) || n = 3612 || n = 3614 || n = 3615 || n = 3620 || n = 3621 || n = 3623 || (n > 3624 && n < 3630) || n = 3636 || n = 3638 || n = 3640 || (n > 3641 && n < 3646) || n = 3647 || n = 3650 || n = 3652 || n = 3654 || n = 3657 || n = 3659 || n = 3660 || n = 3662 || n = 3664 || n = 3666 || n = 3669 || n = 3671 || n = 3672 || n = 3674 || n = 3677 || n = 3679 || n = 3680 || n = 3682 || (n > 3683 && n < 3687) || n = 3688 || n = 3694 || n = 3696 || n = 3700 || n = 3701 || (n > 3702 && n < 3706) || n = 3710 || n = 3712 || n = 3716 || n = 3717 || n = 3719 || n = 3720 || n = 3722 || n = 3728 || n = 3730 || (n > 3731 && n < 3736) || n = 3737 || n = 3740 || n = 3742 || n = 3748 || n = 3749)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3599)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-6
.elseif((n .mod 15 = 1) && ((n > 3600 && n < 3737)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3605 || n = 3622 || n = 3639 || n = 3656 || n = 3673 || n = 3690 || n = 3707 || n = 3724 || n = 3741)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3611 || n = 3630 || n = 3668 || n = 3687 || n = 3725 || n = 3744)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 3613 || n = 3634 || n = 3739)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif((n .mod 16 = 1) && ((n > 3616 && n < 3746)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3619 || n = 3637 || n = 3655 || n = 3727)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3653)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
.elseif(n = 3695)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 3698 || n = 3714)
	; 10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3702 || n = 3718 || n = 3750)
	; 10 bytes
	php
	delay_n (n-7)
	plp
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
.endif
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
; 3751 to 4062
.if((n .mod 15 = 1) && ((n > 3750 && n < 3827)))
	; 9 bytes
	ldx #(n-1)/15
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3752 || (n > 3753 && n < 3757) || n = 3759 || n = 3762 || n = 3764 || n = 3767 || (n > 3768 && n < 3772) || n = 3773 || n = 3774 || n = 3776 || n = 3778 || n = 3780 || n = 3785 || n = 3786 || (n > 3787 && n < 3792) || n = 3797 || n = 3800 || (n > 3803 && n < 3809) || n = 3810 || n = 3812 || (n > 3813 && n < 3817) || n = 3818 || n = 3819 || n = 3821 || n = 3824 || (n > 3828 && n < 3835) || (n > 3835 && n < 3839) || n = 3840 || n = 3842 || (n > 3845 && n < 3853) || n = 3854 || n = 3855 || n = 3861 || n = 3864 || (n > 3865 && n < 3871) || n = 3872 || n = 3874 || n = 3876 || n = 3878 || (n > 3879 && n < 3886) || n = 3887 || n = 3888 || n = 3890 || n = 3892 || n = 3893 || n = 3895 || n = 3897 || (n > 3898 && n < 3905) || n = 3906 || (n > 3907 && n < 3911) || n = 3912 || n = 3914 || n = 3916 || (n > 3917 && n < 3921) || n = 3922 || n = 3924 || n = 3927 || n = 3929 || (n > 3930 && n < 3934) || n = 3935 || n = 3938 || (n > 3939 && n < 3943) || n = 3944 || n = 3946 || n = 3948 || (n > 3949 && n < 3953) || n = 3954 || (n > 3955 && n < 3961) || n = 3963 || (n > 3964 && n < 3969) || n = 3973 || n = 3975 || n = 3976 || n = 3978 || n = 3980 || (n > 3981 && n < 3985) || n = 3986 || (n > 3987 && n < 3991) || n = 3992 || n = 3994 || n = 3995 || n = 4000 || n = 4002 || (n > 4003 && n < 4010) || n = 4011 || n = 4014 || n = 4016 || n = 4018 || (n > 4019 && n < 4029) || n = 4034 || (n > 4035 && n < 4047) || n = 4052 || n = 4053 || (n > 4054 && n < 4059) || (n > 4059 && n < 4063))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3753 || n = 3760 || n = 3765 || n = 3768 || n = 3779 || n = 3783 || n = 3784 || n = 3794 || n = 3795 || n = 3798 || n = 3803 || n = 3813 || n = 3822 || n = 3827 || n = 3828 || n = 3845 || n = 3859 || n = 3862 || n = 3875 || n = 3879 || n = 3891 || n = 3898 || n = 3913 || n = 3917 || n = 3923 || n = 3930 || n = 3936 || n = 3939 || n = 3947 || n = 3955 || n = 3964 || n = 3971 || n = 3981 || n = 3987 || n = 3993 || n = 3998 || n = 4003 || n = 4015 || n = 4019 || n = 4031 || n = 4032 || n = 4035 || n = 4050)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3758 || n = 3775 || n = 3792 || n = 3809 || n = 3843 || n = 3860 || n = 3877 || n = 3894 || n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3979 || n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif((n .mod 16 = 1) && ((n > 3760 && n < 3794) || n = 3825 || (n > 3856 && n < 4050)))
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3763 || n = 3782 || n = 3801 || n = 3820 || n = 3839 || n = 3858 || n = 3896 || n = 3915 || n = 3934 || n = 3972 || n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
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
.elseif(n = 3799)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 12
	inx
	Jne *-6
.elseif(n = 3802 || n = 3823 || n = 3844 || n = 3865 || n = 3886 || n = 3949 || n = 3970 || n = 4012 || n = 4054)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 3817 || n = 3835 || n = 3853 || n = 3871 || n = 3907 || n = 3925 || n = 3961 || n = 3997 || n = 4051)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 3841)
	; 9 bytes
	ldx #0
	rol $26
	ror $26
	dex
	Jne *-5
.elseif(n = 3856)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
.elseif(n = 3863)
	; 10 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3926)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-7
.elseif(n = 3943)
	; 9 bytes
	ldx #24 ;hides 'clc'
	rol $26
	ror $26
	inx
	Jne *-6
.elseif(n = 3974)
	; 10 bytes
	php
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
	plp
.elseif(n = 3977)
	; 9 bytes
	ldx #234 ;hides 'nop'
	rol $26
	ror $26
	dex
	Jne *-6
.elseif(n = 3999)
	; 10 bytes
	ldx #56 ;hides 'sec'
	delay_n 13
	inx
	Jne *-7
.elseif(n = 4059)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 8
	Jne *-7
.endif
.else
; 4063 to 4375
.if(n = 4063 || n = 4068 || (n > 4069 && n < 4075) || (n > 4075 && n < 4081) || n = 4082 || n = 4084 || n = 4085 || (n > 4088 && n < 4096) || (n > 4100 && n < 4105) || n = 4106 || (n > 4107 && n < 4115) || (n > 4117 && n < 4123) || n = 4125 || (n > 4126 && n < 4132) || n = 4133 || (n > 4134 && n < 4138) || n = 4139 || n = 4140 || n = 4142 || n = 4144 || (n > 4145 && n < 4149) || n = 4150 || (n > 4151 && n < 4157) || n = 4158 || n = 4160 || n = 4161 || n = 4163 || n = 4165 || n = 4167 || (n > 4168 && n < 4175) || n = 4176 || n = 4178 || n = 4179 || n = 4182 || n = 4184 || (n > 4185 && n < 4195) || (n > 4195 && n < 4199) || (n > 4202 && n < 4211) || n = 4212 || (n > 4213 && n < 4217) || n = 4218 || n = 4220 || (n > 4222 && n < 4231) || n = 4232 || n = 4233 || n = 4235 || n = 4237 || n = 4239 || n = 4241 || n = 4242 || (n > 4243 && n < 4249) || n = 4250 || n = 4252 || (n > 4253 && n < 4257) || n = 4258 || n = 4260 || n = 4261 || n = 4263 || n = 4265 || n = 4266 || n = 4269 || (n > 4270 && n < 4276) || n = 4277 || (n > 4278 && n < 4285) || n = 4286 || (n > 4287 && n < 4295) || n = 4296 || (n > 4297 && n < 4302) || n = 4305 || (n > 4306 && n < 4314) || n = 4315 || n = 4317 || n = 4318 || n = 4320 || (n > 4321 && n < 4327) || (n > 4327 && n < 4333) || n = 4334 || n = 4337 || (n > 4339 && n < 4348) || (n > 4348 && n < 4352) || n = 4356 || (n > 4357 && n < 4369) || n = 4370 || n = 4372 || n = 4374)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132 || n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217 || n = 4234 || n = 4251 || n = 4268 || n = 4285 || n = 4302 || n = 4319 || n = 4336)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4065)
	; 9 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4066 || n = 4083 || n = 4088 || n = 4099 || n = 4100 || n = 4107 || n = 4117 || n = 4126 || n = 4134 || n = 4145 || n = 4151 || n = 4164 || n = 4168 || n = 4185 || n = 4202 || n = 4221 || n = 4236 || n = 4240 || n = 4253 || n = 4259 || n = 4270 || n = 4278 || n = 4287 || n = 4297 || n = 4304 || n = 4316 || n = 4321 || n = 4335 || n = 4338 || n = 4354 || n = 4355 || n = 4373)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4067 || n = 4086 || n = 4105 || n = 4124 || n = 4143 || n = 4162 || n = 4181 || n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295 || n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4069 || n = 4087 || n = 4123 || n = 4141 || n = 4159 || n = 4177 || n = 4195 || n = 4213 || n = 4231 || n = 4249 || n = 4267 || n = 4303 || n = 4339 || n = 4357 || n = 4375)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4075 || n = 4096 || n = 4138 || n = 4180 || n = 4201 || n = 4222 || n = 4243 || n = 4264 || n = 4306 || n = 4327 || n = 4348 || n = 4369)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4097)
	; 9 bytes
	ldx #0
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4116)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
.elseif(n = 4157)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
.elseif(n = 4175)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-6
.elseif(n = 4199)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 4211)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 11
	dex
	Jne *-6
.elseif(n = 4262)
	; 9 bytes
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-6
.elseif(n = 4353)
	; 9 bytes
	ldx #0
	delay_n 12
	dex
	Jne *-5
.endif
.endif
.else
; 4376 to 5000
.if((n > 4375 && n < 4388) || n = 4389 || n = 4391 || (n > 4393 && n < 4407) || n = 4408 || n = 4410 || (n > 4411 && n < 4428) || n = 4431 || (n > 4432 && n < 4445) || n = 4446 || n = 4448 || (n > 4449 && n < 4453) || (n > 4453 && n < 4465) || n = 4467 || (n > 4468 && n < 4474) || (n > 4474 && n < 4483) || n = 4484 || n = 4486 || (n > 4487 && n < 4495) || (n > 4495 && n < 4501) || n = 4502 || n = 4503 || n = 4505 || (n > 4506 && n < 4516) || n = 4517 || n = 4518 || (n > 4519 && n < 4523) || n = 4524 || (n > 4525 && n < 4537) || (n > 4537 && n < 4542) || n = 4543 || (n > 4544 && n < 4555) || n = 4556 || n = 4557 || n = 4559 || n = 4560 || n = 4562 || (n > 4563 && n < 4573) || (n > 4573 && n < 4579) || n = 4581 || (n > 4582 && n < 4591) || (n > 4591 && n < 4599) || (n > 4601 && n < 4609) || (n > 4609 && n < 4618) || (n > 4621 && n < 4637) || n = 4638 || n = 4640 || n = 4641 || (n > 4642 && n < 4656) || n = 4657 || (n > 4658 && n < 4663) || (n > 4663 && n < 4668) || (n > 4668 && n < 4675) || n = 4676 || n = 4678 || (n > 4679 && n < 4684) || (n > 4684 && n < 4694) || n = 4695 || (n > 4696 && n < 4705) || (n > 4705 && n < 4713) || n = 4714 || (n > 4715 && n < 4726) || (n > 4726 && n < 4732) || n = 4733 || (n > 4734 && n < 4747) || (n > 4747 && n < 4751) || n = 4752 || (n > 4753 && n < 4768) || n = 4769 || n = 4771 || (n > 4772 && n < 4789) || n = 4790 || (n > 4791 && n < 4808) || n = 4809 || (n > 4810 && n < 4827) || n = 4828 || n = 4830 || (n > 4831 && n < 4846) || n = 4847 || n = 4849 || n = 4851 || (n > 4852 && n < 4865) || n = 4866 || (n > 4867 && n < 4871) || n = 4872 || (n > 4873 && n < 4894) || (n > 4894 && n < 4913) || n = 4914 || (n > 4915 && n < 4936) || (n > 4936 && n < 4957) || (n > 4957 && n < 4978) || (n > 4978 && n < 4999) || n = 5000)
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4388)
	; 10 bytes
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
.elseif(n = 4390 || n = 4409 || n = 4428 || n = 4447 || n = 4466 || n = 4485 || n = 4504 || n = 4523 || n = 4542 || n = 4561 || n = 4580 || n = 4599 || n = 4618 || n = 4637 || n = 4656 || n = 4675 || n = 4694 || n = 4713 || n = 4732 || n = 4751 || n = 4770 || n = 4789 || n = 4808 || n = 4827 || n = 4846)
	; 9 bytes
	ldx #(n-1)/19
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4392 || n = 4430 || n = 4449 || n = 4468 || n = 4487 || n = 4506 || n = 4525 || n = 4544 || n = 4563 || n = 4582 || n = 4601 || n = 4620 || n = 4639 || n = 4658 || n = 4677 || n = 4696 || n = 4715 || n = 4734 || n = 4753 || n = 4772 || n = 4791 || n = 4829 || n = 4848 || n = 4867)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4393 || n = 4411 || n = 4429 || n = 4483 || n = 4501 || n = 4519 || n = 4537 || n = 4555 || n = 4573 || n = 4591)
	; 10 bytes
	ldx #(n-1)/18
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4407)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 12
	inx
	Jne *-6
.elseif(n = 4432 || n = 4453 || n = 4474 || n = 4495 || n = 4516 || n = 4558 || n = 4579 || n = 4600 || n = 4621 || n = 4642 || n = 4663 || n = 4684 || n = 4705 || n = 4726 || n = 4747 || n = 4768 || n = 4810 || n = 4831 || n = 4852 || n = 4873 || n = 4894 || n = 4915 || n = 4936 || n = 4957 || n = 4978 || n = 4999)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 4445)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4465)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 17
	Jne *-7
.elseif(n = 4609)
	; 10 bytes
	ldx #0
	delay_n 13
	dex
	Jne *-6
.elseif(n = 4619)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
.elseif(n = 4668)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-7
.elseif(n = 4679)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 13
	dex
	Jne *-7
.elseif(n = 4850)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 12
	dex
	dex
	Jne *-7
.elseif(n = 4865)
	; 9 bytes
	ldx #0
	delay_n 14
	dex
	Jne *-5
.elseif(n = 4871)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 14
	inx
	Jne *-6
.elseif(n = 4913)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 14
	dex
	Jne *-6
.endif
.endif
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5020) || (n > 5020 && n < 5041) || (n > 5041 && n < 5062) || (n > 5062 && n < 5083) || (n > 5083 && n < 5104) || (n > 5104 && n < 5125) || (n > 5125 && n < 5146) || (n > 5146 && n < 5167) || (n > 5167 && n < 5188) || (n > 5188 && n < 5209) || (n > 5209 && n < 5230) || (n > 5230 && n < 5251) || (n > 5251 && n < 5272) || (n > 5272 && n < 5277) || (n > 5277 && n < 5293) || (n > 5293 && n < 5312) || n = 5313 || (n > 5314 && n < 5335) || (n > 5335 && n < 5356) || (n > 5356 && n < 5377) || (n > 5377 && n < 5381) || (n > 5381 && n < 10001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5020 || n = 5041 || n = 5062 || n = 5083 || n = 5104 || n = 5125 || n = 5146 || n = 5167 || n = 5188 || n = 5209 || n = 5230 || n = 5251 || n = 5272 || n = 5293 || n = 5314 || n = 5335 || n = 5356)
	; 10 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5277)
	; 10 bytes
	ldx #202 ;hides 'dex'
	.repeat 3
	 php
	 plp
	.endrepeat
	Jne *-7
.elseif(n = 5312)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 14
	dex
	dex
	Jne *-7
.elseif(n = 5377)
	; 10 bytes
	ldx #0
	delay_n 16
	dex
	Jne *-6
.elseif(n = 5381)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 16
	dex
	Jne *-7
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
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
	rest
.endmacro
